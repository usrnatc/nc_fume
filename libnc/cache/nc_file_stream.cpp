#include "nc_file_stream.h"
#include "../nc_atomics.h"
#include "nc_object_cache.h"
#include "nc_content.h"
#include "../nc_file.h"
#include "../nc_memory.h"

FSShared* FS_SHARED = NULL;

void 
FSInit(void)
{
    Arena* MemPool = ArenaAlloc();

    FS_SHARED = ArenaPushArrayZero(MemPool, FSShared, 1);
    FS_SHARED->MemPool = MemPool;
    FS_SHARED->ChangeGeneration = 1;
    FS_SHARED->SlotsCount = 1024;
    FS_SHARED->Slots = ArenaPushArrayZero(MemPool, FSSlot, FS_SHARED->SlotsCount);
    FS_SHARED->Stripes = StripeArrayAlloc(MemPool);
}

u64 
FSChangeGeneration(void)
{
    return AtomicLoadU64(&FS_SHARED->ChangeGeneration, MEM_ORDER_SEQ_CST);
}

OCObject 
FSObjectAlloc(Str8 Key, b32* CancelSignal, OUT b32* Retry, OUT u64* Generation)
{
    TempArena Scratch = GetScratch(NULL, 0);
    Str8 Path = {};
    r1u64 Range = {};
    u64 KeyReadOffset = 0;

    KeyReadOffset += SerialPopStruct(Key, KeyReadOffset, &Path.Size);
    Path.Str = ArenaPushArrayZero(Scratch.MemPool, u8, Path.Size);
    KeyReadOffset += SerialPop(Key, KeyReadOffset, Path.Str, Path.Size, 1);
    KeyReadOffset += SerialPopStruct(Key, KeyReadOffset, &Range);

    b32 FileOkay = FALSE;
    FileProperties FilePropsPre = {};

    if (!LaneIndex()) {
        FilePropsPre = SystemGetFileProperties(Path);
        FileOkay = !!FilePropsPre.Modified;
    }

    LaneSyncU64(&FileOkay, 0);

    Arena* DataMemPool = NULL;
    u64 DataBufferSize = 0;
    u8* DataBuffer = NULL;

    if (FileOkay) {
        if (!LaneIndex()) {
            u64 RangeSize = Length(Range);
            u64 ReadSize = MIN(FilePropsPre.Size - Range.Min, RangeSize);
            u64 DataMemPoolSize = ReadSize + ARENA_HEADER_SIZE;

            DataMemPoolSize += KB(4) - 1;
            DataMemPoolSize -= DataMemPoolSize % KB(4);
            DataMemPool = ArenaAlloc(
                __Params.ReserveSize = DataMemPoolSize,
                __Params.CommitSize = DataMemPoolSize
            );
            DataBufferSize = ReadSize;
            DataBuffer = ArenaPushArray(DataMemPool, u8, DataBufferSize);
        }

        LaneSyncU64(&DataBuffer, 0);
        LaneSyncU64(&DataBufferSize, 0);
    }

    Handle File = {};

    if (FileOkay) {
        if (!LaneIndex()) {
            File = SystemOpenFile(
                SYS_ACCESS_READ | SYS_ACCESS_SHARE_READ | SYS_ACCESS_SHARE_WRITE, 
                Path
            );
        }

        LaneSyncU64(&File, 0);
    }

    b32 FileHandleOkay = (File != EMPTY_HANDLE_VALUE);
    u64 TotalBytesRead = 0;

    if (FileHandleOkay) {
        u64 *TotalBytesReadPtr = NULL;

        if (!LaneIndex())
            TotalBytesReadPtr = &TotalBytesRead;

        LaneSyncU64(&TotalBytesReadPtr, 0);

        r1u64 LaneReadRange = LaneRange(DataBufferSize);
        u64 BytesRead = SystemReadFile(
            File, 
            Shift(LaneReadRange, Range.Min), 
            DataBuffer + LaneReadRange.Min
        );

        AtomicAddFetchU64(TotalBytesReadPtr, BytesRead, MEM_ORDER_SEQ_CST);
        LaneSync();
        LaneSyncU64(&TotalBytesRead, 0);
    }

    if (FileHandleOkay)
        if (!LaneIndex())
            SystemCloseFile(File);

    FileProperties FilePropsPost = {};

    if (!LaneIndex())
        FilePropsPost = SystemGetFileProperties(Path);

    CKey ContentKey = {};

    ContentKey.ID.V[0] = BigHash(Key);

    b32 ReadOkay = FALSE;

    if (FileOkay) {
        if (!LaneIndex()) {
            ReadOkay = (
                FilePropsPre.Modified == FilePropsPost.Modified &&
                FilePropsPre.Size == FilePropsPost.Size &&
                DataBufferSize == TotalBytesRead &&
                (
                    FileHandleOkay ||
                    FilePropsPre.Kind & SYS_FILE_IS_DIR
                )
            );

            if (!ReadOkay) {
                *Retry = TRUE;
                ArenaRelease(DataMemPool);
                MemSet(&ContentKey, 0, sizeof(ContentKey));
            } else {
                CSubmitData(ContentKey, &DataMemPool, Str(DataBuffer, DataBufferSize));
            }
        }

        LaneSync();
    }

    u64 PathHash = Hash(Path);

    if (!LaneIndex() && ReadOkay) {
        u64 SlotIndex = PathHash % FS_SHARED->SlotsCount;
        FSSlot* Slot = &FS_SHARED->Slots[SlotIndex];
        Stripe* PathStripe = StripeFromSlotIndex(&FS_SHARED->Stripes, SlotIndex);

        LOCK_SCOPE_W(PathStripe->RWMtx) {
            FSNode* Node = NULL;

            for (FSNode* N = Slot->Head; N; N = N->Next) {
                if (StrMatch(N->Path, Path, 0)) {
                    Node = N;
                    break;
                }
            }

            if (!Node) {
                Node = (FSNode*) PathStripe->Free;

                if (Node)
                    PathStripe->Free = Node->Next;
                else
                    Node = ArenaPushArray(PathStripe->MemPool, FSNode, 1);

                MemSet(Node, 0, sizeof(*Node));
                Node->Path = ArenaPushStrCpy(PathStripe->MemPool, Path);
                SLL_QUEUE_PUSH(Slot->Head, Slot->Tail, Node);
            }

            Node->LastModifiedTimestamp = FilePropsPre.Modified;
            Node->Size = FilePropsPre.Size;
        }
    }

    LaneSync();

    OCObject Object = {};

    STATIC_ASSERT(sizeof(ContentKey) == sizeof(Object), ObjectKeySizeCheck);

    MemCpy(&Object, &ContentKey, sizeof(Object));
    ReleaseScratch(Scratch);

    return Object;
}

void 
FSObjectRelease(OCObject Object)
{
    CKey Key = {};

    MemCpy(&Key, &Object, sizeof(Key));
    CCloseKey(Key);
}

CKey 
FSKeyFromPathRange(Str8 Path, r1u64 Range, u64 EndTimeUSecs)
{
    CKey Result = {};
    TempArena Scratch = GetScratch(NULL, 0);
    Access* Acc = AccessOpen();
    Str8List KeyParts = {};

    ListPush(Scratch.MemPool, &KeyParts, Str8Struct(&Path.Size));
    ListPush(Scratch.MemPool, &KeyParts, Path);
    ListPush(Scratch.MemPool, &KeyParts, Str8Struct(&Range));

    Str8 Key = StrListJoin(Scratch.MemPool, &KeyParts, NULL);
    u64 Generation = 0;
    u64 PathHash = Hash(Path);
    u64 SlotIndex = PathHash % FS_SHARED->SlotsCount;
    FSSlot* Slot = &FS_SHARED->Slots[SlotIndex];
    Stripe* PathStripe = StripeFromSlotIndex(&FS_SHARED->Stripes, SlotIndex);

    LOCK_SCOPE_R(PathStripe->RWMtx) {
        for (FSNode* N = Slot->Head; N; N = N->Next) {
            if (StrMatch(Path, N->Path, 0)) {
                Generation = N->Generation;
                break;
            }
        }
    }

    OCObject Object = OCObjectFromKey(
        Acc, 
        Key, 
        FSObjectAlloc, 
        FSObjectRelease, 
        EndTimeUSecs
    );

    MemCpy(&Result, &Object, sizeof(Result));
    AccessClose(Acc);
    ReleaseScratch(Scratch);

    return Result;
}

u128 
FSHashFromPathRange(Str8 Path, r1u64 Range, u64 EndTimeUSecs)
{
    u128 Result = {};
    CKey Key = FSKeyFromPathRange(Path, Range, EndTimeUSecs);

    for (
        u32 RewindIndex = 0; 
        RewindIndex < C_KEY_HASH_HISTORY_COUNT; 
        ++RewindIndex
    ) {
        Result = CHashFromKey(Key, RewindIndex);

        if (Result != u128{})
            break;
    }

    return Result;
}

void 
FSAsyncTick(void)
{
    r1u64 Range = LaneRange(FS_SHARED->SlotsCount);

    for (u64 SlotIndex = Range.Min; SlotIndex < Range.Max; ++SlotIndex) {
        FSSlot* Slot = &FS_SHARED->Slots[SlotIndex];
        Stripe* CurrStripe = StripeFromSlotIndex(&FS_SHARED->Stripes, SlotIndex);

        for (b32 WriteMode = FALSE; WriteMode <= TRUE; ++WriteMode) {
            b32 FoundWork = FALSE;

            if (!WriteMode) {
                LOCK_SCOPE_R(CurrStripe->RWMtx) {
                    for (FSNode* N = Slot->Head; N; N = N->Next) {
                        FileProperties FileProps = SystemGetFileProperties(N->Path);

                        if (FileProps.Modified != N->LastModifiedTimestamp)
                            FoundWork = TRUE;
                    }
                }
            } else {
                LOCK_SCOPE_W(CurrStripe->RWMtx) {
                    for (FSNode* N = Slot->Head; N; N = N->Next) {
                        FileProperties FileProps = SystemGetFileProperties(N->Path);

                        if (FileProps.Modified != N->LastModifiedTimestamp) {
                            FoundWork = TRUE;
                            ++N->Generation;
                            AtomicIncFetchU64(&FS_SHARED->ChangeGeneration, MEM_ORDER_SEQ_CST);
                        }
                    }
                }
            }

            if (!FoundWork)
                break;
        }
    }
}
