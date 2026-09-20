#include "nc_types.h"
#include "nc_ring.h"
#include "nc_memory.h"
#include "nc_sync.h"

ByteRing* 
ByteRingAlloc(Arena* MemPool, u64 Size)
{
    ByteRing* Result = ArenaPushArrayZero(MemPool, ByteRing, 1);

    Result->Size = Size;
    Result->Base = ArenaPushArrayZero(MemPool, u8, Result->Size);

    return Result;
}

u64 
ByteRingWrappedWrite(
    u8* RingBase, 
    u64 RingSize, 
    u64 RingPosition, 
    void* Data, 
    u64 DataSize
) {
    ASSERT(DataSize <= RingSize);

    u64 RingOffset = RingPosition % RingSize;
    u64 BytesBeforeSplit = RingSize - RingOffset;
    u64 BeforeSplitBytes = MIN(BytesBeforeSplit, DataSize);
    u64 AfterSplitBytes = DataSize - BeforeSplitBytes;
    void* BeforeSplitDataPtr = Data;
    void* AfterSplitDataPtr = ((u8*) Data + BeforeSplitBytes);

    MemCpy(
        RingBase + RingOffset,
        BeforeSplitDataPtr,
        BeforeSplitBytes
    );
    MemCpy(
        RingBase,
        AfterSplitDataPtr,
        AfterSplitBytes
    );

    return DataSize;
}

u64 
ByteRingWrappedRead(
    u8* RingBase, 
    u64 RingSize, 
    u64 RingPosition, 
    void* Data, 
    IN u64 ReadSize
) {
    ASSERT(ReadSize <= RingSize);

    u64 RingOffset = RingPosition % RingSize;
    u64 BytesBeforeSplit = RingSize - RingOffset;
    u64 BeforeSplitBytes = MIN(BytesBeforeSplit, ReadSize);
    u64 AfterSplitBytes = ReadSize - BeforeSplitBytes;

    MemCpy(
        Data,
        RingBase + RingOffset,
        BeforeSplitBytes
    );
    MemCpy(
        (u8*) Data + BeforeSplitBytes,
        RingBase,
        AfterSplitBytes
    );

    return ReadSize;
}

b32 
ByteRingTryWrite(ByteRing* Ring, u64 Size, void* Data)
{
    u64 BytesUnconsumed = (Ring->WritePosition - Ring->ReadPosition);
    u64 BytesAvailable = Ring->Size - BytesUnconsumed;
    b32 Result = FALSE;

    if (BytesAvailable >= Size) {
        Result = TRUE;
        Ring->WritePosition += ByteRingWrappedWrite(
            Ring->Base,
            Ring->Size,
            Ring->WritePosition,
            Data,
            Size
        );
    }

    return Result;
}

b32 
ByteRingTryRead(ByteRing* Ring, u64 Size, OUT void* Data)
{
    u64 BytesUnconsumed = (Ring->WritePosition - Ring->ReadPosition);
    b32 Result = FALSE;

    if (BytesUnconsumed >= Size) {
        Result = TRUE;
        Ring->ReadPosition += ByteRingWrappedRead(
            Ring->Base,
            Ring->Size,
            Ring->ReadPosition,
            Data,
            Size
        );
    }

    return Result;
}

SafeByteRing* 
SafeByteRingAlloc(Arena* MemPool, u64 Size)
{
    SafeByteRing* Result = ArenaPushArrayZero(
        MemPool,
        SafeByteRing,
        1
    );

    Result->Ring = ByteRingAlloc(MemPool, Size);
    Result->Mtx = MutexAlloc();
    Result->CondVar = CondVarAlloc();

    return Result;
}

void 
SafeByteRingRelease(SafeByteRing* SafeRing)
{
    MutexDestroy(SafeRing->Mtx);
    CondVarDestroy(SafeRing->CondVar);
}

ByteRingGuard 
SafeByteRingOpen(SafeByteRing* SafeRing)
{
    ByteRingGuard Result = {
        SafeRing
    };

    MutexLock(SafeRing->Mtx);

    return Result;
}

void 
SafeByteRingClose(ByteRingGuard* Guard)
{
    MutexUnlock(Guard->SafeRing->Mtx);
}

b32 
SafeByteRingTryWrite(ByteRingGuard* Guard, u64 Size, void* Data)
{
    b32 Result = ByteRingTryWrite(Guard->SafeRing->Ring, Size, Data);

    if (Result)
        CondVarBroadcast(Guard->SafeRing->CondVar);

    return Result;
}

b32 
SafeByteRingTryRead(ByteRingGuard* Guard, u64 Size, OUT void* Data)
{
    b32 Result = ByteRingTryRead(Guard->SafeRing->Ring, Size, Data);

    if (Result)
        CondVarBroadcast(Guard->SafeRing->CondVar);

    return Result;
}

b32 
SafeByteRingWriteOrWait(ByteRingGuard* Guard, u64 Size, void* Data, u64 EndTimeUSecs)
{
    b32 WriteOkay = FALSE;

    for ( ; !WriteOkay; ) {
        WriteOkay = SafeByteRingTryWrite(Guard, Size, Data);

        if (TimeNow() >= EndTimeUSecs)
            break;

        if (!WriteOkay) {
            CondVarWait(
                Guard->SafeRing->CondVar, 
                Guard->SafeRing->Mtx, 
                EndTimeUSecs
            );
        }
    }

    return WriteOkay;
}

b32 
SafeByteRingReadOrWait(ByteRingGuard* Guard, u64 Size, void* Data, u64 EndTimeUSecs)
{
    b32 ReadOkay = FALSE;

    for ( ; !ReadOkay; ) {
        ReadOkay = SafeByteRingTryRead(Guard, Size, Data);

        if (TimeNow() >= EndTimeUSecs)
            break;

        if (!ReadOkay) {
            CondVarWait(
                Guard->SafeRing->CondVar,
                Guard->SafeRing->Mtx,
                EndTimeUSecs
            );
        }
    }

    return ReadOkay;
}
