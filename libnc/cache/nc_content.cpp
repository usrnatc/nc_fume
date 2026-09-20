#include "nc_content.h"
#include "../nc_memory.h"
#include "../nc_arena.h"
#include "../nc_sync.h"
#include "../nc_atomics.h"
#include "../nc_time.h"

CShared* C_SHARED = NULL;

CID 
CIDAlloc(u64 V0, u64 V1)
{
    CID Result = {};

    Result.V[0].U64[0] = V0;
    Result.V[0].U64[1] = V1;

    return Result;
}

CKey 
CKeyAlloc(CRoot Root, CID ID)
{
    CKey Result = {
        Root,
        0,
        ID
    };

    return Result;
}


#if defined(NC_LANG_CPP)
    b32 
    operator==(CID IDA, CID IDB)
    {
        return MemCmpStruct(&IDA, &IDB);
    }

    b32 
    operator!=(CID IDA, CID IDB)
    {
        return !(IDA == IDB);
    }

    b32 
    operator==(CKey KeyA, CKey KeyB)
    {
        return (MemCmpStruct(&KeyA, &KeyB) && KeyA.ID == KeyB.ID);
    }

    b32 
    operator!=(CKey KeyA, CKey KeyB)
    {
        return !(KeyA == KeyB);
    }
#endif

void 
CInit(void)
{
    Arena* MemPool = ArenaAlloc();

    C_SHARED = ArenaPushArrayZero(MemPool, CShared, 1);
    C_SHARED->MemPool = MemPool;
    C_SHARED->BlobSlotsCount = 16384;
    C_SHARED->BlobStripesCount = MIN(C_SHARED->BlobSlotsCount, GetSystemProperties()->LogicalProcessorCount);
    C_SHARED->BlobSlots = ArenaPushArrayZero(MemPool, CBlobSlot, C_SHARED->BlobSlotsCount);
    C_SHARED->BlobStripes = ArenaPushArrayZero(MemPool, CStripe, C_SHARED->BlobStripesCount);
    C_SHARED->BlobStripesFreeNodes = ArenaPushArrayZero(MemPool, CBlobNode*, C_SHARED->BlobStripesCount);

    for (u64 Index = 0; Index < C_SHARED->BlobStripesCount; ++Index) {
        CStripe* BlobStripe = &C_SHARED->BlobStripes[Index];

        BlobStripe->MemPool = ArenaAlloc();
        BlobStripe->RWMtx = RWMutexAlloc();
        BlobStripe->CondVar = CondVarAlloc();
    }

    C_SHARED->KeySlotsCount = 4096;
    C_SHARED->KeyStripesCount = MIN(C_SHARED->KeySlotsCount, GetSystemProperties()->LogicalProcessorCount);
    C_SHARED->KeySlots = ArenaPushArrayZero(MemPool, CKeySlot, C_SHARED->KeySlotsCount);
    C_SHARED->KeyStripes = ArenaPushArrayZero(MemPool, CStripe, C_SHARED->KeyStripesCount);
    C_SHARED->KeyStripesFreeNodes = ArenaPushArrayZero(MemPool, CKeyNode*, C_SHARED->KeyStripesCount);

    for (u64 Index = 0; Index < C_SHARED->KeyStripesCount; ++Index) {
        CStripe* KeyStripe = &C_SHARED->KeyStripes[Index];

        KeyStripe->MemPool = ArenaAlloc();
        KeyStripe->RWMtx = RWMutexAlloc();
        KeyStripe->CondVar = CondVarAlloc();
    }

    C_SHARED->RootSlotsCount = 4096;
    C_SHARED->RootStripesCount = MIN(C_SHARED->RootSlotsCount, GetSystemProperties()->LogicalProcessorCount);
    C_SHARED->RootSlots = ArenaPushArrayZero(MemPool, CRootSlot, C_SHARED->RootSlotsCount);
    C_SHARED->RootStripes = ArenaPushArrayZero(MemPool, CStripe, C_SHARED->RootStripesCount);
    C_SHARED->RootStripesFreeNodes = ArenaPushArrayZero(MemPool, CRootNode*, C_SHARED->RootStripesCount);

    for (u64 Index = 0; Index < C_SHARED->RootStripesCount; ++Index) {
        CStripe* RootStripe = &C_SHARED->RootStripes[Index];

        RootStripe->MemPool = ArenaAlloc();
        RootStripe->RWMtx = RWMutexAlloc();
        RootStripe->CondVar = CondVarAlloc();
    }
}

CRoot 
CRootAlloc(void)
{
    CRoot Root = {};

    Root.V[0] = AtomicIncFetchU64(&C_SHARED->RootIDGeneration, MEM_ORDER_SEQ_CST);

    u64 SlotIndex = Root.V[0] % C_SHARED->RootSlotsCount;
    u64 StripeIndex = SlotIndex % C_SHARED->RootStripesCount;
    CRootSlot* Slot = &C_SHARED->RootSlots[SlotIndex];
    CStripe* RootStripe = &C_SHARED->RootStripes[StripeIndex];

    LOCK_SCOPE_W(RootStripe->RWMtx) {
        CRootNode* Node = C_SHARED->RootStripesFreeNodes[StripeIndex];

        if (Node)
            SLL_STACK_POP(C_SHARED->RootStripesFreeNodes[StripeIndex]);
        else
            Node = ArenaPushArrayZero(RootStripe->MemPool, CRootNode, 1);

        DLL_PUSH_BACK(Slot->Head, Slot->Tail, Node);
        Node->Root = Root;
        Node->MemPool = ArenaAlloc();
    }

    return Root;
}

void 
CRootRelease(CRoot Root)
{
    u64 SlotIndex = Root.V[0] % C_SHARED->RootSlotsCount;
    u64 StripeIndex = SlotIndex % C_SHARED->RootStripesCount;
    CRootSlot* Slot = &C_SHARED->RootSlots[SlotIndex];
    CStripe* RootStripe = &C_SHARED->RootStripes[StripeIndex];
    Arena* RootMemPool = NULL;
    CRootIDChunkList RootIDs = {};

    LOCK_SCOPE_W(RootStripe->RWMtx) {
        for (CRootNode* N = Slot->Head; N; N = N->Next) {
            if (MemCmpStruct(&Root, &N->Root)) {
                DLL_REMOVE(Slot->Head, Slot->Tail, N);
                RootMemPool = N->MemPool;
                RootIDs = N->IDs;
                SLL_STACK_PUSH(C_SHARED->RootStripesFreeNodes[StripeIndex], N);
                break;
            }
        }
    }

    for (
        CRootIDChunkNode* IDChunkNode = RootIDs.Head; 
        IDChunkNode; 
        IDChunkNode = IDChunkNode->Next
    ) {
        for (u64 Index = 0; Index < IDChunkNode->Count; ++Index) {
            CID ID = IDChunkNode->V[Index];
            CKey Key = CKeyAlloc(Root, ID);

            CCloseKey(Key);
        }
    }
}

u128 
CSubmitData(CKey Key, Arena** DataMemPool, Str8 Data)
{
    u64 KeyHash = Hash(Str8Struct(&Key));
    u64 KeySlotIndex = KeyHash % C_SHARED->KeySlotsCount;
    u64 KeyStripeIndex = KeySlotIndex % C_SHARED->KeyStripesCount;
    CKeySlot* KeySlot = &C_SHARED->KeySlots[KeySlotIndex];
    CStripe* KeyStripe = &C_SHARED->KeyStripes[KeyStripeIndex];
    u128 DataHash = BigHash(Data);
    u64 SlotIndex = DataHash.U64[1] % C_SHARED->BlobSlotsCount;
    u64 StripeIndex = SlotIndex % C_SHARED->BlobStripesCount;
    CBlobSlot* BlobSlot = &C_SHARED->BlobSlots[SlotIndex];
    CStripe* BlobStripe = &C_SHARED->BlobStripes[StripeIndex];

    LOCK_SCOPE_W(BlobStripe->RWMtx) {
        CBlobNode* Node = NULL;

        for (CBlobNode* N = BlobSlot->Head; N; N = N->Next) {
            if (N->BlobHash == DataHash) {
                Node = N;
                break;
            }
        }

        if (Node && DataMemPool && *DataMemPool)
            ArenaRelease(*DataMemPool);

        if (!Node) {
            Node = C_SHARED->BlobStripesFreeNodes[StripeIndex];

            if (Node)
                SLL_STACK_POP(C_SHARED->BlobStripesFreeNodes[StripeIndex]);
            else
                Node = ArenaPushArray(BlobStripe->MemPool, CBlobNode, 1);

            MemSet(Node, 0, sizeof(*Node));
            Node->BlobHash = DataHash;

            if (DataMemPool)
                Node->MemPool = *DataMemPool;

            Node->Data = Data;
            DLL_PUSH_BACK(BlobSlot->Head, BlobSlot->Tail, Node);
        }

        ++Node->KeyReferenceCount;

        if (DataMemPool)
            *DataMemPool = NULL;
    }

    u128 KeyExpiredHash = {};

    LOCK_SCOPE_W(KeyStripe->RWMtx) {
        CKeyNode* KeyNode = NULL;

        for (CKeyNode* N = KeySlot->Head; N; N = N->Next) {
            if (N->Key == Key) {
                KeyNode = N;
                break;
            }
        }

        b32 KeyIsNew = FALSE;

        if (!KeyNode) {
            KeyIsNew = TRUE;
            KeyNode = C_SHARED->KeyStripesFreeNodes[KeyStripeIndex];

            if (KeyNode)
                SLL_STACK_POP(C_SHARED->KeyStripesFreeNodes[KeyStripeIndex]);
            else
                KeyNode = ArenaPushArray(KeyStripe->MemPool, CKeyNode, 1);

            MemSet(KeyNode, 0, sizeof(*KeyNode));
            KeyNode->Key = Key;
            DLL_PUSH_BACK(KeySlot->Head, KeySlot->Tail, KeyNode);
        }

        if (KeyNode) {
            if (KeyNode->HashHistoryGeneration >= C_KEY_HASH_HISTORY_STRONG_REF_COUNT)
                KeyExpiredHash = KeyNode->HashHistory[
                    (
                        KeyNode->HashHistoryGeneration - C_KEY_HASH_HISTORY_STRONG_REF_COUNT
                    ) % ARRAY_COUNT(KeyNode->HashHistory)
                ];
            
            KeyNode->HashHistory[
                KeyNode->HashHistoryGeneration % ARRAY_COUNT(KeyNode->HashHistory)
            ] = DataHash;
            ++KeyNode->HashHistoryGeneration;
        }

        if (KeyIsNew) {
            u64 RootHash = Hash(Str8Struct(&Key.Root));
            u64 RootSlotIndex = RootHash % C_SHARED->RootSlotsCount;
            u64 RootStripeIndex = RootSlotIndex % C_SHARED->RootStripesCount;
            CRootSlot* RootSlot = &C_SHARED->RootSlots[RootSlotIndex];
            CStripe* RootStripe = &C_SHARED->RootStripes[RootStripeIndex];

            LOCK_SCOPE_W(RootStripe->RWMtx) {
                for (CRootNode* N = RootSlot->Head; N; N = N->Next) {
                    if (MemCmpStruct(&N->Root, &Key.Root)) {
                        CRootIDChunkNode* Chunk = N->IDs.Tail;

                        if (!Chunk || Chunk->Count > Chunk->Capacity) {
                            Chunk = ArenaPushArrayZero(
                                N->MemPool, 
                                CRootIDChunkNode, 
                                1
                            );
                            SLL_QUEUE_PUSH(N->IDs.Head, N->IDs.Tail, Chunk);
                            ++N->IDs.ChunkCount;
                            Chunk->Capacity = 1024;
                            Chunk->V = ArenaPushArray(
                                N->MemPool, 
                                CID, 
                                Chunk->Capacity
                            );
                        }

                        Chunk->V[Chunk->Count] = Key.ID;
                        ++Chunk->Count;
                        ++N->IDs.TotalCount;
                        break;
                    }
                }
            }
        }
    }

    if (KeyExpiredHash != u128{}) {
        u64 OldHashSlotIndex = KeyExpiredHash.U64[1] % C_SHARED->BlobSlotsCount;
        u64 OldHashStripeIndex = OldHashSlotIndex % C_SHARED->BlobStripesCount;
        CBlobSlot* OldHashSlot = &C_SHARED->BlobSlots[OldHashSlotIndex];
        CStripe* OldHashStripe = &C_SHARED->BlobStripes[OldHashStripeIndex];

        LOCK_SCOPE_R(OldHashStripe->RWMtx) {
            for (CBlobNode* N = OldHashSlot->Head; N; N = N->Next) {
                if (N->BlobHash == KeyExpiredHash) {
                    AtomicDecFetchU64(&N->KeyReferenceCount, MEM_ORDER_SEQ_CST);
                    break;
                }
            }
        }
    }

    return DataHash;
}

void 
CCloseKey(CKey Key)
{
    u64 KeyHash = Hash(Str8Struct(&Key));
    u64 KeySlotIndex = KeyHash % C_SHARED->KeySlotsCount;
    u64 KeyStripeIndex = KeySlotIndex % C_SHARED->KeyStripesCount;
    CKeySlot* KeySlot = &C_SHARED->KeySlots[KeySlotIndex];
    CStripe* KeyStripe = &C_SHARED->KeyStripes[KeyStripeIndex];

    LOCK_SCOPE_W(KeyStripe->RWMtx) {
        for (CKeyNode* N = KeySlot->Head; N; N = N->Next) {
            if (N->Key == Key) {
                for (
                    u64 HistoryIndex = 0;
                    HistoryIndex < C_KEY_HASH_HISTORY_STRONG_REF_COUNT && HistoryIndex < N->HashHistoryGeneration;
                    ++HistoryIndex
                ) {
                    u128 NHash = N->HashHistory[(N->HashHistoryGeneration - 1 - HistoryIndex) % ARRAY_COUNT(N->HashHistory)];
                    u64 NHashSlotIndex = NHash.U64[1] % C_SHARED->BlobSlotsCount;
                    u64 NHashStripeIndex = NHashSlotIndex % C_SHARED->BlobStripesCount;
                    CBlobSlot* NHashSlot = &C_SHARED->BlobSlots[NHashSlotIndex];
                    CStripe* NHashStripe = &C_SHARED->BlobStripes[NHashStripeIndex];

                    LOCK_SCOPE_R(NHashStripe->RWMtx) {
                        for (CBlobNode* N = NHashSlot->Head; N; N = N->Next) {
                            if (N->BlobHash == NHash) {
                                AtomicDecFetchU64(&N->KeyReferenceCount, MEM_ORDER_SEQ_CST);
                                break;
                            }
                        }
                    }
                }

                DLL_REMOVE(KeySlot->Head, KeySlot->Tail, N);
                SLL_STACK_PUSH(C_SHARED->KeyStripesFreeNodes[KeyStripeIndex], N);
                break;
            }
        }
    }
}

void 
CHashDownstreamInc(u128 CHash)
{
    u64 SlotIndex = CHash.U64[1] % C_SHARED->BlobSlotsCount;
    u64 StripeIndex = SlotIndex % C_SHARED->BlobStripesCount;
    CBlobSlot* Slot = &C_SHARED->BlobSlots[SlotIndex];
    CStripe* BlobStripe = &C_SHARED->BlobStripes[StripeIndex];

    LOCK_SCOPE_R(BlobStripe->RWMtx) {
        for (CBlobNode* N = Slot->Head; N; N = N->Next) {
            if (CHash == N->BlobHash) {
                AtomicIncFetchU64(&N->DownstreamReferenceCount, MEM_ORDER_SEQ_CST);
                break;
            }
        }
    }
}

void 
CHashDownstreamDec(u128 CHash)
{
    u64 SlotIndex = CHash.U64[1] % C_SHARED->BlobSlotsCount;
    u64 StripeIndex = SlotIndex % C_SHARED->BlobStripesCount;
    CBlobSlot* Slot = &C_SHARED->BlobSlots[SlotIndex];
    CStripe* BlobStripe = &C_SHARED->BlobStripes[StripeIndex];

    LOCK_SCOPE_R(BlobStripe->RWMtx) {
        for (CBlobNode* N = Slot->Head; N; N = N->Next) {
            if (CHash == N->BlobHash) {
                AtomicDecFetchU64(&N->DownstreamReferenceCount, MEM_ORDER_SEQ_CST);
                break;
            }
        }
    }
}

u128 
CHashFromKey(CKey Key, u64 RewindCount)
{
    u128 Result = {};
    u64 KeyHash = Hash(Str8Struct(&Key));
    u64 KeySlotIndex = KeyHash % C_SHARED->KeySlotsCount;
    u64 KeyStripeIndex = KeySlotIndex % C_SHARED->KeyStripesCount;
    CKeySlot* KeySlot = &C_SHARED->KeySlots[KeySlotIndex];
    CStripe* KeyStripe = &C_SHARED->KeyStripes[KeyStripeIndex];

    LOCK_SCOPE_R(KeyStripe->RWMtx) {
        for (CKeyNode* N = KeySlot->Head; N; N = N->Next) {
            if (
                (N->Key == Key) && 
                N->HashHistoryGeneration && 
                N->HashHistoryGeneration - 1 >= RewindCount
            ) {
                Result = N->HashHistory[(N->HashHistoryGeneration - 1 - RewindCount) % ARRAY_COUNT(N->HashHistory)];
                break;
            }
        }
    }

    return Result;
}

Str8 
CDataFromHash(Access* Acc, u128 CHash)
{
    Str8 Result = {};
    u64 SlotIndex = CHash.U64[1] % C_SHARED->BlobSlotsCount;
    u64 StripeIndex = SlotIndex % C_SHARED->BlobStripesCount;
    CBlobSlot* Slot = &C_SHARED->BlobSlots[SlotIndex];
    CStripe* BlobStripe = &C_SHARED->BlobStripes[StripeIndex];

    LOCK_SCOPE_R(BlobStripe->RWMtx) {
        for (CBlobNode* N = Slot->Head; N; N = N->Next) {
            if (N->BlobHash == CHash) {
                Result = N->Data;
                AccessTouch(Acc, &N->Point, BlobStripe->CondVar);
                break;
            }
        }
    }

    return Result;
}

void 
CAsyncTick(void)
{
    r1u64 Range = LaneRange(C_SHARED->BlobSlotsCount);

    for (u64 SlotIndex = Range.Min; SlotIndex < Range.Max; ++SlotIndex) {
        u64 StripeIndex = SlotIndex % C_SHARED->BlobStripesCount;
        CBlobSlot* Slot = &C_SHARED->BlobSlots[SlotIndex];
        CStripe* BlobStripe = &C_SHARED->BlobStripes[StripeIndex];

        for (b32 WriteMode = FALSE; WriteMode <= TRUE; ++WriteMode) {
            b32 SlotHasWork = FALSE;

            if (!WriteMode) {
                LOCK_SCOPE_R(BlobStripe->RWMtx) {
                    for (
                        CBlobNode* N = Slot->Head, *Next = NULL; 
                        N; 
                        N = Next
                    ) {
                        Next = N->Next;

                        u64 KeyReferenceCount = AtomicLoadU64(&N->KeyReferenceCount, MEM_ORDER_SEQ_CST);
                        u64 DownstreamReferenceCount = AtomicLoadU64(&N->DownstreamReferenceCount, MEM_ORDER_SEQ_CST);

                        if (
                            AccessPointIsExpired(&N->Point, __Params.Time = SECONDS(5)) &&
                            !KeyReferenceCount &&
                            !DownstreamReferenceCount
                        ) {
                            SlotHasWork = TRUE;
                            break;
                        }
                    }
                }
            } else {
                LOCK_SCOPE_W(BlobStripe->RWMtx) {
                    for (
                        CBlobNode* N = Slot->Head, *Next = NULL; 
                        N; 
                        N = Next
                    ) {
                        Next = N->Next;

                        u64 KeyReferenceCount = AtomicLoadU64(&N->KeyReferenceCount, MEM_ORDER_SEQ_CST);
                        u64 DownstreamReferenceCount = AtomicLoadU64(&N->DownstreamReferenceCount, MEM_ORDER_SEQ_CST);

                        if (
                            AccessPointIsExpired(&N->Point, __Params.Time = SECONDS(5)) &&
                            !KeyReferenceCount &&
                            !DownstreamReferenceCount
                        ) {
                            SlotHasWork = TRUE;
                            DLL_REMOVE(Slot->Head, Slot->Tail, N);
                            SLL_STACK_PUSH(C_SHARED->BlobStripesFreeNodes[StripeIndex], N);

                            if (N->MemPool)
                                ArenaRelease(N->MemPool);
                        }
                    }
                }
            }

            if (!SlotHasWork)
                break;
        }
    }
}
