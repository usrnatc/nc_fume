#include "win32_platform.h"
#include "../../nc_sock.h"
#include "../../nc_system.h"

struct Win32TLSState {
    CredHandle                Credentials;
    CtxHandle                 Context;
    b32                       IsCredentialValid;
    b32                       IsContextValid;
    SecPkgContext_StreamSizes StreamSizes;
    HCERTSTORE                CertStore;
    PCCERT_CONTEXT            Certificate;
};
STATIC_ASSERT(sizeof(MEMBER(TLSSession, Memory)) >= sizeof(Win32TLSState), Win32TLSStateSizeCheck);

INTERNAL SOCKET
SocketFromHandle(Handle SocketHandle)
{
    SOCKET Result = (SOCKET) SocketHandle.V[0];

    return Result;
}

INTERNAL Handle
HandleFromSocket(SOCKET Socket)
{
    Handle Result = {};

    if (Socket != INVALID_SOCKET)
        Result.V[0] = (u64) Socket;

    return Result;
}

INTERNAL void
SockAddrFromSocketAddress(
    SocketAddress Address, 
    OUT SOCKADDR_STORAGE* SockAddr, 
    OUT i64* SockAddrLength
) {
    MemSet(SockAddr, 0, sizeof(*SockAddr));

    if (Address.IsIPv6) {
        sockaddr_in6* SockAddrv6 = (sockaddr_in6*) SockAddr;

        SockAddrv6->sin6_family = AF_INET6;
        SockAddrv6->sin6_port = htons(Address.Port);
        MemCpy(&SockAddrv6->sin6_addr, Address.IP, 16);
        *SockAddrLength = sizeof(sockaddr_in6);
    } else {
        SOCKADDR_IN* SockAddrv4 = (SOCKADDR_IN*) SockAddr;

        SockAddrv4->sin_family = AF_INET;
        SockAddrv4->sin_port = htons(Address.Port);
        MemCpy(&SockAddrv4->sin_addr, Address.IP, 4);
        *SockAddrLength = sizeof(SOCKADDR_IN);
    }
}

INTERNAL SocketAddress
SocketAddressFromSockAddr(SOCKADDR_STORAGE* SockAddr)
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
    WSADATA WSAData = {};
    int Result = WSAStartup(MAKEWORD(2, 2), &WSAData);

    return !Result;
}

void 
SockShutdown(void)
{
    WSACleanup();
}

Handle 
SocketOpen(SocketKind Kind, b32 IsIPv6)
{    
    int Family = (IsIPv6) ? AF_INET6 : AF_INET;
    int Type = (Kind == SOCKET_UDP) ? SOCK_DGRAM : SOCK_STREAM;
    int Protocol = (Kind == SOCKET_UDP) ? IPPROTO_UDP : IPPROTO_TCP;
    SOCKET Socket = socket(Family, Type, Protocol);

    return HandleFromSocket(Socket);
}

void 
SocketClose(Handle Socket)
{
    if (Socket == EMPTY_HANDLE_VALUE)
        return;

    closesocket(SocketFromHandle(Socket));
}

b32 
SocketSetFlag(Handle Socket, SocketFlag Flags)
{
    if (Socket == EMPTY_HANDLE_VALUE)
        return FALSE;

    SOCKET Sock = SocketFromHandle(Socket);
    b32 Result = TRUE;
    int FlagValue = 1;

    if (Flags & SOCKET_FLAG_REUSE_ADDRESS) {
        if (
            setsockopt(
                Sock, 
                SOL_SOCKET, 
                SO_REUSEADDR, 
                (CHAR const*) &FlagValue, 
                sizeof(FlagValue)
            )
        ) {
            Result = FALSE;
        }
    }

    if (Flags & SOCKET_FLAG_BROADCAST) {
        if (
            setsockopt(
                Sock, 
                SOL_SOCKET, 
                SO_BROADCAST, 
                (CHAR const*) &FlagValue, 
                sizeof(FlagValue)
            )
        ) {
            Result = FALSE;
        }
    }

    if (Flags & SOCKET_FLAG_NONBLOCKING) {
        ULONG Mode = 1;

        if (
            ioctlsocket(
                Sock,
                FIONBIO,
                &Mode
            )
        ) {
            Result = FALSE;
        }
    }

    if (Flags & SOCKET_FLAG_NODELAY) {
        if (
            setsockopt(
                Sock, 
                IPPROTO_TCP, 
                TCP_NODELAY, 
                (CHAR const*) &FlagValue, 
                sizeof(FlagValue)
            )
        ) {
            Result = FALSE;
        }
    }

    if (Flags & SOCKET_FLAG_KEEPALIVE) {
        if (
            setsockopt(
                Sock, 
                SOL_SOCKET, 
                SO_KEEPALIVE, 
                (CHAR const*) &FlagValue, 
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

    SOCKADDR_STORAGE SockAddr = {};
    i64 SockAddrLength = 0;

    SockAddrFromSocketAddress(Address, &SockAddr, &SockAddrLength);

    int BindResult = bind(
        SocketFromHandle(Socket), 
        (SOCKADDR*) &SockAddr, 
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
        SocketFromHandle(Socket),
        (int) Backlog
    );

    return !ListenResult;
}

Handle 
SocketAccept(Handle Socket, OUT SocketAddress* ClientAddress)
{
    if (Socket == EMPTY_HANDLE_VALUE)
        return EMPTY_HANDLE_VALUE;

    SOCKADDR_STORAGE SockAddr = {};
    i64 SockAddrLength = (i64) sizeof(SockAddr);
    SOCKET Client = accept(
        SocketFromHandle(Socket),
        (SOCKADDR*) &SockAddr,
        (INT*) &SockAddrLength
    );

    if (Client != INVALID_SOCKET && ClientAddress)
        *ClientAddress = SocketAddressFromSockAddr(&SockAddr);

    return HandleFromSocket(Client);
}

b32 
SocketConnect(Handle Socket, SocketAddress Address)
{
    if (Socket == EMPTY_HANDLE_VALUE)
        return FALSE;

    SOCKADDR_STORAGE SockAddr = {};
    i64 SockAddrLength = 0;

    SockAddrFromSocketAddress(Address, &SockAddr, &SockAddrLength);

    int ConnResult = connect(
        SocketFromHandle(Socket), 
        (SOCKADDR*) &SockAddr, 
        SockAddrLength
    );

    return !ConnResult;
}

void 
SocketShutdown(Handle Socket, SocketTransferKind TransferKind)
{
    if (Socket == EMPTY_HANDLE_VALUE)
        return;

    int ShutdownMethod = SD_BOTH;

    switch (TransferKind) {
        case SOCKET_XFER_KIND_READ: { 
            ShutdownMethod = SD_RECEIVE;
        } break;

        case SOCKET_XFER_KIND_WRITE: { 
            ShutdownMethod = SD_SEND; 
        } break;

        case SOCKET_XFER_KIND_BOTH: { 
            ShutdownMethod = SD_BOTH; 
        } break;
    }

    shutdown(SocketFromHandle(Socket), ShutdownMethod);
}

u64 
SocketSend(Handle Socket, void* Data, u64 DataSize)
{
    if (Socket == EMPTY_HANDLE_VALUE)
        return 0;

    SOCKET Sock = SocketFromHandle(Socket);
    u64 BytesSent = 0;

    for ( ; BytesSent < DataSize; ) {
        i64 ChunkSize = MIN((i64) (DataSize - BytesSent), I64_MAX);
        i64 Sent = send(
            Sock,
            (char*) Data + BytesSent, 
            ChunkSize, 
            0
        );

        if (Sent == SOCKET_ERROR)
            break;

        BytesSent += (u64) Sent;
    }

    return BytesSent;
}

u64 
SocketRecv(Handle Socket, void* Buffer, u64 BufferSize)
{
    if (Socket == EMPTY_HANDLE_VALUE)
        return 0;

    i64 ChunkSize = MIN((i64) BufferSize, I64_MAX);
    i64 Receieved = recv(
        SocketFromHandle(Socket),
        (char*) Buffer,
        ChunkSize,
        0
    );

    if (Receieved == SOCKET_ERROR || Receieved < 0)
        return 0;

    return (u64) Receieved;
}

u64 
SocketSendTo(Handle Socket, void* Data, u64 DataSize, SocketAddress Dst)
{
    if (Socket == EMPTY_HANDLE_VALUE)
        return 0;

    SOCKADDR_STORAGE SockAddr = {};
    i64 SockAddrLength = 0;

    SockAddrFromSocketAddress(Dst, &SockAddr, &SockAddrLength);

    i64 ChunkSize = MIN((i64) DataSize, I64_MAX);
    i64 Sent = sendto(
        SocketFromHandle(Socket),
        (char*) Data,
        ChunkSize,
        0,
        (SOCKADDR*) &SockAddr,
        SockAddrLength
    );

    if (Sent == SOCKET_ERROR)
        return 0;

    return (u64) Sent;
}

SocketRecvResult 
SocketRecvFrom(Handle Socket, void* Buffer, u64 BufferSize)
{
    SocketRecvResult Result = {};

    if (Socket == EMPTY_HANDLE_VALUE)
        return Result;

    SOCKADDR_STORAGE SockAddr = {};
    i64 SockAddrLength = sizeof(SockAddr);
    i64 ChunkSize = MIN((i64) BufferSize, I64_MAX);
    i64 Received = recvfrom(
        SocketFromHandle(Socket),
        (char*) Buffer,
        ChunkSize,
        0,
        (SOCKADDR*) &SockAddr,
        (INT*) &SockAddrLength
    );

    if (Received > 0) {
        Result.BytesRead = (u64) Received;
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
    return SockAddrIPv4(127, 0, 0, 1, Port);
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
    ADDRINFOA Hints = {};
    ADDRINFOA* AddrInfo = NULL;

    Hints.ai_family = AF_UNSPEC;
    Hints.ai_socktype = SOCK_STREAM;

    int AddrInfoResult = getaddrinfo(
        (char*) HostCopy.Str,
        NULL,
        &Hints,
        &AddrInfo
    );

    if (!AddrInfoResult && AddrInfo) {
        SOCKADDR_STORAGE Storage = {};

        MemCpy(&Storage, AddrInfo->ai_addr, AddrInfo->ai_addrlen);
        *Address = SocketAddressFromSockAddr(&Storage);
        Address->Port = Port;
        Result = TRUE;
        freeaddrinfo(AddrInfo);
    }

    ReleaseScratch(Scratch);

    return Result;
}

b32
TLSInit(void)
{
    return TRUE;
}

void
TLSShutdownGlobal(void)
{}

INTERNAL PCCERT_CONTEXT
LoadCertPFX(u8 const* Data, u64 DataSize, OUT HCERTSTORE* CertStore)
{
    CRYPT_DATA_BLOB PFXBlob = {};

    PFXBlob.cbData = (DWORD) DataSize;
    PFXBlob.pbData = (BYTE*) Data;

    HCERTSTORE Store = PFXImportCertStore(
        &PFXBlob,
        L"",
        CRYPT_EXPORTABLE
    );

    if (!Store)
        return NULL;

    PCCERT_CONTEXT Result = CertFindCertificateInStore(
        Store,
        X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
        0,
        CERT_FIND_ANY,
        NULL,
        NULL
    );

    if (CertStore)
        *CertStore = Store;
    else if (!Result)
        CertCloseStore(Store, 0);

    return Result;
}

INTERNAL HCERTSTORE
LoadCAStore(u8 const* Data, u64 DataSize)
{
    HCERTSTORE Result = CertOpenStore(
        CERT_STORE_PROV_MEMORY,
        0,
        0,
        CERT_STORE_CREATE_NEW_FLAG,
        NULL
    );

    if (!Result)
        return NULL;

    if (
        !CertAddEncodedCertificateToStore(
            Result,
            X509_ASN_ENCODING,
            Data,
            (DWORD) DataSize,
            CERT_STORE_ADD_ALWAYS,
            NULL
        )
    ) {
        CRYPT_DATA_BLOB Blob = {};

        Blob.cbData = (DWORD) DataSize;
        Blob.pbData = (BYTE*) Data;

        HCERTSTORE Store = CertOpenStore(
            CERT_STORE_PROV_PKCS7,
            X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
            0,
            0,
            &Blob
        );

        if (Store) {
            PCCERT_CONTEXT Ctx = NULL;

            for (;;) {
                Ctx = CertEnumCertificatesInStore(Store, Ctx);

                if (!Ctx)
                    break;

                CertAddCertificateContextToStore(
                    Result,
                    Ctx,
                    CERT_STORE_ADD_ALWAYS,
                    NULL
                );
            }

            CertCloseStore(Store, 0);
        }
    }

    return Result;
}

TLSSession 
TLSHandshake(Handle Socket, TLSConfig Cfg)
{
    TLSSession Session = {};

    Session.Socket = Socket;
    Session.State = TLS_STATE_ERROR;
    MemZero(Session.Memory, sizeof(Session.Memory));
 
    if (Socket == EMPTY_HANDLE_VALUE)
        return Session;
 
    Win32TLSState* State = (Win32TLSState*) Session.Memory;
    HCERTSTORE PFXStore = NULL;                                                 // NOTE: load our certificate if provided (PFX/PKCS12)

    if (Cfg.CertData && Cfg.CertDataSize > 0) {
        State->Certificate = LoadCertPFX(
            Cfg.CertData, 
            Cfg.CertDataSize, 
            &PFXStore
        );
    }
 
    // NOTE: load CA store for peer verification
    if (Cfg.CAData && Cfg.CADataSize)
        State->CertStore = LoadCAStore(Cfg.CAData, Cfg.CADataSize);
 
    SCHANNEL_CRED SchCred = {};

    SchCred.dwVersion = SCHANNEL_CRED_VERSION;
    SchCred.grbitEnabledProtocols = SP_PROT_TLS1_3;
    SchCred.dwFlags = SCH_CRED_NO_DEFAULT_CREDS;
 
    if (Cfg.PeerCertRequired)
        SchCred.dwFlags |= SCH_CRED_MANUAL_CRED_VALIDATION;
    else
        SchCred.dwFlags |= SCH_CRED_NO_SERVERNAME_CHECK;
 
    if (State->Certificate) {
        SchCred.cCreds = 1;
        SchCred.paCred = &State->Certificate;
    }
 
    ULONG Direction = (Cfg.Role == TLS_ROLE_KIND_SERVER)
        ? SECPKG_CRED_INBOUND
        : SECPKG_CRED_OUTBOUND;
    SECURITY_STATUS CredStatus = AcquireCredentialsHandleW(
        NULL,
        (LPWSTR) UNISP_NAME_W,
        Direction,
        NULL,
        &SchCred,
        NULL,
        NULL,
        &State->Credentials,
        NULL
    );
 
    if (CredStatus != SEC_E_OK) {
        if (PFXStore)
            CertCloseStore(PFXStore, 0);

        return Session;
    }
 
    State->IsCredentialValid = TRUE;
 
    // NOTE: perform the handshake loop
    SOCKET Sock = SocketFromHandle(Socket);
    SecBufferDesc SendBufferDesc = {};
    SecBuffer SendBuffer = {};

    SendBuffer.BufferType = SECBUFFER_TOKEN;
    SendBufferDesc.ulVersion = SECBUFFER_VERSION;
    SendBufferDesc.cBuffers = 1;
    SendBufferDesc.pBuffers = &SendBuffer;
 
    ULONG CtxReq = (
        ISC_REQ_SEQUENCE_DETECT |
        ISC_REQ_REPLAY_DETECT |
        ISC_REQ_CONFIDENTIALITY |
        ISC_REQ_STREAM
    );
 
    if (Cfg.PeerCertRequired)
        CtxReq |= ISC_REQ_MUTUAL_AUTH;
 
    u8 RecvBuffer[16384];
    u64 RecvBufferUsed = 0;
    b32 IsFirstCall = TRUE;
    ULONG CtxAttr = 0;
 
    for (;;) {
        SECURITY_STATUS Status;
 
        if (Cfg.Role == TLS_ROLE_KIND_CLIENT) {
            SecBufferDesc RecvBufferDesc = {};
            SecBuffer RecvBuffers[2] = {};
 
            if (!IsFirstCall) {
                RecvBuffers[0].BufferType = SECBUFFER_TOKEN;
                RecvBuffers[0].cbBuffer = (ULONG) RecvBufferUsed;
                RecvBuffers[0].pvBuffer = RecvBuffer;
                RecvBuffers[1].BufferType = SECBUFFER_EMPTY;
                RecvBufferDesc.ulVersion = SECBUFFER_VERSION;
                RecvBufferDesc.cBuffers = 2;
                RecvBufferDesc.pBuffers = RecvBuffers;
            }
 
            SendBuffer.cbBuffer = 0;
            SendBuffer.pvBuffer = NULL;
            Status = InitializeSecurityContextW(
                &State->Credentials,
                IsFirstCall ? NULL : &State->Context,
                NULL,
                CtxReq,
                0,
                0,
                IsFirstCall ? NULL : &RecvBufferDesc,
                0,
                &State->Context,
                &SendBufferDesc,
                &CtxAttr,
                NULL
            );
        } else {
            SecBufferDesc RecvBufferDesc = {};
            SecBuffer RecvBuffers[2] = {};

            RecvBuffers[0].BufferType = SECBUFFER_TOKEN;
            RecvBuffers[0].cbBuffer = (ULONG) RecvBufferUsed;
            RecvBuffers[0].pvBuffer = RecvBuffer;
            RecvBuffers[1].BufferType = SECBUFFER_EMPTY;
            RecvBufferDesc.ulVersion = SECBUFFER_VERSION;
            RecvBufferDesc.cBuffers = 2;
            RecvBufferDesc.pBuffers = RecvBuffers;
            SendBuffer.cbBuffer = 0;
            SendBuffer.pvBuffer = NULL;
 
            ULONG SrvReq = (
                ASC_REQ_SEQUENCE_DETECT |
                ASC_REQ_REPLAY_DETECT |
                ASC_REQ_CONFIDENTIALITY |
                ASC_REQ_STREAM
            );
 
            if (Cfg.PeerCertRequired)
                SrvReq |= ASC_REQ_MUTUAL_AUTH;
 
            Status = AcceptSecurityContext(
                &State->Credentials,
                IsFirstCall ? NULL : &State->Context,
                &RecvBufferDesc,
                SrvReq,
                0,
                &State->Context,
                &SendBufferDesc,
                &CtxAttr,
                NULL
            );
        }
 
        State->IsContextValid = TRUE;
        IsFirstCall = FALSE;
 
        // NOTE: send any output token to peer
        if (SendBuffer.cbBuffer > 0 && SendBuffer.pvBuffer) {
            u64 Sent = SocketSend(
                Socket, 
                SendBuffer.pvBuffer, 
                SendBuffer.cbBuffer
            );

            FreeContextBuffer(SendBuffer.pvBuffer);
            SendBuffer.pvBuffer = NULL;
 
            if (!Sent) {
                Session.State = TLS_STATE_ERROR;
                break;
            }
        }
 
        if (Status == SEC_E_OK) {
            // NOTE: handshake complete
            QueryContextAttributes(
                &State->Context, 
                SECPKG_ATTR_STREAM_SIZES, 
                &State->StreamSizes
            );
            Session.State = TLS_STATE_ESTABLISHED;
            Session.PeerVerify = TLS_VERIFY_OK;
            break;
        } else if (
            Status == SEC_I_CONTINUE_NEEDED || 
            Status == SEC_E_INCOMPLETE_MESSAGE
        ) {
            // NOTE: need more data from peer
            u64 Received = SocketRecv(
                Socket,
                RecvBuffer + RecvBufferUsed,
                sizeof(RecvBuffer) - RecvBufferUsed
            );
 
            if (!Received) {
                Session.State = TLS_STATE_ERROR;
                break;
            }
 
            RecvBufferUsed += Received;
        } else {
            // NOTE: handshake failed
            Session.State = TLS_STATE_ERROR;
            Session.PeerVerify = TLS_VERIFY_ERROR;
            break;
        }
    }
 
    if (PFXStore)
        CertCloseStore(PFXStore, 0);
 
    return Session;
}

u64 
TLSSend(TLSSession* Session, IN void const* Buffer, u64 BufferSize)
{
    if (!Session || Session->State != TLS_STATE_ESTABLISHED)
        return 0;
 
    Win32TLSState* State = (Win32TLSState*) Session->Memory;
    SOCKET Sock = SocketFromHandle(Session->Socket);
    u64 TotalSent = 0;
    u8 const* Src = (u8 const*) Buffer;
 
    for ( ; TotalSent < BufferSize; ) {
        ULONG ChunkSize = (ULONG) MIN(
            BufferSize - TotalSent,
            (u64) State->StreamSizes.cbMaximumMessage
        );
        u64 BufSize = (
            State->StreamSizes.cbHeader + 
            ChunkSize + 
            State->StreamSizes.cbTrailer
        );
        u8 SendBuf[16384 + 128];                                                // TODO: use a persistent send buffer for large messages
 
        if (BufSize > sizeof(SendBuf)) {
            // NOTE: message too large for stack buffer
            break;
        }
 
        SecBuffer Buffers[3] = {};

        Buffers[0].BufferType = SECBUFFER_STREAM_HEADER;
        Buffers[0].cbBuffer = State->StreamSizes.cbHeader;
        Buffers[0].pvBuffer = SendBuf;
        Buffers[1].BufferType = SECBUFFER_DATA;
        Buffers[1].cbBuffer = ChunkSize;
        Buffers[1].pvBuffer = SendBuf + State->StreamSizes.cbHeader;
        Buffers[2].BufferType = SECBUFFER_STREAM_TRAILER;
        Buffers[2].cbBuffer = State->StreamSizes.cbTrailer;
        Buffers[2].pvBuffer = SendBuf + State->StreamSizes.cbHeader + ChunkSize;
        MemCpy(Buffers[1].pvBuffer, Src + TotalSent, ChunkSize);
 
        SecBufferDesc BufferDesc = {};

        BufferDesc.ulVersion = SECBUFFER_VERSION;
        BufferDesc.cBuffers = 3;
        BufferDesc.pBuffers = Buffers;
 
        SECURITY_STATUS Status = EncryptMessage(
            &State->Context,
            0, 
            &BufferDesc, 
            0
        );
 
        if (Status != SEC_E_OK) {
            Session->State = TLS_STATE_ERROR;
            break;
        }
 
        u64 EncryptedSize = (
            Buffers[0].cbBuffer + 
            Buffers[1].cbBuffer + 
            Buffers[2].cbBuffer
        );
        u64 Sent = SocketSend(Session->Socket, SendBuf, EncryptedSize);
 
        if (Sent != EncryptedSize) {
            Session->State = TLS_STATE_ERROR;
            break;
        }
 
        TotalSent += ChunkSize;
    }
 
    return TotalSent;
}

u64 
TLSRecv(TLSSession* Session, OUT void* Buffer, u64 BufferSize)
{
    if (!Session || Session->State != TLS_STATE_ESTABLISHED)
        return 0;
 
    Win32TLSState* State = (Win32TLSState*) Session->Memory;
    u8 RecvBuf[16384 + 128];
    u64 RecvBufUsed = 0;
    u64 Received = SocketRecv(Session->Socket, RecvBuf, sizeof(RecvBuf));
 
    if (!Received)
        return 0;
 
    RecvBufUsed = Received;
 
    SecBuffer Buffers[4] = {};

    Buffers[0].BufferType = SECBUFFER_DATA;
    Buffers[0].cbBuffer = (ULONG) RecvBufUsed;
    Buffers[0].pvBuffer = RecvBuf;
    Buffers[1].BufferType = SECBUFFER_EMPTY;
    Buffers[2].BufferType = SECBUFFER_EMPTY;
    Buffers[3].BufferType = SECBUFFER_EMPTY;
 
    SecBufferDesc BufferDesc = {};

    BufferDesc.ulVersion = SECBUFFER_VERSION;
    BufferDesc.cBuffers = 4;
    BufferDesc.pBuffers = Buffers;
 
    SECURITY_STATUS Status = DecryptMessage(
        &State->Context, 
        &BufferDesc, 
        0, 
        NULL
    );
 
    if (Status == SEC_E_OK) {
        // NOTE: find the data buffer in the output
        for (u32 Index = 0; Index < 4; Index++) {
            if (
                Buffers[Index].BufferType == SECBUFFER_DATA && 
                Buffers[Index].cbBuffer > 0
            ) {
                u64 CopySize = MIN(
                    (u64) Buffers[Index].cbBuffer, 
                    BufferSize
                );

                MemCpy(Buffer, Buffers[Index].pvBuffer, CopySize);

                return CopySize;
            }
        }
    } else if (Status == SEC_I_CONTEXT_EXPIRED) {
        Session->State = TLS_STATE_CLOSED;
    } else {
        Session->State = TLS_STATE_ERROR;
    }
 
    return 0;
}

void 
TLSCloseSession(TLSSession* Session)
{
    if (!Session)
        return;
 
    Win32TLSState* State = (Win32TLSState*) Session->Memory;
 
    // NOTE: send close_notify
    if (State->IsContextValid) {
        DWORD ShutdownToken = SCHANNEL_SHUTDOWN;
        SecBuffer SendBuffer = {};

        SendBuffer.BufferType = SECBUFFER_TOKEN;
        SendBuffer.cbBuffer = sizeof(ShutdownToken);
        SendBuffer.pvBuffer = &ShutdownToken;
 
        SecBufferDesc SendBufferDesc = {};

        SendBufferDesc.ulVersion = SECBUFFER_VERSION;
        SendBufferDesc.cBuffers = 1;
        SendBufferDesc.pBuffers = &SendBuffer;
        ApplyControlToken(&State->Context, &SendBufferDesc);
        SendBuffer.BufferType = SECBUFFER_TOKEN;
        SendBuffer.cbBuffer = 0;
        SendBuffer.pvBuffer = NULL;
 
        ULONG ContextAttr = 0;
        SECURITY_STATUS Status = InitializeSecurityContextW(
            &State->Credentials,
            &State->Context,
            NULL,
            (
                ISC_REQ_SEQUENCE_DETECT | 
                ISC_REQ_REPLAY_DETECT | 
                ISC_REQ_CONFIDENTIALITY | 
                ISC_REQ_STREAM
            ),
            0, 
            0,
            NULL,
            0,
            &State->Context,
            &SendBufferDesc,
            &ContextAttr,
            NULL
        );

        UNUSED(ContextAttr);
 
        if (SendBuffer.pvBuffer && SendBuffer.cbBuffer > 0) {
            SocketSend(
                Session->Socket, 
                SendBuffer.pvBuffer, 
                SendBuffer.cbBuffer
            );
            FreeContextBuffer(SendBuffer.pvBuffer);
        }
 
        DeleteSecurityContext(&State->Context);
        State->IsContextValid = FALSE;
    }
 
    if (State->IsCredentialValid) {
        FreeCredentialsHandle(&State->Credentials);
        State->IsCredentialValid = FALSE;
    }
 
    if (State->Certificate) {
        CertFreeCertificateContext(State->Certificate);
        State->Certificate = NULL;
    }
 
    if (State->CertStore) {
        CertCloseStore(State->CertStore, 0);
        State->CertStore = NULL;
    }
 
    Session->State = TLS_STATE_CLOSED;
    // NOTE: does NOT close the underlying socket
}

b32 
TLSLoadCRL(TLSSession* Session, u8 const* CRLData, u64 CRLDataSize)
{
    if (!Session || !CRLData || !CRLDataSize)
        return FALSE;
 
    Win32TLSState* State = (Win32TLSState*) Session->Memory;
 
    if (!State->CertStore)
        return FALSE;
 
    return (
        !!CertAddEncodedCRLToStore(
            State->CertStore,
            X509_ASN_ENCODING,
            CRLData,
            (DWORD) CRLDataSize,
            CERT_STORE_ADD_ALWAYS,
            NULL
        )
    );
}

b32 
TLSGetPeerCertInfo(TLSSession* Session, OUT TLSCertInfo* CertInfo)
{
    if (!Session || !CertInfo)
        return FALSE;
 
    Win32TLSState* State = (Win32TLSState*) Session->Memory;
 
    if (!State->IsContextValid)
        return FALSE;
 
    MemZero(CertInfo, sizeof(*CertInfo));
 
    PCCERT_CONTEXT PeerCert = NULL;
    SECURITY_STATUS Status = QueryContextAttributes(
        &State->Context,
        SECPKG_ATTR_REMOTE_CERT_CONTEXT,
        &PeerCert
    );
 
    if (Status != SEC_E_OK || !PeerCert)
        return FALSE;
 
    // NOTE: SHA-256 fingerprint of DER-encoded cert
    DWORD HashSize = 32;

    CryptHashCertificate2(
        BCRYPT_SHA256_ALGORITHM,
        0,
        NULL,
        PeerCert->pbCertEncoded,
        PeerCert->cbCertEncoded,
        CertInfo->FingerPrint,
        &HashSize
    );
 
    // NOTE: extract subject CN
    DWORD CNLen = CertGetNameStringA(
        PeerCert,
        CERT_NAME_ATTR_TYPE,
        0,
        (void*) szOID_COMMON_NAME,
        (LPSTR) CertInfo->SubjectCN,
        256
    );

    if (CNLen > 0) 
        CertInfo->SubjectCNLength = CNLen - 1;
 
    // NOTE: extract issuer CN
    DWORD IssuerLen = CertGetNameStringA(
        PeerCert,
        CERT_NAME_ATTR_TYPE,
        CERT_NAME_ISSUER_FLAG,
        (void*) szOID_COMMON_NAME,
        (LPSTR) CertInfo->IssuerCN,
        256
    );

    if (IssuerLen > 0) 
        CertInfo->IssuerCNLength = IssuerLen - 1;
 
    // NOTE: validity times
    FILETIME NotBefore = PeerCert->pCertInfo->NotBefore;
    FILETIME NotAfter = PeerCert->pCertInfo->NotAfter;
    u64 FTBefore = ((u64) NotBefore.dwHighDateTime << 32) | NotBefore.dwLowDateTime;
    u64 FTAfter = ((u64) NotAfter.dwHighDateTime << 32) | NotAfter.dwLowDateTime;

    CertInfo->NotBeforeUNIX = (FTBefore / 10000000ULL) - 11644473600ULL;
    CertInfo->NotAfterUNIX = (FTAfter  / 10000000ULL) - 11644473600ULL;
    CertFreeCertificateContext(PeerCert);
 
    return TRUE;
}
