#include "tlog.h"
#include "nc_memory.h"
#include "nc_arena.h"
#include "nc_string.h"

Str8 TLOG_STYLES[TLOG_STYLE_COUNT] = {};

void
TLOGStylesInit(b32 UseColour)
{
    TLOG_STYLES[TLOG_STYLE_RESET] = UseColour 
        ? "\x1b[0m"_s8 
        : ""_s8;
    TLOG_STYLES[TLOG_STYLE_HEADING] = UseColour 
        ? "\x1b[1;36m"_s8 
        : ""_s8;
    TLOG_STYLES[TLOG_STYLE_DIM] = UseColour 
        ? "\x1b[90m"_s8 
        : ""_s8;
    TLOG_STYLES[TLOG_STYLE_GOOD] = UseColour 
        ? "\x1b[32m"_s8 
        : ""_s8;
    TLOG_STYLES[TLOG_STYLE_BAD] = UseColour 
        ? "\x1b[1;31m"_s8 
        : ""_s8;
    TLOG_STYLES[TLOG_STYLE_WARN] = UseColour 
        ? "\x1b[33m"_s8 
        : ""_s8;
}

INTERNAL b32
TLOGTimeIsSane(u64 USecs)
{
    return (USecs - TLOG_TIME_MIN_USECS) <= (TLOG_TIME_MAX_USECS - TLOG_TIME_MIN_USECS);
}

INTERNAL b32
TLOGIsRecordHead(u8* Base, u64 Size, u64 Offset)
{
    b32 Result = FALSE;

    if (Offset + TLOG_RECORD_SIZE_MIN <= Size) {
        u8* Ptr = Base + Offset + TLOG_TIMESTAMP_SIZE;

        Result = (
            MAVLINK_IS_MAGIC(*Ptr) &&
            (Offset + TLOG_TIMESTAMP_SIZE + MAVLinkFrameSizeFromPtr(Ptr) <= Size) &&
            TLOGTimeIsSane(SwapByteOrder(*(u64*) (Base + Offset)))
        );
    }

    return Result;
}

u64
TLOGFindRecord(u8* Base, u64 Size, u64 Offset, u64 Limit)
{
    u64 Result = Limit;

    for ( ; Offset < Limit; ++Offset) {
        if (!TLOGIsRecordHead(Base, Size, Offset))
            continue;

        u64 End = (
            Offset +
            TLOG_TIMESTAMP_SIZE +
            MAVLinkFrameSizeFromPtr(Base + Offset + TLOG_TIMESTAMP_SIZE)
        );

        if (End + TLOG_RECORD_SIZE_MIN > Size) {
            Result = Offset;
            break;
        }

        if (!TLOGIsRecordHead(Base, Size, End))
            continue;

        u64 Time = SwapByteOrder(*(u64*) (Base + Offset));
        u64 NextTime = SwapByteOrder(*(u64*) (Base + End));

        if (NextTime + TLOG_TIME_SLACK_USECS >= Time) {
            Result = Offset;
            break;
        }
    }

    return Result;
}

INTERNAL TLOGRecord
TLOGRecordFromOffset(u8* Base, u64 Size, u64 Offset)
{
    TLOGRecord Result = {};
    u8* Ptr = Base + Offset + TLOG_TIMESTAMP_SIZE;

    Result.Offset = Offset;

    if (
        LIKELY(Offset + TLOG_RECORD_SIZE_MIN <= Size) &&
        LIKELY(MAVLINK_IS_MAGIC(*Ptr)) &&
        LIKELY(Offset + TLOG_TIMESTAMP_SIZE + MAVLinkFrameSizeFromPtr(Ptr) <= Size)
    ) {
        Result.Frame = MAVLinkFrameFromPtr(Ptr);
        Result.Kind = MAVLinkFrameKindFromFrame(&Result.Frame, &Result.Extra);
        Result.Size = TLOG_TIMESTAMP_SIZE + Result.Frame.Size;
        Result.Time = SwapByteOrder(*(u64*) (Base + Offset));
        Result.IsPacket = (
            LIKELY(Result.Kind == MAVLINK_FRAME_KIND_OKAY) ||
            (Offset + Result.Size + TLOG_RECORD_SIZE_MIN > Size) ||
            TLOGIsRecordHead(Base, Size, Offset + Result.Size)
        );
    }

    if (UNLIKELY(!Result.IsPacket))
        Result.Size = TLOGFindRecord(Base, Size, Offset + 1, Size) - Offset;

    return Result;
}

void
TLOGWalk(
    u8* Base,
    u64 Size,
    u64 Offset,
    u64 Limit,
    TLOGLane* Lane,
    u64 ProblemsMax
) {
    TLOGStats* Stats = &Lane->Stats;
    u64 PartSize = (Size / TLOG_PARTS_COUNT) + 1;
    u64 PrevTime = 0;

    while (Offset < Limit) {
        TLOGRecord Record = TLOGRecordFromOffset(Base, Size, Offset);
        u64 PartIndex = Offset / PartSize;
        TLOGPart* Part = &Stats->Parts[PartIndex];
        TLOGProblemKind ProblemKind = TLOG_PROBLEM_KIND_JUNK;
        b32 IsProblem = TRUE;

        if (LIKELY(Record.IsPacket)) {
            MAVLinkFrame Frame = Record.Frame;
            MAVLinkFrameKind Kind = Record.Kind;
            u16 Extra = Record.Extra;
            u64 Time = Record.Time;
            TLOGSource* Source = NULL;

            ++Stats->KindCounts[Kind];
            ++Part->KindCounts[Kind];
            ++Stats->VersionCounts[!!(Frame.Flag & MAVLINK_FRAME_FLAG_IS_VERSION_2)];
            Stats->SignedCount += !!(Frame.Flag & MAVLINK_FRAME_FLAG_IS_SIGNED);

            for (u64 SIndex = 0; SIndex < Stats->SourcesCount; ++SIndex) {
                TLOGSource* Candidate = &Stats->Sources[SIndex];

                if (
                    LIKELY(Candidate->SysID == Frame.SysID) &&
                    LIKELY(Candidate->CompID == Frame.CompID)
                ) {
                    Source = Candidate;
                    break;
                }
            }

            if (UNLIKELY(!Source) && Stats->SourcesCount < TLOG_SOURCES_MAX) {
                Source = &Stats->Sources[Stats->SourcesCount++];
                Source->SysID = Frame.SysID;
                Source->CompID = Frame.CompID;
            }

            if (LIKELY(!!Source)) {
                u64 PacketCount = (
                    Source->KindCounts[MAVLINK_FRAME_KIND_OKAY] +
                    Source->KindCounts[MAVLINK_FRAME_KIND_BAD_CRC] +
                    Source->KindCounts[MAVLINK_FRAME_KIND_UNKNOWN_MSG]
                );

                if (UNLIKELY(PacketCount < TLOG_SOURCE_PACKETS_MAX))
                    Source->HeadOffsets[PacketCount] = Offset;

                ++Source->KindCounts[Kind];
            } else {
                ++Stats->SourcesOverflowCount;
            }

            if (LIKELY(Frame.MsgSlot != MAVLINK_MSG_SLOT_NONE)) {
                ++Stats->MsgCounts[Frame.MsgSlot][Kind];
            } else if (Kind == MAVLINK_FRAME_KIND_UNKNOWN_MSG) {
                u64 UIndex = 0;

                for ( ; UIndex < Stats->UnknownMsgsCount; ++UIndex) {
                    if (Stats->UnknownMsgs[UIndex].MsgID == Frame.MsgID)
                        break;
                }

                if (UIndex < TLOG_UNKNOWN_MSGS_MAX) {
                    TLOGUnknownMsg* UnknownMsg = &Stats->UnknownMsgs[UIndex];

                    if (UIndex == Stats->UnknownMsgsCount) {
                        ++Stats->UnknownMsgsCount;
                        UnknownMsg->MsgID = Frame.MsgID;
                        UnknownMsg->Extra = Extra;
                    }

                    UnknownMsg->ExtraVaries |= (UnknownMsg->Extra != Extra);
                    ++UnknownMsg->Count;
                }
            }

            if (UNLIKELY(Kind == MAVLINK_FRAME_KIND_BAD_CRC)) {
                MAVLinkByteFix Fix = MAVLinkByteFixFromFrame(&Frame);

                ++Stats->FixCounts[Fix.Field];
                ProblemKind = TLOG_PROBLEM_KIND_BAD_CRC;
            } else {
                IsProblem = FALSE;
            }

            if (UNLIKELY(!TLOGTimeIsSane(Time))) {
                ++Stats->TimeInsaneCount;
            } else {
                if (UNLIKELY(!PrevTime)) {
                    Stats->HeadTime = Time;
                    Stats->HeadTimeKind = Kind;
                    Stats->TimeRange = Rng(Time, Time + 1);
                } else if (UNLIKELY(Time < PrevTime)) {
                    ++Stats->TimeBackwardsCount;
                } else if (UNLIKELY(Time - PrevTime > TLOG_TIME_GAP_USECS)) {
                    ++Stats->TimeGapsCount;
                    Stats->TimeGapMaxUSecs = MAX(Stats->TimeGapMaxUSecs, Time - PrevTime);
                    Stats->BadAfterTimeGapCount += (Kind == MAVLINK_FRAME_KIND_BAD_CRC);
                }

                if (UNLIKELY(!Part->HeadTime))
                    Part->HeadTime = Time;

                Stats->TimeRange.Min = MIN(Stats->TimeRange.Min, Time);
                Stats->TimeRange.Max = MAX(Stats->TimeRange.Max, Time);
                PrevTime = Time;
            }
        } else {
            Stats->JunkBytes += Record.Size;
            Part->JunkBytes += Record.Size;
            ++Stats->JunkRuns;
        }

        if (UNLIKELY(IsProblem) && Lane->ProblemsCount < ProblemsMax) {
            TLOGProblem* Problem = &Lane->Problems[Lane->ProblemsCount++];

            Problem->Offset = Offset;
            Problem->Size = Record.Size;
            Problem->Kind = ProblemKind;
        }

        Offset += Record.Size;
    }

    Stats->TailTime = PrevTime;
}

void
TLOGStatsMerge(TLOGStats* Dst, TLOGStats* Src)
{
    for (u32 Kind = 0; Kind < MAVLINK_FRAME_KIND_COUNT; ++Kind) {
        Dst->KindCounts[Kind] += Src->KindCounts[Kind];

        for (u32 Slot = 0; Slot < MAVLINK_MSG_COUNT; ++Slot)
            Dst->MsgCounts[Slot][Kind] += Src->MsgCounts[Slot][Kind];
    }

    for (u32 Field = 0; Field < MAVLINK_FIELD_COUNT; ++Field)
        Dst->FixCounts[Field] += Src->FixCounts[Field];

    Dst->VersionCounts[0] += Src->VersionCounts[0];
    Dst->VersionCounts[1] += Src->VersionCounts[1];
    Dst->SignedCount += Src->SignedCount;
    Dst->SourcesOverflowCount += Src->SourcesOverflowCount;

    for (u64 SIndex = 0; SIndex < Src->SourcesCount; ++SIndex) {
        TLOGSource* SrcSource = &Src->Sources[SIndex];
        u64 DIndex = 0;

        for ( ; DIndex < Dst->SourcesCount; ++DIndex) {
            if (
                Dst->Sources[DIndex].SysID == SrcSource->SysID &&
                Dst->Sources[DIndex].CompID == SrcSource->CompID
            ) {
                break;
            }
        }

        if (DIndex == TLOG_SOURCES_MAX) {
            for (u32 Kind = 0; Kind < MAVLINK_FRAME_KIND_COUNT; ++Kind)
                Dst->SourcesOverflowCount += SrcSource->KindCounts[Kind];

            continue;
        }

        TLOGSource* DstSource = &Dst->Sources[DIndex];
        u64 DstCount = 0;
        u64 SrcCount = 0;

        if (DIndex == Dst->SourcesCount) {
            ++Dst->SourcesCount;
            DstSource->SysID = SrcSource->SysID;
            DstSource->CompID = SrcSource->CompID;
        }

        for (u32 Kind = 0; Kind < MAVLINK_FRAME_KIND_COUNT; ++Kind) {
            DstCount += DstSource->KindCounts[Kind];
            SrcCount += SrcSource->KindCounts[Kind];
            DstSource->KindCounts[Kind] += SrcSource->KindCounts[Kind];
        }

        for (
            u64 PIndex = 0;
            (
                PIndex < MIN(SrcCount, TLOG_SOURCE_PACKETS_MAX) && 
                DstCount < TLOG_SOURCE_PACKETS_MAX
            );
            ++PIndex, ++DstCount
        ) {
            DstSource->HeadOffsets[DstCount] = SrcSource->HeadOffsets[PIndex];
        }
    }

    for (u32 PIndex = 0; PIndex < TLOG_PARTS_COUNT; ++PIndex) {
        TLOGPart* DstPart = &Dst->Parts[PIndex];
        TLOGPart* SrcPart = &Src->Parts[PIndex];

        for (u32 Kind = 0; Kind < MAVLINK_FRAME_KIND_COUNT; ++Kind)
            DstPart->KindCounts[Kind] += SrcPart->KindCounts[Kind];

        DstPart->JunkBytes += SrcPart->JunkBytes;

        if (!DstPart->HeadTime)
            DstPart->HeadTime = SrcPart->HeadTime;
    }

    for (u64 SIndex = 0; SIndex < Src->UnknownMsgsCount; ++SIndex) {
        TLOGUnknownMsg* SrcMsg = &Src->UnknownMsgs[SIndex];
        u64 DIndex = 0;

        for ( ; DIndex < Dst->UnknownMsgsCount; ++DIndex) {
            if (Dst->UnknownMsgs[DIndex].MsgID == SrcMsg->MsgID)
                break;
        }

        if (DIndex < TLOG_UNKNOWN_MSGS_MAX) {
            TLOGUnknownMsg* DstMsg = &Dst->UnknownMsgs[DIndex];

            if (DIndex == Dst->UnknownMsgsCount) {
                ++Dst->UnknownMsgsCount;
                DstMsg->MsgID = SrcMsg->MsgID;
                DstMsg->Extra = SrcMsg->Extra;
            }

            DstMsg->ExtraVaries |= (
                SrcMsg->ExtraVaries | 
                (DstMsg->Extra != SrcMsg->Extra)
            );
            DstMsg->Count += SrcMsg->Count;
        }
    }

    Dst->JunkBytes += Src->JunkBytes;
    Dst->JunkRuns += Src->JunkRuns;
    Dst->TimeInsaneCount += Src->TimeInsaneCount;
    Dst->TimeBackwardsCount += Src->TimeBackwardsCount;
    Dst->TimeGapsCount += Src->TimeGapsCount;
    Dst->TimeGapMaxUSecs = MAX(Dst->TimeGapMaxUSecs, Src->TimeGapMaxUSecs);
    Dst->BadAfterTimeGapCount += Src->BadAfterTimeGapCount;

    if (Src->HeadTime) {
        if (!Dst->HeadTime) {
            Dst->HeadTime = Src->HeadTime;
            Dst->HeadTimeKind = Src->HeadTimeKind;
            Dst->TimeRange = Src->TimeRange;
        } else if (Src->HeadTime < Dst->TailTime) {
            ++Dst->TimeBackwardsCount;
        } else if (Src->HeadTime - Dst->TailTime > TLOG_TIME_GAP_USECS) {
            ++Dst->TimeGapsCount;
            Dst->TimeGapMaxUSecs = MAX(Dst->TimeGapMaxUSecs, Src->HeadTime - Dst->TailTime);
            Dst->BadAfterTimeGapCount += (Src->HeadTimeKind == MAVLINK_FRAME_KIND_BAD_CRC);
        }

        Dst->TimeRange.Min = MIN(Dst->TimeRange.Min, Src->TimeRange.Min);
        Dst->TimeRange.Max = MAX(Dst->TimeRange.Max, Src->TimeRange.Max);
        Dst->TailTime = Src->TailTime;
    }
}

Str8
TLOGStrFromUSecs(Arena* MemPool, u64 USecs)
{
    Str8 Result = {};

    if (!TLOGTimeIsSane(USecs)) {
        Result = ArenaPushStrFmt(MemPool, "0x%llX", USecs);
    } else {
        u64 Secs = USecs / MILLION(1);
        u64 Days = Secs / 86400;
        u64 SecsOfDay = Secs - (Days * 86400);
        u64 Shifted = Days + 719468;
        u64 Era = Shifted / 146097;
        u64 DayOfEra = Shifted - (Era * 146097);
        u64 YearOfEra = (
            DayOfEra -
            (DayOfEra / 1460) +
            (DayOfEra / 36524) -
            (DayOfEra / 146096)
        ) / 365;
        u64 DayOfYear = DayOfEra - ((365 * YearOfEra) + (YearOfEra / 4) - (YearOfEra / 100));
        u64 MonthShifted = ((5 * DayOfYear) + 2) / 153;
        u64 Day = DayOfYear - (((153 * MonthShifted) + 2) / 5) + 1;
        u64 Month = (MonthShifted < 10) ? (MonthShifted + 3) : (MonthShifted - 9);
        u64 Year = YearOfEra + (Era * 400) + (Month <= 2);

        Result = ArenaPushStrFmt(
            MemPool,
            "%04d-%02d-%02d %02d:%02d:%02d.%06d",
            (i32) Year,
            (i32) Month,
            (i32) Day,
            (i32) (SecsOfDay / 3600),
            (i32) ((SecsOfDay / 60) % 60),
            (i32) (SecsOfDay % 60),
            (i32) (USecs % MILLION(1))
        );
    }

    return Result;
}

internal void
TLOGDumpLineFmt(u8* Line, char* Fmt, ...)
{
    va_list Args;

    va_start(Args, Fmt);

    i32 Count = VSNPrintf((char*) Line, TLOG_DUMP_LINE_SIZE, Fmt, Args);

    va_end(Args);
    Count = CLAMP(0, Count, TLOG_DUMP_LINE_SIZE - 1);
    MemSet(Line + Count, ' ', TLOG_DUMP_LINE_SIZE - 1 - Count);
    Line[TLOG_DUMP_LINE_SIZE - 1] = '\n';
}

void
TLOGDumpHeader(u8* Line)
{
    TLOGDumpLineFmt(
        Line,
        "%-12s %-26s %7s %6s %9s %8s %10s  %-40s %6s  %s",
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
}

void
TLOGDump(u8* Base, u64 Size, u64 Offset, u64 Limit, u8* Lines)
{
    TempArena Scratch = GetScratch(NULL, 0);

    for (u8* Line = Lines; Offset < Limit; Line += TLOG_DUMP_LINE_SIZE) {
        TempArena Temp = ArenaBeginTemp(Scratch.MemPool);
        TLOGRecord Record = TLOGRecordFromOffset(Base, Size, Offset);
        Str8 OffsetStr = ArenaPushStrFmt(Temp.MemPool, "0x%llX", Offset);

        if (LIKELY(Record.IsPacket)) {
            MAVLinkFrame* Frame = &Record.Frame;
            Str8 Time = TLOGStrFromUSecs(Temp.MemPool, Record.Time);
            Str8 Name = (Frame->MsgSlot != MAVLINK_MSG_SLOT_NONE)
                ? MAVLINK_MSG_NAMES[Frame->MsgSlot]
                : "Not in table"_s8;
            char* Checksum = "Correct";

            if (Record.Kind == MAVLINK_FRAME_KIND_BAD_CRC)
                Checksum = "Incorrect";
            else if (Record.Kind == MAVLINK_FRAME_KIND_UNKNOWN_MSG)
                Checksum = "Not tested";

            TLOGDumpLineFmt(
                Line,
                "%-12S %-26S %7d %6d %9d %8d %10d  %-40.40S %6d  %s%s",
                PRINT_STR(OffsetStr),
                PRINT_STR(Time),
                (Frame->Flag & MAVLINK_FRAME_FLAG_IS_VERSION_2) ? 2 : 1,
                (i32) Frame->SysID,
                (i32) Frame->CompID,
                (i32) Frame->Seq,
                (i32) Frame->MsgID,
                PRINT_STR(Name),
                (i32) Frame->PayloadSize,
                Checksum,
                (Frame->Flag & MAVLINK_FRAME_FLAG_IS_SIGNED) ? ", signature" : ""
            );
        } else {
            TLOGDumpLineFmt(
                Line,
                "%-12S %llu bytes that are not in a record",
                PRINT_STR(OffsetStr),
                Record.Size
            );
        }

        Offset += Record.Size;
        ArenaEndTemp(Temp);
    }

    ReleaseScratch(Scratch);
}

internal Str8
TLOGStrFromCount(Arena* MemPool, u64 Count)
{
    u8 Buffer[32];
    u64 Index = sizeof(Buffer);
    u64 Digits = 0;

    do {
        if (Digits && !(Digits % 3))
            Buffer[--Index] = ',';

        Buffer[--Index] = (u8) ('0' + (Count % 10));
        Count /= 10;
        ++Digits;
    } while (Count);

    Str8 Result = {
        Buffer + Index,
        sizeof(Buffer) - Index
    };

    return ArenaPushStrCpy(MemPool, Result);
}

void
TLOGReportRule(Arena* MemPool, OUT Str8List* Strings)
{
    ListPush(MemPool, Strings, TLOG_STYLES[TLOG_STYLE_DIM]);

    for (u32 Index = 0; Index < TLOG_RULE_SIZE; ++Index)
        ListPush(MemPool, Strings, "\xE2\x94\x80"_s8);

    ListPush(MemPool, Strings, TLOG_STYLES[TLOG_STYLE_RESET]);
    ListPush(MemPool, Strings, "\n"_s8);
}

internal void
TLOGReportHeading(Arena* MemPool, OUT Str8List* Strings, char* Heading)
{
    ListPushFmt(
        MemPool,
        Strings,
        "\n  %S%s%S\n",
        PRINT_STR(TLOG_STYLES[TLOG_STYLE_HEADING]),
        Heading,
        PRINT_STR(TLOG_STYLES[TLOG_STYLE_RESET])
    );
}

internal void
TLOGReportCount(
    Arena* MemPool,
    OUT Str8List* Strings,
    Str8 Label,
    u64 Count,
    u64 Total,
    TLOGStyle Style
) {
    u64 Hundredths = Total ? ((Count * 10000) / Total) : 0;
    Str8 CountString = TLOGStrFromCount(MemPool, Count);

    ListPushFmt(
        MemPool,
        Strings,
        "    %S%-40S %14S%S",
        PRINT_STR(TLOG_STYLES[Count ? Style : TLOG_STYLE_DIM]),
        PRINT_STR(Label),
        PRINT_STR(CountString),
        PRINT_STR(TLOG_STYLES[TLOG_STYLE_RESET])
    );

    if (Total) {
        ListPushFmt(
            MemPool,
            Strings,
            "   %S%3lld.%02lld %%%S",
            PRINT_STR(TLOG_STYLES[TLOG_STYLE_DIM]),
            (i64) (Hundredths / 100),
            (i64) (Hundredths % 100),
            PRINT_STR(TLOG_STYLES[TLOG_STYLE_RESET])
        );
    }

    ListPush(MemPool, Strings, "\n"_s8);
}

internal void
TLOGReportCell(
    Arena* MemPool, 
    OUT Str8List* Strings, 
    u64 Count, 
    TLOGStyle Style
) {
    Str8 CountString = TLOGStrFromCount(MemPool, Count);

    ListPushFmt(
        MemPool,
        Strings,
        " %S%14S%S",
        PRINT_STR(TLOG_STYLES[Count ? Style : TLOG_STYLE_DIM]),
        PRINT_STR(CountString),
        PRINT_STR(TLOG_STYLES[TLOG_STYLE_RESET])
    );
}

internal void
TLOGReportPacketHeader(Arena* MemPool, OUT Str8List* Strings)
{
    ListPushFmt(
        MemPool,
        Strings,
        "    %S%-12s %-26s %6s %9s %8s  %-46s %6s  %s%S\n",
        PRINT_STR(TLOG_STYLES[TLOG_STYLE_DIM]),
        "OFFSET",
        "TIMESTAMP (UTC)",
        "SYSTEM",
        "COMPONENT",
        "SEQUENCE",
        "MESSAGE",
        "LENGTH",
        "CHECKSUM",
        PRINT_STR(TLOG_STYLES[TLOG_STYLE_RESET])
    );
}

internal void
TLOGReportPacket(Arena* MemPool, OUT Str8List* Strings, u8* Base, u64 Offset)
{
    MAVLinkFrame Frame = MAVLinkFrameFromPtr(Base + Offset + TLOG_TIMESTAMP_SIZE);
    u16 Extra = 0;
    MAVLinkFrameKind Kind = MAVLinkFrameKindFromFrame(&Frame, &Extra);
    Str8 OffsetStr = ArenaPushStrFmt(MemPool, "0x%llX", Offset);
    Str8 Time = TLOGStrFromUSecs(MemPool, SwapByteOrder(*(u64*) (Base + Offset)));
    Str8 Msg = (Frame.MsgSlot != MAVLINK_MSG_SLOT_NONE)
        ? ArenaPushStrFmt(MemPool, "%S (%u)", PRINT_STR(MAVLINK_MSG_NAMES[Frame.MsgSlot]), Frame.MsgID)
        : ArenaPushStrFmt(MemPool, "Not in table (%u)", Frame.MsgID);

    ListPushFmt(
        MemPool,
        Strings,
        "    %-12S %-26S %6d %9d %8d  %-46S %6d  ",
        PRINT_STR(OffsetStr),
        PRINT_STR(Time),
        (i32) Frame.SysID,
        (i32) Frame.CompID,
        (i32) Frame.Seq,
        PRINT_STR(Msg),
        (i32) Frame.PayloadSize
    );

    if (Kind == MAVLINK_FRAME_KIND_OKAY) {
        ListPushFmt(
            MemPool,
            Strings,
            "%SCorrect%S\n",
            PRINT_STR(TLOG_STYLES[TLOG_STYLE_GOOD]),
            PRINT_STR(TLOG_STYLES[TLOG_STYLE_RESET])
        );
    } else if (Kind == MAVLINK_FRAME_KIND_UNKNOWN_MSG) {
        ListPushFmt(
            MemPool,
            Strings,
            "%SNot tested.%S Correct if CRC_EXTRA is %u.\n",
            PRINT_STR(TLOG_STYLES[TLOG_STYLE_WARN]),
            PRINT_STR(TLOG_STYLES[TLOG_STYLE_RESET]),
            (u32) Extra
        );
    } else {
        MAVLinkByteFix Fix = MAVLinkByteFixFromFrame(&Frame);

        ListPushFmt(
            MemPool,
            Strings,
            "%SIncorrect%S",
            PRINT_STR(TLOG_STYLES[TLOG_STYLE_BAD]),
            PRINT_STR(TLOG_STYLES[TLOG_STYLE_RESET])
        );

        if (Fix.Field == MAVLINK_FIELD_PAYLOAD)
            ListPush(MemPool, Strings, ". One payload byte is incorrect."_s8);
        else if (Fix.Field != MAVLINK_FIELD_NONE)
            ListPushFmt(
                MemPool,
                Strings,
                ". Correct if %S is %u.",
                PRINT_STR(MAVLINK_FIELD_NAMES[Fix.Field]),
                (u32) Fix.Original
            );

        ListPush(MemPool, Strings, "\n"_s8);
    }
}

internal void
TLOGReportParts(Arena* MemPool, OUT Str8List* Strings, TLOGStats* Stats)
{
    TLOGReportHeading(MemPool, Strings, "THE FILE IN PARTS OF EQUAL SIZE");
    ListPushFmt(
        MemPool,
        Strings,
        "    %S%4s  %-26s %14s %14s %14s %21s%S\n",
        PRINT_STR(TLOG_STYLES[TLOG_STYLE_DIM]),
        "PART",
        "FIRST TIMESTAMP (UTC)",
        "CORRECT",
        "INCORRECT",
        "NOT TESTED",
        "BYTES NOT IN A RECORD",
        PRINT_STR(TLOG_STYLES[TLOG_STYLE_RESET])
    );

    for (u32 PIndex = 0; PIndex < TLOG_PARTS_COUNT; ++PIndex) {
        TLOGPart* Part = &Stats->Parts[PIndex];
        Str8 HeadTime = Part->HeadTime
            ? TLOGStrFromUSecs(MemPool, Part->HeadTime)
            : "No timestamp"_s8;

        ListPushFmt(MemPool, Strings, "    %4d  %-26S", (i32) (PIndex + 1), PRINT_STR(HeadTime));
        TLOGReportCell(MemPool, Strings, Part->KindCounts[MAVLINK_FRAME_KIND_OKAY], TLOG_STYLE_RESET);
        TLOGReportCell(MemPool, Strings, Part->KindCounts[MAVLINK_FRAME_KIND_BAD_CRC], TLOG_STYLE_BAD);
        TLOGReportCell(MemPool, Strings, Part->KindCounts[MAVLINK_FRAME_KIND_UNKNOWN_MSG], TLOG_STYLE_WARN);
        ListPush(MemPool, Strings, "       "_s8);
        TLOGReportCell(MemPool, Strings, Part->JunkBytes, TLOG_STYLE_BAD);
        ListPush(MemPool, Strings, "\n"_s8);
    }
}

void
TLOGReportFileRowHeader(Arena* MemPool, OUT Str8List* Strings)
{
    TLOGReportHeading(MemPool, Strings, "ALL FILES");
    ListPushFmt(
        MemPool,
        Strings,
        "    %S%-32s %14s %14s %8s %14s %21s  %s%S\n",
        PRINT_STR(TLOG_STYLES[TLOG_STYLE_DIM]),
        "FILE",
        "PACKETS",
        "INCORRECT",
        "%",
        "NOT TESTED",
        "BYTES NOT IN A RECORD",
        "SYSTEM IDS THAT HAVE A CORRECT PACKET",
        PRINT_STR(TLOG_STYLES[TLOG_STYLE_RESET])
    );
}

void
TLOGReportFileRow(
    Arena* MemPool, 
    OUT Str8List* Strings, 
    Str8 Path, 
    TLOGStats* Stats
) {
    u64 Bad = Stats->KindCounts[MAVLINK_FRAME_KIND_BAD_CRC];
    u64 Total = (
        Stats->KindCounts[MAVLINK_FRAME_KIND_OKAY] +
        Stats->KindCounts[MAVLINK_FRAME_KIND_BAD_CRC] +
        Stats->KindCounts[MAVLINK_FRAME_KIND_UNKNOWN_MSG]
    );
    u64 Hundredths = Total ? ((Bad * 10000) / Total) : 0;

    ListPushFmt(
        MemPool, 
        Strings, 
        "    %-32.32S", 
        PRINT_STR(StrSkipLastSlash(Path))
    );
    TLOGReportCell(MemPool, Strings, Total, TLOG_STYLE_RESET);
    TLOGReportCell(MemPool, Strings, Bad, TLOG_STYLE_BAD);
    ListPushFmt(
        MemPool, 
        Strings, 
        "   %3lld.%02lld", 
        (i64) (Hundredths / 100), 
        (i64) (Hundredths % 100)
    );
    TLOGReportCell(
        MemPool,
        Strings,
        Stats->KindCounts[MAVLINK_FRAME_KIND_UNKNOWN_MSG],
        TLOG_STYLE_WARN
    );
    ListPush(MemPool, Strings, "       "_s8);
    TLOGReportCell(MemPool, Strings, Stats->JunkBytes, TLOG_STYLE_BAD);
    ListPush(MemPool, Strings, " "_s8);

    for (u32 SysID = 0; SysID < 256; ++SysID) {
        for (u64 SIndex = 0; SIndex < Stats->SourcesCount; ++SIndex) {
            TLOGSource* Source = &Stats->Sources[SIndex];

            if (
                Source->SysID == SysID && 
                Source->KindCounts[MAVLINK_FRAME_KIND_OKAY]
            ) {
                ListPushFmt(MemPool, Strings, " %u", SysID);
                break;
            }
        }
    }

    ListPush(MemPool, Strings, "\n"_s8);
}

void
TLOGReport(
    Arena* MemPool,
    OUT Str8List* Strings,
    Str8 Path,
    u64 FileIndex,
    u64 FilesCount,
    u8* Base,
    u64 Size,
    TLOGStats* Stats,
    TLOGLane* Lanes,
    u64 LanesCount,
    u64 ProblemsMax,
    b32 ShouldReportMsgs
) {
    u64 Okay = Stats->KindCounts[MAVLINK_FRAME_KIND_OKAY];
    u64 Bad = Stats->KindCounts[MAVLINK_FRAME_KIND_BAD_CRC];
    u64 Unknown = Stats->KindCounts[MAVLINK_FRAME_KIND_UNKNOWN_MSG];
    u64 Total = Okay + Bad + Unknown;
    Str8 SizeStr = TLOGStrFromCount(MemPool, Size);
    Str8 FileStr = ArenaPushStrFmt(
        MemPool, 
        "FILE %llu OF %llu", 
        FileIndex + 1, 
        FilesCount
    );

    ListPush(MemPool, Strings, "\n"_s8);
    TLOGReportRule(MemPool, Strings);
    ListPushFmt(
        MemPool,
        Strings,
        "  %S%-78S%S %S%18S%S\n",
        PRINT_STR(TLOG_STYLES[TLOG_STYLE_HEADING]),
        PRINT_STR(StrSkipLastSlash(Path)),
        PRINT_STR(TLOG_STYLES[TLOG_STYLE_RESET]),
        PRINT_STR(TLOG_STYLES[TLOG_STYLE_DIM]),
        PRINT_STR(FileStr),
        PRINT_STR(TLOG_STYLES[TLOG_STYLE_RESET])
    );
    TLOGReportRule(MemPool, Strings);
    ListPushFmt(
        MemPool, 
        Strings, 
        "\n    %-22s %S bytes\n", "Size", 
        PRINT_STR(SizeStr)
    );

    if (Stats->HeadTime) {
        Str8 MinTime = TLOGStrFromUSecs(MemPool, Stats->TimeRange.Min);
        Str8 MaxTime = TLOGStrFromUSecs(MemPool, Stats->TimeRange.Max);

        ListPushFmt(
            MemPool, 
            Strings, 
            "    %-22s %S UTC\n", "First timestamp", 
            PRINT_STR(MinTime)
        );
        ListPushFmt(
            MemPool, 
            Strings, 
            "    %-22s %S UTC\n", "Last timestamp", 
            PRINT_STR(MaxTime)
        );
    }

    TLOGReportHeading(MemPool, Strings, "PACKETS");
    TLOGReportCount(MemPool, Strings, "Total"_s8, Total, 0, TLOG_STYLE_RESET);
    TLOGReportCount(
        MemPool, 
        Strings, 
        "MAVLink 1"_s8, 
        Stats->VersionCounts[0], 
        Total, 
        TLOG_STYLE_RESET
    );
    TLOGReportCount(
        MemPool, 
        Strings, 
        "MAVLink 2"_s8, 
        Stats->VersionCounts[1], 
        Total, 
        TLOG_STYLE_RESET
    );
    TLOGReportCount(
        MemPool, 
        Strings, 
        "MAVLink 2 with a signature"_s8, 
        Stats->SignedCount, 
        Total, 
        TLOG_STYLE_RESET
    );
    TLOGReportHeading(MemPool, Strings, "CHECKSUM");
    TLOGReportCount(MemPool, Strings, "Correct"_s8, Okay, Total, TLOG_STYLE_GOOD);
    TLOGReportCount(MemPool, Strings, "Incorrect"_s8, Bad, Total, TLOG_STYLE_BAD);
    TLOGReportCount(
        MemPool, 
        Strings, 
        "Not tested (message ID not in table)"_s8, 
        Unknown, 
        Total, 
        TLOG_STYLE_WARN
    );
    TLOGReportHeading(MemPool, Strings, "BYTES THAT ARE NOT IN A RECORD");
    TLOGReportCount(
        MemPool, 
        Strings, 
        "Bytes"_s8, 
        Stats->JunkBytes, 
        0, 
        TLOG_STYLE_BAD
    );
    TLOGReportCount(
        MemPool, 
        Strings, 
        "Blocks"_s8, 
        Stats->JunkRuns, 
        0, 
        TLOG_STYLE_BAD
    );

    if (Stats->UnknownMsgsCount) {
        TLOGReportHeading(
            MemPool, 
            Strings, 
            "MESSAGE IDS THAT ARE NOT IN THE TABLE"
        );
        ListPush(
            MemPool,
            Strings,
            "    FUME cannot test the checksum of these packets.\n"
            "    FUME calculated each CRC_EXTRA value from the packets.\n"
            "    A message ID that has many packets with the same CRC_EXTRA value is a real message.\n"
            "    Add it to MAVLINK_MSG_XLIST with that value.\n"
            "    A message ID that has one or two packets is possibly not a real message. Do not add it.\n\n"_s8
        );
        ListPushFmt(
            MemPool,
            Strings,
            "    %S%10s %14s   %s%S\n",
            PRINT_STR(TLOG_STYLES[TLOG_STYLE_DIM]),
            "MESSAGE ID",
            "PACKETS",
            "CRC_EXTRA",
            PRINT_STR(TLOG_STYLES[TLOG_STYLE_RESET])
        );

        for (u64 UIndex = 0; UIndex < Stats->UnknownMsgsCount; ++UIndex) {
            TLOGUnknownMsg* UnknownMsg = &Stats->UnknownMsgs[UIndex];
            Str8 CountStr = TLOGStrFromCount(MemPool, UnknownMsg->Count);

            ListPushFmt(
                MemPool, 
                Strings, 
                "    %10d %14S   ", 
                (i32) UnknownMsg->MsgID, 
                PRINT_STR(CountStr)
            );

            if (UnknownMsg->ExtraVaries) {
                ListPush(
                    MemPool, 
                    Strings, 
                    "The packets do not agree. These packets are possibly corrupt.\n"_s8
                );
            } else if (UnknownMsg->Count == 1) {
                ListPushFmt(
                    MemPool, 
                    Strings, 
                    "%u (from one packet only, possibly corrupt)\n", 
                    (u32) UnknownMsg->Extra
                );
            } else {
                ListPushFmt(
                    MemPool, 
                    Strings, 
                    "%u (the same in all packets)\n", 
                    (u32) UnknownMsg->Extra
                );
            }
        }
    }

    TLOGReportHeading(MemPool, Strings, "SYSTEM ID AND COMPONENT ID");
    ListPushFmt(
        MemPool,
        Strings,
        "    %S%6s %9s %14s %14s %14s %S\n",
        PRINT_STR(TLOG_STYLES[TLOG_STYLE_DIM]),
        "SYSTEM",
        "COMPONENT",
        "CORRECT",
        "INCORRECT",
        "NOT TESTED",
        PRINT_STR(TLOG_STYLES[TLOG_STYLE_RESET])
    );

    b32 HaveRareSources = FALSE;
    u64 NoiseBadCount = 0;
    u64 NoiseUnknownCount = 0;

    for (u32 Pass = 0; Pass < 2; ++Pass) {
        if (Pass && (NoiseBadCount | NoiseUnknownCount)) {
            Str8 NoiseString = TLOGStrFromCount(MemPool, NoiseBadCount + NoiseUnknownCount);
            Str8 NoiseBadString = TLOGStrFromCount(MemPool, NoiseBadCount);
            Str8 NoiseUnknownString = TLOGStrFromCount(MemPool, NoiseUnknownCount);

            ListPushFmt(
                MemPool,
                Strings,
                "\n    %S more packets have a system ID and component ID that has no correct packet.\n"
                "    Incorrect checksum: %S. Not tested: %S. Use --dump to see these packets.\n",
                PRINT_STR(NoiseString),
                PRINT_STR(NoiseBadString),
                PRINT_STR(NoiseUnknownString)
            );
        }

        for (u32 SysID = 0; SysID < 256; ++SysID) {
            for (u64 SIndex = 0; SIndex < Stats->SourcesCount; ++SIndex) {
                TLOGSource* Source = &Stats->Sources[SIndex];
                u64 PacketCount = (
                    Source->KindCounts[MAVLINK_FRAME_KIND_OKAY] +
                    Source->KindCounts[MAVLINK_FRAME_KIND_BAD_CRC] +
                    Source->KindCounts[MAVLINK_FRAME_KIND_UNKNOWN_MSG]
                );

                if (Source->SysID != SysID)
                    continue;

                if (!Source->KindCounts[MAVLINK_FRAME_KIND_OKAY]) {
                    if (!Pass) {
                        NoiseBadCount += Source->KindCounts[MAVLINK_FRAME_KIND_BAD_CRC];
                        NoiseUnknownCount += Source->KindCounts[MAVLINK_FRAME_KIND_UNKNOWN_MSG];
                    }

                    continue;
                }

                if (!Pass) {
                    ListPushFmt(
                        MemPool, 
                        Strings, 
                        "    %6d %9d", 
                        (i32) Source->SysID, 
                        (i32) Source->CompID
                    );
                    TLOGReportCell(
                        MemPool, 
                        Strings, 
                        Source->KindCounts[MAVLINK_FRAME_KIND_OKAY], 
                        TLOG_STYLE_RESET
                    );
                    TLOGReportCell(
                        MemPool, 
                        Strings, 
                        Source->KindCounts[MAVLINK_FRAME_KIND_BAD_CRC], 
                        TLOG_STYLE_BAD
                    );
                    TLOGReportCell(
                        MemPool, 
                        Strings, 
                        Source->KindCounts[MAVLINK_FRAME_KIND_UNKNOWN_MSG], 
                        TLOG_STYLE_WARN
                    );
                    ListPush(MemPool, Strings, "\n"_s8);
                } else if (PacketCount <= TLOG_SOURCE_PACKETS_MAX) {
                    if (!HaveRareSources) {
                        ListPushFmt(
                            MemPool,
                            Strings,
                            "\n  %SALL PACKETS FROM EACH SYSTEM ID AND COMPONENT ID THAT HAS %u PACKETS OR LESS%S\n",
                            PRINT_STR(TLOG_STYLES[TLOG_STYLE_HEADING]),
                            (u32) TLOG_SOURCE_PACKETS_MAX,
                            PRINT_STR(TLOG_STYLES[TLOG_STYLE_RESET])
                        );
                        TLOGReportPacketHeader(MemPool, Strings);
                    }

                    HaveRareSources = TRUE;

                    for (u64 PIndex = 0; PIndex < PacketCount; ++PIndex) {
                        TLOGReportPacket(
                            MemPool, 
                            Strings, 
                            Base, 
                            Source->HeadOffsets[PIndex]
                        );
                    }
                }
            }
        }
    }

    if (Stats->SourcesOverflowCount) {
        ListPushFmt(
            MemPool,
            Strings,
            "    %llu packets are not in this table. The table is full (%u rows).\n",
            Stats->SourcesOverflowCount,
            (u32) TLOG_SOURCES_MAX
        );
    }

    if ((Bad || Stats->JunkRuns) && ProblemsMax) {
        u64 Remaining = ProblemsMax;
        Str8 ShownStr = TLOGStrFromCount(
            MemPool, 
            MIN(ProblemsMax, Bad + Stats->JunkRuns)
        );
        Str8 AllStr = TLOGStrFromCount(MemPool, Bad + Stats->JunkRuns);

        ListPushFmt(
            MemPool,
            Strings,
            "\n  %SINCORRECT CHECKSUMS AND BYTES THAT ARE NOT IN A RECORD (THE FIRST %S OF %S)%S\n",
            PRINT_STR(TLOG_STYLES[TLOG_STYLE_HEADING]),
            PRINT_STR(ShownStr),
            PRINT_STR(AllStr),
            PRINT_STR(TLOG_STYLES[TLOG_STYLE_RESET])
        );
        TLOGReportPacketHeader(MemPool, Strings);

        for (u64 LIndex = 0; LIndex < LanesCount && Remaining; ++LIndex) {
            TLOGLane* Lane = &Lanes[LIndex];

            for (
                u64 PIndex = 0; 
                PIndex < Lane->ProblemsCount && Remaining; 
                ++PIndex, --Remaining
            ) {
                TLOGProblem* Problem = &Lane->Problems[PIndex];

                if (Problem->Kind == TLOG_PROBLEM_KIND_JUNK) {
                    Str8 OffsetStr = ArenaPushStrFmt(
                        MemPool, 
                        "0x%llX", 
                        Problem->Offset
                    );

                    ListPushFmt(
                        MemPool,
                        Strings,
                        "    %-12S %S%llu bytes that are not in a record%S\n",
                        PRINT_STR(OffsetStr),
                        PRINT_STR(TLOG_STYLES[TLOG_STYLE_BAD]),
                        Problem->Size,
                        PRINT_STR(TLOG_STYLES[TLOG_STYLE_RESET])
                    );
                } else {
                    TLOGReportPacket(MemPool, Strings, Base, Problem->Offset);
                }
            }
        }
    }

    if (Bad) {
        TLOGReportHeading(MemPool, Strings, "CAUSE OF THE INCORRECT CHECKSUMS");

        for (u32 Field = 0; Field < MAVLINK_FIELD_COUNT; ++Field) {
            Str8 Label = (Field == MAVLINK_FIELD_NONE)
                ? "More than one byte is incorrect"_s8
                : ArenaPushStrFmt(
                    MemPool, 
                    "One byte is incorrect: %S", 
                    PRINT_STR(MAVLINK_FIELD_NAMES[Field])
                );

            if (Stats->FixCounts[Field]) {
                TLOGReportCount(
                    MemPool, 
                    Strings, 
                    Label, 
                    Stats->FixCounts[Field], 
                    Bad, 
                    TLOG_STYLE_RESET
                );
            }
        }
    }

    TLOGReportHeading(MemPool, Strings, "TIMESTAMPS");
    TLOGReportCount(
        MemPool, 
        Strings, 
        "Intervals longer than 1 s"_s8, 
        Stats->TimeGapsCount, 
        0, 
        TLOG_STYLE_RESET
    );
    TLOGReportCount(
        MemPool, 
        Strings, 
        "Incorrect packets after these intervals"_s8, 
        Stats->BadAfterTimeGapCount, 
        0, 
        TLOG_STYLE_WARN
    );
    TLOGReportCount(
        MemPool, 
        Strings, 
        "Longest interval, in seconds"_s8, 
        Stats->TimeGapMaxUSecs / MILLION(1), 
        0, 
        TLOG_STYLE_RESET
    );
    TLOGReportCount(
        MemPool, 
        Strings, 
        "Earlier than the previous timestamp"_s8, 
        Stats->TimeBackwardsCount, 
        0, 
        TLOG_STYLE_WARN
    );
    TLOGReportCount(
        MemPool, 
        Strings, 
        "Not between 2010 and 2050"_s8, 
        Stats->TimeInsaneCount, 
        0, 
        TLOG_STYLE_WARN
    );
    TLOGReportParts(MemPool, Strings, Stats);

    if (ShouldReportMsgs) {
        TLOGReportHeading(MemPool, Strings, "MESSAGES");
        ListPushFmt(
            MemPool,
            Strings,
            "    %S%10s  %-40s %14s %14s%S\n",
            PRINT_STR(TLOG_STYLES[TLOG_STYLE_DIM]),
            "MESSAGE ID",
            "NAME",
            "CORRECT",
            "INCORRECT",
            PRINT_STR(TLOG_STYLES[TLOG_STYLE_RESET])
        );

        for (u32 Slot = 0; Slot < MAVLINK_MSG_COUNT; ++Slot) {
            u64 MsgOkay = Stats->MsgCounts[Slot][MAVLINK_FRAME_KIND_OKAY];
            u64 MsgBad = Stats->MsgCounts[Slot][MAVLINK_FRAME_KIND_BAD_CRC];
            Str8 OkayStr = TLOGStrFromCount(MemPool, MsgOkay);
            Str8 BadStr = TLOGStrFromCount(MemPool, MsgBad);

            if (!(MsgOkay | MsgBad))
                continue;

            ListPushFmt(
                MemPool,
                Strings,
                "    %10d  %-40S %14S %S%14S%S%s\n",
                (i32) MAVLINK_MSG_IDS[Slot],
                PRINT_STR(MAVLINK_MSG_NAMES[Slot]),
                PRINT_STR(OkayStr),
                PRINT_STR(TLOG_STYLES[MsgBad ? TLOG_STYLE_BAD : TLOG_STYLE_DIM]),
                PRINT_STR(BadStr),
                PRINT_STR(TLOG_STYLES[TLOG_STYLE_RESET]),
                (!MsgOkay && MsgBad > 2) 
                    ? "   No packet is correct. Examine the CRC_EXTRA value of this message in MAVLINK_MSG_XLIST." 
                    : ""
            );
        }
    }
}
