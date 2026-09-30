#include "nc_draw.h"
#include "nc_arena.h"
#include "nc_memory.h"
#include "nc_tls.h"

DrawState* DRAW_STATE = NULL;

local readonly u32 DRAW_EIGHTHS_LEFT[9] = {
    ' ', 
    0x258F, 
    0x258E, 
    0x258D, 
    0x258C, 
    0x258B, 
    0x258A, 
    0x2589, 
    0x2588
};

local readonly u32 DRAW_EIGHTHS_LOWER[9] = {
    ' ', 
    0x2581, 
    0x2582, 
    0x2583, 
    0x2584, 
    0x2585, 
    0x2586, 
    0x2587, 
    0x2588
};

local readonly u32 DRAW_POINTER_GLYPHS[CURSOR_KIND_COUNT] = {
    0,
    0x2502,
    0x2194,
    0x2195,
    0x2198,
    0x2197,
    0x2725,
    0,
    0x2298
};

INTERNAL u64
DTabAdvance(u64 Columns, u64 TabColumns)
{
    u64 Result = TabColumns - (Columns % TabColumns);

    return Result;
}

v2f32 
DDimensionsFromFancyStrList(f32 TabSize, FancyStrList* List)
{
    u64 Columns = 0;
    u64 TabColumns = MAX((u64) TabSize, 1);

    for (FancyStrNode* Node = List->Head; Node; Node = Node->Next) {
        Str8 String = Node->V.String;

        for (u64 Offset = 0; Offset < String.Size;) {
            UnicodeDecode Decode = UTF8Decode(
                String.Str + Offset, 
                String.Size - Offset
            );

            Columns += (Decode.CodePoint == '\t') 
                ? DTabAdvance(Columns, TabColumns)
                : 1;
            Offset += MAX(Decode.Inc, 1);
        }
    }

    v2f32 Result = {};

    Result.X = (f32) Columns;
    Result.Y = List->Head ? 1.0f : 0.0f;

    return Result;
}

v2f32 
DDimensionsFromStr(f32 TabSize, Str8 String)
{
    FancyStrNode Node = {};
    FancyStrList List = {
        &Node,
        &Node,
        1,
        String.Size
    };

    Node.V.String = String;

    v2f32 Result = DDimensionsFromFancyStrList(TabSize, &List);

    return Result;
}

u64
DCharPositionFromStr(f32 TabSize, Str8 String, f32 X)
{
    u64 Columns = 0;
    u64 TabColumns = MAX((u64) TabSize, 1ULL);
    u64 Result = 0;

    while (Result < String.Size) {
        UnicodeDecode Decode = UTF8Decode(
            String.Str + Result,
            String.Size - Result
        );

        Columns += (Decode.CodePoint == '\t')
            ? DTabAdvance(Columns, TabColumns)
            : 1;

        if (X < (f32) Columns)
            break;

        Result += MAX(Decode.Inc, 1);
    }

    return Result;
}

Str8List 
DWrappedLinesFromStr(Arena* MemPool, f32 TabSize, Str8 String, f32 Max)
{
    Str8List Result = {};
    u64 MaxColumns = MAX((u64) Max, 1);
    u64 TabColumns = MAX((u64) TabSize, 1);
    u64 Columns = 0;
    u64 LineStart = 0;
    u64 BreakOffset = 0;

    for (u64 Offset = 0; Offset < String.Size;) {
        UnicodeDecode Decode = UTF8Decode(
            String.Str + Offset, 
            String.Size - Offset
        );
        u64 Inc = MAX(Decode.Inc, 1);
        u64 Width = (Decode.CodePoint == '\t') 
            ? DTabAdvance(Columns, TabColumns)
            : 1;

        if (Decode.CodePoint == '\n') {
            ListPush(
                MemPool, 
                &Result, 
                Str(
                    String.Str + LineStart, 
                    Offset - LineStart
                )
            );
            Offset += Inc;
            LineStart = Offset;
            BreakOffset = 0;
            Columns = 0;
        } else if (
            Columns + Width > MaxColumns && 
            Offset > LineStart
        ) {
            u64 LineEnd = (BreakOffset > LineStart) 
                ? BreakOffset 
                : Offset;
            Str8 Line = StrSkipChopWhitespace(
                Str(
                    String.Str + LineStart, 
                    LineEnd - LineStart
                )
            );

            if (Line.Size)
                ListPush(MemPool, &Result, Line);

            Offset = LineEnd;
            LineStart = LineEnd;
            BreakOffset = 0;
            Columns = 0;
        } else {
            Columns += Width;
            Offset += Inc;

            if (Decode.CodePoint == ' ')
                BreakOffset = Offset;
        }
    }

    Str8 LastLine = StrSkipChopWhitespace(
        Str(
            String.Str + LineStart, 
            String.Size - LineStart
        )
    );

    if (LastLine.Size)
        ListPush(MemPool, &Result, LastLine);

    return Result;
}

void 
ListPush(Arena* MemPool, FancyStrList* List, FancyStr* String)
{
    FancyStrNode* Node = ArenaPushArray(MemPool, FancyStrNode, 1);

    MemCpy(&Node->V, String, sizeof(Node->V));
    SLL_QUEUE_PUSH(List->Head, List->Tail, Node);
    ++List->NodeCount;
    List->TotalSize += String->String.Size;
}

void 
ListPush(
    Arena* MemPool, 
    FancyStrList* List, 
    FancyStrParams* Params, 
    FancyStrParams* Overrides, 
    Str8 String
) {
    FancyStr FString = {};

    FString.String = String;
    FString.Params = *Params;

    if (Overrides) {
        if (
            Overrides->Colour.X != 0.0f ||
            Overrides->Colour.Y != 0.0f ||
            Overrides->Colour.Z != 0.0f ||
            Overrides->Colour.W != 0.0f
        ) {
            FString.Params.Colour = Overrides->Colour;
        }

        FString.Params.Flags |= Overrides->Flags;
    }

    ListPush(MemPool, List, &FString);
}

void 
ListCat(FancyStrList* Dst, FancyStrList* Src)
{
    if (Dst->Tail && Src->Head) {
        Dst->Tail->Next = Src->Head;
        Dst->Tail = Src->Tail;
        Dst->TotalSize += Src->TotalSize;
        Dst->NodeCount += Src->NodeCount;
    } else if (Src->Head) {
        MemCpy(Dst, Src, sizeof(*Dst));
    }

    MemSet(Src, 0, sizeof(*Src));
}

FancyStrList 
ListCpy(Arena* MemPool, FancyStrList* List)
{
    FancyStrList Result = {};

    for (FancyStrNode* Node = List->Head; Node; Node = Node->Next) {
        FancyStr FString = Node->V;

        FString.String = ArenaPushStrCpy(MemPool, FString.String);
        ListPush(MemPool, &Result, &FString);
    }

    return Result;
}

Str8 
Str8FromFancyStrList(Arena* MemPool, FancyStrList* List)
{
    TempArena Scratch = GetScratch(&MemPool, 1);
    Str8List Parts = {};

    for (
        FancyStrNode* Node = List->Head;
        Node;
        Node = Node->Next
    ) {
        ListPush(Scratch.MemPool, &Parts, Node->V.String);
    }

    Str8 Result = StrSkipChopWhitespace(
        StrListJoin(MemPool, &Parts, NULL)
    );

    ReleaseScratch(Scratch);

    return Result;
}

void 
DrawInit(void)
{
    Arena* MemPool = ArenaAlloc();

    DRAW_STATE = ArenaPushArrayZero(MemPool, DrawState, 1);
    DRAW_STATE->MemPool = MemPool;
}

void 
DBeginFrame(Arena* FrameMemPool, v4f32 ForegroundColour, v4f32 BackgroundColour)
{
    r2f32 ConsoleRect = GetConsoleRect();
    r2i32 RendCellRect = RendCellRectFromRect(ConsoleRect);
    v2i32 RendCellLength = Length(RendCellRect);

    RendBeginFrame(
        FrameMemPool,
        RendCellLength,
        ForegroundColour,
        BackgroundColour
    );
    DRAW_STATE->HeadClip = NULL;
    DRAW_STATE->FreeClip = NULL;
}

void 
DEndFrame(void)
{
    RendEndFrame();
}

r2f32 
DPushClip(r2f32 Clip)
{
    DClipNode* Node = DRAW_STATE->FreeClip;

    if (Node)
        SLL_STACK_POP(DRAW_STATE->FreeClip);
    else
        Node = ArenaPushArray(REND_STATE->FrameMemPool, DClipNode, 1);

    r2i32 RendCellRect = RendCellRectFromRect(Clip);

    Node->V = RendPushClip(RendCellRect);
    SLL_STACK_PUSH(DRAW_STATE->HeadClip, Node);

    r2f32 Result = DHeadClip();

    return Result;
}

r2f32 
DPopClip(void)
{
    DClipNode* Node = DRAW_STATE->HeadClip;
    r2f32 Result = DHeadClip();

    if (Node) {
        RendPopClip(Node->V);
        DRAW_STATE->HeadClip = Node->Next;
        Node->Next = DRAW_STATE->FreeClip;
        DRAW_STATE->FreeClip = Node;
    }

    return Result;
}

r2f32 
DHeadClip(void)
{
    r2i32 Clip = REND_STATE->Clip;
    r2f32 Result = Rng(
        (f32) Clip.X0,
        (f32) Clip.Y0,
        (f32) Clip.X1,
        (f32) Clip.Y1
    );

    return Result;
}

void 
DrawRect(r2f32 Destination, v4f32 Colour, f32 BorderThickness)
{
    r2i32 Rect = RendCellRectFromRect(Destination);

    if (BorderThickness > 0.0f)
        DrawBorder(Destination, Colour, BorderThickness >= 2.0f);
    else
        RendFillRect(Rect, Colour);
}

void 
DrawBorder(r2f32 Destination, v4f32 Colour, b32 IsDouble)
{
    r2i32 Rect = RendCellRectFromRect(Destination);
    i32 X0 = Rect.X0;
    i32 Y0 = Rect.Y0;
    i32 X1 = Rect.X1 - 1;
    i32 Y1 = Rect.Y1 - 1;
    RendLineMask Style = IsDouble ? REND_LINE_DOUBLE : 0;

    if (X1 < X0 || Y1 < Y0)
        return;

    if (X1 == X0 && Y1 == Y0) {
        RendPutLine(
            X0,
            Y0,
            (RendLineMask) (Style | REND_LINE_LEFT | REND_LINE_RIGHT),
            Colour
        );

        return;
    }

    for (i32 X = X0 + 1; X < X1; ++X) {
        RendPutLine(
            X,
            Y0,
            (RendLineMask) (Style | REND_LINE_LEFT | REND_LINE_RIGHT),
            Colour
        );
        RendPutLine(
            X,
            Y1,
            (RendLineMask) (Style | REND_LINE_LEFT | REND_LINE_RIGHT),
            Colour
        );
    }

    for (i32 Y = Y0 + 1; Y < Y1; ++Y) {
        RendPutLine(
            X0,
            Y,
            (RendLineMask) (Style | REND_LINE_UP | REND_LINE_DOWN),
            Colour
        );
        RendPutLine(
            X1,
            Y,
            (RendLineMask) (Style | REND_LINE_UP | REND_LINE_DOWN),
            Colour
        );
    }

    RendPutLine(
        X0,
        Y0,
        (RendLineMask) (Style | REND_LINE_RIGHT | REND_LINE_DOWN),
        Colour
    );
    RendPutLine(
        X1,
        Y0,
        (RendLineMask) (Style | REND_LINE_LEFT | REND_LINE_DOWN),
        Colour
    );
    RendPutLine(
        X0,
        Y1,
        (RendLineMask) (Style | REND_LINE_RIGHT | REND_LINE_UP),
        Colour
    );
    RendPutLine(
        X1,
        Y1,
        (RendLineMask) (Style | REND_LINE_LEFT | REND_LINE_UP),
        Colour
    );
}

void 
DrawLine(v2f32 Point0, v2f32 Point1, v4f32 Colour)
{
    i32 X0 = (i32) FloorF32(Point0.X);
    i32 Y0 = (i32) FloorF32(Point0.Y);
    i32 X1 = (i32) FloorF32(Point1.X);
    i32 Y1 = (i32) FloorF32(Point1.Y);

    if (Y0 == Y1) {
        for (i32 X = MIN(X0, X1); X < MAX(X0, X1); ++X)
            RendPutLine(X, Y0, REND_LINE_LEFT | REND_LINE_RIGHT, Colour);
    } else if (X0 == X1) {
        for (i32 Y = MIN(Y0, Y1); Y < MAX(Y0, Y1); ++Y)
            RendPutLine(X0, Y, REND_LINE_UP | REND_LINE_DOWN, Colour);
    }
}

void 
DrawBar(
    r2f32 Destination, 
    Axis2D Axis, 
    r1f32 Fill, 
    v4f32 FillColour, 
    v4f32 TrackColour
) {
    r2i32 Rect = RendCellRectFromRect(Destination);
    Axis2D CrossAxis = FLIP_AXIS(Axis);
    u32 const* Eighths = (Axis == AXIS_2D_X) 
        ? DRAW_EIGHTHS_LEFT 
        : DRAW_EIGHTHS_LOWER;
    b32 AnchorAtStart = (Axis == AXIS_2D_X);
    f32 FillMin = CLAMP(
        Destination.Point0.V[Axis], 
        Fill.Min, 
        Destination.Point1.V[Axis]
    );
    f32 FillMax = CLAMP(
        Destination.Point0.V[Axis], 
        Fill.Max, 
        Destination.Point1.V[Axis]
    );

    RendFillRect(Rect, TrackColour);

    for (
        i32 C = Rect.Point0.V[CrossAxis]; 
        C < Rect.Point1.V[CrossAxis]; 
        ++C
    ) {
        for (
            i32 I = Rect.Point0.V[Axis]; 
            I < Rect.Point1.V[Axis]; 
            ++I
        ) {
            f32 A = CLAMP(0.0f, FillMin - (f32) I, 1.0f);
            f32 B = CLAMP(0.0f, FillMax - (f32) I, 1.0f);
            v2i32 P = {};

            P.V[Axis] = I;
            P.V[CrossAxis] = C;

            if (B - A >= 0.999f) {
                RendFillRect(Rng(P, Vec(P.X + 1, P.Y + 1)), FillColour);
            } else if (B > A) {
                b32 FromStart = (A <= 0.001f);
                b32 ToEnd = (B >= 0.999f);
                b32 Complement = (
                    AnchorAtStart 
                        ? ToEnd && !FromStart 
                        : FromStart && !ToEnd
                );
                f32 Covered = Complement 
                    ? (1.0f - (B - A)) 
                    : (B - A);
                u32 Steps = (u32) (Covered * 8.0f + 0.5f);

                if (Steps == 0 || Steps == 8) {
                    if ((Steps == 8) != Complement)
                        RendFillRect(
                            Rng(
                                P, 
                                Vec(P.X + 1, P.Y + 1)
                            ),
                            FillColour
                        );
                } else if (Complement) {
                    RendPutCell(
                        P.X, 
                        P.Y, 
                        Eighths[Steps], 
                        TrackColour, 
                        FillColour, 
                        0
                    );
                } else {
                    RendPutCell(
                        P.X, 
                        P.Y, 
                        Eighths[Steps], 
                        FillColour, 
                        TrackColour, 
                        0
                    );
                }
            }
        }
    }
}

void 
DrawShadow(r2f32 Destination)
{
    r2i32 Rect = RendCellRectFromRect(Destination);
    r2i32 Right = Rng(Rect.X1, Rect.Y0 + 1, Rect.X1 + 2, Rect.Y1 + 1);
    r2i32 Bottom = Rng(Rect.X0 + 2, Rect.Y1, Rect.X1, Rect.Y1 + 1);

    RendDarkenRect(Right, 0.5f);
    RendDarkenRect(Bottom, 0.5f);
}

void 
DrawCaret(v2f32 Position)
{
    RendSetCaret(
        Vec(
            (i32) FloorF32(Position.X), 
            (i32) FloorF32(Position.Y)
        )
    );
}

void 
DrawMousePointer(v2f32 Position, CursorKind Kind)
{
    i32 X = (i32) FloorF32(Position.X);
    i32 Y = (i32) FloorF32(Position.Y);
    u32 Glyph = (Kind < CURSOR_KIND_COUNT) ? DRAW_POINTER_GLYPHS[Kind] : 0;

    if (
        Glyph && 
        InRange(
            REND_STATE->Clip, 
            Vec(X, Y)
        )
    ) {
        RendCell* Cell = GetRendCellFromXY(
            &REND_STATE->BackBuffer, 
            X, 
            Y
        );

        Cell->CodePoint = Glyph;
    }

    RendInvertCell(X, Y);
}

INTERNAL RendAttr
RendAttrsFromFancyStrFlags(FancyStrFlag Flags)
{
    return (RendAttr) (Flags & 0x3F);
}

void 
DrawFancyStrList(
    v2f32 Position, 
    f32 TabSize, 
    FancyStrList* List, 
    f32 MaxX, 
    Str8 Trailer, 
    FMRangeList* Ranges, 
    v4f32 RangeColour
) {
    i32 X0 = (i32) FloorF32(Position.X);
    i32 Y = (i32) FloorF32(Position.Y);
    i32 Limit = (i32) FloorF32(MaxX);
    u64 TabColumns = MAX((u64) TabSize, 1);
    u64 TotalColumns = (u64) DDimensionsFromFancyStrList(TabSize, List).X;
    u64 TrailerColumns = (u64) DDimensionsFromStr(TabSize, Trailer).X;
    b32 Truncate = (X0 + (i64) TotalColumns > Limit);
    i32 TextLimit = Truncate ? Limit - (i32) TrailerColumns : Limit;
    u64 Columns = 0;
    u64 ByteOffset = 0;
    FMRangeNode* Range = Ranges ? Ranges->Head : NULL;

    for (FancyStrNode* Node = List->Head; Node; Node = Node->Next) {
        Str8 String = Node->V.String;
        v4f32 Colour = Node->V.Params.Colour;
        RendAttr Attrs = RendAttrsFromFancyStrFlags(Node->V.Params.Flags);

        for (u64 Offset = 0; Offset < String.Size;) {
            UnicodeDecode Decode = UTF8Decode(String.Str + Offset, String.Size - Offset);
            u64 Inc = MAX(Decode.Inc, 1);
            i32 X = X0 + (i32) Columns;

            if (X >= TextLimit)
                goto Done;

            while (Range && Range->Range.Max <= ByteOffset)
                Range = Range->Next;

            b32 InRange = (
                Range && 
                Range->Range.Min <= ByteOffset && 
                ByteOffset < Range->Range.Max
            );

            if (Decode.CodePoint == '\t') {
                Columns += DTabAdvance(Columns, TabColumns);
            } else {
                RendPutCell(
                    X, 
                    Y, 
                    Decode.CodePoint, 
                    InRange ? RangeColour : Colour, 
                    (RendAttr) (InRange ? (Attrs | REND_ATTR_BOLD) : Attrs)
                );
                ++Columns;
            }

            Offset += Inc;
            ByteOffset += Inc;
        }
    }

Done:
    if (Truncate) {
        i32 X = MAX(X0 + (i32) Columns, TextLimit);
        v4f32 Colour = List->Tail 
            ? List->Tail->V.Params.Colour 
            : RangeColour;

        for (u64 Offset = 0; Offset < Trailer.Size;) {
            UnicodeDecode Decode = UTF8Decode(
                Trailer.Str + Offset, 
                Trailer.Size - Offset
            );

            RendPutCell(
                X++, 
                Y, 
                Decode.CodePoint, 
                Colour, 
                REND_ATTR_DIMMED
            );
            Offset += MAX(Decode.Inc, 1);
        }
    }
}

void 
DrawTruncatedFancyStrList(
    v2f32 Position, 
    f32 TabSize, 
    FancyStrList* List, 
    f32 MaxX, 
    Str8 Trailer
) {
    DrawFancyStrList(
        Position, 
        TabSize, 
        List, 
        MaxX, 
        Trailer, 
        NULL, 
        {}
    );
}

FMRangeList 
FuzzyFindFStrs(Arena* MemPool, FancyStrList* FancyStrings, Str8 Needle)
{
    Str8 Joined = {};

    Joined.Size = FancyStrings->TotalSize;
    Joined.Str = ArenaPushArrayZero(MemPool, u8, Joined.Size);

    u64 Offset = 0;

    for (
        FancyStrNode* Node = FancyStrings->Head; 
        Node; 
        Node = Node->Next
    ) {
        MemCpy(
            Joined.Str + Offset, 
            Node->V.String.Str, 
            Node->V.String.Size
        );
        Offset += Node->V.String.Size;
    }

    return FuzzyFind(MemPool, Needle, Joined);
}

void 
DrawTruncatedFancyStrListFuzzyMatches(
    v2f32 Position, 
    f32 TabSize, 
    FancyStrList* List, 
    f32 MaxX, 
    FMRangeList* Ranges, 
    v4f32 Colour
) {
    DrawFancyStrList(
        Position, 
        TabSize, 
        List, 
        MaxX, 
        "\xE2\x80\xA6"_s8, 
        Ranges, 
        Colour
    );
}
