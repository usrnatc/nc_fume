#include "nc_system.h"

u64 GLOBAL_UPDATE_TICK_INDEX = 0;
u64 ASYNC_THREADS_COUNT = 0;
Handle ASYNC_TICK_BEGIN_COND_VAR = EMPTY_HANDLE_VALUE;
Handle ASYNC_TICK_BEGIN_MTX = EMPTY_HANDLE_VALUE;
Handle ASYNC_TICK_END_MTX = EMPTY_HANDLE_VALUE;
b32 ASYNC_LOOP_REPEAT = FALSE;
b32 ASYNC_LOOP_REPEAT_HIGH_PRIORITY = FALSE;
b32 GLOBAL_ASYNC_EXIT = FALSE;

INTERNAL b32
operator==(Handle HndlA, Handle HndlB)
{
    return (*HndlA.V == *HndlB.V);
}

INTERNAL b32
operator!=(Handle HndlA, Handle HndlB)
{
    return (*HndlA.V != *HndlB.V);
}

INTERNAL void
ListPush(Arena* MemPool, HandleList* List, Handle Hndl)
{
    HandleNode* Node = ArenaPushArrayZero(MemPool, HandleNode, 1);

    Node->Value = Hndl;
    SLL_QUEUE_PUSH(List->Head, List->Tail, Node);
    ++List->Count;
}

INTERNAL HandleArray
HandleArrayFromList(Arena* MemPool, HandleList* List)
{
    HandleArray Result = {};

    Result.Count = List->Count;
    Result.Data = ArenaPushArray(MemPool, Handle, Result.Count);

    u64 Index = 0;

    for (HandleNode* Node = List->Head; Node; Node = Node->Next, ++Index)
        Result.Data[Index] = Node->Value;

    return Result;
}

INTERNAL u64
UpdateTickIndex(void)
{
    u64 Result = AtomicLoadU64(&GLOBAL_UPDATE_TICK_INDEX, MEM_ORDER_SEQ_CST);

    return Result;
}
