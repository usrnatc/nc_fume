#include "linux_platform.h"

void
MemInit(void)
{}

void*
MemReserve(u64 Size)
{
    void* Result = mmap(
        NULL,
        Size,
        PROT_NONE,
        MAP_PRIVATE | MAP_ANONYMOUS,
        -1,
        0
    );

    if (Result == MAP_FAILED)
        Result = NULL;

    return Result;
}

void*
MemReserveLarge(u64 Size)
{
    void* Result = mmap(
        NULL,
        Size,
        PROT_READ | PROT_WRITE,
        MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB,
        -1,
        0
    );

    if (Result == MAP_FAILED)
        Result = NULL;

    madvise(Result, Size, MADV_HUGEPAGE);

    return Result;
}

b32 
MemCommit(void* Ptr, u64 Size)
{
    int MProtectResult = mprotect(Ptr, Size, PROT_READ | PROT_WRITE);

    if (MProtectResult == -1)
        return FALSE;

    return TRUE;
}

b32 
MemCommitLarge(void *Ptr, u64 Size)
{
    // mprotect(Ptr, Size, PROT_READ | PROT_WRITE);

    return TRUE;
}

void
MemDeCommit(void *Ptr, u64 Size)
{
    madvise(Ptr, Size, MADV_DONTNEED);
    mprotect(Ptr, Size, PROT_NONE);
}

void
MemRelease(void *Ptr, u64 Size)
{
    munmap(Ptr, Size);
}

void
EnableMaxMemorySpace(void)
{
    rlimit ResourceLimit = {};

    ResourceLimit.rlim_cur = RLIM_INFINITY;
    ResourceLimit.rlim_max = RLIM_INFINITY;
    setrlimit(RLIMIT_AS, &ResourceLimit);
    setrlimit(RLIMIT_DATA, &ResourceLimit);
}
