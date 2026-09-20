#include "win32_platform.h"

void 
MemInit(void)
{
    SYSTEM_INFO SystemInfo;

    GetSystemInfo(&SystemInfo);
    MEM_INFO.PageSize = SystemInfo.dwPageSize;
}

void* 
MemReserve(u64 Size)
{
    return VirtualAlloc(0, Size, MEM_RESERVE, PAGE_NOACCESS);
}

void* 
MemReserveLarge(u64 Size)
{
    return VirtualAlloc(
        NULL,
        Size,
        MEM_RESERVE | MEM_COMMIT | MEM_LARGE_PAGES,
        PAGE_READWRITE
    );
}

b32 
MemCommit(void* Ptr, u64 Size)
{
    return (!!VirtualAlloc(Ptr, Size, MEM_COMMIT, PAGE_READWRITE));
}

b32 
MemCommitLarge(void* Ptr, u64 Size)
{
    return TRUE;
}

void 
MemDeCommit(void* Ptr, u64 Size)
{
    VirtualFree(Ptr, Size, MEM_DECOMMIT);
}

void 
MemRelease(void* Ptr, u64 Size)
{
    UNUSED(Size);
    VirtualFree(Ptr, 0, MEM_RELEASE);
}

void 
EnableMaxMemorySpace(void)
{
    HANDLE ThisProc = GetCurrentProcess();

    SetProcessWorkingSetSize(ThisProc, U64_MAX, U64_MAX);
}
