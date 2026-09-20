#include "win32_platform.h"
#include "../nc_types.h"
#include "../nc_http.h"
#include "../nc_atomics.h"
#include "../nc_sync.h"
#include "../nc_thread.h"
#include "../nc_string.h"
#include "../nc_memory.h"

#define WIN32_HTTP_MAX_ACTIVE_REQUESTS 16

typedef u32 Win32HTTPRequestStatusKind;
enum : u32 {
    WIN32_HTTP_REQUEST_STATUS_KIND_NULL,
    WIN32_HTTP_REQUEST_STATUS_KIND_SEND_PENDING,
    WIN32_HTTP_REQUEST_STATUS_KIND_SENT,
    WIN32_HTTP_REQUEST_STATUS_KIND_RECEIVE_PENDING,
    WIN32_HTTP_REQUEST_STATUS_KIND_RECEIVED,
    WIN32_HTTP_REQUEST_STATUS_KIND_DATA_SIZE_PENDING,
    WIN32_HTTP_REQUEST_STATUS_KIND_DATA_SIZE,
    WIN32_HTTP_REQUEST_STATUS_KIND_READ_PENDING,
    WIN32_HTTP_REQUEST_STATUS_KIND_COMPLETED,
    WIN32_HTTP_REQUEST_STATUS_KIND_FAILED
};

struct Win32HTTPRequest {
    b32                        Active;
    Arena*                     MemPool;
    SafeByteRing*              SafeRing;
    u64                        ID;
    Win32HTTPRequestStatusKind Status;
    HINTERNET                  ConnectionHandle;
    HINTERNET                  RequestHandle;
    void*                      OptionalData;
    u64                        OptionalDataSize;
    HTTPStatusCode             StatusCode;
    u64                        TotalResponseBytesSent;
    u64                        TotalResponseBytes;
    u64                        MemPoolBeginBodyReadPosition;
    void*                      NextBodyChunk;
    u64                        NextBodyChunkSize;
    Arena*                     CompletedBodyMemPool;
    Str8List                   CompletedBodyChunks;
};

struct Win32HTTPResponse {
    Win32HTTPResponse* Next;
    SafeByteRing*      SafeRing;
    Str8               Record;
};

struct Win32HTTPState {
    Arena*           MemPool;
    HINTERNET        SessionHandle;
    SafeByteRing*    RequestSafeRing;
    Win32HTTPRequest ActiveRequests[WIN32_HTTP_MAX_ACTIVE_REQUESTS];
};

global Win32HTTPState* WIN32_HTTP_STATE = NULL;

INTERNAL void
Win32HTTPStatusFunc(
    HINTERNET hInternet,
    DWORD_PTR dwContext,
    DWORD dwInternetStatus,
    LPVOID lpvStatusInformation,
    DWORD dwStatusInformationLength
) {
    Win32HTTPRequest* Request = (Win32HTTPRequest*) dwContext;
    b32 ReLoop = TRUE;

    switch (dwInternetStatus) {
        default: {
            ReLoop = FALSE;
        } break;

        case  WINHTTP_CALLBACK_STATUS_REQUEST_ERROR: {
            AtomicExchangeU32(
                &Request->Status, 
                WIN32_HTTP_REQUEST_STATUS_KIND_FAILED,
                MEM_ORDER_CST_SEQ
            );
        } break;

        case  WINHTTP_CALLBACK_STATUS_REQUEST_SENT: {
            AtomicCASU32(
                &Request->Status,
                WIN32_HTTP_REQUEST_STATUS_KIND_SENT,
                WIN32_HTTP_REQUEST_STATUS_KIND_SEND_PENDING,
                MEM_ORDER_CST_SEQ,
                MEM_ORDER_CST_SEQ
            );
        } break;

        case  WINHTTP_CALLBACK_STATUS_HEADERS_AVAILABLE: {
            Request->NextBodyChunk = NULL;
            Request->NextBodyChunkSize = 0;
            AtomicCASU32(
                &Request->Status,
                WIN32_HTTP_REQUEST_STATUS_KIND_RECEIVED,
                WIN32_HTTP_REQUEST_STATUS_KIND_RECEIVE_PENDING,
                MEM_ORDER_CST_SEQ,
                MEM_ORDER_CST_SEQ
            );
        } break;

        case  WINHTTP_CALLBACK_STATUS_DATA_AVAILABLE: {
            DWORD* SizePtr = (DWORD*) lpvStatusInformation;

            Request->NextBodyChunkSize = *SizePtr;

            if (*SizePtr) {
                AtomicExchangeU32(
                    &Request->Status,
                    WIN32_HTTP_REQUEST_STATUS_KIND_DATA_SIZE,
                    MEM_ORDER_CST_SEQ
                );
            } else {
                AtomicExchangeU32(
                    &Request->Status,
                    WIN32_HTTP_REQUEST_STATUS_KIND_COMPLETED,
                    MEM_ORDER_CST_SEQ
                );
            }
        } break;

        case  WINHTTP_CALLBACK_STATUS_READ_COMPLETE: {
            Request->NextBodyChunkSize = dwStatusInformationLength;
            AtomicExchangeU32(
                &Request->Status,
                WIN32_HTTP_REQUEST_STATUS_KIND_RECEIVED,
                MEM_ORDER_CST_SEQ
            );
        } break;
    }

    if (ReLoop) {
        AtomicExchangeU32(&ASYNC_LOOP_REPEAT, TRUE, MEM_ORDER_CST_SEQ);
        CondVarBroadcast(ASYNC_TICK_BEGIN_COND_VAR);
    }
}

void 
HTTPInit(void)
{
    Arena* MemPool = ArenaAlloc();

    WIN32_HTTP_STATE = ArenaPushArrayZero(
        MemPool,
        Win32HTTPState,
        1
    );
    WIN32_HTTP_STATE->MemPool = MemPool;
    WIN32_HTTP_STATE->RequestSafeRing = SafeByteRingAlloc(MemPool, MB(1));

    for (u64 Index = 0; Index < WIN32_HTTP_MAX_ACTIVE_REQUESTS; ++Index) {
        WIN32_HTTP_STATE->ActiveRequests[Index].MemPool = ArenaAlloc();
        WIN32_HTTP_STATE->ActiveRequests[Index].CompletedBodyMemPool = ArenaAlloc();
    }

    WIN32_HTTP_STATE->SessionHandle = WinHttpOpen(
        NULL,
        WINHTTP_ACCESS_TYPE_NO_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS,
        WINHTTP_FLAG_ASYNC
    );
    WinHttpSetStatusCallback(
        WIN32_HTTP_STATE->SessionHandle,
        Win32HTTPStatusFunc,
        WINHTTP_CALLBACK_FLAG_ALL_NOTIFICATIONS,
        (DWORD_PTR) 0
    );
}

void 
HTTPAsyncTick(void)
{
    if (!LaneIndex()) {
        for (;;) {
            TempArena Scratch = GetScratch(NULL, 0);
            u64 FreeSlotIndex = 0;

            for (
                u64 Index = 0;
                Index < WIN32_HTTP_MAX_ACTIVE_REQUESTS;
                ++Index
            ) {
                if (!WIN32_HTTP_STATE->ActiveRequests[Index].Active) {
                    FreeSlotIndex = Index + 1;
                    break;
                }
            }

            Str8 RequestData = {};

            if (FreeSlotIndex) {
                ByteRingGuard Guard = SafeByteRingOpen(WIN32_HTTP_STATE->RequestSafeRing);

                SafeByteRingTryReadStruct(
                    &Guard,
                    &RequestData.Size
                );

                if (RequestData.Size) {
                    RequestData.Str = ArenaPushArrayZero(
                        Scratch.MemPool,
                        u8,
                        RequestData.Size
                    );
                    SafeByteRingReadOrWait(
                        &Guard,
                        RequestData.Size,
                        RequestData.Str,
                        U64_MAX
                    );
                }

                SafeByteRingClose(&Guard);
            }

            SafeByteRing* SafeRing = NULL;
            HTTPRequestParams Params = {};

            // NOTE(nathan): read request
            if (RequestData.Size) {
                u64 Offset = 0;

                Offset += SerialPopStruct(RequestData, Offset, &SafeRing);
                Offset += SerialPopStruct(RequestData, Offset, &Params.ID);
                Offset += SerialPopStruct(RequestData, Offset, &Params.Method);
                Offset += SerialPopStruct(RequestData, Offset, &Params.URL.Size);
                Params.URL = StrSub(
                    RequestData,
                    Rng(Offset, Offset + Params.URL.Size)
                );
                Offset += Params.URL.Size;
                Offset += SerialPopStruct(RequestData, Offset, &Params.Body.Size);
                Params.Body = StrSub(
                    RequestData,
                    Rng(Offset, Offset + Params.Body.Size)
                );
                Offset += Params.Body.Size;
                Offset += SerialPopStruct(RequestData, Offset, &Params.UserAgent.Size);
                Params.UserAgent = StrSub(
                    RequestData,
                    Rng(Offset, Offset + Params.UserAgent.Size)
                );
                Offset += Params.UserAgent.Size;
                Offset += SerialPopStruct(RequestData, Offset, &Params.Authorisation.Size);
                Params.Authorisation = StrSub(
                    RequestData,
                    Rng(Offset, Offset + Params.Authorisation.Size)
                );
                Offset += Params.Authorisation.Size;
                Offset += SerialPopStruct(RequestData, Offset, &Params.ContentType.Size);
                Params.ContentType = StrSub(
                    RequestData,
                    Rng(Offset, Offset + Params.ContentType.Size)
                );
                Offset += Params.ContentType.Size;
            }

            Win32HTTPRequest* Request = NULL;

            if (FreeSlotIndex && RequestData.Size) {
                Request = &WIN32_HTTP_STATE->ActiveRequests[FreeSlotIndex - 1];
                Request->Active = TRUE;
                ArenaClear(Request->MemPool);
                ArenaClear(Request->CompletedBodyMemPool);
                Request->SafeRing = SafeRing;
                Request->ID = Params.ID;
                Request->Status = WIN32_HTTP_REQUEST_STATUS_KIND_NULL;
            }

            if (Request) {
                b32 Okay = FALSE;
                HTTPMethodKind MethodKind = Params.Method;
                Str8 URL = Params.URL;
                Str8 Body = Params.Body;
                Str8 UserAgent = Params.UserAgent;
                Str8 Authorisation = Params.Authorisation;
                Str8 ContentType = Params.ContentType;
                Str8 URLProtocol = {};
                Str8 URLPort = {};
                Str8 URLHostname = {};
                Str8 URLPath = {};
                u64 ProtocolSplitCharPosition = StrFindSubStr(
                    URL,
                    "://"_s8,
                    0,
                    0
                );
                u64 AfterProtocolPosition = 0;

                if (ProtocolSplitCharPosition < URL.Size) {
                    AfterProtocolPosition = ProtocolSplitCharPosition + 3;
                    URLProtocol = StrPrefix(URL, AfterProtocolPosition);
                }

                u64 FinalColonPosition = StrFindSubStr(
                    URL,
                    ":"_s8,
                    AfterProtocolPosition,
                    0
                );

                if (FinalColonPosition < URL.Size)
                    URLPort = StrSkip(URL, FinalColonPosition + 1);

                u64 FirstNonProtocolSlashPosition = StrFindSubStr(
                    URL,
                    "/"_s8,
                    AfterProtocolPosition,
                    0
                );

                if (FirstNonProtocolSlashPosition < URL.Size) {
                    URLHostname = StrPrefix(URL, FirstNonProtocolSlashPosition);
                    URLHostname = StrSkip(URLHostname, AfterProtocolPosition);
                    URLPath = StrSkip(URL, FirstNonProtocolSlashPosition);
                } else {
                    URLHostname = StrSkip(URL, AfterProtocolPosition);
                }

                u16 Port = INTERNET_DEFAULT_HTTPS_PORT;
                Str16 HostnameWide = {};

                if (URLPort.Size) {
                    Port = SafeTruncateU32(
                        SafeTruncateU64(
                            U64FromStr(URLPort, 10)
                        )
                    );
                } else if (
                    StrMatch(
                        URLProtocol,
                        "https://"_s8,
                        STR_MATCH_ALL_CASES
                    )
                ) {
                    Port = INTERNET_DEFAULT_HTTPS_PORT;
                } else if (
                    StrMatch(
                        URLProtocol,
                        "http://"_s8,
                        STR_MATCH_ALL_CASES
                    )
                ) {
                    Port = INTERNET_DEFAULT_HTTP_PORT;
                } else if (
                    StrMatch(
                        URLProtocol,
                        "ftp://"_s8,
                        STR_MATCH_ALL_CASES
                    )
                ) {
                    Port = INTERNET_DEFAULT_FTP_PORT;
                } else if (
                    StrMatch(
                        URLProtocol,
                        "ssh://"_s8,
                        STR_MATCH_ALL_CASES
                    )
                ) {
                    Port = INTERNET_DEFAULT_SSH_PORT;
                }

                HostnameWide = Str16FromStr8(
                    Scratch.MemPool,
                    URLHostname
                );

                WCHAR* Action = L"GET";

                switch (MethodKind) {
                    default:
                    case HTTP_METHOD_KIND_GET: {} break;

                    case HTTP_METHOD_KIND_HEAD: {
                        Action = L"HEAD";
                    } break;

                    case HTTP_METHOD_KIND_POST: {
                        Action = L"POST";
                    } break;

                    case HTTP_METHOD_KIND_PUT: {
                        Action = L"PUT";
                    } break;

                    case HTTP_METHOD_KIND_DELETE: {
                        Action = L"DELETE";
                    } break;

                    case HTTP_METHOD_KIND_CONNECT: {
                        Action = L"CONNECT";
                    } break;

                    case HTTP_METHOD_KIND_OPTIONS: {
                        Action = L"OPTIONS";
                    } break;

                    case HTTP_METHOD_KIND_TRACE: {
                        Action = L"TRACE";
                    } break;

                    case HTTP_METHOD_KIND_PATCH: {
                        Action = L"PATCH";
                    } break;
                }

                WCHAR* PathName = L"";

                if (URLPath.Size) {
                    Str16 URLPathWide = Str16FromStr8(Scratch.MemPool, URLPath);

                    PathName = (WCHAR*) URLPathWide.Str;
                }

                Str16 HeaderWide = {};
                Str8List HeaderStrings = {};

                if (UserAgent.Size) {
                    ListPushFmt(
                        Scratch.MemPool,
                        &HeaderStrings,
                        "User-Agent: %S\n",
                        PRINT_STR(UserAgent)
                    );
                }

                if (Authorisation.Size) {
                    ListPushFmt(
                        Scratch.MemPool,
                        &HeaderStrings,
                        "Authorization: %S\n",
                        PRINT_STR(Authorisation)
                    );
                }

                if (ContentType.Size) {
                    ListPushFmt(
                        Scratch.MemPool,
                        &HeaderStrings,
                        "Content-Type: %S\n",
                        PRINT_STR(ContentType)
                    );
                }

                {
                    Str8 Header = StrListJoin(
                        Scratch.MemPool,
                        &HeaderStrings,
                        NULL
                    );

                    HeaderWide = Str16FromStr8(Scratch.MemPool, Header);
                }

                WCHAR* Header = WINHTTP_NO_ADDITIONAL_HEADERS;
                u64 HeaderSize = 0;
                void* OptionalData = WINHTTP_NO_REQUEST_DATA;
                u64 OptionalDataSize = 0;

                if (HeaderWide.Size) {
                    Header = (WCHAR*) HeaderWide.Str;
                    HeaderSize = HeaderWide.Size;
                }

                if (Body.Size) {
                    Str8 BodyCopy = ArenaPushStrCpy(Request->MemPool, Body);

                    OptionalData = BodyCopy.Str;
                    OptionalDataSize = BodyCopy.Size;
                }

                HINTERNET ConnectionHandle = NULL;

                if (WIN32_HTTP_STATE->SessionHandle) {
                    ConnectionHandle = WinHttpConnect(
                        WIN32_HTTP_STATE->SessionHandle,
                        (WCHAR*) HostnameWide.Str,
                        Port,
                        0
                    );
                    Okay = !!ConnectionHandle;
                }

                HINTERNET RequestHandle = NULL;

                if (Okay) {
                    RequestHandle = WinHttpOpenRequest(
                        ConnectionHandle,
                        Action,
                        PathName,
                        NULL,
                        WINHTTP_NO_REFERER,
                        WINHTTP_DEFAULT_ACCEPT_TYPES,
                        WINHTTP_FLAG_SECURE
                    );
                    Okay = !!RequestHandle;
                }

                if (Okay) {
                    Request->Status = WIN32_HTTP_REQUEST_STATUS_KIND_SEND_PENDING;
                } else {
                    Request->Status = WIN32_HTTP_REQUEST_STATUS_KIND_FAILED;
                }

                Request->ConnectionHandle = ConnectionHandle;
                Request->RequestHandle = RequestHandle;
                Request->MemPoolBeginBodyReadPosition = ArenaGetPosition(
                    Request->MemPool
                );

                if (Okay) {
                    WinHttpSendRequest(
                        RequestHandle,
                        Header,
                        HeaderSize,
                        OptionalData,
                        OptionalDataSize,
                        OptionalDataSize,
                        (DWORD_PTR) Request
                    );
                }
            }

            ReleaseScratch(Scratch);

            if (!FreeSlotIndex || !RequestData.Size)
                break;
        }
    }

    LaneSync();

    // NOTE(nathan): begin receiving
    {
        r1u64 Range = LaneRange(WIN32_HTTP_MAX_ACTIVE_REQUESTS);

        for (u64 Index = Range.Min; Index < Range.Max; ++Index) {
            Win32HTTPRequest* Request = &WIN32_HTTP_STATE->ActiveRequests[Index];

            if (
                Request->Active &&
                AtomicLoadU32(&Request->Status, MEM_ORDER_SEQ_CST) == WIN32_HTTP_REQUEST_STATUS_KIND_SENT
            ) {
                AtomicExchangeU32(
                    &Request->Status, 
                    WIN32_HTTP_REQUEST_STATUS_KIND_RECEIVE_PENDING, 
                    MEM_ORDER_CST_SEQ
                );
                WinHttpReceiveResponse(Request->RequestHandle, NULL);
            }
        }

        LaneSync();
    }

    // NOTE(nathan): push response output
    {
        r1u64 Range = LaneRange(WIN32_HTTP_MAX_ACTIVE_REQUESTS);

        for (u64 Index = Range.Min; Index < Range.Max; ++Index) {
            Win32HTTPRequest* Request = &WIN32_HTTP_STATE->ActiveRequests[Index];
            Win32HTTPRequestStatusKind Status = AtomicLoadU32(
                &Request->Status,
                MEM_ORDER_SEQ_CST
            );

            if (
                Request->Active &&
                (
                    Status == WIN32_HTTP_REQUEST_STATUS_KIND_COMPLETED ||
                    Status == WIN32_HTTP_REQUEST_STATUS_KIND_FAILED ||
                    Request->CompletedBodyChunks.TotalSize >= Request->SafeRing->Ring->Size / 2
                )
            ) {
                TempArena Scratch = GetScratch(NULL, 0);

                if (!Request->StatusCode) {
                    DWORD StatusCode = 0;
                    DWORD StatusCodeSize = sizeof(StatusCode);

                    WinHttpQueryHeaders(
                        Request->RequestHandle,
                        WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER,
                        WINHTTP_HEADER_NAME_BY_INDEX,
                        &StatusCode,
                        &StatusCodeSize,
                        WINHTTP_NO_HEADER_INDEX
                    );
                    Request->StatusCode = StatusCode;
                }

                if (!Request->TotalResponseBytes) {
                    DWORD BufferSize = sizeof(Request->TotalResponseBytes);

                    WinHttpQueryHeaders(
                        Request->RequestHandle,
                        WINHTTP_QUERY_CONTENT_LENGTH | WINHTTP_QUERY_FLAG_NUMBER64,
                        WINHTTP_HEADER_NAME_BY_INDEX,
                        &Request->TotalResponseBytes,
                        &BufferSize,
                        WINHTTP_NO_HEADER_INDEX
                    );
                }

                Str8 Body = StrListJoin(
                    Scratch.MemPool,
                    &Request->CompletedBodyChunks,
                    NULL
                );

                ArenaClear(Request->CompletedBodyMemPool);
                MemZero(
                    &Request->CompletedBodyChunks, 
                    sizeof(Request->CompletedBodyChunks)
                );

                u64 MaxRecordSize = Request->SafeRing->Ring->Size;
                u64 BytesPerRecordHeader = (
                    sizeof(u64) +
                    sizeof(u64) +
                    sizeof(u64) +
                    sizeof(u64) +
                    sizeof(b32) +
                    sizeof(HTTPStatusCode)
                );

                for (
                    u64 Offset = 0, NextOffset = 0; 
                    Offset <= Body.Size; 
                    Offset = NextOffset
                ) {
                    u64 BodyDataSize = Body.Size - Offset;

                    if (BytesPerRecordHeader + BodyDataSize > MaxRecordSize)
                        BodyDataSize = MaxRecordSize - BytesPerRecordHeader;

                    NextOffset = Offset + BodyDataSize;

                    b32 IsFinal = (
                        NextOffset == Body.Size &&
                        (
                            Status == WIN32_HTTP_REQUEST_STATUS_KIND_COMPLETED ||
                            Status == WIN32_HTTP_REQUEST_STATUS_KIND_FAILED
                        )
                    );
                    b32 DataRemaining = !IsFinal;
                    Str8 RecordData = {};

                    if (IsFinal || NextOffset != Offset) {
                        Str8List Chunks = {};
                        u64 RecordSize = BytesPerRecordHeader + (NextOffset - Offset);

                        ListPush(
                            Scratch.MemPool,
                            &Chunks,
                            Str8Struct(&RecordSize)
                        );
                        ListPush(
                            Scratch.MemPool,
                            &Chunks,
                            Str8Struct(&Request->ID)
                        );
                        ListPush(
                            Scratch.MemPool,
                            &Chunks,
                            Str8Struct(&Request->TotalResponseBytesSent)
                        );
                        ListPush(
                            Scratch.MemPool,
                            &Chunks,
                            Str8Struct(&Request->TotalResponseBytes)
                        );
                        ListPush(
                            Scratch.MemPool,
                            &Chunks,
                            Str8Struct(&DataRemaining)
                        );
                        ListPush(
                            Scratch.MemPool,
                            &Chunks,
                            Str8Struct(&Request->StatusCode)
                        );
                        ListPush(
                            Scratch.MemPool,
                            &Chunks,
                            StrSub(
                                Body,
                                Rng(Offset, NextOffset)
                            )
                        );
                        RecordData = StrListJoin(
                            Scratch.MemPool,
                            &Chunks,
                            NULL
                        );
                    }

                    b32 PushFailed = TRUE;

                    if (RecordData.Size) {
                        ByteRingGuard G = SafeByteRingOpen(Request->SafeRing);

                        PushFailed = !SafeByteRingWriteOrWait(
                            &G,
                            RecordData.Size,
                            RecordData.Str,
                            0
                        );
                        SafeByteRingClose(&G);
                    }

                    if (PushFailed) {
                        Str8 UnPushedBodyChunk = StrSub(
                            Body,
                            Rng(Offset, Body.Size)
                        );

                        if (UnPushedBodyChunk.Size) {
                            ListPush(
                                Request->CompletedBodyMemPool,
                                &Request->CompletedBodyChunks,
                                ArenaPushStrCpy(
                                    Request->CompletedBodyMemPool,
                                    UnPushedBodyChunk
                                )
                            );
                        }
                    }

                    if (!PushFailed)
                        Request->TotalResponseBytesSent += (NextOffset - Offset);

                    if (NextOffset == Offset || PushFailed)
                        break;
                }

                ReleaseScratch(Scratch);
            }
        }

        LaneSync();
    }

    // NOTE(nathan): begin body size queries
    {
        r1u64 Range = LaneRange(WIN32_HTTP_MAX_ACTIVE_REQUESTS);

        for (u64 Index = Range.Min; Index < Range.Max; ++Index) {
            Win32HTTPRequest* Request = &WIN32_HTTP_STATE->ActiveRequests[Index];

            if (
                Request->Active &&
                AtomicLoadU32(
                    &Request->Status,
                    MEM_ORDER_SEQ_CST
                ) == WIN32_HTTP_REQUEST_STATUS_KIND_RECEIVED
            ) {
                if (Request->NextBodyChunkSize) {
                    ListPush(
                        Request->CompletedBodyMemPool,
                        &Request->CompletedBodyChunks,
                        ArenaPushStrCpy(
                            Request->CompletedBodyMemPool,
                            Str((u8*) Request->NextBodyChunk, Request->NextBodyChunkSize)
                        )
                    );
                    Request->NextBodyChunkSize = 0;
                    Request->NextBodyChunk = NULL;
                    ArenaPopTo(Request->MemPool, Request->MemPoolBeginBodyReadPosition);
                }

                AtomicExchangeU32(
                    &Request->Status,
                    WIN32_HTTP_REQUEST_STATUS_KIND_DATA_SIZE_PENDING,
                    MEM_ORDER_CST_SEQ
                );
                WinHttpQueryDataAvailable(
                    Request->RequestHandle,
                    NULL
                );
            }
        }

        LaneSync();
    }

    // NOTE(nathan): begin body reads
    {
        r1u64 Range = LaneRange(WIN32_HTTP_MAX_ACTIVE_REQUESTS);

        for (u64 Index = Range.Min; Index < Range.Max; ++Index) {
            Win32HTTPRequest* Request = &WIN32_HTTP_STATE->ActiveRequests[Index];

            if (
                Request->Active &&
                AtomicLoadU64(
                    &Request->Status,
                    MEM_ORDER_CST_SEQ
                ) == WIN32_HTTP_REQUEST_STATUS_KIND_DATA_SIZE
            ) {
                Request->NextBodyChunk = ArenaPushArrayZero(
                    Request->MemPool,
                    u8,
                    Request->NextBodyChunkSize
                );
                AtomicExchangeU32(
                    &Request->Status,
                    WIN32_HTTP_REQUEST_STATUS_KIND_READ_PENDING,
                    MEM_ORDER_CST_SEQ
                );
                WinHttpReadData(
                    Request->RequestHandle,
                    Request->NextBodyChunk,
                    Request->NextBodyChunkSize,
                    NULL
                );
            }
        }

        LaneSync();
    }


    // NOTE(nathan): complete requests
    {
        r1u64 Range = LaneRange(WIN32_HTTP_MAX_ACTIVE_REQUESTS);

        for (u64 Index = Range.Min; Index < Range.Max; ++Index) {
            Win32HTTPRequest* Request = &WIN32_HTTP_STATE->ActiveRequests[Index];
            Win32HTTPRequestStatusKind Status = AtomicLoadU32(
                &Request->Status, 
                MEM_ORDER_SEQ_CST
            );

            if (
                Request->Active &&
                !Request->CompletedBodyChunks.TotalSize &&
                !Request->NextBodyChunkSize &&
                (
                    Status == WIN32_HTTP_REQUEST_STATUS_KIND_COMPLETED ||
                    Status == WIN32_HTTP_REQUEST_STATUS_KIND_FAILED
                )
            ) {
                Request->Active = FALSE;
                ArenaClear(Request->MemPool);
                ArenaClear(Request->CompletedBodyMemPool);
                MemZero(&Request->CompletedBodyChunks, sizeof(Request->CompletedBodyChunks));
                Request->NextBodyChunk = NULL;
                Request->NextBodyChunkSize = 0;
                WinHttpCloseHandle(Request->ConnectionHandle);
                WinHttpCloseHandle(Request->RequestHandle);
            }
        }

        LaneSync();
    }
}

b32 
HTTPPushRequest(
    OUT SafeByteRing* SafeRing, 
    HTTPRequestParams* Params, 
    u64 EndTimeUSecs
) {
    TempArena Scratch = GetScratch(NULL, 0);
    b32 Result = FALSE;
    Str8 Data = {};
    Str8List Chunks = {};

    ListPush(
        Scratch.MemPool,
        &Chunks,
        Str8Struct(&SafeRing)
    );
    ListPush(
        Scratch.MemPool,
        &Chunks,
        Str8Struct(&Params->ID)
    );
    ListPush(
        Scratch.MemPool,
        &Chunks,
        Str8Struct(&Params->Method)
    );
    ListPush(
        Scratch.MemPool,
        &Chunks,
        Str8Struct(&Params->URL.Size)
    );
    ListPush(
        Scratch.MemPool,
        &Chunks,
        Params->URL
    );
    ListPush(
        Scratch.MemPool,
        &Chunks,
        Str8Struct(&Params->Body.Size)
    );
    ListPush(
        Scratch.MemPool,
        &Chunks,
        Params->Body
    );
    ListPush(
        Scratch.MemPool,
        &Chunks,
        Str8Struct(&Params->UserAgent.Size)
    );
    ListPush(
        Scratch.MemPool,
        &Chunks,
        Params->UserAgent
    );
    ListPush(
        Scratch.MemPool,
        &Chunks,
        Str8Struct(&Params->Authorisation.Size)
    );
    ListPush(
        Scratch.MemPool,
        &Chunks,
        Params->Authorisation
    );
    ListPush(
        Scratch.MemPool,
        &Chunks,
        Str8Struct(&Params->ContentType.Size)
    );
    ListPush(
        Scratch.MemPool,
        &Chunks,
        Params->ContentType
    );

    u64 DataSize = Chunks.TotalSize;

    ListPushFront(Scratch.MemPool, &Chunks, Str8Struct(&DataSize));
    Data = StrListJoin(
        Scratch.MemPool,
        &Chunks,
        NULL
    );

    if (Data.Size < WIN32_HTTP_STATE->RequestSafeRing->Ring->Size) {
        ByteRingGuard G = SafeByteRingOpen(
            WIN32_HTTP_STATE->RequestSafeRing
        );

        Result = SafeByteRingWriteOrWait(
            &G,
            Data.Size,
            Data.Str,
            EndTimeUSecs
        );
        SafeByteRingClose(&G);
    }

    ReleaseScratch(Scratch);

    return Result;
}

b32 
HTTPPopResponse(
    Arena* MemPool, 
    OUT SafeByteRing* SafeRing, 
    OUT HTTPResponse* Response, 
    u64 EndTimeUSecs
) {
    ByteRingGuard G = SafeByteRingOpen(SafeRing);
    u64 DataSize = 0;
    b32 Result = SafeByteRingReadStructOrWait(
        &G,
        &DataSize,
        EndTimeUSecs
    );

    if (Result) {
        TempArena Scratch = GetScratch(&MemPool, 1);
        u64 DataSizeLessSizePrefix = DataSize - sizeof(DataSize);
        Str8 Data = {};

        Data.Size = DataSizeLessSizePrefix;
        Data.Str = ArenaPushArrayZero(
            Scratch.MemPool,
            u8,
            Data.Size
        );
        SafeByteRingReadOrWait(
            &G,
            Data.Size,
            Data.Str,
            U64_MAX
        );

        u64 Offset = 0;

        Offset += SerialPopStruct(Data, Offset, &Response->ID);
        Offset += SerialPopStruct(Data, Offset, &Response->StartOffset);
        Offset += SerialPopStruct(Data, Offset, &Response->EndOffset);
        Offset += SerialPopStruct(Data, Offset, &Response->HasExtra);
        Offset += SerialPopStruct(Data, Offset, &Response->Code);
        Response->Body.Size = Data.Size - Offset;
        Response->Body.Str = ArenaPushArrayZero(
            MemPool,
            u8,
            Response->Body.Size
        );
        Offset += SerialPop(
            Data,
            Offset,
            Response->Body.Str,
            Response->Body.Size,
            1
        );
        ReleaseScratch(Scratch);
    }

    SafeByteRingClose(&G);

    return Result;
}
