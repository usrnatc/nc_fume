#include "linux_platform.h"

u32 PRINT_STDIN_FILENO = 0;
u32 PRINT_STDOUT_FILENO = 1;
u32 PRINT_STDERR_FILENO = 2;

void
PrintInit(void)
{}

void
PrintRelease(void)
{}

void
PrintOut(Str8 Msg)
{
    write(PRINT_STDOUT_FILENO, (char*) Msg.Str, Msg.Size);
}

void 
PrintOut(char* Fmt, ...)
{
    TempArena Scratch = GetScratch(NULL, 0);
    va_list Args;

    va_start(Args, Fmt);

    Str8 String = ArenaPushStrFmtV(Scratch.MemPool, Fmt, Args);

    PrintOut(String);
    va_end(Args);
    ReleaseScratch(Scratch);
}

void
PrintErr(Str8 Msg)
{
    write(PRINT_STDERR_FILENO, (char*) Msg.Str, Msg.Size);
}

Str8
UserIn(Arena* MemPool, Str8 Prompt)
{
    Str8 Result = {};

    return Result;
}

void 
PrintErr(char* Fmt, ...)
{
    TempArena Scratch = GetScratch(NULL, 0);
    va_list Args;

    va_start(Args, Fmt);

    Str8 String = ArenaPushStrFmtV(Scratch.MemPool, Fmt, Args);

    PrintErr(String);
    va_end(Args);
    ReleaseScratch(Scratch);
}

Str8
UserIn(Arena* MemPool)
{
    return UserIn(MemPool, {});
}

void 
SetStdOut(i32 NewStdOut)
{
    PRINT_STDOUT_FILENO = (u32) NewStdOut;
}

void 
SetStdErr(i32 NewStdErr)
{
    PRINT_STDERR_FILENO = (u32) NewStdErr;
}

void 
SetStdIn(i32 NewStdIn)
{
    PRINT_STDIN_FILENO = (u32) NewStdIn;
}

Handle 
GetStdOut(void)
{
    Handle Result = {
        (u64) PRINT_STDOUT_FILENO
    };

    return Result;
}

Handle 
GetStdErr(void)
{
    Handle Result = {
        (u64) PRINT_STDERR_FILENO
    };

    return Result;
}

Handle 
GetStdIn(void)
{
    Handle Result = {
        (u64) PRINT_STDIN_FILENO
    };

    return Result;
}
