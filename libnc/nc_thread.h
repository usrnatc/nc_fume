#if !defined(__NC_THREAD_H__)
#define __NC_THREAD_H__

#include "nc_types.h"
#include "nc_string.h"
#include "nc_system.h"

// @defines____________________________________________________________________
struct Arena;

#define ThreadContextWriteThisSrcLoc() ThreadContextWriteSrcLoc(__FILE__, __LINE__)

#define LaneIndex() (ThreadContextSelected()->LaneCtx.Index)
#define LaneCount() (ThreadContextSelected()->LaneCtx.Count)
#define LaneFromTaskIndex(X) ((X) % LaneCount())
#define SetLaneContext(X) ThreadContextSetLaneContext((X))
#define LaneSync() ThreadContextBarrierWait(NULL, 0, 0)
#define LaneSyncU64(X, Y) ThreadContextBarrierWait((X), sizeof(*(X)), (Y))

#define __RangeFromIndexCount(IndexN, CountN, CountM) [&]() -> r1u64 {                                   \
    u64 MainIndexesPerLane = CountM / CountN;                                                            \
    u64 LeftoverIndexes = CountM - MainIndexesPerLane * CountN;                                          \
    u64 LeftoverIndexesPre = MIN(IndexN, LeftoverIndexes);                                               \
    u64 LaneBaseIndex = IndexN * MainIndexesPerLane + LeftoverIndexesPre;                                \
    u64 LaneBaseIndexClamped = MIN(LaneBaseIndex, CountM);                                               \
    u64 LaneEndIndex = LaneBaseIndexClamped + MainIndexesPerLane + ((IndexN < LeftoverIndexes) ? 1 : 0); \
    u64 LaneEndIndexClamped = MIN(LaneEndIndex, CountM);                                                 \
    r1u64 Result = Rng(LaneBaseIndexClamped, LaneEndIndexClamped);                                       \
    return Result;                                                                                       \
}()

#define LaneRange(X) __RangeFromIndexCount(LaneIndex(), LaneCount(), (X))

#define AccessPointIsExpired(X, ...) [&]() -> b32 {     \
    AccessPointExpireParams __Params = {MILLION(2), 2}; \
    __VA_ARGS__;                                        \
    return __AccessPointIsExpired((X), &__Params);      \
}()

#define SetProgressPtr(X) (ThreadContextSelected()->ProgressCounterPtr = (X))
#define SetProgressTargetPtr(X) (ThreadContextSelected()->ProgressTargetPtr = (X))
#define SetProgress(X) (ThreadContextSelected()->ProgressCounterPtr ? AtomicExchangeU64(ThreadContextSelected()->ProgressCounterPtr, (X), MEM_ORDER_SEQ_CST) : (void) 0)
#define AddProgress(X) (ThreadContextSelected()->ProgressCounterPtr ? AtomicAddFetchU64(ThreadContextSelected()->ProgressCounterPtr, (X), MEM_ORDER_SEQ_CST) : (void) 0)
#define SetProgressTarget(X) (ThreadContextSelected()->ProgressTargetPtr ? AtomicExchangeU64(ThreadContextSelected()->ProgressTargetPtr, (X), MEM_ORDER_SEQ_CST) : (void) 0)

// @types______________________________________________________________________
struct LaneContext {
    u64    Index;
    u64    Count;
    Handle Barrier;
    u64*   BroadcastMemory;
};

struct AccessPoint {
    u64 ReferenceCount;
    u64 LastTimeTouchedUSecs;
    u64 LastUpdateIndexTouched;
};

struct AccessPointExpireParams {
    u64 Time;
    u64 UpdateIndexes;
};

struct Touch {
    Touch*       Next;
    AccessPoint* Point;
    Handle       CondVar;
};

struct Access {
    Access* Next;
    Touch*  HeadTouch;
};

struct ThreadContext {
    Arena* MemPools[2];
    u8 ThreadName[32];
    u64 ThreadNameSize;
    LaneContext LaneCtx;
    char* FileName;
    u64 LineNumber;
    Arena* AccessMemPool;
    Access* FreeAccess;
    Touch* FreeTouch;
    u64* ProgressCounterPtr;
    u64* ProgressTargetPtr;
};

// @functions__________________________________________________________________
u32 ThreadGetID(void);
void ThreadSetName(Str8 Name);
void ThreadSetName(char* Fmt, ...);
Handle ThreadLaunch(ThreadFuncType* Func, void* Ptr, void* Params);
Handle ThreadLaunch(ThreadFuncType* Func, void* Params);
b32 ThreadJoin(Handle ThreadHandle, u64 EndTimeUSecs);
void ThreadDetach(Handle ThreadHandle);
void ThreadContextSetThreadName(Str8 Name);
Str8 ThreadContextGetThreadName(void);
void ThreadContextWriteSrcLoc(char* FileName, u64 LineNum);
void ThreadContextReadSrcLoc(OUT char** FileName, OUT u64* LineNum);
ThreadContext* ThreadContextAlloc(void);
void ThreadContextRelease(ThreadContext* Ctx);
void ThreadContextSelect(ThreadContext* Ctx);
ThreadContext* ThreadContextSelected(void);
LaneContext ThreadContextSetLaneContext(LaneContext LaneCtx);
void ThreadContextBarrierWait(void* BroadcastPtr, u64 BroadcastSize, u64 BroadcastSrcLaneIndex);
Access* AccessOpen(void);
void AccessClose(Access* Acc);
void AccessTouch(Access* Acc, AccessPoint* Point, Handle CondVar);
b32 __AccessPointIsExpired(AccessPoint* Point, AccessPointExpireParams* Params);

#endif // __NC_THREAD_H__
