#include "linux_platform.h"

struct LinuxFileIter {
    DIR*    Dir;
    dirent* DP;
    Str8    Path;
};
STATIC_ASSERT(sizeof(MEMBER(FileIter, Memory)) >= sizeof(LinuxFileIter), FileIterMemorySizeCheck);

INTERNAL FileProperties
StatToFileProperties(struct stat* Stat)
{
    FileProperties Result = {};

    Result.Size = Stat->st_size;
    Result.Created = TimeTimeSpecToWallClockTime(Stat->st_ctim);
    Result.Modified = TimeTimeSpecToWallClockTime(Stat->st_mtim);

    if (Stat->st_mode & S_IFDIR)
        Result.Kind |= SYS_FILE_IS_DIR;

    return Result;
}

Handle
SystemOpenFile(SystemAccessKind Access, Str8 FilePath)
{
    TempArena Scratch = GetScratch(NULL, 0);
    Str8 PathCopy = ArenaPushStrCpy(Scratch.MemPool, FilePath);
    int AccessFlags = 0;

    if (Access & SYS_ACCESS_READ && Access & SYS_ACCESS_WRITE)
        AccessFlags = O_RDWR;
    else if (Access & SYS_ACCESS_WRITE)
        AccessFlags = O_WRONLY;
    else if (Access & SYS_ACCESS_READ)
        AccessFlags = O_RDONLY;

    if (Access & SYS_ACCESS_APPEND)
        AccessFlags |= O_APPEND;

    if (Access & (SYS_ACCESS_WRITE | SYS_ACCESS_APPEND))
        AccessFlags |= O_CREAT;

    AccessFlags |= O_CLOEXEC;

    int FileDesc = open((char*) PathCopy.Str, AccessFlags, 0755);
    Handle Result = {};

    if (FileDesc != -1)
        Result.V[0] = FileDesc;

    ReleaseScratch(Scratch);

    return Result;
}

void
SystemCloseFile(Handle FileHandle)
{
    if (FileHandle == EMPTY_HANDLE_VALUE)
        return;

    int FileDesc = (int) FileHandle.V[0];

    close(FileDesc);
}

u64
SystemReadFile(Handle FileHandle, r1u64 FileRange, OUT void* Contents)
{
    if (FileHandle == EMPTY_HANDLE_VALUE)
        return 0;

    int FileDesc = (int) FileHandle.V[0];
    u64 BytesToRead = Length(FileRange);
    u64 BytesRead = 0;
    u64 AmountRemaining = BytesToRead;

    for ( ; AmountRemaining; ) {
        int ReadResult = pread(
            FileDesc,
            (u8*) Contents + BytesRead,
            AmountRemaining,
            FileRange.Min + BytesRead
        );

        if (ReadResult >= 0) {
            BytesRead += ReadResult;
            AmountRemaining -= ReadResult;
        } else if (errno != EINTR) {
            break;
        }
    }

    return BytesRead;
}

u64
SystemWriteFile(Handle FileHandle, r1u64 FileRange, void* Contents)
{
    if (FileHandle == EMPTY_HANDLE_VALUE)
        return 0;

    int FileDesc = (int) FileHandle.V[0];
    u64 BytesToWrite = Length(FileRange);
    u64 BytesWritten = 0;
    u64 AmountRemaining = BytesToWrite;

    for ( ; AmountRemaining; ) {
        int WriteResult = pwrite(
            FileDesc,
            (u8*) Contents + BytesWritten,
            AmountRemaining,
            FileRange.Min + BytesWritten
        );

        if (WriteResult >= 0) {
            BytesWritten += WriteResult;
            AmountRemaining -= WriteResult;
        } else if (errno != EINTR) {
            break;
        }
    }

    return BytesWritten;
}

b32
SystemSetFileTimes(Handle FileHandle, DateTime Time)
{
    if (FileHandle == EMPTY_HANDLE_VALUE)
        return FALSE;

    int FileDesc = (int) FileHandle.V[0];
    timespec SystemTime = TimeDateTimeToTimeSpec(Time);
    timespec SystemTimes[2] = {
        SystemTime,
        SystemTime
    };
    int FuTimeNSResult = futimens(FileDesc, SystemTimes);
    b32 Okay = (FuTimeNSResult != -1);

    return Okay;
}

FileProperties
SystemGetFileProperties(Handle FileHandle)
{
    FileProperties Result = {};

    if (FileHandle == EMPTY_HANDLE_VALUE)
        return Result;

    int FileDesc = (int) FileHandle.V[0];
    struct stat FileDescStat = {};
    int FStatResult = fstat(FileDesc, &FileDescStat);

    if (FStatResult != -1)
        Result = StatToFileProperties(&FileDescStat);

    return Result;
}

FileProperties
SystemGetFileProperties(Str8 FilePath)
{
    FileProperties Result = {};
    TempArena Scratch = GetScratch(NULL, 0);
    Str8 PathCopy = ArenaPushStrCpy(Scratch.MemPool, FilePath);
    struct stat FileStat = {};
    int StatResult = stat((char*) PathCopy.Str, &FileStat);

    if (StatResult != -1)
        Result = StatToFileProperties(&FileStat);

    ReleaseScratch(Scratch);

    return Result;
}

FileID
SystemGetFileID(Handle FileHandle)
{
    FileID Result = {};

    if (FileHandle == EMPTY_HANDLE_VALUE)
        return Result;

    int FileDesc = (int) FileHandle.V[0];
    struct stat FileDescStat = {};
    int FStatResult = fstat(FileDesc, &FileDescStat);

    if (FStatResult != -1) {
        Result.V[0] = FileDescStat.st_dev;
        Result.V[1] = FileDescStat.st_ino;
    }

    return Result;
}

b32
SystemDeleteFile(Str8 FilePath)
{
    b32 Result = FALSE;
    TempArena Scratch = GetScratch(NULL, 0);
    Str8 PathCopy = ArenaPushStrCpy(Scratch.MemPool, FilePath);

    if (remove((char*) PathCopy.Str) != -1)
        Result = TRUE;

    ReleaseScratch(Scratch);

    return Result;
}

b32
SystemCopyFile(Str8 SrcPath, Str8 DstPath)
{
    b32 Result = FALSE;
    Handle SrcHandle = SystemOpenFile(SYS_ACCESS_READ, SrcPath);
    Handle DstHandle = SystemOpenFile(SYS_ACCESS_WRITE, DstPath);

    if (
        SrcHandle != EMPTY_HANDLE_VALUE &&
        DstHandle != EMPTY_HANDLE_VALUE
    ) {
        int SrcFileDesc = (int) SrcHandle.V[0];
        int DstFileDesc = (int) DstHandle.V[0];
        FileProperties SrcProps = SystemGetFileProperties(SrcHandle);
        u64 Size = SrcProps.Size;
        u64 BytesCopied = 0;
        u64 AmountRemaining = Size;

        for ( ; AmountRemaining; ) {
            off_t SendFileOffset = BytesCopied;
            int SendResult = sendfile(
                DstFileDesc, 
                SrcFileDesc, 
                &SendFileOffset, 
                AmountRemaining
            );

            if (SendResult <= 0)
                break;

            AmountRemaining -= (u64) SendResult;
            BytesCopied += (u64) SendResult;
        }
    }

    SystemCloseFile(SrcHandle);
    SystemCloseFile(DstHandle);

    return Result;
}

b32
SystemMoveFile(Str8 DstPath, Str8 FilePath)
{
    b32 Result = FALSE;
    TempArena Scratch = GetScratch(NULL, 0);

    {
        char* FilePathString = (char*) ArenaPushStrCpy(
            Scratch.MemPool, 
            FilePath
        ).Str;
        char* DstString = (char*) ArenaPushStrCpy(
            Scratch.MemPool,
            DstPath
        ).Str;
        int RenameResult = rename(FilePathString, DstString);

        Result = (RenameResult != -1);
    }

    ReleaseScratch(Scratch);

    return Result;
}

Str8
SystemGetFullFilePath(Arena *MemPool, Str8 FilePath)
{
    TempArena Scratch = GetScratch(&MemPool, 1);
    Str8 PathCopy = ArenaPushStrCpy(Scratch.MemPool, FilePath);
    char Buffer[PATH_MAX] = {};

    realpath((char*) PathCopy.Str, Buffer);

    Str8 Result = ArenaPushStrCpy(MemPool, Str(Buffer));

    ReleaseScratch(Scratch);

    return Result;
}

b32
SystemFileExists(Str8 FilePath)
{
    b32 Result = FALSE;
    TempArena Scratch = GetScratch(NULL, 0);
    Str8 PathCopy = ArenaPushStrCpy(Scratch.MemPool, FilePath);
    int AccessResult = access((char*) PathCopy.Str, F_OK);

    Result = AccessResult == 0;
    ReleaseScratch(Scratch);

    return Result;
}

b32
SystemDirExists(Str8 DirPath)
{
    b32 Result = FALSE;
    TempArena Scratch = GetScratch(NULL, 0);
    Str8 PathCopy = ArenaPushStrCpy(Scratch.MemPool, DirPath);
    DIR* DirHandle = opendir((char*) PathCopy.Str);

    if (DirHandle) {
        closedir(DirHandle);
        Result = TRUE;
    }

    ReleaseScratch(Scratch);

    return Result;
}

Handle
SystemOpenFileMap(SystemAccessKind Access, Handle FileHandle)
{
    Handle Result = FileHandle;

    UNUSED(Access);

    return Result;
}

void
SystemCloseFileMap(Handle MapHandle)
{
    UNUSED(MapHandle);
}

void*
SystemOpenFileMapView(Handle MapHandle, SystemAccessKind Access, r1u64 MapRange)
{
    void* Result = NULL;

    if (MapHandle == EMPTY_HANDLE_VALUE)
        return Result;

    int MapDesc = (int) MapHandle.V[0];
    int ProtFlags = 0;

    if (Access & SYS_ACCESS_WRITE)
        ProtFlags |= PROT_WRITE;

    if (Access & SYS_ACCESS_READ)
        ProtFlags |= PROT_READ;

    int MapFlags = MAP_PRIVATE;
    Result = mmap(
        NULL,
        Length(MapRange),
        ProtFlags,
        MapFlags,
        MapDesc,
        MapRange.Min
    );

    if (Result == MAP_FAILED)
        Result = NULL;

    return Result;
}

void
SystemCloseFileMapView(Handle MapHandle, void *Ptr, r1u64 MapRange)
{
    munmap(Ptr, Length(MapRange));
    UNUSED(MapHandle);
}

FileIter*
SystemFileIterBegin(Arena* MemPool, Str8 FilePath, SystemFileIterKind Method)
{
    FileIter* Iter = ArenaPushArrayZero(MemPool, FileIter, 1);

    Iter->Kind = Method;

    LinuxFileIter* SysFileIter = (LinuxFileIter*) Iter->Memory;
    Str8 PathCopy = ArenaPushStrCpy(MemPool, FilePath);

    SysFileIter->Dir = opendir((char*) PathCopy.Str);
    SysFileIter->Path = PathCopy;

    return Iter;
}

b32
SystemFileIterNext(Arena *MemPool, FileIter *Iter, FileInfo *Info)
{
    b32 Result = FALSE;
    LinuxFileIter* SysFileIter = (LinuxFileIter*) Iter->Memory;

    for ( ; SysFileIter->Dir; ) {
        SysFileIter->DP = readdir(SysFileIter->Dir);
        Result = !!SysFileIter->DP;

        struct stat Stat = {};
        int StatResult = 0;

        if (Result) {
            TempArena Scratch = GetScratch(&MemPool, 1);
            Str8 FullPath = ArenaPushStrFmt(
                Scratch.MemPool,
                "%S/%s",
                PRINT_STR(SysFileIter->Path),
                SysFileIter->DP->d_name
            );

            StatResult = stat((char*) FullPath.Str, &Stat);
            ReleaseScratch(Scratch);
        }

        b32 Filtered = FALSE;

        if (Result) {
            Filtered = (
                (Stat.st_mode == S_IFDIR && Iter->Kind & SYS_FILE_ITER_SKIP_DIRS) ||
                (Stat.st_mode == S_IFREG && Iter->Kind & SYS_FILE_ITER_SKIP_FILES) ||
                (SysFileIter->DP->d_name[0] == '.' && !SysFileIter->DP->d_name[1]) ||
                (
                    SysFileIter->DP->d_name[0] == '.' && 
                    SysFileIter->DP->d_name[1] == '.' &&
                    !SysFileIter->DP->d_name[2]
                )
            );
        }

        if (Result && !Filtered) {
            Info->Name = ArenaPushStrCpy(MemPool, Str(SysFileIter->DP->d_name));

            if (StatResult != -1)
                Info->Properties = StatToFileProperties(&Stat);

            break;
        }

        if (!Result)
            break;
    }

    return Result;
}

void
SystemFileIterEnd(FileIter* Iter)
{
    LinuxFileIter* SysFileIter = (LinuxFileIter*) Iter->Memory;

    closedir(SysFileIter->Dir);
}

b32
SystemCreateDirectory(Str8 DirPath)
{
    b32 Result = FALSE;
    TempArena Scratch = GetScratch(NULL, 0);
    Str8 PathCopy = ArenaPushStrCpy(Scratch.MemPool, DirPath);

    if (mkdir((char*) PathCopy.Str, 0755) != -1)
        Result = TRUE;

    ReleaseScratch(Scratch);

    return Result;
}

Handle
SystemOpenLibrary(Str8 LibraryPath)
{
    TempArena Scratch = GetScratch(NULL, 0);
    char* PathString = (char*) ArenaPushStrCpy(
        Scratch.MemPool, 
        LibraryPath
    ).Str;
    void* Library = dlopen(PathString, RTLD_LAZY | RTLD_LOCAL);
    Handle Result = {
        INT_FROM_PTR(Library)
    };

    ReleaseScratch(Scratch);

    return Result;
}

void
SystemCloseLibrary(Handle LibraryHandle)
{
    void* Library = PTR_FROM_INT(LibraryHandle.V[0]);

    dlclose(Library);
}

vproc*
SystemLibraryLoadProc(Handle LibraryHandle, Str8 ProcName)
{
    TempArena Scratch = GetScratch(NULL, 0);
    void* Library = PTR_FROM_INT(LibraryHandle.V[0]);
    char* NameString = (char*) ArenaPushStrCpy(
        Scratch.MemPool, 
        ProcName
    ).Str;
    vproc* Result = (vproc*) dlsym(Library, NameString);

    ReleaseScratch(Scratch);

    return Result;
}
