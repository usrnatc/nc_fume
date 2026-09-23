#include "nc_draw.h"
#include "nc_arena.h"
#include "nc_memory.h"

DrawState* DRAW_STATE = NULL;

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
                ? TabColumns - (Columns % TabColumns) 
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
            ? TabColumns - (Columns % TabColumns) 
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
            u64 LineEnd = (BreakOffset > LineStart) ? BreakOffset : Offset;
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

u64 
DCharPositionFromStr(f32 TabSize, Str8 String, f32 X)
{
    u64 Columns = 0;
    u64 TabColumns = MAX((u64) TabSize, 1);
    u64 Result = 0;

    while (Result < String.Size) {
        UnicodeDecode Decode = UTF8Decode(
            String.Str + Result, 
            String.Size - Result
        );

        Columns += (Decode.CodePoint == '\t') 
            ? TabColumns - (Columns % TabColumns) 
            : 1;

        if (X < (f32) Columns)
            break;

        Result += MAX(Decode.Inc, 1);
    }

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

// void 
// ListPush(
//     Arena* MemPool, 
//     FancyStrList* List, 
//     FancyStrParams* Params, 
//     FancyStrParams* Overrides, 
//     Str8 String
// ) {
//     FancyStr FString = {};

//     FString.String = String;
//     FString.Params = *Params;

//     if (Overrides) {
//         if (Overrides->Font != EMPTY_FONT_TAG_VALUE)
//             FString.Params.Font = Overrides->Font;

//         if (Overrides->RasterKind != 0)
//             FString.Params.RasterKind = Overrides->RasterKind;

//         if (
//             Overrides->Colour.X != 0.0f ||
//             Overrides->Colour.Y != 0.0f ||
//             Overrides->Colour.Z != 0.0f ||
//             Overrides->Colour.W != 0.0f
//         ) {
//             FString.Params.Colour = Overrides->Colour;
//         }

//         if (Overrides->Size > 0.0f)
//             FString.Params.Size = Overrides->Size;

//         if (Overrides->UnderlineThickness > 0.0f)
//             FString.Params.UnderlineThickness = Overrides->UnderlineThickness;

//         if (Overrides->StrikethroughThickness > 0.0f)
//             FString.Params.StrikethroughThickness = Overrides->StrikethroughThickness;
//     }

//     ListPush(MemPool, List, &FString);
// }

// void 
// ListCat(FancyStrList* Dst, FancyStrList* Src)
// {
//     if (Dst->Tail && Src->Head) {
//         Dst->Tail->Next = Src->Head;
//         Dst->Tail = Src->Tail;
//         Dst->TotalSize += Src->TotalSize;
//         Dst->NodeCount += Src->NodeCount;
//     } else if (Src->Head) {
//         MemCpy(Dst, Src, sizeof(*Dst));
//     }

//     MemSet(Src, 0, sizeof(*Src));
// }

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
    ThreadLocalStorage* TLS = GetTLS();
    Str8 Result = {};
    TempArena Scratch = GetScratch(&MemPool, 1);
    Str8List Parts = {};

    for (FancyStrNode* Node = List->Head; Node; Node = Node->Next)
        ListPush(Scratch.MemPool, &Parts, Node->V.String);

    Result = StrListJoin(MemPool, &Parts, NULL);
    Result = StrSkipChopWhitespace(Result);
    ReleaseScratch(Scratch);

    return Result;
}
