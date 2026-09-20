#if !defined(__NC_SYNC_H__)
#define __NC_SYNC_H__

#include "nc_types.h"
#include "nc_system.h"
#include "nc_string.h"

// @defines____________________________________________________________________
#define LOCK_SCOPE_R(X)       DEFER(RWMutexLockR(X), RWMutexUnlockR(X))
#define LOCK_SCOPE_BREAK_R(X) DEFER(RWMutexUnlockR(X), RWMutexLockR(X))
#define LOCK_SCOPE_W(X)       DEFER(RWMutexLockW(X), RWMutexUnlockW(X))
#define LOCK_SCOPE_BREAK_W(X) DEFER(RWMutexUnlockW(X), RWMutexLockW(X))
#define LOCK_SCOPE(X)         DEFER(MutexLock(X), MutexUnlock(X))
#define LOCK_SCOPE_UPGRADE(X) DEFER((RWMutexUnlockR(X), RWMutexLockW(X)), (RWMutexUnlockW(X), RWMutexLockR(X)))
#define RWLOCK_SCOPE(X, Y)    DEFER(RWMutexLock(X, Y), RWMutexUnlock(X, Y))

// @types______________________________________________________________________
struct Stripe {
    Arena* MemPool;
    Handle RWMtx;
    Handle CondVar;
    void*  Free;
};

struct StripeArray {
    Stripe* V;
    u64     Count;
};

// @functions__________________________________________________________________
Handle MutexAlloc(void);
void MutexUnlock(Handle MtxHandle);
void MutexLock(Handle MtxHandle);
void MutexDestroy(Handle MtxHandle);
Handle RWMutexAlloc(void);
void RWMutexDestroy(Handle MtxHandle);
void RWMutexLockR(Handle MtxHandle);
void RWMutexUnlockR(Handle MtxHandle);
void RWMutexLockW(Handle MtxHandle);
void RWMutexUnlockW(Handle MtxHandle);
void RWMutexLock(Handle MtxHandle, b32 Write);
void RWMutexUnlock(Handle MtxHandle, b32 Write);
Handle CondVarAlloc(void);
void CondVarDestroy(Handle CondHandle);
b32 CondVarWait(Handle CondHandle, Handle MtxHandle, u64 EndTimeUSecs);
b32 CondVarWaitR(Handle CondHandle, Handle RWMtxHandle, u64 EndTimeUSecs);
b32 CondVarWaitW(Handle CondHandle, Handle RWMtxHandle, u64 EndTimeUSecs);
void CondVarSignal(Handle CondHandle);
void CondVarBroadcast(Handle CondHandle);
Handle SemaphoreAlloc(u32 InitialCount, u32 MaxCount, Str8 Name);
void SemaphoreDestroy(Handle SemHandle, Str8 Name);
Handle SemaphoreOpen(Str8 Name);
void SemaphoreClose(Handle SemHandle);
b32 SemaphoreLock(Handle SemHandle, u64 EndTimeUSecs);
void SemaphoreUnlock(Handle SemHandle);
Handle BarrierAlloc(u64 Count);
void BarrierRelease(Handle Barrier);
void BarrierWait(Handle Barrier);
StripeArray StripeArrayAlloc(Arena* MemPool);
void StripeArrayRelease(StripeArray* Stripes);
Stripe* StripeFromSlotIndex(StripeArray* Stripes, u64 SlotIndex);

#endif // __NC_SYNC_H__
