#include "win32_platform.h"

Handle 
ProcessLaunch(ProcessLaunchProperties* Properties)
{
    Handle Result = {};
    TempArena Scratch = GetScratch(NULL, 0);
    Str8 Command = {};

    {
        Str8JoinPart JoinParts = {};

        JoinParts.Pre = Str8Lit("\"");
        JoinParts.Sep = Str8Lit("\" \"");
        JoinParts.Post = Str8Lit("\"");
        Command = StrListJoin(Scratch.MemPool, &Properties->CommandLineArgs, &JoinParts);
    }

    b32 NULLEnvironmentArg = FALSE;
    Str8 Environment = {};
    Str8JoinPart JoinParts = {};

    JoinParts.Sep = Str8Lit("\0");
    JoinParts.Post = Str8Lit("\0");

    Str8List AllOptions = Properties->Environment;

    if (Properties->InheritEnvironment) {
        if (AllOptions.Count) {
            MemSet(&AllOptions, 0, sizeof(Str8List));

            for (Str8Node* Node = Properties->Environment.Head; Node; Node = Node->Next)
                ListPush(Scratch.MemPool, &AllOptions, Node->String);

            for (Str8Node* Node = OS_STATE.ProcessProps.Environment.Head; Node; Node = Node->Next)
                ListPush(Scratch.MemPool, &AllOptions, Node->String);
        } else {
            NULLEnvironmentArg = TRUE;
        }
    }

    if (!NULLEnvironmentArg)
        Environment = StrListJoin(Scratch.MemPool, &AllOptions, &JoinParts);

    Str16 WideCommand = Str16FromStr8(Scratch.MemPool, Command);
    Str16 WidePath = Str16FromStr8(Scratch.MemPool, Properties->Path);
    Str16 WideEnvironment = {};

    if (!NULLEnvironmentArg)
        WideEnvironment = Str16FromStr8(Scratch.MemPool, Environment);

    DWORD CreationFlags = CREATE_UNICODE_ENVIRONMENT;

    if (Properties->HideConsole)
        CreationFlags |= CREATE_NO_WINDOW;

    BOOL InheritHandles = FALSE;
    STARTUPINFOW StartupInfo = {};

    StartupInfo.cb = sizeof(StartupInfo);

    if (Properties->StdOut != EMPTY_HANDLE_VALUE) {
        HANDLE StdOutHandle = (HANDLE) PTR_FROM_INT(*Properties->StdOut.V);

        StartupInfo.hStdOutput = StdOutHandle;
        StartupInfo.dwFlags |= STARTF_USESTDHANDLES;
        InheritHandles = TRUE;
    }

    if (Properties->StdIn != EMPTY_HANDLE_VALUE) {
        HANDLE StdInHandle = (HANDLE) PTR_FROM_INT(*Properties->StdIn.V);

        StartupInfo.hStdOutput = StdInHandle;
        StartupInfo.dwFlags |= STARTF_USESTDHANDLES;
        InheritHandles = TRUE;
    }

    if (Properties->StdErr != EMPTY_HANDLE_VALUE) {
        HANDLE StdErrHandle = (HANDLE) PTR_FROM_INT(*Properties->StdErr.V);

        StartupInfo.hStdOutput = StdErrHandle;
        StartupInfo.dwFlags |= STARTF_USESTDHANDLES;
        InheritHandles = TRUE;
    }

    PROCESS_INFORMATION ProcessInfo = {};

    if (
        CreateProcessW(
            NULL,
            (WCHAR*) WideCommand.Str,
            NULL,
            NULL,
            InheritHandles,
            CreationFlags,
            (NULLEnvironmentArg) ? NULL : (WCHAR*) WideEnvironment.Str,
            (WCHAR*) WidePath.Str,
            &StartupInfo,
            &ProcessInfo
        )
    ) {
        *Result.V = INT_FROM_PTR(ProcessInfo.hProcess);
        CloseHandle(ProcessInfo.hThread);
    }

    ReleaseScratch(Scratch);

    return Result;
}

b32 
ProcessJoin(Handle ProcHandle, u64 EndTimeUSecs)
{
    HANDLE Proc = (HANDLE) PTR_FROM_INT(*ProcHandle.V);
    DWORD SleepMSecs = SleepMSecsFromEndTimeUSecs(EndTimeUSecs);
    DWORD Result  = WaitForSingleObject(Proc, SleepMSecs);
    b32 ProcHasJoined = (Result == WAIT_OBJECT_0);

    if (ProcHasJoined)
        CloseHandle(Proc);

    return ProcHasJoined;
}

b32 
ProcessJoinExitCode(Handle ProcHandle, u64 EndTimeUSecs, OUT int* ExitCode)
{
    HANDLE Proc = (HANDLE) PTR_FROM_INT(*ProcHandle.V);
    DWORD SleepMSecs = SleepMSecsFromEndTimeUSecs(EndTimeUSecs);
    DWORD Result = WaitForSingleObject(Proc, SleepMSecs);
    b32 ProcHasJoined = (Result == WAIT_OBJECT_0);

    if (ProcHasJoined)
        CloseHandle(Proc);

    if (ProcHasJoined && ExitCode) {
        DWORD RetCode = 0;

        if (GetExitCodeProcess(Proc, &RetCode))
            *ExitCode = (int) RetCode;
    }

    return ProcHasJoined;
}

void 
ProcessDetach(Handle ProcHandle)
{
    HANDLE Proc = (HANDLE) PTR_FROM_INT(*ProcHandle.V);

    CloseHandle(Proc);
}

b32 
ProcessKill(Handle ProcHandle)
{
    HANDLE Proc = (HANDLE) PTR_FROM_INT(*ProcHandle.V);

    return (TerminateProcess(Proc, 999));
}
