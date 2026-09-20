#include "linux_platform.h"

struct OSEntity {
    OSEntity*               Next;
    EntityKind              Kind;

    union {
        struct {
            pthread_t       ThreadHandle;
            ThreadFuncType* Func;
            void*           Ptr;
        } Thread;

        pthread_mutex_t     Mutex;
        pthread_rwlock_t    RWMutex;

        struct {
            pthread_cond_t  CondVarHandle;
            pthread_mutex_t RWMutexHandle;
        } CondVar;

        pthread_barrier_t   Barrier;
    };
};

struct OSState {
    Arena*            MemPool;
    SystemProperties  SystemProps;
    ProcessProperties ProcessProps;
    pthread_mutex_t   EntityMutex;
    Arena*            EntityMemPool;
    OSEntity*         EntityFree;
};

OSState OS_STATE = {};

void 
SysInit(void)
{
    OS_STATE.EntityMemPool = ArenaAlloc();
    pthread_mutex_init(&OS_STATE.EntityMutex, NULL);
}

SystemProperties* 
GetSystemProperties(void)
{
    return &OS_STATE.SystemProps;
}

ProcessProperties* 
GetProcessProperties(void)
{
    return &OS_STATE.ProcessProps;
}

Str8 
GetCurrentPath(Arena* MemPool)
{
    char* ResultRaw = getcwd(NULL, 0);
    Str8 Result = ArenaPushStrCpy(MemPool, Str(ResultRaw));

    free(ResultRaw);

    return Result;
}

OSEntity* 
EntityAlloc(EntityKind Kind)
{
    OSEntity* Result = NULL;

    DEFER(
        pthread_mutex_lock(&OS_STATE.EntityMutex), 
        pthread_mutex_unlock(&OS_STATE.EntityMutex)
    ) {
        Result = OS_STATE.EntityFree;

        if (Result)
            SLL_STACK_POP(OS_STATE.EntityFree);
        else
            Result = ArenaPushArray(OS_STATE.EntityMemPool, OSEntity, 1);
    }

    MemSet(Result, 0, sizeof(*Result));
    Result->Kind = Kind;

    return Result;
}

void 
EntityDestroy(OSEntity* Entity)
{
    DEFER(
        pthread_mutex_lock(&OS_STATE.EntityMutex),
        pthread_mutex_unlock(&OS_STATE.EntityMutex)
    ) {
        SLL_STACK_PUSH(OS_STATE.EntityFree, Entity);
    }
}

Handle 
SharedMemoryAlloc(u64 Size, Str8 Name)
{
    TempArena Scratch = GetScratch(NULL, 0);
    Str8 NameCopy = ArenaPushStrCpy(Scratch.MemPool, Name);
    int ID = shm_open((char*) NameCopy.Str, O_RDWR | O_CREAT, 0666);

    ftruncate(ID, Size);

    Handle Result = {
        (u64) ID
    };

    ReleaseScratch(Scratch);

    return Result;
}

Handle 
SharedMemoryOpen(Str8 Name)
{
    TempArena Scratch = GetScratch(NULL, 0);
    Str8 NameCopy = ArenaPushStrCpy(Scratch.MemPool, Name);
    int ID = shm_open((char*) NameCopy.Str, O_RDWR, 0);
    Handle Result = {
        (u64) ID
    };

    ReleaseScratch(Scratch);

    return Result;
}

void 
SharedMemoryClose(Handle MemHandle)
{
    if (MemHandle == EMPTY_HANDLE_VALUE)
        return;

    int ID = (int) MemHandle.V[0];

    close(ID);
}

void* 
SharedMemoryViewOpen(Handle MemHandle, r1u64 Range)
{
    if (MemHandle == EMPTY_HANDLE_VALUE)
        return NULL;

    int ID = (int) MemHandle.V[0];
    void* Base = mmap(
        NULL, 
        Length(Range), 
        PROT_READ | PROT_WRITE, 
        MAP_SHARED, 
        ID, 
        Range.Min
    );

    if (Base == MAP_FAILED)
        Base = NULL;

    return Base;
}

void 
SharedMemoryViewClose(Handle MemHandle, void* Ptr, r1u64 Range)
{
    if (MemHandle == EMPTY_HANDLE_VALUE)
        return;

    munmap(Ptr, Length(Range));
    UNUSED(MemHandle);
}

WallClockTime 
TimeNow(void)
{
    timespec TimeSpec;
    WallClockTime Result;

    clock_gettime(CLOCK_MONOTONIC, &TimeSpec);
    Result = (
        TimeSpec.tv_sec * MILLION(1) +
        (
            TimeSpec.tv_nsec / THOUSAND(1)
        )
    );

    return Result;
}

u32 
SleepMSecsFromEndTimeUSecs(u64 EndTimeUSecs)
{}

void 
Abort(u32 Code)
{
    exit(Code);
}
