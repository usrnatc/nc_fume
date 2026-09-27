#include "tlog.h"
#include "nc_memory.h"
#include "nc_arena.h"
#include "nc_string.h"
#include "nc_simd.h"

Str8 TLOG_STYLES[TLOG_STYLE_COUNT] = {};

global Str8 TLOG_CHECKSUM_NAMES[MAVLINK_FRAME_KIND_COUNT] = {
    Str8Lit("Correct"),
    Str8Lit("Incorrect"),
    Str8Lit("Not tested")
};

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
TLOGFindMagic(u8 *Base, u64 Offset, u64 Limit)
{
#if NC_SIMD_LEVEL >= 6
    v256 Magic256V1 = _mm256_set1_epi8((i8) MAVLINK_MAGIC_VERSION_1);
    v256 Magic256V2 = _mm256_set1_epi8((i8) MAVLINK_MAGIC_VERSION_2);

    for ( ; Offset + 32 <= Limit; Offset += 32) {
        v256 Bytes = _mm256_loadu_si256((v256*) (Base + Offset));
        u32 Mask = (u32) _mm256_movemask_epi8(
            _mm256_or_si256(
                _mm256_cmpeq_epi8(Bytes, Magic256V1),
                _mm256_cmpeq_epi8(Bytes, Magic256V2)
            )
        );

        if (Mask) {
            Offset += Ctz32(Mask);
            break;
        }
    }
#endif

#if NC_SIMD_LEVEL >= 3
    v128 Magic128V1 = _mm_set1_epi8((i8) MAVLINK_MAGIC_VERSION_1);
    v128 Magic128V2 = _mm_set1_epi8((i8) MAVLINK_MAGIC_VERSION_2);

    for ( ; Offset + 16 <= Limit; Offset += 16) {
        v128 Bytes = _mm_loadu_si128((v128*) (Base + Offset));
        u32 Mask = (u32) _mm_movemask_epi8(
            _mm_or_si128(
                _mm_cmpeq_epi8(Bytes, Magic128V1),
                _mm_cmpeq_epi8(Bytes, Magic128V2)
            )
        );

        if (Mask) {
            Offset += Ctz32(Mask);
            break;
        }
    }
#endif

    for ( ; Offset < Limit && !MAVLINK_IS_MAGIC(Base[Offset]); ++Offset);

    return MIN(Offset, Limit);
}

u64
TLOGFindRecord(u8* Base, u64 Size, u64 Offset, u64 Limit)
{
    u64 Result = Limit;
    u64 End = MIN(Limit + TLOG_TIMESTAMP_SIZE, Size);

    for (
        u64 Magic = TLOGFindMagic(Base, Offset + TLOG_TIMESTAMP_SIZE, End);
        Magic < End;
        Magic = TLOGFindMagic(Base, Magic + 1, End)
    ) {
        Offset = Magic - TLOG_TIMESTAMP_SIZE;

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

u64
TLOGFindHiddenPacket(u8 *Base, u64 Size, u64 Offset, u64 Limit)
{
    u64 Result = Limit;

    for (
        Offset = TLOGFindMagic(Base, Offset, Limit);
        Offset < Limit;
        Offset = TLOGFindMagic(Base, Offset + 1, Limit)
    ) {
        u8* Ptr = Base + Offset;

        if (
            (Offset + MAVLINK_FRAME_SIZE_MIN > Size) ||
            (Offset + MAVLinkFrameSizeFromPtr(Ptr) > Size)
        ) {
            continue;
        }

        MAVLinkFrame Frame = MAVLinkFrameFromPtr(Ptr);
        u16 Extra = 0;

        if (
            Frame.MsgSlot != MAVLINK_MSG_SLOT_NONE &&
            MAVLinkFrameKindFromFrame(&Frame, &Extra) == MAVLINK_FRAME_KIND_OKAY
        ) {
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
    Result.Kind = TLOG_RECORD_KIND_JUNK;

    if (
        LIKELY(Offset + TLOG_RECORD_SIZE_MIN <= Size) &&
        LIKELY(MAVLINK_IS_MAGIC(*Ptr)) &&
        LIKELY(Offset + TLOG_TIMESTAMP_SIZE + MAVLinkFrameSizeFromPtr(Ptr) <= Size)
    ) {
        Result.Frame = MAVLinkFrameFromPtr(Ptr);

        MAVLinkFrameKind FrameKind = MAVLinkFrameKindFromFrame(
            &Result.Frame, 
            &Result.Extra
        );

        Result.Size = TLOG_TIMESTAMP_SIZE + Result.Frame.Size;
        Result.Time = SwapByteOrder(*(u64*) (Base + Offset));

        if (
            LIKELY(FrameKind == MAVLINK_FRAME_KIND_OKAY) ||
            (Offset + Result.Size + TLOG_RECORD_SIZE_MIN > Size) ||
            TLOGIsRecordHead(Base, Size, Offset + Result.Size)
        ) {
            Result.Kind = FrameKind;
        }
    }

    if (UNLIKELY(Result.Kind == TLOG_RECORD_KIND_JUNK))
        Result.Size = TLOGFindRecord(Base, Size, Offset + 1, Size) - Offset;

    return Result;
}

INTERNAL b32
TLOGIsText(u8 *Ptr, u64 Size)
{
    b32 Result = !!Size;

    for (u64 Index = 0; Index < Size; ++Index) {
        u8 Char = Ptr[Index];
        r1u32 CharRange = Rng((u32) ' ', (u32) 0x7F);

        if (
            !InRange(CharRange, (u32) Char) &&
            Char != '\r' &&
            Char != '\n' &&
            Char != '\t'
        ) {
            Result = FALSE;
            break;
        }
    }

    return Result;
}

INTERNAL b32
TLOGPassesFilter(TLOGFilter* Filter, MAVLinkFrame* Frame, u64 Time)
{
    u32 MsgID = MIN(Frame->MsgID, MAVLINK_MSG_SLOTS_COUNT - 1);
    b32 Result = (
        ((Filter->SysIDs[Frame->SysID >> 6] >> (Frame->SysID & 63)) & 1) &&
        ((Filter->CompIDs[Frame->CompID >> 6] >> (Frame->CompID & 63)) & 1) &&
        ((Filter->MsgIDs[MsgID >> 6] >> (MsgID & 63)) & 1) &&
        InRange(Filter->Times, Time)
    );

    return Result;
}

INTERNAL u64
TLOGSourceIndexFromKey(TLOGStats *Stats, u16 Key)
{
    u64 Result = TLOG_SOURCES_MAX;

#if NC_SIMD_LEVEL >= 6
    v256 Wide = _mm256_set1_epi16((i16) Key);
    v256* Keys = (v256*) Stats->SourceKeys;
    u64 Live = (Stats->SourcesCount < TLOG_SOURCES_MAX)
        ? ((1ULL << Stats->SourcesCount) - 1)
        : U64_MAX;
    u64 Mask = 0;

    for (u32 Index = 0; Index < TLOG_SOURCES_MAX / 32; ++Index) {
        v256 Hits = _mm256_packs_epi16(
            _mm256_cmpeq_epi16(_mm256_loadu_si256(Keys + (Index << 1)), Wide),
            _mm256_cmpeq_epi16(_mm256_loadu_si256(Keys + (Index << 1) + 1), Wide)
        );

        Mask |= ((u64) (u32) _mm256_movemask_epi8(Hits)) << (Index << 4);
    }

    Mask &= Live;

    if (Mask)
        Result = Ctz64(Mask);
#elif NC_SIMD_LEVEL >= 3
    v128 Wide = _mm_set1_epi16((i16) Key);
    v128* Keys = (v128*) Stats->SourceKeys;
    u64 Live = (Stats->SourcesCount < TLOG_SOURCES_MAX)
        ? ((1ULL << Stats->SourcesCount) - 1)
        : U64_MAX;
    u64 Mask = 0;

    for (u32 Index = 0; Index < TLOG_SOURCES_MAX / 16; ++Index) {
        v128 Hits = _mm_packs_epi16(
            _mm_cmpeq_epi16(_mm_loadu_si128(Keys + (Index << 1)), Wide),
            _mm_cmpeq_epi16(_mm_loadu_si128(Keys + (Index << 1) + 1), Wide)
        );

        Mask |= ((u64) (u32) _mm_movemask_epi8(Hits)) << (Index << 4);
    }

    Mask &= Live;

    if (Mask)
        Result = Ctz64(Mask);
#else
    for (u64 Index = 0; Index < Stats->SourcesCount; ++Index) {
        if (Stats->SourceKeys[Index] == Key) {
            Result = Index;
            break;
        }
    }
#endif

    return Result;
}

void
TLOGSourcesSort(TLOGStats *Stats)
{
    for (u64 Index = 1; Index < Stats->SourcesCount; ++Index) {
        u16 Key = Stats->SourceKeys[Index];
        TLOGSource Source = Stats->Sources[Index];
        u64 Slot = Index;

        for ( ; Slot && Stats->SourceKeys[Slot - 1] > Key; --Slot) {
            Stats->SourceKeys[Slot] = Stats->SourceKeys[Slot - 1];
            Stats->Sources[Slot] = Stats->Sources[Slot - 1];
        }

        Stats->SourceKeys[Slot] = Key;
        Stats->Sources[Slot] = Source;
    }
}

void
TLOGWalk(
    u8* Base,
    u64 Size,
    u64 Offset,
    u64 Limit,
    TLOGLane* Lane,
    u64 ProblemsMax,
    TLOGFilter* Filter
) {
    TLOGStats* Stats = &Lane->Stats;
    TLOGSource* Source = NULL;
    u16 SourceKey = 0;
    u64 PartSize = (Size / TLOG_PARTS_COUNT) + 1;
    u64 PrevTime = 0;

    while (Offset < Limit) {
        TLOGRecord Record = TLOGRecordFromOffset(Base, Size, Offset);
        TLOGPart* Part = &Stats->Parts[Offset / PartSize];
        TLOGRecordKind Kind = Record.Kind;
        b32 IsProblem = (
            Kind == TLOG_RECORD_KIND_BAD_CRC || 
            Kind == TLOG_RECORD_KIND_JUNK
        );

        if (
            Kind != TLOG_RECORD_KIND_JUNK && 
            !TLOGPassesFilter(Filter, &Record.Frame, Record.Time)
        ) {
            Offset += Record.Size;
            continue;
        }

        ++Stats->KindCounts[Kind];
        ++Part->KindCounts[Kind];

        if (LIKELY(Kind != TLOG_RECORD_KIND_JUNK)) {
            MAVLinkFrame Frame = Record.Frame;
            u16 Extra = Record.Extra;
            u64 Time = Record.Time;
            u16 Key = (u16) ((Frame.SysID << 8) | Frame.CompID);

            ++Stats->VersionCounts[!!(Frame.Flag & MAVLINK_FRAME_FLAG_IS_VERSION_2)];
            Stats->SignedCount += !!(Frame.Flag & MAVLINK_FRAME_FLAG_IS_SIGNED);

            if (UNLIKELY(!Source || SourceKey != Key)) {
                u64 SIndex = TLOGSourceIndexFromKey(Stats, Key);

                Source = NULL;
                SourceKey = Key;

                if (
                    SIndex == TLOG_SOURCES_MAX && 
                    Stats->SourcesCount < TLOG_SOURCES_MAX
                ) {
                    SIndex = Stats->SourcesCount++;
                    Stats->SourceKeys[SIndex] = Key;
                    Stats->Sources[SIndex].SysID = Frame.SysID;
                    Stats->Sources[SIndex].CompID = Frame.CompID;
                }

                if (SIndex < TLOG_SOURCES_MAX)
                    Source = &Stats->Sources[SIndex];
            }

            if (LIKELY(!!Source)) {
                u64 PacketCount = TLOG_PACKET_COUNT(Source->KindCounts);

                if (UNLIKELY(PacketCount < TLOG_SOURCE_PACKETS_MAX))
                    Source->HeadOffsets[PacketCount] = Offset;

                if (
                    PacketCount &&
                    UNLIKELY(Frame.Seq == Source->TailSeq) &&
                    Frame.Size == Source->TailSize &&
                    Kind == TLOG_RECORD_KIND_OKAY &&
                    MemCmp(
                        Frame.Ptr, 
                        Base + Source->TailOffset + TLOG_TIMESTAMP_SIZE, 
                        Frame.Size
                    )
                ) {
                    ++Source->CopyCount;
                    ++Stats->CopyCount;
                }

                ++Source->KindCounts[Kind];
                Source->TailOffset = Offset;
                Source->TailSize = Frame.Size;
                Source->TailSeq = Frame.Seq;

                switch (
                    Kind == TLOG_RECORD_KIND_OKAY 
                        ? Frame.MsgSlot 
                        : (u16) MAVLINK_MSG_SLOT_NONE
                ) {
                    default: {} break;

                    case MAVLINK_MSG_SLOT_HEARTBEAT: {
                        u8 Type = (u8) MAVLinkPayloadFromFrame(
                            &Frame, 
                            MAVLINK_HEARTBEAT_TYPE, 
                            1
                        );
                        u8 Autopilot = (u8) MAVLinkPayloadFromFrame(
                            &Frame, 
                            MAVLINK_HEARTBEAT_AUTOPILOT, 
                            1
                        );

                        if (!Source->HeartbeatCount) {
                            Source->HeartbeatHeadType = Type;
                            Source->HeartbeatHeadAutopilot = Autopilot;
                        } else {
                            Source->HeartbeatChangeCount += (
                                Type != Source->HeartbeatTailType ||
                                Autopilot != Source->HeartbeatTailAutopilot
                            );
                        }

                        if (TLOGTimeIsSane(Time)) {
                            if (!Source->HeartbeatHeadTime)
                                Source->HeartbeatHeadTime = Time;

                            Source->HeartbeatTailTime = Time;
                        }

                        Source->HeartbeatTailType = Type;
                        Source->HeartbeatTailAutopilot = Autopilot;
                        ++Source->HeartbeatCount;
                    } break;

                    case MAVLINK_MSG_SLOT_SYS_STATUS: {
                        u16 ErrorsComm = (u16) MAVLinkPayloadFromFrame(
                            &Frame, 
                            MAVLINK_SYS_STATUS_ERRORS_COMM, 
                            2
                        );
                        u16 DropRate = (u16) MAVLinkPayloadFromFrame(
                            &Frame, 
                            MAVLINK_SYS_STATUS_DROP_RATE_COMM, 
                            2
                        );

                        if (!Source->SysStatusCount)
                            Source->ErrorsCommHead = ErrorsComm;

                        Source->ErrorsCommTail = ErrorsComm;
                        Source->DropRateMax = MAX(
                            Source->DropRateMax, 
                            DropRate
                        );
                        ++Source->SysStatusCount;
                    } break;

                    case MAVLINK_MSG_SLOT_SYSTEM_TIME: {
                        u64 UnixUSecs = MAVLinkPayloadFromFrame(
                            &Frame, 
                            MAVLINK_SYSTEM_TIME_UNIX_USEC, 
                            8
                        );

                        if (UnixUSecs && TLOGTimeIsSane(Time)) {
                            i64 ClockOffset = (i64) (Time - UnixUSecs);

                            if (!Source->SystemTimeCount) {
                                Source->ClockOffsetMin = ClockOffset;
                                Source->ClockOffsetMax = ClockOffset;
                            }

                            Source->ClockOffsetMin = MIN(
                                Source->ClockOffsetMin, 
                                ClockOffset
                            );
                            Source->ClockOffsetMax = MAX(
                                Source->ClockOffsetMax, 
                                ClockOffset
                            );
                            ++Source->SystemTimeCount;
                        }
                    } break;

                    case MAVLINK_MSG_SLOT_RADIO_STATUS: {
                        u8 RSSI = (u8) MAVLinkPayloadFromFrame(
                            &Frame, 
                            MAVLINK_RADIO_STATUS_RSSI, 
                            1
                        );
                        u8 Noise = (u8) MAVLinkPayloadFromFrame(
                            &Frame, 
                            MAVLINK_RADIO_STATUS_NOISE, 
                            1
                        );
                        u16 RxErrors = (u16) MAVLinkPayloadFromFrame(
                            &Frame, 
                            MAVLINK_RADIO_STATUS_RX_ERRORS, 
                            2
                        );
                        u16 Fixed = (u16) MAVLinkPayloadFromFrame(
                            &Frame, 
                            MAVLINK_RADIO_STATUS_FIXED, 
                            2
                        );

                        if (!Source->RadioStatusCount) {
                            Source->RSSIMin = RSSI;
                            Source->NoiseMax = Noise;
                            Source->RXErrorsHead = RxErrors;
                            Source->FixedHead = Fixed;
                        }

                        Source->RSSIMin = MIN(Source->RSSIMin, RSSI);
                        Source->NoiseMax = MAX(Source->NoiseMax, Noise);
                        Source->RXErrorsTail = RxErrors;
                        Source->FixedTail = Fixed;
                        ++Source->RadioStatusCount;
                    } break;
                }
            } else {
                ++Stats->SourcesOverflowCount;
            }

            if (LIKELY(Frame.MsgSlot != MAVLINK_MSG_SLOT_NONE)) {
                ++Stats->MsgCounts[Frame.MsgSlot][Kind];
                Stats->MsgBytes[Frame.MsgSlot] += Frame.Size;
            } else if (Kind == TLOG_RECORD_KIND_UNKNOWN_MSG) {
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

                    if (UnknownMsg->Extra != Extra)
                        UnknownMsg->Extra = MAVLINK_EXTRA_NONE;

                    ++UnknownMsg->Count;
                }
            }

            if (UNLIKELY(!TLOGTimeIsSane(Time))) {
                ++Stats->TimeInsaneCount;
            } else {
                if (UNLIKELY(!PrevTime)) {
                    Stats->HeadTime = Time;
                    Stats->HeadTimeKind = (MAVLinkFrameKind) Kind;
                    Stats->TimeRange = Rng(Time, Time + 1);
                } else if (UNLIKELY(Time < PrevTime)) {
                    ++Stats->TimeBackwardsCount;
                } else if (UNLIKELY(Time - PrevTime > TLOG_TIME_GAP_USECS)) {
                    ++Stats->TimeGapsCount;
                    Stats->TimeGapMaxUSecs = MAX(Stats->TimeGapMaxUSecs, Time - PrevTime);
                    Stats->BadAfterTimeGapCount += (Kind == TLOG_RECORD_KIND_BAD_CRC);
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
            Stats->JunkTextRuns += TLOGIsText(Base + Offset, Record.Size);
        }

        if (UNLIKELY(Kind != TLOG_RECORD_KIND_OKAY)) {
            u64 End = Offset + Record.Size;
            u64 Hidden = TLOGFindHiddenPacket(
                Base, 
                Size, 
                Offset + TLOG_TIMESTAMP_SIZE + 1, 
                End
            );
            u64 HiddenCount = 0;
            u64 Tail = End;
            MAVLinkFix Fix = {};

            Fix.Kind = MAVLINK_FIX_KIND_NO_FRAME;

            if (IsProblem && Record.Frame.Ptr) {
                u64 FixEnd = (Kind == TLOG_RECORD_KIND_JUNK) 
                    ? MIN(Hidden, End) 
                    : End;

                Fix = MAVLinkFixFromFrame(&Record.Frame, Base + FixEnd);
            }

            if (IsProblem) {
                ++Stats->FixCounts[Fix.Kind];
                Stats->FieldCounts[Fix.Field] += (Fix.Kind == MAVLINK_FIX_KIND_CHANGE);

                if (Lane->ProblemsCount < ProblemsMax) {
                    TLOGProblem* Problem = &Lane->Problems[Lane->ProblemsCount++];

                    Problem->Record = Record;
                    Problem->Fix = Fix;
                }
            }

            while (Hidden < End) {
                ++HiddenCount;
                Tail = Hidden + MAVLinkFrameSizeFromPtr(Base + Hidden);
                Hidden = TLOGFindHiddenPacket(Base, Size, Tail, End);
            }

            if (HiddenCount) {
                Stats->HiddenCount += HiddenCount;
                ++Stats->HidingCount;
                Stats->CutOffCount += (
                    Tail < End &&
                    MAVLINK_IS_MAGIC(Base[Tail])
                );

                if (Lane->HidingCount < ProblemsMax) {
                    TLOGProblem* Hiding = &Lane->Hiding[Lane->HidingCount++];

                    Hiding->Record = Record;
                    Hiding->Fix = Fix;
                }
            }
        }

        Offset += Record.Size;
    }

    Stats->TailTime = PrevTime;
}

void
TLOGStatsMerge(TLOGStats* Dst, TLOGStats* Src, u8* Base)
{
    for (u32 Kind = 0; Kind < TLOG_RECORD_KIND_COUNT; ++Kind)
        Dst->KindCounts[Kind] += Src->KindCounts[Kind];

    for (u32 Slot = 0; Slot < MAVLINK_MSG_COUNT; ++Slot) {
        Dst->MsgBytes[Slot] += Src->MsgBytes[Slot];

        for (u32 Kind = 0; Kind < MAVLINK_FRAME_KIND_COUNT; ++Kind)
            Dst->MsgCounts[Slot][Kind] += Src->MsgCounts[Slot][Kind];
    }

    for (u32 Kind = 0; Kind < MAVLINK_FIX_KIND_COUNT; ++Kind)
        Dst->FixCounts[Kind] += Src->FixCounts[Kind];

    for (u32 Field = 0; Field < MAVLINK_FIELD_COUNT; ++Field)
        Dst->FieldCounts[Field] += Src->FieldCounts[Field];

    Dst->VersionCounts[0] += Src->VersionCounts[0];
    Dst->VersionCounts[1] += Src->VersionCounts[1];
    Dst->SignedCount += Src->SignedCount;
    Dst->SourcesOverflowCount += Src->SourcesOverflowCount;

    for (u64 SIndex = 0; SIndex < Src->SourcesCount; ++SIndex) {
        TLOGSource* SrcSource = &Src->Sources[SIndex];
        u16 Key = Src->SourceKeys[SIndex];
        u64 DIndex = TLOGSourceIndexFromKey(Dst, Key);

        if (DIndex == TLOG_SOURCES_MAX) {
            if (Dst->SourcesCount == TLOG_SOURCES_MAX) {
                Dst->SourcesOverflowCount += TLOG_PACKET_COUNT(SrcSource->KindCounts);
                continue;
            }

            DIndex = Dst->SourcesCount++;
            Dst->SourceKeys[DIndex] = Key;
            Dst->Sources[DIndex].SysID = SrcSource->SysID;
            Dst->Sources[DIndex].CompID = SrcSource->CompID;
        }

        TLOGSource* DstSource = &Dst->Sources[DIndex];
        u64 DstCount = TLOG_PACKET_COUNT(DstSource->KindCounts);
        u64 SrcCount = TLOG_PACKET_COUNT(SrcSource->KindCounts);

        for (u32 Kind = 0; Kind < MAVLINK_FRAME_KIND_COUNT; ++Kind)
            DstSource->KindCounts[Kind] += SrcSource->KindCounts[Kind];

        if (SrcCount) {
            if (DstCount) {
                u8* DstPtr = Base + DstSource->TailOffset + TLOG_TIMESTAMP_SIZE;
                u8* SrcPtr = (
                    Base + 
                    SrcSource->HeadOffsets[0] + 
                    TLOG_TIMESTAMP_SIZE
                );
                MAVLinkFrame Head = MAVLinkFrameFromPtr(SrcPtr);
                u16 Extra = 0;

                if (
                    Head.Size == DstSource->TailSize &&
                    MemCmp(SrcPtr, DstPtr, Head.Size) &&
                    MAVLinkFrameKindFromFrame(&Head, &Extra) == MAVLINK_FRAME_KIND_OKAY
                ) {
                    ++DstSource->CopyCount;
                    ++Dst->CopyCount;
                }
            }

            DstSource->TailOffset = SrcSource->TailOffset;
            DstSource->TailSize = SrcSource->TailSize;
            DstSource->TailSeq = SrcSource->TailSeq;
        }

        DstSource->CopyCount += SrcSource->CopyCount;

        if (SrcSource->HeartbeatCount) {
            if (!DstSource->HeartbeatCount) {
                DstSource->HeartbeatHeadType = SrcSource->HeartbeatHeadType;
                DstSource->HeartbeatHeadAutopilot = SrcSource->HeartbeatHeadAutopilot;
            } else {
                DstSource->HeartbeatChangeCount += (
                    SrcSource->HeartbeatHeadType != DstSource->HeartbeatTailType ||
                    SrcSource->HeartbeatHeadAutopilot != DstSource->HeartbeatTailAutopilot
                );
            }

            if (!DstSource->HeartbeatHeadTime)
                DstSource->HeartbeatHeadTime = SrcSource->HeartbeatHeadTime;

            if (SrcSource->HeartbeatTailTime)
                DstSource->HeartbeatTailTime = SrcSource->HeartbeatTailTime;

            DstSource->HeartbeatTailType = SrcSource->HeartbeatTailType;
            DstSource->HeartbeatTailAutopilot = SrcSource->HeartbeatTailAutopilot;
            DstSource->HeartbeatChangeCount += SrcSource->HeartbeatChangeCount;
            DstSource->HeartbeatCount += SrcSource->HeartbeatCount;
        }

        if (SrcSource->SysStatusCount) {
            if (!DstSource->SysStatusCount)
                DstSource->ErrorsCommHead = SrcSource->ErrorsCommHead;

            DstSource->ErrorsCommTail = SrcSource->ErrorsCommTail;
            DstSource->DropRateMax = MAX(DstSource->DropRateMax, SrcSource->DropRateMax);
            DstSource->SysStatusCount += SrcSource->SysStatusCount;
        }

        if (SrcSource->SystemTimeCount) {
            if (!DstSource->SystemTimeCount) {
                DstSource->ClockOffsetMin = SrcSource->ClockOffsetMin;
                DstSource->ClockOffsetMax = SrcSource->ClockOffsetMax;
            }

            DstSource->ClockOffsetMin = MIN(DstSource->ClockOffsetMin, SrcSource->ClockOffsetMin);
            DstSource->ClockOffsetMax = MAX(DstSource->ClockOffsetMax, SrcSource->ClockOffsetMax);
            DstSource->SystemTimeCount += SrcSource->SystemTimeCount;
        }

        if (SrcSource->RadioStatusCount) {
            if (!DstSource->RadioStatusCount) {
                DstSource->RSSIMin = SrcSource->RSSIMin;
                DstSource->NoiseMax = SrcSource->NoiseMax;
                DstSource->RXErrorsHead = SrcSource->RXErrorsHead;
                DstSource->FixedHead = SrcSource->FixedHead;
            }

            DstSource->RSSIMin = MIN(DstSource->RSSIMin, SrcSource->RSSIMin);
            DstSource->NoiseMax = MAX(DstSource->NoiseMax, SrcSource->NoiseMax);
            DstSource->RXErrorsTail = SrcSource->RXErrorsTail;
            DstSource->FixedTail = SrcSource->FixedTail;
            DstSource->RadioStatusCount += SrcSource->RadioStatusCount;
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

        for (u32 Kind = 0; Kind < TLOG_RECORD_KIND_COUNT; ++Kind)
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

            if (DstMsg->Extra != SrcMsg->Extra)
                DstMsg->Extra = MAVLINK_EXTRA_NONE;

            DstMsg->Count += SrcMsg->Count;
        }
    }

    Dst->JunkBytes += Src->JunkBytes;
    Dst->JunkTextRuns += Src->JunkTextRuns;
    Dst->CopyCount += Src->CopyCount;
    Dst->TimeInsaneCount += Src->TimeInsaneCount;
    Dst->TimeBackwardsCount += Src->TimeBackwardsCount;
    Dst->TimeGapsCount += Src->TimeGapsCount;
    Dst->TimeGapMaxUSecs = MAX(Dst->TimeGapMaxUSecs, Src->TimeGapMaxUSecs);
    Dst->BadAfterTimeGapCount += Src->BadAfterTimeGapCount;
    Dst->HiddenCount += Src->HiddenCount;
    Dst->HidingCount += Src->HidingCount;
    Dst->CutOffCount += Src->CutOffCount;

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

INTERNAL u8*
TLOGPutStr(u8* Out, Str8 String, u64 Width)
{
    u64 Count = MIN(String.Size, Width);

    MemCpy(Out, String.Str, Count);
    MemSet(Out + Count, ' ', Width - Count);

    return Out + Width;
}

INTERNAL u8*
TLOGPutU64(u8* Out, u64 Val, u64 Width)
{
    u8* End = Out + Width;
    u8* Ptr = End;

    do {
        *--Ptr = (u8) ('0' + (Val % 10));
        Val /= 10;
    } while (Val && Ptr > Out);

    MemSet(Out, ' ', (u64) (Ptr - Out));

    return End;
}

INTERNAL u8*
TLOGPutHex(u8* Out, u64 Val, u64 Width)
{
    u8 Buffer[18];
    u8* Ptr = Buffer + sizeof(Buffer);

    do {
        *--Ptr = (u8) ("0123456789ABCDEF"[Val & 15]);
        Val >>= 4;
    } while (Val);

    *--Ptr = 'x';
    *--Ptr = '0';

    Str8 Hex = {
        Ptr, 
        (u64) (Buffer + sizeof(Buffer) - Ptr)
    };

    return TLOGPutStr(Out, Hex, Width);
}

void
TLOGDump(
    u8* Base, 
    u64 Size, 
    u64 Offset, 
    u64 Limit, 
    Arena* Out, 
    TLOGFilter* Filter
) {
    TempArena Scratch = GetScratch(&Out, 1);
    u64 LastSecs = U64_MAX;
    u8 TimeStr[TLOG_DUMP_TIME_SIZE];

    while (Offset < Limit) {
        TLOGRecord Record = TLOGRecordFromOffset(Base, Size, Offset);
        u8* Line = ArenaPushArray(Out, u8, TLOG_DUMP_LINE_SIZE);
        u8* Ptr = Line;

        Offset += Record.Size;

        if (Record.Kind == TLOG_RECORD_KIND_JUNK) {
            Ptr = TLOGPutHex(Ptr, Record.Offset, 12);
            *Ptr++ = ' ';
            Ptr = TLOGPutU64(Ptr, Record.Size, 12);
            Ptr = TLOGPutStr(Ptr, " bytes that are not in a record"_s8, 31);
        } else if (TLOGPassesFilter(Filter, &Record.Frame, Record.Time)) {
            MAVLinkFrame* Frame = &Record.Frame;
            u64 Secs = Record.Time / MILLION(1);
            Str8 Name = (Frame->MsgSlot != MAVLINK_MSG_SLOT_NONE)
                ? MAVLINK_MSG_NAMES[Frame->MsgSlot]
                : "Not in table"_s8;

            if (UNLIKELY(Secs != LastSecs)) {
                TempArena Temp = ArenaBeginTemp(Scratch.MemPool);
                Str8 Time = TLOGStrFromUSecs(Temp.MemPool, Record.Time);

                TLOGPutStr(TimeStr, Time, TLOG_DUMP_TIME_SIZE);
                LastSecs = Secs;
                ArenaEndTemp(Temp);
            } else if (LIKELY(TLOGTimeIsSane(Record.Time))) {
                u64 USecs = Record.Time % MILLION(1);

                for (u32 Digit = 0; Digit < 6; ++Digit, USecs /= 10)
                    TimeStr[TLOG_DUMP_TIME_SIZE - 1 - Digit] = (u8) ('0' + (USecs % 10));
            }

            Ptr = TLOGPutHex(Ptr, Record.Offset, 12);
            *Ptr++ = ' ';
            Ptr = TLOGPutStr(Ptr, Str8{ TimeStr, TLOG_DUMP_TIME_SIZE }, 26);
            *Ptr++ = ' ';
            Ptr = TLOGPutU64(Ptr, 1 + !!(Frame->Flag & MAVLINK_FRAME_FLAG_IS_VERSION_2), 7);
            *Ptr++ = ' ';
            Ptr = TLOGPutU64(Ptr, Frame->SysID, 6);
            *Ptr++ = ' ';
            Ptr = TLOGPutU64(Ptr, Frame->CompID, 9);
            *Ptr++ = ' ';
            Ptr = TLOGPutU64(Ptr, Frame->Seq, 8);
            *Ptr++ = ' ';
            Ptr = TLOGPutU64(Ptr, Frame->MsgID, 10);
            *Ptr++ = ' ';
            *Ptr++ = ' ';
            Ptr = TLOGPutStr(Ptr, Name, 40);
            *Ptr++ = ' ';
            Ptr = TLOGPutU64(Ptr, Frame->PayloadSize, 6);
            *Ptr++ = ' ';
            *Ptr++ = ' ';
            Ptr = TLOGPutStr(
                Ptr, 
                TLOG_CHECKSUM_NAMES[Record.Kind], 
                TLOG_CHECKSUM_NAMES[Record.Kind].Size
            );

            if (Frame->Flag & MAVLINK_FRAME_FLAG_IS_SIGNED)
                Ptr = TLOGPutStr(Ptr, ", signature"_s8, 11);
        } else {
            ArenaPop(Out, TLOG_DUMP_LINE_SIZE);
            continue;
        }

        *Ptr++ = '\n';
        ArenaPop(Out, TLOG_DUMP_LINE_SIZE - (u64) (Ptr - Line));
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
TLOGReportFrame(
    Arena* MemPool, 
    OUT Str8List* Strings, 
    u64 Offset, 
    MAVLinkFrame* Frame, 
    MAVLinkFrameKind Kind,
    u16 Extra,
    Str8 Time,
    MAVLinkFix Fix
) {
    Str8 OffsetStr = ArenaPushStrFmt(MemPool, "0x%llX", Offset);
    Str8 Msg = (Frame->MsgSlot != MAVLINK_MSG_SLOT_NONE)
        ? ArenaPushStrFmt(
            MemPool, 
            "%S (%u)", 
            PRINT_STR(MAVLINK_MSG_NAMES[Frame->MsgSlot]), 
            Frame->MsgID
        ) 
        : ArenaPushStrFmt(
            MemPool, 
            "Not in table (%u)", 
            Frame->MsgID
        );

    ListPushFmt(
        MemPool,
        Strings,
        "    %-12S %-26S %6d %9d %8d  %-46S %6d  ",
        PRINT_STR(OffsetStr),
        PRINT_STR(Time),
        (i32) Frame->SysID,
        (i32) Frame->CompID,
        (i32) Frame->Seq,
        PRINT_STR(Msg),
        (i32) Frame->PayloadSize
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
        ListPushFmt(
            MemPool,
            Strings,
            "%SIncorrect%S",
            PRINT_STR(TLOG_STYLES[TLOG_STYLE_BAD]),
            PRINT_STR(TLOG_STYLES[TLOG_STYLE_RESET])
        );

        if (
            Fix.Kind == MAVLINK_FIX_KIND_CHANGE && 
            Fix.Field == MAVLINK_FIELD_PAYLOAD
        ) {
            ListPush(MemPool, Strings, ". One payload byte is incorrect."_s8);
        } else if (Fix.Kind == MAVLINK_FIX_KIND_CHANGE) {
            ListPushFmt(
                MemPool,
                Strings,
                ". Correct if %S is %u.",
                PRINT_STR(MAVLINK_FIELD_NAMES[Fix.Field]),
                (u32) Fix.Value
            );
        } else if (Fix.Kind == MAVLINK_FIX_KIND_LOSS) {
            ListPushFmt(
                MemPool,
                Strings,
                ". Correct if the byte 0x%02X is put in at index %u.",
                (u32) Fix.Value,
                (u32) Fix.Index
            );
        } else if (Fix.Kind == MAVLINK_FIX_KIND_ADDITION) {
            ListPushFmt(
                MemPool,
                Strings,
                ". Correct if the byte at index %u is removed.",
                (u32) Fix.Index
            );
        }

        ListPush(MemPool, Strings, "\n"_s8);
    }
}

internal void
TLOGReportRecord(
    Arena* MemPool, 
    OUT Str8List* Strings, 
    u8* Base, 
    u64 Size, 
    TLOGRecord* Record,
    MAVLinkFix Fix
) {
    Str8 Time = TLOGStrFromUSecs(MemPool, Record->Time);

    if (Record->Kind != TLOG_RECORD_KIND_JUNK) {
        TLOGReportFrame(
            MemPool,
            Strings,
            Record->Offset,
            &Record->Frame,
            (MAVLinkFrameKind) Record->Kind,
            Record->Extra,
            Time,
            Fix
        );

        return;
    }

    u8* Ptr = Base + Record->Offset + TLOG_TIMESTAMP_SIZE;
    Str8 Junk = { Base + Record->Offset, Record->Size };
    Str8 OffsetStr = ArenaPushStrFmt(MemPool, "0x%llX", Record->Offset);

    ListPushFmt(
        MemPool,
        Strings,
        "    %-12S %S%llu bytes that are not in a record%S",
        PRINT_STR(OffsetStr),
        PRINT_STR(TLOG_STYLES[TLOG_STYLE_BAD]),
        Record->Size,
        PRINT_STR(TLOG_STYLES[TLOG_STYLE_RESET])
    );

    if (Record->Frame.Ptr) {
        MAVLinkFrame Frame = Record->Frame;
        u16 Extra = 0;
        MAVLinkFrameKind Kind = MAVLinkFrameKindFromFrame(&Frame, &Extra);

        ListPush(MemPool, Strings, ". The first bytes are a packet:\n"_s8);
        TLOGReportFrame(
            MemPool,
            Strings,
            Record->Offset,
            &Frame,
            Kind,
            Extra,
            Time,
            Fix
        );
    } else if (
        Record->Size >= TLOG_RECORD_SIZE_MIN && 
        MAVLINK_IS_MAGIC(*Ptr)
    ) {
        ListPush(
            MemPool, 
            Strings, 
            ". The first bytes are a packet that the end of the file cuts off.\n"_s8
        );
    } else if (TLOGIsText(Junk.Str, Junk.Size)) {
        Str8 Text = ArenaPushStrCpy(
            MemPool, 
            StrPrefix(Junk, TLOG_JUNK_TEXT_SIZE)
        );

        for (u64 Index = 0; Index < Text.Size; ++Index) {
            Text.Str[Index] = (Text.Str[Index] < ' ') 
                ? ' ' 
                : Text.Str[Index];
        }

        ListPushFmt(
            MemPool, 
            Strings, 
            ". The bytes are text: \"%S\"\n", 
            PRINT_STR(Text)
        );
    } else {
        ListPush(MemPool, Strings, "\n"_s8);
    }
}

internal void
TLOGReportPacket(
    Arena* MemPool, 
    OUT Str8List* Strings, 
    u8* Base, 
    u64 Size, 
    u64 Offset
) {
    TLOGRecord Record = TLOGRecordFromOffset(Base, Size, Offset);
    MAVLinkFix Fix = {};

    if (Record.Kind == TLOG_RECORD_KIND_BAD_CRC)
        Fix = MAVLinkFixFromFrame(&Record.Frame, Base + Offset + Record.Size);

    TLOGReportRecord(MemPool, Strings, Base, Size, &Record, Fix);
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

    u32 LastSysID = 256;

    for (u64 SIndex = 0; SIndex < Stats->SourcesCount; ++SIndex) {
        TLOGSource* Source = &Stats->Sources[SIndex];

        if (
            Source->SysID != LastSysID && 
            Source->KindCounts[MAVLINK_FRAME_KIND_OKAY]
        ) {
            ListPushFmt(MemPool, Strings, " %u", (u32) Source->SysID);
            LastSysID = Source->SysID;
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
    b32 ShouldReportMsgs,
    TLOGFilter* Filter
) {
    u64 Okay = Stats->KindCounts[MAVLINK_FRAME_KIND_OKAY];
    u64 Bad = Stats->KindCounts[MAVLINK_FRAME_KIND_BAD_CRC];
    u64 Unknown = Stats->KindCounts[MAVLINK_FRAME_KIND_UNKNOWN_MSG];
    u64 JunkRuns = Stats->KindCounts[TLOG_RECORD_KIND_JUNK];
    u64 Total = TLOG_PACKET_COUNT(Stats->KindCounts);
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

    if (Filter->Label.Size) {
        ListPushFmt(
            MemPool,
            Strings,
            "    %-22s %S%S%S\n", "Filter",
            PRINT_STR(TLOG_STYLES[TLOG_STYLE_WARN]),
            PRINT_STR(Filter->Label),
            PRINT_STR(TLOG_STYLES[TLOG_STYLE_RESET])
        );
    }

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
    TLOGReportCount(
        MemPool, 
        Strings, 
        "Correct, a copy of the previous packet"_s8, 
        Stats->CopyCount, 
        Total, 
        TLOG_STYLE_WARN
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
        JunkRuns, 
        0, 
        TLOG_STYLE_BAD
    );
    TLOGReportCount(
        MemPool, 
        Strings, 
        "Blocks that are text"_s8, 
        Stats->JunkTextRuns, 
        JunkRuns, 
        TLOG_STYLE_WARN
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

            if (UnknownMsg->Extra == MAVLINK_EXTRA_NONE) {
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
        "    %S%6s %9s %14s %14s %14s %14s%S\n",
        PRINT_STR(TLOG_STYLES[TLOG_STYLE_DIM]),
        "SYSTEM",
        "COMPONENT",
        "CORRECT",
        "INCORRECT",
        "NOT TESTED",
        "COPIES",
        PRINT_STR(TLOG_STYLES[TLOG_STYLE_RESET])
    );

    b32 HaveRareSources = FALSE;
    u64 NoiseBadCount = 0;
    u64 NoiseUnknownCount = 0;

    for (u64 SIndex = 0; SIndex < Stats->SourcesCount; ++SIndex) {
        TLOGSource* Source = &Stats->Sources[SIndex];

        if (!Source->KindCounts[MAVLINK_FRAME_KIND_OKAY]) {
            NoiseBadCount += Source->KindCounts[MAVLINK_FRAME_KIND_BAD_CRC];
            NoiseUnknownCount += Source->KindCounts[MAVLINK_FRAME_KIND_UNKNOWN_MSG];
            continue;
        }

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
        TLOGReportCell(
            MemPool, 
            Strings, 
            Source->CopyCount, 
            TLOG_STYLE_WARN
        );
        ListPush(MemPool, Strings, "\n"_s8);
    }

    if (NoiseBadCount | NoiseUnknownCount) {
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

    for (u64 SIndex = 0; SIndex < Stats->SourcesCount; ++SIndex) {
        TLOGSource* Source = &Stats->Sources[SIndex];
        u64 PacketCount = TLOG_PACKET_COUNT(Source->KindCounts);

        if (
            !Source->KindCounts[MAVLINK_FRAME_KIND_OKAY] || 
            PacketCount > TLOG_SOURCE_PACKETS_MAX
        ) {
            continue;
        }

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
                Size,
                Source->HeadOffsets[PIndex]
            );
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

    u64 HeartbeatSources = 0;
    u64 StatusSources = 0;
    u64 RadioSources = 0;

    for (u64 SIndex = 0; SIndex < Stats->SourcesCount; ++SIndex) {
        TLOGSource* Source = &Stats->Sources[SIndex];

        HeartbeatSources += !!Source->HeartbeatCount;
        StatusSources += !!(Source->SysStatusCount | Source->SystemTimeCount);
        RadioSources += !!Source->RadioStatusCount;
    }

    if (HeartbeatSources) {
        TLOGReportHeading(
            MemPool, 
            Strings, 
            "HEARTBEAT FROM EACH SYSTEM ID AND COMPONENT ID"
        );
        ListPush(
            MemPool,
            Strings,
            "    An autopilot sends one HEARTBEAT each second. SECONDS is the time from the first to the last HEARTBEAT.\n"
            "    CHANGES is the number of HEARTBEAT packets that have a type or autopilot that is not the same as in\n"
            "    the HEARTBEAT before it.\n\n"_s8
        );
        ListPushFmt(
            MemPool,
            Strings,
            "    %S%6s %9s %14s %14s %6s %9s %14s%S\n",
            PRINT_STR(TLOG_STYLES[TLOG_STYLE_DIM]),
            "SYSTEM",
            "COMPONENT",
            "HEARTBEAT",
            "SECONDS",
            "TYPE",
            "AUTOPILOT",
            "CHANGES",
            PRINT_STR(TLOG_STYLES[TLOG_STYLE_RESET])
        );

        for (u64 SIndex = 0; SIndex < Stats->SourcesCount; ++SIndex) {
            TLOGSource* Source = &Stats->Sources[SIndex];

            if (!Source->HeartbeatCount)
                continue;

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
                Source->HeartbeatCount, 
                TLOG_STYLE_RESET
            );
            TLOGReportCell(
                MemPool, 
                Strings, 
                (
                    Source->HeartbeatTailTime - Source->HeartbeatHeadTime
                ) / MILLION(1), 
                TLOG_STYLE_RESET
            );
            ListPushFmt(
                MemPool, 
                Strings, 
                " %6d %9d", 
                (i32) Source->HeartbeatHeadType, 
                (i32) Source->HeartbeatHeadAutopilot
            );
            TLOGReportCell(
                MemPool, 
                Strings, 
                Source->HeartbeatChangeCount, 
                TLOG_STYLE_WARN
            );
            ListPush(MemPool, Strings, "\n"_s8);
        }
    }

    if (StatusSources) {
        TLOGReportHeading(
            MemPool, 
            Strings, 
            "SYS_STATUS AND SYSTEM_TIME FROM EACH SYSTEM ID AND COMPONENT ID"
        );
        ListPush(
            MemPool,
            Strings,
            "    ERRORS_COMM is the increase of errors_comm from the first to the last SYS_STATUS.\n"
            "    DROP_RATE_COMM is the largest drop_rate_comm, in percent.\n"
            "    CLOCK OFFSET is the log timestamp minus time_unix_usec, in milliseconds. A large range shows that\n"
            "    the clock of the log or the clock of the system moved.\n\n"_s8
        );
        ListPushFmt(
            MemPool,
            Strings,
            "    %S%6s %9s %14s %14s %14s %14s %16s %16s%S\n",
            PRINT_STR(TLOG_STYLES[TLOG_STYLE_DIM]),
            "SYSTEM",
            "COMPONENT",
            "SYS_STATUS",
            "ERRORS_COMM",
            "DROP_RATE_COMM",
            "SYSTEM_TIME",
            "CLOCK OFFSET MIN",
            "CLOCK OFFSET MAX",
            PRINT_STR(TLOG_STYLES[TLOG_STYLE_RESET])
        );

        for (u64 SIndex = 0; SIndex < Stats->SourcesCount; ++SIndex) {
            TLOGSource* Source = &Stats->Sources[SIndex];
            u64 ErrorsComm = (u16) (
                Source->ErrorsCommTail - Source->ErrorsCommHead
            );

            if (!(Source->SysStatusCount | Source->SystemTimeCount))
                continue;

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
                Source->SysStatusCount, 
                TLOG_STYLE_RESET
            );
            TLOGReportCell(MemPool, Strings, ErrorsComm, TLOG_STYLE_BAD);
            ListPushFmt(
                MemPool,
                Strings,
                "         %S%3d.%02d%S",
                PRINT_STR(
                    TLOG_STYLES[Source->DropRateMax 
                        ? TLOG_STYLE_WARN 
                        : TLOG_STYLE_DIM]
                ),
                (i32) (Source->DropRateMax / 100),
                (i32) (Source->DropRateMax % 100),
                PRINT_STR(TLOG_STYLES[TLOG_STYLE_RESET])
            );
            TLOGReportCell(
                MemPool, 
                Strings, 
                Source->SystemTimeCount, 
                TLOG_STYLE_RESET
            );

            if (Source->SystemTimeCount) {
                ListPushFmt(
                    MemPool,
                    Strings,
                    " %16lld %16lld",
                    Source->ClockOffsetMin / 1000,
                    Source->ClockOffsetMax / 1000
                );
            }

            ListPush(MemPool, Strings, "\n"_s8);
        }
    }

    if (RadioSources) {
        TLOGReportHeading(
            MemPool, 
            Strings, 
            "RADIO_STATUS FROM EACH SYSTEM ID AND COMPONENT ID"
        );
        ListPush(
            MemPool,
            Strings,
            "    RXERRORS and FIXED are the increase from the first to the last RADIO_STATUS.\n\n"_s8
        );
        ListPushFmt(
            MemPool,
            Strings,
            "    %S%6s %9s %14s %9s %9s %14s %14s%S\n",
            PRINT_STR(TLOG_STYLES[TLOG_STYLE_DIM]),
            "SYSTEM",
            "COMPONENT",
            "RADIO_STATUS",
            "RSSI MIN",
            "NOISE MAX",
            "RXERRORS",
            "FIXED",
            PRINT_STR(TLOG_STYLES[TLOG_STYLE_RESET])
        );

        for (u64 SIndex = 0; SIndex < Stats->SourcesCount; ++SIndex) {
            TLOGSource* Source = &Stats->Sources[SIndex];

            if (!Source->RadioStatusCount)
                continue;

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
                Source->RadioStatusCount, 
                TLOG_STYLE_RESET
            );
            ListPushFmt(
                MemPool, 
                Strings, 
                " %9d %9d", 
                (i32) Source->RSSIMin, 
                (i32) Source->NoiseMax
            );
            TLOGReportCell(
                MemPool, 
                Strings, 
                (u16) (
                    Source->RXErrorsTail - Source->RXErrorsHead
                ), 
                TLOG_STYLE_BAD
            );
            TLOGReportCell(
                MemPool, 
                Strings, 
                (u16) (
                    Source->FixedTail - Source->FixedHead
                ), 
                TLOG_STYLE_WARN
            );
            ListPush(MemPool, Strings, "\n"_s8);
        }
    }

    if ((Bad || JunkRuns) && ProblemsMax) {
        u64 Remaining = ProblemsMax;
        Str8 ShownStr = TLOGStrFromCount(
            MemPool, 
            MIN(ProblemsMax, Bad + JunkRuns)
        );
        Str8 AllStr = TLOGStrFromCount(MemPool, Bad + JunkRuns);

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

        for (
            u64 LIndex = 0; 
            LIndex < LanesCount && Remaining; 
            ++LIndex
        ) {
            TLOGLane* Lane = &Lanes[LIndex];

            for (
                u64 PIndex = 0; 
                PIndex < Lane->ProblemsCount && Remaining; 
                ++PIndex, --Remaining
            ) {
                TLOGProblem* Problem = &Lane->Problems[PIndex];

                TLOGReportRecord(
                    MemPool, 
                    Strings, 
                    Base, 
                    Size, 
                    &Problem->Record,
                    Problem->Fix
                );
            }
        }
    }

    if (Stats->HiddenCount) {
        u64 Remaining = ProblemsMax;
        Str8 HiddenString = TLOGStrFromCount(MemPool, Stats->HiddenCount);
        Str8 HidingString = TLOGStrFromCount(MemPool, Stats->HidingCount);
        Str8 CutOffString = TLOGStrFromCount(MemPool, Stats->CutOffCount);
        Str8 ShownString = TLOGStrFromCount(
            MemPool, 
            MIN(ProblemsMax, Stats->HidingCount)
        );

        TLOGReportHeading(MemPool, Strings, "LOST PACKETS");
        ListPushFmt(
            MemPool,
            Strings,
            "    An incorrect packet can have a length that is too large. The bytes of correct packets are then\n"
            "    in the incorrect packet, and the last packet in it can be cut off. Bytes that are not in a record\n"
            "    can also have correct packets in them. FUME does not count these packets in the tables above\n\n"
            "    Correct packets that are in incorrect packets:       %S\n"
            "    Packets that are cut off at the end of these:        %S\n"
            "    Incorrect packets that have correct packets in them: %S\n",
            PRINT_STR(HiddenString),
            PRINT_STR(CutOffString),
            PRINT_STR(HidingString)
        );

        if (ProblemsMax) {
            ListPushFmt(
                MemPool,
                Strings,
                "\n    The first %S of %S incorrect packets, each followed by the correct packets in it:\n",
                PRINT_STR(ShownString),
                PRINT_STR(HidingString)
            );
            TLOGReportPacketHeader(MemPool, Strings);
        }

        for (
            u64 LIndex = 0;
            LIndex < LanesCount && Remaining;
            ++LIndex
        ) {
            TLOGLane* Lane = &Lanes[LIndex];

            for (
                u64 HIndex = 0;
                HIndex < Lane->HidingCount && Remaining;
                ++HIndex, --Remaining
            ) {
                TLOGProblem* Hiding = &Lane->Hiding[HIndex];
                u64 Offset = Hiding->Record.Offset;
                u64 End = Offset + Hiding->Record.Size;

                ListPush(MemPool, Strings, TLOG_STYLES[TLOG_STYLE_BAD]);
                TLOGReportRecord(
                    MemPool, 
                    Strings, 
                    Base, 
                    Size, 
                    &Hiding->Record,
                    Hiding->Fix
                );

                u64 Hidden = TLOGFindHiddenPacket(
                    Base,
                    Size,
                    Offset + TLOG_TIMESTAMP_SIZE + 1,
                    End
                );
                u64 Tail = End;

                while (Hidden < End) {
                    MAVLinkFrame HiddenFrame = MAVLinkFrameFromPtr(
                        Base + Hidden
                    );

                    ListPush(
                        MemPool,
                        Strings,
                        TLOG_STYLES[TLOG_STYLE_DIM]
                    );
                    TLOGReportFrame(
                        MemPool,
                        Strings,
                        Hidden,
                        &HiddenFrame,
                        MAVLINK_FRAME_KIND_OKAY,
                        0,
                        "No timestamp"_s8,
                        MAVLinkFix{}
                    );

                    Tail = Hidden + MAVLinkFrameSizeFromPtr(Base + Hidden);
                    Hidden = TLOGFindHiddenPacket(Base, Size, Tail, End);
                }

                if (Tail < End && MAVLINK_IS_MAGIC(Base[Tail])) {
                    Str8 TailString = ArenaPushStrFmt(
                        MemPool,
                        "0x%llX",
                        Tail
                    );

                    ListPushFmt(
                        MemPool,
                        Strings,
                        "    %S%-12S %llu bytes of a packet that is cut off%S\n",
                        PRINT_STR(TLOG_STYLES[TLOG_STYLE_DIM]),
                        PRINT_STR(TailString),
                        End - Tail,
                        PRINT_STR(TLOG_STYLES[TLOG_STYLE_RESET])
                    );
                }
            }
        }
    }

    if (Bad || JunkRuns) {
        u64 Problems = Bad + JunkRuns;

        TLOGReportHeading(
            MemPool, 
            Strings, 
            "CAUSE OF THE INCORRECT PACKETS AND THE BYTES THAT ARE NOT IN A RECORD"
        );
        ListPush(
            MemPool,
            Strings,
            "    FUME finds the one change that makes the checksum correct. The checksum has 16 bits, so a cause\n"
            "    can be correct by chance. This is rare, less than 1 in 200 for each packet.\n\n"_s8
        );

        for (u32 Field = 0; Field < MAVLINK_FIELD_COUNT; ++Field) {
            if (Stats->FieldCounts[Field]) {
                TLOGReportCount(
                    MemPool, 
                    Strings, 
                    ArenaPushStrFmt(
                        MemPool, 
                        "One byte is incorrect: %S", 
                        PRINT_STR(MAVLINK_FIELD_NAMES[Field])
                    ), 
                    Stats->FieldCounts[Field], 
                    Problems, 
                    TLOG_STYLE_RESET
                );
            }
        }

        TLOGReportCount(
            MemPool, 
            Strings, 
            "One byte is lost"_s8, 
            Stats->FixCounts[MAVLINK_FIX_KIND_LOSS], 
            Problems, 
            TLOG_STYLE_RESET
        );
        TLOGReportCount(
            MemPool, 
            Strings, 
            "One byte is added"_s8, 
            Stats->FixCounts[MAVLINK_FIX_KIND_ADDITION], 
            Problems, 
            TLOG_STYLE_RESET
        );
        TLOGReportCount(
            MemPool, 
            Strings, 
            "More than one change is necessary"_s8, 
            Stats->FixCounts[MAVLINK_FIX_KIND_NONE], 
            Problems, 
            TLOG_STYLE_RESET
        );
        TLOGReportCount(
            MemPool, 
            Strings, 
            "The bytes do not start with a packet"_s8, 
            Stats->FixCounts[MAVLINK_FIX_KIND_NO_FRAME], 
            Problems, 
            TLOG_STYLE_RESET
        );
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
            "    %S%10s  %-40s %14s %14s %14s%S\n",
            PRINT_STR(TLOG_STYLES[TLOG_STYLE_DIM]),
            "MESSAGE ID",
            "NAME",
            "CORRECT",
            "INCORRECT",
            "BYTES",
            PRINT_STR(TLOG_STYLES[TLOG_STYLE_RESET])
        );

        for (u32 Slot = 0; Slot < MAVLINK_MSG_COUNT; ++Slot) {
            u64 MsgOkay = Stats->MsgCounts[Slot][MAVLINK_FRAME_KIND_OKAY];
            u64 MsgBad = Stats->MsgCounts[Slot][MAVLINK_FRAME_KIND_BAD_CRC];
            Str8 OkayStr = TLOGStrFromCount(MemPool, MsgOkay);
            Str8 BadStr = TLOGStrFromCount(MemPool, MsgBad);
            Str8 BytesStr = TLOGStrFromCount(MemPool, Stats->MsgBytes[Slot]);

            if (!(MsgOkay | MsgBad))
                continue;

            ListPushFmt(
                MemPool,
                Strings,
                "    %10d  %-40S %14S %S%14S%S %14S%s\n",
                (i32) MAVLINK_MSG_IDS[Slot],
                PRINT_STR(MAVLINK_MSG_NAMES[Slot]),
                PRINT_STR(OkayStr),
                PRINT_STR(TLOG_STYLES[MsgBad ? TLOG_STYLE_BAD : TLOG_STYLE_DIM]),
                PRINT_STR(BadStr),
                PRINT_STR(TLOG_STYLES[TLOG_STYLE_RESET]),
                PRINT_STR(BytesStr),
                (!MsgOkay && MsgBad > 2) 
                    ? "   No packet is correct. Examine the CRC_EXTRA value of this message in MAVLINK_MSG_XLIST." 
                    : ""
            );
        }
    }
}
