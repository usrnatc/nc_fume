#include "win32_platform.h"

struct OSEntity {
    OSEntity*                   Next;
    EntityKind                  Kind;

    union {
        struct {
            ThreadFuncType*     Func;
            void*               Ptr;
            HANDLE              ThreadHandle;
            DWORD               ThreadID;
        } Thread;

        CRITICAL_SECTION        Mutex;
        SRWLOCK                 RWMutex;
        CONDITION_VARIABLE      CondVar;
        SYNCHRONIZATION_BARRIER Barrier;
    };
};

struct OSState {
    Arena*            MemPool;
    SystemProperties  SystemProps;
    ProcessProperties ProcessProps;
    u64               USecsResolution;
    CRITICAL_SECTION  EntityMutex;
    Arena*            EntityMemPool;
    OSEntity*         EntityFree;
};

OSState OS_STATE = {};

void 
SysInit(void)
{
    OS_STATE.EntityMemPool = ArenaAlloc();
    InitializeCriticalSection(&OS_STATE.EntityMutex);
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
    TempArena Scratch = GetScratch(&MemPool, 1);
    DWORD ResultLength = GetCurrentDirectoryW(0, NULL);
    u16* Memory = ArenaPushArrayZero(Scratch.MemPool, u16, ResultLength + 1);

    ResultLength = GetCurrentDirectoryW(ResultLength + 1, (WCHAR*) Memory);

    Str8 Result = Str8FromStr16(MemPool, Str(Memory, ResultLength));

    ReleaseScratch(Scratch);

    return Result;
}

OSEntity*
EntityAlloc(EntityKind Kind)
{
    OSEntity* Result = NULL;

    EnterCriticalSection(&OS_STATE.EntityMutex);
    {
        Result = OS_STATE.EntityFree;

        if (Result)
            SLL_STACK_POP(OS_STATE.EntityFree);
        else
            Result = ArenaPushArrayZero(OS_STATE.EntityMemPool, OSEntity, 1);

        MemSet(Result, 0, sizeof(*Result));
    }
    LeaveCriticalSection(&OS_STATE.EntityMutex);

    Result->Kind = Kind;

    return Result;
}

void 
EntityDestroy(OSEntity* Entity)
{
    Entity->Kind = ENTITY_KIND_NULL;

    EnterCriticalSection(&OS_STATE.EntityMutex);
    {
        SLL_STACK_PUSH(OS_STATE.EntityFree, Entity);
    }
    LeaveCriticalSection(&OS_STATE.EntityMutex);
}

Handle 
SharedMemoryAlloc(u64 Size, Str8 Name)
{
    Handle Result = {};
    TempArena Scratch = GetScratch(NULL, 0);
    Str16 WideName = Str16FromStr8(Scratch.MemPool, Name);
    HANDLE FileHandle = CreateFileMappingW(
        INVALID_HANDLE_VALUE,
        NULL,
        PAGE_READWRITE,
        (u32) ((Size & 0xFFFFFFFF00000000) >> 32),
        (u32) ((Size & 0x00000000FFFFFFFF)),
        (WCHAR*) WideName.Str
    );

    *Result.V = INT_FROM_PTR(FileHandle);
    ReleaseScratch(Scratch);

    return Result;
}

Handle 
SharedMemoryOpen(Str8 Name)
{
    Handle Result = {};
    TempArena Scratch = GetScratch(NULL, 0);
    Str16 WideName = Str16FromStr8(Scratch.MemPool, Name);
    HANDLE FileHandle = OpenFileMappingW(
        FILE_MAP_ALL_ACCESS,
        FALSE,
        (WCHAR*) WideName.Str
    );

    *Result.V = INT_FROM_PTR(FileHandle);
    ReleaseScratch(Scratch);

    return Result;
}

void 
SharedMemoryClose(Handle MemHandle)
{
    HANDLE FileHandle = (HANDLE) *MemHandle.V;

    CloseHandle(FileHandle);
}

void* 
SharedMemoryViewOpen(Handle MemHandle, r1u64 Range)
{
    HANDLE FileHandle = (HANDLE) *MemHandle.V;
    u64 Offset = Range.Min;
    u64 Size = Length(Range);
    void* Result = MapViewOfFile(
        FileHandle,
        FILE_MAP_ALL_ACCESS,
        (u32) ((Offset & 0xFFFFFFFF00000000) >> 32),
        (u32) ((Offset & 0x00000000FFFFFFFF)),
        Size
    );

    return Result;
}

void 
SharedMemoryViewClose(Handle MemHandle, void* Ptr, r1u64 Range)
{
    UNUSED(MemHandle);
    UNUSED(Range);

    UnmapViewOfFile(Ptr);
}

u64 
GetUSecsResolution(void)
{
    return OS_STATE.USecsResolution;
}

WallClockTime
TimeNow(void)
{
    u64 Result = 0;
    LARGE_INTEGER Counter;

    if (QueryPerformanceCounter(&Counter))
        Result = (Counter.QuadPart * MILLION(1)) / OS_STATE.USecsResolution;

    return Result;
}

u32 
SleepMSecsFromEndTimeUSecs(u64 EndTimeUSecs)
{
    u32 SleepMSecs = 0;

    if (EndTimeUSecs == U64_MAX) {
        SleepMSecs = INFINITE;
    } else {
        u64 BeginTimeUSecs = ((u64) TimeNow() * MILLION(1)) / GetUSecsResolution();

        if (BeginTimeUSecs < EndTimeUSecs) {
            u64 SleepUSecs = EndTimeUSecs - BeginTimeUSecs;

            SleepMSecs = (u32) ((SleepUSecs + 999) / THOUSAND(1));
        }
    }

    return SleepMSecs;
}

void 
Abort(u32 Code)
{
    ExitProcess(Code);
}
