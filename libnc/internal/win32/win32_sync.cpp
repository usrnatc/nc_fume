#include "win32_platform.h"

Handle 
MutexAlloc(void)
{
    OSEntity* Entity = EntityAlloc(ENTITY_KIND_MUTEX);

    InitializeCriticalSection(&Entity->Mutex);

    Handle Result = {};

    *Result.V = INT_FROM_PTR(Entity);

    return Result;
}

void 
MutexUnlock(Handle MtxHandle)
{
    OSEntity* Entity = (OSEntity*) PTR_FROM_INT(*MtxHandle.V);

    LeaveCriticalSection(&Entity->Mutex);
}

void 
MutexLock(Handle MtxHandle)
{
    OSEntity* Entity = (OSEntity*) PTR_FROM_INT(*MtxHandle.V);

    EnterCriticalSection(&Entity->Mutex);
}

void 
MutexDestroy(Handle MtxHandle)
{
    OSEntity* Entity = (OSEntity*) PTR_FROM_INT(*MtxHandle.V);

    EntityDestroy(Entity);
}

Handle 
RWMutexAlloc(void)
{
    OSEntity* Entity = EntityAlloc(ENTITY_KIND_RWMUTEX);

    InitializeSRWLock(&Entity->RWMutex);

    Handle Result = {};

    *Result.V = INT_FROM_PTR(Entity);

    return Result;
}

void 
RWMutexDestroy(Handle MtxHandle)
{
    OSEntity* Entity = (OSEntity*) PTR_FROM_INT(*MtxHandle.V);

    EntityDestroy(Entity);
}

void 
RWMutexLockR(Handle MtxHandle)
{
    OSEntity* Entity = (OSEntity*) PTR_FROM_INT(*MtxHandle.V);

    AcquireSRWLockShared(&Entity->RWMutex);
}

void 
RWMutexUnlockR(Handle MtxHandle)
{
    OSEntity* Entity = (OSEntity*) PTR_FROM_INT(*MtxHandle.V);

    ReleaseSRWLockShared(&Entity->RWMutex);
}

void 
RWMutexLockW(Handle MtxHandle)
{
    OSEntity* Entity = (OSEntity*) PTR_FROM_INT(*MtxHandle.V);

    AcquireSRWLockExclusive(&Entity->RWMutex);
}

void 
RWMutexUnlockW(Handle MtxHandle)
{
    OSEntity* Entity = (OSEntity*) PTR_FROM_INT(*MtxHandle.V);

    ReleaseSRWLockExclusive(&Entity->RWMutex);
}

void 
RWMutexLock(Handle MtxHandle, b32 Write)
{
    if (Write)
        RWMutexLockW(MtxHandle);
    else
        RWMutexLockR(MtxHandle);
}

void 
RWMutexUnlock(Handle MtxHandle, b32 Write)
{
    if (Write)
        RWMutexUnlockW(MtxHandle);
    else
        RWMutexUnlockR(MtxHandle);
}

Handle 
CondVarAlloc(void)
{
    OSEntity* Entity = EntityAlloc(ENTITY_KIND_CONDVAR);

    InitializeConditionVariable(&Entity->CondVar);

    Handle Result = {};

    *Result.V = INT_FROM_PTR(Entity);

    return Result;
}

void 
CondVarDestroy(Handle CondHandle)
{
    OSEntity* Entity = (OSEntity*) PTR_FROM_INT(*CondHandle.V);

    EntityDestroy(Entity);
}

b32 
CondVarWait(Handle CondHandle, Handle MtxHandle, u64 EndTimeUSecs)
{
    u32 SleepMSecs = SleepMSecsFromEndTimeUSecs(EndTimeUSecs);
    BOOL Result = FALSE;

    if (SleepMSecs) {
        OSEntity* Entity = (OSEntity*) PTR_FROM_INT(*CondHandle.V);
        OSEntity* MtxEntity = (OSEntity*) PTR_FROM_INT(*MtxHandle.V);

        Result = SleepConditionVariableCS(
            &Entity->CondVar, 
            &MtxEntity->Mutex, 
            SleepMSecs
        );
    } 

    return Result;
}

b32 
CondVarWaitR(Handle CondHandle, Handle RWMtxHandle, u64 EndTimeUSecs)
{
    u32 SleepMSecs = SleepMSecsFromEndTimeUSecs(EndTimeUSecs);
    BOOL Result = FALSE;

    if (SleepMSecs) {
        OSEntity* Entity = (OSEntity*) PTR_FROM_INT(*CondHandle.V);
        OSEntity* RWMtxEntity = (OSEntity*) PTR_FROM_INT(*RWMtxHandle.V);

        Result = SleepConditionVariableSRW(
            &Entity->CondVar, 
            &RWMtxEntity->RWMutex, 
            SleepMSecs, 
            CONDITION_VARIABLE_LOCKMODE_SHARED
        );
    }

    return Result;
}

b32 
CondVarWaitW(Handle CondHandle, Handle RWMtxHandle, u64 EndTimeUSecs)
{
    u32 SleepMSecs = SleepMSecsFromEndTimeUSecs(EndTimeUSecs);
    BOOL Result = FALSE;

    if (SleepMSecs) {
        OSEntity* Entity = (OSEntity*) PTR_FROM_INT(*CondHandle.V);
        OSEntity* RWMtxEntity = (OSEntity*) PTR_FROM_INT(*RWMtxHandle.V);

        Result = SleepConditionVariableSRW(
            &Entity->CondVar, 
            &RWMtxEntity->RWMutex, 
            SleepMSecs, 
            0
        );
    }

    return Result;
}

void 
CondVarSignal(Handle CondHandle)
{
    OSEntity* Entity = (OSEntity*) PTR_FROM_INT(*CondHandle.V);

    WakeConditionVariable(&Entity->CondVar);
}

void 
CondVarBroadcast(Handle CondHandle)
{
    OSEntity* Entity = (OSEntity*) PTR_FROM_INT(*CondHandle.V);

    WakeAllConditionVariable(&Entity->CondVar);
}

Handle 
SemaphoreAlloc(u32 InitialCount, u32 MaxCount, Str8 Name)
{
    TempArena Scratch = GetScratch(NULL, 0);
    Str16 WideName = Str16FromStr8(Scratch.MemPool, Name);
    HANDLE SemHandle = CreateSemaphoreW(
        NULL, 
        InitialCount, 
        MaxCount, 
        (WCHAR*) WideName.Str
    );
    Handle Result = {};

    *Result.V = INT_FROM_PTR(SemHandle);
    ReleaseScratch(Scratch);

    return Result;
}

void 
SemaphoreDestroy(Handle SemHandle, Str8 Name)
{
    UNUSED(Name);

    HANDLE Sem = (HANDLE) PTR_FROM_INT(*SemHandle.V);
    
    CloseHandle(Sem);
}

Handle 
SemaphoreOpen(Str8 Name)
{
    TempArena Scratch = GetScratch(NULL, 0);
    Str16 WideName = Str16FromStr8(Scratch.MemPool, Name);
    HANDLE SemHandle = OpenSemaphoreW(
        SEMAPHORE_ALL_ACCESS, 
        FALSE, 
        (WCHAR*) WideName.Str
    );
    Handle Result = {};

    *Result.V = INT_FROM_PTR(SemHandle);
    ReleaseScratch(Scratch);

    return Result;
}

void 
SemaphoreClose(Handle SemHandle)
{
    HANDLE Sem = (HANDLE) PTR_FROM_INT(*SemHandle.V);

    CloseHandle(Sem);
}

b32 
SemaphoreLock(Handle SemHandle, u64 EndTimeUSecs)
{
    u32 SleepMSecs = SleepMSecsFromEndTimeUSecs(EndTimeUSecs);
    HANDLE Sem = (HANDLE) PTR_FROM_INT(*SemHandle.V);
    DWORD WaitResult = WaitForSingleObject(Sem, SleepMSecs);

    return (WaitResult == WAIT_OBJECT_0);
}

void 
SemaphoreUnlock(Handle SemHandle)
{
    HANDLE Sem = (HANDLE) PTR_FROM_INT(*SemHandle.V);

    ReleaseSemaphore(Sem, 1, NULL);
}

Handle 
BarrierAlloc(u64 Count)
{
    OSEntity* Entity = EntityAlloc(ENTITY_KIND_BARRIER);

    if (Entity) {
        BOOL InitOkay = InitializeSynchronizationBarrier(&Entity->Barrier, Count, -1);

        UNUSED(InitOkay);
    }

    Handle Result = {
        INT_FROM_PTR(Entity)
    };

    return Result;
}

void 
BarrierRelease(Handle Barrier)
{
    OSEntity* Entity = (OSEntity*) PTR_FROM_INT(*Barrier.V);

    if (Entity) {
        DeleteSynchronizationBarrier(&Entity->Barrier);
        EntityDestroy(Entity);
    }
}

void 
BarrierWait(Handle Barrier)
{
    OSEntity* Entity = (OSEntity*) PTR_FROM_INT(*Barrier.V);

    if (Entity)
        EnterSynchronizationBarrier(&Entity->Barrier, 0);
}

StripeArray 
StripeArrayAlloc(Arena* MemPool)
{
    StripeArray Result = {};

    Result.Count = GetSystemProperties()->LogicalProcessorCount;
    Result.V = ArenaPushArrayZero(MemPool, Stripe, Result.Count);

    for (u64 Index = 0; Index < Result.Count; ++Index) {
        Result.V[Index].MemPool = ArenaAlloc();
        Result.V[Index].RWMtx = RWMutexAlloc();
        Result.V[Index].CondVar = CondVarAlloc();
    }

    return Result;
}

void 
StripeArrayRelease(StripeArray* Stripes)
{
    for (u64 Index = 0; Index < Stripes->Count; ++Index) {
        ArenaRelease(Stripes->V[Index].MemPool);
        RWMutexDestroy(Stripes->V[Index].RWMtx);
        CondVarDestroy(Stripes->V[Index].CondVar);
    }
}

Stripe* 
StripeFromSlotIndex(StripeArray* Stripes, u64 SlotIndex)
{
    Stripe* Result = &Stripes->V[SlotIndex % Stripes->Count];

    return Result;
}
