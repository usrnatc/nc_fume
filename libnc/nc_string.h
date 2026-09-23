#if !defined(__NC_STRING_H__)
#define __NC_STRING_H__

#include <stdarg.h>

#include "nc_types.h"
#include "nc_math.h"

struct Arena;


// @defines____________________________________________________________________
#define PRINT_STR(X)                (int) (X).Size, (X).Str
#define PRINT_MQTTSTR(X)            (int) (X).Size, (X).Str
#define PRINT_MQTTSTRRAW(X)         (int) (X).Size, (X).Str + 2                 // FIXME: what if we get empty string?
#define CStr8(String)               Str(String)
#define CStr16(String)              Str((u16*) (String), StrLen((u16*) String))
#define CStr32(String)              Str((u32*) (String), StrLen((u32*) String))
#define Str8Struct(X)               Str((u8*) (X), sizeof(*(X)))
#define SerialPushArray(X, Y, Z, W) SerialPush((X), (Y), (Z), sizeof(*(Z)) * (W))
#define SerialPushStruct(X, Y, Z)   SerialPushArray(X, Y, Z, 1)
#define SerialPopArray(X, Y, Z, W)  SerialPop((X), (Y), (Z), sizeof(*(Z)) * W, sizeof(*(Z)))
#define SerialPopStruct(X, Y, Z)    SerialPopArray((X), (Y), (Z), 1)
#define VSCPrintf(X, Y)             __VSNPrintf(NULL, 0, X, Y)
#define VSNPrintf(X, Y, Z, W)       __VSNPrintf(X, Y, Z, W)
#define VSCPrintfW(X, Y)            __VSNPrintfW(NULL, 0, X, Y)
#define VSNPrintfW(X, Y, Z, W)      __VSNPrintfW(X, Y, Z, W)
#define Str8Lit(String)             {(u8*) (String), sizeof(String) - 1}

typedef u32 PathStyle;
enum : u32 {
    PATH_STYLE_NULL,
    PATH_STYLE_RELATIVE,
    PATH_STYLE_WIN_ABSOLUTE,
    PATH_STYLE_UNIX_ABSOLUTE,
#if defined(NC_OS_WIN)
    PATH_STYLE_SYSTEM = PATH_STYLE_WIN_ABSOLUTE,
#elif defined(NC_OS_LINUX)
    PATH_STYLE_SYSTEM = PATH_STYLE_UNIX_ABSOLUTE,
#else
    #error System path style not defined for operating system
#endif
};

typedef u32 StrMatchKind;
enum : u32 {
    STR_MATCH_LAST       = (1 << 0),
    STR_MATCH_ALL_CASES  = (1 << 4),
    STR_MATCH_JUNK_AFTER = (1 << 5),
    STR_MATCH_SKIP_SLASH = (1 << 6)
};

// @types______________________________________________________________________
struct Str8 {
    u8* Str;
    u64 Size;
};

struct Str16 {
    u16* Str;
    u64  Size;
};

struct Str32 {
    u32* Str;
    u64  Size;
};

struct MQTTStr8 {
    u8* Str;
    u16 Size;
};

struct MQTTStr8Raw {
    u8* Str;
    u16 Size;
};

struct Str8JoinPart {
    Str8 Pre;
    Str8 Sep;
    Str8 Post;
};

struct Str8Node {
    Str8Node* Next;
    Str8      String;
};

struct Str8MetaNode {
    Str8MetaNode* Next;
    Str8Node*     Node;
};

struct Str8List {
    Str8Node* Head;
    Str8Node* Tail;
    u64       Count;
    u64       TotalSize;
};

struct Str8Array {
    Str8* Data;
    u64   Count;
};

struct __StringPathStyleMapNode {
    Str8      String;
    PathStyle Style;
};

struct TextPoint {
    i64 Line;
    i64 Column;
};

struct Str8TextPointPair {
    Str8      String;
    TextPoint Point;
};

struct TextRange {
    TextPoint Min;
    TextPoint Max;
};

struct UnicodeDecode {
    u32 Inc;
    u32 CodePoint;
};

struct FMRangeNode {
    FMRangeNode* Next;
    r1u64        Range;
};

struct FMRangeList {
    FMRangeNode* Head;
    FMRangeNode* Tail;
    u64          Count;
    u64          StrPartCount;
    u64          TotalDimensions;
};

// @runtime____________________________________________________________________
extern const u8 UTF8_CLASS[32];

// @functions__________________________________________________________________
u64 StrLen(u8* CString);
u64 StrLen(char* CString);
u64 StrLen(u16* CString);
u64 StrLen(u32* CString);
Str8 Str(u8* String, u64 StringCount);
Str8 Str(char* String);
Str8 Str(u8* StringStart, u8* StringEnd);
Str16 Str(u16* String, u64 StringCount);
Str16 Str(u16* String);
Str32 Str(u32* String, u64 StringCount);
Str32 Str(u32* String);
Str8 ArenaPushStrCpy(Arena* MemPool, Str8 String);
Str8 ArenaPushStrCat(Arena* MemPool, Str8 StringA, Str8 StringB);
Str8Node* ArenaPushStrList(Arena* MemPool, Str8List* List, Str8 String);
void ListPushAlign(Arena* MemPool, Str8List* List, u64 Alignment);
void ListPush(Arena* MemPool, Str8List* List, Str8 String);
Str8Array StrArrayFromList(Arena* MemPool, Str8List* List);
Str8 StrListJoin(Arena* MemPool, Str8List* List, Str8JoinPart const* JoinParts);
Str8List StrListCopy(Arena* MemPool, Str8List* List);
Str8 StrTrimLastSlash(Str8 String);
Str8 StrRange(u8* First, u8 const* End);
Str8 StrSub(Str8 String, r1u64 Range);
Str8 StrSkip(Str8 String, u64 Min);
Str8 StrChop(Str8 String, u64 NMax);
Str8 StrChopLastSlash(Str8 String);
Str8 StrSkipChopWhitespace(Str8 String);
Str8 StrSkipChopEmpty(Str8 String);
Str8 StrPrefix(Str8 String, u64 Size);
Str8 StrSuffix(Str8 String, u64 Size);
Str8 StrPostfix(Str8 String, u64 Size);
b32 StrMatch(Str8 StringA, Str8 StringB, StrMatchKind Method);
u64 StrFindSubStr(Str8 String, Str8 SubString, u64 Start, StrMatchKind Method);
b32 StrIsInteger(Str8 String, u32 Base);
i8 SignFromStr(Str8 String, Str8* StringTail);
u128 U128FromStr(Str8 String, u32 Base = 10);
u64 U64FromStr(Str8 String, u32 Base = 10);
i64 I64FromStr(Str8 String, u32 Base = 10);
u32 U32FromStr(Str8 String, u32 Base = 10);
i32 I32FromStr(Str8 String, u32 Base = 10);
f64 F64FromStr(Str8 String);
UnicodeDecode UTF8DecodeMQTT(u8 const* PacketData, u64 PacketSize);
UnicodeDecode UTF8Decode(u8 const* String, u64 Max);
UnicodeDecode UTF16Decode(u16 const* String, u64 Max);
u32 UTF8Encode(u8* String, u32 CodePoint);
u32 UTF16Encode(u16* String, u32 CodePoint);
Str8 Str8FromStr16(Arena* MemPool, Str16 String);
Str16 Str16FromStr8(Arena* MemPool, Str8 String);
Str8 Str8FromStr32(Arena* MemPool, Str32 String);
Str32 Str32FromStr8(Arena* MemPool, Str8 String);
Str8 ArenaPushStrFmtV(Arena* MemPool, char const* Fmt, va_list Args);
Str8 ArenaPushStrFmt(Arena* MemPool, char* Fmt, ...);
Str16 ArenaPushStrFmtV(Arena* MemPool, wchar const* Fmt, va_list Args);
Str16 ArenaPushStrFmt(Arena* MemPool, wchar* Fmt, ...);
void ListPushFront(Arena* MemPool, Str8List* List, Str8 String);
void ListPushFmt(Arena* MemPool, Str8List* List, char* Fmt, ...);
void ListPushCat(Str8List* List, Str8List* Items);
Str8List StrSplit(Arena* MemPool, Str8 String, u8 const* Splits, u32 SplitsCount, b32 KeepEmpties);
Str8 RawFromEscapedStr8(Arena* MemPool, Str8 String);
Str8 EscapedFromRawStr8(Arena* MemPool, Str8 String);
Str8 PathAbsoluteDstFromRelativeDstSrc(Arena* MemPool, Str8 Dst, Str8 Src);
Str8 PathRelativeDstFromAbsoluteDstSrc(Arena* MemPool, Str8 Dst, Str8 Src);
Str8List PathNormalisedListFromStr(Arena* MemPool, Str8 Path, OUT PathStyle* Style);
Str8 PathNormalisedFromStr(Arena* MemPool, Str8 Path);
b32 PathMatchNormalised(Str8 Left, Str8 Right);
Str8 StrSkipLastSlash(Str8 String);
Str8 StrChopLastDot(Str8 String);
Str8 StrSkipLastDot(Str8 String);
PathStyle PathStyleFromStr(Str8 String);
Str8List StrSplitPath(Arena* MemPool, Str8 String);
void StrPathListResolveDots(Str8List* Path, PathStyle Style);
Str8 StrPathListJoinByStyle(Arena* MemPool, Str8List* Path, PathStyle Style);
Str8TextPointPair Str8TextPointPairFromStr(Str8 String);
Str8 StrFromPathStyle(PathStyle Style);
Str8 PathSeparatorStrFromStyle(PathStyle Style);
StrMatchKind PathMatchKindFromSystem(void);
Str8 PathConvertSlashes(Arena* MemPool, Str8 Path, PathStyle Style);
Str8 PathReplaceFileExtension(Arena* MemPool, Str8 FileName, Str8 Extension);
void StrListCat(Str8List* Dst, Str8List* Src);
Str8 IndentedFromStr(Arena* MemPool, Str8 String);
void StrListFromKinds(Arena* MemPool, Str8List* List, u32 Kinds, Str8 const* KindStringTable, u32 KindStringTableSize);
void SerialBegin(Arena* MemPool, Str8List* List);
Str8 SerialEnd(Arena* MemPool, Str8List* List);
void SerialComplete(Str8List* List, OUT void* Dst);
u64 SerialPushAligned(Arena* MemPool, Str8List* List, u64 Align);
void* SerialPush(Arena* MemPool, Str8List* List, u64 Size);
void* SerialPush(Arena* MemPool, Str8List* List, void* Data, u64 Size);
void SerialPush(Arena* MemPool, Str8List* List, Str8Node* Head);
void SerialPushU64(Arena* MemPool, Str8List* List, u64 Value);
void SerialPushU32(Arena* MemPool, Str8List* List, u32 Value);
void SerialPushU16(Arena* MemPool, Str8List* List, u16 Value);
void SerialPushU8(Arena* MemPool, Str8List* List, u8 Value);
void SerialPushCStr(Arena* MemPool, Str8List* List, Str8 CString);
void SerialPushStr(Arena* MemPool, Str8List* List, Str8 String);
u64 SerialPop(Str8 String, u64 Offset, void* ReadDst, u64 ReadSize, u64 Granularity);
u64 SerialFindFirstMatch(Str8 String, u64 Offset, u16 ScanValue);
void* SerialPopRawPtr(Str8 String, u64 Offset, u64 Size);
u64 SerialPopCStr(Str8 String, u64 Offset, OUT Str8* CString);
u64 SerialPopStr16(Str8 String, u64 Offset, OUT Str16* WideString);
u64 SerialPopBlock(Str8 String, u64 Offset, u64 Size, OUT Str8* Block);
b32 IsSpace(u8 Char);
b32 IsUpper(u8 Char);
b32 IsLower(u8 Char);
b32 IsAlpha(u8 Char);
b32 IsSlash(u8 Char);
b32 IsDigit(u8 Char, u32 Base = 10);
u8 ToLower(u8 Char);
u8 ToUpper(u8 Char);
u8 ToForwardSlash(u8 Char);
u64 Hash(u64 Seed, Str8 String);
u64 Hash(Str8 String);
u128 BigHash(Str8 String);
u128 BigHash(u64 Seed, Str8 String);
TextPoint TxtPt(i64 Line, i64 Column);
TextRange TxtRng(TextPoint Min, TextPoint Max);
FMRangeList FuzzyFind(Arena* MemPool, Str8 Needle, Str8 String);
Str8 ToLower(Arena* MemPool, Str8 String);
FMRangeList ListCpy(Arena* MemPool, FMRangeList* List);

#if defined(NC_LANG_CPP)
    #if NC_LANG_CPP_VERSION != 0 && NC_LANG_CPP_VERSION != 98
        INTERNAL Str8  operator""_s8(char const* CString, sz Size);
        INTERNAL Str16 operator""_s16(const char* CString, sz Size);
        INTERNAL Str32 operator""_s32(const char* CString, sz Size);
    #endif

    INTERNAL b32 operator<(Str8 StringA, Str8 StringB);
    INTERNAL b32 operator==(TextPoint TPA, TextPoint TPB);
    INTERNAL b32 operator!=(TextPoint TPA, TextPoint TPB);
    INTERNAL b32 operator<(TextPoint TPA, TextPoint TPB);
    INTERNAL b32 operator>(TextPoint TPA, TextPoint TPB);
    INTERNAL b32 operator==(TextRange RangeA, TextRange RangeB);
    INTERNAL b32 operator!=(TextRange RangeA, TextRange RangeB);
#endif

#endif // __NC_STRING_H__
