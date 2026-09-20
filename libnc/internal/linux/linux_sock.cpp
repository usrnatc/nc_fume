#include "linux_platform.h"
#include "../../nc_sock.h"
#include "../../nc_memory.h"

INTERNAL int
SocketFDFromHandle(Handle SocketHandle)
{
    return (int) SocketHandle.V[0];
}

INTERNAL Handle
HandleFromSocketFD(int SocketFD)
{
    Handle Result = EMPTY_HANDLE_VALUE;

    if (SocketFD != -1)
        Result.V[0] = (u64) SocketFD;

    return Result;
}

INTERNAL void
SockAddrFromSocketAddress(
    SocketAddress Address, 
    OUT sockaddr_storage* SockAddr, 
    OUT socklen_t* SockAddrLength
) {
    MemSet(SockAddr, 0, sizeof(*SockAddr));

    if (Address.IsIPv6) {
        sockaddr_in6* SockAddrv6 = (sockaddr_in6*) SockAddr;

        SockAddrv6->sin6_family = AF_INET6;
        SockAddrv6->sin6_port = htons(Address.Port);
        MemCpy(&SockAddrv6->sin6_addr, Address.IP, 16);
        *SockAddrLength = sizeof(sockaddr_in6);
    } else {
        sockaddr_in* SockAddrv4 = (sockaddr_in*) SockAddr;

        SockAddrv4->sin_family = AF_INET;
        SockAddrv4->sin_port = htons(Address.Port);
        MemCpy(&SockAddrv4->sin_addr, Address.IP, 4);
        *SockAddrLength = sizeof(sockaddr_in);
    }
}

INTERNAL SocketAddress
SocketAddressFromSockAddr(sockaddr_storage* SockAddr)
{
    SocketAddress Result = {};

    if (SockAddr->ss_family == AF_INET6) {
        sockaddr_in6* SockAddrv6 = (sockaddr_in6*) SockAddr;

        Result.IsIPv6 = TRUE;
        Result.Port = ntohs(SockAddrv6->sin6_port);
        MemCpy(Result.IP, &SockAddrv6->sin6_addr, 16);
    } else {
        sockaddr_in* SockAddrv4 = (sockaddr_in*) SockAddr;

        Result.IsIPv6 = FALSE;
        Result.Port = ntohs(SockAddrv4->sin_port);
        MemCpy(Result.IP, &SockAddrv4->sin_addr, 4);
    }

    return Result;
}

b32 
SockInit(void)
{
    return TRUE;
}

void 
SockShutdown(void)
{}

Handle 
SocketOpen(SocketKind Kind, b32 IsIPv6)
{
    int Family = (IsIPv6) ? AF_INET6 : AF_INET;
    int Type = (Kind == SOCKET_UDP) ? SOCK_DGRAM : SOCK_STREAM;
    int Protocol = (Kind == SOCKET_UDP) ? IPPROTO_UDP : IPPROTO_TCP;
    int SockFD = socket(Family, Type | SOCK_CLOEXEC, Protocol);

    return HandleFromSocketFD(SockFD);
}

void 
SocketClose(Handle Socket)
{
    if (Socket == EMPTY_HANDLE_VALUE)
        return;

    close(SocketFDFromHandle(Socket));
}

// FIXME: should be more information about which Flag failed if any
b32 
SocketSetFlag(Handle Socket, SocketFlag Flags)
{
    if (Socket == EMPTY_HANDLE_VALUE)
        return FALSE;

    int SockFD = SocketFDFromHandle(Socket);
    b32 Result = TRUE;
    int FlagValue = 1;

    if (Flags & SOCKET_FLAG_REUSE_ADDRESS) {
        if (
            setsockopt(
                SockFD, 
                SOL_SOCKET, 
                SO_REUSEADDR, 
                &FlagValue, 
                sizeof(FlagValue)
            )
        ) {
            Result = FALSE;
        }
    }

    if (Flags & SOCKET_FLAG_BROADCAST) {
        if (
            setsockopt(
                SockFD, 
                SOL_SOCKET, 
                SO_BROADCAST, 
                &FlagValue, 
                sizeof(FlagValue)
            )
        ) {
            Result = FALSE;
        }
    }

    if (Flags & SOCKET_FLAG_NONBLOCKING) {
        int F = fcntl(SockFD, F_GETFL, 0);

        if (F == -1 || fcntl(SockFD, F_SETFL, F | O_NONBLOCK) == -1)
            Result = FALSE;
    }

    if (Flags & SOCKET_FLAG_NODELAY) {
        if (
            setsockopt(
                SockFD, 
                IPPROTO_TCP, 
                TCP_NODELAY, 
                &FlagValue, 
                sizeof(FlagValue)
            )
        ) {
            Result = FALSE;
        }
    }

    if (Flags & SOCKET_FLAG_KEEPALIVE) {
        if (
            setsockopt(
                SockFD, 
                SOL_SOCKET, 
                SO_KEEPALIVE, 
                &FlagValue, 
                sizeof(FlagValue)
            )
        ) {
            Result = FALSE;
        }
    }

    return Result;
}

b32 
SocketBind(Handle Socket, SocketAddress Address)
{
    if (Socket == EMPTY_HANDLE_VALUE)
        return FALSE;

    sockaddr_storage SockAddr = {};
    socklen_t SockAddrLength = 0;

    SockAddrFromSocketAddress(Address, &SockAddr, &SockAddrLength);

    int BindResult = bind(
        SocketFDFromHandle(Socket), 
        (sockaddr*) &SockAddr, 
        SockAddrLength
    );

    return !BindResult;
}

b32 
SocketListen(Handle Socket, u32 Backlog)
{
    if (Socket == EMPTY_HANDLE_VALUE)
        return FALSE;

    int ListenResult = listen(
        SocketFDFromHandle(Socket),
        (int) Backlog
    );

    return !ListenResult;
}

Handle 
SocketAccept(Handle Socket, OUT SocketAddress* ClientAddress)
{
    if (Socket == EMPTY_HANDLE_VALUE)
        return EMPTY_HANDLE_VALUE;

    sockaddr_storage SockAddr = {};
    socklen_t SockAddrLength = 0;
    int ClientSockFD = accept4(
        SocketFDFromHandle(Socket),
        (sockaddr*) &SockAddr,
        &SockAddrLength,
        SOCK_CLOEXEC
    );

    if (ClientSockFD != -1 && ClientAddress)
        *ClientAddress = SocketAddressFromSockAddr(&SockAddr);

    return HandleFromSocketFD(ClientSockFD);
}

b32 
SocketConnect(Handle Socket, SocketAddress Address)
{
    if (Socket == EMPTY_HANDLE_VALUE)
        return FALSE;

    sockaddr_storage SockAddr = {};
    socklen_t SockAddrLength = 0;

    SockAddrFromSocketAddress(Address, &SockAddr, &SockAddrLength);

    int ConnResult = connect(
        SocketFDFromHandle(Socket),
        (sockaddr*) &SockAddr,
        SockAddrLength
    );

    return !ConnResult;
}

void 
SocketShutdown(Handle Socket, SocketTransferKind TransferKind)
{
    if (Socket == EMPTY_HANDLE_VALUE)
        return;

    int ShutdownMethod = SHUT_RDWR;

    switch (TransferKind) {
        case SOCKET_XFER_KIND_READ: {
            ShutdownMethod = SHUT_RD;
        } break;

        case SOCKET_XFER_KIND_WRITE: {
            ShutdownMethod = SHUT_WR;
        } break;

        case SOCKET_XFER_KIND_BOTH: {
            ShutdownMethod = SHUT_RDWR;
        } break;
    }

    shutdown(SocketFDFromHandle(Socket), ShutdownMethod);
}

u64 
SocketSend(Handle Socket, void* Data, u64 DataSize)
{
    if (Socket == EMPTY_HANDLE_VALUE)
        return 0;

    int SockFD = SocketFDFromHandle(Socket);
    u64 BytesSent = 0;

    for ( ; BytesSent < DataSize; ) {
        u64 BytesToSend = DataSize - BytesSent;
        i64 Sent = send(
            SockFD, 
            (u8*) Data + BytesSent, 
            BytesToSend, 
            MSG_NOSIGNAL
        );

        if (Sent <= 0) {
            if (errno == EINTR)
                continue;

            break;
        }

        BytesSent += (u64) Sent;
    }

    return BytesSent;
}

u64 
SocketRecv(Handle Socket, void* Buffer, u64 BufferSize)
{
    if (Socket == EMPTY_HANDLE_VALUE)
        return 0;

    i64 Received = recv(
        SocketFDFromHandle(Socket),
        Buffer,
        BufferSize,
        0
    );

    if (Received < 0)
        return 0;

    return (u64) Received;
}

u64 
SocketSendTo(Handle Socket, void* Data, u64 DataSize, SocketAddress Dst)
{
    if (Socket == EMPTY_HANDLE_VALUE)
        return 0;

    sockaddr_storage SockAddr = {};
    socklen_t SockAddrLength = 0;

    SockAddrFromSocketAddress(Dst, &SockAddr, &SockAddrLength);

    i64 BytesSent = sendto(
        SocketFDFromHandle(Socket),
        Data,
        DataSize,
        MSG_NOSIGNAL,
        (sockaddr*) &SockAddr,
        SockAddrLength
    );

    if (BytesSent <= 0)
        return 0;

    return (u64) BytesSent;
}

SocketRecvResult 
SocketRecvFrom(Handle Socket, void* Buffer, u64 BufferSize)
{
    SocketRecvResult Result = {};

    if (Socket == EMPTY_HANDLE_VALUE)
        return Result;

    sockaddr_storage SockAddr = {};
    socklen_t SockAddrLength = 0;
    i64 BytesReceived = recvfrom(
        SocketFDFromHandle(Socket),
        Buffer,
        BufferSize,
        0,
        (sockaddr*) &SockAddr,
        &SockAddrLength
    );

    if (BytesReceived > 0) {
        Result.BytesRead = (u64) BytesReceived;
        Result.From = SocketAddressFromSockAddr(&SockAddr);
    }

    return Result;
}

SocketAddress 
SockAddrIPv4(u8 A, u8 B, u8 C, u8 D, u16 Port)
{
    SocketAddress Result = {
        {
            A,
            B,
            C,
            D
        },
        Port,
        FALSE
    };

    return Result;
}

SocketAddress 
SockAddrAny(u16 Port)
{
    SocketAddress Result = {};

    Result.Port = Port;

    return Result;
}

SocketAddress 
SockAddrLocal(u16 Port)
{
    SocketAddress Result = SockAddrIPv4(127, 0, 0, 1, Port);

    return Result;
}

b32 
SocketResolveAddr(
    Arena* MemPool, 
    Str8 Host, 
    u16 Port, 
    OUT SocketAddress* Address
) {
    b32 Result = FALSE;
    TempArena Scratch = GetScratch(&MemPool, 1);
    Str8 HostCopy = ArenaPushStrCpy(Scratch.MemPool, Host);
    addrinfo Hints = {};
    addrinfo* AddrInfo = NULL;

    Hints.ai_family = AF_UNSPEC;
    Hints.ai_socktype = SOCK_STREAM;

    int AddrInfoResult = getaddrinfo(
        (char*) HostCopy.Str,
        NULL,
        &Hints,
        &AddrInfo
    );

    if (!AddrInfoResult && AddrInfo) {
        sockaddr_storage Storage = {};

        MemCpy(&Storage, AddrInfo->ai_addr, AddrInfo->ai_addrlen);
        *Address = SocketAddressFromSockAddr(&Storage);
        Address->Port = Port;
        Result = TRUE;
        freeaddrinfo(AddrInfo);
    }

    ReleaseScratch(Scratch);

    return Result;
}
