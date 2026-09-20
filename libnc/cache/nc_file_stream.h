#if !defined(__NC_FILE_STREAM_H__)
#define __NC_FILE_STREAM_H__

#include "../nc_types.h"
#include "../nc_string.h"
#include "../nc_sync.h"

// @defines____________________________________________________________________
struct OCObject;
struct CKey;

// @types______________________________________________________________________
struct FSNode {
    FSNode* Next;
    Str8    Path;
    u64     Generation;
    u64     LastModifiedTimestamp;
    u64     Size;
};

struct FSSlot {
    FSNode* Head;
    FSNode* Tail;
};

struct FSShared {
    Arena*      MemPool;
    u64         ChangeGeneration;
    u64         SlotsCount;
    FSSlot*     Slots;
    StripeArray Stripes;
};

// @runtime____________________________________________________________________
extern FSShared* FS_SHARED;

// @functions__________________________________________________________________
void FSInit(void);
u64 FSChangeGeneration(void);
OCObject FSObjectAlloc(Str8 Key, b32* CancelSignal, OUT b32* Retry, OUT u64* Generation);
void FSObjectRelease(OCObject Object);
CKey FSKeyFromPathRange(Str8 Path, r1u64 Range, u64 EndTimeUSecs);
u128 FSHashFromPathRange(Str8 Path, r1u64 Range, u64 EndTimeUSecs);
void FSAsyncTick(void);

#endif // __NC_FILE_STREAM_H__
