#include "win32_platform.h"

struct WinFileIter {
    HANDLE IterHandle;
    WIN32_FIND_DATAW FindData;
    b32 IsVolumeIter;
    Str8Array DriveStrings;
    u64 DriveStringsIndex;
};
STATIC_ASSERT(sizeof(MEMBER(FileIter, Memory)) >= sizeof(WinFileIter), FileIterMemorySizeCheck);

INTERNAL WallClockTime 
TimeFileTimeToWallClock(FILETIME const* FTime)
{
    return ((u64) FTime->dwHighDateTime << 32) | FTime->dwLowDateTime;
}

INTERNAL FILETIME
TimeWallClockToFileTime(WallClockTime ClockTime)
{
    FILETIME Result = {};

    Result.dwLowDateTime = (u32) (ClockTime & U32_MAX);
    Result.dwHighDateTime = (u32) ((ClockTime >> 32) & U32_MAX);

    return Result;
}

INTERNAL Handle
HandleFromHANDLE(HANDLE RawHandle)
{
    Handle Result = {};

    if (RawHandle != INVALID_HANDLE_VALUE)
        *Result.V = INT_FROM_PTR(RawHandle);

    return Result;
}

INTERNAL SystemFileKind
FilePropertyKindFromFileAttributes(DWORD Attributes)
{
    if (Attributes & FILE_ATTRIBUTE_DIRECTORY)
        return SYS_FILE_IS_DIR;

    return (SystemFileKind) 0;
}

Handle 
SystemOpenFile(SystemAccessKind Access, Str8 FilePath)
{
    Handle Result = {};
    TempArena Scratch = GetScratch(NULL, 0);
    Str16 WidePath = Str16FromStr8(Scratch.MemPool, FilePath);
    DWORD AccessFlags = 0;
    DWORD ShareMode = 0;
    DWORD CreationDisposition = OPEN_EXISTING;
    SECURITY_ATTRIBUTES SecurityAttributes = {
        sizeof(SecurityAttributes),
        0,
        0
    };

    if (Access & SYS_ACCESS_READ)
        AccessFlags |= GENERIC_READ;

    if (Access & SYS_ACCESS_WRITE)
        AccessFlags |= GENERIC_WRITE;

    if (Access & SYS_ACCESS_EXECUTE)
        AccessFlags |= GENERIC_EXECUTE;

    if (Access & SYS_ACCESS_SHARE_READ)
        AccessFlags |= FILE_SHARE_READ;

    if (Access & SYS_ACCESS_SHARE_WRITE)
        AccessFlags |= FILE_SHARE_WRITE | FILE_SHARE_DELETE;

    if (Access & SYS_ACCESS_WRITE)
        CreationDisposition = CREATE_ALWAYS;

    if (Access & SYS_ACCESS_APPEND) {
        CreationDisposition = OPEN_ALWAYS;
        AccessFlags |= FILE_APPEND_DATA;
    }

    if (Access & SYS_ACCESS_INHERITED) {
        SecurityAttributes.bInheritHandle = TRUE;
    }

    HANDLE File = CreateFileW(
        (WCHAR*) WidePath.Str,
        AccessFlags,
        ShareMode,
        &SecurityAttributes,
        CreationDisposition,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (File != INVALID_HANDLE_VALUE) {
        Result.V[0] = INT_FROM_PTR(File);
    } else {
        DWORD Err = GetLastError();

        UNUSED(Err);
    }

    ReleaseScratch(Scratch);

    return Result;
}

void 
SystemCloseFile(Handle FileHandle)
{
    if (FileHandle == EMPTY_HANDLE_VALUE)
        return;

    HANDLE SysFileHandle = (HANDLE) *FileHandle.V;
    BOOL Result = CloseHandle(SysFileHandle);

    UNUSED(Result);
}

u64 
SystemReadFile(Handle FileHandle, r1u64 FileRange, OUT void* Contents)
{
    if (FileHandle == EMPTY_HANDLE_VALUE)
        return 0;

    HANDLE SysFileHandle = (HANDLE) *FileHandle.V;
    u64 Size = 0;

    GetFileSizeEx(SysFileHandle, (LARGE_INTEGER*) &Size);

    r1u64 RangeClamped = Rng(
        CLAMP_TOP(FileRange.Min, Size), 
        CLAMP_TOP(FileRange.Max, Size)
    );
    u64 BytesRead = 0;
    u64 BytesToRead = Length(RangeClamped);

    for (u64 I = FileRange.Min; BytesRead < BytesToRead;) {
        u64 AmountRemaining64 = BytesToRead - BytesRead;
        u32 AmountRemaining32 = SafeTruncateU64(AmountRemaining64);
        DWORD ReadSize = 0;
        OVERLAPPED Overlapped = {};

        Overlapped.Offset = (I & 0x00000000FFFFFFFFULL);
        Overlapped.OffsetHigh = (I & 0xFFFFFFFF00000000ULL) >> 32;
        ReadFile(SysFileHandle, (u8*) Contents + BytesRead, AmountRemaining32, &ReadSize, &Overlapped);
        I += ReadSize;
        BytesRead += ReadSize;

        if (ReadSize != AmountRemaining32)
            break;
    }

    return BytesRead;
}


u64 
SystemWriteFile(Handle FileHandle, r1u64 FileRange, void* Contents)
{
    if (FileHandle == EMPTY_HANDLE_VALUE)
        return 0;

    HANDLE SysFileHandle = (HANDLE) *FileHandle.V;
    u64 SrcOffset = 0;
    u64 DstOffset = FileRange.Min;
    u64 BytesToWrite = Length(FileRange);

    for (;;) {
        void* BytesSrc = (u8*) Contents + SrcOffset;
        u64 AmountRemaining = BytesToWrite - SrcOffset;
        DWORD AmountToWrite = MIN(MB(1), AmountRemaining);
        DWORD BytesWritten = 0;
        OVERLAPPED Overlapped = {};

        Overlapped.Offset = (DstOffset & 0x00000000FFFFFFFFULL);
        Overlapped.OffsetHigh = (DstOffset & 0xFFFFFFFF00000000ULL) >> 32;

        BOOL Success = WriteFile(SysFileHandle, BytesSrc, AmountToWrite, &BytesWritten, &Overlapped);

        if (!Success)
            break;

        SrcOffset += BytesWritten;
        DstOffset += BytesWritten;

        if (!AmountRemaining)
            break;
    }

    return SrcOffset;
}

b32 
SystemSetFileTimes(Handle FileHandle, DateTime Time)
{
    if (FileHandle == EMPTY_HANDLE_VALUE)
        return FALSE;

    b32 Result = FALSE;
    HANDLE SysFileHandle = (HANDLE) *FileHandle.V;
    SYSTEMTIME SystemTime = {};
    WallClockTime ClockTime = TimeDateTimeToWallClock(&Time);
    FILETIME FileTime = TimeWallClockToFileTime(ClockTime);

    Result = (
        SetFileTime(SysFileHandle, &FileTime, &FileTime, &FileTime)
    );

    return Result;
}

FileProperties 
SystemGetFileProperties(Handle FileHandle)
{
    FileProperties Result = {};

    if (FileHandle == EMPTY_HANDLE_VALUE)
        return Result;

    HANDLE SysFileHandle = (HANDLE) PTR_FROM_INT(*FileHandle.V);
    BY_HANDLE_FILE_INFORMATION FileInfo;

    if (GetFileInformationByHandle(SysFileHandle, &FileInfo)) {
        Result.Size = ((u64) FileInfo.nFileSizeHigh << 32) | FileInfo.nFileSizeLow;
        Result.Modified = TimeFileTimeToWallClock(&FileInfo.ftLastWriteTime);
        Result.Created = TimeFileTimeToWallClock(&FileInfo.ftCreationTime);
        Result.Kind = FilePropertyKindFromFileAttributes(FileInfo.dwFileAttributes);
    }

    return Result;
}

FileProperties 
SystemGetFileProperties(Str8 FilePath)
{
    FileProperties Result = {};
    TempArena Scratch = GetScratch(NULL, 0);
    Str16 FilePathWide = Str16FromStr8(Scratch.MemPool, FilePath);
    WIN32_FIND_DATAW FindData;
    HANDLE FileHandle = FindFirstFileW((WCHAR*) FilePathWide.Str, &FindData);

    if (FileHandle != INVALID_HANDLE_VALUE) {
        Result.Size = ((u64) FindData.nFileSizeHigh << 32) | FindData.nFileSizeLow;
        Result.Modified = TimeFileTimeToWallClock(&FindData.ftLastWriteTime);
        Result.Created = TimeFileTimeToWallClock(&FindData.ftCreationTime);
        Result.Kind = FilePropertyKindFromFileAttributes(FindData.dwFileAttributes);
        FindClose(FileHandle);
    } else {
        DWORD Attributes = GetFileAttributesW((WCHAR*) FilePathWide.Str);

        if (Attributes != INVALID_FILE_ATTRIBUTES)
            Result.Kind = FilePropertyKindFromFileAttributes(Attributes);
    }

    ReleaseScratch(Scratch);

    return Result;
}

FileID 
SystemGetFileID(Handle FileHandle)
{
    FileID Result = {};

    if (FileHandle == EMPTY_HANDLE_VALUE)
        return Result;

    HANDLE SysFileHandle = (HANDLE) *FileHandle.V;
    BY_HANDLE_FILE_INFORMATION Information;
    BOOL Success = GetFileInformationByHandle(SysFileHandle, &Information);

    if (Success) {
        Result.V[0] = Information.dwVolumeSerialNumber;
        Result.V[1] = Information.nFileIndexLow;
        Result.V[2] = Information.nFileSizeHigh;
    }

    return Result;
}

b32 
SystemReserveFileSize(Handle FileHandle, u64 Size)
{
    b32 Result = FALSE;
    HANDLE SysFileHandle = (HANDLE) *FileHandle.V;
    FILE_ALLOCATION_INFO Information = {};

    Information.AllocationSize.LowPart = Size & U32_MAX;
    Information.AllocationSize.HighPart = (Size >> 32) & U32_MAX;
    Result = SetFileInformationByHandle(SysFileHandle, FileAllocationInfo, &Information, sizeof(Information));

    return Result;
}

b32 
SystemDeleteFile(Str8 FilePath)
{
    b32 Result = FALSE;
    TempArena Scratch = GetScratch(NULL, 0);
    Str16 WideFilePath = Str16FromStr8(Scratch.MemPool, FilePath);

    Result = DeleteFileW((WCHAR*) WideFilePath.Str);
    ReleaseScratch(Scratch);

    return Result;
}

Str8 
SystemGetFullFilePath(Arena* MemPool, Str8 FilePath)
{
    TempArena Scratch = ArenaBeginTemp(MemPool);
    Str16 WideFilePath = Str16FromStr8(Scratch.MemPool, FilePath);
    DWORD BufferSize = MAX(MAX_PATH, FilePath.Size * 2) + 1;
    WCHAR* Buffer = ArenaPushArray(Scratch.MemPool, WCHAR, BufferSize);
    DWORD FilePathSize = GetFullPathNameW((WCHAR*) WideFilePath.Str, BufferSize, Buffer, NULL);

    if (FilePathSize > BufferSize) {
        ArenaPop(Scratch.MemPool, BufferSize);
        BufferSize = FilePathSize + 1;
        Buffer = ArenaPushArray(Scratch.MemPool, WCHAR, BufferSize);
        FilePathSize = GetFullPathNameW((WCHAR*) WideFilePath.Str, BufferSize, Buffer, NULL);
    }

    Str8 Result = Str((u8*) Buffer, BufferSize);
    Result = ArenaPushStrCpy(MemPool, Result);
    ArenaEndTemp(Scratch);

    return Result;
}

b32 
SystemFileExists(Str8 FilePath)
{
    b32 Result = FALSE;
    TempArena Scratch = GetScratch(NULL, 0);
    Str16 WideFilePath = Str16FromStr8(Scratch.MemPool, FilePath);
    DWORD Attributes = GetFileAttributesW((WCHAR*) WideFilePath.Str);

    Result = ((Attributes != INVALID_FILE_ATTRIBUTES) && !!(~Attributes & FILE_ATTRIBUTE_DIRECTORY));
    ReleaseScratch(Scratch);

    return Result;
}

b32 
SystemDirExists(Str8 DirPath)
{
    b32 Result = FALSE;
    TempArena Scratch = GetScratch(NULL, 0);
    Str16 WideDirPath = Str16FromStr8(Scratch.MemPool, DirPath);
    DWORD Attributes = GetFileAttributesW((WCHAR*) WideDirPath.Str);

    Result = ((Attributes != INVALID_FILE_ATTRIBUTES) && (Attributes & FILE_ATTRIBUTE_DIRECTORY));
    ReleaseScratch(Scratch);

    return Result;
}

Handle 
SystemOpenFileMap(SystemAccessKind Access, Handle FileHandle)
{
    Handle Result = {};
    HANDLE SysFileHandle = (HANDLE) *FileHandle.V;
    DWORD ProtectKind = 0;

    switch (Access) {
        default: {} break;

        case SYS_ACCESS_READ: {
            ProtectKind = PAGE_READONLY;
        } break;

        case SYS_ACCESS_WRITE:
        case (SYS_ACCESS_READ | SYS_ACCESS_WRITE): {
            ProtectKind = PAGE_READWRITE;
        } break;

        case SYS_ACCESS_EXECUTE:
        case (SYS_ACCESS_READ | SYS_ACCESS_EXECUTE): {
            ProtectKind = PAGE_EXECUTE_READ;
        } break;

        case (SYS_ACCESS_EXECUTE | SYS_ACCESS_WRITE | SYS_ACCESS_READ):
        case (SYS_ACCESS_EXECUTE | SYS_ACCESS_WRITE): {
            ProtectKind = PAGE_EXECUTE_READWRITE;
        } break;
    }

    HANDLE MapHandle = CreateFileMapping(SysFileHandle, NULL, ProtectKind, 0, 0, NULL);

    *Result.V = (u64) MapHandle;

    return Result;
}

void 
SystemCloseFileMap(Handle MapHandle)
{
    HANDLE SysMapHandle = (HANDLE) *MapHandle.V;
    BOOL Result = CloseHandle(SysMapHandle);

    UNUSED(Result);
}

void* 
SystemOpenFileMapView(Handle MapHandle, SystemAccessKind Access, r1u64 MapRange)
{
    HANDLE SysMapHandle = (HANDLE) *MapHandle.V;
    u32 OffsetLo = (u32) ((MapRange.Min & 0x00000000FFFFFFFFULL) >> 0);
    u32 OffsetHi = (u32) ((MapRange.Min & 0xFFFFFFFF00000000ULL) >> 32);
    u64 Size = Length(MapRange);
    DWORD AccessKind = 0;

    {
        switch (Access) {
            default: {} break;

            case SYS_ACCESS_READ: {
                AccessKind = FILE_MAP_READ;
            } break;

            case SYS_ACCESS_WRITE: {
                AccessKind = FILE_MAP_WRITE;
            } break;

            case (SYS_ACCESS_READ | SYS_ACCESS_WRITE): {
                AccessKind = FILE_MAP_ALL_ACCESS;
            } break;

            case SYS_ACCESS_EXECUTE:
            case (SYS_ACCESS_READ | SYS_ACCESS_EXECUTE):
            case (SYS_ACCESS_WRITE | SYS_ACCESS_EXECUTE):
            case (SYS_ACCESS_READ | SYS_ACCESS_WRITE | SYS_ACCESS_EXECUTE): {
                AccessKind = (FILE_MAP_ALL_ACCESS | FILE_MAP_EXECUTE);
            } break;
        }
    }

    void* Result = MapViewOfFile(SysMapHandle, AccessKind, OffsetHi, OffsetLo, Size);

    return Result;
}

void 
SystemCloseFileMapView(Handle MapHandle, void* Ptr, r1u64 MapRange)
{
    BOOL Result = UnmapViewOfFile(Ptr);

    UNUSED(Result);
    UNUSED(MapHandle);
    UNUSED(MapRange);
}

FileIter* 
SystemFileIterBegin(Arena* MemPool, Str8 FilePath, SystemFileIterKind Method)
{
    TempArena Scratch = GetScratch(&MemPool, 1);
    Str8 FilePathWithWildCard = ArenaPushStrCat(Scratch.MemPool, FilePath, Str8Lit("\\*"));
    Str16 FilePathWide = Str16FromStr8(Scratch.MemPool, FilePathWithWildCard);
    FileIter* Iter = ArenaPushArrayZero(MemPool, FileIter, 1);

    Iter->Kind = Method;

    WinFileIter* SysFileIter = (WinFileIter*) Iter->Memory;

    if (!FilePath.Size) {
        SysFileIter->IsVolumeIter = TRUE;
        
        WCHAR Buffer[512] = {};
        DWORD BufferLength = GetLogicalDriveStringsW(sizeof(Buffer), Buffer);
        Str8List DriveStrings = {};

        for (u64 Offset = 0; Offset < (u64) BufferLength; ) {
            Str16 NextDriveStringWide = CStr16((u16*) Buffer + Offset);

            Offset += NextDriveStringWide.Size + 1;

            Str8 NextDriveString = Str8FromStr16(MemPool, NextDriveStringWide);

            NextDriveString = StrChopLastSlash(NextDriveString);
            ListPush(Scratch.MemPool, &DriveStrings, NextDriveString);
        }

        SysFileIter->DriveStrings = StrArrayFromList(MemPool, &DriveStrings);
        SysFileIter->DriveStringsIndex = 0;
    } else {
        SysFileIter->IterHandle = FindFirstFileExW(
            (WCHAR*) FilePathWide.Str, 
            FindExInfoBasic, 
            &SysFileIter->FindData, 
            FindExSearchNameMatch, 
            0, 
            FIND_FIRST_EX_LARGE_FETCH
        );
    }

    ReleaseScratch(Scratch);

    return Iter;
}

b32 
SystemFileIterNext(Arena* MemPool, FileIter* Iter, OUT FileInfo* Info)
{
    b32 Result = FALSE;
    SystemFileIterKind Kind = Iter->Kind;
    WinFileIter* SysFileIter = (WinFileIter*) Iter->Memory;

    switch (SysFileIter->IsVolumeIter) {
        default:
        case FALSE: {
            if (!(Kind & SYS_FILE_ITER_DONE) && SysFileIter->IterHandle != INVALID_HANDLE_VALUE) {
                do {
                    b32 UsableFile = TRUE;
                    WCHAR* FileName = SysFileIter->FindData.cFileName;
                    DWORD Attributes = SysFileIter->FindData.dwFileAttributes;

                    if (*FileName == '.') {
                        if (Kind & SYS_FILE_ITER_SKIP_HIDDEN)
                            UsableFile = FALSE;
                        else if (!FileName[1])
                            UsableFile = FALSE;
                        else if (FileName[1] == '.' && !FileName[2])
                            UsableFile = FALSE;
                    }

                    if (Attributes & FILE_ATTRIBUTE_DIRECTORY) {
                        if (Kind & SYS_FILE_ITER_SKIP_DIRS)
                            UsableFile = FALSE;
                    } else {
                        if (Kind & SYS_FILE_ITER_SKIP_FILES)
                            UsableFile = FALSE;
                    }

                    if (UsableFile) {
                        Info->Name = Str8FromStr16(MemPool, CStr16((u16*) FileName));
                        Info->Properties.Size = (u64) SysFileIter->FindData.nFileSizeLow | (((u64) SysFileIter->FindData.nFileSizeHigh) << 32);
                        Info->Properties.Created = TimeFileTimeToWallClock(&SysFileIter->FindData.ftCreationTime);
                        Info->Properties.Modified = TimeFileTimeToWallClock(&SysFileIter->FindData.ftLastWriteTime);
                        Info->Properties.Kind = FilePropertyKindFromFileAttributes(Attributes);
                        Result = TRUE;

                        if (!FindNextFileW(SysFileIter->IterHandle, &SysFileIter->FindData))
                            Iter->Kind |= SYS_FILE_ITER_DONE;

                        break;
                    }
                } while (FindNextFileW(SysFileIter->IterHandle, &SysFileIter->FindData));
            }
        } break;

        case TRUE: {
            Result = (SysFileIter->DriveStringsIndex < SysFileIter->DriveStrings.Count);

            if (Result) {
                MemSet(Info, 0, sizeof(*Info));
                Info->Name = SysFileIter->DriveStrings.Data[SysFileIter->DriveStringsIndex];
                Info->Properties.Kind = (SystemFileKind) (Info->Properties.Kind | SYS_FILE_IS_DIR);
                ++SysFileIter->DriveStringsIndex;
            }
        } break;
    }

    if (!Result)
        Iter->Kind |= SYS_FILE_ITER_DONE;

    return Result;
}

void 
SystemFileIterEnd(FileIter* Iter)
{
    WinFileIter* SysFileIter = (WinFileIter*) Iter->Memory;
    HANDLE EmptyHandle = {};

    MemSet(&EmptyHandle, 0, sizeof(EmptyHandle));

    if (!MemCmpStruct(&EmptyHandle, &SysFileIter->IterHandle))
        FindClose(SysFileIter->IterHandle);
}

Handle 
SystemOpenLibrary(Str8 LibraryPath)
{
    Handle Result = {};
    TempArena Scratch = GetScratch(NULL, 0);
    Str16 WideLibraryPath = Str16FromStr8(Scratch.MemPool, LibraryPath);
    HMODULE Module = LoadLibraryW((LPCWSTR) WideLibraryPath.Str);

    *Result.V = (u64) Module;
    ReleaseScratch(Scratch);

    return Result;
}

void 
SystemCloseLibrary(Handle LibraryHandle)
{
    HMODULE Module = (HMODULE) *LibraryHandle.V;

    FreeLibrary(Module);
}

vproc* 
SystemLibraryLoadProc(Handle LibraryHandle, Str8 ProcName)
{
    HMODULE Module = (HMODULE) *LibraryHandle.V;
    vproc* Result = (vproc*) GetProcAddress(Module, (LPCSTR) ProcName.Str);

    return Result;
}

b32 
SystemCreateDirectory(Str8 DirPath)
{
    b32 Result = FALSE;
    TempArena Scratch = GetScratch(NULL, 0);
    Str16 DirPathWide = Str16FromStr8(Scratch.MemPool, DirPath);
    WIN32_FILE_ATTRIBUTE_DATA Attributes = {};

    GetFileAttributesExW((WCHAR*) DirPathWide.Str, GetFileExInfoStandard, &Attributes);

    if (Attributes.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        Result = TRUE;
    else if (CreateDirectoryW((WCHAR*) DirPathWide.Str, NULL))
        Result = TRUE;

    ReleaseScratch(Scratch);

    return Result;
}

b32 
SystemMoveFile(Str8 DstPath, Str8 FilePath)
{
    TempArena Scratch = GetScratch(NULL, 0);
    b32 Result = FALSE;
    Str16 DstPathWide = Str16FromStr8(Scratch.MemPool, DstPath);
    Str16 FilePathWide = Str16FromStr8(Scratch.MemPool, FilePath);

    Result = CopyFileW((WCHAR*) FilePathWide.Str, (WCHAR*) DstPathWide.Str, FALSE);
    ReleaseScratch(Scratch);

    return Result;
}
