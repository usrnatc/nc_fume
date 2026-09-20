#include "nc_thread.h"
#include "nc_atomics.h"
#include "nc_memory.h"
#include "nc_arena.h"
#include "nc_tls.h"
#include "nc_sync.h"

void 
ThreadContextSetThreadName(Str8 Name)
{
    ThreadContext* Ctx = ThreadContextSelected();
    u64 Size = CLAMP_TOP(Name.Size, ARRAY_COUNT(Ctx->ThreadName));

    MemCpy(Ctx->ThreadName, Name.Str, Size);
    Ctx->ThreadNameSize = Size;
}

Str8 
ThreadContextGetThreadName(void)
{
    ThreadContext* Ctx = ThreadContextSelected();
    Str8 Result = Str(Ctx->ThreadName, Ctx->ThreadNameSize);

    return Result;
}

void 
ThreadContextWriteSrcLoc(char* FileName, u64 LineNum)
{
    ThreadContext* Ctx = ThreadContextSelected();

    Ctx->FileName = FileName;
    Ctx->LineNumber = LineNum;
}

void 
ThreadContextReadSrcLoc(OUT char** FileName, OUT u64* LineNum)
{
    ThreadContext* Ctx = ThreadContextSelected();

    *FileName = Ctx->FileName;
    *LineNum = Ctx->LineNumber;
}

ThreadContext* 
ThreadContextAlloc(void)
{
    Arena* MemPool = ArenaAlloc();
    ThreadContext* Result = ArenaPushArrayZero(MemPool, ThreadContext, 1);

    Result->MemPools[0] = MemPool;
    Result->MemPools[1] = ArenaAlloc();
    Result->LaneCtx.Count = 1;

    return Result;
}

void 
ThreadContextRelease(ThreadContext* Ctx)
{
    ArenaRelease(Ctx->MemPools[1]);
    ArenaRelease(Ctx->MemPools[0]);
}

void 
ThreadContextSelect(ThreadContext* Ctx)
{
    GetTLS()->LOCAL_THREAD_CTX = Ctx;
}

ThreadContext* 
ThreadContextSelected(void)
{
    return GetTLS()->LOCAL_THREAD_CTX;
}

LaneContext 
ThreadContextSetLaneContext(LaneContext LaneCtx)
{
    ThreadContext* Ctx = ThreadContextSelected();
    LaneContext Restore = Ctx->LaneCtx;

    Ctx->LaneCtx = LaneCtx;

    return Restore;
}

void 
ThreadContextBarrierWait(void* BroadcastPtr, u64 BroadcastSize, u64 BroadcastSrcLaneIndex)
{
    ThreadContext* Ctx = ThreadContextSelected();
    u64 BroadcastSizeClamped = CLAMP_TOP(BroadcastSize, sizeof(Ctx->LaneCtx.BroadcastMemory[0]));

    if (BroadcastPtr && LaneIndex() == BroadcastSrcLaneIndex)
        MemCpy(Ctx->LaneCtx.BroadcastMemory, BroadcastPtr, BroadcastSizeClamped);

    BarrierWait(Ctx->LaneCtx.Barrier);

    if (BroadcastPtr && LaneIndex() != BroadcastSrcLaneIndex)
        MemCpy(BroadcastPtr, Ctx->LaneCtx.BroadcastMemory, BroadcastSizeClamped);

    if (BroadcastPtr)
        BarrierWait(Ctx->LaneCtx.Barrier);
}

Access* 
AccessOpen(void)
{
    ThreadContext* LOCAL_THREAD_CTX = GetTLS()->LOCAL_THREAD_CTX;

    if (!LOCAL_THREAD_CTX->AccessMemPool)
        LOCAL_THREAD_CTX->AccessMemPool = ArenaAlloc();

    Access* Acc = LOCAL_THREAD_CTX->FreeAccess;

    if (Acc)
        SLL_STACK_POP(LOCAL_THREAD_CTX->FreeAccess);
    else
        Acc = ArenaPushArray(LOCAL_THREAD_CTX->AccessMemPool, Access, 1);

    MemSet(Acc, 0, sizeof(*Acc));

    return Acc;
}

void 
AccessClose(Access* Acc)
{
    ThreadContext* LOCAL_THREAD_CTX = GetTLS()->LOCAL_THREAD_CTX;

    for (
        Touch* T = Acc->HeadTouch, *Next = NULL;
        T;
        T = Next
    ) {
        Next = T->Next;
        AtomicDecFetchU64(&T->Point->ReferenceCount, MEM_ORDER_SEQ_CST);

        if (T->CondVar.V[0])
            CondVarBroadcast(T->CondVar);

        SLL_STACK_PUSH(LOCAL_THREAD_CTX->FreeTouch, T);
    }

    SLL_STACK_PUSH(LOCAL_THREAD_CTX->FreeAccess, Acc);
}

void 
AccessTouch(Access* Acc, AccessPoint* Point, Handle CondVar)
{
    AtomicIncFetchU64(&Point->ReferenceCount, MEM_ORDER_SEQ_CST);
    AtomicExchangeU64(&Point->LastTimeTouchedUSecs, TimeNow(), MEM_ORDER_SEQ_CST);
    AtomicExchangeU64(&Point->LastUpdateIndexTouched, UpdateTickIndex(), MEM_ORDER_SEQ_CST);

    ThreadContext* LOCAL_THREAD_CTX = GetTLS()->LOCAL_THREAD_CTX;
    Touch* T = LOCAL_THREAD_CTX->FreeTouch;

    if (T)
        SLL_STACK_POP(LOCAL_THREAD_CTX->FreeTouch);
    else
        T = ArenaPushArray(LOCAL_THREAD_CTX->AccessMemPool, Touch, 1);

    MemSet(T, 0, sizeof(*T));
    SLL_STACK_PUSH(Acc->HeadTouch, T);
    T->CondVar = CondVar;
    T->Point = Point;
}

b32 
__AccessPointIsExpired(AccessPoint* Point, AccessPointExpireParams* Params)
{
    u64 ReferenceCount = AtomicLoadU64(&Point->ReferenceCount, MEM_ORDER_SEQ_CST);
    u64 LastTimeTouchedUSecs = AtomicLoadU64(&Point->LastTimeTouchedUSecs, MEM_ORDER_SEQ_CST);
    u64 LastUpdateIndexTouched = AtomicLoadU64(&Point->LastUpdateIndexTouched, MEM_ORDER_SEQ_CST);
    b32 Result = (
        !ReferenceCount &&
        LastTimeTouchedUSecs + Params->Time <= TimeNow() &&
        LastUpdateIndexTouched + Params->UpdateIndexes <= UpdateTickIndex()
    );

    return Result;
}
