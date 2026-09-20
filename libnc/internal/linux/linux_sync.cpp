#include "linux_platform.h"

Handle
MutexAlloc(void)
{
    OSEntity* Entity = EntityAlloc(ENTITY_KIND_MUTEX);
    int InitResult = pthread_mutex_init(&Entity->Mutex, NULL);

    if (InitResult == -1) {
        EntityDestroy(Entity);
        Entity = NULL;
    }

    Handle Result = {
        INT_FROM_PTR(Entity)
    };

    return Result;
}

void 
MutexUnlock(Handle MtxHandle)
{
    if (MtxHandle == EMPTY_HANDLE_VALUE)
        return;

    OSEntity* Entity = (OSEntity*) PTR_FROM_INT(MtxHandle.V[0]);

    pthread_mutex_unlock(&Entity->Mutex);
}

void 
MutexLock(Handle MtxHandle)
{
    if (MtxHandle == EMPTY_HANDLE_VALUE)
        return;

    OSEntity* Entity = (OSEntity*) PTR_FROM_INT(MtxHandle.V[0]);

    pthread_mutex_lock(&Entity->Mutex);
}

void 
MutexDestroy(Handle MtxHandle)
{
    if (MtxHandle == EMPTY_HANDLE_VALUE)
        return;

    OSEntity* Entity = (OSEntity*) PTR_FROM_INT(MtxHandle.V[0]);

    pthread_mutex_destroy(&Entity->Mutex);
    EntityDestroy(Entity);
}

Handle 
RWMutexAlloc(void)
{
    OSEntity* Entity = EntityAlloc(ENTITY_KIND_RWMUTEX);
    int InitResult = pthread_rwlock_init(&Entity->RWMutex, NULL);

    if (InitResult == -1) {
        EntityDestroy(Entity);
        Entity = NULL;
    }

    Handle Result = {
        INT_FROM_PTR(Entity)
    };

    return Result;
}

void 
RWMutexDestroy(Handle MtxHandle)
{
    if (MtxHandle == EMPTY_HANDLE_VALUE)
        return;

    OSEntity* Entity = (OSEntity*) PTR_FROM_INT(MtxHandle.V[0]);

    pthread_rwlock_destroy(&Entity->RWMutex);
    EntityDestroy(Entity);
}

void 
RWMutexLockR(Handle MtxHandle)
{
    if (MtxHandle == EMPTY_HANDLE_VALUE)
        return;

    OSEntity* Entity = (OSEntity*) PTR_FROM_INT(MtxHandle.V[0]);

    pthread_rwlock_rdlock(&Entity->RWMutex);
}

void 
RWMutexUnlockR(Handle MtxHandle)
{
    if (MtxHandle == EMPTY_HANDLE_VALUE)
        return;

    OSEntity* Entity = (OSEntity*) PTR_FROM_INT(MtxHandle.V[0]);

    pthread_rwlock_unlock(&Entity->RWMutex);
}

void 
RWMutexLockW(Handle MtxHandle)
{
    if (MtxHandle == EMPTY_HANDLE_VALUE)
        return;

    OSEntity* Entity = (OSEntity*) PTR_FROM_INT(MtxHandle.V[0]);

    pthread_rwlock_wrlock(&Entity->RWMutex);
}

void 
RWMutexUnlockW(Handle MtxHandle)
{
    if (MtxHandle == EMPTY_HANDLE_VALUE)
        return;

    OSEntity* Entity = (OSEntity*) PTR_FROM_INT(MtxHandle.V[0]);

    pthread_rwlock_unlock(&Entity->RWMutex);
}

void 
RWMutexLock(Handle MtxHandle, b32 Write)
{
    if (MtxHandle == EMPTY_HANDLE_VALUE)
        return;

    OSEntity* Entity = (OSEntity*) PTR_FROM_INT(MtxHandle.V[0]);

    if (Write)
        pthread_rwlock_wrlock(&Entity->RWMutex);
    else
        pthread_rwlock_rdlock(&Entity->RWMutex);
}

void 
RWMutexUnlock(Handle MtxHandle, b32 Write)
{
    if (MtxHandle == EMPTY_HANDLE_VALUE)
        return;

    OSEntity* Entity = (OSEntity*) PTR_FROM_INT(MtxHandle.V[0]);

    pthread_rwlock_unlock(&Entity->RWMutex);
    UNUSED(Write);
}

Handle 
CondVarAlloc(void)
{
    OSEntity* Entity = EntityAlloc(ENTITY_KIND_CONDVAR);
    pthread_condattr_t Attributes;

    pthread_condattr_init(&Attributes);
    pthread_condattr_setclock(&Attributes, CLOCK_MONOTONIC);

    int CVInitResult = pthread_cond_init(
        &Entity->CondVar.CondVarHandle, 
        &Attributes
    );

    if (CVInitResult == -1) {
        EntityDestroy(Entity);
        Entity = NULL;
    }

    if (Entity) {
        // FIXME: the name RWMutexHandle is confusing when its not a RWMutex
        int RWInitResult = pthread_mutex_init(
            &Entity->CondVar.RWMutexHandle, 
            NULL
        );

        if (RWInitResult == -1) {
            pthread_cond_destroy(&Entity->CondVar.CondVarHandle);
            EntityDestroy(Entity);
            Entity = NULL;
        }
    }

    Handle Result = {
        INT_FROM_PTR(Entity)
    };

    return Result;
}

void 
CondVarDestroy(Handle CondHandle)
{
    if (CondHandle == EMPTY_HANDLE_VALUE)
        return;

    OSEntity* Entity = (OSEntity*) PTR_FROM_INT(CondHandle.V[0]);

    // NOTE: assuming cv uses mtx, destroy it first ???
    pthread_cond_destroy(&Entity->CondVar.CondVarHandle);
    pthread_mutex_destroy(&Entity->CondVar.RWMutexHandle);
    EntityDestroy(Entity);
}

b32 
CondVarWait(Handle CondHandle, Handle MtxHandle, u64 EndTimeUSecs)
{
    b32 Result = FALSE;

    if (
        CondHandle == EMPTY_HANDLE_VALUE ||
        MtxHandle == EMPTY_HANDLE_VALUE
    ) {
        return Result;
    }

    OSEntity* CVEntity = (OSEntity*) PTR_FROM_INT(CondHandle.V[0]);
    OSEntity* MtxEntity = (OSEntity*) PTR_FROM_INT(MtxHandle.V[0]);
    timespec EndTimeSpec = {};

    EndTimeSpec.tv_sec = EndTimeUSecs / MILLION(1);
    EndTimeSpec.tv_nsec = (
        THOUSAND(1) * 
        (
            EndTimeUSecs - (
                EndTimeUSecs / MILLION(1)
            ) * MILLION(1)
        )
    );

    int WaitResult = pthread_cond_timedwait(
        &CVEntity->CondVar.CondVarHandle,
        &MtxEntity->Mutex,
        &EndTimeSpec
    );

    Result = (WaitResult != ETIMEDOUT);

    return Result;
}

b32 
CondVarWaitR(Handle CondHandle, Handle RWMtxHandle, u64 EndTimeUSecs)
{
    b32 Result = FALSE;

    if (CondHandle == EMPTY_HANDLE_VALUE)
        return Result;

    if (RWMtxHandle == EMPTY_HANDLE_VALUE)
        return Result;

    OSEntity* CondEntity = (OSEntity*) PTR_FROM_INT(CondHandle.V[0]);
    OSEntity* RWMtxEntity = (OSEntity*) PTR_FROM_INT(RWMtxHandle.V[0]);
    int WaitResult;

    pthread_mutex_lock(&CondEntity->CondVar.RWMutexHandle);
    pthread_rwlock_unlock(&RWMtxEntity->RWMutex);

    if (EndTimeUSecs == U64_MAX) {
        WaitResult = pthread_cond_wait(
            &CondEntity->CondVar.CondVarHandle, 
            &CondEntity->CondVar.RWMutexHandle
        );
    } else {
        timespec TimeSpec = {};

        TimeSpec.tv_sec = EndTimeUSecs / MILLION(1);
        TimeSpec.tv_nsec = THOUSAND(1) * (
            EndTimeUSecs - (EndTimeUSecs / MILLION(1)) * MILLION(1)
        );
        WaitResult = pthread_cond_timedwait(
            &CondEntity->CondVar.CondVarHandle, 
            &CondEntity->CondVar.RWMutexHandle, 
            &TimeSpec
        );
    }

    Result = (WaitResult != ETIMEDOUT);
    pthread_mutex_unlock(&CondEntity->CondVar.RWMutexHandle);
    pthread_rwlock_rdlock(&RWMtxEntity->RWMutex);

    return Result;
}

b32 
CondVarWaitW(Handle CondHandle, Handle RWMtxHandle, u64 EndTimeUSecs)
{
    if (CondHandle == EMPTY_HANDLE_VALUE)
        return FALSE;

    if (RWMtxHandle == EMPTY_HANDLE_VALUE)
        return FALSE;

    OSEntity* CondEntity = (OSEntity*) PTR_FROM_INT(CondHandle.V[0]);
    OSEntity* RWMtxEntity = (OSEntity*) PTR_FROM_INT(RWMtxHandle.V[0]);
    int WaitResult;

    pthread_mutex_lock(&CondEntity->CondVar.RWMutexHandle);
    pthread_rwlock_unlock(&RWMtxEntity->RWMutex);

    if (EndTimeUSecs == U64_MAX) {
        WaitResult = pthread_cond_wait(
            &CondEntity->CondVar.CondVarHandle, 
            &CondEntity->CondVar.RWMutexHandle
        );
    } else {
        timespec TimeSpec = {};

        TimeSpec.tv_sec = EndTimeUSecs / MILLION(1);
        TimeSpec.tv_nsec = THOUSAND(1) * (EndTimeUSecs - (EndTimeUSecs / MILLION(1)) * MILLION(1));
        WaitResult = pthread_cond_timedwait(
            &CondEntity->CondVar.CondVarHandle, 
            &CondEntity->CondVar.RWMutexHandle, 
            &TimeSpec
        );
    }

    pthread_mutex_unlock(&CondEntity->CondVar.RWMutexHandle);
    pthread_rwlock_wrlock(&RWMtxEntity->RWMutex);

    return (WaitResult != ETIMEDOUT);
}

void 
CondVarSignal(Handle CondHandle)
{
    if (CondHandle == EMPTY_HANDLE_VALUE)
        return;

    OSEntity* Entity = (OSEntity*) PTR_FROM_INT(*CondHandle.V);

    pthread_cond_signal(&Entity->CondVar.CondVarHandle);
}

void 
CondVarBroadcast(Handle CondHandle)
{
    if (CondHandle == EMPTY_HANDLE_VALUE)
        return;

    OSEntity* Entity = (OSEntity*) PTR_FROM_INT(*CondHandle.V);

    pthread_cond_broadcast(&Entity->CondVar.CondVarHandle);
}

Handle 
SemaphoreAlloc(u32 InitialCount, u32 MaxCount, Str8 Name)
{
    Handle Result = {};

    if (Name.Size) {
        TempArena Scratch = GetScratch(NULL, 0);
        Str8 NameCopy = ArenaPushStrCpy(Scratch.MemPool, Name);
        sem_t* Sem = sem_open(
            (char*) NameCopy.Str, 
            O_CREAT | O_EXCL, 
            0666, 
            InitialCount
        );

        if (Sem == SEM_FAILED)
            if (errno == EEXIST)
                Sem = sem_open((char*) NameCopy.Str, 0);

        ReleaseScratch(Scratch);

        if (Sem != SEM_FAILED)
            Result.V[0] = INT_FROM_PTR(Sem);
    } else {
        sem_t* Sem = (sem_t*) mmap(
            NULL, 
            sizeof(sem_t), 
            PROT_READ | PROT_WRITE, 
            MAP_PRIVATE | MAP_ANONYMOUS, 
            -1, 
            0
        );

        ASSERT_ALWAYS(Sem != MAP_FAILED);

        int InitResult = sem_init(Sem, 0, InitialCount);

        if (InitResult == 0)
            *Result.V = INT_FROM_PTR(Sem);
    }

    return Result;
}

void 
SemaphoreDestroy(Handle SemHandle, Str8 Name)
{
    sem_t* Sem = (sem_t*) PTR_FROM_INT(SemHandle.V[0]);

    if (Name.Size) {
        TempArena Scratch = GetScratch(NULL, 0);
        Str8 NameCopy = ArenaPushStrCpy(Scratch.MemPool, Name);
        int UnlinkResult = sem_unlink((char*) NameCopy.Str);

        ASSERT_ALWAYS(!UnlinkResult || errno == ENOENT);

        ReleaseScratch(Scratch);
    } else {
        int DestroyResult = sem_destroy(Sem);

        ASSERT_ALWAYS(DestroyResult == 0);

        DestroyResult = munmap(Sem, sizeof(*Sem));

        ASSERT_ALWAYS(!DestroyResult);
    }
}

Handle 
SemaphoreOpen(Str8 Name)
{
    Handle Result = {};

    if (Name.Size) {
        TempArena Scratch = GetScratch(NULL, 0);
        Str8 NameCopy = ArenaPushStrCpy(Scratch.MemPool, Name);
        sem_t* Sem = sem_open((char*) NameCopy.Str, 0);

        if (Sem != SEM_FAILED)
            Result.V[0] = INT_FROM_PTR(Sem);

        ReleaseScratch(Scratch);
    } else {
        ASSERT_ALWAYS(!"Cannot open unnamed semaphore");
    }

    return Result;
}

void 
SemaphoreClose(Handle SemHandle)
{
    sem_t* Sem = (sem_t*) PTR_FROM_INT(SemHandle.V[0]);

    if (Sem != MAP_FAILED) {
        int CloseResult = sem_close(Sem);

        ASSERT_ALWAYS(!CloseResult);
    }
}

b32 
SemaphoreLock(Handle SemHandle, u64 EndTimeUSecs)
{
    ASSERT_ALWAYS(EndTimeUSecs == U64_MAX);

    for (;;) {
        int WaitResult = sem_wait(
            (sem_t*) PTR_FROM_INT(SemHandle.V[0])
        );

        if (!WaitResult) {
            break;
        } else {
            if (errno == EAGAIN)
                continue;
        }

        break;
    }

    return TRUE;
}

void 
SemaphoreUnlock(Handle SemHandle)
{
    for (;;) {
        int PostResult = sem_post(
            (sem_t*) PTR_FROM_INT(SemHandle.V[0])
        );

        if (PostResult == 0) {
            break;
        } else {
            if (errno == EAGAIN)
                continue;
        }

        break;
    }
}

Handle 
BarrierAlloc(u64 Count)
{
    OSEntity* Entity = EntityAlloc(ENTITY_KIND_BARRIER);

    if (Entity)
        pthread_barrier_init(&Entity->Barrier, NULL, Count);

    Handle Result = {
        INT_FROM_PTR(Entity)
    };

    return Result;
}

void 
BarrierRelease(Handle Barrier)
{
    OSEntity* Entity = (OSEntity*) PTR_FROM_INT(Barrier.V[0]);

    if (Entity) {
        pthread_barrier_destroy(&Entity->Barrier);
        EntityDestroy(Entity);
    }
}

void 
BarrierWait(Handle Barrier)
{
    OSEntity* Entity = (OSEntity*) PTR_FROM_INT(Barrier.V[0]);

    if (Entity)
        pthread_barrier_wait(&Entity->Barrier);
}

StripeArray 
StripeArrayAlloc(Arena* MemPool)
{
    StripeArray Result = {};

    Result.Count = GetSystemProperties()->LogicalProcessorCount;
    Result.V = ArenaPushArrayZero(
        MemPool,
        Stripe,
        Result.Count
    );

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
