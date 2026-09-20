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

typedef u8 TLOGProblemKind;
enum : u8 {
    TLOG_PROBLEM_KIND_BAD_CRC,
    TLOG_PROBLEM_KIND_JUNK
};

// @types______________________________________________________________________
struct TLOGRecord {
    MAVLinkFrame     Frame;
    u64              Offset;
    u64              Size;
    u64              Time;
    u16              Extra;
    MAVLinkFrameKind Kind;
    b32              IsPacket;
};

struct TLOGProblem {
    u64             Offset;
    u64             Size;
    TLOGProblemKind Kind;
};

struct TLOGUnknownMsg {
    u64 Count;
    u32 MsgID;
    u16 Extra;
    b32 ExtraVaries;
};

struct TLOGSource {
    u64 KindCounts[MAVLINK_FRAME_KIND_COUNT];
    u64 HeadOffsets[TLOG_SOURCE_PACKETS_MAX];
    u8  SysID;
    u8  CompID;
};

struct TLOGPart {
    u64 KindCounts[MAVLINK_FRAME_KIND_COUNT];
    u64 JunkBytes;
    u64 HeadTime;
};

struct TLOGStats {
    u64              KindCounts[MAVLINK_FRAME_KIND_COUNT];
    u64              VersionCounts[2];
    u64              SignedCount;
    u64              MsgCounts[MAVLINK_MSG_COUNT][MAVLINK_FRAME_KIND_COUNT];
    u64              FixCounts[MAVLINK_FIELD_COUNT];
    TLOGSource       Sources[TLOG_SOURCES_MAX];
    u64              SourcesCount;
    u64              SourcesOverflowCount;
    TLOGPart         Parts[TLOG_PARTS_COUNT];
    TLOGUnknownMsg   UnknownMsgs[TLOG_UNKNOWN_MSGS_MAX];
    u64              UnknownMsgsCount;
    u64              JunkBytes;
    u64              JunkRuns;
    u64              TimeInsaneCount;
    u64              TimeBackwardsCount;
    u64              TimeGapsCount;
    u64              TimeGapMaxUSecs;
    u64              BadAfterTimeGapCount;
    u64              HeadTime;
    u64              TailTime;
    r1u64            TimeRange;
    MAVLinkFrameKind HeadTimeKind;
};

struct TLOGLane {
    u64          HeadOffset;
    u64          DumpHeadLine;
    TLOGProblem* Problems;
    u64          ProblemsCount;
    TLOGStats    Stats;
};

// @runtime____________________________________________________________________
extern Str8 TLOG_STYLES[TLOG_STYLE_COUNT];

// @functions__________________________________________________________________
void TLOGStylesInit(b32 UseColour);
INTERNAL TLOGRecord TLOGRecordFromOffset(u8* Base, u64 Size, u64 Offset);
void TLOGDumpHeader(u8* Line);
void TLOGDump(u8* Base, u64 Size, u64 Offset, u64 Limit, u8* Lines);
void TLOGReportRule(Arena* MemPool, Str8List* Out);
INTERNAL b32 TLOGTimeIsSane(u64 USecs);
INTERNAL b32 TLOGIsRecordHead(u8* Base, u64 Size, u64 Offset);
u64 TLOGFindRecord(u8* Base, u64 Size, u64 Offset, u64 Limit);
void TLOGWalk(u8* Base, u64 Size, u64 Offset, u64 Limit, TLOGLane* Lane, u64 ProblemsMax);
void TLOGStatsMerge(TLOGStats* Dst, TLOGStats* Src);
Str8 TLOGStrFromUSecs(Arena* MemPool, u64 USecs);
void TLOGReportFileRowHeader(Arena* MemPool, Str8List* Out);
void TLOGReportFileRow(Arena* MemPool, Str8List* Out, Str8 Path, TLOGStats* Stats);
void TLOGReport(Arena* MemPool, Str8List* Out, Str8 Path, u64 FileIndex, u64 FilesCount, u8* Base, u64 Size, TLOGStats* Stats, TLOGLane* Lanes, u64 LanesCount, u64 ProblemsMax, b32 ShouldReportMsgs);

#endif // __TLOG_H__
