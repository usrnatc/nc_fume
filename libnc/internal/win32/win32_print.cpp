#include "win32_platform.h"

u32 PRINT_STDIN_FILENO = ((DWORD) -10); 
u32 PRINT_STDOUT_FILENO = ((DWORD) -11);
u32 PRINT_STDERR_FILENO = ((DWORD) -12);
IOHandles IO_HANDLES = {};

void
PrintInit(void)
{
#if defined(NC_GUI)
    if (!AllocConsole()) {
        return;
    }
#endif

    HANDLE Out = GetStdHandle(PRINT_STDOUT_FILENO);
    HANDLE Err = GetStdHandle(PRINT_STDERR_FILENO);
    HANDLE In = GetStdHandle(PRINT_STDIN_FILENO);
    DWORD OutMode = 0;
    DWORD ErrMode = 0;
    DWORD InMode = 0;
    DWORD OutConsoleFlags = ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    DWORD InConsoleFlags = (
        ENABLE_INSERT_MODE |
        ENABLE_LINE_INPUT |
        ENABLE_MOUSE_INPUT |
        ENABLE_PROCESSED_INPUT |
        ENABLE_QUICK_EDIT_MODE |
        ENABLE_EXTENDED_FLAGS
    );

    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    GetConsoleMode(In, &InMode);
    SetConsoleMode(In, InMode | InConsoleFlags);

#if defined(NC_GUI)
    GetConsoleMode(Out, &OutMode);
    SetConsoleMode(Out, OutMode | OutConsoleFlags);
    GetConsoleMode(Err, &ErrMode);
    SetConsoleMode(Err, ErrMode | OutConsoleFlags);
#endif

    IO_HANDLES.Out = HandleFromHANDLE(Out);
    IO_HANDLES.Err = HandleFromHANDLE(Err);
    IO_HANDLES.In = HandleFromHANDLE(In);
}

void
PrintRelease(void)
{
#if defined(NC_GUI)
    FreeConsole();
#endif
}

void 
PrintOut(Str8 Msg)
{
    ASSERT_ALWAYS(IO_HANDLES.Out != EMPTY_HANDLE_VALUE);

    HANDLE Out = (HANDLE) PTR_FROM_INT(*IO_HANDLES.Out.V);

    WriteFile(Out, Msg.Str, Msg.Size, NULL, NULL);
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
    ASSERT_ALWAYS(IO_HANDLES.Err != EMPTY_HANDLE_VALUE);

    HANDLE Err = (HANDLE) PTR_FROM_INT(*IO_HANDLES.Err.V);

    WriteFile(Err, Msg.Str, Msg.Size, NULL, NULL);
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
UserIn(Arena* MemPool, Str8 Prompt)
{
    ASSERT_ALWAYS(IO_HANDLES.In != EMPTY_HANDLE_VALUE);

    HANDLE In = (HANDLE) PTR_FROM_INT(*IO_HANDLES.In.V);

    if (Prompt.Size)
        PrintOut("%S", PRINT_STR(Prompt));

    u8 Buffer[U16_MAX];
    DWORD BytesRead = 0;

    ReadConsoleA(In, Buffer, sizeof(Buffer), &BytesRead, NULL);

    Str8 Result = ArenaPushStrFmt(                                              // NOTE: do not store \r\n, maybe add a flag to keep these if needed
        MemPool, 
        "%.*s", 
        (int) (BytesRead - 2), 
        Buffer
    );

    return Result;
}

Str8
UserIn(Arena* MemPool)
{
    return UserIn(MemPool, {});
}

void 
SetStdOut(i32 NewStdOut)
{
    PRINT_STDOUT_FILENO = NewStdOut;
}

void 
SetStdErr(i32 NewStdErr)
{
    PRINT_STDERR_FILENO = NewStdErr;
}

void 
SetStdIn(i32 NewStdIn)
{
    PRINT_STDIN_FILENO = NewStdIn;
}

Handle 
GetStdOut(void)
{
    if (IO_HANDLES.Out != EMPTY_HANDLE_VALUE)
        return IO_HANDLES.Out;

    return HandleFromHANDLE(GetStdHandle(PRINT_STDOUT_FILENO));
}

Handle 
GetStdErr(void)
{
    if (IO_HANDLES.Err != EMPTY_HANDLE_VALUE)
        return IO_HANDLES.Err;

    return HandleFromHANDLE(GetStdHandle(PRINT_STDERR_FILENO));
}

Handle 
GetStdIn(void)
{
    if (IO_HANDLES.In != EMPTY_HANDLE_VALUE)
        return IO_HANDLES.In;

    return HandleFromHANDLE(GetStdHandle(PRINT_STDIN_FILENO));
}
