#if !defined(__NC_ARENA_H__)
#define __NC_ARENA_H__

#include "nc_defines.h"
#include "nc_types.h"

// @defines____________________________________________________________________
#define ARENA_HEADER_SIZE          128
#define ARENA_DEFAULT_RESERVE_SIZE MB(64)
#define ARENA_DEFAULT_COMMIT_SIZE  KB(64)
#define ARENA_DEFAULT_FLAGS        0
#define ARENA_SCRATCH_COUNT        2LLU

#define ARENA_EX_RESERVE_SIZE GB(4)
#define ARENA_EX_COMMIT_SIZE  KB(256)

#define ArenaAlloc(...) [&]() -> Arena* {               \
    ArenaParams __Params = {};                          \
    __Params.ReserveSize = ARENA_DEFAULT_RESERVE_SIZE;  \
    __Params.CommitSize = ARENA_DEFAULT_COMMIT_SIZE;    \
    __Params.AllocationSiteFile = (u8*) __FILE__;       \
    __Params.AllocationSiteLine = __LINE__;             \
    __Params.Flags = ARENA_DEFAULT_FLAGS;               \
    __VA_ARGS__;                                        \
    return __ArenaAlloc(&__Params);                     \
}()

#define GetScratch(Conflicts, Count) ArenaBeginTemp(__GetScratch(Conflicts, Count))
#define ReleaseScratch(Scratch)      ArenaEndTemp(Scratch)

#define ArenaPushType(MemPool, Type, Align)               (Type*) ArenaPushSize(MemPool, sizeof(Type), Align, FALSE)
#define ArenaPushStruct(MemPool, Type)                    (Type*) ArenaPushSize(MemPool, sizeof(Type), ALIGNOF(Type), FALSE)
#define ArenaPushArray(MemPool, Type, Count)              (Type*) ArenaPushSize(MemPool, sizeof(Type) * (Count), ALIGNOF(Type), FALSE)
#define ArenaPushArrayEx(MemPool, Type, Count, Align)     (Type*) ArenaPushSize(MemPool, sizeof(Type) * (Count), (Align), FALSE)
#define ArenaPushVArray(MemPool, Type, Count)             (Type*) ArenaPushSize(MemPool, sizeof(Type) * (Count), NC_SIMD_ALIGN, FALSE)
#define ArenaPushStructZero(MemPool, Type)                (Type*) ArenaPushSize(MemPool, sizeof(Type), ALIGNOF(Type), TRUE)
#define ArenaPushArrayZero(MemPool, Type, Count)          (Type*) ArenaPushSize(MemPool, sizeof(Type) * (Count), ALIGNOF(Type), TRUE)
#define ArenaPushVArrayZero(MemPool, Type, Count)         (Type*) ArenaPushSize(MemPool, sizeof(Type) * (Count), NC_SIMD_ALIGN, TRUE)
#define ArenaPushArrayZeroEx(MemPool, Type, Count, Align) (Type*) ArenaPushSize(MemPool, sizeof(Type) * (Count), Align, TRUE)


typedef u32 ArenaFlag;
enum : u32 {
    ARENA_FLAG_NO_CHAIN     = (1 << 0),
    ARENA_FLAG_LARGE_PAGES  = (1 << 1)
};

struct SystemProperties;
struct ProcessProperties;

extern SystemProperties* GetSystemProperties(void);
extern ProcessProperties* GetProcessProperties(void);

// @types______________________________________________________________________
struct ArenaParams {
    ArenaFlag Flags;
    u64       ReserveSize;
    u64       CommitSize;
    void*     BackingBuffer;
    u8*       AllocationSiteFile;
    i32       AllocationSiteLine;
};

struct Arena {
    Arena*    Current;
    Arena*    Prev;
    u64       CommitSize;
    u64       ReserveSize;
    u64       BasePosition;
    u64       Position;
    u64       Committed;
    u64       Reserved;
    ArenaFlag Flags;
    u8*       AllocationSiteFile;
    i32       AllocationSiteLine;
    Arena*    FreeTail;
};
STATIC_ASSERT(sizeof(Arena) <= ARENA_HEADER_SIZE, ArenaHeaderSizeCheck);

struct TempArena {
    Arena* MemPool;
    u64    Position;
};

// @functions__________________________________________________________________
INTERNAL void ArenaInit(void);
INTERNAL Arena* __ArenaAlloc(ArenaParams* Params);
INTERNAL void ArenaRelease(Arena* MemPool);
INTERNAL void* ArenaPushSize(Arena* MemPool, u64 Size, u64 Align, b32 Zero);
INTERNAL u64 ArenaGetPosition(Arena* MemPool);
INTERNAL void ArenaPopTo(Arena* MemPool, u64 Position);
INTERNAL void ArenaPop(Arena* MemPool, u64 Size);
INTERNAL void ArenaClear(Arena* MemPool);
INTERNAL TempArena ArenaBeginTemp(Arena* MemPool);
INTERNAL void ArenaEndTemp(TempArena TempMemPool);
INTERNAL Arena* __GetScratch(Arena** Conflicts, u64 Count);
INTERNAL Arena* ArenaAllocEx(u64 ReserveSize = ARENA_EX_RESERVE_SIZE);
INTERNAL u8* ArenaExBase(Arena* Stream);

#endif // __NC_ARENA_H__
