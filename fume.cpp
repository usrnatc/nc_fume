#include "libnc.h"

#include "mavlink.h"
#include "tlog.h"

#include "mavlink.cpp"
#include "tlog.cpp"

#define FUME_MAX_LANE_COUNT  256
#define FUME_DUMP_PRINT_SIZE MB(4)

struct FUMEParams {
    CommandLine* CLI;
    u64          ProblemsMax;
    b32          ShouldReportMsgs;
    b32          ShouldDump;
};

struct FUMELaneParams {
    LaneContext LaneCtx;
    FUMEParams* Params;
};

internal void
FUMEPushPaths(Arena* MemPool, Str8List* Paths, Str8List* Inputs)
{
    for (Str8Node* Node = Inputs->Head; Node; Node = Node->Next) {
        FileProperties Props = SystemGetFileProperties(Node->String);

        if (!(Props.Kind & SYS_FILE_IS_DIR)) {
            ListPush(MemPool, Paths, Node->String);
            continue;
        }

        Str8 Dir = StrTrimLastSlash(Node->String);
        FileIter* Iter = SystemFileIterBegin(
            MemPool, 
            Dir, 
            SYS_FILE_ITER_SKIP_DIRS
        );
        FileInfo Info = {};

        while (SystemFileIterNext(MemPool, Iter, &Info)) {
            if (Info.Properties.Kind & SYS_FILE_IS_DIR)
                continue;

            // NOTE(nc): we only care about *.tlog or *.TLOG files
            if (
                !StrMatch(
                    StrSkipLastDot(Info.Name), 
                    "tlog"_s8, 
                    STR_MATCH_ALL_CASES
                )
            ) {
                continue;
            }

            ListPushFmt(
                MemPool,
                Paths,
                "%S/%S",
                PRINT_STR(Dir),
                PRINT_STR(Info.Name)
            );
        }

        SystemFileIterEnd(Iter);
    }
}

internal void
FUMEAnalyse(FUMEParams* Params)
{
    TempArena Scratch = GetScratch(NULL, 0);
    Str8Array Paths = {};
    TLOGLane* Lanes = NULL;
    TLOGStats* Stats = NULL;
    Str8List FileRows = {};
    u64 TotalBytes = 0;
    PerfCounter StartTime = TimeGetTimestamp();

    if (!LaneIndex()) {
        Str8List PathList = {};

        FUMEPushPaths(Scratch.MemPool, &PathList, &Params->CLI->Inputs);
        Paths = StrArrayFromList(Scratch.MemPool, &PathList);
        Lanes = ArenaPushArrayZero(Scratch.MemPool, TLOGLane, LaneCount());
        Stats = ArenaPushArray(Scratch.MemPool, TLOGStats, 1);
        TLOGReportFileRowHeader(Scratch.MemPool, &FileRows);

        for (u64 LIndex = 0; LIndex < LaneCount(); ++LIndex) {
            Lanes[LIndex].Problems = ArenaPushArray(
                Scratch.MemPool,
                TLOGProblem,
                Params->ProblemsMax
            );
        }
    }

    LaneSyncU64(&Paths.Data, 0);
    LaneSyncU64(&Paths.Count, 0);
    LaneSyncU64(&Lanes, 0);

    for (u64 PIndex = 0; PIndex < Paths.Count; ++PIndex) {
        Handle File = {};
        Handle Map = {};
        u8* Base = NULL;
        u64 Size = 0;

        if (!LaneIndex()) {
            File = SystemOpenFile(
                SYS_ACCESS_READ | SYS_ACCESS_SHARE_READ,
                Paths.Data[PIndex]
            );
            Size = SystemGetFileProperties(File).Size;

            if (File != EMPTY_HANDLE_VALUE && Size >= TLOG_RECORD_SIZE_MIN) {
                Map = SystemOpenFileMap(SYS_ACCESS_READ, File);
                Base = (u8*) SystemOpenFileMapView(
                    Map,
                    SYS_ACCESS_READ,
                    Rng((u64) 0, Size)
                );
            }

            if (!Base) {
                PrintErr(
                    "[ERROR] :: FUME cannot read \"%S\". The file does not exist or is empty\n",
                    PRINT_STR(Paths.Data[PIndex])
                );
            }
        }

        LaneSyncU64(&Base, 0);
        LaneSyncU64(&Size, 0);

        if (Base) {
            TLOGLane* Lane = &Lanes[LaneIndex()];
            TLOGProblem* Problems = Lane->Problems;
            r1u64 Range = LaneRange(Size);

            MemZero(Lane, sizeof(*Lane));
            Lane->Problems = Problems;
            Lane->HeadOffset = Range.Min
                ? TLOGFindRecord(Base, Size, Range.Min, Range.Max)
                : 0;

            if (Range.Min && Lane->HeadOffset == Range.Max)
                Lane->HeadOffset = TLOG_OFFSET_NONE;

            LaneSync();

            if (Lane->HeadOffset != TLOG_OFFSET_NONE) {
                u64 Limit = Size;

                for (u64 LIndex = LaneIndex() + 1; LIndex < LaneCount(); ++LIndex) {
                    if (Lanes[LIndex].HeadOffset != TLOG_OFFSET_NONE) {
                        Limit = Lanes[LIndex].HeadOffset;
                        break;
                    }
                }

                TLOGWalk(
                    Base,
                    Size,
                    Lane->HeadOffset,
                    Limit,
                    Lane,
                    Params->ProblemsMax
                );
            }

            LaneSync();
        }

        if (!LaneIndex() && Base) {
            MemZero(Stats, sizeof(*Stats));

            for (u64 LIndex = 0; LIndex < LaneCount(); ++LIndex)
                TLOGStatsMerge(Stats, &Lanes[LIndex].Stats);

            TLOGReportFileRow(
                Scratch.MemPool, 
                &FileRows, 
                Paths.Data[PIndex], 
                Stats
            );

            Str8List Strings = {};

            TLOGReport(
                Scratch.MemPool,
                &Strings,
                Paths.Data[PIndex],
                PIndex,
                Paths.Count,
                Base,
                Size,
                Stats,
                Lanes,
                LaneCount(),
                Params->ProblemsMax,
                Params->ShouldReportMsgs
            );
            PrintOut(StrListJoin(Scratch.MemPool, &Strings, NULL));
            TotalBytes += Size;
        }

        if (Base && Params->ShouldDump) {
            Arena* DumpMemPool = NULL;
            u8* Lines = NULL;
            u64 LinesCount = 0;

            if (!LaneIndex()) {
                for (u64 LIndex = 0; LIndex < LaneCount(); ++LIndex) {
                    TLOGStats* LaneStats = &Lanes[LIndex].Stats;

                    Lanes[LIndex].DumpHeadLine = LinesCount;
                    LinesCount += (
                        LaneStats->KindCounts[MAVLINK_FRAME_KIND_OKAY] +
                        LaneStats->KindCounts[MAVLINK_FRAME_KIND_BAD_CRC] +
                        LaneStats->KindCounts[MAVLINK_FRAME_KIND_UNKNOWN_MSG] +
                        LaneStats->JunkRuns
                    );
                }

                DumpMemPool = ArenaAlloc(
                    __Params.ReserveSize = ((LinesCount + 1) * TLOG_DUMP_LINE_SIZE) + MB(1)
                );
                Lines = ArenaPushArray(
                    DumpMemPool,
                    u8,
                    (LinesCount + 1) * TLOG_DUMP_LINE_SIZE
                );
                TLOGDumpHeader(Lines);
            }

            LaneSyncU64(&Lines, 0);

            if (Lanes[LaneIndex()].HeadOffset != TLOG_OFFSET_NONE) {
                u64 Limit = Size;

                for (
                    u64 LIndex = LaneIndex() + 1; 
                    LIndex < LaneCount(); 
                    ++LIndex
                ) {
                    if (Lanes[LIndex].HeadOffset != TLOG_OFFSET_NONE) {
                        Limit = Lanes[LIndex].HeadOffset;

                        break;
                    }
                }

                TLOGDump(
                    Base,
                    Size,
                    Lanes[LaneIndex()].HeadOffset,
                    Limit,
                    Lines + ((Lanes[LaneIndex()].DumpHeadLine + 1) * TLOG_DUMP_LINE_SIZE)
                );
            }

            LaneSync();

            if (!LaneIndex()) {
                Str8 Remaining = {
                    Lines,
                    (LinesCount + 1) * TLOG_DUMP_LINE_SIZE
                };

                PrintOut(
                    "\n  %SALL PACKETS IN THE FILE%S\n",
                    PRINT_STR(TLOG_STYLES[TLOG_STYLE_HEADING]),
                    PRINT_STR(TLOG_STYLES[TLOG_STYLE_RESET])
                );

                while (Remaining.Size) {
                    Str8 Part = StrPrefix(Remaining, FUME_DUMP_PRINT_SIZE);

                    PrintOut(Part);
                    Remaining = StrSkip(Remaining, Part.Size);
                }

                ArenaRelease(DumpMemPool);
            }
        }

        if (!LaneIndex()) {
            if (Base)
                SystemCloseFileMapView(Map, Base, Rng((u64) 0, Size));

            if (Map != EMPTY_HANDLE_VALUE)
                SystemCloseFileMap(Map);

            if (File != EMPTY_HANDLE_VALUE)
                SystemCloseFile(File);
        }
    }

    if (!LaneIndex()) {
        f64 ElapsedSecs = TimeElapsedSec(StartTime, TimeGetTimestamp());
        Str8List Strings = {};

        ListPush(Scratch.MemPool, &Strings, "\n"_s8);
        TLOGReportRule(Scratch.MemPool, &Strings);

        if (Paths.Count > 1) {
            ListPush(
                Scratch.MemPool, 
                &Strings, 
                StrListJoin(Scratch.MemPool, &FileRows, NULL)
            );
            ListPush(Scratch.MemPool, &Strings, "\n"_s8);
        }

        ListPushFmt(
            Scratch.MemPool,
            &Strings,
            "  %SFILES%S %llu    %SSIZE%S %.1f MB    %STIME%S %.3f s    %SLANES%S %llu    %SRATE%S %.1f MB/s\n",
            PRINT_STR(TLOG_STYLES[TLOG_STYLE_DIM]),
            PRINT_STR(TLOG_STYLES[TLOG_STYLE_RESET]),
            Paths.Count,
            PRINT_STR(TLOG_STYLES[TLOG_STYLE_DIM]),
            PRINT_STR(TLOG_STYLES[TLOG_STYLE_RESET]),
            (f64) TotalBytes / (f64) MB(1),
            PRINT_STR(TLOG_STYLES[TLOG_STYLE_DIM]),
            PRINT_STR(TLOG_STYLES[TLOG_STYLE_RESET]),
            ElapsedSecs,
            PRINT_STR(TLOG_STYLES[TLOG_STYLE_DIM]),
            PRINT_STR(TLOG_STYLES[TLOG_STYLE_RESET]),
            LaneCount(),
            PRINT_STR(TLOG_STYLES[TLOG_STYLE_DIM]),
            PRINT_STR(TLOG_STYLES[TLOG_STYLE_RESET]),
            ((f64) TotalBytes / (f64) MB(1)) / MAX(ElapsedSecs, 0.000001)
        );
        TLOGReportRule(Scratch.MemPool, &Strings);
        PrintOut(StrListJoin(Scratch.MemPool, &Strings, NULL));
        ReleaseScratch(Scratch);
    }
}

internal void
FUMELaneEntryPoint(void* Params)
{
    FUMELaneParams* LaneParams = (FUMELaneParams*) Params;

    ThreadSetName("[LANE %llu]", LaneParams->LaneCtx.Index);
    SetLaneContext(LaneParams->LaneCtx);
    FUMEAnalyse(LaneParams->Params);
}

void 
EntryPoint(CommandLine* CLI)
{
    if (
        !CLI->Inputs.Count || 
        CommandLineHasFlag(CLI, "help"_s8) ||
        CommandLineHasFlag(CLI, "h"_s8) ||
        CommandLineHasFlag(CLI, "?"_s8)
    ) {
        PrintOut(
            "Usage: %S [options] <file.tlog | directory> ...\n"
            "\t--threads=N  Number of threads. The default is the number of logical processors.\n"
            "\t--bad=N      Number of incorrect packets that the report shows for each file. The default is %u.\n"
            "\t--msgs       Show the number of correct and incorrect packets for each message.\n"
            "\t--dump       Show all packets in the sequence that they have in the file. FUME does not show payloads.\n"
            "\t--no-colour  Do not use colour in the report.\n"
            "\t--help\n"
            "\t--h\n"
            "\t--?          Show this usage message and quit\n",
            PRINT_STR(StrSkipLastSlash(CLI->ProgramName)),
            (u32) TLOG_PROBLEMS_MAX_DEFAULT
        );

        return;
    }

    FUMEParams Params = {};
    u64 Lanes = GetSystemProperties()->LogicalProcessorCount;

    Params.CLI = CLI;
    Params.ProblemsMax = TLOG_PROBLEMS_MAX_DEFAULT;
    Params.ShouldReportMsgs = CommandLineHasFlag(CLI, "msgs"_s8);
    Params.ShouldDump = CommandLineHasFlag(CLI, "dump"_s8);

    if (CommandLineHasArgument(CLI, "bad"_s8))
        Params.ProblemsMax = U64FromStr(CommandLineString(CLI, "bad"_s8));

    if (CommandLineHasArgument(CLI, "threads"_s8))
        Lanes = U64FromStr(CommandLineString(CLI, "threads"_s8));

    Lanes = CLAMP(1, Lanes, FUME_MAX_LANE_COUNT);
    MAVLinkInit();
    TLOGStylesInit(!CommandLineHasFlag(CLI, "no-colour"_s8));

    TempArena Scratch = GetScratch(NULL, 0);
    FUMELaneParams* LaneParams = ArenaPushArrayZero(
        Scratch.MemPool,
        FUMELaneParams,
        Lanes
    );
    Handle* Threads = ArenaPushArrayZero(Scratch.MemPool, Handle, Lanes);
    Handle Barrier = BarrierAlloc(Lanes);
    u64 BroadcastMemory = 0;

    for (u64 Index = 0; Index < Lanes; ++Index) {
        LaneParams[Index].LaneCtx.Index = Index;
        LaneParams[Index].LaneCtx.Count = Lanes;
        LaneParams[Index].LaneCtx.Barrier = Barrier;
        LaneParams[Index].LaneCtx.BroadcastMemory = &BroadcastMemory;
        LaneParams[Index].Params = &Params;
        Threads[Index] = ThreadLaunch(FUMELaneEntryPoint, &LaneParams[Index]);
    }

    for (u64 Index = 0; Index < Lanes; ++Index)
        ThreadJoin(Threads[Index], U64_MAX);

    BarrierRelease(Barrier);
    ReleaseScratch(Scratch);
}
