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
    }
}

r2f32 
DHeadClip(void)
{}
