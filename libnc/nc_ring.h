#if !defined(__NC_RING_H__)
#define __NC_RING_H__

#include "nc_types.h"
#include "nc_system.h"

// @defines____________________________________________________________________
#define ByteRingTryWriteStruct(X, Y)           ByteRingTryWrite((X), sizeof(*(Y)), (Y))
#define ByteRingTryReadStruct(X, Y)            ByteRingTryRead((X), sizeof(*(Y)), (Y))
#define SafeByteRingTryWriteStruct(X, Y)       SafeByteRingTryWrite((X), sizeof(*(Y)), (Y))
#define SafeByteRingTryReadStruct(X, Y)        SafeByteRingTryRead((X), sizeof(*(Y)), (Y))
#define SafeByteRingWriteStructOrWait(X, Y, Z) SafeByteRingWriteOrWait((X), sizeof(*(Y)), (Y), Z)
#define SafeByteRingReadStructOrWait(X, Y, Z)  SafeByteRingReadOrWait((X), sizeof(*(Y)), (Y), Z)

// @types______________________________________________________________________
struct ByteRing {
    u8* Base;
    u64 Size;
    u64 WritePosition;
    u64 ReadPosition;
};

struct SafeByteRing {
    ByteRing* Ring;
    Handle    Mtx;
    Handle    CondVar;
};

struct ByteRingGuard {
    SafeByteRing* SafeRing;
};

// @runtime____________________________________________________________________


// @functions__________________________________________________________________
ByteRing* ByteRingAlloc(Arena* MemPool, u64 Size);
u64 ByteRingWrappedWrite(u8* RingBase, u64 RingSize, u64 RingPosition, void* Data, u64 DataSize);
u64 ByteRingWrappedRead(u8* RingBase, u64 RingSize, u64 RingPosition, void* Data, u64 ReadSize);
b32 ByteRingTryWrite(ByteRing* Ring, u64 Size, void* Data);
b32 ByteRingTryRead(ByteRing* Ring, u64 Size, OUT void* Data);
SafeByteRing* SafeByteRingAlloc(Arena* MemPool, u64 Size);
void SafeByteRingRelease(SafeByteRing* SafeRing);
ByteRingGuard SafeByteRingOpen(SafeByteRing* SafeRing);
void SafeByteRingClose(ByteRingGuard* Guard);
b32 SafeByteRingTryWrite(ByteRingGuard* Guard, u64 Size, void* Data);
b32 SafeByteRingTryRead(ByteRingGuard* Guard, u64 Size, OUT void* Data);
b32 SafeByteRingWriteOrWait(ByteRingGuard* Guard, u64 Size, void* Data, u64 EndTimeUSecs);
b32 SafeByteRingReadOrWait(ByteRingGuard* Guard, u64 Size, void* Data, u64 EndTimeUSecs);

#endif // __NC_RING_H__
