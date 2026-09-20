#include "nc_arena.h"
#include "nc_memory.h"
#include "nc_tls.h"
#include "nc_log.h"

INTERNAL void 
ArenaInit(void)
{
    ThreadLocalStorage* TLS = GetTLS();

    if (!TLS->ARENA_THREAD_SCRATCH_INIT) {
        for (u8 I = 0; I < ARENA_SCRATCH_COUNT; ++I)
            TLS->ARENA_THREAD_SCRATCH_POOL[I] = ArenaAlloc();

        TLS->ARENA_THREAD_SCRATCH_INIT = TRUE;
    }
}

INTERNAL void
ArenaRelease(Arena* MemPool)
{
    for (Arena* Node = MemPool->Current, *Previous = NULL; Node; Node = Previous) {
        Previous = Node->Prev;
        MemRelease(Node, Node->Reserved);
    }
}

INTERNAL void*
ArenaPushSize(Arena* MemPool, u64 Size, u64 Align, b32 Zero)
{
    Arena* Current = MemPool->Current;
    u64 PrevPosition = ALIGNTO(Current->Position, Align);
    u64 NewPosition = PrevPosition + Size;

    if (Current->Reserved < NewPosition && !(MemPool->Flags & ARENA_FLAG_NO_CHAIN)) {
        Arena* NewBlock = NULL;
        Arena* PrevBlock;

        for (
            NewBlock = MemPool->FreeTail, PrevBlock = NULL;
            NewBlock;
            PrevBlock = NewBlock, NewBlock = NewBlock->Prev
        ) {
            if (NewBlock->Reserved >= ALIGNTO(NewBlock->Position, Align) + Size) {
                if (PrevBlock)
                    PrevBlock->Prev = NewBlock->Prev;
                else
                    MemPool->FreeTail = NewBlock->Prev;

                break;
            }
        }

        if (!NewBlock) {
            u64 ReserveSize = Current->ReserveSize;
            u64 CommitSize = Current->CommitSize;

            if (Size + ARENA_HEADER_SIZE > ReserveSize) {
                ReserveSize = ALIGNTO(Size + ARENA_HEADER_SIZE, Align);
                CommitSize = ALIGNTO(Size + ARENA_HEADER_SIZE, Align);
            }

            NewBlock = ArenaAlloc(
                __Params.ReserveSize = ReserveSize,
                __Params.CommitSize = CommitSize,
                __Params.Flags = Current->Flags,
                __Params.AllocationSiteFile = Current->AllocationSiteFile,
                __Params.AllocationSiteLine = Current->AllocationSiteLine
            );
        }

        NewBlock->BasePosition = Current->BasePosition + Current->Reserved;
        SLL_STACK_PUSH_EX(MemPool->Current, NewBlock, Prev);
        Current = NewBlock;
        PrevPosition = ALIGNTO(Current->Position, Align);
        NewPosition = PrevPosition + Size;
    }

    u64 SizeToZero = 0;

    if (Zero)
        SizeToZero = Size;

    if (Current->Committed < NewPosition) {
        u64 NewCommittedAligned = NewPosition + Current->CommitSize - 1;

        NewCommittedAligned -= NewCommittedAligned % Current->CommitSize;

        u64 NewCommittedClamped = CLAMP_TOP(NewCommittedAligned, Current->Reserved);
        u64 NewCommitSize = NewCommittedClamped - Current->Committed;
        u8* CommittedPtr = (u8*) Current + Current->Committed;

        if (Current->Flags & ARENA_FLAG_LARGE_PAGES)
            MemCommitLarge(CommittedPtr, NewCommitSize);
        else
            MemCommit(CommittedPtr, NewCommitSize);

        Current->Committed = NewCommittedClamped;
    }

    void* Result = NULL;

    if (Current->Committed >= NewPosition) {
        Result = (u8*) Current + PrevPosition;
        Current->Position = NewPosition;
        ASAN_UNPOISON(Result, Size);

        if (SizeToZero)
            MemSet(Result, 0, SizeToZero);
    }

#if defined(NC_GUI)
    if (UNLIKELY(!Result)) {
        GraphicalMessage(
            TRUE,
            "Fatal Allocation Error"_s8,
            "Unexpected memory allocation failure"_s8
        );
        Abort(1);
    }
#else
    if (UNLIKELY(!Result)) {
        LogPanic("Fatal Allocation Error"_s8);
        LogPanic("Unexpected memory allocation failure"_s8);
        Abort(1);
    }
#endif

    return Result;
}

INTERNAL u64
ArenaGetPosition(Arena* MemPool)
{
    Arena const* Current = MemPool->Current;

    return Current->BasePosition + Current->Position;
}

INTERNAL void
ArenaPopTo(Arena *MemPool, u64 Position)
{
    u64 BigPosition = CLAMP_BOT(ARENA_HEADER_SIZE, Position);
    Arena* Current = MemPool->Current;

    for (Arena* Prev = NULL; Current->BasePosition >= BigPosition; Current = Prev) {
        Prev = Current->Prev;
        Current->Position = ARENA_HEADER_SIZE;
        SLL_STACK_PUSH_EX(MemPool->FreeTail, Current, Prev);
        ASAN_POISON((u8*) Current + ARENA_HEADER_SIZE, Current->ReserveSize - ARENA_HEADER_SIZE);
    }

    MemPool->Current = Current;

    u64 NewPosition = BigPosition - Current->BasePosition;

    ASSERT_ALWAYS(NewPosition <= Current->Position);
    ASAN_POISON((u8*) Current + NewPosition, (Current->Position - NewPosition));

    Current->Position = NewPosition;
}

INTERNAL void
ArenaPop(Arena* MemPool, u64 Size)
{
    u64 OldPosition = ArenaGetPosition(MemPool);
    u64 NewPosition = OldPosition;

    if (Size < OldPosition)
        NewPosition = OldPosition - Size;

    ArenaPopTo(MemPool, NewPosition);
}

INTERNAL void
ArenaClear(Arena *MemPool)
{
    ArenaPopTo(MemPool, 0);
}

INTERNAL TempArena
ArenaBeginTemp(Arena *MemPool)
{
    u64 Position = ArenaGetPosition(MemPool);
    TempArena Result = {
        MemPool,
        Position
    };

    return Result;
}

INTERNAL void
ArenaEndTemp(TempArena TempMemPool)
{
    ArenaPopTo(TempMemPool.MemPool, TempMemPool.Position);
}

INTERNAL Arena*
__GetScratch(Arena** Conflicts, u64 Count)
{
    ThreadLocalStorage* TLS = GetTLS();

    Arena** ScratchPool = TLS->ARENA_THREAD_SCRATCH_POOL;

    if (!TLS->ARENA_THREAD_SCRATCH_INIT)
        ArenaInit();

    Arena* Result = NULL;
    Arena** ArenaPtr = ScratchPool;

    for (u64 I = 0; I < ARENA_SCRATCH_COUNT; ++I, ++ArenaPtr) {
        Arena** ConflictPtr = Conflicts;
        b32 HasConflict = FALSE;

        for (u64 J = 0; J < Count; ++J, ++ConflictPtr) {
            if (*ArenaPtr == *ConflictPtr) {
                HasConflict = TRUE;
                break;
            }
        }

        if (!HasConflict) {
            Result = *ArenaPtr;
            break;
        }
    }

    return Result;
}

INTERNAL Arena*
__ArenaAlloc(ArenaParams* Params)
{
    u64 ReserveSize = Params->ReserveSize;
    u64 CommitSize = Params->CommitSize;

    if (Params->Flags & ARENA_FLAG_LARGE_PAGES) {
        ReserveSize = ALIGNTO(ReserveSize, GetSystemProperties()->LargePageSize);
        CommitSize = ALIGNTO(CommitSize, GetSystemProperties()->LargePageSize);
    } else {
        ReserveSize = ALIGNTO(ReserveSize, GetSystemProperties()->PageSize);
        CommitSize = ALIGNTO(CommitSize, GetSystemProperties()->PageSize);
    }

    void* Base = Params->BackingBuffer;

    if (!Base) {
        if (Params->Flags & ARENA_FLAG_LARGE_PAGES) {
            Base = MemReserveLarge(ReserveSize);
            if (!MemCommitLarge(Base, CommitSize)) {
                LogError(
                    "mem commit large failed: reserve = %llu, commit = %llu",
                    ReserveSize,
                    CommitSize
                );
                LogError(
                    "flags = %u from %s:%d",
                    Params->Flags, 
                    (char*) Params->AllocationSiteFile, 
                    Params->AllocationSiteLine
                );

                return NULL;
            }
        } else {
            Base = MemReserve(ReserveSize);
            if (!MemCommit(Base, CommitSize)) {
                LogError(
                    "mem commit failed: reserve = %llu, commit = %llu",
                    ReserveSize,
                    CommitSize
                );
                LogError(
                    "flags = %u from %s:%d",
                    Params->Flags, 
                    (char*) Params->AllocationSiteFile, 
                    Params->AllocationSiteLine
                );

                return NULL;
            }
        }
    }

#if defined(NC_GUI) && NC_GUI
    if (UNLIKELY(!Base)) {
        GraphicalMessage(
            TRUE,
            Str8Lit("Fatal allocation failure"),
            Str8Lit("Unexpected memory allocation failure")
        );
        Abort(1);
    }
#else
    if (UNLIKELY(!Base)) {
        LogPanic("Fatal Allocation Error"_s8);
        LogPanic("Unexpected memory allocation failure"_s8);
        Abort(1);
    }
#endif

    Arena* Result = (Arena*) Base;

    Result->Current = Result;
    Result->Flags = Params->Flags;
    Result->CommitSize = Params->CommitSize;
    Result->ReserveSize = Params->ReserveSize;
    Result->BasePosition = 0;
    Result->Position = ARENA_HEADER_SIZE;
    Result->Committed = CommitSize;
    Result->Reserved = ReserveSize;
    Result->AllocationSiteFile = Params->AllocationSiteFile;
    Result->AllocationSiteLine = Params->AllocationSiteLine;
    Result->FreeTail = NULL;
    ASAN_POISON(Base, CommitSize);
    ASAN_UNPOISON(Base, ARENA_HEADER_SIZE);

    return Result;
}

Arena*
ArenaAllocEx(u64 ReserveSize)
{
    return ArenaAlloc(
        __Params.ReserveSize = ReserveSize,
        __Params.CommitSize = ARENA_EX_COMMIT_SIZE,
        __Params.Flags = ARENA_FLAG_NO_CHAIN
    );
}

u8*
ArenaExBase(Arena* Stream)
{
    return (u8*) Stream + ARENA_HEADER_SIZE;
}
