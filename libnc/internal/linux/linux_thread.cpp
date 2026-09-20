#include "linux_platform.h"

void*
LinuxThreadEntryFunc(void* Ptr)
{
    GetTLS()->ARENA_THREAD_SCRATCH_INIT = FALSE;

    OSEntity* Entity = (OSEntity*) Ptr;
    ThreadFuncType* Func = Entity->Thread.Func;
    void* ThreadPtr = Entity->Thread.Ptr;
    ThreadContext* Ctx = ThreadContextAlloc();

    ThreadContextSelect(Ctx);
    Func(ThreadPtr);
    ThreadContextRelease(Ctx);

    return NULL;
}

u32
ThreadGetID(void)
{
    u32 Result = (u32) pthread_self();

    return Result;
}

void
ThreadSetName(Str8 Name)
{
    TempArena Scratch = GetScratch(NULL, 0);
    Str8 NameCopy = ArenaPushStrCpy(Scratch.MemPool, Name);
    pthread_t CurrentThread = pthread_self();

    pthread_setname_np(CurrentThread, (char*) NameCopy.Str);
}

void 
ThreadSetName(char* Fmt, ...)
{
    TempArena Scratch = GetScratch(NULL, 0);
    va_list Args;

    va_start(Args, Fmt);

    Str8 String = ArenaPushStrFmtV(Scratch.MemPool, Fmt, Args);

    va_end(Args);

    pthread_t CurrentThread = pthread_self();

    pthread_setname_np(CurrentThread, (char*) String.Str);
    ReleaseScratch(Scratch);
}

Handle
ThreadLaunch(ThreadFuncType *Func, void *Params)
{
    OSEntity* Entity = EntityAlloc(ENTITY_KIND_THREAD);

    Entity->Thread.Func = Func;
    Entity->Thread.Ptr = Params;

    {
        int PThreadResult = pthread_create(
            &Entity->Thread.ThreadHandle,
            NULL,
            LinuxThreadEntryFunc,
            Entity
        );

        if (PThreadResult == -1) {
            EntityDestroy(Entity);
            Entity = NULL;
        }
    }

    Handle Result = {
        INT_FROM_PTR(Entity)
    };

    return Result;
}

b32
ThreadJoin(Handle ThreadHandle, u64 EndTimeUSecs)
{
    b32 Result = FALSE;

    if (MemIsZeroStruct(&ThreadHandle))
        return Result;

    OSEntity* Entity = (OSEntity*) PTR_FROM_INT(ThreadHandle.V[0]);
    int JoinResult = pthread_join(Entity->Thread.ThreadHandle, NULL);

    Result = (!JoinResult);

    EntityDestroy(Entity);

    return Result;
}

void
ThreadDetach(Handle ThreadHandle)
{
    if (MemIsZeroStruct(&ThreadHandle))
        return;

    OSEntity* Entity = (OSEntity*) PTR_FROM_INT(ThreadHandle.V[0]);

    EntityDestroy(Entity);
}
