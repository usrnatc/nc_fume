#if !defined(__NC_DRAW_H__)
#define __NC_DRAW_H__

#include "nc_types.h"
#include "nc_string.h"

// @defines____________________________________________________________________
typedef u32 FancyStrFlag;
enum : u32 {
    FANCY_STR_FLAG_BOLD          = (1 << 0),
    FANCY_STR_FLAG_DIMMED        = (1 << 1),
    FANCY_STR_FLAG_ITALIC        = (1 << 2),
    FANCY_STR_FLAG_UNDERLINE     = (1 << 3),
    FANCY_STR_FLAG_STRIKETHROUGH = (1 << 4),
    FANCY_STR_FLAG_INVERSE       = (1 << 5)
};

// @types______________________________________________________________________
struct FancyStrParams {
    v4f32          Colour;
    FancyStrFlag   Flags;
};

struct FancyStr {
    Str8           String;
    FancyStrParams Params;
};

struct FancyStrNode {
    FancyStrNode* Next;
    FancyStr      V;
};

struct FancyStrList {
    FancyStrNode* Head;
    FancyStrNode* Tail;
    u64           NodeCount;
    u64           TotalSize;
};

// @runtime____________________________________________________________________


// @functions__________________________________________________________________
void ListPush(Arena* MemPool, FancyStrList* List, FancyStr* String);
void ListPush(Arena* MemPool, FancyStrList* List, FancyStrParams* Params, FancyStrParams* Overrides, Str8 String);
void ListCat(FancyStrList* Dst, FancyStrList* Src);
FancyStrList ListCpy(Arena* MemPool, FancyStrList* List);
Str8 Str8FromFancyStrList(Arena* MemPool, FancyStrList* List);
v2f32 DDimensionsFromFancyStrList(f32 TabSize, FancyStrList* List);
u64 DCharPositionFromStr(f32 TabSize, Str8 String, f32 X);
void DrawRect(r2f32 Destination, v4f32 Colour, f32 BorderThickness);
void DrawTruncatedFancyStrList(v2f32 Position, FancyStrList* List, f32 MaxX, Str8 Trailer);
FMRangeList FuzzyFindFStrs(Arena* MemPool, FancyStrList* FancyStrings, Str8 Needle);
void DrawTruncatedFancyStrListFuzzyMatches(v2f32 Position, FancyStrList* List, f32 MaxX, FMRangeList* Ranges, v4f32 Colour);

#endif // __NC_DRAW_H__
