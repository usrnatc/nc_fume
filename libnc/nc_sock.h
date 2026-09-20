#if !defined(__NC_SOCK_H__)
#define __NC_SOCK_H__

#include "nc_types.h"
#include "nc_string.h"
#include "nc_system.h"

// @defines____________________________________________________________________
#define SOCKET_DEFAULT_BACKLOG 128

typedef u32 SocketKind;
enum : u32 {
    SOCKET_TCP = 0,
    SOCKET_UDP = 1
};

typedef u32 SocketTransferKind;
enum : u32 {
    SOCKET_XFER_KIND_READ  = 0,
    SOCKET_XFER_KIND_WRITE = 1,
    SOCKET_XFER_KIND_BOTH  = 2
};

typedef u32 SocketFlag;
enum : u32 {
    SOCKET_FLAG_REUSE_ADDRESS = (1 << 0),
    SOCKET_FLAG_BROADCAST     = (1 << 1),
    SOCKET_FLAG_NONBLOCKING   = (1 << 2),
    SOCKET_FLAG_NODELAY       = (1 << 3),
    SOCKET_FLAG_KEEPALIVE     = (1 << 4)
};

typedef u32 TLSRoleKind;
enum : u32 {
    TLS_ROLE_KIND_CLIENT,
    TLS_ROLE_KIND_SERVER
};

typedef u32 TLSState;
enum : u32 {
    TLS_STATE_NONE,
    TLS_STATE_HANDSHAKE,
    TLS_STATE_ESTABLISHED,
    TLS_STATE_CLOSED,
    TLS_STATE_ERROR
};

typedef u32 TLSVerifyResult;
enum : u32 {
    TLS_VERIFY_OK,
    TLS_VERIFY_CERT_EXPIRED,
    TLS_VERIFY_CERT_UNTRUSTED,
    TLS_VERIFY_CERT_REVOKED,
    TLS_VERIFY_CERT_CN_MISMATCH,
    TLS_VERIFY_CERT_SELF_SIGNED,
    TLS_VERIFY_ERROR
};

// @types______________________________________________________________________
struct SocketAddress {
    u8  IP[16];
    u16 Port;
    b32 IsIPv6;
};

struct SocketRecvResult {
    SocketAddress From;
    u64           BytesRead;
};

struct TLSConfig {
    u8 const*   CertData;
    u64         CertDataSize;
    u8 const*   KeyData;
    u64         KeyDataSize;
    u8 const*   CAData;
    u64         CADataSize;
    u8 const*   CRLData;
    u64         CRLDataSize;
    TLSRoleKind Role;
    b32         PeerCertRequired;
};

struct TLSSession {
    Handle          Socket;
    TLSState        State;
    TLSVerifyResult PeerVerify;
    u8              Memory[256];
};

struct TLSCertInfo {
    u8  FingerPrint[32];
    u8  SubjectCN[256];
    u64 SubjectCNLength;
    u8  IssuerCN[256];
    u64 IssuerCNLength;
    u64 NotBeforeUNIX;
    u64 NotAfterUNIX;
};

struct SocketInfo {
    Handle        Socket;
    SocketAddress Address;
};

// @runtime____________________________________________________________________


// @functions__________________________________________________________________
b32 SockInit(void);
void SockShutdown(void);
Handle SocketOpen(SocketKind Kind, b32 IsIPv6 = FALSE);
void SocketClose(Handle Socket);
b32 SocketSetFlag(Handle Socket, SocketFlag Flags);
b32 SocketBind(Handle Socket, SocketAddress Address);
b32 SocketListen(Handle Socket, u32 Backlog = SOCKET_DEFAULT_BACKLOG);
Handle SocketAccept(Handle Socket, OUT SocketAddress* ClientAddress);
b32 SocketConnect(Handle Socket, SocketAddress Address);
void SocketShutdown(Handle Socket, SocketTransferKind TransferKind);
u64 SocketSend(Handle Socket, void* Data, u64 DataSize);
u64 SocketRecv(Handle Socket, void* Buffer, u64 BufferSize);
u64 SocketSendTo(Handle Socket, void* Data, u64 DataSize, SocketAddress Dst);
SocketRecvResult SocketRecvFrom(Handle Socket, void* Buffer, u64 BufferSize);
SocketAddress SockAddrIPv4(u8 A, u8 B, u8 C, u8 D, u16 Port);
SocketAddress SockAddrAny(u16 Port);
SocketAddress SockAddrLocal(u16 Port);
b32 SocketResolveAddr(Arena* MemPool, Str8 Host, u16 Port, OUT SocketAddress* Address);
b32 TLSInit(void);
void TLSShutdownGlobal(void);
TLSSession TLSHandshake(Handle Socket, TLSConfig Cfg);
u64 TLSSend(TLSSession* Session, IN void const* Buffer, u64 BufferSize);
u64 TLSRecv(TLSSession* Session, OUT void* Buffer, u64 BufferSize);
void TLSCloseSession(TLSSession* Session);
b32 TLSLoadCRL(TLSSession* Session, u8 const* CRLData, u64 CRLDataSize);
b32 TLSGetPeerCertInfo(TLSSession* Session, OUT TLSCertInfo* CertInfo);

#endif // __NC_SOCK_H__
