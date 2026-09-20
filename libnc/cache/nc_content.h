#if !defined(__NC_CONTENT_H__)
#define __NC_CONTENT_H__

#include "../nc_types.h"
#include "../nc_system.h"
#include "../nc_thread.h"

// @defines____________________________________________________________________
struct Arena;

#define C_KEY_HASH_HISTORY_COUNT            64
#define C_KEY_HASH_HISTORY_STRONG_REF_COUNT  2

// @types______________________________________________________________________
struct CRoot {
    u64 V[1];
};

struct CID {
    u128 V[1];
};

struct CKey {
    CRoot Root;
    u64   __Unused;
    CID   ID;
};

struct CStripe {
    Arena* MemPool;
    Handle RWMtx;
    Handle CondVar;
};

struct CRootIDChunkNode {
    CRootIDChunkNode* Next;
    CID*              V;
    u64               Count;
    u64               Capacity;
};

struct CRootIDChunkList {
    CRootIDChunkNode* Head;
    CRootIDChunkNode* Tail;
    u64               ChunkCount;
    u64               TotalCount;
};

struct CRootNode {
    CRootNode*       Next;
    CRootNode*       Prev;
    Arena*           MemPool;
    CRoot            Root;
    CRootIDChunkList IDs;
};

struct CRootSlot {
    CRootNode* Head;
    CRootNode* Tail;
};

struct CKeyNode {
    CKeyNode* Next;
    CKeyNode* Prev;
    CKey      Key;
    u128      HashHistory[C_KEY_HASH_HISTORY_COUNT];
    u64       HashHistoryGeneration;
};

struct CKeySlot {
    CKeyNode* Head;
    CKeyNode* Tail;
};

struct CBlobNode {
    CBlobNode*  Next;
    CBlobNode*  Prev;
    u128        BlobHash;
    Arena*      MemPool;
    Str8        Data;
    AccessPoint Point;
    u64         KeyReferenceCount;
    u64         DownstreamReferenceCount;
};

struct CBlobSlot {
    CBlobNode* Head;
    CBlobNode* Tail;
};

struct CShared {
    Arena*      MemPool;
    u64         BlobSlotsCount;
    u64         BlobStripesCount;
    CBlobSlot*  BlobSlots;
    CStripe*    BlobStripes;
    CBlobNode** BlobStripesFreeNodes;
    u64         KeySlotsCount;
    u64         KeyStripesCount;
    CKeySlot*   KeySlots;
    CStripe*    KeyStripes;
    CKeyNode**  KeyStripesFreeNodes;
    u64         RootSlotsCount;
    u64         RootStripesCount;
    CRootSlot*  RootSlots;
    CStripe*    RootStripes;
    CRootNode** RootStripesFreeNodes;
    u64         RootIDGeneration;
};

// @runtime____________________________________________________________________
extern CShared* C_SHARED;

// @functions__________________________________________________________________
CID CIDAlloc(u64 V0, u64 V1);
CKey CKeyAlloc(CRoot Root, CID ID);

#if defined(NC_LANG_CPP)
    b32 operator==(CID IDA, CID IDB);
    b32 operator!=(CID IDA, CID IDB);
    b32 operator==(CKey KeyA, CKey KeyB);
    b32 operator!=(CKey KeyA, CKey KeyB);
#endif

void CInit(void);
CRoot CRootAlloc(void);
void CRootRelease(CRoot Root);
u128 CSubmitData(CKey Key, Arena** DataMemPool, Str8 Data);
void CCloseKey(CKey Key);
void CHashDownstreamInc(u128 CHash);
void CHashDownstreamDec(u128 CHash);
u128 CHashFromKey(CKey Key, u64 RewindCount);
Str8 CDataFromHash(Access* Acc, u128 CHash);
void CAsyncTick(void);

#endif // __NC_CONTENT_H__
