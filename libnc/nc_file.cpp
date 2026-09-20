#include "nc_file.h"
#include "nc_arena.h"

API_INTERNAL Str8
SystemReadFileRangeContents(Arena* MemPool, Handle FileHandle, r1u64 FileRange)
{
    u64 PreviousPosition = ArenaGetPosition(MemPool);
    Str8 Result = {};

    Result.Size = Length(FileRange);
    Result.Str = ArenaPushArray(MemPool, u8, Result.Size);

    u64 BytesRead = SystemReadFile(FileHandle, FileRange, Result.Str);

    if (BytesRead < Result.Size) {
        ArenaPopTo(MemPool, PreviousPosition + BytesRead);
        Result.Size = BytesRead;
    }

    return Result;
}

API_INTERNAL Str8
ReadFileContents(Arena* MemPool, Str8 FilePath)
{
    SystemAccessKind Access = (SystemAccessKind) (SYS_ACCESS_READ | SYS_ACCESS_SHARE_READ);
    Handle FileHandle = SystemOpenFile(Access, FilePath);
    FileProperties Properties = SystemGetFileProperties(FileHandle);
    Str8 Result = SystemReadFileRangeContents(MemPool, FileHandle, Rng((u64) 0, Properties.Size));

    SystemCloseFile(FileHandle);

    return Result;
}

API_INTERNAL b32
WriteFileContents(Str8 FilePath, Str8 Contents)
{
    b32 Result = FALSE;
    Handle FileHandle = SystemOpenFile(SYS_ACCESS_WRITE, FilePath);

    if (FileHandle != EMPTY_HANDLE_VALUE) {
        u64 BytesWritten = SystemWriteFile(
            FileHandle, 
            Rng(0ULL, Contents.Size), 
            Contents.Str
        );

        Result = (BytesWritten == Contents.Size);
        SystemCloseFile(FileHandle);
    }

    return Result;
}

API_INTERNAL b32
WriteFileContents(Str8 FilePath, Str8List ContentList)
{
    b32 Result = FALSE;
    Handle File = SystemOpenFile(SYS_ACCESS_WRITE, FilePath);

    if (File != EMPTY_HANDLE_VALUE) {
        LogInfo("file has been created"_s8);

        TempArena Scratch = GetScratch(NULL, 0);
        u64 WriteBufferSize = KB(64);
        u8* WriteBuffer = ArenaPushArray(
            Scratch.MemPool,
            u8,
            WriteBufferSize
        );
        u64 WriteBufferWritePosition = 0;
        u64 WriteBufferReadPosition = 0;
        u64 FileOffset = 0;

        for (Str8Node* Node = ContentList.Head; Node; Node = Node->Next) {
            for (u64 NodeOffset = 0; NodeOffset < Node->String.Size; ) {
                u64 WriteBufferUnconsumeSize = (WriteBufferWritePosition - WriteBufferReadPosition);
                u64 WriteBufferAvailableSize = (WriteBufferSize - WriteBufferUnconsumeSize);

                if (!WriteBufferAvailableSize) {
                    u64 FileWriteSize = SystemWriteFile(
                        File,
                        Rng(FileOffset, FileOffset + WriteBufferSize),
                        WriteBuffer
                    );

                    if (FileWriteSize != WriteBufferSize)
                        goto DBL_BREAK;

                    FileOffset += WriteBufferSize;
                    WriteBufferReadPosition += WriteBufferSize;
                } else {
                    u64 BytesToCopy = MIN(WriteBufferAvailableSize, Node->String.Size - NodeOffset);
                    
                    WriteBufferWritePosition += RingWrite(
                        WriteBuffer, 
                        WriteBufferSize, 
                        WriteBufferWritePosition, 
                        Node->String.Str + NodeOffset, 
                        BytesToCopy
                    );
                    NodeOffset += BytesToCopy;
                }
            }
        }

        if (WriteBufferWritePosition > WriteBufferReadPosition) {
            u64 FileWriteSize = SystemWriteFile(
                File,
                Rng(FileOffset, FileOffset + (WriteBufferWritePosition - WriteBufferReadPosition)),
                WriteBuffer
            );

            FileOffset += FileWriteSize;
        }

DBL_BREAK:

        Result = (FileOffset == ContentList.TotalSize);
        SystemCloseFile(File);
        ReleaseScratch(Scratch);
    } else {
        LogError("file was not created"_s8);
    }

    return Result;
}

API_INTERNAL b32
AppendFileContents(Str8 FilePath, Str8 Contents)
{
    b32 Result = FALSE;

    if (Contents.Size) {
        SystemAccessKind Access = (SystemAccessKind) (SYS_ACCESS_WRITE | SYS_ACCESS_APPEND);
        Handle FileHandle = SystemOpenFile(Access, FilePath);

        if (FileHandle != EMPTY_HANDLE_VALUE) {
            Result = TRUE;

            u64 Position = SystemGetFileProperties(FileHandle).Size;

            SystemWriteFile(FileHandle, Rng(Position, Position + Contents.Size), Contents.Str);
            SystemCloseFile(FileHandle);
        }
    }

    return Result;
}

API_INTERNAL FileID
GetFileID(Str8 FilePath)
{
    SystemAccessKind Access = (SystemAccessKind) (SYS_ACCESS_READ | SYS_ACCESS_SHARE_READ);
    Handle FileHandle = SystemOpenFile(Access, FilePath);
    FileID ID = SystemGetFileID(FileHandle);

    SystemCloseFile(FileHandle);

    return ID;
}

u64
RingWrite(
    u8* RingBase, 
    u64 RingSize, 
    u64 RingPosition, 
    void* SrcData, 
    u64 SrcDataSize
) {
    ASSERT(SrcDataSize <= RingSize);

    u64 RingOffset = RingPosition % RingSize;
    u64 BytesBeforeSplit = RingSize - RingOffset;
    u64 PreSplitBytes = MIN(BytesBeforeSplit, SrcDataSize);
    u64 PostSplitBytes = SrcDataSize - PreSplitBytes;
    void* PreSplitData = SrcData;
    void* PostSplitData = ((u8*) SrcData + PreSplitBytes);

    MemCpy(RingBase + RingOffset, PreSplitData, PreSplitBytes);
    MemCpy(RingBase + 0, PostSplitData, PostSplitBytes);

    return SrcDataSize;
}

u64
RingRead(
    u8* RingBase, 
    u64 RingSize, 
    u64 RingPosition, 
    void* DstData, 
    u64 DstDataSize
) {
    ASSERT(DstDataSize <= RingSize);

    u64 RingOffset = RingPosition % RingSize;
    u64 BytesBeforeSplit = RingSize - RingOffset;
    u64 PreSplitBytes = MIN(BytesBeforeSplit, DstDataSize);
    u64 PostSplitBytes = DstDataSize - PreSplitBytes;

    MemCpy(DstData, RingBase + RingOffset, PreSplitBytes);
    MemCpy((u8*) DstData + PreSplitBytes, RingBase + 0, PostSplitBytes);

    return DstDataSize;
}

Str8
SystemReadFileRangeContentsEx(Arena* MemPool, Handle FileHandle, r1u64 FileRange, u64 Align)
{
    u64 PreviousPosition = ArenaGetPosition(MemPool);
    Str8 Result = {};

    Result.Size = Length(FileRange);
    Result.Str = ArenaPushArrayEx(MemPool, u8, Result.Size, Align);

    u64 BytesRead = SystemReadFile(FileHandle, FileRange, Result.Str);

    if (BytesRead < Result.Size) {
        ArenaPopTo(MemPool, PreviousPosition + BytesRead);
        Result.Size = BytesRead;
    }

    return Result;
}

Str8
ReadFileContentsEx(Arena* MemPool, Str8 FilePath, u64 Align)
{
    SystemAccessKind Access = (SystemAccessKind) (SYS_ACCESS_READ | SYS_ACCESS_SHARE_READ);
    Handle FileHandle = SystemOpenFile(Access, FilePath);
    FileProperties Properties = SystemGetFileProperties(FileHandle);
    Str8 Result = SystemReadFileRangeContentsEx(MemPool, FileHandle, Rng((u64) 0, Properties.Size), Align);

    SystemCloseFile(FileHandle);

    return Result;
}
