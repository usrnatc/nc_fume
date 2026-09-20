#include "linux_platform.h"

Handle
ProcessLaunch(ProcessLaunchProperties* Properties)
{
    Handle Result = {};
    posix_spawn_file_actions_t FileActions = {};
    int FileActionsInitCode = posix_spawn_file_actions_init(&FileActions);
    
    if (!FileActionsInitCode) {
        int StdOutCode = posix_spawn_file_actions_adddup2(
            &FileActions,
            (int) Properties->StdOut.V[0],
            STDOUT_FILENO
        );

        ASSERT(!StdOutCode);

        int StdErrCode = posix_spawn_file_actions_adddup2(
            &FileActions,
            (int) Properties->StdErr.V[0],
            STDERR_FILENO
        );

        ASSERT(!StdErrCode);

        int StdInCode = posix_spawn_file_actions_adddup2(
            &FileActions,
            (int) Properties->StdIn.V[0],
            STDIN_FILENO
        );

        ASSERT(!StdInCode);

        posix_spawnattr_t Attributes = {};
        int AttributesInitCode = posix_spawnattr_init(&Attributes);

        if (!AttributesInitCode) {
            TempArena Scratch = GetScratch(NULL, 0);
            char** ArgV = ArenaPushArrayZero(
                Scratch.MemPool,
                char*,
                Properties->CommandLineArgs.Count + 1
            );

            {
                Str8List L = StrSplitPath(
                    Scratch.MemPool, 
                    Properties->Path
                );

                ListPush(
                    Scratch.MemPool, 
                    &L, 
                    Properties->CommandLineArgs.Head->String
                );

                Str8 PathToExe = StrPathListJoinByStyle(
                    Scratch.MemPool,
                    &L,
                    PATH_STYLE_UNIX_ABSOLUTE
                );

                ArgV[0] = (char*) PathToExe.Str;

                u64 ArgIndex = 1;

                for (
                    Str8Node* Node = Properties->CommandLineArgs.Head->Next;
                    Node;
                    Node = Node->Next
                ) {
                    ArgV[ArgIndex++] = (char*) Node->String.Str;
                }
            }

            char** EnvP = NULL;

            if (Properties->InheritEnvironment) {
                EnvP = __environ;
            } else {
                EnvP = ArenaPushArrayZero(
                    Scratch.MemPool,
                    char*,
                    Properties->Environment.Count + 2
                );

                u64 EnvIndex = 0;

                for (
                    Str8Node* Node = Properties->CommandLineArgs.Head;
                    Node;
                    Node = Node->Next
                ) {
                    EnvP[EnvIndex] = (char*) Node->String.Str;
                }
            }

            if (Properties->DebugSubProcesses)
                UNREACHABLE();

            if (!Properties->HideConsole)
                UNREACHABLE();

            pid_t PID = 0;
            int SpawnCode = posix_spawn(
                &PID,
                ArgV[0],
                &FileActions,
                &Attributes,
                ArgV,
                EnvP
            );

            if (!SpawnCode)
                Result.V[0] = (u64) PID;

            int AttributeDestroyCode = posix_spawnattr_destroy(&Attributes);

            ASSERT(!AttributeDestroyCode);

            ReleaseScratch(Scratch);
        }

        int FileActionsDestroyCode = posix_spawn_file_actions_destroy(&FileActions);

        ASSERT(!FileActionsDestroyCode);
    }

    return Result;
}

b32
ProcessJoin(Handle ProcHandle, u64 EndTimeUSecs)
{
    pid_t PID = (pid_t) ProcHandle.V[0];
    b32 Result = FALSE;

    if (!EndTimeUSecs) {
        if (kill(PID, 0) >= 0) {
            Result = (errno == ENOENT);

            if (Result) {
                int Stat;

                waitpid(PID, &Stat, 0);
            }
        } else {
            ASSERT(FALSE && "Failed to get status from PID");
        }
    } else if (EndTimeUSecs == U64_MAX) {
        for (;;) {
            int Stat = 0;
            int W = waitpid(PID, &Stat, 0);

            if (W == -1)
                break;

            if (
                WIFEXITED(Stat) ||
                WIFSTOPPED(Stat) ||
                WIFSIGNALED(Stat)
            ) {
                Result = TRUE;
                break;
            }
        }
    } else {
        // TODO(nathan): join and wait for EndTimeUSecs
    }

    return Result;
}

b32 
ProcessJoinExitCode(Handle ProcHandle, u64 EndTimeUSecs, int *ExitCode)
{}

void
ProcessDetach(Handle ProcHandle)
{
    UNUSED(ProcHandle);
}

b32
ProcessKill(Handle ProcHandle)
{
    int ErrorCode = kill((pid_t) ProcHandle.V[0], SIGKILL);
    b32 Result = !ErrorCode;

    return Result;
}
