#include "libnc.h"

#include "mavlink.h"
#include "tlog.h"

#include "mavlink.cpp"
#include "tlog.cpp"

#define FUME_MAX_LANE_COUNT  256
#define FUME_DUMP_PRINT_SIZE MB(4)

struct FUMEParams {
    CommandLine* CLI;
    TLOGFilter   Filter;
    r1u64        TimeRange;
    Str8         ExtractPath;
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
FUMEFilterPushIDs(u64* Words, u64 IDsCount, Str8List Strings)
{
    if (!Strings.Count) {
        MemSet(Words, 0xFF, IDsCount >> 3);

        return;
    }

    for (Str8Node* Node = Strings.Head; Node; Node = Node->Next) {
        u64 ID = IDsCount;

        if (Node->String.Size && IsDigit(Node->String.Str[0])) {
            ID = U64FromStr(Node->String);
        } else {
            for (u32 Slot = 0; Slot < MAVLINK_MSG_COUNT; ++Slot) {
                if (
                    StrMatch(
                        Node->String, 
                        MAVLINK_MSG_NAMES[Slot], 
                        STR_MATCH_ALL_CASES
                    )
                ) {
                    ID = MAVLINK_MSG_IDS[Slot];
                    break;
                }
            }
        }

        if (ID < IDsCount)
            Words[ID >> 6] |= (1ULL << (ID & 63));
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
            Lanes[LIndex].Hiding = ArenaPushArray(
                Scratch.MemPool,
                TLOGProblem,
                Params->ProblemsMax
            );
        }
    }

    LaneSyncU64(&Paths.Data, 0);
    LaneSyncU64(&Paths.Count, 0);
    LaneSyncU64(&Lanes, 0);

    if (Params->ShouldDump)
        Lanes[LaneIndex()].Dump = ArenaAllocEx(GB(1));

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
            } else {
                MemPrefetch(Base, Size);
            }

            if (Base && Length(Params->TimeRange)) {
                u64 HeadOffset = TLOGFindRecord(Base, Size, 0, Size);
                u64 HeadTime = (HeadOffset < Size)
                    ? SwapByteOrder(*(u64*) (Base + HeadOffset))
                    : 0;

                Params->Filter.Times = Shift(Params->TimeRange, HeadTime);
            }
        }

        LaneSyncU64(&Base, 0);
        LaneSyncU64(&Size, 0);

        r1u64 Window = Intersect(Rng((u64) 0, Size), Params->Filter.Offsets);

        if (Window.Max < Window.Min)
            Window.Max = Window.Min;

        if (Base) {
            TLOGLane* Lane = &Lanes[LaneIndex()];
            r1u64 Range = Shift(LaneRange(Length(Window)), Window.Min);

            MemZero(&Lane->Stats, sizeof(Lane->Stats));
            Lane->ProblemsCount = 0;
            Lane->HidingCount = 0;
            Lane->HeadOffset = Range.Min
                ? TLOGFindRecord(Base, Size, Range.Min, Range.Max)
                : 0;
            Lane->Limit = Window.Max;

            if (Range.Min && Lane->HeadOffset == Range.Max)
                Lane->HeadOffset = TLOG_OFFSET_NONE;

            LaneSync();

            for (
                u64 LIndex = LaneIndex() + 1; 
                LIndex < LaneCount(); 
                ++LIndex
            ) {
                if (Lanes[LIndex].HeadOffset != TLOG_OFFSET_NONE) {
                    Lane->Limit = Lanes[LIndex].HeadOffset;
                    break;
                }
            }

            if (Lane->HeadOffset != TLOG_OFFSET_NONE) {
                TLOGWalk(
                    Base,
                    Size,
                    Lane->HeadOffset,
                    Lane->Limit,
                    Lane,
                    Params->ProblemsMax,
                    &Params->Filter
                );
            }

            LaneSync();
        }

        if (!LaneIndex() && Base) {
            MemZero(Stats, sizeof(*Stats));

            for (u64 LIndex = 0; LIndex < LaneCount(); ++LIndex)
                TLOGStatsMerge(Stats, &Lanes[LIndex].Stats, Base);

            TLOGSourcesSort(Stats);
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
                Params->ShouldReportMsgs,
                &Params->Filter
            );
            PrintOut(StrListJoin(Scratch.MemPool, &Strings, NULL));
            TotalBytes += Size;

            if (Params->ExtractPath.Size) {
                Str8 Bytes = { Base + Window.Min, Length(Window) };
                Str8 CountStr = TLOGStrFromCount(Scratch.MemPool, Bytes.Size);

                if (WriteFileContents(Params->ExtractPath, Bytes)) {
                    PrintOut(
                        "\n    FUME wrote %S bytes from offset 0x%llX to \"%S\"\n",
                        PRINT_STR(CountStr),
                        Window.Min,
                        PRINT_STR(Params->ExtractPath)
                    );
                } else {
                    PrintErr(
                        "[ERROR] :: FUME cannot write \"%S\"\n",
                        PRINT_STR(Params->ExtractPath)
                    );
                }
            }
        }

        if (Base && Params->ShouldDump) {
            TLOGLane* Lane = &Lanes[LaneIndex()];

            ArenaClear(Lane->Dump);

            if (Lane->HeadOffset != TLOG_OFFSET_NONE) {
                TLOGDump(
                    Base,
                    Size,
                    Lane->HeadOffset,
                    Lane->Limit,
                    Lane->Dump,
                    &Params->Filter
                );
            }

            LaneSync();

            if (!LaneIndex()) {
                PrintOut(
                    "\n  %SALL PACKETS IN THE FILE%S\n"
                    "%-12s %-26s %7s %6s %9s %8s %10s  %-40s %6s  %s\n",
                    PRINT_STR(TLOG_STYLES[TLOG_STYLE_HEADING]),
                    PRINT_STR(TLOG_STYLES[TLOG_STYLE_RESET]),
                    "OFFSET",
                    "TIMESTAMP (UTC)",
                    "MAVLINK",
                    "SYSTEM",
                    "COMPONENT",
                    "SEQUENCE",
                    "MESSAGE ID",
                    "MESSAGE",
                    "LENGTH",
                    "CHECKSUM"
                );

                for (u64 LIndex = 0; LIndex < LaneCount(); ++LIndex) {
                    Arena* Dump = Lanes[LIndex].Dump;
                    Str8 Remaining = {
                        ArenaExBase(Dump),
                        ArenaGetPosition(Dump) - ARENA_HEADER_SIZE
                    };

                    while (Remaining.Size) {
                        Str8 Part = StrPrefix(Remaining, FUME_DUMP_PRINT_SIZE);

                        PrintOut(Part);
                        Remaining = StrSkip(Remaining, Part.Size);
                    }
                }
            }

            LaneSync();
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

    if (Params->ShouldDump)
        ArenaRelease(Lanes[LaneIndex()].Dump);

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

struct FUMEFilesViewState {
    v2i64 Cursor;
    v2i64 Mark;
};
 
void
AppViewUI(r2f32 Rect)
{
    ConfigNode* View = ConfigNodeFromID(Registers()->View);
    AppViewState* VS = AppViewStateFromConfig(View);
    Str8 ViewName = View->String;
    ConfigNode* QueryRoot = ConfigNodeChildFromStr(View, "query"_s8);
    ConfigNode* InputRoot = ConfigNodeChildFromStr(QueryRoot, "input"_s8);
    ConfigNode* CmdRoot = ConfigNodeChildFromStr(QueryRoot, "cmd"_s8);
    Str8 CurrentInput = InputRoot->Head->String;
 
    if (VS->QueryIsOpen) {
        Str8 CommandName = CmdRoot->Head->String;
        AppCommandKindInfo* CommandInfo = AppCommandKindInfoFromStr(CommandName);
 
        VS->QueryStringSize = MIN(sizeof(VS->QueryBuffer), CurrentInput.Size);
        MemCpy(
            VS->QueryBuffer,
            CurrentInput.Str,
            VS->QueryStringSize
        );
 
        if (!VS->QueryCursor.Column) {
            VS->QueryMark = TxtPt(1, 1);
            VS->QueryCursor = TxtPt(1, VS->QueryStringSize + 1);
        }
 
        Rect.Y0 += 1.0f;
 
        UIBox* SearchRow = EMPTY_UI_BOX_VALUE;
 
        UITag("pop"_s8) {
            UISetNextChildLayoutAxis(AXIS_2D_X);
            SearchRow = UIBuildBoxFromStr(
                UI_BOX_KIND_DRAW_BACKGROUND,
                "###search"_s8
            );
 
            UIParent(SearchRow) {
                UIFocus(VS->QueryIsOpen && !VS->ContentsAreFocused ? UI_FOCUS_KIND_ON : UI_FOCUS_KIND_OFF) {
                    if (CommandName.Size) {
                        UIPreferredWidth(UI_TEXT_DIM(2.0f, 1.0f)) {
                            UILabel(CommandInfo->DisplayName);
                        }
                    }
 
                    UIWidthFill() {
                        UISignal Sig = UILineEdit(
                            &VS->QueryCursor,
                            &VS->QueryMark,
                            VS->QueryBuffer,
                            sizeof(VS->QueryBuffer),
                            &VS->QueryStringSize,
                            CurrentInput,
                            "###search_edit"_s8
                        );
 
                        if (UI_PRESSED(Sig)) {
                            VS->QueryIsOpen = TRUE;
                            VS->ContentsAreFocused = FALSE;
                            AppCmd(APP_COMMAND_KIND_FOCUS_PANEL);
                        }
                    }
                }
            }
        }
 
        if (!VS->ContentsAreFocused)
            APP_STATE->TextEditMode = TRUE;
 
        if (InputRoot == EMPTY_CFG_NODE_VALUE) {
            InputRoot = ConfigNodeChildFromStrOrAlloc(
                APP_STATE->Config,
                QueryRoot,
                "input"_s8
            );
        }
 
        ConfigNodeNewReplace(
            APP_STATE->Config,
            InputRoot,
            Str(VS->QueryBuffer, VS->QueryStringSize)
        );
    }
 
    for (
        UIEvent* Evt = NULL;
        UINextEvent(&Evt);
    ) {
        if (
            Evt->Kind == UI_EVENT_KIND_PRESS &&
            Evt->Input == INPUT_KIND_LEFT_MOUSE_BTN &&
            InRange(Rect, Evt->Position)
        ) {
            VS->ContentsAreFocused = TRUE;
            break;
        }
    }
 
    UIBox* ViewContainer = EMPTY_UI_BOX_VALUE;
 
    UIWidthFill() {
        UIHeightFill() {
            UISetNextChildLayoutAxis(AXIS_2D_Y);
            ViewContainer = UIBuildBoxFromKey(0, EMPTY_UI_KEY_VALUE);
        }
    }
 
    UIParent(ViewContainer) {
        UIFocus(VS->QueryIsOpen && !VS->ContentsAreFocused ? UI_FOCUS_KIND_OFF : UI_FOCUS_KIND_NULL) {
            if (StrMatch(ViewName, "files"_s8, 0)) {
                TempArena Scratch = GetScratch(NULL, 0);
                FUMEFilesViewState* State = GetAppViewState(FUMEFilesViewState);
                ConfigNodePtrList FileList = ConfigNodeTopLevelListFromStr(
                    Scratch.MemPool,
                    "file"_s8
                );
                ConfigNodePtrArray Files = ConfigNodePtrArrayFromList(
                    Scratch.MemPool,
                    &FileList
                );
                UIScrollPoint2D ScrollPosition = AppViewScrollPosition();
 
                State->Cursor.Y = MIN(State->Cursor.Y, (i64) Files.Count);
                State->Mark = State->Cursor;
 
                if (!Files.Count) {
                    UIPadding(UI_PERCENT(1.0f, 0.0f)) {
                        UIWidthFill() {
                            UITextAlignment(UI_TEXT_ALIGN_CENTRE) {
                                UITag("weak"_s8) {
                                    UILabel("There are no files in the list."_s8);
                                    UILabel("Press Ctrl+O to open a tlog file or a folder. You can also drop files into the terminal."_s8);
                                }
                            }
                        }
                    }
                } else {
                    UIScrollListParameters Params = {};
                    UIScrollListSignal ListSig = {};
                    r1i64 Visible = {};
                    Str8 AcceptedPath = {};
 
                    Params.Kind = UI_SCROLL_LIST_KIND_ALL;
                    Params.DimensionsPX = Length(Rect);
                    Params.RowHeightPX = 1.0f;
                    Params.CursorRange.Max.Y = (i64) Files.Count;
                    Params.ItemRange = Rng((i64) 0, (i64) Files.Count);
                    Params.CursorMinIsEmptySelection[AXIS_2D_Y] = TRUE;
 
                    UIFocus(UI_FOCUS_KIND_ON) {
                        UIScrollList(
                            &Params,
                            &ScrollPosition.Y,
                            &State->Cursor,
                            &State->Mark,
                            &Visible,
                            &ListSig
                        ) {
                            for (
                                i64 Row = Visible.Min;
                                Row <= Visible.Max && Row < (i64) Files.Count;
                                ++Row
                            ) {
                                Str8 Path = Files.V[Row]->Head->String;
                                b32 IsCursor = (State->Cursor.Y == Row + 1);
 
                                UITag(IsCursor ? "pop"_s8 : ""_s8) {
                                    UISetNextChildLayoutAxis(AXIS_2D_X);
 
                                    UIBox* RowBox = UIBuildBoxFromStrFmt(
                                        (
                                            UI_BOX_KIND_MOUSE_CLICKABLE |
                                            UI_BOX_KIND_DRAW_BACKGROUND |
                                            UI_BOX_KIND_DRAW_HOT_EFFECTS
                                        ),
                                        "###file_row_%p",
                                        Files.V[Row]
                                    );
 
                                    UIParent(RowBox) {
                                        UIWidthFill() {
                                            UILabel(Path);
                                        }
                                    }
 
                                    UISignal RowSig = UISignalFromBox(RowBox);
 
                                    if (UI_PRESSED(RowSig)) {
                                        State->Cursor.Y = Row + 1;
                                        State->Mark = State->Cursor;
                                    }
 
                                    if (UI_DOUBLE_CLICKED(RowSig))
                                        AcceptedPath = Path;
                                }
                            }
                        }
 
                        if (
                            UIIsFocusActive() &&
                            State->Cursor.Y > 0 &&
                            UISlotPress(UI_EVENT_ACTION_SLOT_ACCEPT)
                        ) {
                            AcceptedPath = Files.V[State->Cursor.Y - 1]->Head->String;
                        }
                    }
 
                    // NOTE(nc): the selected file goes to the registers, thus
                    //         : the commands that need a file work from this view
                    if (State->Cursor.Y > 0) {
                        Registers()->FilePath = ArenaPushStrCpy(
                            AppFrameMemPool(),
                            Files.V[State->Cursor.Y - 1]->Head->String
                        );
                    }
 
                    if (AcceptedPath.Size) {
                        AppCmd(
                            APP_COMMAND_KIND_OPEN_REPORT,
                            __Registers.FilePath = AcceptedPath
                        );
                    }
                }
 
                ScrollPosition.Y.Offset = 0.0f;
                AppStoreViewScrollPosition(ScrollPosition);
                ReleaseScratch(Scratch);
            } else if (StrMatch(ViewName, "pending"_s8, 0)) {
                // NOTE(nc): the view asks for the first bytes of the file. When
                //         : the file stream has them, the tab changes to the report
                Str8 FilePath = AppViewFilePath();
                CKey Key = FSKeyFromPathRange(FilePath, Rng((u64) 0, (u64) KB(4)), 0);
                u128 KeyHash = CHashFromKey(Key, 0);
 
                AppStoreViewLoadingInfo(TRUE, 0, 0);
 
                if (KeyHash != u128{}) {
                    ConfigNodeEquipStr(
                        APP_STATE->Config,
                        View,
                        APP_COMMAND_KIND_INFO_TABLE[APP_COMMAND_KIND_OPEN_REPORT].String
                    );
 
                    for (
                        AppArenaExt* Ext = VS->HeadArenaExt;
                        Ext;
                        Ext = Ext->Next
                    ) {
                        ArenaRelease(Ext->MemPool);
                    }
 
                    ArenaPopTo(VS->MemPool, VS->MemPoolResetPosition);
                    VS->UserData = NULL;
                    VS->HeadArenaExt = NULL;
                    VS->TailArenaExt = NULL;
                    AppRequestFrame();
                }
            } else {
                // TODO(nc): report, incorrect packets, lost packets, all packets,
                // TODO    : messages and sources views. They need the TLI
                TempArena Scratch = GetScratch(NULL, 0);
                FancyStrList Title = AppTitleFStrFromConfig(Scratch.MemPool, View);
 
                UIPadding(UI_PERCENT(1.0f, 0.0f)) {
                    UIWidthFill() {
                        UITextAlignment(UI_TEXT_ALIGN_CENTRE) {
                            UIBox* TitleBox = UIBuildBoxFromKey(UI_BOX_KIND_DRAW_TEXT, {});
 
                            UIBoxEquipDisplayFancyStrs(TitleBox, &Title);
 
                            UITag("weak"_s8) {
                                UILabel("FUME does not have this view yet."_s8);
                            }
                        }
                    }
                }
 
                ReleaseScratch(Scratch);
            }
        }
    }
 
    if (VS->QueryIsOpen) {
        UIFocus(UI_FOCUS_KIND_ON) {
            if (UIIsFocusActive() && UISlotPress(UI_EVENT_ACTION_SLOT_CANCEL)) {
                VS->QueryIsOpen = FALSE;
                VS->QueryStringSize = 0;
            }
 
            if (UIIsFocusActive() && UISlotPress(UI_EVENT_ACTION_SLOT_ACCEPT)) {
                Str8 CommandName = AppViewQueryCommand();
                Str8 Input = Str(VS->QueryBuffer, VS->QueryStringSize);
                AppCommandKindInfo* CommandKindInfo = AppCommandKindInfoFromStr(
                    CommandName
                );
 
                AppRegistersScope() {
                    AppRegistersFillSlotFromStr(
                        CommandKindInfo->Query.Slot,
                        ""_s8,
                        Input
                    );
                    AppCmd(APP_COMMAND_KIND_COMPLETE_QUERY);
                }
            }
        }
    }
 
    VS->LastFrameIndexBuilt = APP_STATE->FrameIndex;
}
 
internal void
AsyncThreadEntryPoint(void* Params)
{
    LaneContext LaneCtx = *(LaneContext*) Params;
 
    SetLaneContext(LaneCtx);
    GetTLS()->IS_ASYNC_THREAD = TRUE;
    ThreadSetName("[ASYNC %llu]", LaneIndex());
 
    for (;;) {
        if (!LaneIndex()) {
            if (!AtomicLoadU32(&ASYNC_LOOP_REPEAT, MEM_ORDER_SEQ_CST)) {
                LOCK_SCOPE(ASYNC_TICK_BEGIN_MTX) {
                    CondVarWait(
                        ASYNC_TICK_BEGIN_COND_VAR,
                        ASYNC_TICK_BEGIN_MTX,
                        TimeNow() + SECONDS(1)
                    );
                }
            }
 
            AtomicExchangeU32(&ASYNC_LOOP_REPEAT, 0, MEM_ORDER_SEQ_CST);
            AtomicExchangeU32(&ASYNC_LOOP_REPEAT_HIGH_PRIORITY, 0, MEM_ORDER_SEQ_CST);
        }
 
        LaneSync();
        OCAsyncTick();
        CAsyncTick();
        FSAsyncTick();
        LogAsyncTick();
        LaneSync();
 
        b32 ShouldQuit = FALSE;
 
        if (!LaneIndex())
            ShouldQuit = AtomicLoadU32(&GLOBAL_ASYNC_EXIT, MEM_ORDER_SEQ_CST);
 
        LaneSyncU64(&ShouldQuit, 0);
 
        if (ShouldQuit)
            break;
    }
}
 
internal void
FUMEInteractive(CommandLine* CLI)
{
    TempArena Scratch = GetScratch(NULL, 0);
 
    ASYNC_TICK_BEGIN_COND_VAR = CondVarAlloc();
    ASYNC_TICK_BEGIN_MTX = MutexAlloc();
    ASYNC_TICK_END_MTX = MutexAlloc();
 
    OCInit();
    CInit();
    FSInit();
    MDInit();
    ConfigInit();
    MAVLinkInit();
    TerminalInit();
    RendInit(CLI);
    DrawInit();
    UIInit();
 
    Handle* AsyncThreads = NULL;
    u64 LaneBroadcastValue = 0;
    u64 MainThreadCount = 1;
    u64 AsyncThreadsCount = GetSystemProperties()->LogicalProcessorCount;
    u64 MainThreadCountClamped = MIN(AsyncThreadsCount, MainThreadCount);
 
    AsyncThreadsCount -= MainThreadCountClamped;
 
    Str8 AsyncThreadsCountString = CommandLineString(CLI, "threads"_s8);
 
    if (AsyncThreadsCountString.Size)
        AsyncThreadsCount = U64FromStr(AsyncThreadsCountString);
 
    AsyncThreadsCount = CLAMP(1, AsyncThreadsCount, FUME_MAX_LANE_COUNT);
 
    Handle Barrier = BarrierAlloc(AsyncThreadsCount);
    LaneContext* LaneCtxs = ArenaPushArrayZero(
        Scratch.MemPool,
        LaneContext,
        AsyncThreadsCount
    );
 
    ASYNC_THREADS_COUNT = AsyncThreadsCount;
    AsyncThreads = ArenaPushArrayZero(Scratch.MemPool, Handle, ASYNC_THREADS_COUNT);
 
    for (u64 Index = 0; Index < ASYNC_THREADS_COUNT; ++Index) {
        LaneCtxs[Index].Index = Index;
        LaneCtxs[Index].Count = ASYNC_THREADS_COUNT;
        LaneCtxs[Index].Barrier = Barrier;
        LaneCtxs[Index].BroadcastMemory = &LaneBroadcastValue;
        AsyncThreads[Index] = ThreadLaunch(AsyncThreadEntryPoint, &LaneCtxs[Index]);
    }
 
    AppInit(CLI);
 
    for (; !APP_STATE->ShouldQuit; )
        AppFrame();
 
    AtomicIncFetchU32(&GLOBAL_ASYNC_EXIT, MEM_ORDER_SEQ_CST);
    CondVarBroadcast(ASYNC_TICK_BEGIN_COND_VAR);
 
    for (u32 Index = 0; Index < ASYNC_THREADS_COUNT; ++Index)
        ThreadJoin(AsyncThreads[Index], U64_MAX);
 
    BarrierRelease(Barrier);
    RendRelease();
    TerminalRelease();
    ReleaseScratch(Scratch);
}

void 
EntryPoint(CommandLine* CLI)
{
    if (CommandLineHasFlag(CLI, "tui"_s8)) {
        FUMEInteractive(CLI);
 
        return;
    }

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
            "\t--sys=A,B    Count only the packets that have one of these system IDs.\n"
            "\t--comp=A,B   Count only the packets that have one of these component IDs.\n"
            "\t--msg=A,B    Count only the packets that have one of these message IDs or names.\n"
            "\t--time=A,B   Count only the packets from A seconds to B seconds after the first timestamp of the file.\n"
            "\t--offset=A,B Examine only the bytes from offset A to offset B, in hexadecimal.\n"
            "\t             Bytes that are not in a record are always counted.\n"
            "\t--extract=F  Write the bytes in --offset to the file F. Use with one input file only.\n"
            "\t--no-colour  Do not use colour in the report.\n"
            "\t--tui        Show the files in the interactive screen. Input files are optional.\n"
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

    TempArena Scratch = GetScratch(NULL, 0);
    Str8List Label = {};
    TLOGFilter* Filter = &Params.Filter;

    Filter->Offsets = Rng((u64) 0, U64_MAX);
    Filter->Times = Rng((u64) 0, U64_MAX);
    FUMEFilterPushIDs(
        Filter->SysIDs, 
        256, 
        CommandLineStrings(CLI, "sys"_s8)
    );
    FUMEFilterPushIDs(
        Filter->CompIDs, 
        256, 
        CommandLineStrings(CLI, "comp"_s8)
    );
    FUMEFilterPushIDs(
        Filter->MsgIDs, 
        MAVLINK_MSG_SLOTS_COUNT, 
        CommandLineStrings(CLI, "msg"_s8)
    );
    Params.ExtractPath = CommandLineString(CLI, "extract"_s8);

    if (CommandLineHasArgument(CLI, "offset"_s8)) {
        Str8List Strings = CommandLineStrings(CLI, "offset"_s8);

        Filter->Offsets.Min = U64FromStr(Strings.Head->String, 16);

        if (Strings.Tail != Strings.Head)
            Filter->Offsets.Max = U64FromStr(Strings.Tail->String, 16);
    }

    if (CommandLineHasArgument(CLI, "time"_s8)) {
        Str8List Strings = CommandLineStrings(CLI, "time"_s8);

        Params.TimeRange = Rng(
            (u64) (F64FromStr(Strings.Head->String) * MILLION(1)), 
            U64_MAX
        );

        if (Strings.Tail != Strings.Head) {
            Params.TimeRange.Max = (u64) (
                F64FromStr(Strings.Tail->String) * MILLION(1)
            );
        }
    }

    if (Params.ExtractPath.Size && CLI->Inputs.Count > 1) {
        PrintErr("[ERROR] :: --extract works with one input file only\n");
        ReleaseScratch(Scratch);

        return;
    }

    {
        Str8JoinPart Join = { ""_s8, ", "_s8, ""_s8 };
        Str8 Flags[] = { "sys"_s8, "comp"_s8, "msg"_s8, "time"_s8, "offset"_s8 };
        Str8 Names[] = { "system "_s8, "component "_s8, "message "_s8, "seconds "_s8, "offset 0x"_s8 };

        for (u32 Index = 0; Index < ARRAY_COUNT(Flags); ++Index) {
            Str8List Strings = CommandLineStrings(CLI, Flags[Index]);

            if (Strings.Count) {
                ListPushFmt(
                    Scratch.MemPool,
                    &Label,
                    "%S%S    ",
                    PRINT_STR(Names[Index]),
                    PRINT_STR(StrListJoin(Scratch.MemPool, &Strings, &Join))
                );
            }
        }

        Filter->Label = StrListJoin(Scratch.MemPool, &Label, NULL);
    }

    Lanes = CLAMP(1, Lanes, FUME_MAX_LANE_COUNT);
    MAVLinkInit();
    TLOGStylesInit(!CommandLineHasFlag(CLI, "no-colour"_s8));

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
