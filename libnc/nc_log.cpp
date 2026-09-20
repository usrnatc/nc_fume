#include "nc_types.h"
#include "nc_log.h"
#include "nc_sync.h"
#include "nc_print.h"
#include "nc_sock.h"
#include "nc_file.h"
#include "nc_memory.h"
#include "nc_atomics.h"
#include "nc_thread.h"

LogEndpoint __EMPTY_LOG_ENDPOINT_VALUE;
LogEndpoint* const EMPTY_LOG_ENDPOINT_VALUE = &__EMPTY_LOG_ENDPOINT_VALUE;

LogLevelKindInfo LOG_LEVEL_KIND_INFO_TABLE[LOG_LEVEL_KIND_COUNT] = {
    {
        Str8Lit("    MSG"),
        LOG_COLOUR_NORMAL
    },
    {
        Str8Lit("SUCCESS"),
        LOG_COLOUR_GREEN
    },
    {
        Str8Lit("   INFO"),
        LOG_COLOUR_CYAN
    },
    {
        Str8Lit("   WARN"),
        LOG_COLOUR_BLUE
    },
    {
        Str8Lit("  ERROR"),
        LOG_COLOUR_YELLOW
    },
    {
        Str8Lit("  PANIC"),
        LOG_COLOUR_RED
    }
};

LogState* LOG_STATE = NULL;

INTERNAL u64 __LogDrain(void);
INTERNAL void __LogDrainAll(void);

Str8
LogFormatEntry(
    Arena* MemPool, 
    LogEntry* Entry, 
    b32 DoColour
) {
    LogLevelKind Severity = MIN(
        Entry->Header.Severity,
        LOG_LEVEL_KIND_COUNT - 1
    );
    LogLevelKindInfo* Info = &LOG_LEVEL_KIND_INFO_TABLE[Severity];
    Str8 Result = ArenaPushStrFmt(
        MemPool,
        "%s%S | %S | %-5d | %S%s\n",
        DoColour ? Info->ColourPrefix : "",
        PRINT_STR(Info->Prefix),
        PRINT_STR(Entry->FileName),
        Entry->Header.Line,
        PRINT_STR(Entry->Msg),
        DoColour ? LOG_COLOUR_RESET : ""
    );

    return Result;
}

void
LogInit(u64 RingSize, b32 RunAsync)
{
    if (LOG_STATE)
        return;

    if (!RingSize)
        RingSize = MB(1);

    EMPTY_LOG_ENDPOINT_VALUE->Next = EMPTY_LOG_ENDPOINT_VALUE;
    EMPTY_LOG_ENDPOINT_VALUE->Prev = EMPTY_LOG_ENDPOINT_VALUE;
    EMPTY_LOG_ENDPOINT_VALUE->EndpointFunc = LOG_ENDPOINT_FUNC_NAME(Null);

    Arena* MemPool = ArenaAlloc();

    LOG_STATE = ArenaPushArrayZero(
        MemPool,
        LogState,
        1
    );
    LOG_STATE->MemPool = MemPool;
    LOG_STATE->FrameMemPool = ArenaAlloc();
    LOG_STATE->SafeRing = SafeByteRingAlloc(MemPool, RingSize);
    LOG_STATE->Mtx = MutexAlloc();
    LOG_STATE->Thread = EMPTY_HANDLE_VALUE;
    LOG_STATE->HeadEndpoint = EMPTY_LOG_ENDPOINT_VALUE;
    LOG_STATE->TailEndpoint = EMPTY_LOG_ENDPOINT_VALUE;

    if (RunAsync) {
        LOG_STATE->ThreadIsRunning = TRUE;
        LOG_STATE->Thread = ThreadLaunch(
            LogThreadEntryPoint,
            NULL
        );
    }
}

void
LogShutdown(void)
{
    if (!LOG_STATE)
        return;

    if (LOG_STATE->ThreadIsRunning) {
        LOCK_SCOPE(LOG_STATE->SafeRing->Mtx) {
            AtomicExchangeU32(&LOG_STATE->ThreadShouldExit, TRUE, MEM_ORDER_SEQ_CST);
            CondVarBroadcast(LOG_STATE->SafeRing->CondVar);
        }

        ThreadJoin(LOG_STATE->Thread, U64_MAX);
        LOG_STATE->ThreadIsRunning = FALSE;
        LOG_STATE->Thread = EMPTY_HANDLE_VALUE;
    } else {
        __LogDrainAll();
    }
}

LogEndpoint*
LogRegisterEndpoint(LogEndpointParams Params)
{
    LogEndpoint* Result = EMPTY_LOG_ENDPOINT_VALUE;

    if (!LOG_STATE)
        return Result;

    if (!Params.EndpointFunc)
        Params.EndpointFunc = LOG_ENDPOINT_FUNC_NAME(Null);

    LOCK_SCOPE(LOG_STATE->Mtx) {
        Result = ArenaPushArrayZero(
            LOG_STATE->MemPool,
            LogEndpoint,
            1
        );

        Result->Name = ArenaPushStrCpy(LOG_STATE->MemPool, Params.Name);
        Result->ID = Hash(Params.Name);
        Result->EndpointFunc = Params.EndpointFunc;
        Result->FlushFunc = Params.FlushFunc;
        Result->MinSeverity = Params.MinSeverity;

        if (Params.UserDataSize) {
            Result->UserData = ArenaPushArrayZero(
                LOG_STATE->MemPool,
                u8,
                Params.UserDataSize
            );
            MemCpy(Result->UserData, Params.UserData, Params.UserDataSize);
        } else {
            Result->UserData = NULL;
        }

        DLL_PUSH_BACK_EX(
            EMPTY_LOG_ENDPOINT_VALUE,
            LOG_STATE->HeadEndpoint,
            LOG_STATE->TailEndpoint,
            Result,
            Next,
            Prev
        );
    }

    return Result;
}

LogEndpoint*
LogRegisterEndpoint(LogEndpointParams Params, char* Fmt, ...)
{
    TempArena Scratch = GetScratch(NULL, 0);
    va_list Args;

    va_start(Args, Fmt);

    Params.Name = ArenaPushStrFmtV(
        Scratch.MemPool,
        Fmt,
        Args
    );

    va_end(Args);

    LogEndpoint* Result = LogRegisterEndpoint(Params);

    ReleaseScratch(Scratch);

    return Result;
}

void
LogSetLevelKindInfo(LogLevelKind Severity, Str8 Prefix, char* ColourPrefix)
{
    if (Severity < LOG_LEVEL_KIND_COUNT) {
        LogLevelKindInfo* Info = &LOG_LEVEL_KIND_INFO_TABLE[Severity];

        Info->Prefix       = Prefix;
        Info->ColourPrefix = ColourPrefix;
    }
}

void
__LogBase(
    LogEndpointSelectKind EndpointSelect,
    LogEndpointID EndpointID,
    char const* FileName,
    int Line,
    LogLevelKind Severity,
    Str8 Msg
) {
    if (!LOG_STATE)
        return;

    Severity = CLAMP(0, Severity, LOG_LEVEL_KIND_COUNT - 1);

    Str8 FileNameString = StrSkipLastSlash(
        CStr8((char*) FileName)
    );

    if (FileNameString.Size > U16_MAX)
        FileNameString = StrPrefix(FileNameString, U16_MAX);

    u64 MaxMsgSize = (LOG_STATE->SafeRing->Ring->Size / 4);
    u64 Overhead = sizeof(LogEntryHeader) + FileNameString.Size;

    MaxMsgSize = (MaxMsgSize > Overhead) ? (MaxMsgSize - Overhead) : 0;

    if (Msg.Size > MaxMsgSize)
        Msg = StrPrefix(Msg, MaxMsgSize);

    LogEntryHeader Header = {};

    Header.EndpointID = EndpointID;
    Header.TimeStamp = TimeNow();
    Header.MsgSize = SafeTruncateU64(Msg.Size);
    Header.Line = SafeTruncateU64(Line);
    Header.FileNameSize = SafeTruncateU32(
        SafeTruncateU64(
            FileNameString.Size
        )
    );
    Header.EndpointSelect = EndpointSelect;
    Header.Severity = Severity;

    TempArena Scratch = GetScratch(NULL, 0);
    Str8List Chunks = {};

    SerialBegin(Scratch.MemPool, &Chunks);
    SerialPushStruct(Scratch.MemPool, &Chunks, &Header);
    SerialPushStr(Scratch.MemPool, &Chunks, FileNameString);
    SerialPushStr(Scratch.MemPool, &Chunks, Msg);

    Str8 Data = SerialEnd(Scratch.MemPool, &Chunks);
    ByteRingGuard Guard = SafeByteRingOpen(LOG_STATE->SafeRing);
    b32 WriteOkay = SafeByteRingTryWrite(&Guard, Data.Size, Data.Str);

    SafeByteRingClose(&Guard);

    if (!WriteOkay)
        AtomicIncFetchU64(&LOG_STATE->DroppedCount, MEM_ORDER_SEQ_CST);

    ReleaseScratch(Scratch);
}

void
__LogBase(
    LogEndpointSelectKind EndpointSelect,
    LogEndpointID EndpointID,
    char const* FileName,
    int Line,
    LogLevelKind Severity,
    char* Fmt,
    ...
) {
    TempArena Scratch = GetScratch(NULL, 0);
    va_list Args;

    va_start(Args, Fmt);

    Str8 String = ArenaPushStrFmtV(
        Scratch.MemPool,
        Fmt,
        Args
    );

    va_end(Args);
    __LogBase(
        EndpointSelect,
        EndpointID,
        FileName,
        Line,
        Severity,
        String
    );
    ReleaseScratch(Scratch);
}

void
__LogBase(
    LogEndpointSelectKind EndpointSelect,
    LogEndpointID EndpointID,
    char const* FileName,
    int Line,
    LogLevelKind Severity,
    Str16 Msg
) {
    TempArena Scratch = GetScratch(NULL, 0);
    Str8 Msg8 = Str8FromStr16(Scratch.MemPool, Msg);

    __LogBase(
        EndpointSelect,
        EndpointID,
        FileName,
        Line,
        Severity,
        Msg8
    );
    ReleaseScratch(Scratch);
}

void
__LogBase(
    LogEndpointSelectKind EndpointSelect,
    LogEndpointID EndpointID,
    char const* FileName,
    int Line,
    LogLevelKind Severity,
    wchar* Fmt,
    ...
) {
    TempArena Scratch = GetScratch(NULL, 0);
    va_list Args;

    va_start(Args, Fmt);

    Str16 Msg = ArenaPushStrFmtV(Scratch.MemPool, Fmt, Args);

    va_end(Args);
    __LogBase(
        EndpointSelect,
        EndpointID,
        FileName,
        Line,
        Severity,
        Msg
    );
    ReleaseScratch(Scratch);
}

INTERNAL u64
__LogDrain(void)
{
    ArenaClear(LOG_STATE->FrameMemPool);
    LOG_STATE->ActiveEntriesCount = 0;
    LOG_STATE->RegisteredEndpointsCount = 0;

    LOCK_SCOPE(LOG_STATE->Mtx) {
        for (
            LogEndpoint* Endpoint = LOG_STATE->HeadEndpoint;
            (
                Endpoint != EMPTY_LOG_ENDPOINT_VALUE && 
                LOG_STATE->RegisteredEndpointsCount < LOG_MAX_REGISTERED_ENDPOINTS
            );
            Endpoint = Endpoint->Next
        ) {
            LOG_STATE->RegisteredEndpoints[LOG_STATE->RegisteredEndpointsCount++] = Endpoint;
        }
    }

    {
        ByteRingGuard Guard = SafeByteRingOpen(LOG_STATE->SafeRing);

        for ( ; LOG_STATE->ActiveEntriesCount < LOG_MAX_ACTIVE_ENTRIES; ) {
            LogEntryHeader Header = {};

            if (!SafeByteRingTryReadStruct(&Guard, &Header))
                break;

            Str8 Payload = {};

            Payload.Size = (u64) Header.FileNameSize + (u64) Header.MsgSize;

            if (Payload.Size) {
                Payload.Str = ArenaPushArrayZero(
                    LOG_STATE->FrameMemPool,
                    u8,
                    Payload.Size
                );

                b32 ReadOkay = SafeByteRingTryRead(
                    &Guard, 
                    Payload.Size, 
                    Payload.Str
                );

                ASSERT(ReadOkay);
                UNUSED(ReadOkay);
            }

            LogEntry* Entry = &LOG_STATE->ActiveEntries[LOG_STATE->ActiveEntriesCount++];
            u64 Offset = 0;

            Entry->Header = Header;
            Offset += SerialPopBlock(
                Payload, 
                Offset, 
                Header.FileNameSize, 
                &Entry->FileName
            );
            Offset += SerialPopBlock(
                Payload, 
                Offset, 
                Header.MsgSize, 
                &Entry->Msg
            );
        }

        SafeByteRingClose(&Guard);
    }

    if (!LOG_STATE->ActiveEntriesCount)
        return 0;

    for (
        u64 EndpointIndex = 0;
        EndpointIndex < LOG_STATE->RegisteredEndpointsCount;
        ++EndpointIndex
    ) {
        LogEndpoint* Endpoint = LOG_STATE->RegisteredEndpoints[EndpointIndex];

        MemZero(&Endpoint->PendingWrites, sizeof(Endpoint->PendingWrites));

        for (u64 Index = 0; Index < LOG_STATE->ActiveEntriesCount; ++Index) {
            LogEntry* Entry = &LOG_STATE->ActiveEntries[Index];
            b32 IsMatch = TRUE;

            if (Entry->Header.EndpointSelect == LOG_ENDPOINT_SELECT_KIND_ID)
                IsMatch = Entry->Header.EndpointID == Endpoint->ID;

            if (IsMatch && Entry->Header.Severity >= Endpoint->MinSeverity)
                Endpoint->EndpointFunc(Endpoint, Entry);
        }

        if (Endpoint->FlushFunc)
            Endpoint->FlushFunc(Endpoint);

        MemZero(&Endpoint->PendingWrites, sizeof(Endpoint->PendingWrites));
    }

    return LOG_STATE->ActiveEntriesCount;
}

INTERNAL void
__LogDrainAll(void)
{
    while (__LogDrain() == LOG_MAX_ACTIVE_ENTRIES);
}

void
LogThreadEntryPoint(void *Params)
{
    UNUSED(Params);

    ThreadSetName("[LOG]"_s8);

    SafeByteRing* SafeRing = LOG_STATE->SafeRing;

    for (;;) {
        b32 ShouldExit = AtomicLoadU32(
            &LOG_STATE->ThreadShouldExit, 
            MEM_ORDER_SEQ_CST
        );

        __LogDrainAll();

        if (ShouldExit)
            break;

        LOCK_SCOPE(SafeRing->Mtx) {
            for (;;) {
                if (
                    AtomicLoadU32(
                        &LOG_STATE->ThreadShouldExit, 
                        MEM_ORDER_SEQ_CST
                    )
                ) {
                    break;
                }

                if (SafeRing->Ring->WritePosition != SafeRing->Ring->ReadPosition)
                    break;

                CondVarWait(SafeRing->CondVar, SafeRing->Mtx, U64_MAX);
            }
        }
    }
}

void
LogAsyncTick(void)
{
    if (!LOG_STATE)
        return;

    if (LOG_STATE->ThreadIsRunning)
        return;

    __LogDrainAll();
    // if (!LaneIndex()) {
    //     ArenaClear(LOG_STATE->FrameMemPool);
    //     LOG_STATE->ActiveEntriesCount = 0;
    //     LOG_STATE->RegisteredEndpointsCount = 0;

    //     LOCK_SCOPE(LOG_STATE->Mtx) {
    //         for (
    //             LogEndpoint* Endpoint = LOG_STATE->HeadEndpoint;
    //             Endpoint != EMPTY_LOG_ENDPOINT_VALUE && LOG_STATE->RegisteredEndpointsCount < LOG_MAX_REGISTERED_ENDPOINTS;
    //             Endpoint = Endpoint->Next
    //         ) {
    //             LOG_STATE->RegisteredEndpoints[LOG_STATE->RegisteredEndpointsCount++] = Endpoint;
    //         }
    //     }

    //     for (;;) {
    //         if (LOG_STATE->ActiveEntriesCount >= LOG_MAX_ACTIVE_ENTRIES)
    //             break;

    //         LogEntryHeader Header = {};
    //         ByteRingGuard Guard = SafeByteRingOpen(LOG_STATE->SafeRing);
    //         b32 HasData = SafeByteRingTryReadStruct(&Guard, &Header);
    //         Str8 Payload = {};

    //         if (HasData) {
    //             Payload.Size = (u64) (Header.FileNameSize + Header.MsgSize);

    //             if (Payload.Size) {
    //                 Payload.Str = ArenaPushArrayZero(
    //                     LOG_STATE->FrameMemPool,
    //                     u8,
    //                     Payload.Size
    //                 );
    //                 SafeByteRingTryRead(&Guard, Payload.Size, Payload.Str);
    //             }
    //         }

    //         SafeByteRingClose(&Guard);

    //         if (!HasData)
    //             break;

    //         LogEntry* Entry = &LOG_STATE->ActiveEntries[LOG_STATE->ActiveEntriesCount++];
    //         u64 Offset = 0;

    //         Entry->Header = Header;
    //         Offset += SerialPopBlock(Payload, Offset, Header.FileNameSize, &Entry->FileName);
    //         Offset += SerialPopBlock(Payload, Offset, Header.MsgSize, &Entry->Msg);
    //     }

    //     for (u64 Index = 0; Index < LOG_STATE->ActiveEntriesCount; ++Index) {
    //         LogEntry* Entry = &LOG_STATE->ActiveEntries[Index];

    //         for (
    //             u64 EndpointIndex = 0; 
    //             EndpointIndex < LOG_STATE->RegisteredEndpointsCount; 
    //             ++EndpointIndex
    //         ) {
    //             LogEndpoint* Endpoint = LOG_STATE->RegisteredEndpoints[EndpointIndex];
    //             b32 IsMatch = TRUE;

    //             if (Entry->Header.EndpointSelect == LOG_ENDPOINT_SELECT_KIND_ID)
    //                 IsMatch = Entry->Header.EndpointID == Endpoint->ID;

    //             if (IsMatch && Entry->Header.Severity >= Endpoint->MinSeverity)
    //                 Endpoint->EndpointFunc(Endpoint, Entry);
    //         }
    //     }
    // }

    // {
    //     r1u64 Range = LaneRange(LOG_STATE->RegisteredEndpointsCount);

    //     for (u64 Index = Range.Min; Index < Range.Max; ++Index) {
    //         LogEndpoint* Endpoint = LOG_STATE->RegisteredEndpoints[Index];

    //         if (Endpoint->FlushFunc)
    //             Endpoint->FlushFunc(Endpoint);

    //         MemZero(&Endpoint->PendingWrites, sizeof(Endpoint->PendingWrites));
    //     }

    //     LaneSync();
    // }
}

LOG_ENDPOINT_FUNC_DEF(Null)
{
    UNUSED(Endpoint);
    UNUSED(Entry);
}

LOG_ENDPOINT_FUNC_DEF(Console)
{
    Str8 Line = LogFormatEntry(
        LOG_STATE->FrameMemPool,
        Entry,
        TRUE
    );

    ListPush(
        LOG_STATE->FrameMemPool,
        &Endpoint->PendingWrites,
        Line
    );
}

LOG_ENDPOINT_FUNC_DEF(File)
{
    Str8 Line = LogFormatEntry(
        LOG_STATE->FrameMemPool,
        Entry,
        FALSE
    );

    ListPush(
        LOG_STATE->FrameMemPool,
        &Endpoint->PendingWrites,
        Line
    );
}

LOG_ENDPOINT_FUNC_DEF(Socket)
{
    Str8 Line = LogFormatEntry(
        LOG_STATE->FrameMemPool,
        Entry,
        FALSE
    );

    ListPush(
        LOG_STATE->FrameMemPool,
        &Endpoint->PendingWrites,
        Line
    );
}

LOG_ENDPOINT_FLUSH_FUNC_DEF(Null)
{
    UNUSED(Endpoint);
}

LOG_ENDPOINT_FLUSH_FUNC_DEF(Console)
{
    TempArena Scratch = GetScratch(NULL, 0);
    Str8 BatchedData = StrListJoin(
        Scratch.MemPool,
        &Endpoint->PendingWrites,
        NULL
    );

    if (BatchedData.Size)
        PrintErr(BatchedData);

    ReleaseScratch(Scratch);
}

LOG_ENDPOINT_FLUSH_FUNC_DEF(File)
{
    Handle* FileHandle = (Handle*) Endpoint->UserData;

    if (!FileHandle || *FileHandle == EMPTY_HANDLE_VALUE)
        return;

    TempArena Scratch = GetScratch(NULL, 0);
    Str8 BatchedData = StrListJoin(
        Scratch.MemPool,
        &Endpoint->PendingWrites,
        NULL
    );

    if (BatchedData.Size) {
        FileProperties FileProps = SystemGetFileProperties(*FileHandle);
        u64 Offset = FileProps.Size;

        SystemWriteFile(
            *FileHandle,
            Rng(Offset, Offset + BatchedData.Size),
            BatchedData.Str
        );
    }

    ReleaseScratch(Scratch);
}

LOG_ENDPOINT_FLUSH_FUNC_DEF(Socket)
{
    SocketInfo* FullSocket = (SocketInfo*) Endpoint->UserData;

    if (!FullSocket || FullSocket->Socket == EMPTY_HANDLE_VALUE)
        return;

    TempArena Scratch = GetScratch(NULL, 0);
    Str8 BatchedData = StrListJoin(
        Scratch.MemPool,
        &Endpoint->PendingWrites,
        NULL
    );

    if (BatchedData.Size) {
        SocketSendTo(
            FullSocket->Socket,
            BatchedData.Str,
            BatchedData.Size,
            FullSocket->Address
        );
    }

    ReleaseScratch(Scratch);
}
