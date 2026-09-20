#if !defined(__NC_OBJECT_CACHE_H__)
#define __NC_OBJECT_CACHE_H__

#include "../nc_types.h"
#include "../nc_string.h"
#include "../nc_thread.h"
#include "../nc_sync.h"

struct OCObject;

// @defines____________________________________________________________________
typedef OCObject OCCreateFuncType(Str8 Key, b32* CancelSignal, OUT b32* Retry, OUT u64* Generation);
typedef void OCDestroyFuncType(OCObject Obj);

typedef u32 OCFlag;
enum : u32 {
    OC_WAIT_FOR_FRESH = (1 << 0),
    OC_HIGH_PRIORITY  = (1 << 1),
    OC_WIDE           = (1 << 2)
};

#define OCObjectFromKey(X, Y, Z, W, K, ...) [&]() -> OCObject { \
    OCObjectParams __Params = {};                               \
    __Params.CreateFunc = Z;                                    \
    __Params.DestroyFunc = W;                                   \
    __Params.EvictionThresholdUSecs = MILLION(2);               \
    __VA_ARGS__;                                                \
    return __OCObjectFromKey((X), (Y), &__Params, (K));         \
}()

// @types______________________________________________________________________
struct OCObject {
    u64 V[4];
};

struct OCObjectParams {
    OCCreateFuncType*  CreateFunc;
    OCDestroyFuncType* DestroyFunc;
    u64                SlotsCount;
    u64                Generation;
    u64                EvictionThresholdUSecs;
OUT b32*               Stale;
    OCFlag             Flags;
};

struct OCRequest {
    Str8              Key;
    u64               Generation;
    b32*              CancelSignal;
    OCCreateFuncType* CreateFunc;
};

struct OCRequestNode {
    OCRequestNode* Next;
    OCRequest      V;
};

struct OCNode {
    OCNode*     Next;
    OCNode*     Prev;
    Str8        Key;
    u64         LastRequestedGeneration;
    u64         LastCompletedGeneration;
    OCObject      V;
    AccessPoint Point;
    u64         WorkingCount;
    u64         CompletionCount;
    u64         EvictionThresholdUSecs;
    b32         Cancelled;
    u64         __Reserved;
};

struct OCSlot {
    OCNode* Head;
    OCNode* Tail;
};

struct OCCache {
    OCCache*           Next;
    OCCreateFuncType*  CreateFunc;
    OCDestroyFuncType* DestroyFunc;
    u64                SlotsCount;
    OCSlot*            Slots;
    StripeArray        Stripes;
};

struct OCRequestBatch {
    Handle         Mtx;
    Arena*         MemPool;
    OCRequestNode* HeadWide;
    OCRequestNode* TailWide;
    OCRequestNode* HeadThin;
    OCRequestNode* TailThin;
    u64            WideCount;
    u64            ThinCount;
};

struct OCShared {
    Arena*         MemPool;
    u64            CacheSlotsCount;
    OCCache**      CacheSlots;
    StripeArray    CacheStripes;
    OCRequestBatch RequestBatches[2];
    Handle         CancelThread;
    Handle         CancelThreadSemaphore;
};

// @runtime____________________________________________________________________
extern OCShared* OC_SHARED;

// @functions__________________________________________________________________
void OCInit(void);
OCObject __OCObjectFromKey(Access* Acc, Str8 Key, OCObjectParams* Params, u64 EndTimeUSecs);
void OCAsyncTick(void);
void OCCancelThreadEntryPoint(void* Params);

#endif // __NC_OBJECT_CACHE_H__
