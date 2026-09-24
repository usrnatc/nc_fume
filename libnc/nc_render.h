#if !defined(__NC_RENDER_H__)
#define __NC_RENDER_H__

#include "nc_types.h"
#include "nc_math.h"
#include "nc_string.h"

struct Arena;
struct CommandLine;

// @defines____________________________________________________________________
typedef u8 RendAttr;
enum : u8 {
    REND_ATTR_BOLD          = (1 << 0),
    REND_ATTR_DIMMED        = (1 << 1),
    REND_ATTR_ITALIC        = (1 << 2),
    REND_ATTR_UNDERLINE     = (1 << 3),
    REND_ATTR_STRIKETHROUGH = (1 << 4),
    REND_ATTR_INVERSE       = (1 << 5)
};

typedef u8 RendLineMask;
enum : u8 {
    REND_LINE_UP     = (1 << 0),
    REND_LINE_DOWN   = (1 << 1),
    REND_LINE_LEFT   = (1 << 2),
    REND_LINE_RIGHT  = (1 << 3),
    REND_LINE_DOUBLE = (1 << 4)
};

enum RendColourMode {
    REND_COLOUR_MODE_TRUE,
    REND_COLOUR_MODE_256,
    REND_COLOUR_MODE_16,
    REND_COLOUR_MODE_COUNT
};

#define EMPTY_REND_CELL_CODEPOINT_VALUE ' '
#define REND_SGR_MAX_BYTES              64

// @types______________________________________________________________________
union RendCell {
    struct {
        u32          CodePoint;
        u32          ForegroundColour;
        u32          BackgroundColour;
        RendAttr     Attributes;
        RendLineMask Lines;
        u8           __Padding[2];
    };

    u64 U64[2];
};
STATIC_ASSERT(sizeof(RendCell) == sizeof(u128), RendCellSizeCheck);

struct RendGrid {
    RendCell* Cells;
    v2i32     Size;
};

struct RendCaret {
    b32   IsVisible;
    v2i32 Position;
};

struct RendState {
    Arena*         MemPool;
    Arena*         GridMemPool;
    Arena*         FrameMemPool;
    RendColourMode ColourMode;
    RendGrid       Buffer;
    RendGrid       BackBuffer;
    b32            IsFrontValid;
    r2i32          Clip;
    RendCaret      Caret;
    u32            ForegroundColour;
    u32            BackgroundColour;
    u64            PrevFrameBytes;
};

// @runtime____________________________________________________________________
extern RendState* REND_STATE;
extern u32 REND_LINE_GLYPHS[2][16];

// @functions__________________________________________________________________
u32 RendBlend(u32 Dst, v4f32 Src);
b32 RendCodePointIsNarrow(u32 CodePoint);
RendCell* GetRendCellFromXY(RendGrid* Grid, i32 X, i32 Y);
r2i32 RendCellRectFromRect(r2f32 Rect);
void RendInit(CommandLine* CLI);
void RendRelease(void);
v2i32 RendSize(void);
r2i32 RendPushClip(r2i32 Clip);
r2i32 RendPopClip(r2i32 Prev);
void RendBeginFrame(Arena* FrameMemPool, v2i32 Size, v4f32 ForegroundColour, v4f32 BackgroundColour);
void RendFillRect(r2i32 Rect, v4f32 Colour);
void RendPutCell(i32 X, i32 Y, u32 CodePoint, v4f32 ForegroundColour, RendAttr Attributes);
void RendPutCell(i32 X, i32 Y, u32 CodePoint, v4f32 ForegroundColour, v4f32 BackgroundColour, RendAttr Attributes);
void RendPutLine(i32 X, i32 Y, RendLineMask Mask, v4f32 ForegroundColour);
void RendDarkenRect(r2i32 Rect, f32 Factor);
void RendInvertCell(i32 X, i32 Y);
void RendSetCaret(v2i32 Position);
void RendEndFrame(void);

#endif // __NC_RENDER_H__
