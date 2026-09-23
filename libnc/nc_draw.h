#if !defined(__NC_DRAW_H__)
#define __NC_DRAW_H__

#include "nc_types.h"
#include "nc_string.h"
#include "nc_render.h"
#include "nc_console.h"

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

#define DClipScope(X) DEFER(DPushClip(X), DPopClip())

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

struct DClipNode {
    DClipNode* Next;
    r2i32      V;
};

struct DrawState {
    Arena*     MemPool;
    DClipNode* HeadClip;
    DClipNode* FreeClip;
};

// @runtime____________________________________________________________________
extern DrawState* DRAW_STATE;

// @functions__________________________________________________________________
void DrawInit(void);
void DBeginFrame(Arena* FrameMemPool, v4f32 ForegroundColour, v4f32 BackgroundColour);
void DEndFrame(void);
r2f32 DPushClip(r2f32 Clip);
r2f32 DPopClip(void);
r2f32 DHeadClip(void);
void ListPush(Arena* MemPool, FancyStrList* List, FancyStr* String);
void ListPush(Arena* MemPool, FancyStrList* List, FancyStrParams* Params, FancyStrParams* Overrides, Str8 String);
void ListCat(FancyStrList* Dst, FancyStrList* Src);
FancyStrList ListCpy(Arena* MemPool, FancyStrList* List);
Str8 Str8FromFancyStrList(Arena* MemPool, FancyStrList* List);
v2f32 DDimensionsFromFancyStrList(f32 TabSize, FancyStrList* List);
v2f32 DDimensionsFromStr(f32 TabSize, Str8 String);
u64 DCharPositionFromStr(f32 TabSize, Str8 String, f32 X);
Str8List DWrappedLinesFromStr(Arena* MemPool, f32 TabSize, Str8 String, f32 Max);
void DrawRect(r2f32 Destination, v4f32 Colour, f32 BorderThickness);
void DrawBorder(r2f32 Destination, v4f32 Colour, b32 IsDouble);
void DrawLine(v2f32 Point0, v2f32 Point1, v4f32 Colour);
void DrawBar(r2f32 Destination, Axis2D Axis, r1f32 Fill, v4f32 FillColour, v4f32 TrackColour);
void DrawShadow(r2f32 Destination);
void DrawCaret(v2f32 Position);
void DrawTruncatedFancyStrList(v2f32 Position, FancyStrList* List, f32 MaxX, Str8 Trailer);
FMRangeList FuzzyFindFStrs(Arena* MemPool, FancyStrList* FancyStrings, Str8 Needle);
void DrawTruncatedFancyStrListFuzzyMatches(v2f32 Position, f32 TabSize, FancyStrList* List, f32 MaxX, FMRangeList* Ranges, v4f32 Colour);

#endif // __NC_DRAW_H__
