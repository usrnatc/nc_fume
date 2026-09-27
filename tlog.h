#if !defined(__TLOG_H__)
#define __TLOG_H__

#include "nc_types.h"
#include "nc_string.h"
#include "nc_time.h"
#include "mavlink.h"

struct Arena;

// @defines____________________________________________________________________
#define TLOG_TIMESTAMP_SIZE        8
#define TLOG_RECORD_SIZE_MIN       (TLOG_TIMESTAMP_SIZE + MAVLINK_FRAME_SIZE_MIN)
#define TLOG_TIME_MIN_USECS        1262304000000000ULL                          // NOTE(nc): 2010-01-01 00:00:00 UTC
#define TLOG_TIME_MAX_USECS        2524608000000000ULL                          // NOTE(nc): 2050-01-01 00:00:00 UTC
#define TLOG_TIME_GAP_USECS        SECONDS(1)
#define TLOG_TIME_SLACK_USECS      SECONDS(1)
#define TLOG_PROBLEMS_MAX_DEFAULT  16
#define TLOG_UNKNOWN_MSGS_MAX      16
#define TLOG_SOURCES_MAX           64
#define TLOG_SOURCE_PACKETS_MAX    8
#define TLOG_PARTS_COUNT           16
#define TLOG_OFFSET_NONE           U64_MAX
#define TLOG_DUMP_LINE_SIZE        160
#define TLOG_RULE_SIZE             100
#define TLOG_JUNK_TEXT_SIZE        60
#define TLOG_FILTER_MSG_WORDS      (MAVLINK_MSG_SLOTS_COUNT / 64)
#define TLOG_DUMP_TIME_SIZE        26

#define TLOG_PACKET_COUNT(X) ((X)[MAVLINK_FRAME_KIND_OKAY] + (X)[MAVLINK_FRAME_KIND_BAD_CRC] + (X)[MAVLINK_FRAME_KIND_UNKNOWN_MSG])

typedef u8 TLOGStyle;
enum : u8 {
    TLOG_STYLE_RESET,
    TLOG_STYLE_HEADING,
    TLOG_STYLE_DIM,
    TLOG_STYLE_GOOD,
    TLOG_STYLE_BAD,
    TLOG_STYLE_WARN,
    TLOG_STYLE_COUNT
};

typedef u8 TLOGRecordKind;
enum : u8 {
    TLOG_RECORD_KIND_OKAY        = MAVLINK_FRAME_KIND_OKAY,
    TLOG_RECORD_KIND_BAD_CRC     = MAVLINK_FRAME_KIND_BAD_CRC,
    TLOG_RECORD_KIND_UNKNOWN_MSG = MAVLINK_FRAME_KIND_UNKNOWN_MSG,
    TLOG_RECORD_KIND_JUNK        = MAVLINK_FRAME_KIND_COUNT,
    TLOG_RECORD_KIND_COUNT
};

// @types______________________________________________________________________
struct TLOGRecord {
    MAVLinkFrame   Frame;
    u64            Offset;
    u64            Size;
    u64            Time;
    u16            Extra;
    TLOGRecordKind Kind;
};

struct TLOGProblem {
    TLOGRecord Record;
    MAVLinkFix Fix;
};

struct TLOGUnknownMsg {
    u64 Count;
    u32 MsgID;
    u16 Extra;
};

struct TLOGSource {
    u64 KindCounts[MAVLINK_FRAME_KIND_COUNT];
    u64 HeadOffsets[TLOG_SOURCE_PACKETS_MAX];
    u64 CopyCount;
    u64 TailOffset;
    u64 HeartbeatCount;
    u64 HeartbeatChangeCount;
    u64 HeartbeatHeadTime;
    u64 HeartbeatTailTime;
    u64 SysStatusCount;
    u64 SystemTimeCount;
    u64 RadioStatusCount;
    i64 ClockOffsetMin;
    i64 ClockOffsetMax;
    u32 TailSize;
    u16 ErrorsCommHead;
    u16 ErrorsCommTail;
    u16 DropRateMax;
    u16 RXErrorsHead;
    u16 RXErrorsTail;
    u16 FixedHead;
    u16 FixedTail;
    u8  HeartbeatHeadType;
    u8  HeartbeatHeadAutopilot;
    u8  HeartbeatTailType;
    u8  HeartbeatTailAutopilot;
    u8  RSSIMin;
    u8  NoiseMax;
    u8  TailSeq;
    u8  SysID;
    u8  CompID;
};

struct TLOGPart {
    u64 KindCounts[TLOG_RECORD_KIND_COUNT];
    u64 JunkBytes;
    u64 HeadTime;
};

struct TLOGStats {
    u64              KindCounts[TLOG_RECORD_KIND_COUNT];
    u64              VersionCounts[2];
    u64              SignedCount;
    u64              MsgCounts[MAVLINK_MSG_COUNT][MAVLINK_FRAME_KIND_COUNT];
    u64              MsgBytes[MAVLINK_MSG_COUNT];
    u64              FixCounts[MAVLINK_FIX_KIND_COUNT];
    u64              FieldCounts[MAVLINK_FIELD_COUNT];
    u64              CopyCount;
    u64              JunkTextRuns;
    u16              SourceKeys[TLOG_SOURCES_MAX];
    TLOGSource       Sources[TLOG_SOURCES_MAX];
    u64              SourcesCount;
    u64              SourcesOverflowCount;
    TLOGPart         Parts[TLOG_PARTS_COUNT];
    TLOGUnknownMsg   UnknownMsgs[TLOG_UNKNOWN_MSGS_MAX];
    u64              UnknownMsgsCount;
    u64              JunkBytes;
    u64              TimeInsaneCount;
    u64              TimeBackwardsCount;
    u64              TimeGapsCount;
    u64              TimeGapMaxUSecs;
    u64              BadAfterTimeGapCount;
    u64              HeadTime;
    u64              HiddenCount;
    u64              HidingCount;
    u64              CutOffCount;
    u64              TailTime;
    r1u64            TimeRange;
    MAVLinkFrameKind HeadTimeKind;
};

struct TLOGFilter {
    u64   MsgIDs[TLOG_FILTER_MSG_WORDS];
    u64   SysIDs[4];
    u64   CompIDs[4];
    Str8  Label;
    r1u64 Times;
    r1u64 Offsets;
};

struct TLOGLane {
    u64          HeadOffset;
    u64          Limit;
    Arena*       Dump;
    TLOGProblem* Problems;
    u64          ProblemsCount;
    TLOGProblem* Hiding;
    u64          HidingCount;
    TLOGStats    Stats;
};

// @runtime____________________________________________________________________
extern Str8 TLOG_STYLES[TLOG_STYLE_COUNT];

// @functions__________________________________________________________________
void TLOGStylesInit(b32 UseColour);
INTERNAL TLOGRecord TLOGRecordFromOffset(u8* Base, u64 Size, u64 Offset);
INTERNAL u64 TLOGFindMagic(u8* Base, u64 Offset, u64 Limit);
INTERNAL u64 TLOGSourceIndexFromKey(TLOGStats* Stats, u16 Key);
void TLOGSourcesSort(TLOGStats* Stats);
void TLOGDump(u8* Base, u64 Size, u64 Offset, u64 Limit, OUT Arena* Dump, TLOGFilter* Filter);
void TLOGReportRule(Arena* MemPool, Str8List* Out);
INTERNAL b32 TLOGTimeIsSane(u64 USecs);
INTERNAL b32 TLOGIsRecordHead(u8* Base, u64 Size, u64 Offset);
u64 TLOGFindRecord(u8* Base, u64 Size, u64 Offset, u64 Limit);
u64 TLOGFindHiddenPacket(u8* Base, u64 Size, u64 Offset, u64 Limit);
INTERNAL b32 TLOGIsText(u8* Ptr, u64 Size);
INTERNAL b32 TLOGPassesFilter(TLOGFilter* Filter, MAVLinkFrame* Frame, u64 Time);
void TLOGWalk(u8* Base, u64 Size, u64 Offset, u64 Limit, TLOGLane* Lane, u64 ProblemsMax, TLOGFilter* Filter);
void TLOGStatsMerge(TLOGStats* Dst, TLOGStats* Src, u8* Base);
Str8 TLOGStrFromUSecs(Arena* MemPool, u64 USecs);
void TLOGReportFileRowHeader(Arena* MemPool, Str8List* Out);
void TLOGReportFileRow(Arena* MemPool, Str8List* Out, Str8 Path, TLOGStats* Stats);
void TLOGReport(Arena* MemPool, Str8List* Out, Str8 Path, u64 FileIndex, u64 FilesCount, u8* Base, u64 Size, TLOGStats* Stats, TLOGLane* Lanes, u64 LanesCount, u64 ProblemsMax, b32 ShouldReportMsgs, TLOGFilter* Filter);

#endif // __TLOG_H__
