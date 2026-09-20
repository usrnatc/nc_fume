#include "win32_platform.h"

DWORD
WinThreadEntryFunc(void* Ptr)
{
    ThreadLocalStorage* TLS = GetTLS();

    TLS->ARENA_THREAD_SCRATCH_INIT = FALSE;

    OSEntity* Entity = (OSEntity*) Ptr;
    ThreadFuncType* Func = Entity->Thread.Func;
    void* ThreadPtr = Entity->Thread.Ptr;
    ThreadContext* Ctx = ThreadContextAlloc();

    ThreadContextSelect(Ctx);
    Func(ThreadPtr);
    ThreadContextRelease(Ctx);

    return 0;
}

u32 
ThreadGetID(void)
{
    u32 Result;

    #if defined(NC_CPU_X86_32)
        Result = ((u32*) __readfsdword(24))[9];
    #elif defined(NC_CPU_X86_64)
        Result = ((u32*) __readgsqword(48))[18];
    #else
        Result = GetCurrentThreadId();
    #endif

    return Result;
}

void 
ThreadSetName(Str8 Name)
{
    TempArena Scratch = GetScratch(NULL, 0);

    if (WinSetThreadDescriptionFunc) {
        Str16 WideName = Str16FromStr8(Scratch.MemPool, Name);
        HRESULT Result = WinSetThreadDescriptionFunc(GetCurrentThread(), (WCHAR*) WideName.Str);
    }

    {
        Str8 NameCopy = ArenaPushStrCpy(Scratch.MemPool, Name);

    #pragma pack(push, 8)
        struct THREADNAME_INFO {
            u32 dwType;
            char* szName;
            u32 dwThreadID;
            u32 dwFlags;
        };
    #pragma pack(pop)

        THREADNAME_INFO Info = {};

        Info.dwType = 0x1000;
        Info.szName = (char*) NameCopy.Str;
        Info.dwThreadID = ThreadGetID();
        Info.dwFlags = 0;

    #pragma warning(push)
    #pragma warning(disable: 6320 6322)

        __try {
            RaiseException(0x406D1388, 0, sizeof(Info) / sizeof(void*), (const ULONG_PTR*) &Info);
        } __except (EXCEPTION_EXECUTE_HANDLER) {}

    #pragma warning(pop)
    }

    ReleaseScratch(Scratch);
}

void 
ThreadSetName(char* Fmt, ...)
{
    TempArena Scratch = GetScratch(NULL, 0);
    va_list Args;

    va_start(Args, Fmt);

    Str8 String = ArenaPushStrFmtV(Scratch.MemPool, Fmt, Args);

    va_end(Args);
    ThreadSetName(String);
    ReleaseScratch(Scratch);
}

Handle 
ThreadLaunch(ThreadFuncType* Func, void* Ptr, void* Params)
{
    OSEntity* Entity = EntityAlloc(ENTITY_KIND_THREAD);

    Entity->Thread.Func = Func;
    Entity->Thread.Ptr = Ptr;
    Entity->Thread.ThreadHandle = CreateThread(NULL, 0, WinThreadEntryFunc, Entity, 0, &Entity->Thread.ThreadID);

    Handle Result = {};

    *Result.V = INT_FROM_PTR(Entity);

    return Result;
}

Handle 
ThreadLaunch(ThreadFuncType* Func, void* Params)
{
    OSEntity* Entity = EntityAlloc(ENTITY_KIND_THREAD);

    Entity->Thread.Func = Func;
    Entity->Thread.Ptr = Params;
    Entity->Thread.ThreadHandle = CreateThread(
        0,
        0,
        WinThreadEntryFunc,
        Entity,
        0,
        &Entity->Thread.ThreadID
    );

    Handle Result = {};

    *Result.V = INT_FROM_PTR(Entity);

    return Result;
}

b32 
ThreadJoin(Handle ThreadHandle, u64 EndTimeUSecs)
{
    u32 SleepMSecs = SleepMSecsFromEndTimeUSecs(EndTimeUSecs);
    OSEntity* Entity = (OSEntity*) PTR_FROM_INT(*ThreadHandle.V);
    DWORD WaitResult = WAIT_OBJECT_0;

    if (Entity) {
        WaitResult = WaitForSingleObject(Entity->Thread.ThreadHandle, SleepMSecs);
        CloseHandle(Entity->Thread.ThreadHandle);
        EntityDestroy(Entity);
    }

    return (WaitResult == WAIT_OBJECT_0);
}

void 
ThreadDetach(Handle ThreadHandle)
{
    OSEntity* Entity = (OSEntity*) *ThreadHandle.V;

    if (Entity) {
        CloseHandle(Entity->Thread.ThreadHandle);
        EntityDestroy(Entity);
    }
}
