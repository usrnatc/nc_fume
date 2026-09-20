#include <stdarg.h>

#include "nc_string.h"
#include "nc_memory.h"
#include "nc_arena.h"

#if !defined(XXH_IMPLEMENTATION)
    #define XXH_IMPLEMENTATION
    #define XXH_STATIC_LINKING_ONLY
    #define XXH_INLINE_ALL
    #define XXH_NO_STDLIB
    #include "xxHash/xxhash.h"
#endif

// TODO: Conversion functions should be XToY and not YFromX

#if defined(NC_LANG_CPP)
    #if NC_LANG_CPP_VERSION != 0 && NC_LANG_CPP_VERSION != 98
        INTERNAL Str8
        operator""_s8(char const* CString, sz Size)
        {
            Str8 Result = Str((u8*) CString, (u64) Size);

            return Result;
        }

        INTERNAL Str16
        operator""_s16(char const* CString, sz Size)
        {
            Str16 Result = {};
            u16* String = (u16*) CString;

            Size = StrLen(String);
            Result.Size = Size;
            Result.Str = String;

            return Result;
        }

        INTERNAL Str32
        operator""_s32(char const* CString, sz Size)
        {
            Str32 Result = {};
            u32* String = (u32*) CString;

            Size = StrLen(String);
            Result.Size = Size;
            Result.Str = String;

            return Result;
        }
    #endif


    INTERNAL b32 
    operator<(Str8 StringA, Str8 StringB)
    {
        b32 Result = FALSE;
        u64 CommonSize = MIN(StringA.Size, StringB.Size);

        for (u64 Offset = 0; Offset < CommonSize; ++Offset) {
            if (StringA.Str[Offset] < StringB.Str[Offset]) {
                Result = TRUE;
                break;
            } else if (StringA.Str[Offset] > StringB.Str[Offset]) {
                Result = FALSE;
                break;
            } else if (Offset + 1 == CommonSize) {
                Result = (StringA.Size < StringB.Size);
            }
        }

        return Result;
    }

    INTERNAL b32
    operator==(Str8 StringA, Str8 StringB)
    {
        if (StringA.Size != StringB.Size)
            return FALSE;

        if (StringA.Str == StringB.Str)
            return TRUE;

        return MemCmp(StringA.Str, StringB.Str, StringA.Size);
    }

    INTERNAL b32
    operator!=(Str8 StringA, Str8 StringB)
    {
        return !(StringA == StringB);
    }

    INTERNAL b32 
    operator==(Str16 StringA, Str16 StringB)
    {
        if (StringA.Size != StringB.Size)
            return FALSE;

        if (StringA.Str == StringB.Str)
            return TRUE;

        return MemCmp(StringA.Str, StringB.Str, StringA.Size * 2);
    }

    INTERNAL b32 
    operator!=(Str16 StringA, Str16 StringB)
    {
        return !(StringA == StringB);
    }

    INTERNAL b32 
    operator==(Str32 StringA, Str32 StringB)
    {
        if (StringA.Size != StringB.Size)
            return FALSE;

        if (StringA.Str == StringB.Str)
            return TRUE;

        return MemCmp(StringA.Str, StringB.Str, StringA.Size * 4);
    }

    INTERNAL b32 
    operator!=(Str32 StringA, Str32 StringB)
    {
        return !(StringA == StringB);
    }
#endif

INTERNAL Str8Node*
StrListPushNode(Str8List* List, Str8Node* Node)
{
    SLL_QUEUE_PUSH(List->Head, List->Tail, Node);
    ++List->Count;
    List->TotalSize += Node->String.Size;

    return Node;
}

INTERNAL Str8Node*
StrListPushNodeSetStr(Str8List* List, Str8Node* Node, Str8 String)
{
    SLL_QUEUE_PUSH(List->Head, List->Tail, Node);
    ++List->Count;
    List->TotalSize += String.Size;
    Node->String = String;

    return Node;
}

INTERNAL Str8Node*
StrListPushNodeFront(Str8List* List, Str8Node* Node)
{
    SLL_QUEUE_PUSH_FRONT(List->Head, List->Tail, Node);
    ++List->Count;
    List->TotalSize += Node->String.Size;

    return Node;
}

INTERNAL Str8Node*
StrListPushNodeFrontSetStr(Str8List* List, Str8Node* Node, Str8 String)
{
    SLL_QUEUE_PUSH_FRONT(List->Head, List->Tail, Node);
    ++List->Count;
    List->TotalSize += String.Size;
    Node->String = String;

    return Node;
}

INTERNAL u32
StrGNUHash(Str8 String)
{
    u32 Result = 0x1505;

    if (String.Size) {
        for (u8 C = *String.Str; C; C = *++String.Str)
            Result = Result * 0x21 + C;
    }

    return Result & U32_MAX;
}

global readonly __StringPathStyleMapNode STRING_PATH_STYLE_MAP[5] = {
    {
        Str8Lit(""),
        PATH_STYLE_NULL
    },
    {
        Str8Lit("relative"),
        PATH_STYLE_RELATIVE
    },
    {
        Str8Lit("windows"),
        PATH_STYLE_WIN_ABSOLUTE
    },
    {
        Str8Lit("unix"),
        PATH_STYLE_UNIX_ABSOLUTE
    },
    {
        Str8Lit("system"),
        PATH_STYLE_SYSTEM
    }
};

global readonly u8 INTEGER_SYMBOLS[16] = {
    '0', '1', '2', '3', 
    '4', '5', '6', '7', 
    '8', '9', 'A', 'B', 
    'C', 'D', 'E', 'F'
};

global readonly u8 INTEGER_SYMBOL_REVERSE[128] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 
    0x08, 0x09, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0xFF, 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

global readonly u8 BASE64[64] = {
    '0', '1', '2', '3', '4', '5', 
    '6', '7', '8', '9', 'a', 'b', 
    'c', 'd', 'e', 'f', 'g', 'h', 
    'i', 'j', 'k', 'l', 'm', 'n', 
    'o', 'p', 'q', 'r', 's', 't', 
    'u', 'v', 'w', 'x', 'y', 'z',
    'A', 'B', 'C', 'D', 'E', 'F', 
    'G', 'H', 'I', 'J', 'K', 'L', 
    'M', 'N', 'O', 'P', 'Q', 'R', 
    'S', 'T', 'U', 'V', 'W', 'X', 
    'Y', 'Z', '_', '$'
};

global readonly u8 BASE64_REVERSE[128] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0x3F, 0xFF, 0xFF, 0xFF, 
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 
    0x08, 0x09, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00,
    0xFF, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 
    0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32,
    0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 
    0x3B, 0x3C, 0x3D, 0xFF, 0xFF, 0xFF, 0xFF, 0x3E,
    0xFF, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, 0x10, 
    0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
    0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 
    0x21, 0x22, 0x23, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

#if defined(NC_CC_MSVC) && !defined(NC_CC_CLANG)
    readonly const u8 UTF8_CLASS[32] = {
        1, 1, 1, 1,
        1, 1, 1, 1,
        1, 1, 1, 1,
        1, 1, 1, 1,
        0, 0, 0, 0,
        0, 0, 0, 0,
        2, 2, 2, 2,
        3, 3, 4, 5
    };
#else
    // NOTE(nathan): clang and gcc require homogenous types in sections??
    #if defined(NC_LANG_CPP)
        constexpr u8 UTF8_CLASS[32] = {
            1, 1, 1, 1,
            1, 1, 1, 1,
            1, 1, 1, 1,
            1, 1, 1, 1,
            0, 0, 0, 0,
            0, 0, 0, 0,
            2, 2, 2, 2,
            3, 3, 4, 5
        };
    #elif defined(NC_LANG_C)
        const u8 UTF8_CLASS[32] = {
            1, 1, 1, 1,
            1, 1, 1, 1,
            1, 1, 1, 1,
            1, 1, 1, 1,
            0, 0, 0, 0,
            0, 0, 0, 0,
            2, 2, 2, 2,
            3, 3, 4, 5
        };
    #endif
#endif


API_INTERNAL u64
Hash(u64 Seed, Str8 String)
{
    u64 Result = XXH3_64bits_withSeed(String.Str, String.Size, Seed);

    return Result;
}

API_INTERNAL u64
Hash(Str8 String)
{
    u64 Result = Hash(5381, String);

    return Result;
}

API_INTERNAL u128
BigHash(u64 Seed, Str8 String)
{
    u128 Result = {};
    XXH128_hash_t ResultHash = XXH3_128bits_withSeed(String.Str, String.Size, Seed);

    MemCpy(&Result, &ResultHash, sizeof(Result));

    return Result;
}

API_INTERNAL u128 
BigHash(Str8 String)
{
    u128 Result = BigHash(5381, String);

    return Result;
}

API_INTERNAL u64
StrLen(u8* CString)
{
#if defined(NC_CC_CLANG) || defined(NC_CC_GNU)
    return __builtin_strlen((char const*) CString);
#else
    u8* CStringPtr = CString;

    for (; *CStringPtr; ++CStringPtr);

    return INT_FROM_PTR(CStringPtr - CString);
#endif
}

API_INTERNAL u64
StrLen(char* CString)
{
#if defined(NC_CC_CLANG) || defined(NC_CC_GNU)
    return __builtin_strlen(CString);
#else
    char* CStringPtr = CString;

    for (; *CStringPtr; ++CStringPtr);

    return INT_FROM_PTR(CStringPtr - CString);
#endif
}

API_INTERNAL u64 
StrLen(u16* CString)
{
    u16* CStringPtr = CString;

    for (; *CStringPtr; ++CStringPtr);

    return INT_FROM_PTR(CStringPtr - CString);
}

API_INTERNAL u64 
StrLen(u32* CString)
{
    u32* CStringPtr = CString;

    for (; *CStringPtr; ++CStringPtr);

    return INT_FROM_PTR(CStringPtr - CString);
}

API_INTERNAL Str8
Str(u8* String, u64 StringCount)
{
    Str8 Result = {};

    Result.Str = String;
    Result.Size = StringCount;

    return Result;
}

API_INTERNAL Str8 
Str(char* String)
{
    Str8 Result = {};

    Result.Str = (u8*) String;
    Result.Size = StrLen(String);

    return Result;
}

API_INTERNAL Str8 
Str(u8* StringStart, u8* StringEnd)
{
    Str8 Result = {};

    if (StringEnd < StringStart)
        SWAP(u8*, StringStart, StringEnd);

    Result.Str = StringStart;
    Result.Size = (StringEnd - StringStart);

    return Result;
}

API_INTERNAL Str16 
Str(u16* String, u64 StringCount)
{
    Str16 Result = {};

    Result.Str = String;
    Result.Size = StringCount;

    return Result;
}

API_INTERNAL Str16 
Str(u16* String)
{
    return CStr16(String);
}

API_INTERNAL Str32 
Str(u32* String, u64 StringCount)
{
    Str32 Result = {};

    Result.Str = String;
    Result.Size = StringCount;

    return Result;
}

API_INTERNAL Str32 
Str(u32* String)
{
    return CStr32(String);
}

API_INTERNAL MQTTStr8
MQTTStr(u8 const* PacketData)
{
    MQTTStr8 Result = {};

    if (PacketData) {
        u16 ResultSize = (((u16) PacketData[0] << 8) | (u16) PacketData[1]);

        Result.Size = ResultSize;
        Result.Str = (u8*) (PacketData + 2);
    }

    return Result;
}

API_INTERNAL MQTTStr8Raw
MQTTStrRaw(u8 const* PacketData)
{
    MQTTStr8Raw Result = {};

    if (PacketData) {
        u16 ResultSize = (((u16) PacketData[0] << 8) | (u16) PacketData[1]);

        Result.Size = ResultSize;
        Result.Str = (u8*) PacketData;
    }

    return Result;
}

API_INTERNAL Str8
StrTrimLastSlash(Str8 String)
{
    if (String.Size > 0) {
        u8 LastChar = String.Str[String.Size - 1];

        if (IsSlash(LastChar))
            String.Size--;
    }

    return String;
}

API_INTERNAL Str8
ArenaPushStrCpy(Arena* MemPool, Str8 String)
{
    Str8 Result = ""_s8;

    if (!String.Size || !String.Str)
        return Result;

    Result.Size = String.Size;
    Result.Str = ArenaPushArray(MemPool, u8, Result.Size + 1);
    MemCpy(Result.Str, String.Str, String.Size);
    Result.Str[Result.Size] = 0;

    return Result;
}

API_INTERNAL Str8
ArenaPushStrCat(Arena* MemPool, Str8 StringA, Str8 StringB)
{
    Str8 Result = ""_s8;

    Result.Size = StringA.Size + StringB.Size;
    Result.Str = ArenaPushArray(MemPool, u8, Result.Size + 1);
    MemCpy(Result.Str, StringA.Str, StringA.Size);
    MemCpy(Result.Str + StringA.Size, StringB.Str, StringB.Size);
    Result.Str[Result.Size] = 0;

    return Result;
}

API_INTERNAL Str8Node*
ArenaPushStrList(Arena* MemPool, Str8List* List, Str8 String)
{
    Str8Node* Node = ArenaPushStruct(MemPool, Str8Node);

    SLL_QUEUE_PUSH(List->Head, List->Tail, Node);
    List->Count++;
    List->TotalSize += String.Size;
    Node->String = String;

    return Node;
}

API_INTERNAL Str8
StrListJoin(Arena* MemPool, Str8List* List, Str8JoinPart const* JoinParts)
{
    Str8JoinPart Joins = {};

    if (JoinParts)
        MemCpy(&Joins, JoinParts, sizeof(Str8JoinPart));

    u64 SepCount = 0;

    if (List->Count)
        SepCount = List->Count - 1;

    Str8 Result = ""_s8;

    Result.Size = (
        Joins.Pre.Size + 
        Joins.Post.Size + 
        (
            SepCount * Joins.Sep.Size
        ) + 
        List->TotalSize
    );

    u8* Ptr = Result.Str = ArenaPushArray(MemPool, u8, Result.Size + 1);

    MemCpy(Ptr, Joins.Pre.Str, Joins.Pre.Size);
    Ptr += Joins.Pre.Size;

    for (Str8Node* Node = List->Head; Node; Node = Node->Next) {
        MemCpy(Ptr, Node->String.Str, Node->String.Size);
        Ptr += Node->String.Size;

        if (Node->Next) {
            MemCpy(Ptr, Joins.Sep.Str, Joins.Sep.Size);
            Ptr += Joins.Sep.Size;
        }
    }

    MemCpy(Ptr, Joins.Post.Str, Joins.Post.Size);
    Ptr += Joins.Post.Size;
    *Ptr = 0;

    return Result;
}

API_INTERNAL Str8List 
StrListCopy(Arena* MemPool, Str8List* List)
{
    Str8List Result = {};

    for (Str8Node* Node = List->Head; Node; Node = Node->Next) {
        Str8Node* NewNode = ArenaPushArray(MemPool, Str8Node, 1);
        Str8 NewString = ArenaPushStrCpy(MemPool, Node->String);

        StrListPushNodeSetStr(&Result, NewNode, NewString);
    }

    return Result;
}

API_INTERNAL Str8 
StrRange(u8* First, u8 const* End)
{
    Str8 Result = ""_s8;

    Result.Str = First;
    Result.Size = (u64) (End - First);

    return Result;
}

API_INTERNAL Str8 
StrSub(Str8 String, r1u64 Range)
{
    u64 Min = Range.Min;
    u64 Max = Range.Max;

    if (Max > String.Size)
        Max = String.Size;

    if (Min > String.Size)
        Min = String.Size;

    if (Min > Max)
        SWAP(u64, Min, Max);

    String.Size = Max - Min;
    String.Str += Min;

    return String;
}

API_INTERNAL Str8 
StrSkip(Str8 String, u64 Min)
{
    return StrSub(String, Rng(Min, String.Size));
}

API_INTERNAL Str8 
StrChop(Str8 String, u64 NMax)
{
    return StrSub(String, Rng((u64) 0, (String.Size - NMax)));
}

API_INTERNAL Str8 
StrChopLastSlash(Str8 String)
{
    if (String.Size) {
        u8* Ptr = String.Str + String.Size - 1;

        for (; Ptr >= String.Str; Ptr -= 1) {
            if (*Ptr == '/' || *Ptr == '\\')
                break;
        }

        if (Ptr >= String.Str)
            String.Size = (u64) (Ptr - String.Str);
        else
            String.Size = 0;
    }

    return String;
}

API_INTERNAL Str8
StrSkipChopEmpty(Str8 String)
{
    u8* Head = String.Str;
    u8* End = String.Str + String.Size;

    for (; Head < End; ++Head)
        if (*Head)
            break;

    for (; End > Head; ) {
        --End;

        if (*End) {
            ++End;
            break;
        }
    }

    Str8 Result = StrRange(Head, End);

    return Result;
}

API_INTERNAL Str8 
StrSkipChopWhitespace(Str8 String)
{
    u8* Head = String.Str;
    u8* End = String.Str + String.Size;

    for (; Head < End; ++Head)
        if (!IsSpace(*Head))
            break;

    for (; End > Head; ) {
        --End;

        if (!IsSpace(*End)) {
            ++End;
            break;
        }
    }

    Str8 Result = StrRange(Head, End);

    return Result;
}

API_INTERNAL Str8 
StrPrefix(Str8 String, u64 Size)
{
    return StrSub(String, Rng((u64) 0, Size));
}

API_INTERNAL Str8 
StrSuffix(Str8 String, u64 Size)
{
    return StrSub(String, Rng(String.Size - Size, String.Size));
}

API_INTERNAL Str8 
StrPostfix(Str8 String, u64 Size)
{
    Size = CLAMP_TOP(Size, String.Size);
    String.Str = (String.Str + String.Size) - Size;
    String.Size = Size;

    return String;
}

API_INTERNAL b32
StrMatch(Str8 StringA, Str8 StringB, StrMatchKind Method)
{
    b32 Result = FALSE;

    if (StringA.Size == StringB.Size || Method & STR_MATCH_JUNK_AFTER) {
        Result = TRUE;

        for (u64 I = 0; I < StringA.Size && I < StringB.Size; ++I) {
            b32 Match = (StringA.Str[I] == StringB.Str[I]);

            if (Method & STR_MATCH_ALL_CASES)
                Match |= (ToLower(StringA.Str[I]) == ToLower(StringB.Str[I]));

            if (Method & STR_MATCH_SKIP_SLASH)
                Match |= (ToForwardSlash(StringA.Str[I]) == ToForwardSlash(StringB.Str[I]));

            if (!Match) {
                Result = FALSE;
                break;
            }
        }
    }

    return Result;
}

// TODO: make this clearer, what is this function doing
API_INTERNAL u64
StrFindSubStr(Str8 String, Str8 SubString, u64 Start, StrMatchKind Method)
{
    u64 FoundIndex = String.Size;

    for (u64 I = Start; I < String.Size; ++I) {
        if (I + SubString.Size <= String.Size) {
            Str8 SubStringFromString = StrSub(String, Rng(I, I + SubString.Size));

            if (StrMatch(SubStringFromString, SubString, Method)) {
                FoundIndex = I;

                if (!(Method & STR_MATCH_LAST))
                    break;
            }
        }
    }

    return FoundIndex;
}

API_INTERNAL b32 
StrIsInteger(Str8 String, u32 Base)
{
    b32 Result = FALSE;

    if (String.Size) {
        if (Base > 1 && Base <= 16) {
            Result = TRUE;

            for (u64 I = 0; I < String.Size; ++I) {
                u8 C = String.Str[I];

                if (!(C < 0x80) || INTEGER_SYMBOL_REVERSE[C] >= Base) {
                    Result = FALSE;
                    break;
                }
            }
        }
    }

    return Result;
}

API_INTERNAL i8
SignFromStr(Str8 String, Str8* StringTail)
{
    u64 NegativeCount = 0;
    u64 I = 0;

    for (; I < String.Size; ++I) {
        if (String.Str[I] == '-')
            ++NegativeCount;
        else if (String.Str[I] != '+')
            break;
    }

    *StringTail = StrSkip(String, I);

    i8 Result = 1;

    if (NegativeCount & 1)
        Result = -1;

    return Result;
}

API_INTERNAL u128 
U128FromStr(Str8 String, u32 Base)
{
    u128 Result = {};
    u64 StartIndex = 0;
    
    if (
        Base == 16 &&
        String.Size > 2 &&
        String.Str[0] == '0' &&
        (String.Str[1] == 'x' || String.Str[1] == 'X')
    ) {
        StartIndex = 2;
    }
    
    if (Base > 1 && Base <= 16) {
        for (u64 I = StartIndex; I < String.Size; ++I) {
            u8 C = String.Str[I];

            if (C >= 0x80 || INTEGER_SYMBOL_REVERSE[C] >= Base)
                break;
            
            u64 Carry = Result.U64[0] * Base;

            Result.U64[1] = Result.U64[1] * Base + (Carry < Result.U64[0] * Base ? 0 : 0);
            
            u64 HiPrev = Result.U64[1];
            u64 LoPrev = Result.U64[0];
            
            Result.U64[1] = HiPrev * Base + (LoPrev >> (64 - 4));
            Result.U64[0] = LoPrev * Base;
            
            u64 Digit = INTEGER_SYMBOL_REVERSE[C & 0x7F];

            Result.U64[0] += Digit;
            if (Result.U64[0] < Digit)
                ++Result.U64[1];
        }
    }
    
    return Result;
}

API_INTERNAL u64 
U64FromStr(Str8 String, u32 Base)
{
    u64 Result = 0;
    u64 StartIndex = 0;

    if (
        Base == 16 &&
        String.Size > 2 &&
        String.Str[0] == '0' &&
        (
            String.Str[1] == 'x' ||
            String.Str[1] == 'X'
        )
    ) {
        StartIndex = 2;
    }

    if (Base > 1 && Base <= 16) {
        for (u64 I = StartIndex; I < String.Size; ++I) {
            u8 C = String.Str[I];

            if (C >= 0x80 || INTEGER_SYMBOL_REVERSE[C] >= Base)
                break;

            Result *= Base;
            Result += INTEGER_SYMBOL_REVERSE[C & 0x7F];
        }
    }

    return Result;
}

API_INTERNAL i64 
I64FromStr(Str8 String, u32 Base)
{
    i8 Sign = SignFromStr(String, &String);
    i64 Result = (i64) U64FromStr(String, Base) * Sign;

    return Result;
}

API_INTERNAL u32 
U32FromStr(Str8 String, u32 Base)
{
    u64 Result64 = U64FromStr(String, Base);
    u32 Result = SafeTruncateU64(Result64);

    return Result;
}

API_INTERNAL i32 
I32FromStr(Str8 String, u32 Base)
{
    i64 Result64 = I64FromStr(String, Base);
    i32 Result = SafeTruncateI64(Result64);

    return Result;
}

API_INTERNAL f64 
F64FromStr(Str8 String)
{
    f64 Result = 0.0;

    if (!String.Size || !String.Str)
        return Result;

    u8* Ptr = String.Str;
    u8* End = String.Str + String.Size;

    while (Ptr < End && IsSpace(*Ptr))
        ++Ptr;

    if (Ptr >= End)
        return Result;

    f64 Sign = 1.0;

    if (*Ptr == '-') {
        Sign = -1.0;
        ++Ptr;
    } else if (*Ptr == '+') {
        ++Ptr;
    }

    f64 Value = 0.0;

    while (Ptr < End && IsDigit(*Ptr)) {
        Value = Value * 10.0 + (*Ptr - '0');
        ++Ptr;
    }

    if (Ptr < End && *Ptr == '.') {
        ++Ptr;

        f64 Power10 = 10.0;

        while (Ptr < End && IsDigit(*Ptr)) {
            Value += (*Ptr - '0') / Power10;
            Power10 *= 10.0;
            ++Ptr;
        }
    }

    f64 Scale = 1.0;
    b32 ExpNegative = FALSE;

    if (Ptr < End && (*Ptr == 'e' || *Ptr == 'E')) {
        ++Ptr;

        if (Ptr < End && *Ptr == '-') {
            ExpNegative = TRUE;
            ++Ptr;
        } else if (Ptr < End && *Ptr == '+') {
            ++Ptr;
        }

        u32 Exp = 0;

        while (Ptr < End && IsDigit(*Ptr)) {
            Exp = Exp * 10 + (*Ptr - '0');
            ++Ptr;
        }

        Exp = MIN(Exp, 308);

        while (Exp >= 50) {
            Scale *= 1e50;
            Exp -= 50;
        }

        while (Exp >= 8) {
            Scale *= 1e8;
            Exp -= 8;
        }

        while (Exp) {
            Scale *= 10.0;
            Exp -= 1;
        }
    }

    Result = Sign * (ExpNegative ? (Value / Scale) : (Value * Scale));

    return Result;
}

API_INTERNAL UnicodeDecode
UTF8DecodeMQTT(u8 const* PacketData, u64 PacketSize)
{
    UnicodeDecode Result = {};

    if (PacketSize < 2)                                                         // NOTE: need at least 16 bits for size prefix
        return Result;

    u16 StringLength = (PacketData[0] << 8) | PacketData[1];                    // NOTE: size prefix is BE

    if (!StringLength || StringLength > PacketSize - 1) {
        if (!StringLength)
            Result.Inc = 2;                                                     // NOTE: only consumed size prefix

        return Result;
    }

    u8 const* UTF8Data = PacketData + 2;                                        // NOTE: skip size prefix

    Result = UTF8Decode(UTF8Data, StringLength);

    if (Result.Inc > 0)
        Result.Inc += 2;

    return Result;
}

API_INTERNAL UnicodeDecode 
UTF8Decode(u8 const* String, u64 Max)
{
    UnicodeDecode Result = {};

    Result.Inc = 1;
    Result.CodePoint = U32_MAX;

    u8 Byte = *String;
    u8 ByteClass = UTF8_CLASS[Byte >> 3];

    switch (ByteClass) {
        case 1: {
            Result.CodePoint = Byte;
        } break;

        case 2: {
            if (Max > 1) {
                u8 ContinuationByte = String[1];

                if (!UTF8_CLASS[ContinuationByte >> 3]) {
                    Result.CodePoint = (Byte & 0x0000001F) << 6;
                    Result.CodePoint |= (ContinuationByte & 0x0000003F);
                    Result.Inc = 2;
                }
            }
        } break;

        case 3: {
            if (Max > 2) {
                u8 ContinuationByte[2] = {
                    String[1],
                    String[2]
                };

                if (!UTF8_CLASS[ContinuationByte[0] >> 3] && !UTF8_CLASS[ContinuationByte[1] >> 3]) {
                    Result.CodePoint = (Byte & 0x0000000F) << 12;
                    Result.CodePoint |= ((ContinuationByte[0] & 0x0000003F) << 6);
                    Result.CodePoint |= (ContinuationByte[1] & 0x0000003F);
                    Result.Inc = 3;
                }
            }
        } break;

        case 4: {
            u8 ContinuationByte[3] = {
                String[1],
                String[2],
                String[3]
            };

            if (
                !UTF8_CLASS[ContinuationByte[0] >> 3] &&
                !UTF8_CLASS[ContinuationByte[1] >> 3] &&
                !UTF8_CLASS[ContinuationByte[2] >> 3]
            ) {
                Result.CodePoint = (Byte & 0x00000007) << 18;
                Result.CodePoint |= ((ContinuationByte[0] & 0x0000003F) << 12);
                Result.CodePoint |= ((ContinuationByte[1] & 0x0000003F) << 6);
                Result.CodePoint |= (ContinuationByte[2] & 0x0000003F);
                Result.Inc = 4;
            }
        } break;
    }

    return Result;
}

API_INTERNAL UnicodeDecode 
UTF16Decode(u16 const* String, u64 Max)
{
    UnicodeDecode Result = {};

    Result.Inc = 1;
    Result.CodePoint = *String;

    if (
        Max > 1 && 
        String[0] >= 0xD800 && 
        String[0] < 0xDC00 && 
        String[1] >= 0xDC00 && 
        String[1] < 0xE000
    ) {
        Result.CodePoint = ((String[0] - 0xD800) << 10) | ((String[1] - 0xDC00) + 0x10000);
        Result.Inc = 2;
    }

    return Result;
}

API_INTERNAL u32 
UTF8Encode(u8* String, u32 CodePoint)
{
    u32 Inc = 0;

    if (CodePoint <= 0x7F) {
        String[0] = (u8) CodePoint;
        Inc = 1;
    } else if (CodePoint <= 0x7FF) {
        String[0] = (0x00000003 << 6) | ((CodePoint >> 6) & 0x0000001F);
        String[1] = (1 << 7) | (CodePoint & 0x0000003F);
        Inc = 2;
    } else if (CodePoint <= 0xFFFF) {
        String[0] = (0x00000007 << 5) | ((CodePoint >> 12) & 0x0000000F);
        String[1] = (1 << 7) | ((CodePoint >> 6) & 0x0000003F);
        String[2] = (1 << 7) | (CodePoint & 0x0000003F);
        Inc = 3;
    } else if (CodePoint <= 0x10FFFF) {
        String[0] = (0x0000000F << 4) | ((CodePoint >> 18) & 0x00000007);
        String[1] = (1 << 7) | ((CodePoint >> 12) & 0x0000003F);
        String[2] = (1 << 7) | ((CodePoint >> 6) & 0x0000003F);
        String[3] = (1 << 7) | (CodePoint & 0x0000003F);
        Inc = 4;
    } else {
        String[0] = '?';
        Inc = 1;
    }

    return Inc;
}

API_INTERNAL u32 
UTF16Encode(u16* String, u32 CodePoint)
{
    u32 Inc = 1;

    if (CodePoint == U32_MAX) {
        String[0] = (u16) '?';
    } else if (CodePoint < 0x10000) {
        String[0] = (u16) CodePoint;
    } else {
        u32 V = CodePoint - 0x10000;
        
        String[0] = SafeTruncateU32(0xD800 + (V >> 10));
        String[1] = SafeTruncateU32(0xDC00 + (V & 0x000003FF));
        Inc = 2;
    }

    return Inc;
}

API_INTERNAL Str8 
Str8FromStr16(Arena* MemPool, Str16 String)
{
    Str8 Result = ""_s8;

    if (String.Size) {
        u64 Capacity = String.Size * 3;
        u8* NewString = ArenaPushArray(MemPool, u8, Capacity + 1);
        u16* StringPtr = String.Str;
        u16 const* EndPtr = StringPtr + String.Size;
        u64 Size = 0;
        UnicodeDecode Consume = {};

        for (; StringPtr < EndPtr; StringPtr += Consume.Inc) {
            Consume = UTF16Decode(StringPtr, EndPtr - StringPtr);
            Size += UTF8Encode(NewString + Size, Consume.CodePoint);
        }

        NewString[Size] = 0;
        ArenaPop(MemPool, (Capacity - Size));
        Result = Str(NewString, Size);
    }

    return Result;
}

API_INTERNAL Str16 
Str16FromStr8(Arena* MemPool, Str8 String)
{
    Str16 Result = ""_s16;

    if (String.Size) {
        u64 Capacity = String.Size * 2;
        u16* NewString = ArenaPushArray(MemPool, u16, Capacity + 1);
        u8* StringPtr = String.Str;
        u8 const* EndPtr = StringPtr + String.Size;
        u64 Size = 0;
        UnicodeDecode Consume = {};

        for (; StringPtr < EndPtr; StringPtr += Consume.Inc) {
            Consume = UTF8Decode(StringPtr, EndPtr - StringPtr);
            Size += UTF16Encode(NewString + Size, Consume.CodePoint);
        }

        NewString[Size] = 0;
        ArenaPop(MemPool, (Capacity - Size) * 2);
        Result = Str(NewString, Size);
    }

    return Result;
}

API_INTERNAL Str8 
Str8FromStr32(Arena* MemPool, Str32 String)
{
    Str8 Result = ""_s8;

    if (String.Size) {
        u64 Capacity = String.Size * 4;
        u8* NewString = ArenaPushArray(MemPool, u8, Capacity + 1);
        u32* StringPtr = String.Str;
        u32 const* EndPtr = StringPtr + String.Size;
        u64 Size = 0;

        for (; StringPtr < EndPtr; ++StringPtr)
            Size += UTF8Encode(NewString + Size, *StringPtr);

        NewString[Size] = 0;
        ArenaPop(MemPool, (Capacity - Size));
        Result = Str(NewString, Size);
    }

    return Result;
}

API_INTERNAL Str32 
Str32FromStr8(Arena* MemPool, Str8 String)
{
    Str32 Result = {};

    if (String.Size) {
        u64 Capacity = String.Size;
        u32* NewString = ArenaPushArray(MemPool, u32, Capacity + 1);
        u8* StringPtr = String.Str;
        u8 const* EndPtr = StringPtr + String.Size;
        u64 Size = 0;
        UnicodeDecode Consume = {};

        for (; StringPtr < EndPtr; StringPtr += Consume.Inc) {
            Consume = UTF8Decode(StringPtr, EndPtr - StringPtr);
            NewString[Size++] = Consume.CodePoint;
        }

        NewString[Size] = 0;
        ArenaPop(MemPool, (Capacity - Size) * 4);
        Result = Str(NewString, Size);
    }

    return Result;
}

API_INTERNAL MQTTStr8
MQTTStrFromMQTTStrRaw(Arena* MemPool, MQTTStr8Raw RawString)
{
    MQTTStr8 Result = {};

    if (RawString.Size) {
        Result.Size = RawString.Size;
        Result.Str = ArenaPushArray(MemPool, u8, Result.Size);
        MemCpy(Result.Str, RawString.Str + 2, Result.Size);
    }

    return Result;
}

API_INTERNAL MQTTStr8Raw
MQTTStrRawFromMQTTStr(Arena* MemPool, MQTTStr8 String)
{
    MQTTStr8Raw Result = {};

    if (String.Size) {
        Result.Size = SafeTruncateU32(SafeTruncateU64(String.Size));                // NOTE: max size is U16_MAX
        Result.Str = ArenaPushArray(MemPool, u8, Result.Size + 2);
        *(u16*) Result.Str = SwapByteOrder(Result.Size);
        MemCpy(Result.Str + 2, String.Str, Result.Size);
    } else {
        Result.Size = 0;
        Result.Str = ArenaPushArrayZero(MemPool, u8, 2);
    }

    return Result;
}

API_INTERNAL MQTTStr8Raw
MQTTStrRawFromStr8(Arena* MemPool, Str8 String)
{
    u16 ResultSize = SafeTruncateU32(                                           // NOTE: max size is U16_MAX
        SafeTruncateU64(String.Size)
    );
    MQTTStr8Raw Result = {};

    if (ResultSize) {
        Result.Size = ResultSize;
        Result.Str = ArenaPushArray(MemPool, u8, Result.Size + 2);
        *(u16*) Result.Str = SwapByteOrder(ResultSize);
        MemCpy(Result.Str + 2, String.Str, Result.Size);
    } else {
        Result.Size = 0;
        Result.Str = ArenaPushArrayZero(MemPool, u8, 2);
    }

    return Result;
}

#if NC_SIMD_LEVEL >= 2
    #include "nc_simd.h"

    INTERNAL char const*
    FindChar(char const* CString, u8 Char)
    {
        v128 Target = _mm_set1_epi8(Char);
        char const* CStringPtr = CString;

        for (; (uptr) CStringPtr & 0xF; ++CStringPtr)
            if (*CStringPtr == Char || !*CStringPtr)
                return CStringPtr;

        for ( ; ; CStringPtr += NC_SIMD_ALIGN128) {
            v128 Chunk = _mm_load_si128((v128 const*) CStringPtr);
            v128 Cmp = _mm_cmpeq_epi8(Chunk, Target);
            int Mask = _mm_movemask_epi8(Cmp);
            v128 NullCmp = _mm_cmpeq_epi8(Chunk, _mm_setzero_si128());
            int NullMask = _mm_movemask_epi8(NullCmp);

            Mask |= NullMask;

            if (Mask)
                return CStringPtr + Ctz32(Mask);
        }
    }
#else
    INTERNAL char const*
    FindChar(char const* CString, u8 Char)
    {
        char const* Result = CString;

        while (*Result) {
            if (*Result == Char)
                break;

            ++Result;
        }

        return Result;
    }
#endif

internal force_inline char*
U64ToCStr(u64 Val, char* BufferEnd)
{
    char* BufferEndPtr = BufferEnd;

    *(--BufferEndPtr) = 0;

    if (!Val) {
        *(--BufferEndPtr) = '0';

        return BufferEndPtr;
    }

    while (Val) {
        *(--BufferEndPtr) = '0' + (Val % 10);
        Val /= 10;
    }

    return BufferEndPtr;
}

internal force_inline wchar*
U64ToCWStr(u64 Val, wchar* BufferEnd)
{
    wchar* BufferEndPtr = BufferEnd;

    *(--BufferEndPtr) = 0;

    if (!Val) {
        *(--BufferEndPtr) = L'0';

        return BufferEndPtr;
    }

    while (Val) {
        *(--BufferEndPtr) = L'0' + (Val % 10);
        Val /= 10;
    }

    return BufferEndPtr;
}

#define __VSN_WRITE_CHAR(X) do {                        \
    if ((u64) (BufferPtr - Buffer) < BufferCapacity) {  \
        *BufferPtr = X;                                 \
    }                                                   \
    ++BufferPtr;                                        \
    ++Result;                                           \
} while (FALSE)

#define __VSN_WRITE_STR(X, Y) do {                                                                        \
    u64 BufferAvailable = !(BufferPtr > Buffer + BufferCapacity) * (Buffer + BufferCapacity - BufferPtr); \
    u64 BufferToWrite = MIN(Y, BufferAvailable);                                                          \
    if (BufferToWrite)                                                                                    \
        MemCpy(BufferPtr, X, BufferToWrite * sizeof(*BufferPtr));                                         \
    BufferPtr += Y;                                                                                       \
    Result += Y;                                                                                          \
} while (FALSE)

internal force_inline i32
__VSNPrintf(char* Buffer, u64 BufferCount, char const* Fmt, va_list Args)
{
    i32 Result = 0;
    char* BufferPtr = Buffer;
    const u64 BufferCapacity = !!Buffer * BufferCount;
    char const* FmtPtr = Fmt;

    while (*FmtPtr) {
        char const* NextSpecifier = FindChar(FmtPtr, '%');

        if (NextSpecifier > FmtPtr)
            __VSN_WRITE_STR(FmtPtr, NextSpecifier - FmtPtr);

        FmtPtr = NextSpecifier;

        if (!*FmtPtr)
            break;

        ++FmtPtr;

        b32 HaveMinus = FALSE;
        b32 HavePlus = FALSE;
        b32 HaveSpace = FALSE;
        b32 HaveHash = FALSE;
        b32 HaveZero = FALSE;

        while (*FmtPtr) {
            const u8 CurrFmt = *FmtPtr;

            if (CurrFmt == '-')
                HaveMinus = TRUE;
            else if (CurrFmt == '+')
                HavePlus = TRUE;
            else if (CurrFmt == ' ')
                HaveSpace = TRUE;
            else if (CurrFmt == '#')
                HaveHash = TRUE;
            else if (CurrFmt == '0')
                HaveZero = TRUE;
            else
                break;

            ++FmtPtr;
        }

        i32 Width = 0;

        if (*FmtPtr == '*') {
            Width = va_arg(Args, int);
            ++FmtPtr;
        } else {
            while (IsDigit(*FmtPtr)) {
                Width = Width * 10 + (*FmtPtr - '0');
                ++FmtPtr;
            }
        }

        i32 Precision = -1;

        if (*FmtPtr == '.') {
            ++FmtPtr;
            Precision = 0;

            if (*FmtPtr == '*') {
                Precision = va_arg(Args, int);
                ++FmtPtr;
            } else {
                while (IsDigit(*FmtPtr)) {
                    Precision = Precision * 10 + (*FmtPtr - '0');
                    ++FmtPtr;
                }
            }
        }

        enum {
            __VSN_LEN_DEFAULT,
            __VSN_LEN_HH,
            __VSN_LEN_H,
            __VSN_LEN_L,
            __VSN_LEN_LL,
            __VSN_LEN_Z,
            __VSN_LEN_T,
        } ArgLen = __VSN_LEN_DEFAULT;

        if (*FmtPtr == 'h') {
            ++FmtPtr;
            ArgLen = (*FmtPtr == 'h') ? (++FmtPtr, __VSN_LEN_HH) : __VSN_LEN_H;
        } else if (*FmtPtr == 'l') {
            ++FmtPtr;
            ArgLen = (*FmtPtr == 'l') ? (++FmtPtr, __VSN_LEN_LL) : __VSN_LEN_L;
        } else if (*FmtPtr == 'z') {
            ++FmtPtr;
            ArgLen = __VSN_LEN_Z;
        } else if (*FmtPtr == 't') {
            ++FmtPtr;
            ArgLen = __VSN_LEN_T;
        }

        switch (*FmtPtr) {
            default: {
                __VSN_WRITE_CHAR('%');
                __VSN_WRITE_CHAR(*FmtPtr);
            } break;

            case 0: {} goto FINISH;

            case '%': {
                __VSN_WRITE_CHAR('%');
            } break;

            case 'c': {
                char Char = (char) va_arg(Args, int);
                __VSN_WRITE_CHAR(Char);
            } break;

            case 's': {
                char const* String = va_arg(Args, char const*);

                if (!String)
                    String = "[NULL]";  // WARN: this might be more trouble than it's worth

                u64 Len = StrLen((char*) String);

                if (Precision >= 0 && (u64) Precision < Len)
                    Len = Precision;

                if (Width > (i32) Len && !HaveMinus)
                    for (i32 Index = 0; Index < Width - (i32) Len; ++Index)
                        __VSN_WRITE_CHAR(' ');

                __VSN_WRITE_STR(String, Len);

                if (Width > (i32) Len && HaveMinus)
                    for (i32 Index = 0; Index < Width - (i32) Len; ++Index)
                        __VSN_WRITE_CHAR(' ');
            } break;

            case 'S': {
                u64 Len = va_arg(Args, int);
                char const* String = va_arg(Args, char const*);

                if (!String) {
                    String = "[NULL]";  // WARN: this might be more trouble than it's worth
                    Len = 6;
                }

                if (Precision >= 0 && (u64) Precision < Len)
                    Len = Precision;

                if (Width > (i32) Len && !HaveMinus)
                    for (i32 Index = 0; Index < Width - (i32) Len; ++Index)
                        __VSN_WRITE_CHAR(' ');

                __VSN_WRITE_STR(String, Len);

                if (Width > (i32) Len && HaveMinus)
                    for (i32 Index = 0; Index < Width - (i32) Len; ++Index)
                        __VSN_WRITE_CHAR(' ');
            } break;

            case 'd':
            case 'i': {
                i64 Val;

                switch (ArgLen) {
                    default: {
                        Val = va_arg(Args, int);
                    } break;

                    case __VSN_LEN_HH: {
                        Val = (i8) va_arg(Args, int);
                    } break;

                    case __VSN_LEN_H: {
                        Val = (i16) va_arg(Args, int);
                    } break;

                    case __VSN_LEN_LL: {
                        Val = va_arg(Args, long long);
                    } break;

                    case __VSN_LEN_L: {
                        Val = va_arg(Args, long);
                    } break;
                }

                char TempBuffer[32];
                char* String = TempBuffer + sizeof(TempBuffer);
                b32 Negative = (Val < 0);
                u64 UnsignedVal = (Negative) ? -(u64) Val : Val;

                String = U64ToCStr(UnsignedVal, String);

                i32 ValLen = (TempBuffer + sizeof(TempBuffer) - 1) - String;
                i32 TotalLen = ValLen + (Negative || HavePlus || HaveSpace);

                if (!HaveMinus && !HaveZero && Width > TotalLen)
                    for (i32 Index = 0; Index < Width - TotalLen; ++Index)
                        __VSN_WRITE_CHAR(' ');

                if (Negative)
                    __VSN_WRITE_CHAR('-');
                else if (HavePlus)
                    __VSN_WRITE_CHAR('+');
                else if (HaveSpace)
                    __VSN_WRITE_CHAR(' ');

                if (!HaveMinus && HaveZero && Width > TotalLen)
                    for (i32 Index = 0; Index < Width - TotalLen; ++Index)
                        __VSN_WRITE_CHAR('0');

                __VSN_WRITE_STR(String, ValLen);

                if (HaveMinus && Width > TotalLen)
                    for (i32 Index = 0; Index < Width - TotalLen; ++Index)
                        __VSN_WRITE_CHAR(' ');
            } break;

            case 'u': {
                u64 Val;

                switch (ArgLen) {
                    default: {
                        Val = va_arg(Args, unsigned int);
                    } break;

                    case __VSN_LEN_HH: {
                        Val = (u8) va_arg(Args, unsigned int);
                    } break;

                    case __VSN_LEN_H: {
                        Val = (u16) va_arg(Args, unsigned int);
                    } break;

                    case __VSN_LEN_L: {
                        Val = (u32) va_arg(Args, unsigned long);
                    } break;

                    case __VSN_LEN_LL: {
                        Val = va_arg(Args, unsigned long long);
                    } break;

                    case __VSN_LEN_Z: {
                        Val = va_arg(Args, unsigned long long);
                    } break;
                }

                char TempBuffer[32];
                char const* String = U64ToCStr(Val, TempBuffer + sizeof(TempBuffer));

                __VSN_WRITE_STR(String, (TempBuffer + sizeof(TempBuffer) - 1) - String);
            } break;

            case 'x':
            case 'X': {
                u64 Val;

                switch (ArgLen) {
                    default: {
                        Val = va_arg(Args, unsigned int);
                    } break;

                    case __VSN_LEN_HH: {
                        Val = (u8) va_arg(Args, int);
                    } break;

                    case __VSN_LEN_H:  {
                        Val = (u16) va_arg(Args, int);
                    } break;

                    case __VSN_LEN_L:  {
                        Val = va_arg(Args, unsigned long);
                    } break;

                    case __VSN_LEN_LL: {
                        Val = va_arg(Args, unsigned long long);
                    } break;

                    case __VSN_LEN_Z:  {
                        Val = va_arg(Args, size_t);
                    } break;
                }

                char TempBuffer[32];
                char* TempBufferPtr = TempBuffer + sizeof(TempBuffer) - 1;

                *TempBufferPtr = 0;

                if (!Val) {
                    *(--TempBufferPtr) = '0';
                } else {
                    char const* HexString = (*FmtPtr == 'x') ? "0123456789abcdef" : "0123456789ABCDEF";

                    while (Val) {
                        *(--TempBufferPtr) = HexString[Val & 0xF];
                        Val >>= 4;
                    }
                }

                if (HaveHash && TempBufferPtr[0] != '0') {
                    __VSN_WRITE_CHAR('0');
                    __VSN_WRITE_CHAR(*FmtPtr);
                }

                __VSN_WRITE_STR(TempBufferPtr, (TempBuffer + sizeof(TempBuffer) - 1) - TempBufferPtr);
            } break;

            case 'p': {
                void* Ptr = va_arg(Args, void*);
                uptr Val = (uptr) Ptr;

                __VSN_WRITE_CHAR('0');
                __VSN_WRITE_CHAR('x');

                char TempBuffer[32];
                char* TempBufferPtr = TempBuffer + sizeof(TempBuffer) - 1;

                *TempBufferPtr = 0;

                local const char* HexCharacters = "0123456789ABCDEF";

                // NOTE: unrolled for 32-bit addresses but should consider this
                //     : only being run on x64 and could remove loop all-together
                for (i32 __I = 0; __I < (i32) (2 * sizeof(void*)); __I += 4) {
                    *(--TempBufferPtr) = HexCharacters[Val & 0xF];
                    Val >>= 4;  // Val /= 16;
                    *(--TempBufferPtr) = HexCharacters[Val & 0xF];
                    Val >>= 4;  // Val /= 16;
                    *(--TempBufferPtr) = HexCharacters[Val & 0xF];
                    Val >>= 4;  // Val /= 16;
                    *(--TempBufferPtr) = HexCharacters[Val & 0xF];
                    Val >>= 4;  // Val /= 16;
                }

                __VSN_WRITE_STR(TempBufferPtr, (TempBuffer + sizeof(TempBuffer) - 1) - TempBufferPtr);
            } break;

            case 'f':
            case 'F': {
                f64 Val = va_arg(Args, f64);

                if (Precision < 0)
                    Precision = 6;

                char TempBuffer[128];
                u64 Bits = *(u64*) &Val;
                b32 IsNegative = (Bits >> 63);
                i32 IntPart = (i32) Val;
                f64 FracPart = AbsF64(Val - IntPart);
                char* Ptr = U64ToCStr(Abs32(IntPart), TempBuffer + 64);

                if (IsNegative)
                    __VSN_WRITE_CHAR('-');

                __VSN_WRITE_STR(Ptr, (TempBuffer + 64 - 1) - Ptr);

                if (Precision > 0) {
                    __VSN_WRITE_CHAR('.');

                    for (i32 Index = 0; Index < Precision; ++Index) {
                        FracPart *= 10;

                        i32 Digit = (i32) FracPart;

                        __VSN_WRITE_CHAR('0' + Digit);
                        FracPart -= Digit;
                    }
                }
            } break;
        }

        ++FmtPtr;
    }

FINISH:
    if (Buffer && BufferCount) {
        u64 FinalPosition = BufferPtr - Buffer;

        if (FinalPosition < BufferCount) {
            Buffer[FinalPosition] = 0;
        } else {
            Buffer[BufferCount - 1] = 0;
        }
    }

    return Result;
}

internal force_inline i32
__VSNPrintfW(wchar* Buffer, u64 BufferCount, wchar const* Fmt, va_list Args)
{
    i32 Result = 0;
    wchar* BufferPtr = Buffer;
    const u64 BufferCapacity = !!Buffer * BufferCount;
    wchar const* FmtPtr = Fmt;

    while (*FmtPtr) {
        wchar const* NextSpecifier = FmtPtr;
        while (*NextSpecifier && *NextSpecifier != L'%')
            ++NextSpecifier;

        if (NextSpecifier > FmtPtr)
            __VSN_WRITE_STR(FmtPtr, NextSpecifier - FmtPtr);

        FmtPtr = NextSpecifier;

        if (!*FmtPtr)
            break;

        ++FmtPtr;

        b32 HaveMinus = FALSE;
        b32 HavePlus = FALSE;
        b32 HaveSpace = FALSE;
        b32 HaveHash = FALSE;
        b32 HaveZero = FALSE;

        while (*FmtPtr) {
            const wchar CurrFmt = *FmtPtr;

            if (CurrFmt == L'-')
                HaveMinus = TRUE;
            else if (CurrFmt == L'+')
                HavePlus = TRUE;
            else if (CurrFmt == L' ')
                HaveSpace = TRUE;
            else if (CurrFmt == L'#')
                HaveHash = TRUE;
            else if (CurrFmt == L'0')
                HaveZero = TRUE;
            else
                break;

            ++FmtPtr;
        }

        i32 Width = 0;

        if (*FmtPtr == L'*') {
            Width = va_arg(Args, int);
            ++FmtPtr;
        } else {
            while (*FmtPtr >= L'0' && *FmtPtr <= L'9') {
                Width = Width * 10 + (*FmtPtr - L'0');
                ++FmtPtr;
            }
        }

        i32 Precision = -1;

        if (*FmtPtr == L'.') {
            ++FmtPtr;
            Precision = 0;

            if (*FmtPtr == L'*') {
                Precision = va_arg(Args, int);
                ++FmtPtr;
            } else {
                while (*FmtPtr >= L'0' && *FmtPtr <= L'9') {
                    Precision = Precision * 10 + (*FmtPtr - L'0');
                    ++FmtPtr;
                }
            }
        }

        enum {
            __VSNW_LEN_DEFAULT,
            __VSNW_LEN_HH,
            __VSNW_LEN_H,
            __VSNW_LEN_L,
            __VSNW_LEN_LL,
            __VSNW_LEN_Z,
            __VSNW_LEN_T,
        } ArgLen = __VSNW_LEN_DEFAULT;

        if (*FmtPtr == L'h') {
            ++FmtPtr;
            ArgLen = (*FmtPtr == L'h') ? (++FmtPtr, __VSNW_LEN_HH) : __VSNW_LEN_H;
        } else if (*FmtPtr == L'l') {
            ++FmtPtr;
            ArgLen = (*FmtPtr == L'l') ? (++FmtPtr, __VSNW_LEN_LL) : __VSNW_LEN_L;
        } else if (*FmtPtr == L'z') {
            ++FmtPtr;
            ArgLen = __VSNW_LEN_Z;
        } else if (*FmtPtr == L't') {
            ++FmtPtr;
            ArgLen = __VSNW_LEN_T;
        }

        switch (*FmtPtr) {
            default: {
                __VSN_WRITE_CHAR(L'%');
                __VSN_WRITE_CHAR(*FmtPtr);
            } break;

            case 0: {} goto FINISH;

            case L'%': {
                __VSN_WRITE_CHAR(L'%');
            } break;

            case L'c': {
                wchar Char = (wchar) va_arg(Args, int);
                __VSN_WRITE_CHAR(Char);
            } break;

            case L's': {
                wchar const* String = va_arg(Args, wchar const*);

                if (!String)
                    String = (wchar const*) L"[NULL]";

                u64 Len = 0;
                while (String[Len])
                    ++Len;

                if (Precision >= 0 && (u64) Precision < Len)
                    Len = Precision;

                if (Width > (i32) Len && !HaveMinus) {
                    for (i32 Index = 0; Index < Width - (i32) Len; ++Index)
                        __VSN_WRITE_CHAR(L' ');
                }

                __VSN_WRITE_STR(String, Len);

                if (Width > (i32) Len && HaveMinus) {
                    for (i32 Index = 0; Index < Width - (i32) Len; ++Index)
                        __VSN_WRITE_CHAR(L' ');
                }
            } break;

            case 'S': {
                u64 Len = va_arg(Args, int);
                wchar const* String = va_arg(Args, wchar const*);

                if (!String) {
                    String = (wchar const*) L"[NULL]";  // WARN: this might be more trouble than it's worth
                    Len = 6;
                }

                if (Precision >= 0 && (u64) Precision < Len)
                    Len = Precision;

                if (Width > (i32) Len && !HaveMinus)
                    for (i32 Index = 0; Index < Width - (i32) Len; ++Index)
                        __VSN_WRITE_CHAR(L' ');

                __VSN_WRITE_STR(String, Len);

                if (Width > (i32) Len && HaveMinus)
                    for (i32 Index = 0; Index < Width - (i32) Len; ++Index)
                        __VSN_WRITE_CHAR(L' ');
            } break;

            case L'd':
            case L'i': {
                i64 Val;

                switch (ArgLen) {
                    default: {
                        Val = va_arg(Args, int);
                    } break;

                    case __VSNW_LEN_HH: {
                        Val = (i8) va_arg(Args, int);
                    } break;

                    case __VSNW_LEN_H: {
                        Val = (i16) va_arg(Args, int);
                    } break;

                    case __VSNW_LEN_LL: {
                        Val = va_arg(Args, long long);
                    } break;

                    case __VSNW_LEN_L: {
                        Val = va_arg(Args, long);
                    } break;
                }

                wchar TempBuffer[32];
                wchar* String = TempBuffer + ARRAY_COUNT(TempBuffer) - 1;

                *String = 0;
                
                b32 Negative = (Val < 0);
                u64 UnsignedVal = (Negative) ? -(u64) Val : Val;

                if (!UnsignedVal)
                    *(--String) = L'0';
                else {
                    while (UnsignedVal) {
                        *(--String) = L'0' + (UnsignedVal % 10);
                        UnsignedVal /= 10;
                    }
                }

                i32 ValLen = (TempBuffer + ARRAY_COUNT(TempBuffer) - 1) - String;
                i32 TotalLen = ValLen + (Negative || HavePlus || HaveSpace);

                if (!HaveMinus && !HaveZero && Width > TotalLen)
                    for (i32 Index = 0; Index < Width - TotalLen; ++Index)
                        __VSN_WRITE_CHAR(L' ');

                if (Negative)
                    __VSN_WRITE_CHAR(L'-');
                else if (HavePlus)
                    __VSN_WRITE_CHAR(L'+');
                else if (HaveSpace)
                    __VSN_WRITE_CHAR(L' ');

                if (!HaveMinus && HaveZero && Width > TotalLen)
                    for (i32 Index = 0; Index < Width - TotalLen; ++Index)
                        __VSN_WRITE_CHAR(L'0');

                __VSN_WRITE_STR(String, ValLen);

                if (HaveMinus && Width > TotalLen)
                    for (i32 Index = 0; Index < Width - TotalLen; ++Index)
                        __VSN_WRITE_CHAR(L' ');
            } break;

            case L'u':
            case L'x':
            case L'X': {
                u64 Val;

                switch (ArgLen) {
                    default: {
                        Val = va_arg(Args, u32);
                    } break;

                    case __VSNW_LEN_HH: {
                        Val = (u8) va_arg(Args, u32);
                    } break;

                    case __VSNW_LEN_H: {
                        Val = (u16) va_arg(Args, u32);
                    } break;

                    case __VSNW_LEN_L: {
                        Val = va_arg(Args, u32);
                    } break;

                    case __VSNW_LEN_LL: {
                        Val = va_arg(Args, u64);
                    } break;

                    case __VSNW_LEN_Z: {
                        Val = va_arg(Args, u64);
                    } break;
                }

                wchar TempBuffer[32];
                wchar* TempBufferPtr = TempBuffer + ARRAY_COUNT(TempBuffer) - 1;
                *TempBufferPtr = 0;

                if (*FmtPtr == L'u') {
                    if (!Val)
                        *(--TempBufferPtr) = L'0';
                    else {
                        while (Val) {
                            *(--TempBufferPtr) = L'0' + (Val % 10);
                            Val /= 10;
                        }
                    }
                } else {
                    if (!Val) {
                        *(--TempBufferPtr) = L'0';
                    } else {
                        wchar const* HexString = (*FmtPtr == L'x') ? (wchar const*) L"0123456789abcdef" : (wchar const*) L"0123456789ABCDEF";

                        while (Val) {
                            *(--TempBufferPtr) = HexString[Val & 0xF];
                            Val >>= 4;
                        }
                    }

                    if (HaveHash && TempBufferPtr[0] != L'0') {
                        __VSN_WRITE_CHAR(L'0');
                        __VSN_WRITE_CHAR(*FmtPtr);
                    }
                }

                __VSN_WRITE_STR(TempBufferPtr, (TempBuffer + ARRAY_COUNT(TempBuffer) - 1) - TempBufferPtr);
            } break;

            case L'p': {
                void* Ptr = va_arg(Args, void*);
                uptr Val = (uptr) Ptr;

                __VSN_WRITE_CHAR(L'0');
                __VSN_WRITE_CHAR(L'x');

                wchar TempBuffer[32];
                wchar* TempBufferPtr = TempBuffer + ARRAY_COUNT(TempBuffer) - 1;
                *TempBufferPtr = 0;

                local wchar const* HexCharacters = (wchar const*) L"0123456789ABCDEF";

                // NOTE: unrolled for 32-bit addresses but should consider this
                //     : only being run on x64 and could remove loop all-together
                for (i32 __I = 0; __I < (i32) (2 * sizeof(void*)); __I += 4) {
                    *(--TempBufferPtr) = HexCharacters[Val & 0xF];
                    Val >>= 4;  // Val /= 16;
                    *(--TempBufferPtr) = HexCharacters[Val & 0xF];
                    Val >>= 4;  // Val /= 16;
                    *(--TempBufferPtr) = HexCharacters[Val & 0xF];
                    Val >>= 4;  // Val /= 16;
                    *(--TempBufferPtr) = HexCharacters[Val & 0xF];
                    Val >>= 4;  // Val /= 16;
                }

                __VSN_WRITE_STR(TempBufferPtr, (TempBuffer + ARRAY_COUNT(TempBuffer) - 1) - TempBufferPtr);
            } break;

            case 'f':
            case 'F': {
                f64 Val = va_arg(Args, f64);

                if (Precision < 0)
                    Precision = 6;

                wchar TempBuffer[128];
                u64 Bits = *(u64*) &Val;
                b32 IsNegative = (Bits >> 63);
                i32 IntPart = (i32) Val;
                f64 FracPart = AbsF64(Val - IntPart);
                wchar* Ptr = U64ToCWStr(Abs32(IntPart), TempBuffer + 64);

                if (IsNegative)
                    __VSN_WRITE_CHAR(L'-');

                __VSN_WRITE_STR(Ptr, (TempBuffer + 64 - 1) - Ptr);

                if (Precision > 0) {
                    __VSN_WRITE_CHAR(L'.');

                    for (i32 Index = 0; Index < Precision; ++Index) {
                        FracPart *= 10;

                        i32 Digit = (i32) FracPart;

                        __VSN_WRITE_CHAR(L'0' + Digit);
                        FracPart -= Digit;
                    }
                }
            } break;
        }

        ++FmtPtr;
    }

FINISH:
    if (Buffer && BufferCount) {
        u64 FinalPosition = BufferPtr - Buffer;
        if (FinalPosition < BufferCount)
            Buffer[FinalPosition] = 0;
        else
            Buffer[BufferCount - 1] = 0;
    }

    return Result;
}

#undef __VSN_WRITE_STR
#undef __VSN_WRITE_CHAR

Str8 
ArenaPushStrFmtV(Arena* MemPool, char const* Fmt, va_list Args)
{
    Str8 Result = ""_s8;
    va_list Args2;

    va_copy(Args2, Args);

    u64 BytesRequired = VSCPrintf(Fmt, Args2) + 1;

    va_end(Args2);
    Result.Str = ArenaPushArray(MemPool, u8, BytesRequired);
    Result.Size = BytesRequired - 1;
    Result.Str[BytesRequired - 1] = 0;
    VSNPrintf((char*) Result.Str, (int) BytesRequired, Fmt, Args);

    return Result;
}

Str8 
ArenaPushStrFmt(Arena* MemPool, char* Fmt, ...)
{
    va_list Args;

    va_start(Args, Fmt);

    Str8 Result = ArenaPushStrFmtV(MemPool, Fmt, Args);

    va_end(Args);

    return Result;
}

API_INTERNAL void
ListPush(Arena *MemPool, Str8List *List, Str8 String)
    
{
    Str8Node* Node = ArenaPushArray(MemPool, Str8Node, 1);

    StrListPushNodeSetStr(List, Node, String);
}

API_INTERNAL void
ListPushAlign(Arena* MemPool, Str8List* List, u64 Alignment)
{
    u64 TotalSizePreAlign = List->TotalSize;
    u64 TotalSizePostAlign = ALIGNTO(TotalSizePreAlign, Alignment);
    u64 RequiredSize = TotalSizePostAlign - TotalSizePreAlign;

    if (RequiredSize) {
        u8* Padding = ArenaPushArrayZero(MemPool, u8, RequiredSize);

        ListPush(MemPool, List, Str(Padding, RequiredSize));
    }
}

Str16
ArenaPushStrFmtV(Arena* MemPool, wchar const* Fmt, va_list Args)
{
    Str16 Result = {};
    va_list Args2;

    va_copy(Args2, Args);

    u64 BytesRequired = VSCPrintfW(Fmt, Args2) + 1;

    va_end(Args2);
    Result.Str = ArenaPushArray(MemPool, wchar, BytesRequired);
    Result.Size = BytesRequired - 1;
    Result.Str[BytesRequired - 1] = 0;
    VSNPrintfW(Result.Str, (int) BytesRequired, Fmt, Args);

    return Result;
}

Str16
ArenaPushStrFmt(Arena* MemPool, wchar* Fmt, ...)
{
    va_list Args;

    va_start(Args, Fmt);

    Str16 Result = ArenaPushStrFmtV(MemPool, Fmt, Args);

    va_end(Args);

    return Result;
}

API_INTERNAL Str8Array 
StrArrayFromList(Arena* MemPool, Str8List* List)
{
    Str8Array Result = {};
    
    Result.Count = List->Count;
    Result.Data = ArenaPushArray(MemPool, Str8, Result.Count);
    
    u64 Index = 0;

    for (Str8Node* Node = List->Head; Node; Node = Node->Next, ++Index)
        Result.Data[Index] = Node->String;

    return Result;
}

API_INTERNAL void 
ListPushFront(Arena* MemPool, Str8List* List, Str8 String)
{
    Str8Node* Node = ArenaPushArrayZero(MemPool, Str8Node, 1);

    Node->String = String;

    if (List->Head)
        Node->Next = List->Head;
    else
        List->Tail = Node;

    List->Head = Node;
    ++List->Count;
    List->TotalSize += String.Size;
}

void 
ListPushFmt(Arena* MemPool, Str8List* List, char* Fmt, ...)
{
    va_list Args;

    va_start(Args, Fmt);

    Str8 String = ArenaPushStrFmtV(MemPool, Fmt, Args);

    va_end(Args);
    ListPush(MemPool, List, String);
}

API_INTERNAL void 
ListPushCat(Str8List* List, Str8List* Items)
{
    if (Items->Head) {
        List->Count += Items->Count;
        List->TotalSize += Items->TotalSize;

        if (!List->Tail) {
            *List = *Items;
        } else {
            List->Tail->Next = Items->Head;
            List->Tail = Items->Tail;
        }
    }

    MemSet(Items, 0, sizeof(Str8List));
}

API_INTERNAL Str8List 
StrSplit(Arena* MemPool, Str8 String, u8 const* Splits, u32 SplitsCount, b32 KeepEmpties)
{
    Str8List Result = {};
    u8* StringPtr = String.Str;
    u8* EndPtr = String.Str + String.Size;

    for (; StringPtr < EndPtr;) {
        u8* Head = StringPtr;

        for (; StringPtr < EndPtr; ++StringPtr) {
            u8 C = *StringPtr;
            b32 IsSplit = FALSE;

            for (u64 I = 0; I < SplitsCount; ++I) {
                if (Splits[I] == C) {
                    IsSplit = TRUE;
                    break;
                }
            }

            if (IsSplit)
                break;
        }

        Str8 Segment = StrRange(Head, StringPtr);

        if (KeepEmpties || Segment.Size)
            ListPush(MemPool, &Result, Segment);

        ++StringPtr;
    }

    return Result;
}

API_INTERNAL Str8 
RawFromEscapedStr8(Arena* MemPool, Str8 String)
{
    TempArena Scratch = GetScratch(&MemPool, 1);
    Str8List Strings = {};
    u64 Start = 0;

    for (u64 Index = 0; Index <= String.Size; ++Index) {
        if (Index == String.Size || String.Str[Index] == '\\' || String.Str[Index] == '\r') {
            Str8 SubString = StrSub(String, Rng(Start, Index));

            if (SubString.Size)
                ListPush(Scratch.MemPool, &Strings, SubString);

            Start = Index + 1;
        }

        if (Index < String.Size && String.Str[Index] == '\\') {
            u8 NextChar = String.Str[Index + 1];
            u8 ReplaceByte = 0;

            switch (NextChar) {
                default: {} break;

                case 'a': {
                    ReplaceByte = 0x07;
                } break;

                case 'b': {
                    ReplaceByte = 0x08;
                } break;

                case 'e': {
                    ReplaceByte = 0x1B;
                } break;

                case 'f': {
                    ReplaceByte = 0x0C;
                } break;

                case 'n': {
                    ReplaceByte = 0x0A;
                } break;

                case 'r': {
                    ReplaceByte = 0x0D;
                } break;

                case 't': {
                    ReplaceByte = 0x09;
                } break;

                case 'v': {
                    ReplaceByte = 0x0B;
                } break;

                case '\\': {
                    ReplaceByte = '\\';
                } break;

                case '\'': {
                    ReplaceByte = '\'';
                } break;

                case '"': {
                    ReplaceByte = '"';
                } break;

                case '?': {
                    ReplaceByte = '?';
                } break;
            }

            Str8 ReplaceString = ArenaPushStrCpy(Scratch.MemPool, Str(&ReplaceByte, 1));

            ListPush(Scratch.MemPool, &Strings, ReplaceString);
            ++Index;
            ++Start;
        }
    }

    Str8 Result = StrListJoin(MemPool, &Strings, NULL);

    ReleaseScratch(Scratch);

    return Result;
}

API_INTERNAL Str8 
EscapedFromRawStr8(Arena* MemPool, Str8 String)
{
    TempArena Scratch = GetScratch(&MemPool, 1);
    Str8List Parts = {};
    u64 StartSplitIndex = 0;

    for (u64 Index = 0; Index <= String.Size; ++Index) {
        u8 Byte = (Index < String.Size) ? String.Str[Index] : 0;
        b32 Split = TRUE;
        Str8 SeparatorReplace = {};

        switch (Byte) {
            default: {
                Split = FALSE;
            } break;

            case 0: {} break;

            case '\a': {
                SeparatorReplace = Str8Lit("\\a");
            } break;

            case '\b': {
                SeparatorReplace = Str8Lit("\\b");
            } break;

            case '\f': {
                SeparatorReplace = Str8Lit("\\f");
            } break;

            case '\n': {
                SeparatorReplace = Str8Lit("\\n");
            } break;

            case '\r': {
                SeparatorReplace = Str8Lit("\\r");
            } break;

            case '\t': {
                SeparatorReplace = Str8Lit("\\t");
            } break;

            case '\v': {
                SeparatorReplace = Str8Lit("\\v");
            } break;

            case '\\': {
                SeparatorReplace = Str8Lit("\\\\");
            } break;

            case '"': {
                SeparatorReplace = Str8Lit("\\\"");
            } break;
        }

        if (Split) {
            Str8 SubStr = StrSub(String, Rng(StartSplitIndex, Index));

            StartSplitIndex = Index + 1;
            ListPush(Scratch.MemPool, &Parts, SubStr);

            if (SeparatorReplace.Size)
                ListPush(Scratch.MemPool, &Parts, SeparatorReplace);
        }
    }

    Str8JoinPart Join = {};
    Str8 Result = StrListJoin(MemPool, &Parts, &Join);
    
    ReleaseScratch(Scratch);

    return Result;
}

API_INTERNAL Str8 
PathAbsoluteDstFromRelativeDstSrc(
    Arena* MemPool, 
    Str8 Dst, 
    Str8 Src
) {
    Str8 Result = Dst;
    PathStyle DstStyle = PathStyleFromStr(Dst);

    if (Dst.Size && DstStyle == PATH_STYLE_RELATIVE) {
        TempArena Scratch = GetScratch(&MemPool, 1);
        Str8 DstFromSrcAbs = ArenaPushStrFmt(Scratch.MemPool, "%S/%S", PRINT_STR(Src), PRINT_STR(Dst));
        Str8List DstFromSrcAbsParts = StrSplitPath(Scratch.MemPool, DstFromSrcAbs);
        PathStyle DstFromSrcAbsStyle = PathStyleFromStr(Src);

        StrPathListResolveDots(&DstFromSrcAbsParts, DstFromSrcAbsStyle);
        Result = StrPathListJoinByStyle(MemPool, &DstFromSrcAbsParts, DstFromSrcAbsStyle);
        ReleaseScratch(Scratch);
    }

    return Result;
}

API_INTERNAL Str8 
PathRelativeDstFromAbsoluteDstSrc(Arena* MemPool, Str8 Dst, Str8 Src)
{
    TempArena Scratch = GetScratch(&MemPool, 1);
    Str8 DstName = StrSkipLastSlash(Dst);
    Str8 SrcFolder = Src;
    Str8 DstFolder = StrChopLastSlash(Dst);
    Str8List SrcFolders = StrSplitPath(Scratch.MemPool, SrcFolder);
    Str8List DstFolders = StrSplitPath(Scratch.MemPool, DstFolder);
    u64 NumBacktracks = SrcFolders.Count;

    for (
        Str8Node* SrcNode = SrcFolders.Head, *BPNode = DstFolders.Head; 
        SrcNode && BPNode; 
        SrcNode = SrcNode->Next, BPNode = BPNode->Next
    ) {
        if (StrMatch(SrcNode->String, BPNode->String, PathMatchKindFromSystem())) {
            --NumBacktracks;
        } else {
            break;
        }
    }

    Str8 Result;

    if (NumBacktracks >= SrcFolders.Count) {
        Result = Dst;
    } else {
        Str8List DstPathStrings = {};

        for (u64 Index = 0; Index < NumBacktracks; ++Index)
            ListPush(Scratch.MemPool, &DstPathStrings, Str8Lit(".."));

        {
            b32 UniqueFromSrc = FALSE;

            for (
                Str8Node* SrcNode = SrcFolders.Head, *BPNode = DstFolders.Head;
                BPNode;
                BPNode = BPNode->Next
            ) {
                if (!UniqueFromSrc && (!SrcNode || !StrMatch(SrcNode->String, BPNode->String, PathMatchKindFromSystem())))
                    UniqueFromSrc = TRUE;

                if (UniqueFromSrc)
                    ListPush(Scratch.MemPool, &DstPathStrings, BPNode->String);

                if (SrcNode)
                    SrcNode = SrcNode->Next;
            }
        }

        ListPush(Scratch.MemPool, &DstPathStrings, DstName);

        Str8JoinPart Join = {};

        Join.Sep = Str8Lit("/");
        Result = StrListJoin(MemPool, &DstPathStrings, &Join);
    }

    ReleaseScratch(Scratch);

    return Result;
}

API_INTERNAL Str8List 
PathNormalisedListFromStr(Arena* MemPool, Str8 Path, OUT PathStyle* Style)
{
    PathStyle PStyle = PathStyleFromStr(Path);
    Str8List Result = StrSplitPath(MemPool, Path);

    StrPathListResolveDots(&Result, PStyle);

    if (Style)
        *Style = PStyle;

    return Result;
}

API_INTERNAL Str8 
PathNormalisedFromStr(Arena* MemPool, Str8 Path)
{
    TempArena Scratch = GetScratch(&MemPool, 1);
    PathStyle Style = PATH_STYLE_RELATIVE;
    Str8List ActualPath = PathNormalisedListFromStr(Scratch.MemPool, Path, &Style);
    Str8 Result = StrPathListJoinByStyle(MemPool, &ActualPath, Style);

    ReleaseScratch(Scratch);

    return Result;
}

API_INTERNAL b32 
PathMatchNormalised(Str8 Left, Str8 Right)
{
    TempArena Scratch = GetScratch(NULL, 0);
    Str8 LeftNormalised = PathNormalisedFromStr(Scratch.MemPool, Left);
    Str8 RightNormalised = PathNormalisedFromStr(Scratch.MemPool, Right);
    b32 Result = StrMatch(LeftNormalised, RightNormalised, STR_MATCH_ALL_CASES);

    ReleaseScratch(Scratch);

    return Result;
}

API_INTERNAL Str8 
StrSkipLastSlash(Str8 String)
{
    if (String.Size) {
        u8* Ptr = String.Str + String.Size - 1;

        for (; Ptr >= String.Str; --Ptr) {
            if (*Ptr == '/' || *Ptr == '\\')
                break;
        }

        if (Ptr >= String.Str) {
            ++Ptr;
            String.Size = (u64) (String.Str + String.Size - Ptr);
            String.Str = Ptr;
        }
    }

    return String;
}

API_INTERNAL Str8 
StrChopLastDot(Str8 String)
{
    Str8 Result = String;
    u64 P = String.Size;

    for (; P > 0; ) {
        --P;

        if (String.Str[P] == '.') {
            Result = StrPrefix(String, P);
            break;
        }
    }

    return Result;
}

API_INTERNAL Str8 
StrSkipLastDot(Str8 String)
{
    Str8 Result = String;
    u64 P = String.Size;

    for (; P > 0; ) {
        --P;

        if (String.Str[P] == '.') {
            Result = StrSkip(String, P + 1);
            break;
        }
    }

    return Result;
}

API_INTERNAL PathStyle 
PathStyleFromStr(Str8 String)
{
    PathStyle Result = PATH_STYLE_RELATIVE;

    if (String.Size >= 1 && *String.Str == '/') {
        Result = PATH_STYLE_UNIX_ABSOLUTE;
    } else if (String.Size >= 2 && IsAlpha(*String.Str) && String.Str[1] == ':') {
        if (String.Size == 2 || IsSlash(String.Str[2]))
            Result = PATH_STYLE_WIN_ABSOLUTE;
    }

    return Result;
}

API_INTERNAL Str8List 
StrSplitPath(Arena* MemPool, Str8 String)
{
    Str8List Result = StrSplit(MemPool, String, (u8*) "/\\", 2, FALSE);

    return Result;
}

API_INTERNAL void 
StrPathListResolveDots(Str8List* Path, PathStyle Style)
{
    TempArena Scratch = GetScratch(NULL, 0);
    Str8MetaNode* Stack = NULL;
    Str8MetaNode* FreeMetaNode = NULL;
    Str8Node* Head = Path->Head;

    MemSet(Path, 0, sizeof(*Path));

    for (
        Str8Node* Node = Head, *Next = NULL; 
        Node; 
        Node = Next
    ) {
        Next = Node->Next;

        if (Node == Head && Style == PATH_STYLE_WIN_ABSOLUTE)
            goto SAVE_WITHOUT_STACK;

        if (Node->String.Size == 1 && Node->String.Str[0] == '.')
            goto DO_NOTHING;

        if (Node->String.Size == 2 && Node->String.Str[0] == '.' && Node->String.Str[1] == '.') {
            if (Stack)
                goto ELIMINATE_STACK_TOP;
            else
                goto SAVE_WITHOUT_STACK;
        }

        goto SAVE_WITH_STACK;

SAVE_WITH_STACK:
        {
            StrListPushNode(Path, Node);

            Str8MetaNode* StackNode = FreeMetaNode;

            if (StackNode)
                SLL_STACK_POP(FreeMetaNode);
            else
                StackNode = ArenaPushArray(Scratch.MemPool, Str8MetaNode, 1);

            SLL_STACK_PUSH(Stack, StackNode);
            StackNode->Node = Node;
            continue;
        }

SAVE_WITHOUT_STACK:
        {
            StrListPushNode(Path, Node);
            continue;
        }

ELIMINATE_STACK_TOP:
        {
            --Path->Count;
            Path->TotalSize -= Stack->Node->String.Size;
            SLL_STACK_POP(Stack);

            if (!Stack)
                Path->Tail = Path->Head;
            else
                Path->Tail = Stack->Node;

            continue;
        }

DO_NOTHING:
        continue;
    }

    ReleaseScratch(Scratch);
}

API_INTERNAL Str8 
StrPathListJoinByStyle(Arena* MemPool, Str8List* Path, PathStyle Style)
{
    Str8JoinPart Params = {};

    switch (Style) {
        case PATH_STYLE_NULL: {} break;

        case PATH_STYLE_RELATIVE:
        case PATH_STYLE_WIN_ABSOLUTE: {
            Params.Sep = Str8Lit("/");
        } break;

        case PATH_STYLE_UNIX_ABSOLUTE: {
            Params.Pre = Str8Lit("/");
            Params.Sep = Str8Lit("/");
        } break;
    }

    Str8 Result = StrListJoin(MemPool, Path, &Params);

    return Result;
}

Str8TextPointPair 
Str8TextPointPairFromStr(Str8 String)
{
    Str8TextPointPair Result = {};
    Str8 FilePart = {};
    Str8 LinePart = {};
    Str8 ColumnPart = {};

    for (u64 Index = 0; Index <= String.Size; ++Index) {
        u8 Byte = (Index < String.Size) ? (String.Str[Index]) : 0;
        u8 NextByte = ((Index + 1 < String.Size) ? (String.Str[Index + 1]) : 0);

        if (Byte == ':' && NextByte != '/' && NextByte != '\\') {
            FilePart = StrPrefix(String, Index);
            LinePart = StrSkip(String, Index + 1);
            break;
        } else if (!Byte) {
            FilePart = String;
            break;
        }
    }

    u64 ColonPosition = StrFindSubStr(LinePart, ":"_s8, 0, 0);

    if (ColonPosition < LinePart.Size) {
        ColumnPart = StrSkip(LinePart, ColonPosition + 1);
        LinePart = StrPrefix(LinePart, ColonPosition);
    }

    u64 Line = 0;
    u64 Column = 0;

    Line = U64FromStr(LinePart);
    Column = U64FromStr(ColumnPart);
    Result.String = FilePart;
    Result.Point = TxtPt(Line, Column);

    if (Result.Point.Line == 0)
        Result.Point.Line = 1;

    if (Result.Point.Column == 0)
        Result.Point.Column = 1;

    return Result;
}

API_INTERNAL Str8 
StrFromPathStyle(PathStyle Style)
{
    ASSERT(Style < ARRAY_COUNT(STRING_PATH_STYLE_MAP));

    return STRING_PATH_STYLE_MAP[Style].String;
}

API_INTERNAL Str8 
PathSeparatorStrFromStyle(PathStyle Style)
{
    Str8 Result = {};

    switch (Style) {
        case PATH_STYLE_NULL: {} break;

        case PATH_STYLE_RELATIVE: {} break;

        case PATH_STYLE_WIN_ABSOLUTE: {
            Result = Str8Lit("\\");
        } break;

        case PATH_STYLE_UNIX_ABSOLUTE: {
            Result = Str8Lit("/");
        } break;
    }

    return Result;
}

API_INTERNAL StrMatchKind 
PathMatchKindFromSystem(void)
{
    StrMatchKind Result = STR_MATCH_SKIP_SLASH;

#if defined(NC_OS_WIN)
    Result |= STR_MATCH_ALL_CASES;
#endif

    return Result;
}

API_INTERNAL Str8 
PathConvertSlashes(Arena* MemPool, Str8 Path, PathStyle Style)
{
    TempArena Scratch = GetScratch(&MemPool, 1);
    Str8List List = StrSplitPath(Scratch.MemPool, Path);
    Str8JoinPart Join = {};

    Join.Sep = PathSeparatorStrFromStyle(Style);

    Str8 Result = StrListJoin(MemPool, &List, &Join);

    ReleaseScratch(Scratch);

    return Result;
}

API_INTERNAL Str8 
PathReplaceFileExtension(Arena* MemPool, Str8 FileName, Str8 Extension)
{
    Str8 FileNameNoExtension = StrChopLastDot(FileName);
    Str8 Result = ArenaPushStrFmt(
        MemPool,  
        "%S.%S", 
        PRINT_STR(FileNameNoExtension), 
        PRINT_STR(Extension)
    );

    return Result;
}

API_INTERNAL void
StrListCat(Str8List* Dst, Str8List* Src)
{
    if (Src->Count) {
        if (Dst->Tail) {
            Dst->Count += Src->Count;
            Dst->TotalSize += Src->TotalSize;
            Dst->Tail->Next = Src->Head;
            Dst->Tail = Src->Tail;
        } else {
            *Dst = *Src;
        }

        MemSet(Src, 0, sizeof(*Src));
    }
}

API_INTERNAL Str8 
IndentedFromStr(Arena* MemPool, Str8 String)
{
    TempArena Scratch = GetScratch(&MemPool, 1);
    readonly local u8 IndentationBytes[] = "                                                                                                                                ";
    Str8List IndentedStrings = {};
    i64 Depth = 0;
    i64 NextDepth = 0;
    u64 LineBeginOffset = 0;

    for (u64 Offset = 0; Offset <= String.Size; ++Offset) {
        u8 Byte = (Offset < String.Size) ? String.Str[Offset] : 0;

        switch (Byte) {
            default: {} break;

            case '{':
            case '[':
            case '(': {
                ++NextDepth;
                NextDepth = MAX(0, NextDepth);
            } break;

            case '}':
            case ']':
            case ')': {
                --NextDepth;
                NextDepth = MAX(0, NextDepth);
                Depth = NextDepth;
            } break;

            case '\n':
            case 0: {
                Str8 Line = StrSkipChopWhitespace(StrSub(String, Rng(LineBeginOffset, Offset)));

                if (Line.Size) {
                    ListPushFmt(
                        Scratch.MemPool, 
                        &IndentedStrings, 
                        (char*) "%S%S\n", 
                        (int) Depth * 2, 
                        IndentationBytes, 
                        PRINT_STR(Line)
                    );
                }

                if (!Line.Size && IndentedStrings.Count != 0 && Offset < String.Size)
                    ListPushFmt(Scratch.MemPool, &IndentedStrings, "\n");

                LineBeginOffset = Offset + 1;
                Depth = NextDepth;
            } break;
        }
    }

    Str8 Result = StrListJoin(MemPool, &IndentedStrings, NULL);

    ReleaseScratch(Scratch);

    return Result;
}

API_INTERNAL void 
StrListFromKinds(
    Arena* MemPool, 
    Str8List* List, 
    u32 Kinds, 
    Str8 const* KindStringTable,
    u32 KindStringTableSize
) {
    for (u32 Index = 0; Index < KindStringTableSize; ++Index) {
        u32 Kind = (1 << Index);

        if (Kinds & Kind)
            ListPush(MemPool, List, KindStringTable[Index]);
    }
}

API_INTERNAL void 
SerialBegin(Arena* MemPool, Str8List* List)
{
    Str8Node* Node = ArenaPushArrayZero(MemPool, Str8Node, 1);

    Node->String.Str = ArenaPushArray(MemPool, u8, 0);
    List->Head = Node;
    List->Tail = Node;
    List->Count = 1;
    List->TotalSize = 0;
}

API_INTERNAL Str8 
SerialEnd(Arena* MemPool, Str8List* List)
{
    u64 Size = List->TotalSize;
    u8* Out = ArenaPushArray(MemPool, u8, Size);

    SerialComplete(List, Out);

    Str8 Result = Str(Out, Size);

    return Result;
}

API_INTERNAL void 
SerialComplete(Str8List* List, OUT void* Dst)
{
    u8* Ptr = (u8*) Dst;

    for (
        Str8Node* Node = List->Head;
        Node;
        Node = Node->Next
    ) {
        u64 Size = Node->String.Size;

        MemCpy(Ptr, Node->String.Str, Size);
        Ptr += Size;
    }
}

API_INTERNAL u64 
SerialPushAligned(Arena* MemPool, Str8List* List, u64 Align)
{
    ASSERT_POW2(Align);

    u64 Position = List->TotalSize;
    u64 NewPosition = ALIGNTO(Position, Align);
    u64 Size = (NewPosition - Position);

    if (Size) {
        u8* Buffer = ArenaPushArrayZero(MemPool, u8, Size);
        Str8 const* String = &List->Tail->String;

        if (String->Str + String->Size == Buffer) {
            List->Tail->String.Size += Size;
            List->TotalSize += Size;
        } else {
            ListPush(MemPool, List, Str(Buffer, Size));
        }
    }

    return Size;
}

API_INTERNAL void* 
SerialPush(Arena* MemPool, Str8List* List, u64 Size)
{
    void* Result = NULL;

    if (Size) {
        u8* Buffer = ArenaPushArray(MemPool, u8, Size);
        Str8 const* String = &List->Tail->String;

        if (String->Str + String->Size == Buffer) {
            List->Tail->String.Size += Size;
            List->TotalSize += Size;
        } else {
            ListPush(MemPool, List, Str(Buffer, Size));
        }

        Result = Buffer;
    }

    return Result;
}

API_INTERNAL void* 
SerialPush(Arena* MemPool, Str8List* List, void* Data, u64 Size)
{
    void* Result = SerialPush(MemPool, List, Size);

    if (Result)
        MemCpy(Result, Data, Size);

    return Result;
}

API_INTERNAL void 
SerialPush(Arena* MemPool, Str8List* List, Str8Node* Head)
{
    for (
        Str8Node* Node = Head;
        Node;
        Node = Node->Next
    ) {
        SerialPush(MemPool, List, Node->String.Str, Node->String.Size);
    }
}

API_INTERNAL void 
SerialPushU64(Arena* MemPool, Str8List* List, u64 Value)
{
    u8* Buffer = ArenaPushArray(MemPool, u8, 8);

    MemCpy(Buffer, &Value, 8);

    Str8 const* String = &List->Tail->String;

    if (String->Str + String->Size == Buffer) {
        List->Tail->String.Size += 8;
        List->TotalSize += 8;
    } else {
        ListPush(MemPool, List, Str(Buffer, 8));
    }
}

API_INTERNAL void 
SerialPushU32(Arena* MemPool, Str8List* List, u32 Value)
{
    u8* Buffer = ArenaPushArray(MemPool, u8, 4);

    MemCpy(Buffer, &Value, 4);

    Str8 const* String = &List->Tail->String;

    if (String->Str + String->Size == Buffer) {
        List->Tail->String.Size += 4;
        List->TotalSize += 4;
    } else {
        ListPush(MemPool, List, Str(Buffer, 4));
    }
}

API_INTERNAL void 
SerialPushU16(Arena* MemPool, Str8List* List, u16 Value)
{
    SerialPush(MemPool, List, &Value, sizeof(Value));
}

API_INTERNAL void 
SerialPushU8(Arena* MemPool, Str8List* List, u8 Value)
{
    SerialPush(MemPool, List, &Value, sizeof(Value));
}

API_INTERNAL void 
SerialPushCStr(Arena* MemPool, Str8List* List, Str8 CString)
{
    SerialPush(MemPool, List, CString.Str, CString.Size);
    SerialPushU8(MemPool, List, 0);
}

API_INTERNAL void 
SerialPushStr(Arena* MemPool, Str8List* List, Str8 String)
{
    SerialPush(MemPool, List, String.Str, String.Size);
}

u64 
SerialPop(
    Str8 String, 
    u64 Offset, 
    void* ReadDst, 
    u64 ReadSize, 
    u64 Granularity
) {
    u64 BytesLeft = String.Size - MIN(Offset, String.Size);
    u64 ReadableSize = MIN(BytesLeft, ReadSize);
    u64 BytesToBeRead = ReadableSize - ReadableSize % Granularity;

    if (BytesToBeRead)
        MemCpy(ReadDst, String.Str + Offset, BytesToBeRead);

    return BytesToBeRead;
}

u64 
SerialFindFirstMatch(Str8 String, u64 Offset, u16 ScanValue)
{
    u64 Cursor = Offset;

    for (;;) {
        u16 Value = 0;

        SerialPopStruct(String, Cursor, &Value);

        if (Value == ScanValue)
            break;

        Cursor += sizeof(Value);
    }

    return Cursor;
}

void* 
SerialPopRawPtr(Str8 String, u64 Offset, u64 Size)
{
    void* Result = NULL;

    if (Offset + Size <= String.Size)
        Result = String.Str + Offset;

    return Result;
}

u64 
SerialPopCStr(Str8 String, u64 Offset, OUT Str8* CString)
{
    u64 ResultSize = 0;

    if (Offset < String.Size) {
        u8* Ptr = String.Str + Offset;
        u8* Cap = String.Str + String.Size;

        *CString = Str(Ptr, Cap);
        ResultSize = (CString->Size + 1);
    }

    return ResultSize;
}

u64 
SerialPopStr16(Str8 String, u64 Offset, OUT Str16* WideString)
{
    u64 EndOffset = SerialFindFirstMatch(String, Offset, 0);
    u64 Size = EndOffset - Offset;
    u16* RawWideString = (u16*) SerialPopRawPtr(String, Offset, Size);
    u64 Count = Size / sizeof(*RawWideString);

    *WideString = Str(RawWideString, Count);

    u64 ResultSize = Size + sizeof(*RawWideString);

    return ResultSize;
}

u64 
SerialPopBlock(Str8 String, u64 Offset, u64 Size, OUT Str8* Block)
{
    r1u64 Range = Rng(Offset, Offset + Size);

    *Block = StrSub(String, Range);

    return Block->Size;
}

API_INTERNAL b32 
IsSpace(u8 Char)
{
    return (
        Char == ' ' || 
        Char == '\r' || 
        Char == '\t' || 
        Char == '\f' || 
        Char == '\v' || 
        Char == '\n'
    );
}

API_INTERNAL b32 
IsUpper(u8 Char)
{
    return (Char >= 'A' && Char <= 'Z');
}

API_INTERNAL b32 
IsLower(u8 Char)
{
    return (Char >= 'a' && Char <= 'z');
}

API_INTERNAL b32 
IsAlpha(u8 Char)
{
    return (IsUpper(Char) || IsLower(Char));
}

API_INTERNAL b32 
IsSlash(u8 Char)
{
    return (Char == '\\' || Char == '/');
}

API_INTERNAL b32 
IsDigit(u8 Char, u32 Base)
{
    b32 Result = FALSE;

    if (Base > 0 && Base <= 16) {
        u8 Val = INTEGER_SYMBOL_REVERSE[Char];

        if (Val < Base)
            Result = TRUE;
    }

    return Result;
}

API_INTERNAL u8 
ToLower(u8 Char)
{
    if (IsUpper(Char))
        return 'a' + (Char - 'A');

    return Char;
}

API_INTERNAL u8 
ToUpper(u8 Char)
{
    if (IsLower(Char))
        return 'A' + (Char - 'a');

    return Char;
}

API_INTERNAL u8
ToForwardSlash(u8 Char)
{
    if (Char == '\\')
        return '/';

    return Char;
}

TextPoint 
TxtPt(i64 Line, i64 Column)
{
    TextPoint Result = {};

    Result.Line = Line;
    Result.Column = Column;

    return Result;
}

TextRange 
TxtRng(TextPoint Min, TextPoint Max)
{
    TextRange Result = {};

    Result.Min = Min;
    Result.Max = Max;

    return Result;
}

FMRangeList
FuzzyFind(Arena* MemPool, Str8 Needle, Str8 String)
{
    FMRangeList Result = {};
    TempArena Scratch = GetScratch(&MemPool, 1);
    Str8List Needles = StrSplit(Scratch.MemPool, Needle, (u8*) " ", 1, FALSE);

    Result.StrPartCount = Needles.Count;

    for (Str8Node* Node = Needles.Head; Node; Node = Node->Next) {
        u64 FindPosition = 0;

        for ( ; FindPosition < String.Size; ) {
            FindPosition = StrFindSubStr(String, Node->String, FindPosition, STR_MATCH_ALL_CASES | STR_MATCH_SKIP_SLASH);

            b32 IsInGatheredRanges = FALSE;

            for (FMRangeNode* N = Result.Head; N; N = N->Next) {
                if (N->Range.Min <= FindPosition && N->Range.Max > FindPosition) {
                    IsInGatheredRanges = TRUE;
                    FindPosition = N->Range.Max;
                    break;
                }
            }

            if (!IsInGatheredRanges)
                break;
        }

        if (FindPosition < String.Size) {
            r1u64 Range = Rng(FindPosition, FindPosition + Node->String.Size);
            FMRangeNode* N = ArenaPushArrayZero(MemPool, FMRangeNode, 1);

            N->Range = Range;
            SLL_QUEUE_PUSH(Result.Head, Result.Tail, N);
            ++Result.Count;
            Result.TotalDimensions += Length(Range);
        }
    }

    ReleaseScratch(Scratch);

    return Result;
}

Str8
ToLower(Arena* MemPool, Str8 String)
{
    Str8 Result = ArenaPushStrCpy(MemPool, String);

    for (u64 Index = 0; Index < Result.Size; ++Index) {
        u8 Byte = Result.Str[Index];

        if (IsUpper(Byte))
            Result.Str[Index] += ('a' - 'A');
    }

    return Result;
}

#if defined(NC_LANG_CPP)
    b32
    operator==(TextPoint TPA, TextPoint TPB)
    {
        return (
            TPA.Column == TPB.Column &&
            TPA.Line == TPB.Line
        );
    }

    b32
    operator!=(TextPoint TPA, TextPoint TPB)
    {
        return !(TPA == TPB);
    }

    b32
    operator<(TextPoint TPA, TextPoint TPB)
    {
        if (TPA.Line < TPB.Line)
            return TRUE;

        if (TPA.Line == TPB.Line)
            return (TPA.Column < TPB.Column);

        return FALSE;
    }

    b32
    operator>(TextPoint TPA, TextPoint TPB)
    {
        if (TPA.Line > TPB.Line)
            return TRUE;

        if (TPA.Line == TPB.Line)
            return (TPA.Column > TPB.Column);

        return FALSE;
    }

    b32
    operator==(TextRange RangeA, TextRange RangeB)
    {
        return (
            RangeA.Min == RangeB.Min &&
            RangeA.Max == RangeB.Max
        );
    }

    b32
    operator!=(TextRange RangeA, TextRange RangeB)
    {
        return !(RangeA == RangeB);
    }
#endif
