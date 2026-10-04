#include "nc_types.h"
#include "nc_string.h"
#include "nc_config.h"
#include "nc_metadesk.h"
#include "nc_tls.h"
#include "nc_memory.h"

ConfigNode             __EMPTY_CFG_NODE_VALUE;
ConfigNode* const      EMPTY_CFG_NODE_VALUE     = &__EMPTY_CFG_NODE_VALUE;
ConfigPanelNode        __EMPTY_CFG_PANEL_NODE_VALUE;
ConfigPanelNode* const EMPTY_CFG_PANEL_NODE_VALUE = &__EMPTY_CFG_PANEL_NODE_VALUE;

void
ConfigInit(void)
{
    EMPTY_CFG_NODE_VALUE->Next = EMPTY_CFG_NODE_VALUE;
    EMPTY_CFG_NODE_VALUE->Prev = EMPTY_CFG_NODE_VALUE;
    EMPTY_CFG_NODE_VALUE->Head = EMPTY_CFG_NODE_VALUE;
    EMPTY_CFG_NODE_VALUE->Tail = EMPTY_CFG_NODE_VALUE;
    EMPTY_CFG_NODE_VALUE->Parent = EMPTY_CFG_NODE_VALUE;
    EMPTY_CFG_PANEL_NODE_VALUE->Next = EMPTY_CFG_PANEL_NODE_VALUE;
    EMPTY_CFG_PANEL_NODE_VALUE->Prev = EMPTY_CFG_PANEL_NODE_VALUE;
    EMPTY_CFG_PANEL_NODE_VALUE->Head = EMPTY_CFG_PANEL_NODE_VALUE;
    EMPTY_CFG_PANEL_NODE_VALUE->Tail = EMPTY_CFG_PANEL_NODE_VALUE;
    EMPTY_CFG_PANEL_NODE_VALUE->Parent = EMPTY_CFG_PANEL_NODE_VALUE;
    EMPTY_CFG_PANEL_NODE_VALUE->ChildCount = 0;
    EMPTY_CFG_PANEL_NODE_VALUE->Config = EMPTY_CFG_NODE_VALUE;
    EMPTY_CFG_PANEL_NODE_VALUE->SelectedTab = EMPTY_CFG_NODE_VALUE;
}

void 
ListPush(Arena* MemPool, ConfigIDList* List, ConfigID ID)
{
    ConfigIDNode* Node = ArenaPushArrayZero(MemPool, ConfigIDNode, 1);

    SLL_QUEUE_PUSH(List->Head, List->Tail, Node);
    ++List->Count;
    Node->V = ID;
}

ConfigIDList 
ListCpy(Arena* MemPool, ConfigIDList* List)
{
    ConfigIDList Result = {};

    for (
        ConfigIDNode* Node = List->Head;
        Node;
        Node = Node->Next
    ) {
        ListPush(MemPool, &Result, Node->V);
    }

    return Result;
}

void 
ListPush(Arena* MemPool, ConfigNodePtrList* List, ConfigNode* Node)
{
    ConfigNodePtrNode* N = ArenaPushArrayZero(MemPool, ConfigNodePtrNode, 1);

    DLL_PUSH_BACK(List->Head, List->Tail, N);
    ++List->Count;
    N->V = Node;
}

void 
ListPushFront(Arena* MemPool, ConfigNodePtrList* List, ConfigNode* Node)
{
    ConfigNodePtrNode* N = ArenaPushArrayZero(
        MemPool, 
        ConfigNodePtrNode, 
        1
    );

    DLL_PUSH_FRONT(List->Head, List->Tail, N);
    ++List->Count;
    N->V = Node;
}

ConfigNodePtrArray 
ConfigNodePtrArrayFromList(Arena* MemPool, ConfigNodePtrList* List)
{
    ConfigNodePtrArray Result = {};

    Result.Count = List->Count;
    Result.V = ArenaPushArray(MemPool, ConfigNode*, Result.Count);

    u64 Index = 0;

    for (
        ConfigNodePtrNode* Node = List->Head;
        Node;
        Node = Node->Next
    ) {
        Result.V[Index++] = Node->V;
    }

    return Result;
}

void 
ConfigSchemaTableInsert(
    Arena* MemPool, 
    ConfigSchemaTable* Table, 
    Str8 Name, 
    MDNode* Schema
) {
    u64 NameHash = Hash(Name);
    u64 SlotIndex = NameHash % Table->SlotsCount;
    ConfigSchemaNode* Node = NULL;

    for (
        ConfigSchemaNode* N = Table->Slots[SlotIndex];
        N;
        N = N->Next
    ) {
        if (StrMatch(N->Name, Name, 0)) {
            Node = N;
            break;
        }
    }

    if (!Node) {
        Node = ArenaPushArrayZero(MemPool, ConfigSchemaNode, 1);
        Node->Name = ArenaPushStrCpy(MemPool, Name);
        Node->Schema = Schema;
        SLL_STACK_PUSH(Table->Slots[SlotIndex], Node);
    }
}

MDNode* 
ConfigSchemaFromName(ConfigSchemaTable* Table, Str8 Name)
{
    MDNode* Result = EMPTY_MD_NODE_VALUE;
    u64 NameHash = Hash(Name);
    u64 SlotIndex = NameHash % Table->SlotsCount;

    for (
        ConfigSchemaNode* Node = Table->Slots[SlotIndex];
        Node;
        Node = Node->Next
    ) {
        if (StrMatch(Node->Name, Name, 0)) {
            Result = Node->Schema;
            break;
        }
    }

    return Result;
}

MDNodePtrList 
ConfigSchemasFromName(Arena* MemPool, ConfigSchemaTable* Table, Str8 Name)
{
    MDNodePtrList Result = {};
    TempArena Scratch = GetScratch(&MemPool, 1);
    Str8List Tasks = {};
    Str8Node SeedTask = {
        NULL,
        Name
    };

    StrListPushNode(&Tasks, &SeedTask);

    for (
        Str8Node* Task = Tasks.Head;
        Task;
        Task = Task->Next
    ) {
        MDNode* Schema = ConfigSchemaFromName(Table, Task->String);

        if (!MDNodeIsEmpty(Schema)) {
            ListPushFront(MemPool, &Result, Schema);

            for EACH_MDNODE(Tag, Schema->HeadTag) {
                if (StrMatch(Tag->String, "inherit"_s8, 0))
                    ListPush(Scratch.MemPool, &Tasks, Tag->Head->String);
            }
        }
    }

    ReleaseScratch(Scratch);

    return Result;
}

void 
ConfigContextSelect(ConfigContext* Ctx)
{
    GetTLS()->CFG_CTX = Ctx;
}

u64 
ConfigChangeGeneration(void)
{
    u64 Result = 0;
    ThreadLocalStorage* TLS = GetTLS();

    if (TLS->CFG_CTX)
        Result = TLS->CFG_CTX->ChangeGeneration;

    return Result;
}

ConfigNode* 
ConfigNodeRoot(void)
{
    ConfigNode* Result = EMPTY_CFG_NODE_VALUE;
    ThreadLocalStorage* TLS = GetTLS();

    if (TLS->CFG_CTX)
        Result = TLS->CFG_CTX->Root;

    return Result;
}

ConfigNode* 
ConfigNodeFromID(ConfigID ID)
{
    ConfigNode* Result = EMPTY_CFG_NODE_VALUE;
    ThreadLocalStorage* TLS = GetTLS();

    if (
        ID &&
        ID == TLS->CFG_CTX->PrevAccessedID &&
        ID == TLS->CFG_CTX->PrevAccessed->ID
    ) {
        Result = TLS->CFG_CTX->PrevAccessed;
    } else {
        u64 IDHash = Hash(Str8Struct(&ID));
        u64 SlotIndex = IDHash % TLS->CFG_CTX->IDSlotsCount;

        for (
            ConfigNodePtrNode* Node = TLS->CFG_CTX->IDSlots[SlotIndex].Head;
            Node;
            Node = Node->Next
        ) {
            if (Node->V->ID == ID) {
                Result = Node->V;
                break;
            }
        }
    }

    TLS->CFG_CTX->PrevAccessedID = ID;
    TLS->CFG_CTX->PrevAccessed = Result;

    return Result;
}

ConfigNode* 
ConfigNodeChildFromStr(ConfigNode* Parent, Str8 String)
{
    ConfigNode* Result = EMPTY_CFG_NODE_VALUE;

    if (String.Size) {
        for (
            ConfigNode* Node = Parent->Head;
            Node != EMPTY_CFG_NODE_VALUE;
            Node = Node->Next
        ) {
            if (StrMatch(Node->String, String, 0)) {
                Result = Node;
                break;
            }
        }
    }

    return Result;
}

ConfigNode* 
ConfigNodeChildFromStrOrParent(ConfigNode* Parent, Str8 String)
{
    ConfigNode* Result = ConfigNodeChildFromStr(Parent, String);

    if (Result == EMPTY_CFG_NODE_VALUE)
        Result = Parent;

    return Result;
}

ConfigNodePtrList 
ConfigNodeChildListFromStr(Arena* MemPool, ConfigNode* Parent, Str8 String)
{
    ConfigNodePtrList Result = {};

    for (
        ConfigNode* Node = Parent->Head;
        Node != EMPTY_CFG_NODE_VALUE;
        Node = Node->Next
    ) {
        if (StrMatch(Node->String, String, 0))
            ListPush(MemPool, &Result, Node);
    }

    return Result;
}

ConfigNodePtrList 
ConfigNodeTopLevelListFromStr(Arena* MemPool, Str8 String)
{
    ConfigNodePtrList Result = {};
    ThreadLocalStorage* TLS = GetTLS();

    for (
        ConfigNode* Bucket = TLS->CFG_CTX->Root->Head;
        Bucket != EMPTY_CFG_NODE_VALUE;
        Bucket = Bucket->Next
    ) {
        for (
            ConfigNode* Node = Bucket->Head;
            Node != EMPTY_CFG_NODE_VALUE;
            Node = Node->Next
        ) {
            if (StrMatch(Node->String, String, 0))
                ListPush(MemPool, &Result, Node);
        }
    }

    return Result;
}

ConfigNodeRecord 
ConfigNodeRecDepthFirst(ConfigNode* Root, ConfigNode* Node)
{
    ConfigNodeRecord Result = {
        EMPTY_CFG_NODE_VALUE
    };

    if (Node->Head != EMPTY_CFG_NODE_VALUE) {
        Result.Next = Node->Head;
        Result.PushCount = 1;
    } else {
        for (
            ConfigNode* N = Node;
            N != Root;
            N = N->Parent, ++Result.PopCount
        ) {
            if (N->Next != EMPTY_CFG_NODE_VALUE) {
                Result.Next = N->Next;
                break;
            }
        }
    }

    return Result;
}

Str8 
ConfigStrFromTree(
    Arena* MemPool, 
    ConfigSchemaTable* SchemaTable, 
    Str8 RootPath, 
    ConfigNode* Root
) {
    TempArena Scratch = GetScratch(&MemPool, 1);
    Str8List Strings = {};

    struct NestTask {
        NestTask*   Next;
        ConfigNode* Config;
        MDNode*     Schema;
        b32         IsSimple;
    };

    NestTask* HeadNestTask = NULL;
    ConfigNodeRecord Record = {};

    for (
        ConfigNode* Node = Root; 
        Node != EMPTY_CFG_NODE_VALUE; 
        Node = Record.Next
    ) {
        MDNodePtrList Schemas = {};

        if (HeadNestTask) {
            ConfigNode* Parent = HeadNestTask->Config;

            Schemas = ConfigSchemasFromName(
                Scratch.MemPool, 
                SchemaTable, 
                Parent->String
            );
        }

        MDNode* NodeSchema = EMPTY_MD_NODE_VALUE;

        for (
            MDNodePtrNode* N = Schemas.Head;
            N && NodeSchema == EMPTY_MD_NODE_VALUE;
            N = N->Next
        ) {
            NodeSchema = MDChildFromStr(N->V, Node->String, 0);
        }

        if (Node->String.Size || Node->Head == EMPTY_CFG_NODE_VALUE) {
            Str8 NodeSerialisedString = Node->String;

            {
                MDNode* NodeSchema = EMPTY_MD_NODE_VALUE;

                if (HeadNestTask)
                    NodeSchema = HeadNestTask->Schema;

                if (!MDNodeHasTag(NodeSchema->Head, "no_relativise"_s8, 0)) {
                    if (StrMatch(NodeSchema->Head->String, "path"_s8, 0)) {
                        Str8 AbsolutePath = Node->String;
                        Str8 RelativePath = PathRelativeDstFromAbsoluteDstSrc(
                            Scratch.MemPool,
                            AbsolutePath,
                            RootPath
                        );

                        NodeSerialisedString = RelativePath;
                    } else if (StrMatch(NodeSchema->Head->String, "path_pt"_s8, 0)) {
                        Str8 Value = Node->String;
                        Str8TextPointPair Parts = Str8TextPointPairFromStr(Value);
                        Str8 RelativePath = PathRelativeDstFromAbsoluteDstSrc(
                            Scratch.MemPool,
                            Parts.String,
                            RootPath
                        );

                        NodeSerialisedString = ArenaPushStrFmt(
                            MemPool,
                            "%S:%lld:%lld",
                            PRINT_STR(RelativePath),
                            Parts.Point.Line,
                            Parts.Point.Column
                        );
                    }
                }

                NodeSerialisedString = EscapedFromRawStr8(
                    MemPool, 
                    NodeSerialisedString
                );
            }

            Str8List NodeNameStrings = {};

            {
                b32 NameCanBePushedAlone = FALSE;

                {
                    TempArena Temp = ArenaBeginTemp(Scratch.MemPool);
                    MDTokeniseResult NodeNameTokenise = MDTokeniseFromText(
                        Temp.MemPool,
                        NodeSerialisedString
                    );

                    NameCanBePushedAlone = (
                        NodeNameTokenise.Tokens.Count == 1 &&
                        NodeNameTokenise.Tokens.V[0].Kind & (
                            MD_TOKEN_KIND_IDENT |
                            MD_TOKEN_KIND_NUMERIC |
                            MD_TOKEN_KIND_STRLIT |
                            MD_TOKEN_KIND_SYMBOL
                        )
                    );

                    ArenaEndTemp(Temp);
                }

                if (NameCanBePushedAlone) {
                    ListPush(
                        Scratch.MemPool, 
                        &NodeNameStrings, 
                        NodeSerialisedString
                    );
                } else {
                    ListPush(Scratch.MemPool, &NodeNameStrings, "\""_s8);
                    ListPush(Scratch.MemPool, &NodeNameStrings, NodeSerialisedString);
                    ListPush(Scratch.MemPool, &NodeNameStrings, "\""_s8);
                }
            }

            if (HeadNestTask && HeadNestTask->IsSimple)
                ListPush(Scratch.MemPool, &Strings, " "_s8);

            StrListCat(&Strings, &NodeNameStrings);
        }

        Record = ConfigNodeRecDepthFirst(Root, Node);

        if (Node->Head != EMPTY_CFG_NODE_VALUE) {
            b32 IsSimpleChildrenList = TRUE;

            for (
                ConfigNode* Child = Node->Head;
                Child != EMPTY_CFG_NODE_VALUE;
                Child = Child->Next
            ) {
                if (Child->Head != EMPTY_CFG_NODE_VALUE && Child != Node->Tail) {
                    IsSimpleChildrenList = FALSE;
                    break;
                }
            }

            NestTask* Task = ArenaPushArrayZero(
                Scratch.MemPool,
                NestTask,
                1
            );

            Task->Config = Node;
            Task->Schema = NodeSchema;
            Task->IsSimple = IsSimpleChildrenList;
            SLL_STACK_PUSH(HeadNestTask, Task);
        }

        if (Record.PushCount > 0) {
            if (HeadNestTask->IsSimple && Node->String.Size) {
                ListPush(Scratch.MemPool, &Strings, ":"_s8);
            } else {
                if (Node->String.Size)
                    ListPush(Scratch.MemPool, &Strings, ":\n"_s8);

                ListPush(Scratch.MemPool, &Strings, "{"_s8);
            }
        } else {
            for (
                i32 PopIndex = 0; 
                PopIndex < Record.PopCount;
                ++PopIndex
            ) {
                if (HeadNestTask->IsSimple) {
                    if (!HeadNestTask->Config->String.Size)
                        ListPush(Scratch.MemPool, &Strings, " }"_s8);
                } else {
                    ListPush(Scratch.MemPool, &Strings, "\n}"_s8);
                }

                SLL_STACK_POP(HeadNestTask);
            }
        }

        if (!HeadNestTask || !HeadNestTask->IsSimple)
            ListPush(Scratch.MemPool, &Strings, "\n"_s8);
    }

    Str8 ResultUnindented = StrListJoin(Scratch.MemPool, &Strings, NULL);
    Str8 Result = IndentedFromStr(MemPool, ResultUnindented);

    ReleaseScratch(Scratch);

    return Result;
}

ConfigState* 
ConfigStateAlloc(void)
{
    Arena* MemPool = ArenaAlloc();
    ConfigState* Result = ArenaPushArrayZero(MemPool, ConfigState, 1);

    Result->MemPool = MemPool;
    Result->Context.IDSlotsCount = 4096;
    Result->Context.IDSlots = ArenaPushArrayZero(
        MemPool,
        ConfigNodePtrSlot,
        Result->Context.IDSlotsCount
    );
    Result->Context.Root = ConfigNodeAlloc(Result);

    return Result;
}

void 
ConfigStateRelease(ConfigState* State)
{
    ArenaRelease(State->MemPool);
}

ConfigContext* 
ConfigContextFromState(ConfigState* State)
{
    ConfigContext* Result = &State->Context;

    return Result;
}

u64 
ConfigStrBucketNumFromSize(u64 Size)
{
    u64 Result = 0;

    if (Size) {
        for (
            u32 Index = 0; 
            Index < ARRAY_COUNT(CFG_STRING_BUCKET_CHUNK_SIZES); 
            ++Index
        ) {
            if (Size <= CFG_STRING_BUCKET_CHUNK_SIZES[Index]) {
                Result = Index + 1;
                break;
            }
        }
    }

    return Result;
}

Str8 
ConfigStrAlloc(ConfigState* State, Str8 String)
{
    ConfigStrChunkNode* Node = NULL;
    u64 BucketNum = ConfigStrBucketNumFromSize(String.Size);

    if (BucketNum == ARRAY_COUNT(CFG_STRING_BUCKET_CHUNK_SIZES)) {
        ConfigStrChunkNode* BestNode = NULL;
        ConfigStrChunkNode* BestNodePrev = NULL;
        u64 BestNodeSize = U64_MAX;

        for (
            ConfigStrChunkNode* N = State->FreeStringChunks[BucketNum - 1], *Prev = NULL;
            N;
            (Prev = N, N = N->Next)
        ) {
            if (N->Size >= String.Size && N->Size < BestNodeSize) {
                BestNode = N;
                BestNodePrev = Prev;
                BestNodeSize = N->Size;
            }
        }

        if (BestNode) {
            Node = BestNode;

            if (BestNodePrev)
                BestNodePrev->Next = BestNode->Next;
            else
                State->FreeStringChunks[BucketNum - 1] = BestNode->Next;
        } else {
            u64 ChunkSize = Round64NextPow2(String.Size);

            Node = (ConfigStrChunkNode*) ArenaPushArrayZero(
                State->MemPool,
                u8,
                ChunkSize
            );
        }
    } else if (BucketNum) {
        Node = State->FreeStringChunks[BucketNum - 1];

        if (Node) {
            SLL_STACK_POP(State->FreeStringChunks[BucketNum - 1]);
        } else {
            Node = (ConfigStrChunkNode*) ArenaPushArrayZero(
                State->MemPool,
                u8,
                CFG_STRING_BUCKET_CHUNK_SIZES[BucketNum - 1]
            );
        }
    }

    Str8 Result = {};

    if (Node) {
        Result.Str = (u8*) Node;
        Result.Size = String.Size;
        MemCpy(Result.Str, String.Str, Result.Size);
    }

    return Result;
}

void 
ConfigStrRelease(ConfigState* State, Str8 String)
{
    u64 BucketNum = ConfigStrBucketNumFromSize(String.Size);

    if (
        BucketNum >= 1 && 
        BucketNum <= ARRAY_COUNT(CFG_STRING_BUCKET_CHUNK_SIZES)
    ) {
        u64 BucketIndex = BucketNum - 1;
        ConfigStrChunkNode* Node = (ConfigStrChunkNode*) String.Str;

        SLL_STACK_PUSH(State->FreeStringChunks[BucketIndex], Node);
        Node->Size = Round64NextPow2(String.Size);
    }
}

ConfigNode* 
ConfigNodeAlloc(ConfigState* State)
{
    ++State->Context.ChangeGeneration;

    ConfigNode* Result = State->Free;

    if (Result)
        SLL_STACK_POP(State->Free);
    else
        Result = ArenaPushArray(State->MemPool, ConfigNode, 1);

    ++State->IDGeneration;
    MemZero(Result, sizeof(*Result));
    Result->Head = EMPTY_CFG_NODE_VALUE;
    Result->Tail = EMPTY_CFG_NODE_VALUE;
    Result->Next = EMPTY_CFG_NODE_VALUE;
    Result->Prev = EMPTY_CFG_NODE_VALUE;
    Result->Parent = EMPTY_CFG_NODE_VALUE;
    Result->ID = State->IDGeneration;

    ConfigNodePtrNode* ConfigIDNode = State->FreeIDNode;

    if (ConfigIDNode)
        SLL_STACK_POP(State->FreeIDNode);
    else
        ConfigIDNode = ArenaPushArrayZero(State->MemPool, ConfigNodePtrNode, 1);

    u64 IDHash = Hash(Str8Struct(&Result->ID));
    u64 SlotIndex = IDHash % State->Context.IDSlotsCount;

    DLL_PUSH_BACK(
        State->Context.IDSlots[SlotIndex].Head,
        State->Context.IDSlots[SlotIndex].Tail,
        ConfigIDNode
    );
    ConfigIDNode->V = Result;

    return Result;
}

void 
ConfigNodeRelease(ConfigState* State, ConfigNode* Node)
{
    ++State->Context.ChangeGeneration;

    TempArena Scratch = GetScratch(NULL, 0);

    ConfigNodeUnhook(State, Node->Parent, Node);

    ConfigNodePtrList Nodes = {};

    for (
        ConfigNode* N = Node;
        N != EMPTY_CFG_NODE_VALUE;
        N = ConfigNodeRecDepthFirst(Node, N).Next
    ) {
        ListPush(Scratch.MemPool, &Nodes, N);
    }

    for (
        ConfigNodePtrNode* N = Nodes.Head;
        N;
        N = N->Next
    ) {
        ConfigNode* C = N->V;
        u64 IDHash = Hash(Str8Struct(&C->ID));
        u64 SlotIndex = IDHash % State->Context.IDSlotsCount;

        ConfigStrRelease(State, C->String);
        SLL_STACK_PUSH(State->Free, C);
        C->Head = NULL;
        C->Tail = NULL;
        C->Prev = NULL;
        C->Parent = NULL;
        C->ID = 0;
        C->String = ""_s8;

        for (
            ConfigNodePtrNode* PtrNode = State->Context.IDSlots[SlotIndex].Head;
            PtrNode;
            PtrNode = PtrNode->Next
        ) {
            if (PtrNode->V == C) {
                DLL_REMOVE(
                    State->Context.IDSlots[SlotIndex].Head,
                    State->Context.IDSlots[SlotIndex].Tail,
                    PtrNode
                );
                SLL_STACK_PUSH(State->FreeIDNode, PtrNode);
                break;
            }
        }
    }

    ReleaseScratch(Scratch);
}

void 
ConfigNodeReleaseAllChildren(ConfigState* State, ConfigNode* Node)
{
    for (
        ConfigNode* Child = Node->Head, *Next = EMPTY_CFG_NODE_VALUE;
        Child != EMPTY_CFG_NODE_VALUE;
        Child = Next
    ) {
        Next = Child->Next;
        ConfigNodeRelease(State, Child);
    }
}

ConfigNode* 
ConfigNodeNew(ConfigState* State, ConfigNode* Parent, Str8 String)
{
    ConfigNode* Result = ConfigNodeAlloc(State);

    ConfigNodeInsertChild(State, Parent, Parent->Tail, Result);
    ConfigNodeEquipStr(State, Result, String);

    return Result;
}

ConfigNode* 
ConfigNodeNew(ConfigState* State, ConfigNode* Parent, char* Fmt, ...)
{
    TempArena Scratch = GetScratch(NULL, 0);
    va_list Args;

    va_start(Args, Fmt);

    Str8 String = ArenaPushStrFmtV(Scratch.MemPool, Fmt, Args);
    ConfigNode* Result = ConfigNodeNew(State, Parent, String);

    va_end(Args);
    ReleaseScratch(Scratch);

    return Result;
}

ConfigNode* 
ConfigNodeNewReplace(ConfigState* State, ConfigNode* Parent, Str8 String)
{
    TempArena Scratch = GetScratch(NULL, 0);
    
    String = ArenaPushStrCpy(Scratch.MemPool, String);

    for (
        ConfigNode* Child = Parent->Head->Next, *Next = EMPTY_CFG_NODE_VALUE;
        Child != EMPTY_CFG_NODE_VALUE;
        Child = Next
    ) {
        Next = Child->Next;
        ConfigNodeRelease(State, Child);
    }

    if (Parent->Head == EMPTY_CFG_NODE_VALUE)
        ConfigNodeNew(State, Parent, ""_s8);

    ConfigNode* Result = Parent->Head;

    ConfigNodeEquipStr(State, Result, String);
    ReleaseScratch(Scratch);

    return Result;
}

ConfigNode* 
ConfigNodeNewReplace(ConfigState* State, ConfigNode* Parent, char* Fmt, ...)
{
    TempArena Scratch = GetScratch(NULL, 0);
    va_list Args;

    va_start(Args, Fmt);

    Str8 String = ArenaPushStrFmtV(Scratch.MemPool, Fmt, Args);
    ConfigNode* Result = ConfigNodeNewReplace(State, Parent, String);

    va_end(Args);
    ReleaseScratch(Scratch);

    return Result;
}

ConfigNode* 
ConfigNodeCpy(ConfigState* State, ConfigNode* Root)
{
    ConfigNodeRecord Record = {};
    ConfigNode* Result = EMPTY_CFG_NODE_VALUE;
    ConfigNode* DstParent = EMPTY_CFG_NODE_VALUE;

    for (
        ConfigNode* Src = Root;
        Src != EMPTY_CFG_NODE_VALUE;
        Src = Record.Next
    ) {
        ConfigNode* Dst = ConfigNodeNew(State, DstParent, Src->String);

        if (Result == EMPTY_CFG_NODE_VALUE)
            Result = Dst;

        Record = ConfigNodeRecDepthFirst(Root, Src);

        if (Record.PushCount > 0) {
            DstParent = Dst;
        } else {
            for (i32 PopIndex = 0; PopIndex < Record.PopCount; ++PopIndex)
                DstParent = DstParent->Parent;
        }
    }

    return Result;
}

void 
ConfigNodeEquipStr(ConfigState* State, ConfigNode* Node, Str8 String)
{
    ConfigStrRelease(State, Node->String);
    Node->String = ConfigStrAlloc(State, String);
    ++State->Context.ChangeGeneration;
}

void 
ConfigNodeEquipStr(ConfigState* State, ConfigNode* Node, char* Fmt, ...)
{
    TempArena Scratch = GetScratch(NULL, 0);
    va_list Args;

    va_start(Args, Fmt);

    Str8 String = ArenaPushStrFmtV(Scratch.MemPool, Fmt, Args);

    ConfigNodeEquipStr(State, Node, String);
    va_end(Args);
    ReleaseScratch(Scratch);
}

void 
ConfigNodeInsertChild(
    ConfigState* State, 
    ConfigNode* Parent, 
    ConfigNode* PrevChild, 
    ConfigNode* NewChild
) {
    if (Parent != EMPTY_CFG_NODE_VALUE) {
        if (NewChild->Parent != EMPTY_CFG_NODE_VALUE)
            ConfigNodeUnhook(State, NewChild->Parent, NewChild);

        DLL_INSERT_EX(
            EMPTY_CFG_NODE_VALUE, 
            Parent->Head, 
            Parent->Tail, 
            PrevChild, 
            NewChild, 
            Next, 
            Prev
        );
        NewChild->Parent = Parent;
    }
}

void 
ConfigNodeUnhook(ConfigState* State, ConfigNode* Parent, ConfigNode* Child)
{
    if (
        Child != EMPTY_CFG_NODE_VALUE &&
        Parent == Child->Parent &&
        Parent != EMPTY_CFG_NODE_VALUE
    ) {
        DLL_REMOVE_EX(
            EMPTY_CFG_NODE_VALUE,
            Parent->Head,
            Parent->Tail,
            Child,
            Next,
            Prev
        );
        Child->Parent = EMPTY_CFG_NODE_VALUE;
    }
}

ConfigNode* 
ConfigNodeChildFromStrOrAlloc(
    ConfigState* State, 
    ConfigNode* Parent, 
    Str8 String
) {
    ConfigNode* Result = ConfigNodeChildFromStr(Parent, String);

    if (Result == EMPTY_CFG_NODE_VALUE)
        Result = ConfigNodeNew(State, Parent, String);

    return Result;
}

ConfigNodePtrList 
ConfigNodePtrListFromStr(
    Arena* MemPool, 
    ConfigState* State, 
    ConfigSchemaTable* SchemaTable, 
    Str8 RootPath, 
    Str8 String
) {
    ConfigNodePtrList Result = {};
    TempArena Scratch = GetScratch(&MemPool, 1);
    MDNode* Root = MDTreeFromStr(Scratch.MemPool, String);

    for EACH_MDNODE(TLN, Root->Head) {
        ConfigNode* DstRootNode = EMPTY_CFG_NODE_VALUE;
        ConfigNode* DstActiveParentNode = EMPTY_CFG_NODE_VALUE;
        MDNodeRecord Record = {};

        for (MDNode* SrcNode = TLN; !MDNodeIsEmpty(SrcNode); SrcNode = Record.Next) {
            MDNode* Schema = EMPTY_MD_NODE_VALUE;

            {
                MDNodePtrList Schemas = ConfigSchemasFromName(
                    Scratch.MemPool,
                    SchemaTable,
                    DstActiveParentNode->Parent->String
                );

                for (
                    MDNodePtrNode* N = Schemas.Head;
                    N && Schema == EMPTY_MD_NODE_VALUE;
                    N = N->Next
                ) {
                    Schema = MDChildFromStr(N->V, DstActiveParentNode->String, 0);
                }
            }

            Str8 DstNodeString = {};

            {
                Str8 SrcNodeString = SrcNode->String;
                Str8 SrcNodeStringRaw = RawFromEscapedStr8(
                    Scratch.MemPool, 
                    SrcNodeString
                );
                
                if (!MDNodeHasTag(Schema->Head, "no_relativise"_s8, 0)) {
                    if (StrMatch(Schema->Head->String, "path"_s8, 0)) {
                        SrcNodeStringRaw = PathAbsoluteDstFromRelativeDstSrc(
                            Scratch.MemPool,
                            SrcNodeStringRaw,
                            RootPath
                        );
                    } else if (StrMatch(Schema->Head->String, "path_pt"_s8, 0)) {
                        Str8TextPointPair Parts = Str8TextPointPairFromStr(SrcNodeStringRaw);
                        
                        SrcNodeStringRaw = ArenaPushStrFmt(
                            Scratch.MemPool,
                            "%S:%lld:%lld",
                            PRINT_STR(PathAbsoluteDstFromRelativeDstSrc(Scratch.MemPool, Parts.String, RootPath)),
                            Parts.Point.Line,
                            Parts.Point.Column
                        );
                    }
                }

                DstNodeString = SrcNodeStringRaw;
            }

            ConfigNode* DstNode = ConfigNodeAlloc(State);

            ConfigNodeEquipStr(State, DstNode, DstNodeString);

            if (DstActiveParentNode != EMPTY_CFG_NODE_VALUE) {
                ConfigNodeInsertChild(
                    State, 
                    DstActiveParentNode, 
                    DstActiveParentNode->Tail, 
                    DstNode
                );
            }

            Record = MDNodeRecordDepthFirstPreOrder(SrcNode, TLN);

            if (DstActiveParentNode == EMPTY_CFG_NODE_VALUE)
                DstRootNode = DstNode;

            if (Record.PushCount > 0) {
                DstActiveParentNode = DstNode;
            } else {
                for (i32 PopIndex = 0; PopIndex < Record.PopCount; ++PopIndex) {
                    DstActiveParentNode = DstActiveParentNode->Parent;
                }
            }
        }

        ListPush(MemPool, &Result, DstRootNode);
    }

    ReleaseScratch(Scratch);

    return Result;
}

ConfigInputMap* 
ConfigInputMapFromConfig(Arena* MemPool)
{
    TempArena Scratch = GetScratch(&MemPool, 1);
    ConfigInputMap* Result = ArenaPushArrayZero(MemPool, ConfigInputMap, 1);

    Result->NameSlotsCount = 4096;
    Result->NameSlots = ArenaPushArrayZero(
        MemPool,
        ConfigInputMapSlot,
        Result->NameSlotsCount
    );
    Result->BindingSlotsCount = 4096;
    Result->BindingSlots = ArenaPushArrayZero(
        MemPool,
        ConfigInputMapSlot,
        Result->BindingSlotsCount
    );

    ConfigNodePtrList InputBindingsConfigList = ConfigNodeTopLevelListFromStr(
        Scratch.MemPool,
        "keybindings"_s8
    );

    for (
        ConfigNodePtrNode* N = InputBindingsConfigList.Head;
        N;
        N = N->Next
    ) {
        ConfigNode* InputBindingsRoot = N->V;

        for (
            ConfigNode* InputBinding = InputBindingsRoot->Head;
            InputBinding != EMPTY_CFG_NODE_VALUE;
            InputBinding = InputBinding->Next
        ) {
            Str8 Name = {};
            ConfigBinding Binding = {};

            for (
                ConfigNode* Child = InputBinding->Head;
                Child != EMPTY_CFG_NODE_VALUE;
                Child = Child->Next
            ) {
                if (StrMatch(Child->String, "ctrl"_s8, 0)) {
                    Binding.ModKind |= INPUT_MOD_KIND_CTRL;
                } else if (StrMatch(Child->String, "alt"_s8, 0)) {
                    Binding.ModKind |= INPUT_MOD_KIND_ALT;
                } else if (StrMatch(Child->String, "shift"_s8, 0)) {
                    Binding.ModKind |= INPUT_MOD_KIND_SHIFT;
                } else {
                    InputKind Input = INPUT_KIND_NULL;

                    // WARN(nathan): this iterates [0, INPUT_KIND_COUNT)
                    //             : including INPUT_KIND_NULL might cause issues?
                    for EACH_ENUM(InputKind, I, INPUT_KIND_COUNT) {
                        if (StrMatch(Child->String, INPUT_DISPLAY_STR_TABLE[I], STR_MATCH_ALL_CASES)) {
                            Input = I;
                            break;
                        }
                    }

                    if (Input != INPUT_KIND_NULL)
                        Binding.Input = Input;
                    else
                        Name = Child->String;
                }
            }

            if (Name.Size) {
                u64 NameHash = Hash(Name);
                u64 BindingHash = Hash(Str8Struct(&Binding));
                u64 NameSlotIndex = NameHash % Result->NameSlotsCount;
                u64 BindingSlotIndex = BindingHash % Result->BindingSlotsCount;
                ConfigInputMapNode* N = ArenaPushArrayZero(MemPool, ConfigInputMapNode, 1);

                N->ID = InputBinding->ID;
                N->Name = ArenaPushStrCpy(MemPool, Name);
                N->Binding = Binding;

                SLL_QUEUE_PUSH_EX(
                    Result->NameSlots[NameSlotIndex].Head,
                    Result->NameSlots[NameSlotIndex].Tail,
                    N,
                    NameHashNext
                );
                SLL_QUEUE_PUSH_EX(
                    Result->BindingSlots[BindingSlotIndex].Head,
                    Result->BindingSlots[BindingSlotIndex].Tail,
                    N,
                    BindingHashNext
                );
            }
        }
    }

    ReleaseScratch(Scratch);

    return Result;
}

ConfigInputMapNodePtrList 
ConfigInputMapNodePtrListFromName(
    Arena* MemPool, 
    ConfigInputMap* InputMap, 
    Str8 String
) {
    ConfigInputMapNodePtrList Result = {};
    u64 StringHash = Hash(String);
    u64 SlotIndex = StringHash % InputMap->NameSlotsCount;

    for (
        ConfigInputMapNode* N = InputMap->NameSlots[SlotIndex].Head;
        N;
        N = N->NameHashNext
    ) {
        if (StrMatch(N->Name, String, 0)) {
            ConfigInputMapNodePtr* Ptr = ArenaPushArrayZero(
                MemPool,
                ConfigInputMapNodePtr,
                1
            );

            Ptr->V = N;
            SLL_QUEUE_PUSH(Result.Head, Result.Tail, Ptr);
            ++Result.Count;
        }
    }

    return Result;
}

ConfigInputMapNodePtrList 
ConfigInputMapNodePtrListFromBinding(
    Arena* MemPool, 
    ConfigInputMap* InputMap, 
    ConfigBinding Binding
) {
    ConfigInputMapNodePtrList Result = {};
    u64 BindingHash = Hash(Str8Struct(&Binding));
    u64 SlotIndex = BindingHash % InputMap->BindingSlotsCount;

    for (
        ConfigInputMapNode* N = InputMap->BindingSlots[SlotIndex].Head;
        N;
        N = N->BindingHashNext
    ) {
        if (MemCmpStruct(&Binding, &N->Binding)) {
            ConfigInputMapNodePtr* Ptr = ArenaPushArrayZero(
                MemPool,
                ConfigInputMapNodePtr,
                1
            );

            Ptr->V = N;
            SLL_QUEUE_PUSH(Result.Head, Result.Tail, Ptr);
            ++Result.Count;
        }
    }

    return Result;
}

ConfigNode* 
ConfigWindowFromConfig(ConfigNode* Config)
{
    ConfigNode* Result = EMPTY_CFG_NODE_VALUE;

    for (
        ConfigNode* Node = Config; 
        Node != EMPTY_CFG_NODE_VALUE; 
        Node = Node->Parent
    ) {
        if (
            Node->Parent->Parent == ConfigNodeRoot() &&
            StrMatch(Node->String, "window"_s8, 0)
        ) {
            Result = Node;
            break;
        }
    }

    return Result;
}

ConfigPanelTree 
ConfigPanelTreeFromConfig(Arena* MemPool, ConfigNode* CfgRoot)
{
    TempArena Scratch = GetScratch(&MemPool, 1);
    ConfigNode* WindowCfg = ConfigWindowFromConfig(CfgRoot);
    ConfigNode* SrcRoot = ConfigNodeChildFromStr(WindowCfg, "panels"_s8);
    ConfigPanelNode* DstRoot = EMPTY_CFG_PANEL_NODE_VALUE;
    ConfigPanelNode* DstFocused = EMPTY_CFG_PANEL_NODE_VALUE;
    Axis2D ActiveSplitAxis = ConfigNodeChildFromStr(WindowCfg, "split_x"_s8) != EMPTY_CFG_NODE_VALUE
        ? AXIS_2D_X 
        : AXIS_2D_Y;
    ConfigNodeRecord Rec = {};
    ConfigPanelNode* DstActiveParent = EMPTY_CFG_PANEL_NODE_VALUE;

    for (
        ConfigNode* Src = SrcRoot; 
        Src != EMPTY_CFG_NODE_VALUE; 
        Src = Rec.Next
    ) {
        ConfigPanelNode* Dst = ArenaPushArrayZero(MemPool, ConfigPanelNode, 1);

        MemCpy(Dst, EMPTY_CFG_PANEL_NODE_VALUE, sizeof(*Dst));
        Dst->Parent = DstActiveParent;

        if (DstActiveParent != EMPTY_CFG_PANEL_NODE_VALUE) {
            DLL_PUSH_BACK_EX(
                EMPTY_CFG_PANEL_NODE_VALUE,
                DstActiveParent->Head,
                DstActiveParent->Tail,
                Dst,
                Next,
                Prev
            );
            ++DstActiveParent->ChildCount;
        }

        if (DstRoot == EMPTY_CFG_PANEL_NODE_VALUE)
            DstRoot = Dst;

        b32 PanelHasChildren = FALSE;

        Dst->Config = Src;
        Dst->PercentOfParent = (Src == SrcRoot)
            ? 1.0f
            : (f32) F64FromStr(Src->String);
        Dst->TabSide = (ConfigNodeChildFromStr(Src, "tabs_on_bottom"_s8) != EMPTY_CFG_NODE_VALUE)
            ? SIDE_MAX
            : SIDE_MIN;
        Dst->SplitAxis = ActiveSplitAxis;

        for (
            ConfigNode* SrcChild = Src->Head; 
            SrcChild != EMPTY_CFG_NODE_VALUE; 
            SrcChild = SrcChild->Next
        ) {
            MDTokeniseResult Tokenise = MDTokeniseFromText(
                Scratch.MemPool, 
                SrcChild->String
            );

            if (
                Tokenise.Tokens.Count == 1 && 
                Tokenise.Tokens.V[0].Kind & MD_TOKEN_KIND_NUMERIC
            ) {
                PanelHasChildren = TRUE;
            } else if (StrMatch(SrcChild->String, "tabs_on_bottom"_s8, 0)) {
                // NOTE(nathan): NO-OP: panel option
            } else if (StrMatch(SrcChild->String, "selected"_s8, 0)) {
                DstFocused = Dst;
            } else if (
                Tokenise.Tokens.Count == 1 && 
                Tokenise.Tokens.V[0].Kind & MD_TOKEN_KIND_IDENT
            ) {
                ListPush(MemPool, &Dst->Tabs, SrcChild);

                if (ConfigNodeChildFromStr(SrcChild, "selected"_s8) != EMPTY_CFG_NODE_VALUE) {
                    Dst->SelectedTab = SrcChild;
                }
            }
        }

        Rec = ConfigNodeRecDepthFirst(SrcRoot, Src);

        if (!PanelHasChildren) {
            MemSet(&Rec, 0, sizeof(Rec));
            Rec.Next = EMPTY_CFG_NODE_VALUE;

            for (
                ConfigNode* P = Src; 
                P != SrcRoot && P != EMPTY_CFG_NODE_VALUE; 
                P = P->Parent, ++Rec.PopCount
            ) {
                if (P->Next != EMPTY_CFG_NODE_VALUE) {
                    Rec.Next = P->Next;
                    break;
                }
            }
        }

        if (Rec.PushCount > 0) {
            DstActiveParent = Dst;
            ActiveSplitAxis = FLIP_AXIS(ActiveSplitAxis);
        } else {
            for (i32 PopIndex = 0; PopIndex < Rec.PopCount; ++PopIndex) {
                DstActiveParent = DstActiveParent->Parent;
                ActiveSplitAxis = FLIP_AXIS(ActiveSplitAxis);
            }
        }
    }

    ReleaseScratch(Scratch);

    ConfigPanelTree Result = {
        DstRoot,
        DstFocused
    };

    return Result;
}

ConfigPanelNodeRecord 
ConfigPanelNodeRecDepthFirst(
    ConfigPanelNode* Root, 
    ConfigPanelNode* Panel, 
    u64 SiblingOffset, 
    u64 ChildOffset
) {
    ConfigPanelNodeRecord Result = {
        EMPTY_CFG_PANEL_NODE_VALUE
    };

    if (*MEMBER_FROM_OFFSET(ConfigPanelNode**, Panel, ChildOffset) != EMPTY_CFG_PANEL_NODE_VALUE) {
        Result.Next = *MEMBER_FROM_OFFSET(ConfigPanelNode**, Panel, ChildOffset);
        ++Result.PushCount;
    } else {
        for (
            ConfigPanelNode* P = Panel; 
            P != EMPTY_CFG_PANEL_NODE_VALUE && P != Root; 
            P = P->Parent, ++Result.PopCount
        ) {
            if (*MEMBER_FROM_OFFSET(ConfigPanelNode**, P, SiblingOffset) != EMPTY_CFG_PANEL_NODE_VALUE) {
                Result.Next = *MEMBER_FROM_OFFSET(ConfigPanelNode**, P, SiblingOffset);
                break;
            }
        }
    }

    return Result;
}

ConfigPanelNode* 
ConfigPanelNodeFromConfigTree(ConfigPanelNode* Root, ConfigNode* Cfg)
{
    ConfigPanelNode* Result = EMPTY_CFG_PANEL_NODE_VALUE;

    for (
        ConfigPanelNode* P = Root; 
        P != EMPTY_CFG_PANEL_NODE_VALUE; 
        P = ConfigPanelNodeRecDepthFirstPre(Root, P).Next
    ) {
        if (P->Config == Cfg) {
            Result = P;
            break;
        }
    }

    return Result;
}

r2f32 
ConfigTargetRectFromPanelNodeChild(
    r2f32 ParentRect, 
    ConfigPanelNode* Parent, 
    ConfigPanelNode* Panel
) {
    r2f32 Result = ParentRect;

    if (Parent != EMPTY_CFG_PANEL_NODE_VALUE) {
        v2f32 ParentRectSize = Length(ParentRect);
        Axis2D Axis = Parent->SplitAxis;

        Result.Point1.V[Axis] = Result.Point0.V[Axis];

        for (
            ConfigPanelNode* Child = Parent->Head; 
            Child != EMPTY_CFG_PANEL_NODE_VALUE; 
            Child = Child->Next
        ) {
            Result.Point1.V[Axis] += ParentRectSize.V[Axis] * Child->PercentOfParent;

            if (Child == Panel)
                break;

            Result.Point0.V[Axis] = Result.Point1.V[Axis];
        }
    }

    Result.X0 = RoundF32(Result.X0);
    Result.X1 = RoundF32(Result.X1);
    Result.Y0 = RoundF32(Result.Y0);
    Result.Y1 = RoundF32(Result.Y1);

    return Result;
}

r2f32 
ConfigTargetRectFromPanelNode(
    r2f32 RootRect, 
    ConfigPanelNode* Root, 
    ConfigPanelNode* Panel
) {
    TempArena Scratch = GetScratch(NULL, 0);
    u64 AncestorCount = 0;

    for (
        ConfigPanelNode* P = Panel->Parent; 
        P != EMPTY_CFG_PANEL_NODE_VALUE; 
        P = P->Parent
    ) {
        ++AncestorCount;
    }

    ConfigPanelNode** Ancestors = ArenaPushArrayZero(
        Scratch.MemPool, 
        ConfigPanelNode*, 
        AncestorCount
    );
    u64 AncestorIndex = 0;

    for (
        ConfigPanelNode* P = Panel->Parent; 
        P != EMPTY_CFG_PANEL_NODE_VALUE; 
        P = P->Parent
    ) {
        Ancestors[AncestorIndex++] = P;
    }

    r2f32 ParentRect = RootRect;

    for (
        AncestorIndex = (i64) AncestorCount - 1; 
        AncestorIndex >= 0 && AncestorIndex < AncestorCount; 
        --AncestorIndex
    ) {
        ConfigPanelNode* Ancestor = Ancestors[AncestorIndex];
        ConfigPanelNode* Parent = Ancestor->Parent;

        if (Parent != EMPTY_CFG_PANEL_NODE_VALUE) {
            ParentRect = ConfigTargetRectFromPanelNodeChild(
                ParentRect, 
                Parent, 
                Ancestor
            );
        }
    }

    r2f32 Result = ConfigTargetRectFromPanelNodeChild(
        ParentRect, 
        Panel->Parent, 
        Panel
    );

    ReleaseScratch(Scratch);

    return Result;
}
