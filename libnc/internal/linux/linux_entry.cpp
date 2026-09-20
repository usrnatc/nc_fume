#include "linux_platform.h"

internal void
SignalHandler(int Signal, siginfo_t* SigInfo, void* Param)
{
    local volatile u32 First = 0;

    if (AtomicExchangeU32(&First, 1, 0))
        for (;;)
            sleep(U32_MAX);

    local void* IPs[4096] = {};
    int IPsCount = backtrace(IPs, ARRAY_COUNT(IPs));

    PrintErr(
        "A fatal signal was received: %s (%d). The process is terminating.\n",
        strsignal(Signal),
        Signal
    );
    PrintErr("Callstack:\n");

    for (u64 Index = 0; Index < IPsCount; ++Index) {
        Dl_info Info = {};

        dladdr(IPs[Index], &Info);

        char Command[2048] = {};

        snprintf(
            Command,
            sizeof(Command),
            "llvm-symbolizer-19 --relative-address -f -e %s %lu",
            Info.dli_fname,
            (unsigned long) IPs[Index] - (unsigned long) Info.dli_fbase
        );

        FILE* CommandFile = popen(Command, "r");

        if (CommandFile) {
            char FuncName[256] = {};
            char FileName[256] = {};

            if (
                fgets(FuncName, sizeof(FuncName), CommandFile) &&
                fgets(FileName, sizeof(FileName), CommandFile)
            ) {
                Str8 Func = CStr8(FuncName);

                if (Func.Size)
                    --Func.Size;

                Str8 Module = StrSkipLastSlash(Str8Lit(Info.dli_fname));
                Str8 File = StrSkipLastSlash(Str((u8*) FileName, (u8*) (FileName + sizeof(FileName))));

                if (File.Size)
                    --File.Size;

                b32 NoFunc = StrMatch(Func, "??"_s8, STR_MATCH_JUNK_AFTER);
                b32 NoFile = StrMatch(File, "??"_s8, STR_MATCH_JUNK_AFTER);

                if (NoFunc)
                    Func = ""_s8;

                if (NoFile)
                    File = ""_s8;

                PrintErr(
                    "%ld. [0x%016lX] %S%s%S %S\n",
                    Index + 1,
                    (unsigned long) IPs[Index],
                    PRINT_STR(Module),
                    (!NoFunc || !NoFile) ? ", " : "",
                    PRINT_STR(Func),
                    PRINT_STR(File)
                );
            }

            pclose(CommandFile);
        } else {
            PrintErr(
                "%ld. [0x%016lX] %s\n", 
                (unsigned long) IPs[Index], 
                Info.dli_fname
            );
        }
    }

    exit(0);
}

internal void
EntryPointCaller(int ArgC, char** ArgV)
{
    {
        stack_t SS = {};

        SS.ss_size = SIGSTKSZ;
        SS.ss_sp = mmap(
            NULL,
            SS.ss_size,
            PROT_READ | PROT_WRITE,
            MAP_PRIVATE | MAP_ANONYMOUS,
            -1,
            0
        );
        sigaltstack(&SS, NULL);
    }

    {
        struct sigaction Handler = {};

        Handler.sa_sigaction = SignalHandler;
        Handler.sa_flags = SA_SIGINFO | SA_ONSTACK;
        sigfillset(&Handler.sa_mask);
        sigaction(SIGILL, &Handler, NULL);
        sigaction(SIGTRAP, &Handler, NULL);
        sigaction(SIGABRT, &Handler, NULL);
        sigaction(SIGFPE, &Handler, NULL);
        sigaction(SIGBUS, &Handler, NULL);
        sigaction(SIGSEGV, &Handler, NULL);
        sigaction(SIGQUIT, &Handler, NULL);
    }

    {
        {
            SystemProperties* Properties = &OS_STATE.SystemProps;

            Properties->LogicalProcessorCount = (u64) get_nprocs();
            Properties->PageSize = (u32) getpagesize();
            Properties->LargePageSize = MB(2);
            Properties->AllocationGranularity = Properties->PageSize;
        }

        {
            ProcessProperties* Properties = &OS_STATE.ProcessProps;

            Properties->PID = (u32) getpid();
        }

        TimeInit();
        MemInit();
        ArenaInit();

        ThreadContext* TCTX = ThreadContextAlloc();
        
        ThreadContextSelect(TCTX);
        OS_STATE.MemPool = ArenaAlloc();
        OS_STATE.EntityMemPool = ArenaAlloc();
        pthread_mutex_init(&OS_STATE.EntityMutex, NULL);

        {
            TempArena Scratch = GetScratch(NULL, 0);
            SystemProperties* Properties = &OS_STATE.SystemProps;
            b32 NameResultOkay = FALSE;
            u8* Buffer = NULL;
            int Size = 0;

            for (i64 Capacity = 4096, R = 0; R < 4; Capacity *= 2, ++R) {
                ReleaseScratch(Scratch);
                Buffer = ArenaPushArrayZero(Scratch.MemPool, u8, Capacity);

                int GetHostNameResult = gethostname((char*) Buffer, Capacity);

                Size = StrLen(Buffer);

                if (!GetHostNameResult && Size < Capacity) {
                    NameResultOkay = TRUE;
                    break;
                }
            }

            if (NameResultOkay && Size) {
                Properties->MachineName.Size = Size;
                Properties->MachineName.Str = ArenaPushArray(
                    OS_STATE.MemPool,
                    u8,
                    Properties->MachineName.Size + 1
                );
                MemCpy(
                    Properties->MachineName.Str, 
                    Buffer, 
                    Properties->MachineName.Size
                );
                Properties->MachineName.Str[Properties->MachineName.Size] = 0;
            }

            ReleaseScratch(Scratch);
        }

        {
            TempArena Scratch = GetScratch(NULL, 0);
            ProcessProperties* Properties = &OS_STATE.ProcessProps;

            {
                b32 ExePathOkay = FALSE;
                u8* Buffer = NULL;
                int Size = 0;

                for (
                    i64 Capacity = PATH_MAX, R = 0;
                    R < 4;
                    Capacity *= 2, ++R
                ) {
                    ReleaseScratch(Scratch);
                    Buffer = ArenaPushArray(
                        Scratch.MemPool,
                        u8,
                        Capacity
                    );
                    Size = readlink(
                        "/proc/self/exe", 
                        (char*) Buffer, 
                        Capacity
                    );

                    if (Size < Capacity) {
                        ExePathOkay = TRUE;
                        break;
                    }
                }

                if (ExePathOkay && Size) {
                    Str8 FullExePath = Str(Buffer, Size);
                    Str8 ChoppedExePath = StrChopLastSlash(FullExePath);

                    Properties->BinaryPath = ArenaPushStrCpy(
                        OS_STATE.MemPool,
                        ChoppedExePath
                    );
                }
            }

            {
                Properties->InitialPath = GetCurrentPath(OS_STATE.MemPool);
            }

            {
                char* HomeDir = getenv("HOME");

                Properties->UserProgramDataPath = Str(HomeDir);
            }

            ReleaseScratch(Scratch);
        }
    }

    MainThreadBaseEntryPoint(ArgC, ArgV);
}

int
main(int ArgC, char** ArgV)
{
    PrintInit();
    EntryPointCaller(ArgC, ArgV);
    PrintRelease();

    return 0;
}
