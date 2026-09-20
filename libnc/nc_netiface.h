#if !defined(__NC_NETIFACE_H__)
#define __NC_NETIFACE_H__

#include "nc_types.h"
#include "nc_sock.h"

// @defines____________________________________________________________________
#define NET_IFACE_NAME_MAX_LENGTH 256

typedef u32 NetIFaceFlag;
enum : u32 {
    NET_IFACE_FLAG_UP          = (1 << 0),
    NET_IFACE_FLAG_LOOPBACK    = (1 << 1),
    NET_IFACE_FLAG_MULTICAST   = (1 << 2),
    NET_IFACE_FLAG_BROADCAST   = (1 << 3),
    NET_IFACE_FLAG_PROMISCUOUS = (1 << 4),
    NET_IFACE_FLAG_RUNNING     = (1 << 5)
};

typedef u32 NetLinkKind;
enum : u32 {
    NET_LINK_KIND_ETHERNET,
    NET_LINK_KIND_WIFI,
    NET_LINK_KIND_LOOPBACK,
    NET_LINK_KIND_OTHER
};

typedef u32 NetIFaceIterKind;
enum : u32 {
    NET_IFACE_ITER_KIND_SKIP_LOOPBACK = (1 << 0),
    NET_IFACE_ITER_KIND_SKIP_DOWN     = (1 << 1),
    NET_IFACE_ITER_KIND_DONE          = (1 << 2)
};

// @types______________________________________________________________________
struct NetIFaceAddr {
    SocketAddress Addr;
    SocketAddress NetMask;
    SocketAddress Broadcast;
};

struct NetIFaceAddrNode {
    NetIFaceAddrNode* Next;
    NetIFaceAddrNode* Prev;
    NetIFaceAddr      V;
};

struct NetIFaceAddrList {
    NetIFaceAddrNode* Head;
    NetIFaceAddrNode* Tail;
    u64               Count;
};

struct NetIFaceInfo {
    Str8             Name;
    u32              Index;
    NetIFaceFlag     Flags;
    NetLinkKind      LinkKind;
    u8               MAC[6];
    u32              MTU;
    NetIFaceAddrList Addresses;
};

struct NetIFaceIter {
    NetIFaceIterKind Kind;
    u8               Memory[256];
};

// @runtime____________________________________________________________________


// @functions__________________________________________________________________
NetIFaceIter* NetIFaceIterBegin(Arena* MemPool, NetIFaceIterKind Method);

#endif // __NC_NETIFACE_H__
