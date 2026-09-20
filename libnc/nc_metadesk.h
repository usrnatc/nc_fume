#if !defined(__NC_METADESK_H__)
#define __NC_METADESK_H__

#include "nc_types.h"
#include "nc_string.h"

// @defines____________________________________________________________________
enum MDMessageKind {
    MD_MESSAGE_KIND_NULL,
    MD_MESSAGE_KIND_NOTE,
    MD_MESSAGE_KIND_WARNING,
    MD_MESSAGE_KIND_ERROR,
    MD_MESSAGE_KIND_FATAL_ERROR
};

typedef u32 MDTokenKind;
enum : u32 {
    MD_TOKEN_KIND_IDENT          = (1 << 0),
    MD_TOKEN_KIND_NUMERIC        = (1 << 1),
    MD_TOKEN_KIND_STRLIT         = (1 << 2),
    MD_TOKEN_KIND_SYMBOL         = (1 << 3),
    MD_TOKEN_KIND_RESERVED       = (1 << 4),
    MD_TOKEN_KIND_COMMENT        = (1 << 5),
    MD_TOKEN_KIND_WHITESPACE     = (1 << 6),
    MD_TOKEN_KIND_NEWLINE        = (1 << 7),
    MD_TOKEN_KIND_STR_SINGLE     = (1 << 8),
    MD_TOKEN_KIND_STR_DOUBLE     = (1 << 9),
    MD_TOKEN_KIND_STR_TICK       = (1 << 10),
    MD_TOKEN_KIND_STR_TRIPLET    = (1 << 11),
    MD_TOKEN_KIND_BROKEN_COMMENT = (1 << 12),
    MD_TOKEN_KIND_BROKEN_STRLIT  = (1 << 13),
    MD_TOKEN_KIND_BAD_CHAR       = (1 << 14)
};

typedef u32 MDTokenGroupKind;
enum : u32 {
    MD_TOKEN_GROUP_COMMENT    = MD_TOKEN_KIND_COMMENT,
    MD_TOKEN_GROUP_WHITESPACE = (MD_TOKEN_KIND_WHITESPACE | MD_TOKEN_KIND_NEWLINE),
    MD_TOKEN_GROUP_IRREGULAR  = (MD_TOKEN_GROUP_COMMENT | MD_TOKEN_GROUP_WHITESPACE),
    MD_TOKEN_GROUP_REGULAR    = ~MD_TOKEN_GROUP_IRREGULAR,
    MD_TOKEN_GROUP_LABEL      = (MD_TOKEN_KIND_IDENT | MD_TOKEN_KIND_NUMERIC | MD_TOKEN_KIND_STRLIT | MD_TOKEN_KIND_SYMBOL),
    MD_TOKEN_GROUP_ERROR      = (MD_TOKEN_KIND_BROKEN_COMMENT | MD_TOKEN_KIND_BROKEN_STRLIT | MD_TOKEN_KIND_BAD_CHAR)
};

enum MDNodeKind {
    MD_NODE_KIND_NIL,
    MD_NODE_KIND_FILE,
    MD_NODE_KIND_ERROR_MARKER,
    MD_NODE_KIND_MAIN,
    MD_NODE_KIND_TAG,
    MD_NODE_KIND_LIST,
    MD_NODE_KIND_REFERENCE,
    MD_NODE_KIND_COUNT
};

typedef u32 MDNodeFlags;
enum : u32 {
    MD_NODE_FLAG_MASK_SET_DELIMITERS    = (0x3F << 0),
    MD_NODE_FLAG_HAS_PAREN_LEFT         = (1 << 0),
    MD_NODE_FLAG_HAS_PAREN_RIGHT        = (1 << 1),
    MD_NODE_FLAG_HAS_BRACK_LEFT         = (1 << 2),
    MD_NODE_FLAG_HAS_BRACK_RIGHT        = (1 << 3),
    MD_NODE_FLAG_HAS_BRACE_LEFT         = (1 << 4),
    MD_NODE_FLAG_HAS_BRACE_RIGHT        = (1 << 5),
    MD_NODE_FLAG_MASK_SEPARATORS        = (0xF << 6),
    MD_NODE_FLAG_IS_BEFORE_SEMICOLON    = (1 << 6),
    MD_NODE_FLAG_IS_AFTER_SEMICOLON     = (1 << 7),
    MD_NODE_FLAG_IS_BEFORE_COMMA        = (1 << 8),
    MD_NODE_FLAG_IS_AFTER_COMMA         = (1 << 9),
    MD_NODE_FLAG_MASK_STR_DELIMITERS    = (0xF << 10),
    MD_NODE_FLAG_STR_SINGLE             = (1 << 10),
    MD_NODE_FLAG_STR_DOUBLE             = (1 << 11),
    MD_NODE_FLAG_STR_TICK               = (1 << 12),
    MD_NODE_FLAG_STR_TRIPLET            = (1 << 13),
    MD_NODE_FLAG_MASK_LABEL_KIND        = (0xF << 14),
    MD_NODE_FLAG_NUMERIC                = (1 << 14),
    MD_NODE_FLAG_IDENT                  = (1 << 15),
    MD_NODE_FLAG_STRLIT                 = (1 << 16),
    MD_NODE_FLAG_SYMBOL                 = (1 << 17)
};

#define MD_NODE_FLAG_AFTER_FROM_BEFORE(X)           ((X) << 1)
#define EACH_MDNODE(X, Y)                           (MDNode* X = Y; !MDNodeIsEmpty(X); X = X->Next)
#define MDNodeRecordDepthFirstPreOrder(X, Y)        MDNodeRecordDepthFirst((X), (Y), OFFSETOF(MDNode, Head), OFFSETOF(MDNode, Next))
#define MDNodeRecordDepthFirstPreOrderReverse(X, Y) MDNodeRecordDepthFirst((X), (Y), OFFSETOF(MDNode, Tail), OFFSETOF(MDNode, Prev))
#define MDTreeFromStr(X, Y)                         (MDParseFromText((X), {}, (Y)).Root)

struct MDNode;

// @types______________________________________________________________________
struct MDMessage {
    MDMessage*     Next;
    MDNode* Node;
    MDMessageKind  Kind;
    Str8           String;
};

struct MDMessageList {
    MDMessage*    Head;
    MDMessage*    Tail;
    u64           Count;
    MDMessageKind WorstMessageKind;
};

struct MDToken {
    r1u64       Range;
    MDTokenKind Kind;
};

struct MDTokenChunkNode {
    MDTokenChunkNode* Next;
    MDToken*          V;
    u64               Count;
    u64               Capacity;
};

struct MDTokenChunkList {
    MDTokenChunkNode* Head;
    MDTokenChunkNode* Tail;
    u64               ChunkCount;
    u64               TotalTokenCount;
};

struct MDTokenArray {
    MDToken* V;
    u64      Count;
};

struct MDNode {
    MDNode*     Next;
    MDNode*     Prev;
    MDNode*     Parent;
    MDNode*     Head;
    MDNode*     Tail;
    MDNode*     HeadTag;
    MDNode*     TailTag;
    MDNodeKind  Kind;
    MDNodeFlags Flags;
    Str8        String;
    Str8        RawString;
    u64         SrcOffset;
    u64         __UserGeneration;
    u64         __Unused[2];
};

struct MDNodeRecord {
    MDNode* Next;
    i32     PushCount;
    i32     PopCount;
};

struct MDNodePtrNode {
    MDNodePtrNode* Next;
    MDNode*        V;
};

struct MDNodePtrList {
    MDNodePtrNode* Head;
    MDNodePtrNode* Tail;
    u64            Count;
};

struct MDTokeniseResult {
    MDTokenArray  Tokens;
    MDMessageList Messages;
};

struct MDParseResult {
    MDNode*       Root;
    MDMessageList Messages;
};

// @runtime____________________________________________________________________
extern MDNode* const EMPTY_MD_NODE_VALUE;

// @functions__________________________________________________________________
void MDInit(void);
void ListPush(Arena* MemPool, MDMessageList* Messages, MDNode* Node, MDMessageKind Kind, Str8 String);
void ListPush(Arena* MemPool, MDMessageList* Messages, MDNode* Node, MDMessageKind Kind, char* Fmt, ...);
void ListCat(MDMessageList* ListA, MDMessageList* ListB);
MDToken MDTokenAlloc(r1u64 Range, MDTokenKind Kind);
Str8List MDStrListFromTokenKind(Arena* MemPool, MDTokenKind Kind);
void ListPush(Arena* MemPool, MDTokenChunkList* List, u64 Capacity, MDToken Token);
MDTokenArray MDTokenArrayFromChunkList(Arena* MemPool, MDTokenChunkList* List);
Str8 MDContentStrFromTokenKind(MDTokenKind Kind, Str8 String);
MDNodeFlags MDNodeFlagsFromTokenKind(MDTokenKind Kind);
b32 MDNodeIsEmpty(MDNode* Node);
MDNodeRecord MDNodeRecordDepthFirst(MDNode* Node, MDNode* SubTreeRoot, u64 ChildOffset, u64 SiblingOffset);
MDNode* ArenaPushMDNode(Arena* MemPool, MDNodeKind Kind, MDNodeFlags Flags, Str8 String, Str8 RawString, u64 SrcOffset);
void MDNodeInsertChild(MDNode* Parent, MDNode* PrevChild, MDNode* Node);
void MDNodeInsertTag(MDNode* Parent, MDNode* PrevChild, MDNode* Node);
void MDNodePushChild(MDNode* Parent, MDNode* Node);
void MDNodePushTag(MDNode* Parent, MDNode* Node);
void MDUnhook(MDNode* Node);
MDNode* MDNodeFromChainStr(MDNode* Head, MDNode* End, Str8 String, StrMatchKind MatchKind);
MDNode* MDNodeFromChainIndex(MDNode* Head, MDNode* End, u64 Index);
MDNode* MDNodeFromChainFlags(MDNode* Head, MDNode* End, MDNodeFlags Flags);
u64 MDIndexFromNode(MDNode* Node);
MDNode* MDRootFromNode(MDNode* Node);
MDNode* MDChildFromStr(MDNode* Node, Str8 ChildString, StrMatchKind MatchKind);
MDNode* MDTagFromStr(MDNode* Node, Str8 TagString, StrMatchKind MatchKind);
MDNode* MDChildFromIndex(MDNode* Node, u64 Index);
MDNode* MDTagFromIndex(MDNode* Node, u64 Index);
MDNode* MDTagArgFromIndex(MDNode* Node, Str8 TagString, StrMatchKind MatchKind, u64 Index);
MDNode* MDTagArgFromStr(MDNode* Node, Str8 TagString, StrMatchKind TagMatchKind, Str8 ArgString, StrMatchKind ArgMatchKind);
b32 MDNodeHasChild(MDNode* Node, Str8 String, StrMatchKind MatchKind);
b32 MDNodeHasTag(MDNode* Node, Str8 String, StrMatchKind MatchKind);
u64 MDChildCountFromNode(MDNode* Node);
u64 MDTagCountFromNode(MDNode* Node);
Str8 MDStrFromChildren(Arena* MemPool, MDNode* Root);
b32 MDTreeMatch(MDNode* NodeA, MDNode* NodeB, StrMatchKind MatchKind);
b32 MDNodeMatch(MDNode* NodeA, MDNode* NodeB, StrMatchKind MatchKind);
MDNode* MDCopyTree(Arena* MemPool, MDNode* Root);
MDTokeniseResult MDTokeniseFromText(Arena* MemPool, Str8 Text);
MDParseResult MDParseFromTextTokens(Arena* MemPool, Str8 FileName, Str8 Text, MDTokenArray Tokens);
MDParseResult MDParseFromText(Arena* MemPool, Str8 FileName, Str8 Text);
Str8List MDDebugStrListFromTree(Arena* MemPool, MDNode* Root);
void ListPush(Arena* MemPool, MDNodePtrList* List, MDNode* Node);
void ListPushFront(Arena* MemPool, MDNodePtrList* List, MDNode* Node);

#endif // __NC_METADESK_H__
