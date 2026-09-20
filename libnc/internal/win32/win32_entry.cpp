#include "win32_platform.h"

internal LONG WINAPI
ExceptionFilter(PEXCEPTION_POINTERS ExceptionPtrs)
{
    local volatile LONG First = 0;

    if (_InterlockedCompareExchange(&First, 1, 0) != 0) {
        for (;;)
            Sleep(1000);
    }

    WCHAR Buffer[4096] = {};
    int BufferLength = 0;
    DWORD ExceptionCode = ExceptionPtrs->ExceptionRecord->ExceptionCode;

    BufferLength += wnsprintfW(
        Buffer + BufferLength, 
        ARRAY_COUNT(Buffer) - BufferLength, 
        TEXT("A fatal exception (code 0x%X) occured. The process is terminating.\n"),
        ExceptionCode
    );

    BOOL (WINAPI *DBGMiniDumpWriteDump)(
        HANDLE hProcess, 
        DWORD ProcessId, 
        HANDLE hFile, 
        MINIDUMP_TYPE DumpType, 
        PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam, 
        PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam, 
        PMINIDUMP_CALLBACK_INFORMATION CallbackParam
    ) = NULL;

    HMODULE DBGHelp = LoadLibraryA("dbghelp.dll");

    if (DBGHelp) {
        DWORD (WINAPI *DBGSymSetOptions)(DWORD SymOptions);
        BOOL (WINAPI *DBGSymInitializeW)(HANDLE hProcess, PCWSTR UserSearchPath, BOOL fInvadeProcess);
        BOOL (WINAPI *DBGStackWalk64)(DWORD MachineType, HANDLE hProcess, HANDLE hThread, LPSTACKFRAME64 StackFrame, PVOID ContextRecord, PREAD_PROCESS_MEMORY_ROUTINE64 ReadMemoryRoutine, PFUNCTION_TABLE_ACCESS_ROUTINE64 FunctionTableAccessRoutine, PGET_MODULE_BASE_ROUTINE64 GetModuleBaseRoutine, PTRANSLATE_ADDRESS_ROUTINE64 TranslateAddress);
        PVOID (WINAPI *DBGSymFunctionTableAccess64)(HANDLE hProcess, DWORD64 AddrBase);
        DWORD64 (WINAPI *DBGSymGetModuleBase64)(HANDLE hProcess, DWORD64 qwAddr);
        BOOL (WINAPI *DBGSymFromAddrW)(HANDLE hProcess, DWORD64 Address, PDWORD64 Displacement, PSYMBOL_INFOW Symbol);
        BOOL (WINAPI *DBGSymGetLineFromAddrW64)(HANDLE hProcess, DWORD64 dwAddr, PDWORD pdwDisplacement, PIMAGEHLP_LINEW64 Line);
        BOOL (WINAPI *DBGSymGetModuleInfoW64)(HANDLE hProcess, DWORD64 qwAddr, PIMAGEHLP_MODULEW64 ModuleInfo);

        *(FARPROC*) &DBGSymSetOptions = GetProcAddress(DBGHelp, "SymSetOptions");
        *(FARPROC*) &DBGSymInitializeW = GetProcAddress(DBGHelp, "SymInitializeW");
        *(FARPROC*) &DBGStackWalk64 = GetProcAddress(DBGHelp, "StackWalk64");
        *(FARPROC*) &DBGSymFunctionTableAccess64 = GetProcAddress(DBGHelp, "SymFunctionTableAccess64");
        *(FARPROC*) &DBGSymGetModuleBase64 = GetProcAddress(DBGHelp, "SymGetModuleBase64");
        *(FARPROC*) &DBGSymFromAddrW = GetProcAddress(DBGHelp, "SymFromAddrW");
        *(FARPROC*) &DBGSymGetLineFromAddrW64 = GetProcAddress(DBGHelp, "SymGetLineFromAddrW64");
        *(FARPROC*) &DBGSymGetModuleInfoW64 = GetProcAddress(DBGHelp, "SymGetModuleInfoW64");
        *(FARPROC*) &DBGMiniDumpWriteDump = GetProcAddress(DBGHelp, "MiniDumpWriteDump");

        if (
            DBGSymSetOptions &&
            DBGSymInitializeW &&
            DBGStackWalk64 &&
            DBGSymFunctionTableAccess64 &&
            DBGSymGetModuleBase64 &&
            DBGSymFromAddrW &&
            DBGSymGetLineFromAddrW64 &&
            DBGSymGetModuleInfoW64
        ) {
            HANDLE ThisProcess = GetCurrentProcess();
            HANDLE ThisThread = GetCurrentThread();
            CONTEXT* Context = ExceptionPtrs->ContextRecord;
            WCHAR ModulePath[MAX_PATH];

            GetModuleFileNameW(NULL, ModulePath, ARRAY_COUNT(ModulePath));
            PathRemoveFileSpecW(ModulePath);
            DBGSymSetOptions(SYMOPT_EXACT_SYMBOLS | SYMOPT_FAIL_CRITICAL_ERRORS | SYMOPT_LOAD_LINES | SYMOPT_UNDNAME);

            if (DBGSymInitializeW(ThisProcess, ModulePath, TRUE)) {
                b32 OurPDBIsValid = FALSE;

                {
                    IMAGEHLP_MODULEW64 Module = {};

                    Module.SizeOfStruct = sizeof(Module);

                    if (DBGSymGetModuleInfoW64(ThisProcess, (DWORD64) &ExceptionFilter, &Module))
                        OurPDBIsValid = (Module.SymType == SymPdb);
                }

                if (!OurPDBIsValid) {
                    BufferLength += wnsprintfW(
                        Buffer + BufferLength,
                        ARRAY_COUNT(Buffer) - BufferLength,
                        TEXT(
                            "The PDB debug information file for this executable is "
                            "not valid or was not found. Please rebuild binary to get"
                            " the call stack.\n"
                        )
                    );
                } else {
                    STACKFRAME64 SFrame = {};
                    DWORD ImageType = 0;

                #if defined(NC_CPU_X86_64)
                    ImageType = IMAGE_FILE_MACHINE_AMD64;
                    SFrame.AddrPC.Offset = Context->Rip;
                    SFrame.AddrPC.Mode = ADDRESS_MODE_FLAT;
                    SFrame.AddrFrame.Offset = Context->Rbp;
                    SFrame.AddrFrame.Mode = ADDRESS_MODE_FLAT;
                    SFrame.AddrStack.Offset = Context->Rsp;
                    SFrame.AddrStack.Mode = ADDRESS_MODE_FLAT;
                #elif defined(NC_CPU_ARM64)
                    ImageType = IMAGE_FILE_MACHINE_ARM64;
                    SFrame.AddrPC.Offset = Context->Pc;
                    SFrame.AddrPC.Mode = ADDRESS_MODE_FLAT;
                    SFrame.AddrFrame.Offset = Context->Fp;
                    SFrame.AddrFrame.Mode = ADDRESS_MODE_FLAT;
                    SFrame.AddrStack.Offset = Context->Sp;
                    SFrame.AddrStack.Mode = ADDRESS_MODE_FLAT;
                #else
                    #error Debug helper not defined for architecture
                #endif

                    for (u32 I = 0; ; ++I) {
                        const u32 MaxFrames = 32;

                        if (I == MaxFrames) {
                            BufferLength += wnsprintfW(
                                Buffer + BufferLength, 
                                ARRAY_COUNT(Buffer) - BufferLength, 
                                TEXT("...")
                            );
                            break;
                        }

                        if (
                            !DBGStackWalk64(
                                ImageType, 
                                ThisProcess, 
                                ThisThread, 
                                &SFrame, 
                                Context, 
                                0, 
                                DBGSymFunctionTableAccess64, 
                                DBGSymGetModuleBase64, 
                                0
                            )
                        ) {
                            break;
                        }

                        u64 Address = SFrame.AddrPC.Offset;

                        if (!I) {
                            BufferLength += wnsprintfW(
                                Buffer + BufferLength, 
                                ARRAY_COUNT(Buffer) - BufferLength, 
                                TEXT("Call Stack:\n")
                            );
                        }

                        BufferLength += wnsprintfW(
                            Buffer + BufferLength, 
                            ARRAY_COUNT(Buffer) - BufferLength, 
                            TEXT("%u. [0x%I64X]"), 
                            I + 1, 
                            Address
                        );

                        struct {
                            SYMBOL_INFOW Info;
                            WCHAR Name[MAX_SYM_NAME];
                        } Symbol = {};

                        Symbol.Info.SizeOfStruct = sizeof(Symbol.Info);
                        Symbol.Info.MaxNameLen = MAX_SYM_NAME;

                        DWORD64 Displacement = 0;

                        if (DBGSymFromAddrW(ThisProcess, Address, &Displacement, &Symbol.Info)) {
                            BufferLength += wnsprintfW(
                                Buffer + BufferLength, 
                                ARRAY_COUNT(Buffer) - BufferLength, 
                                TEXT(" %s +%u"), 
                                Symbol.Info.Name, 
                                (DWORD) Displacement
                            );

                            IMAGEHLP_LINEW64 Line = {};

                            Line.SizeOfStruct = sizeof(Line);

                            DWORD LineDisplacement = 0;

                            if (DBGSymGetLineFromAddrW64(ThisProcess, Address, &LineDisplacement, &Line))
                                BufferLength += wnsprintfW(
                                    Buffer + BufferLength, 
                                    ARRAY_COUNT(Buffer) - BufferLength, 
                                    TEXT(", %s line %u"), 
                                    PathFindFileNameW(Line.FileName),
                                    Line.LineNumber
                                );
                        } else {
                            IMAGEHLP_MODULEW64 Module = {};

                            Module.SizeOfStruct = sizeof(Module);

                            if (DBGSymGetModuleInfoW64(ThisProcess, Address, &Module))
                                BufferLength += wnsprintfW(
                                    Buffer + BufferLength, 
                                    ARRAY_COUNT(Buffer) - BufferLength, 
                                    TEXT(" %s"), 
                                    Module.ModuleName
                                );
                        }

                        BufferLength += wnsprintfW(Buffer + BufferLength, ARRAY_COUNT(Buffer) - BufferLength, TEXT("\n"));
                    }
                }
            }
        }
    }

    b32 GenerateCrashDump = TRUE;

    LogPanic((wchar*) TEXT("--- Fatal Exception ---"));
    LogPanic((wchar*) TEXT("%s"), (wchar*) Buffer);

    if (DBGMiniDumpWriteDump && GenerateCrashDump) {
        WCHAR DesktopPath[512] = {};

        SHGetFolderPathW(NULL, CSIDL_DESKTOP, NULL, 0, DesktopPath);

        WCHAR DumpFilePath[512] = {};

        wnsprintfW(DumpFilePath, ARRAY_COUNT(DumpFilePath), TEXT("%s\\libnc_crash_dump.dmp"), DesktopPath);

        SECURITY_ATTRIBUTES SecurityAttributes = {
            sizeof(SecurityAttributes), 
            0, 
            0
        };
        HANDLE File = CreateFileW(DumpFilePath, GENERIC_WRITE, 0, &SecurityAttributes, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
        BOOL DumpSuccessful = DBGMiniDumpWriteDump(GetCurrentProcess(), GetProcessProperties()->PID, File, MiniDumpNormal, 0, 0, 0);

        CloseHandle(File);
        UNUSED(DumpSuccessful);
    }

    ExitProcess(1);
}

internal void
EntryPointCaller(int ArgC, WCHAR** WArgV)
{
    SetUnhandledExceptionFilter(&ExceptionFilter);

    {
        HMODULE Module = LoadLibraryA("kernel32.dll");

        WinSetThreadDescriptionFunc = (WinSetThreadDescriptionFuncType*) GetProcAddress(Module, "SetThreadDescription");
        FreeLibrary(Module);
    }

    b32 LargePagesAllowed = FALSE;

    {
        HANDLE Token;

        if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &Token)) {
            LUID LuID;

            if (LookupPrivilegeValue(NULL, SE_LOCK_MEMORY_NAME, &LuID)) {
                TOKEN_PRIVILEGES Private;

                Private.PrivilegeCount = 1;
                Private.Privileges[0].Luid = LuID;
                Private.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
                LargePagesAllowed = !!AdjustTokenPrivileges(
                    Token, 
                    FALSE, 
                    &Private, 
                    sizeof(Private), 
                    NULL, 
                    NULL
                );
            }

            CloseHandle(Token);
        }
    }

    {
        WSADATA WinSockData;

        WSAStartup(MAKEWORD(2, 2), &WinSockData);

        GUID GuID = WSAID_MULTIPLE_RIO;
        DWORD RIOByte = 0;
        SOCKET Sock = socket(AF_UNSPEC, SOCK_STREAM, IPPROTO_TCP);

        WSAIoctl(
            Sock,
            SIO_GET_MULTIPLE_EXTENSION_FUNCTION_POINTER,
            &GuID,
            sizeof(GuID),
            (void**) &WinRIOFunctions,
            sizeof(WinRIOFunctions),
            &RIOByte,
            NULL,
            NULL
        );
        closesocket(Sock);
    }

    SYSTEM_INFO SysInfo = {};

    GetSystemInfo(&SysInfo);

    {
        OS_STATE.USecsResolution = 1;

        LARGE_INTEGER LargeResolution;

        if (QueryPerformanceFrequency(&LargeResolution))
            OS_STATE.USecsResolution = LargeResolution.QuadPart;
    }

    {
        SystemProperties* SysProps = &OS_STATE.SystemProps;

        SysProps->LogicalProcessorCount = (u64) SysInfo.dwNumberOfProcessors;
        SysProps->PageSize = SysInfo.dwPageSize;
        SysProps->LargePageSize = GetLargePageMinimum();
        SysProps->AllocationGranularity = SysInfo.dwAllocationGranularity;
    }

    {
        ProcessProperties* ProcProps = &OS_STATE.ProcessProps;

        ProcProps->LargePagesAllowed = LargePagesAllowed;
        ProcProps->PID = GetCurrentProcessId();
    }

    // NOTE(nathan): ensure memory is initialised before using
    MemInit();
    ArenaInit();

    Arena* ArgsMemPool = ArenaAlloc(
        __Params.ReserveSize = MB(1), 
        __Params.CommitSize = KB(32)
    );
    char** ArgV = ArenaPushArrayZero(ArgsMemPool, char*, ArgC);

    for (u32 I = 0; I < ArgC; ++I) {
        Str16 WideArg = CStr16((u16*) WArgV[I]);
        Str8 Arg = Str8FromStr16(ArgsMemPool, WideArg);

        // TODO(nathan): define and parse CLI arguments here

        ArgV[I] = (char*) Arg.Str;
    }

    ThreadContext* TCTX = ThreadContextAlloc();

    ThreadContextSelect(TCTX);

    Arena* MemPool = ArenaAlloc();

    {
        OS_STATE.MemPool = MemPool;

        {
            SystemProperties* SysProps = &OS_STATE.SystemProps;
            u8 Buffer[MAX_COMPUTERNAME_LENGTH + 1] = {};
            DWORD Size = MAX_COMPUTERNAME_LENGTH + 1;

            if (GetComputerNameA((char*) Buffer, &Size)) {
                SysProps->MachineName = ArenaPushStrCpy(MemPool, Str(Buffer, Size));
            }
        }
    }

    {
        ProcessProperties* ProcProps = &OS_STATE.ProcessProps;

        {
            TempArena Scratch = GetScratch(NULL, 0);
            DWORD Size = KB(32);
            u16* Buffer = ArenaPushArray(Scratch.MemPool, u16, Size);
            DWORD Len = GetModuleFileNameW(NULL, (WCHAR*) Buffer, Size);
            Str8 Name = Str8FromStr16(Scratch.MemPool, Str(Buffer, Len));
            Str8 NameChopped = StrChopLastSlash(Name);

            ProcProps->BinaryPath = ArenaPushStrCpy(MemPool, NameChopped);
            ReleaseScratch(Scratch);
        }

        ProcProps->InitialPath = GetCurrentPath(MemPool);

        {
            TempArena Scratch = GetScratch(NULL, 0);
            u64 Size = KB(32);
            u16* Buffer = ArenaPushArray(Scratch.MemPool, u16, Size);

            if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_APPDATA, 0, 0, (WCHAR*) Buffer)))
                ProcProps->UserProgramDataPath = Str8FromStr16(MemPool, CStr16(Buffer));

            ReleaseScratch(Scratch);
        }

        {
            WCHAR* ThisProcEnv = GetEnvironmentStringsW();
            u64 StartIndex = 0;

            for (u64 Index = 0; ; ++Index) {
                if (!ThisProcEnv[Index]) {
                    if (StartIndex == Index) {
                        break;
                    } else {
                        Str16 String16 = Str((u16*) ThisProcEnv + StartIndex, Index - StartIndex);
                        Str8 String = Str8FromStr16(MemPool, String16);

                        ListPush(MemPool, &ProcProps->Environment, String);
                        StartIndex = Index + 1;
                    }
                }
            }
        }
    }

    InitializeCriticalSection(&OS_STATE.EntityMutex);
    OS_STATE.EntityMemPool = ArenaAlloc();
    SysInit();
    TimeInit();
    MainThreadBaseEntryPoint(ArgC, ArgV);
}

EXTERN_C_LINK INT
wmain(INT ArgC, WCHAR** WArgV)
{
    timeBeginPeriod(1);
    PrintInit();
    EntryPointCaller(ArgC, WArgV);
    PrintRelease();
    timeEndPeriod(1);

    return 0;
}

EXTERN_C_LINK VOID
wmainCRTStartup(VOID)
{
    __WIN32CRTInit();

    INT ArgC;
    WCHAR** WArgV;

    WArgV = CommandLineToArgvW(GetCommandLineW(), &ArgC);

    INT MainResult = wmain(ArgC, WArgV);

    LocalFree(WArgV);
    __WIN32CRTDone();
    ExitProcess(MainResult);
}

EXTERN_C_LINK VOID NTAPI
tls_callback(HMODULE hModule, DWORD dwReason, LPVOID dwReserved)
{
    switch (dwReason) {
        case DLL_PROCESS_ATTACH: {
            _tls_index = TlsAlloc();
        } // WARN: FALLTHROUGH

        case DLL_THREAD_ATTACH: {
            HANDLE ProcessHeap = GetProcessHeap();
            ThreadLocalStorage* TLS = (ThreadLocalStorage*) HeapAlloc(
                ProcessHeap,
                HEAP_ZERO_MEMORY,
                sizeof(ThreadLocalStorage)
            );

            if (TLS)
                TlsSetValue(_tls_index, TLS);
            else
                LogError("unable to HeapAlloc TLS"_s8);
        } break;

        case DLL_THREAD_DETACH: {
            ThreadLocalStorage* TLS = (ThreadLocalStorage*) TlsGetValue(_tls_index);

            if (TLS)
                HeapFree(GetProcessHeap(), 0, TLS);
            else
                LogError("unable to HeapFree TLS"_s8);
        } break;

        case DLL_PROCESS_DETACH: {
            ThreadLocalStorage* TLS = (ThreadLocalStorage*) TlsGetValue(_tls_index);

            if (TLS)
                HeapFree(GetProcessHeap(), 0, TLS);
            else
                LogError("unable to HeapFree TLS"_s8);

            if (_tls_index != 0)
                TlsFree(_tls_index);
        } break;
    }
}
