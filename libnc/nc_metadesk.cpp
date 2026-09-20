#include "nc_types.h"
#include "nc_string.h"
#include "nc_memory.h"
#include "nc_arena.h"
#include "nc_metadesk.h"

MDNode        __EMPTY_MD_NODE_VALUE;
MDNode* const EMPTY_MD_NODE_VALUE = &__EMPTY_MD_NODE_VALUE;

void
MDInit(void)
{
    EMPTY_MD_NODE_VALUE->Next = EMPTY_MD_NODE_VALUE;
    EMPTY_MD_NODE_VALUE->Prev = EMPTY_MD_NODE_VALUE;
    EMPTY_MD_NODE_VALUE->Parent = EMPTY_MD_NODE_VALUE;
    EMPTY_MD_NODE_VALUE->Head = EMPTY_MD_NODE_VALUE;
    EMPTY_MD_NODE_VALUE->Tail = EMPTY_MD_NODE_VALUE;
    EMPTY_MD_NODE_VALUE->String = ""_s8;
    EMPTY_MD_NODE_VALUE->RawString = ""_s8;
    EMPTY_MD_NODE_VALUE->Kind = MD_NODE_KIND_NIL;
}

void 
ListPush(Arena* MemPool, MDNodePtrList* List, MDNode* Node)
{
    MDNodePtrNode* N = ArenaPushArrayZero(MemPool, MDNodePtrNode, 1);

    N->V = Node;
    SLL_QUEUE_PUSH(List->Head, List->Tail, N);
    ++List->Count;
}

void 
ListPushFront(Arena* MemPool, MDNodePtrList* List, MDNode* Node)
{
    MDNodePtrNode* N = ArenaPushArrayZero(MemPool, MDNodePtrNode, 1);

    N->V = Node;
    SLL_QUEUE_PUSH_FRONT(List->Head, List->Tail, N);
    ++List->Count;
}

void
ListPush(
    Arena* MemPool, 
    MDMessageList* Msgs, 
    MDNode* Node, 
    MDMessageKind Kind, 
    Str8 String
) {
    MDMessage* Msg = ArenaPushArrayZero(MemPool, MDMessage, 1);

    Msg->Node = Node;
    Msg->Kind = Kind;
    Msg->String = String;
    SLL_QUEUE_PUSH(Msgs->Head, Msgs->Tail, Msg);
    ++Msgs->Count;
    Msgs->WorstMessageKind = MAX(Kind, Msgs->WorstMessageKind);
}

void
ListPush(
    Arena* MemPool,
    MDMessageList* Msgs,
    MDNode* Node,
    MDMessageKind Kind,
    char* Fmt,
    ...
) {
    va_list Args;

    va_start(Args, Fmt);

    Str8 String = ArenaPushStrFmtV(MemPool, Fmt, Args);

    ListPush(MemPool, Msgs, Node, Kind, String);
    va_end(Args);
}

void
ListCat(MDMessageList* Dst, MDMessageList* List)
{
    if (List->Head) {
        if (Dst->Tail) {
            Dst->Tail->Next = List->Head;
            Dst->Tail = List->Tail;
            Dst->Count += List->Count;
            Dst->WorstMessageKind = MAX(Dst->WorstMessageKind, List->WorstMessageKind);
        } else {
            MemCpy(Dst, List, sizeof(*Dst));
        }
    }

    MemSet(List, 0, sizeof(*List));
}

MDToken
MDTokenAlloc(r1u64 Range, MDTokenKind Kind)
{
    MDToken Result = {};

    Result.Range = Range;
    Result.Kind = Kind;

    return Result;
}

#if defined(NC_LANG_CPP)
    b32
    operator==(MDToken TokenA, MDToken TokenB)
    {
        return (
            TokenA.Range.Min == TokenB.Range.Min &&
            TokenA.Range.Max == TokenB.Range.Max &&
            TokenA.Kind == TokenB.Kind
        );
    }

    b32
    operator!=(MDToken TokenA, MDToken TokenB)
    {
        return !(TokenA == TokenB);
    }
#endif

Str8List
MDStrListFromTokenKinds(Arena* MemPool, MDTokenKind Kinds)
{
    Str8List Result = {};


    if (Kinds & MD_TOKEN_KIND_IDENT)
        ListPush(MemPool, &Result, "Identifier"_s8);

    if (Kinds & MD_TOKEN_KIND_NUMERIC)
        ListPush(MemPool, &Result, "Numeric"_s8);

    if (Kinds & MD_TOKEN_KIND_STRLIT)
        ListPush(MemPool, &Result, "StringLiteral"_s8);

    if (Kinds & MD_TOKEN_KIND_SYMBOL)
        ListPush(MemPool, &Result, "Symbol"_s8);

    if (Kinds & MD_TOKEN_KIND_RESERVED)
        ListPush(MemPool, &Result, "Reserved"_s8);

    if (Kinds & MD_TOKEN_KIND_COMMENT)
        ListPush(MemPool, &Result, "Comment"_s8);

    if (Kinds & MD_TOKEN_KIND_WHITESPACE)
        ListPush(MemPool, &Result, "Whitespace"_s8);

    if (Kinds & MD_TOKEN_KIND_NEWLINE)
        ListPush(MemPool, &Result, "Newline"_s8);

    if (Kinds & MD_TOKEN_KIND_BROKEN_COMMENT)
        ListPush(MemPool, &Result, "BrokenComment"_s8);

    if (Kinds & MD_TOKEN_KIND_BROKEN_STRLIT)
        ListPush(MemPool, &Result, "BrokenStringLiteral"_s8);

    if (Kinds & MD_TOKEN_KIND_BAD_CHAR)
        ListPush(MemPool, &Result, "BadCharacter"_s8);

    return Result;
}

void
ListPush(Arena* MemPool, MDTokenChunkList* List, u64 Capacity, MDToken Token)
{
    MDTokenChunkNode* Node = List->Tail;

    if (!Node || Node->Count >= Node->Capacity) {
        Node = ArenaPushArrayZero(MemPool, MDTokenChunkNode, 1);
        Node->Capacity = Capacity;
        Node->V = ArenaPushArray(MemPool, MDToken, Capacity);
        SLL_QUEUE_PUSH(List->Head, List->Tail, Node);
        ++List->ChunkCount;
    }

    MemCpy(&Node->V[Node->Count], &Token, sizeof(Node->V[Node->Count]));
    ++Node->Count;
    ++List->TotalTokenCount;
}

MDTokenArray
MDTokenArrayFromChunkList(Arena* MemPool, MDTokenChunkList* Chunks)
{
    MDTokenArray Result = {};

    Result.Count = Chunks->TotalTokenCount;
    Result.V = ArenaPushArray(MemPool, MDToken, Result.Count);

    u64 WriteIndex = 0;

    for (MDTokenChunkNode* Node = Chunks->Head; Node; Node = Node->Next) {
        MemCpy(Result.V + WriteIndex, Node->V, sizeof(MDToken) * Node->Count);
        WriteIndex += Node->Count;
    }

    return Result;
}

Str8
MDContentStrFromTokenKind(MDTokenKind Kind, Str8 String)
{
    u64 NumChop = 0;
    u64 NumSkip = 0;

    NumSkip += 3 * !!(Kind & MD_TOKEN_KIND_STR_TRIPLET);
    NumChop += 3 * !!(Kind & MD_TOKEN_KIND_STR_TRIPLET);
    NumSkip += 1 * (!(Kind & MD_TOKEN_KIND_STR_TRIPLET) && Kind & MD_TOKEN_KIND_STRLIT);
    NumChop += 1 * (!(Kind & MD_TOKEN_KIND_STR_TRIPLET) && Kind & MD_TOKEN_KIND_STRLIT);

    Str8 Result = String;

    Result = StrChop(Result, NumChop);
    Result = StrSkip(Result, NumSkip);

    return Result;
}

MDNodeFlags
MDNodeFlagsFromTokenKind(MDTokenKind Kind)
{
    MDNodeFlags Result = 0;

    Result |= MD_NODE_FLAG_IDENT       * !!(Kind & MD_TOKEN_KIND_IDENT);
    Result |= MD_NODE_FLAG_NUMERIC     * !!(Kind & MD_TOKEN_KIND_NUMERIC);
    Result |= MD_NODE_FLAG_STRLIT      * !!(Kind & MD_TOKEN_KIND_STRLIT);
    Result |= MD_NODE_FLAG_SYMBOL      * !!(Kind & MD_TOKEN_KIND_SYMBOL);
    Result |= MD_NODE_FLAG_STR_SINGLE  * !!(Kind & MD_TOKEN_KIND_STR_SINGLE);
    Result |= MD_NODE_FLAG_STR_DOUBLE  * !!(Kind & MD_TOKEN_KIND_STR_DOUBLE);
    Result |= MD_NODE_FLAG_STR_TICK    * !!(Kind & MD_TOKEN_KIND_STR_TICK);
    Result |= MD_NODE_FLAG_STR_TRIPLET * !!(Kind & MD_TOKEN_KIND_STR_TRIPLET);

    return Result;
}

b32
MDNodeIsEmpty(MDNode* Node)
{
    return (
        !Node ||
        Node == EMPTY_MD_NODE_VALUE ||
        Node->Kind == MD_NODE_KIND_NIL
    );
}

MDNodeRecord
MDNodeRecordDepthFirst(
    MDNode* Node, 
    MDNode* SubTreeRoot, 
    u64 ChildOffset, 
    u64 SiblingOffset
) {
    MDNodeRecord Result = { EMPTY_MD_NODE_VALUE };

    if (!MDNodeIsEmpty(*MEMBER_FROM_OFFSET(MDNode**, Node, ChildOffset))) {
        Result.Next = *MEMBER_FROM_OFFSET(MDNode**, Node, ChildOffset);
        Result.PushCount = 1;
    } else {
        for (
            MDNode* P = Node; 
            !MDNodeIsEmpty(P) && P != SubTreeRoot; 
            P = P->Parent, ++Result.PopCount
        ) {
            if (!MDNodeIsEmpty(*MEMBER_FROM_OFFSET(MDNode**, P, SiblingOffset))) {
                Result.Next = *MEMBER_FROM_OFFSET(MDNode**, P, SiblingOffset);
                break;
            }
        }
    }

    return Result;
}

MDNode* 
ArenaPushMDNode(
    Arena* MemPool, 
    MDNodeKind Kind, 
    MDNodeFlags Flags, 
    Str8 String, 
    Str8 RawString, 
    u64 SrcOffset
) {
    MDNode* Node = ArenaPushArrayZero(MemPool, MDNode, 1);

    Node->Next = EMPTY_MD_NODE_VALUE; 
    Node->Prev = EMPTY_MD_NODE_VALUE;
    Node->Parent = EMPTY_MD_NODE_VALUE;
    Node->Head = EMPTY_MD_NODE_VALUE;
    Node->Tail = EMPTY_MD_NODE_VALUE;
    Node->HeadTag = EMPTY_MD_NODE_VALUE;
    Node->TailTag = EMPTY_MD_NODE_VALUE;
    Node->Kind = Kind;
    Node->Flags = Flags;
    Node->String = String;
    Node->RawString = RawString;
    Node->SrcOffset = SrcOffset;

    return Node;
}

void 
MDNodeInsertChild(MDNode* Parent, MDNode* PrevChild, MDNode* Node) 
{
    Node->Parent = Parent;
    DLL_INSERT_EX(
        EMPTY_MD_NODE_VALUE, 
        Parent->Head, 
        Parent->Tail, 
        PrevChild, 
        Node, 
        Next, 
        Prev
    );
}

void 
MDNodeInsertTag(MDNode* Parent, MDNode* PrevChild, MDNode* Node) 
{
    Node->Kind = MD_NODE_KIND_TAG;
    Node->Parent = Parent;
    DLL_INSERT_EX(
        EMPTY_MD_NODE_VALUE, 
        Parent->HeadTag, 
        Parent->TailTag, 
        PrevChild, 
        Node, 
        Next, 
        Prev
    );
}

void 
MDNodePushChild(MDNode* Parent, MDNode* Node) 
{
    Node->Parent = Parent;
    DLL_PUSH_BACK_EX(
        EMPTY_MD_NODE_VALUE, 
        Parent->Head, 
        Parent->Tail, 
        Node, 
        Next, 
        Prev
    );
}

void 
MDNodePushTag(MDNode* Parent, MDNode* Node) 
{
    Node->Kind = MD_NODE_KIND_TAG;
    Node->Parent = Parent;
    DLL_PUSH_BACK_EX(
        EMPTY_MD_NODE_VALUE, 
        Parent->HeadTag, 
        Parent->TailTag, 
        Node, 
        Next, 
        Prev
    );
}

void 
MDUnhook(MDNode* Node) 
{
    MDNode* Parent = Node->Parent;

    if (!MDNodeIsEmpty(Parent)) {
        if (Node->Kind == MD_NODE_KIND_TAG) {
            DLL_REMOVE_EX(EMPTY_MD_NODE_VALUE, Parent->HeadTag, Parent->TailTag, Node, Next, Prev);
        } else {
            DLL_REMOVE_EX(EMPTY_MD_NODE_VALUE, Parent->Head, Parent->Tail, Node, Next, Prev);
        }
        Node->Parent = EMPTY_MD_NODE_VALUE;
    }
}

MDNode* 
MDNodeFromChainStr(MDNode* Head, MDNode* End, Str8 String, StrMatchKind MatchKind) 
{
    MDNode* Result = EMPTY_MD_NODE_VALUE;

    for (MDNode* N = Head; !MDNodeIsEmpty(N) && N != End; N = N->Next) {
        if (StrMatch(N->String, String, MatchKind)) {
            Result = N;
            break;
        }
    }

    return Result;
}

MDNode* 
MDNodeFromChainIndex(MDNode* Head, MDNode* End, u64 Index) 
{
    MDNode* Result = EMPTY_MD_NODE_VALUE;
    u64 CurrentIndex = 0;

    for (MDNode* N = Head; !MDNodeIsEmpty(N) && N != End; N = N->Next, ++CurrentIndex) {
        if (Index == CurrentIndex) {
            Result = N;
            break;
        }
    }

    return Result;
}

MDNode* 
MDNodeFromChainFlags(MDNode* Head, MDNode* End, MDNodeFlags Flags) 
{
    MDNode* Result = EMPTY_MD_NODE_VALUE;

    for (MDNode* N = Head; !MDNodeIsEmpty(N) && N != End; N = N->Next) {
        if (N->Flags & Flags) {
            Result = N;
            break;
        }
    }

    return Result;
}

u64 
MDIndexFromNode(MDNode* Node) 
{
    u64 Index = 0;

    for (MDNode* N = Node->Prev; !MDNodeIsEmpty(N); N = N->Prev) {
        ++Index;
    }

    return Index;
}

MDNode* 
MDRootFromNode(MDNode* Node) 
{
    MDNode* Result = Node;

    for (
        MDNode* P = Node->Parent; 
        (P->Kind == MD_NODE_KIND_MAIN || P->Kind == MD_NODE_KIND_TAG) && !MDNodeIsEmpty(P); 
        P = P->Parent
    ) {
        Result = P;
    }

    return Result;
}

MDNode* 
MDChildFromStr(MDNode* Node, Str8 ChildString, StrMatchKind MatchKind) 
{
    return MDNodeFromChainStr(Node->Head, EMPTY_MD_NODE_VALUE, ChildString, MatchKind);
}

MDNode* 
MDTagFromStr(MDNode* Node, Str8 TagString, StrMatchKind MatchKind) 
{
    return MDNodeFromChainStr(Node->HeadTag, EMPTY_MD_NODE_VALUE, TagString, MatchKind);
}

MDNode* 
MDChildFromIndex(MDNode* Node, u64 Index) 
{
    return MDNodeFromChainIndex(Node->Head, EMPTY_MD_NODE_VALUE, Index);
}

MDNode* 
MDTagFromIndex(MDNode* Node, u64 Index) 
{
    return MDNodeFromChainIndex(Node->HeadTag, EMPTY_MD_NODE_VALUE, Index);
}

MDNode* 
MDTagArgFromIndex(MDNode* Node, Str8 TagString, StrMatchKind MatchKind, u64 Index) 
{
    MDNode* Tag = MDTagFromStr(Node, TagString, MatchKind);

    return MDChildFromIndex(Tag, Index);
}

MDNode* 
MDTagArgFromStr(
    MDNode* Node, 
    Str8 TagString, 
    StrMatchKind TagMatchKind, 
    Str8 ArgString, 
    StrMatchKind ArgMatchKind
) {
    MDNode* Tag = MDTagFromStr(Node, TagString, TagMatchKind);
    MDNode* Arg = MDChildFromStr(Tag, ArgString, ArgMatchKind);

    return Arg;
}

b32 
MDNodeHasChild(MDNode* Node, Str8 String, StrMatchKind MatchKind) 
{
    return !MDNodeIsEmpty(MDChildFromStr(Node, String, MatchKind));
}

b32 
MDNodeHasTag(MDNode* Node, Str8 String, StrMatchKind MatchKind) 
{
    return !MDNodeIsEmpty(MDTagFromStr(Node, String, MatchKind));
}

u64 
MDChildCountFromNode(MDNode* Node) 
{
    u64 Result = 0;

    for (MDNode* Child = Node->Head; !MDNodeIsEmpty(Child); Child = Child->Next) {
        ++Result;
    }

    return Result;
}

u64 
MDTagCountFromNode(MDNode* Node) 
{
    u64 Result = 0;

    for (MDNode* Tag = Node->HeadTag; !MDNodeIsEmpty(Tag); Tag = Tag->Next) {
        ++Result;
    }

    return Result;
}

Str8 
MDStrFromChildren(Arena* MemPool, MDNode* Root) 
{
    TempArena Scratch = GetScratch(&MemPool, 1);
    Str8List Strings = {};

    for EACH_MDNODE(Child, Root->Head) {
        if (Child->Flags == Child->Prev->Flags) {
            ListPush(Scratch.MemPool, &Strings, " "_s8);
        }
        ListPush(Scratch.MemPool, &Strings, Child->String);
    }

    Str8 Result = StrListJoin(MemPool, &Strings, NULL);

    ReleaseScratch(Scratch);

    return Result;
}

b32 
MDNodeMatch(MDNode* NodeA, MDNode* NodeB, StrMatchKind MatchKind) 
{
    b32 Result = FALSE;

    if (NodeA->Kind == NodeB->Kind && StrMatch(NodeA->String, NodeB->String, MatchKind)) {
        Result = (NodeA->Flags == NodeB->Flags);

        if (Result && NodeA->Kind != MD_NODE_KIND_TAG) {
            for (
                MDNode *TagA = NodeA->HeadTag, *TagB = NodeB->HeadTag;
                !MDNodeIsEmpty(TagA) || !MDNodeIsEmpty(TagB);
                TagA = TagA->Next, TagB = TagB->Next
            ) {
                if (MDNodeMatch(TagA, TagB, MatchKind)) {
                    for (
                        MDNode *ArgA = TagA->Head, *ArgB = TagB->Head;
                        !MDNodeIsEmpty(ArgA) || !MDNodeIsEmpty(ArgB);
                        ArgA = ArgA->Next, ArgB = ArgB->Next
                    ) {
                        if (!MDTreeMatch(ArgA, ArgB, MatchKind)) {
                            Result = FALSE;
                            goto end;
                        }
                    }
                } else {
                    Result = FALSE;
                    goto end;
                }
            }
        }
    }

end:
    return Result;
}

b32 
MDTreeMatch(MDNode* NodeA, MDNode* NodeB, StrMatchKind MatchKind) 
{
    b32 Result = MDNodeMatch(NodeA, NodeB, MatchKind);

    if (Result) {
        for (
            MDNode *ChildA = NodeA->Head, *ChildB = NodeB->Head;
            !MDNodeIsEmpty(ChildA) || !MDNodeIsEmpty(ChildB);
            ChildA = ChildA->Next, ChildB = ChildB->Next
        ) {
            if (!MDTreeMatch(ChildA, ChildB, MatchKind)) {
                Result = FALSE;
                goto end;
            }
        }
    }

end:
    return Result;
}

MDNode* 
MDCopyTree(Arena* MemPool, MDNode* Root) 
{
    MDNode* DstRoot = EMPTY_MD_NODE_VALUE;
    MDNode* DstParent = DstRoot;
    MDNodeRecord Record = {};

    for (MDNode* Src = Root; !MDNodeIsEmpty(Src); Src = Record.Next) {
        MDNode* Dst = ArenaPushArray(MemPool, MDNode, 1);

        Dst->Head = Dst->Tail = Dst->Parent = Dst->Next = Dst->Prev = EMPTY_MD_NODE_VALUE;
        Dst->HeadTag = Dst->TailTag = EMPTY_MD_NODE_VALUE;
        Dst->Kind = Src->Kind;
        Dst->Flags = Src->Flags;
        Dst->String = ArenaPushStrCpy(MemPool, Src->String);
        Dst->RawString = ArenaPushStrCpy(MemPool, Src->RawString);
        Dst->SrcOffset = Src->SrcOffset;
        Dst->Parent = DstParent;

        if (!MDNodeIsEmpty(DstParent)) {
            DLL_PUSH_BACK_EX(EMPTY_MD_NODE_VALUE, DstParent->Head, DstParent->Tail, Dst, Next, Prev);
        } else {
            DstRoot = DstParent = Dst;
        }

        Record = MDNodeRecordDepthFirstPreOrder(Src, Root);

        if (Record.PushCount != 0) {
            DstParent = Dst;
        } else {
            for (u64 Index = 0; Index < Record.PopCount; ++Index) {
                DstParent = DstParent->Parent;
            }
        }
    }

    return DstRoot;
}

MDTokeniseResult 
MDTokeniseFromText(Arena* MemPool, Str8 Text) 
{
    TempArena Scratch = GetScratch(&MemPool, 1);
    MDTokenChunkList Tokens = {};
    MDMessageList Messages = {};
    u8* ByteFirst = Text.Str;
    u8* ByteOpl = ByteFirst + Text.Size;
    u8* Byte = ByteFirst;

    for (; Byte < ByteOpl; ) {
        MDTokenKind TokenKind = 0;
        u8* TokenStart = NULL;
        u8* TokenOpl = NULL;

        if (
            TokenKind == 0 && 
            (
                *Byte == ' ' || 
                *Byte == '\t' || 
                *Byte == '\v' || 
                *Byte == '\r'
            )
        ) {
            TokenKind = MD_TOKEN_KIND_WHITESPACE;
            TokenStart = Byte;
            TokenOpl = Byte;
            ++Byte;

            for (; Byte <= ByteOpl; ++Byte) {
                TokenOpl += 1;
                if (
                    Byte == ByteOpl || 
                    (
                        *Byte != ' ' && 
                        *Byte != '\t' && 
                        *Byte != '\v' && 
                        *Byte != '\r'
                    )
                ) {
                    break;
                }
            }
        }

        if (TokenKind == 0 && *Byte == '\n') {
            TokenKind = MD_TOKEN_KIND_NEWLINE;
            TokenStart = Byte;
            TokenOpl = Byte + 1;
            Byte += 1;
        }

        if (
            TokenKind == 0 && 
            (
                Byte + 1 < ByteOpl && 
                *Byte == '/' && 
                Byte[1] == '/'
            )
        ) {
            TokenKind = MD_TOKEN_KIND_COMMENT;
            TokenStart = Byte;
            TokenOpl = Byte + 2;
            Byte += 2;

            b32 Escaped = FALSE;

            for (; Byte <= ByteOpl; ++Byte) {
                TokenOpl += 1;

                if (Byte == ByteOpl)
                    break;

                if (Escaped) {
                    Escaped = FALSE;
                } else {
                    if (*Byte == '\n') 
                        break;
                    else if (*Byte == '\\') 
                        Escaped = TRUE;
                }
            }
        }

        if (
            TokenKind == 0 && 
            (
                Byte + 1 < ByteOpl && 
                *Byte == '/' && 
                Byte[1] == '*'
            )
        ) {
            TokenKind = MD_TOKEN_KIND_COMMENT;
            TokenStart = Byte;
            TokenOpl = Byte + 2;
            Byte += 2;

            for (; Byte <= ByteOpl; ++Byte) {
                TokenOpl += 1;

                if (Byte == ByteOpl) {
                    TokenKind |= MD_TOKEN_KIND_BROKEN_COMMENT;
                    break;
                }

                if (
                    Byte + 1 < ByteOpl && 
                    Byte[0] == '*' && 
                    Byte[1] == '/'
                ) {
                    TokenOpl += 2;
                    break;
                }
            }
        }

        if (
            TokenKind == 0 && 
            (
                ('A' <= *Byte && *Byte <= 'Z') || 
                ('a' <= *Byte && *Byte <= 'z') || 
                *Byte == '_' || 
                UTF8_CLASS[*Byte >> 3] >= 2
            )
        ) {
            TokenKind = MD_TOKEN_KIND_IDENT;
            TokenStart = Byte;
            TokenOpl = Byte;
            Byte += 1;

            for (; Byte <= ByteOpl; ++Byte) {
                TokenOpl += 1;
                if (
                    Byte == ByteOpl || 
                    (
                        !('A' <= *Byte && *Byte <= 'Z') && 
                        !('a' <= *Byte && *Byte <= 'z') && 
                        !('0' <= *Byte && *Byte <= '9') && 
                        *Byte != '_' && 
                        UTF8_CLASS[*Byte >> 3] < 2
                    )
                ) {
                    break;
                }
            }
        }

        if (
            TokenKind == 0 && 
            (
                ('0' <= *Byte && *Byte <= '9') || 
                (*Byte == '.' && Byte + 1 < ByteOpl && '0' <= Byte[1] && Byte[1] <= '9') || 
                (*Byte == '-' && Byte + 1 < ByteOpl && '0' <= Byte[1] && Byte[1] <= '9') || 
                *Byte == '_'
            )
        ) {
            TokenKind = MD_TOKEN_KIND_NUMERIC;
            TokenStart = Byte;
            TokenOpl = Byte;
            ++Byte;

            for (; Byte <= ByteOpl; ++Byte) {
                TokenOpl += 1;

                if (
                    Byte == ByteOpl || 
                    (
                        !('A' <= *Byte && *Byte <= 'Z') && 
                        !('a' <= *Byte && *Byte <= 'z') && 
                        !('0' <= *Byte && *Byte <= '9') && 
                        *Byte != '_' && 
                        *Byte != '.'
                    )
                ) {
                    break;
                }
            }
        }

        if (
            TokenKind == 0 && 
            Byte + 2 < ByteOpl && 
            (
                (Byte[0] == '"' && Byte[1] == '"' && Byte[2] == '"') || 
                (Byte[0] == '\'' && Byte[1] == '\'' && Byte[2] == '\'') || 
                (Byte[0] == '`' && Byte[1] == '`' && Byte[2] == '`')
            )
        ) {
            u8 Style = Byte[0];

            TokenKind = MD_TOKEN_KIND_STRLIT | MD_TOKEN_KIND_STR_TRIPLET;
            TokenKind |= (Style == '\'') * MD_TOKEN_KIND_STR_SINGLE;
            TokenKind |= (Style == '"') * MD_TOKEN_KIND_STR_DOUBLE;
            TokenKind |= (Style == '`') * MD_TOKEN_KIND_STR_TICK;
            TokenStart = Byte;
            TokenOpl = Byte + 3;
            Byte += 3;

            for (; Byte <= ByteOpl; ++Byte) {
                if (Byte == ByteOpl) {
                    TokenKind |= MD_TOKEN_KIND_BROKEN_STRLIT;
                    TokenOpl = Byte;
                    break;
                }

                if (
                    Byte + 2 < ByteOpl &&
                    (
                        Byte[0] == Style && 
                        Byte[1] == Style && 
                        Byte[2] == Style
                    )
                ) {
                    Byte += 3;
                    TokenOpl = Byte;
                    break;
                }
            }
        }

        if (TokenKind == 0 && (Byte[0] == '"' || Byte[0] == '\'' || Byte[0] == '`')) {
            u8 Style = Byte[0];

            TokenKind = MD_TOKEN_KIND_STRLIT;
            TokenKind |= (Style == '\'') * MD_TOKEN_KIND_STR_SINGLE;
            TokenKind |= (Style == '"') * MD_TOKEN_KIND_STR_DOUBLE;
            TokenKind |= (Style == '`') * MD_TOKEN_KIND_STR_TICK;
            TokenStart = Byte;
            TokenOpl = Byte + 1;
            ++Byte;

            b32 Escaped = FALSE;

            for (; Byte <= ByteOpl; ++Byte) {
                if (Byte == ByteOpl || *Byte == '\n') {
                    TokenOpl = Byte;
                    TokenKind |= MD_TOKEN_KIND_BROKEN_STRLIT;
                    break;
                }

                if (!Escaped && Byte[0] == '\\') {
                    Escaped = TRUE;
                } else if (!Escaped && Byte[0] == Style) {
                    TokenOpl = Byte + 1;
                    Byte += 1;
                    break;
                } else if (Escaped) {
                    Escaped = FALSE;
                }
            }
        }

        if (
            TokenKind == 0 && 
            (
                *Byte == '~' || 
                *Byte == '!' || 
                *Byte == '$' || 
                *Byte == '%' || 
                *Byte == '^' || 
                *Byte == '&' || 
                *Byte == '*' || 
                *Byte == '-' || 
                *Byte == '=' || 
                *Byte == '+' || 
                *Byte == '<' || 
                *Byte == '.' || 
                *Byte == '>' || 
                *Byte == '/' || 
                *Byte == '?' || 
                *Byte == '|'
            )
        ) {
            TokenKind = MD_TOKEN_KIND_SYMBOL;
            TokenStart = Byte;
            TokenOpl = Byte;
            ++Byte;
            for (; Byte <= ByteOpl; ++Byte) {
                ++TokenOpl;

                if (
                    Byte == ByteOpl || 
                    (
                        *Byte != '~' && 
                        *Byte != '!' && 
                        *Byte != '$' && 
                        *Byte != '%' && 
                        *Byte != '^' && 
                        *Byte != '&' && 
                        *Byte != '*' && 
                        *Byte != '-' && 
                        *Byte != '=' && 
                        *Byte != '+' && 
                        *Byte != '<' && 
                        *Byte != '.' && 
                        *Byte != '>' && 
                        *Byte != '/' && 
                        *Byte != '?' && 
                        *Byte != '|'
                    )
                ) {
                    break;
                }
            }
        }

        if (
            TokenKind == 0 && 
            (
                *Byte == '{' || 
                *Byte == '}' || 
                *Byte == '(' || 
                *Byte == ')' || 
                *Byte == '[' || 
                *Byte == ']' || 
                *Byte == '#' || 
                *Byte == ',' || 
                *Byte == '\\' || 
                *Byte == ':' || 
                *Byte == ';' || 
                *Byte == '@'
            )
        ) {
            TokenKind = MD_TOKEN_KIND_RESERVED;
            TokenStart = Byte;
            TokenOpl = Byte + 1;
            ++Byte;
        }

        if (TokenKind == 0) {
            TokenKind = MD_TOKEN_KIND_BAD_CHAR;
            TokenStart = Byte;
            TokenOpl = Byte + 1;
            ++Byte;
        }

        if (TokenKind != 0 && TokenStart != NULL && TokenOpl > TokenStart) {
            MDToken Token = {
                {
                    (u64) (TokenStart - ByteFirst), 
                    (u64) (TokenOpl - ByteFirst)
                },
                TokenKind
            };

            ListPush(Scratch.MemPool, &Tokens, 4096, Token);
        }

        if (TokenKind & MD_TOKEN_KIND_BROKEN_COMMENT) {
            MDNode* Error = ArenaPushMDNode(
                MemPool, 
                MD_NODE_KIND_ERROR_MARKER, 
                0, 
                ""_s8, 
                ""_s8, 
                TokenStart - ByteFirst
            );

            ListPush(
                MemPool, 
                &Messages, 
                Error, 
                MD_MESSAGE_KIND_ERROR, 
                "Unterminated comment."_s8
            );
        }

        if (TokenKind & MD_TOKEN_KIND_BROKEN_STRLIT) {
            MDNode* Error = ArenaPushMDNode(
                MemPool, 
                MD_NODE_KIND_ERROR_MARKER, 
                0, 
                ""_s8, 
                ""_s8, 
                TokenStart - ByteFirst
            );

            ListPush(
                MemPool, 
                &Messages, 
                Error, 
                MD_MESSAGE_KIND_ERROR, 
                "Unterminated string literal."_s8
            );
        }
    }

    MDTokeniseResult Result = {};

    Result.Tokens = MDTokenArrayFromChunkList(MemPool, &Tokens);
    Result.Messages = Messages;

    ReleaseScratch(Scratch);

    return Result;
}

enum MDParseWorkKind {
    MD_PARSE_WORK_KIND_MAIN,
    MD_PARSE_WORK_KIND_MAIN_IMPLICIT,
    MD_PARSE_WORK_KIND_NODE_OPTIONAL_FOLLOW_UP,
    MD_PARSE_WORK_KIND_NODE_CHILDREN_STYLE_SCAN,
};

struct MDParseWorkNode {
    MDParseWorkNode* Next;
    MDParseWorkKind  Kind;
    MDNode*          Parent;
    MDNode*          HeadTag;
    MDNode*          TailTag;
    MDNodeFlags      GatheredFlags;
    i32              CountedNewlines;
};

#define MDParseWorkPush(X, Y) do {                                                         \
    MDParseWorkNode* WorkNode = FreeWork;                                                  \
    if (!WorkNode) { WorkNode = ArenaPushArrayZero(Scratch.MemPool, MDParseWorkNode, 1); } \
    else { SLL_STACK_POP(FreeWork); }                                                      \
    WorkNode->Kind = (X);                                                                  \
    WorkNode->Parent = (Y);                                                                \
    WorkNode->HeadTag = EMPTY_MD_NODE_VALUE;                                               \
    WorkNode->TailTag = EMPTY_MD_NODE_VALUE;                                               \
    SLL_STACK_PUSH(HeadWork, WorkNode);                                                    \
} while (FALSE)

#define MDParseWorkPop() do {                  \
    SLL_STACK_POP(HeadWork);                   \
    if (!HeadWork) { HeadWork = &BrokenWork; } \
} while (FALSE)

MDParseResult 
MDParseFromTextTokens(Arena* MemPool, Str8 FileName, Str8 Text, MDTokenArray Tokens) 
{
    TempArena Scratch = GetScratch(&MemPool, 1);
    MDMessageList Messages = {};
    MDNode* Root = ArenaPushMDNode(
        MemPool, 
        MD_NODE_KIND_FILE, 
        0, 
        FileName, 
        Text, 
        0
    );
    MDParseWorkNode StartWork = {
        NULL,
        MD_PARSE_WORK_KIND_MAIN,
        Root,
        EMPTY_MD_NODE_VALUE,
        EMPTY_MD_NODE_VALUE
    };
    MDParseWorkNode BrokenWork = {
        NULL,
        MD_PARSE_WORK_KIND_MAIN,
        Root,
        EMPTY_MD_NODE_VALUE,
        EMPTY_MD_NODE_VALUE
    };
    MDParseWorkNode* HeadWork = &StartWork;
    MDParseWorkNode* FreeWork = NULL;
    MDToken* StartTokens = Tokens.V;
    MDToken* EndTokens = StartTokens + Tokens.Count;
    MDToken* Token = StartTokens;

    for ( ; Token < EndTokens; ) {
        Str8 TokenString = StrSub(Text, Token[0].Range);

        if (Token->Kind & MD_TOKEN_KIND_WHITESPACE) {
            ++Token;
            goto END_CONSUME;
        }

        if (Token->Kind & MD_TOKEN_GROUP_COMMENT) {
            ++Token;
            goto END_CONSUME;
        }

        if (
            HeadWork->Kind == MD_PARSE_WORK_KIND_NODE_OPTIONAL_FOLLOW_UP &&
            StrMatch(TokenString, ":"_s8, 0)
        ) {
            MDNode* Parent = HeadWork->Parent;

            MDParseWorkPop();
            MDParseWorkPush(MD_PARSE_WORK_KIND_NODE_CHILDREN_STYLE_SCAN, Parent);
            ++Token;
            goto END_CONSUME;
        }

        if (HeadWork->Kind == MD_PARSE_WORK_KIND_NODE_OPTIONAL_FOLLOW_UP) {
            MDParseWorkPop();
            goto END_CONSUME;
        }

        if (
            HeadWork->Kind == MD_PARSE_WORK_KIND_MAIN &&
            Token->Kind & MD_TOKEN_KIND_RESERVED &&
            (
                StrMatch(TokenString, ","_s8, 0) ||
                StrMatch(TokenString, ";"_s8, 0)
            )
        ) {
            MDNode* Parent = HeadWork->Parent;

            if (!MDNodeIsEmpty(Parent->Tail)) {
                Parent->Tail->Flags |= MD_NODE_FLAG_IS_BEFORE_COMMA * !!StrMatch(TokenString, ","_s8, 0);
                Parent->Tail->Flags |= MD_NODE_FLAG_IS_BEFORE_SEMICOLON * !!StrMatch(TokenString , ";"_s8, 0);
                HeadWork->GatheredFlags |= MD_NODE_FLAG_IS_AFTER_COMMA * !!StrMatch(TokenString, ","_s8, 0);
                HeadWork->GatheredFlags |= MD_NODE_FLAG_IS_AFTER_SEMICOLON * !!StrMatch(TokenString, ";"_s8, 0);
            }

            ++Token;
            goto END_CONSUME;
        }

        if (
            HeadWork->Kind == MD_PARSE_WORK_KIND_MAIN_IMPLICIT &&
            Token->Kind & MD_TOKEN_KIND_RESERVED &&
            (
                StrMatch(TokenString, ","_s8, 0) ||
                StrMatch(TokenString, ";"_s8, 0)
            )
        ) {
            MDParseWorkPop();
            goto END_CONSUME;
        }

        if (
            (
                HeadWork->Kind == MD_PARSE_WORK_KIND_MAIN ||
                HeadWork->Kind == MD_PARSE_WORK_KIND_MAIN_IMPLICIT
            ) &&
            Token->Kind & MD_TOKEN_KIND_RESERVED &&
            (
                StrMatch(TokenString, "#"_s8, 0) ||
                StrMatch(TokenString, "\\"_s8, 0) ||
                StrMatch(TokenString, ":"_s8, 0)
            )
        ) {
            MDNode* Error = ArenaPushMDNode(
                MemPool, 
                MD_NODE_KIND_ERROR_MARKER, 
                0, 
                TokenString, 
                TokenString, 
                Token->Range.Min
            );
            Str8 ErrorString = ArenaPushStrFmt(
                MemPool,
                "Unexpected reserved symbol \"%S\".",
                PRINT_STR(TokenString)
            );

            ListPush(MemPool, &Messages, Error, MD_MESSAGE_KIND_ERROR, ErrorString);
            ++Token;
            goto END_CONSUME;
        }

        if (
            (
                HeadWork->Kind == MD_PARSE_WORK_KIND_MAIN ||
                HeadWork->Kind == MD_PARSE_WORK_KIND_MAIN_IMPLICIT
            ) &&
            Token[0].Kind & MD_TOKEN_KIND_RESERVED &&
            StrMatch(TokenString, "@"_s8, 0)
        ) {
            if (Token + 1 >= EndTokens || !(Token[1].Kind & MD_TOKEN_GROUP_LABEL)) {
                MDNode* Error = ArenaPushMDNode(
                    MemPool,
                    MD_NODE_KIND_ERROR_MARKER,
                    0,
                    TokenString,
                    TokenString,
                    Token->Range.Min
                );
                Str8 ErrorString = "Tag label expected after `@` symbol."_s8;

                ListPush(
                    MemPool,
                    &Messages,
                    Error,
                    MD_MESSAGE_KIND_ERROR,
                    ErrorString
                );
                ++Token;
                goto END_CONSUME;
            } else {
                Str8 TagNameRaw = StrSub(Text, Token[1].Range);
                Str8 TagName = MDContentStrFromTokenKind(Token[1].Kind, TagNameRaw);
                MDNode* Node = ArenaPushMDNode(
                    MemPool,
                    MD_NODE_KIND_TAG,
                    MDNodeFlagsFromTokenKind(Token[1].Kind),
                    TagName,
                    TagName,
                    Token[0].Range.Min
                );

                DLL_PUSH_BACK_EX(
                    EMPTY_MD_NODE_VALUE,
                    HeadWork->HeadTag,
                    HeadWork->TailTag,
                    Node,
                    Next,
                    Prev
                );

                if (
                    Token + 2 < EndTokens && 
                    Token[2].Kind & MD_TOKEN_KIND_RESERVED &&
                    (
                        StrMatch(StrSub(Text, Token[2].Range), "("_s8, 0) ||
                        StrMatch(StrSub(Text, Token[2].Range), "["_s8, 0) ||
                        StrMatch(StrSub(Text, Token[2].Range), "{"_s8, 0)
                    )
                ) {
                    Token += 3;
                    MDParseWorkPush(MD_PARSE_WORK_KIND_MAIN, Node);
                } else {
                    Token += 2;
                }

                goto END_CONSUME;
            }
        }

        if (
            (
                HeadWork->Kind == MD_PARSE_WORK_KIND_MAIN ||
                HeadWork->Kind == MD_PARSE_WORK_KIND_MAIN_IMPLICIT
            ) &&
            Token->Kind & MD_TOKEN_GROUP_LABEL
        ) {
            Str8 NodeStringRaw = TokenString;
            Str8 NodeString = MDContentStrFromTokenKind(Token->Kind, NodeStringRaw);
            MDNodeFlags Flags = MDNodeFlagsFromTokenKind(Token->Kind) | HeadWork->GatheredFlags;

            HeadWork->GatheredFlags = 0;

            MDNode* Node = ArenaPushMDNode(
                MemPool,
                MD_NODE_KIND_MAIN,
                Flags,
                NodeString,
                NodeStringRaw,
                Token[0].Range.Min
            );

            Node->HeadTag = HeadWork->HeadTag;
            Node->TailTag = HeadWork->TailTag;

            for (MDNode* Tag = HeadWork->HeadTag; !MDNodeIsEmpty(Tag); Tag = Tag->Next)
                Tag->Parent = Node;

            HeadWork->HeadTag = EMPTY_MD_NODE_VALUE;
            HeadWork->TailTag = EMPTY_MD_NODE_VALUE;
            MDNodePushChild(HeadWork->Parent, Node);
            MDParseWorkPush(MD_PARSE_WORK_KIND_NODE_OPTIONAL_FOLLOW_UP, Node);
            ++Token;
            goto END_CONSUME;
        }

        if (
            HeadWork->Kind == MD_PARSE_WORK_KIND_MAIN &&
            Token->Kind & MD_TOKEN_KIND_RESERVED &&
            (
                StrMatch(TokenString, "{"_s8, 0) ||
                StrMatch(TokenString, "["_s8, 0) ||
                StrMatch(TokenString, "("_s8, 0)
            )
        ) {
            MDNodeFlags Flags = MDNodeFlagsFromTokenKind(Token->Kind) | HeadWork->GatheredFlags;

            Flags |= MD_NODE_FLAG_HAS_BRACE_LEFT * !!StrMatch(TokenString, "{"_s8, 0);
            Flags |= MD_NODE_FLAG_HAS_BRACK_LEFT * !!StrMatch(TokenString, "["_s8, 0);
            Flags |= MD_NODE_FLAG_HAS_PAREN_LEFT * !!StrMatch(TokenString, "("_s8, 0);
            HeadWork->GatheredFlags = 0;

            MDNode* Node = ArenaPushMDNode(
                MemPool, 
                MD_NODE_KIND_MAIN, 
                Flags,
                ""_s8,
                ""_s8,
                Token[0].Range.Min
            );

            Node->HeadTag = HeadWork->HeadTag;
            Node->TailTag = HeadWork->TailTag;

            for (MDNode* Tag = HeadWork->HeadTag; !MDNodeIsEmpty(Tag); Tag = Tag->Next)
                Tag->Parent = Node;

            HeadWork->HeadTag = EMPTY_MD_NODE_VALUE;
            HeadWork->TailTag = EMPTY_MD_NODE_VALUE;
            MDNodePushChild(HeadWork->Parent, Node);
            MDParseWorkPush(MD_PARSE_WORK_KIND_MAIN, Node);
            ++Token;
            goto END_CONSUME;
        }

        if (
            HeadWork->Kind == MD_PARSE_WORK_KIND_NODE_CHILDREN_STYLE_SCAN &&
            Token->Kind & MD_TOKEN_KIND_RESERVED &&
            (
                StrMatch(TokenString, "{"_s8, 0) ||
                StrMatch(TokenString, "["_s8, 0) ||
                StrMatch(TokenString, "("_s8, 0)
            )
        ) {
            MDNode* Parent = HeadWork->Parent;

            Parent->Flags |= MD_NODE_FLAG_HAS_BRACE_LEFT * !!StrMatch(TokenString, "{"_s8, 0);
            Parent->Flags |= MD_NODE_FLAG_HAS_BRACK_LEFT * !!StrMatch(TokenString, "["_s8, 0);
            Parent->Flags |= MD_NODE_FLAG_HAS_PAREN_LEFT * !!StrMatch(TokenString, "("_s8, 0);
            MDParseWorkPop();
            MDParseWorkPush(MD_PARSE_WORK_KIND_MAIN, Parent);
            ++Token;
            goto END_CONSUME;
        }

        if (
            HeadWork->Kind == MD_PARSE_WORK_KIND_NODE_CHILDREN_STYLE_SCAN &&
            Token->Kind & MD_TOKEN_KIND_NEWLINE
        ) {
            ++HeadWork->CountedNewlines;
            ++Token;
            goto END_CONSUME;
        }

        if (
            HeadWork->Kind == MD_PARSE_WORK_KIND_MAIN_IMPLICIT && 
            Token->Kind & MD_TOKEN_KIND_NEWLINE
        ) {
            for ( ; HeadWork->Kind == MD_PARSE_WORK_KIND_MAIN_IMPLICIT; )
                MDParseWorkPop();

            ++Token;
            goto END_CONSUME;
        }

        if (
            HeadWork->Kind != MD_PARSE_WORK_KIND_MAIN_IMPLICIT &&
            Token->Kind & MD_TOKEN_KIND_NEWLINE
        ) {
            ++Token;
            goto END_CONSUME;
        }

        if (HeadWork->Kind == MD_PARSE_WORK_KIND_NODE_CHILDREN_STYLE_SCAN) {
            if (HeadWork->CountedNewlines >= 2) {
                MDNode* Node = HeadWork->Parent;
                MDNode* Error = ArenaPushMDNode(
                    MemPool,
                    MD_NODE_KIND_ERROR_MARKER,
                    0,
                    TokenString,
                    TokenString,
                    Token->Range.Min
                );
                Str8 ErrorString = ArenaPushStrFmt(
                    MemPool,
                    "More than two newlines following \"%S\", which has implicitly-delimited children, resulting in an empty list of children.",
                    PRINT_STR(Node->String)
                );
                ListPush(MemPool, &Messages, Error, MD_MESSAGE_KIND_ERROR, ErrorString);
                MDParseWorkPop();
            } else {
                MDNode* Parent = HeadWork->Parent;

                MDParseWorkPop();
                MDParseWorkPush(MD_PARSE_WORK_KIND_MAIN_IMPLICIT, Parent);
            }

            goto END_CONSUME;
        }

        if (
            HeadWork->Kind == MD_PARSE_WORK_KIND_MAIN &&
            Token->Kind & MD_TOKEN_KIND_RESERVED &&
            (
                StrMatch(TokenString, "}"_s8, 0) ||
                StrMatch(TokenString, "]"_s8, 0) ||
                StrMatch(TokenString, ")"_s8, 0)
            )
        ) {
            MDNode* Parent = HeadWork->Parent;

            Parent->Flags |= MD_NODE_FLAG_HAS_BRACE_RIGHT * !!StrMatch(TokenString, "}"_s8, 0);
            Parent->Flags |= MD_NODE_FLAG_HAS_BRACK_RIGHT * !!StrMatch(TokenString, "]"_s8, 0);
            Parent->Flags |= MD_NODE_FLAG_HAS_PAREN_RIGHT * !!StrMatch(TokenString, ")"_s8, 0);
            MDParseWorkPop();
            ++Token;
            goto END_CONSUME;
        }

        if (
            HeadWork->Kind == MD_PARSE_WORK_KIND_MAIN_IMPLICIT &&
            Token->Kind & MD_TOKEN_KIND_RESERVED &&
            (
                StrMatch(TokenString, "}"_s8, 0) ||
                StrMatch(TokenString, "]"_s8, 0) ||
                StrMatch(TokenString, ")"_s8, 0)
            )
        ) {
            MDParseWorkPop();
            goto END_CONSUME;
        }

        {
            MDNode* Error = ArenaPushMDNode(
                MemPool,
                MD_NODE_KIND_ERROR_MARKER,
                0,
                TokenString,
                TokenString,
                Token->Range.Min
            );
            Str8 ErrorString = ArenaPushStrFmt(
                MemPool,
                "Unexpected \"%S\" token.",
                PRINT_STR(TokenString)
            );

            ListPush(
                MemPool,
                &Messages,
                Error,
                MD_MESSAGE_KIND_ERROR,
                ErrorString
            );
            ++Token;
        }

END_CONSUME:;
    }

    MDParseResult Result = {};

    Result.Root = Root;
    Result.Messages = Messages;
    ReleaseScratch(Scratch);

    return Result;
}

MDParseResult 
MDParseFromText(Arena* MemPool, Str8 FileName, Str8 Text) 
{
    TempArena Scratch = GetScratch(&MemPool, 1);
    MDTokeniseResult Tokenise = MDTokeniseFromText(Scratch.MemPool, Text);
    MDParseResult Parse = MDParseFromTextTokens(MemPool, FileName, Text, Tokenise.Tokens);

    ReleaseScratch(Scratch);

    return Parse;
}

Str8List 
MDDebugStrListFromTree(Arena* MemPool, MDNode* Root) 
{
    Str8List Strings = {};
    char* Indentation = "                                                                                                                                ";
    i32 Depth = 0;

    for (MDNode *Node = Root, *Next = EMPTY_MD_NODE_VALUE; !MDNodeIsEmpty(Node); Node = Next) {
        MDNodeRecord Record = MDNodeRecordDepthFirstPreOrder(Node, Root);

        Next = Record.Next;

        Str8 KindStr = "Unknown"_s8;

        switch (Node->Kind) {
            default: {} break;

            case MD_NODE_KIND_FILE: {
                KindStr = "File"_s8; 
            } break;

            case MD_NODE_KIND_ERROR_MARKER: {
                KindStr = "ErrorMarker"_s8; 
            } break;

            case MD_NODE_KIND_MAIN: {
                KindStr = "Main"_s8; 
            } break;

            case MD_NODE_KIND_TAG: {
                KindStr = "Tag"_s8; 
            } break;

            case MD_NODE_KIND_LIST:{
                KindStr = "List"_s8;
            } break;

            case MD_NODE_KIND_REFERENCE:{
                KindStr = "Reference"_s8;
            } break;
        }

        ListPushFmt(MemPool, &Strings, "%S\"%S\" : %S", Depth, Indentation, PRINT_STR(Node->String), PRINT_STR(KindStr));

        if (Record.PushCount != 0) {
            ListPushFmt(MemPool, &Strings, "%S{", Depth, Indentation);
        }

        Depth += Record.PushCount;

        for (i32 PopIndex = 0; PopIndex < Record.PopCount; ++PopIndex) {
            ListPushFmt(MemPool, &Strings, "%S}", Depth - 1 - PopIndex, Indentation);
        }

        Depth -= Record.PopCount;
    }

    return Strings;
}
