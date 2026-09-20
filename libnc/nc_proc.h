#if !defined(__NC_PROC_H__)
#define __NC_PROC_H__

#include "nc_types.h"
#include "nc_string.h"
#include "nc_system.h"

// @types______________________________________________________________________
struct ProcessLaunchProperties {
    Str8List CommandLineArgs;
    Str8List Environment;
    Str8 Path;
    Handle StdOut;
    Handle StdIn;
    Handle StdErr;
    b32 InheritEnvironment;
    b32 DebugSubProcesses;
    b32 HideConsole;
};

// @functions__________________________________________________________________
Str8List StringListFromCommandLineArgs(Arena* MemPool, int ArgC, char** ArgV);
Handle ProcessLaunch(ProcessLaunchProperties* Properties);
b32 ProcessJoin(Handle ProcHandle, u64 EndTimeUSecs);
b32 ProcessJoinExitCode(Handle ProcHandle, u64 EndTimeUSecs, OUT int* ExitCode);
void ProcessDetach(Handle ProcHandle);
b32 ProcessKill(Handle ProcHandle);

#endif // __NC_PROC_H__
