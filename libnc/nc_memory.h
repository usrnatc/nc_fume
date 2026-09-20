#if !defined(__NC_MEMORY_H__)
#define __NC_MEMORY_H__

#include "nc_types.h"

// @types______________________________________________________________________
struct MemInfo {
    u64 PageSize;
};

// @defines____________________________________________________________________
extern MemInfo MEM_INFO;

#define MemCpyArray(X, Y)  MemCpy((X), (Y), sizeof(X))
#define MemCmpStruct(X, Y) MemCmp((X), (Y), sizeof(*(X)))
#define MemIsZeroStruct(X) MemIsZero((X), sizeof(*(X)))

// @functions__________________________________________________________________
// NOTE: see win32/win32_intrin.asm
EXTERN_C_LINK_BEGIN
extern void MemZero(void* Ptr, u64 Size);
EXTERN_C_LINK_END

void MemInit(void);
void* MemReserve(u64 Size);
void* MemReserveLarge(u64 Size);
b32 MemCommit(void* Ptr, u64 Size);
b32 MemCommitLarge(void* Ptr, u64 Size);
void MemDeCommit(void* Ptr, u64 Size);
void MemRelease(void* Ptr, u64 Size);
void* MemCpy(void* _Dst, void const* _Src, u64 N);
void* MemSet(void* _Dst, u8 Value, u64 N);
b32 MemCmp(void const* Ptr1, void const* Ptr2, u64 Size);
void* MemMove(void* _Dst, void const* _Src, u64 N);
void EnableMaxMemorySpace(void);
b32 MemIsZero(void* _Ptr, u64 Size);

#endif // __NC_MEMORY_H__
