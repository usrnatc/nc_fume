#include "nc_render.h"
#include "nc_arena.h"
#include "nc_memory.h"
#include "nc_cli.h"
#include "nc_console.h"

RendState* REND_STATE = NULL;

u32 REND_LINE_GLYPHS[2][16] = {
    {
        ' ',
        0x2502, 
        0x2502, 
        0x2502,
        0x2500, 
        0x2518, 
        0x2510, 
        0x2524,
        0x2500, 
        0x2514, 
        0x250C, 
        0x251C,
        0x2500, 
        0x2534, 
        0x252C, 
        0x253C
    },
    {
        ' ',
        0x2551, 
        0x2551, 
        0x2551,
        0x2550, 
        0x255D, 
        0x2557, 
        0x2563,
        0x2550, 
        0x255A, 
        0x2554, 
        0x2560,
        0x2550, 
        0x2569, 
        0x2566, 
        0x256C
    }
};

local readonly u32 REND_16_COLOURS[16] = {
    0x000000FF, 
    0xAA0000FF, 
    0x00AA00FF, 
    0xAA5500FF, 
    0x0000AAFF, 
    0xAA00AAFF, 
    0x00AAAAFF, 
    0xAAAAAAFF,
    0x555555FF, 
    0xFF5555FF, 
    0x55FF55FF, 
    0xFFFF55FF, 
    0x5555FFFF, 
    0xFF55FFFF, 
    0x55FFFFFF, 
    0xFFFFFFFF
};

INTERNAL u8
RendChannelFromF32(f32 Value)
{
    f32 Clamped = CLAMP(0.0f, Value, 1.0f);
    f32 Encoded = (Clamped <= 0.0031308f) 
        ? Clamped * 12.92f 
        : 1.055f * PowF32(Clamped, 1.0f / 2.4f) - 0.055f;

    return (u8) (Encoded * 255.0f + 0.5f);
}

INTERNAL f32
F32FromRendChannel(u8 Value)
{
    f32 Encoded = (f32) Value / 255.0f;
    f32 Result = (Encoded <= 0.04045f) 
        ? Encoded / 12.92f 
        : PowF32((Encoded + 0.055f) / 1.055f, 2.4f);

    return Result;
}

u32 
RendBlend(u32 Dst, v4f32 Src)
{
    u32 Result = U32FromRGBA(Src);

    if (Src.W < 1.0f) {
        v4f32 Blended = Lerp(
            RGBAFromU32(Dst), 
            Src, 
            CLAMP(0.0f, Src.W, 1.0f)
        );

        Blended.W = 1.0f;
        Result = U32FromRGBA(Blended);
    }

    return Result;
}

INTERNAL u32
RendDarken(u32 Colour, f32 Factor)
{
    v4f32 Scaled = RGBAFromU32(Colour) * Factor;

    Scaled.W = 1.0f;

    return U32FromRGBA(Scaled);
}

INTERNAL u32
Rend256FromColour(u32 Colour)
{
    u32 R = (Colour >> 24) & 0xFF;
    u32 G = (Colour >> 16) & 0xFF;
    u32 B = (Colour >> 8) & 0xFF;
    u32 Result = 0;

    if (R == G && G == B && R > 4 && R < 247) {
        Result = 232 + (R - 8) / 10;
    } else {
        Result = 16 + 36 * ((R + 25) / 51) + 6 * ((G + 25) / 51) + ((B + 25) / 51);
    }

    return Result;
}

INTERNAL u32
Rend16FromColour(u32 Colour)
{
    i32 R = (i32) ((Colour >> 24) & 0xFF);
    i32 G = (i32) ((Colour >> 16) & 0xFF);
    i32 B = (i32) ((Colour >> 8) & 0xFF);
    i32 BestDistance = I32_MAX;
    u32 Result = 0;

    for (u32 Index = 0; Index < ARRAY_COUNT(REND_16_COLOURS); ++Index) {
        u32 Candidate = REND_16_COLOURS[Index];
        i32 DR = R - (i32) ((Candidate >> 24) & 0xFF);
        i32 DG = G - (i32) ((Candidate >> 16) & 0xFF);
        i32 DB = B - (i32) ((Candidate >> 8) & 0xFF);
        i32 Distance = DR * DR + DG * DG + DB * DB;

        if (Distance < BestDistance) {
            BestDistance = Distance;
            Result = Index;
        }
    }

    return Result;
}

b32 
RendCodePointIsNarrow(u32 CodePoint)
{
    b32 Wide = (
        (
            CodePoint >= 0x1100 && 
            CodePoint <= 0x115F
        ) ||
        (
            CodePoint >= 0x2E80 && 
            CodePoint <= 0xA4CF
        ) ||
        (
            CodePoint >= 0xAC00 && 
            CodePoint <= 0xD7A3
        ) ||
        (
            CodePoint >= 0xF900 && 
            CodePoint <= 0xFAFF
        ) ||
        (
            CodePoint >= 0xFE30 && 
            CodePoint <= 0xFE4F
        ) ||
        (
            CodePoint >= 0xFF00 && 
            CodePoint <= 0xFF60
        ) ||
        (
            CodePoint >= 0xFFE0 && 
            CodePoint <= 0xFFE6
        ) ||
        (
            CodePoint >= 0x1F300 && 
            CodePoint <= 0x1FAFF
        ) ||
        (
            CodePoint >= 0x20000 && 
            CodePoint <= 0x3FFFD
        )
    );
    b32 Combining = (
        (
            CodePoint >= 0x0300 && 
            CodePoint <= 0x036F
        ) ||
        (
            CodePoint >= 0x200B && 
            CodePoint <= 0x200F
        ) ||
        (
            CodePoint >= 0xFE00 && 
            CodePoint <= 0xFE0F
        )
    );

    b32 Result = (
        !Wide && 
        !Combining && 
        CodePoint >= 32 && 
        CodePoint != 127
    );

    return Result;
}

RendCell* 
GetRendCellFromXY(RendGrid* Grid, i32 X, i32 Y)
{
    RendCell* Result = (
        Grid->Cells + 
        (u64) Y * (u64) Grid->Size.X + 
        (u64) X
    );

    return Result;
}

r2i32 
RendCellRectFromRect(r2f32 Rect)
{
    return Rng(
        (i32) FloorF32(Rect.X0),
        (i32) FloorF32(Rect.Y0),
        (i32) FloorF32(Rect.X1),
        (i32) FloorF32(Rect.Y1)
    );
}

INTERNAL void
RendGridAlloc(RendGrid* Grid, v2i32 Size)
{
    Grid->Size = Size;
    Grid->Cells = ArenaPushArray(
        REND_STATE->GridMemPool, 
        RendCell, 
        (u64) Size.X * (u64) Size.Y
    );
}

void 
RendInit(CommandLine* CLI)
{
    Arena* MemPool = ArenaAlloc();

    REND_STATE = ArenaPushArrayZero(MemPool, RendState, 1);
    REND_STATE->MemPool = MemPool;
    REND_STATE->GridMemPool = ArenaAlloc();
    REND_STATE->ColourMode = REND_COLOUR_MODE_TRUE;

    if (CLI && CommandLineHasFlag(CLI, "256"_s8))
        REND_STATE->ColourMode = REND_COLOUR_MODE_256;

    if (CLI && CommandLineHasFlag(CLI, "16"_s8))
        REND_STATE->ColourMode = REND_COLOUR_MODE_16;
}

void 
RendRelease(void)
{
    ArenaRelease(REND_STATE->GridMemPool);
    ArenaRelease(REND_STATE->MemPool);
    REND_STATE = NULL;
}

v2i32 
RendSize(void)
{
    return REND_STATE->BackBuffer.Size;
}

r2i32 
RendPushClip(r2i32 Clip)
{
    r2i32 Prev = REND_STATE->Clip;

    REND_STATE->Clip = Intersect(Prev, Clip);

    return Prev;
}

r2i32 
RendPopClip(r2i32 Prev)
{
    r2i32 Current = REND_STATE->Clip;

    REND_STATE->Clip = Prev;

    return Current;
}

void 
RendBeginFrame(
    Arena* FrameMemPool, 
    v2i32 Size, 
    v4f32 ForegroundColour, 
    v4f32 BackgroundColour
) {
    RendState* State = REND_STATE;

    Size.X = MAX(Size.X, 1);
    Size.Y = MAX(Size.Y, 1);

    if (
        Size.X != State->BackBuffer.Size.X || 
        Size.Y != State->BackBuffer.Size.Y
    ) {
        ArenaClear(State->GridMemPool);
        RendGridAlloc(&State->Buffer, Size);
        RendGridAlloc(&State->BackBuffer, Size);
        State->IsBufferValid = FALSE;
    }

    State->FrameMemPool = FrameMemPool;
    State->ForegroundColour = U32FromRGBA(ForegroundColour);
    State->BackgroundColour = U32FromRGBA(BackgroundColour);
    State->Clip = Rng(0, 0, Size.X, Size.Y);
    State->Caret.IsVisible = FALSE;

    RendCell Empty = {};

    Empty.CodePoint = EMPTY_REND_CELL_CODEPOINT_VALUE;
    Empty.ForegroundColour = State->ForegroundColour;
    Empty.BackgroundColour = State->BackgroundColour;

    u64 CellCount = (u64) Size.X * (u64) Size.Y;

    for (u64 Index = 0; Index < CellCount; ++Index)
        State->BackBuffer.Cells[Index] = Empty;
}

void 
RendFillRect(r2i32 Rect, v4f32 BackgroundColour)
{
    r2i32 Clipped = Intersect(Rect, REND_STATE->Clip);
    b32 Opaque = (BackgroundColour.W >= 1.0f);
    u32 Packed = U32FromRGBA(BackgroundColour);

    if (Opaque && InRange(Clipped, REND_STATE->Caret.Position))
        REND_STATE->Caret.IsVisible = FALSE;

    for (i32 Y = Clipped.Y0; Y < Clipped.Y1; ++Y) {
        RendCell* Cell = GetRendCellFromXY(
            &REND_STATE->BackBuffer, 
            Clipped.X0, 
            Y
        );

        for (i32 X = Clipped.X0; X < Clipped.X1; ++X, ++Cell) {
            if (Opaque) {
                Cell->CodePoint = EMPTY_REND_CELL_CODEPOINT_VALUE;
                Cell->BackgroundColour = Packed;
                Cell->Attributes = 0;
                Cell->Lines = 0;
            } else {
                Cell->BackgroundColour = RendBlend(
                    Cell->BackgroundColour, 
                    BackgroundColour
                );
                Cell->ForegroundColour = RendBlend(
                    Cell->ForegroundColour, 
                    BackgroundColour
                );
            }
        }
    }
}

void 
RendPutCell(i32 X, i32 Y, u32 CodePoint, v4f32 Fg, RendAttr Attrs)
{
    if (InRange(REND_STATE->Clip, Vec(X, Y))) {
        RendCell* Cell = GetRendCellFromXY(&REND_STATE->BackBuffer, X, Y);

        Cell->CodePoint = RendCodePointIsNarrow(CodePoint) 
            ? CodePoint 
            : '?';
        Cell->ForegroundColour = RendBlend(Cell->BackgroundColour, Fg);
        Cell->Attributes = Attrs;
        Cell->Lines = 0;
    }
}

void 
RendPutCell(
    i32 X, 
    i32 Y, 
    u32 CodePoint, 
    v4f32 ForegroundColour, 
    v4f32 BackgroundColour, 
    RendAttr Attributes
) {
    if (InRange(REND_STATE->Clip, Vec(X, Y))) {
        RendCell* Cell = GetRendCellFromXY(&REND_STATE->BackBuffer, X, Y);

        Cell->CodePoint = RendCodePointIsNarrow(CodePoint) 
            ? CodePoint 
            : '?';
        Cell->BackgroundColour = RendBlend(
            Cell->BackgroundColour, 
            BackgroundColour
        );
        Cell->ForegroundColour = RendBlend(
            Cell->BackgroundColour, 
            ForegroundColour
        );
        Cell->Attributes = Attributes;
        Cell->Lines = 0;
    }
}

void 
RendPutLine(i32 X, i32 Y, RendLineMask Mask, v4f32 ForegroundColour)
{
    if (InRange(REND_STATE->Clip, Vec(X, Y))) {
        RendCell* Cell = GetRendCellFromXY(&REND_STATE->BackBuffer, X, Y);
        RendLineMask Joined = (RendLineMask) ((Cell->Lines & 0xF) | Mask);
        u32 Style = !!(Mask & REND_LINE_DOUBLE);

        Cell->Lines = Joined;
        Cell->CodePoint = REND_LINE_GLYPHS[Style][Joined & 0xF];
        Cell->ForegroundColour = RendBlend(
            Cell->BackgroundColour, 
            ForegroundColour
        );
        Cell->Attributes = 0;
    }
}

void 
RendDarkenRect(r2i32 Rect, f32 Factor)
{
    r2i32 Clipped = Intersect(Rect, REND_STATE->Clip);

    for (i32 Y = Clipped.Y0; Y < Clipped.Y1; ++Y) {
        RendCell* Cell = GetRendCellFromXY(
            &REND_STATE->BackBuffer, 
            Clipped.X0, 
            Y
        );

        for (i32 X = Clipped.X0; X < Clipped.X1; ++X, ++Cell) {
            Cell->BackgroundColour = RendDarken(
                Cell->BackgroundColour, 
                Factor
            );
            Cell->ForegroundColour = RendDarken(
                Cell->ForegroundColour, 
                Factor
            );
        }
    }
}

void 
RendInvertCell(i32 X, i32 Y)
{
    if (InRange(REND_STATE->Clip, Vec(X, Y))) {
        RendCell* Cell = GetRendCellFromXY(&REND_STATE->BackBuffer, X, Y);
        u32 ForegroundColour = Cell->ForegroundColour;

        Cell->ForegroundColour = Cell->BackgroundColour;
        Cell->BackgroundColour = ForegroundColour;
    }
}

void 
RendSetCaret(v2i32 Position)
{
    if (
        InRange(
            REND_STATE->Clip, 
            Vec(Position.X, Position.Y)
        )
    ) {
        REND_STATE->Caret.IsVisible = TRUE;
        REND_STATE->Caret.Position = Position;
    }
}

INTERNAL void
EmitBytes(RendEmit* Emit, char const* Bytes, u64 Count)
{
    if (Emit->Curr + Count <= Emit->End) {
        MemCpy(Emit->Curr, Bytes, Count);
        Emit->Curr += Count;
    }
}

#define EmitLit(Emit, X) EmitBytes((Emit), (X), sizeof(X) - 1)

INTERNAL void
EmitU32(RendEmit* Emit, u32 Value)
{
    char Digits[10];
    u64 Count = 0;

    do {
        Digits[Count++] = (char) ('0' + Value % 10);
        Value /= 10;
    } while (Value);

    while (Count)
        EmitBytes(Emit, &Digits[--Count], 1);
}

INTERNAL void
EmitCursorMove(RendEmit* Emit, i32 X, i32 Y)
{
    EmitLit(Emit, "\x1b[");
    EmitU32(Emit, (u32) Y + 1);
    EmitLit(Emit, ";");
    EmitU32(Emit, (u32) X + 1);
    EmitLit(Emit, "H");
    Emit->Cursor = Vec(X, Y);
}

INTERNAL void
EmitParam(RendEmit* Emit, b32* First, u32 Value)
{
    if (!*First)
        EmitLit(Emit, ";");

    EmitU32(Emit, Value);
    *First = FALSE;
}

INTERNAL void
EmitColour(RendEmit* Emit, b32* First, u32 Colour, b32 IsBg)
{
    switch (REND_STATE->ColourMode) {
        default:
        case REND_COLOUR_MODE_TRUE: {
            EmitParam(Emit, First, IsBg ? 48 : 38);
            EmitParam(Emit, First, 2);
            EmitParam(Emit, First, (Colour >> 24) & 0xFF);
            EmitParam(Emit, First, (Colour >> 16) & 0xFF);
            EmitParam(Emit, First, (Colour >> 8) & 0xFF);
        } break;

        case REND_COLOUR_MODE_256: {
            EmitParam(Emit, First, IsBg ? 48 : 38);
            EmitParam(Emit, First, 5);
            EmitParam(Emit, First, Rend256FromColour(Colour));
        } break;

        case REND_COLOUR_MODE_16: {
            u32 Index = Rend16FromColour(Colour);
            // FIXME(nc): wtf is this absolute dog shit?>??
            u32 Base = (Index < 8) 
                ? (
                    IsBg 
                        ? 40 
                        : 30
                ) 
                : (
                    IsBg 
                        ? 100 
                        : 90
                );

            EmitParam(Emit, First, Base + (Index & 7));
        } break;
    }
}

INTERNAL void
EmitAttrs(RendEmit* Emit, b32* First, u32 Attrs)
{
    local readonly struct {
        RendAttr Attr;
        u32      Code;
    } Map[] = {
        {
            REND_ATTR_BOLD,
            1
        },
        {
            REND_ATTR_DIMMED,
            2
        },
        {
            REND_ATTR_ITALIC,
            3
        },
        {
            REND_ATTR_UNDERLINE,
            4
        },
        {
            REND_ATTR_INVERSE,
            7
        },
        {
            REND_ATTR_STRIKETHROUGH,
            9
        }
    };

    for (u64 Index = 0; Index < ARRAY_COUNT(Map); ++Index) {
        if (Attrs & Map[Index].Attr)
            EmitParam(Emit, First, Map[Index].Code);
    }
}

INTERNAL void
EmitStyle(RendEmit* Emit, RendCell* Cell)
{
    if (
        Cell->Attributes != Emit->Attributes || 
        Cell->ForegroundColour != Emit->ForegroundColour || 
        Cell->BackgroundColour != Emit->BackgroundColour
    ) {
        b32 First = TRUE;

        EmitLit(Emit, "\x1b[");

        if (Emit->Attributes & ~Cell->Attributes) {
            EmitParam(Emit, &First, 0);
            Emit->ForegroundColour = U32_MAX;
            Emit->BackgroundColour = U32_MAX;
            Emit->Attributes = 0;
        }

        EmitAttrs(Emit, &First, Cell->Attributes & ~Emit->Attributes);

        if (Cell->ForegroundColour != Emit->ForegroundColour)
            EmitColour(Emit, &First, Cell->ForegroundColour, FALSE);

        if (Cell->BackgroundColour != Emit->BackgroundColour)
            EmitColour(Emit, &First, Cell->BackgroundColour, TRUE);

        EmitLit(Emit, "m");
        Emit->ForegroundColour = Cell->ForegroundColour;
        Emit->BackgroundColour = Cell->BackgroundColour;
        Emit->Attributes = Cell->Attributes;
    }
}

INTERNAL void
EmitCodePoint(RendEmit* Emit, u32 CodePoint)
{
    u8 Bytes[4];
    u32 Count = UTF8Encode(Bytes, CodePoint);

    EmitBytes(Emit, (char*) Bytes, Count);
}

INTERNAL b32
RendCellsMatch(RendCell* A, RendCell* B, u64 Count)
{
    b32 Result = TRUE;

    for (u64 Index = 0; Result && Index < Count; ++Index) {
        Result = (
            A[Index].U64[0] == B[Index].U64[0] && 
            A[Index].U64[1] == B[Index].U64[1]
        );
    }

    return Result;
}

void 
RendEndFrame(void)
{
    RendState* State = REND_STATE;
    RendGrid* Front = &State->Buffer;
    RendGrid* Back = &State->BackBuffer;
    v2i32 Size = Back->Size;
    u64 CellCount = (u64) Size.X * (u64) Size.Y;
    u64 Capacity = CellCount * (REND_SGR_MAX_BYTES + 16) + 256;
    RendEmit Emit = {};

    Emit.Curr = ArenaPushArray(State->FrameMemPool, u8, Capacity);
    Emit.End = Emit.Curr + Capacity;
    Emit.Cursor = Vec(-1, -1);
    Emit.ForegroundColour = U32_MAX;
    Emit.BackgroundColour = U32_MAX;
    Emit.Attributes = 0;

    u8* Start = Emit.Curr;

    EmitLit(&Emit, "\x1b[?2026h\x1b[?25l\x1b[0m");

    if (!State->IsBufferValid)
        EmitLit(&Emit, "\x1b[2J");

    for (i32 Y = 0; Y < Size.Y; ++Y) {
        RendCell* BackRow = GetRendCellFromXY(Back, 0, Y);
        RendCell* FrontRow = GetRendCellFromXY(Front, 0, Y);

        if (
            State->IsBufferValid && 
            RendCellsMatch(BackRow, FrontRow, (u64) Size.X)
        ) {
            continue;
        }

        for (i32 X = 0; X < Size.X; ++X) {
            RendCell* Cell = BackRow + X;

            if (
                State->IsBufferValid && 
                Cell->U64[0] == FrontRow[X].U64[0] && 
                Cell->U64[1] == FrontRow[X].U64[1]
            ) {
                continue;
            }

            if (Emit.Cursor.X != X || Emit.Cursor.Y != Y)
                EmitCursorMove(&Emit, X, Y);

            EmitStyle(&Emit, Cell);
            EmitCodePoint(&Emit, Cell->CodePoint);
            ++Emit.Cursor.X;

            if (Emit.Cursor.X >= Size.X)
                Emit.Cursor = Vec(-1, -1);
        }
    }

    if (State->Caret.IsVisible) {
        EmitCursorMove(
            &Emit, 
            State->Caret.Position.X, 
            State->Caret.Position.Y
        );
        EmitLit(&Emit, "\x1b[?25h");
    }

    EmitLit(&Emit, "\x1b[?2026l");
    State->PrevFrameBytes = (u64) (Emit.Curr - Start);
    State->PrevFrame = Str(Start, State->PrevFrameBytes);
    TerminalWrite(State->PrevFrame);

    RendGrid Swap = State->Buffer;

    State->Buffer = State->BackBuffer;
    State->BackBuffer = Swap;
    State->IsBufferValid = TRUE;
}
