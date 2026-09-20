#if !defined(__NC_FILE_H__)
#define __NC_FILE_H__

#include "nc_types.h"
#include "nc_time.h"
#include "nc_string.h"
#include "nc_system.h"

// @defines____________________________________________________________________
#define SystemReadStructFromFile(FileHandle, FileOffset, Struct) SystemReadFile((FileHandle), Rng((u64) (FileOffset), (FileOffset) + sizeof(*(Struct))), (Struct))

// @types______________________________________________________________________
typedef u32 SystemFileIterKind;
enum : u32 {
    SYS_FILE_ITER_SKIP_DIRS =   (1 << 0),
    SYS_FILE_ITER_SKIP_FILES =  (1 << 1),
    SYS_FILE_ITER_SKIP_HIDDEN = (1 << 2),
    SYS_FILE_ITER_DONE =        (1 << 3)
};

typedef u8 SystemFileKind;
enum : u8 {
    SYS_FILE_IS_DIR = (1 << 0)
};

struct FileProperties {
    u64 Size;
    WallClockTime Modified;
    WallClockTime Created;
    SystemFileKind Kind;
};

struct FileIter {
    SystemFileIterKind Kind;
    u8 Memory[800];
};

struct FileInfo {
    Str8 Name;
    FileProperties Properties;
};

struct FileID {
    u64 V[3];
};

#if defined(NC_OS_WIN)
    struct WinFileIter;
#elif defined(NC_OS_LINUX)
    struct LinuxFileIter;
#elif defined(NC_OS_MAC)
    struct MacFileIter;
#endif

// @functions__________________________________________________________________
Handle SystemOpenFile(SystemAccessKind Access, Str8 FilePath);
void SystemCloseFile(Handle FileHandle);
u64 SystemReadFile(Handle FileHandle, r1u64 FileRange, OUT void* Contents);
Str8 SystemReadFileRangeContents(Arena* MemPool, Handle FileHandle, r1u64 FileRange);
u64 SystemWriteFile(Handle FileHandle, r1u64 FileRange, void* Contents);
b32 SystemSetFileTimes(Handle FileHandle, DateTime Time);
FileProperties SystemGetFileProperties(Handle FileHandle);
FileProperties SystemGetFileProperties(Str8 FilePath);
FileID SystemGetFileID(Handle FileHandle);
b32 SystemReserveFileSize(Handle FileHandle, u64 Size);
b32 SystemDeleteFile(Str8 FilePath);
Str8 SystemGetFullFilePath(Arena* MemPool, Str8 FilePath);
b32 SystemFileExists(Str8 FilePath);
b32 SystemDirExists(Str8 DirPath);
Handle SystemOpenFileMap(SystemAccessKind Access, Handle FileHandle);
void SystemCloseFileMap(Handle MapHandle);
void* SystemOpenFileMapView(Handle MapHandle, SystemAccessKind Access, r1u64 MapRange);
void SystemCloseFileMapView(Handle MapHandle, void* Ptr, r1u64 MapRange);
FileIter* SystemFileIterBegin(Arena* MemPool, Str8 FilePath, SystemFileIterKind Method);
b32 SystemFileIterNext(Arena* MemPool, FileIter* Iter, OUT FileInfo* Info);
void SystemFileIterEnd(FileIter* Iter);
Handle SystemOpenLibrary(Str8 LibraryPath);
void SystemCloseLibrary(Handle LibraryHandle);
vproc* SystemLibraryLoadProc(Handle LibraryHandle, Str8 ProcName);
Str8 ReadFileContents(Arena* MemPool, Str8 FilePath);
b32 WriteFileContents(Str8 FilePath, Str8 Contents);
b32 WriteFileContents(Str8 FilePath, Str8List ContentList);
b32 AppendFileContents(Str8 FilePath, Str8 Contents);
FileID GetFileID(Str8 FilePath);
b32 SystemCreateDirectory(Str8 DirPath);
b32 SystemMoveFile(Str8 DstPath, Str8 FilePath);
u64 RingWrite(u8* RingBase, u64 RingSize, u64 RingPosition, void* SrcData, u64 SrcDataSize);
u64 RingRead(u8* RingBase, u64 RingSize, u64 RingPosition, void* DstData, u64 DstDataSize);
Str8 ReadFileContentsEx(Arena* MemPool, Str8 FilePath, u64 Align);
Str8 SystemReadFileRangeContentsEx(Arena* MemPool, Handle FileHandle, r1u64 FileRange, u64 Align);

#endif // __NC_FILE_H__
