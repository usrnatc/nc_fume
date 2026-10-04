#if !defined(__NC_CONFIG_H__)
#define __NC_CONFIG_H__

#include "nc_types.h"
#include "nc_string.h"
#include "nc_metadesk.h"
#include "nc_console.h"

// @defines____________________________________________________________________
typedef u64 ConfigID;

#define ConfigHeadNode(X) ((X)->Count ? (X)->Head->V : EMPTY_CFG_NODE_VALUE)
#define ConfigTailNode(X) ((X)->Count ? (X)->Tail->V : EMPTY_CFG_NODE_VALUE)

#define ConfigPanelNodeRecDepthFirstPre(X, Y) ConfigPanelNodeRecDepthFirst((X), (Y), OFFSETOF(ConfigPanelNode, Next), OFFSETOF(ConfigPanelNode, Head))
#define ConfigPanelNodeRecDepthFirstPreRev(X, Y) ConfigPanelNodeRecDepthFirst((X), (Y), OFFSETOF(ConfigPanelNode, Prev), OFFSETOF(ConfigPanelNode, Tail))

// @types______________________________________________________________________
struct ConfigIDNode {
    ConfigIDNode* Next;
    ConfigID      V;
};

struct ConfigIDList {
    ConfigIDNode* Head;
    ConfigIDNode* Tail;
    u64           Count;
};

struct ConfigNode {
    ConfigNode* Head;
    ConfigNode* Tail;
    ConfigNode* Next;
    ConfigNode* Prev;
    ConfigNode* Parent;
    ConfigID    ID;
    Str8        String;
};

struct ConfigNodePtrNode {
    ConfigNodePtrNode* Next;
    ConfigNodePtrNode* Prev;
    ConfigNode*        V;
};

struct ConfigNodePtrSlot {
    ConfigNodePtrNode* Head;
    ConfigNodePtrNode* Tail;
};

struct ConfigNodePtrList {
    ConfigNodePtrNode* Head;
    ConfigNodePtrNode* Tail;
    u64                Count;
};

struct ConfigNodePtrArray {
    ConfigNode** V;
    u64          Count;
};

struct ConfigNodeRecord {
    ConfigNode* Next;
    i32         PushCount;
    i32         PopCount;
};

struct ConfigStrChunkNode {
    ConfigStrChunkNode* Next;
    u64                 Size;
};

struct ConfigContext {
    ConfigNode*        Root;
    u64                IDSlotsCount;
    ConfigNodePtrSlot* IDSlots;
    u64                ChangeGeneration;
    ConfigID           PrevAccessedID;
    ConfigNode*        PrevAccessed;
};

struct ConfigPanelNode {
    ConfigPanelNode*  Head;
    ConfigPanelNode*  Tail;
    ConfigPanelNode*  Next;
    ConfigPanelNode*  Prev;
    ConfigPanelNode*  Parent;
    u64               ChildCount;
    ConfigNode*       Config;
    Axis2D            SplitAxis;
    f32               PercentOfParent;
    SideKind          TabSide;
    ConfigNodePtrList Tabs;
    ConfigNode*       SelectedTab;
};

struct ConfigPanelTree {
    ConfigPanelNode* Root;
    ConfigPanelNode* Focused;
};

struct ConfigPanelNodeRecord {
    ConfigPanelNode* Next;
    i32              PushCount;
    i32              PopCount;
};

// @runtime____________________________________________________________________
readonly global u64 CFG_STRING_BUCKET_CHUNK_SIZES[] = {
    16,
    64,
    256,
    1024,
    4096,
    16384,
    65536,
    U64_MAX
};

// @types______________________________________________________________________
struct ConfigState {
    Arena*                  MemPool;
    ConfigNode*             Free;
    ConfigNodePtrNode*      FreeIDNode;
    ConfigStrChunkNode*     FreeStringChunks[ARRAY_COUNT(CFG_STRING_BUCKET_CHUNK_SIZES)];
    u64                     IDGeneration;
    ConfigContext           Context;
};

struct ConfigSchemaNode {
    ConfigSchemaNode* Next;
    Str8              Name;
    MDNode*           Schema;
};

struct ConfigSchemaTable {
    ConfigSchemaNode** Slots;
    u64                SlotsCount;
};

struct ConfigBinding {
    InputKind         Input;
    InputModifierKind ModKind;
};

struct ConfigInputMapNode {
    ConfigInputMapNode* NameHashNext;
    ConfigInputMapNode* BindingHashNext;
    ConfigID            ID;
    Str8                Name;
    ConfigBinding       Binding;
};

struct ConfigInputMapNodePtr {
    ConfigInputMapNodePtr* Next;
    ConfigInputMapNode*    V;
};

struct ConfigInputMapNodePtrList {
    ConfigInputMapNodePtr* Head;
    ConfigInputMapNodePtr* Tail;
    u64                    Count;
};

struct ConfigInputMapSlot {
    ConfigInputMapNode* Head;
    ConfigInputMapNode* Tail;
};

struct ConfigInputMap {
    u64                 NameSlotsCount;
    ConfigInputMapSlot* NameSlots;
    u64                 BindingSlotsCount;
    ConfigInputMapSlot* BindingSlots;
};

// @runtime____________________________________________________________________
extern ConfigNode* const EMPTY_CFG_NODE_VALUE;
extern ConfigPanelNode* const EMPTY_CFG_PANEL_NODE_VALUE;

// @functions__________________________________________________________________
void ConfigInit(void);
void ListPush(Arena* MemPool, ConfigIDList* List, ConfigID ID);
ConfigIDList ListCpy(Arena* MemPool, ConfigIDList* List);
void ListPush(Arena* MemPool, ConfigNodePtrList* List, ConfigNode* Node);
void ListPushFront(Arena* MemPool, ConfigNodePtrList* Lisdt, ConfigNode* Node);
ConfigNodePtrArray ConfigNodePtrArrayFromList(Arena* MemPool, ConfigNodePtrList* List);
void ConfigSchemaTableInsert(Arena* MemPool, ConfigSchemaTable* Table, Str8 Name, MDNode* Schema);
MDNode* ConfigSchemaFromName(ConfigSchemaTable* Table, Str8 Name);
MDNodePtrList ConfigSchemasFromName(Arena* MemPool, ConfigSchemaTable* Table, Str8 Name);
void ConfigContextSelect(ConfigContext* Ctx);
u64 ConfigChangeGeneration(void);
ConfigNode* ConfigNodeRoot(void);
ConfigNode* ConfigNodeFromID(ConfigID ID);
ConfigNode* ConfigNodeChildFromStr(ConfigNode* Parent, Str8 String);
ConfigNode* ConfigNodeChildFromStrOrParent(ConfigNode* Parent, Str8 String);
ConfigNodePtrList ConfigNodeChildListFromStr(Arena* MemPool, ConfigNode* Parent, Str8 String);
ConfigNodePtrList ConfigNodeTopLevelListFromStr(Arena* MemPool, Str8 String);
ConfigNodeRecord ConfigNodeRecDepthFirst(ConfigNode* Root, ConfigNode* Node);
Str8 ConfigStrFromTree(Arena* MemPool, ConfigSchemaTable* SchemaTable, Str8 RootPath, ConfigNode* Root);
ConfigState* ConfigStateAlloc(void);
void ConfigStateRelease(ConfigState* State);
ConfigContext* ConfigContextFromState(ConfigState* State);
u64 ConfigStrBucketNumFromSize(u64 Size);
Str8 ConfigStrAlloc(ConfigState* State, Str8 String);
void ConfigStrRelease(ConfigState* State, Str8 String);
ConfigNode* ConfigNodeAlloc(ConfigState* State);
void ConfigNodeRelease(ConfigState* State, ConfigNode* Node);
void ConfigNodeReleaseAllChildren(ConfigState* State, ConfigNode* Node);
ConfigNode* ConfigNodeNew(ConfigState* State, ConfigNode* Parent, Str8 String);
ConfigNode* ConfigNodeNew(ConfigState* State, ConfigNode* Parent, char* Fmt, ...);
ConfigNode* ConfigNodeNewReplace(ConfigState* State, ConfigNode* Parent, Str8 String);
ConfigNode* ConfigNodeNewReplace(ConfigState* State, ConfigNode* Parent, char* Fmt, ...);
ConfigNode* ConfigNodeCpy(ConfigState* State, ConfigNode* Root);
void ConfigNodeEquipStr(ConfigState* State, ConfigNode* Node, Str8 String);
void ConfigNodeEquipStr(ConfigState* State, ConfigNode* Node, char* Fmt, ...);
void ConfigNodeInsertChild(ConfigState* State, ConfigNode* Parent, ConfigNode* PrevChild, ConfigNode* NewChild);
void ConfigNodeUnhook(ConfigState* State, ConfigNode* Parent, ConfigNode* Child);
ConfigNode* ConfigNodeChildFromStrOrAlloc(ConfigState* State, ConfigNode* Parent, Str8 String);
ConfigNodePtrList ConfigNodePtrListFromStr(Arena* MemPool, ConfigState* State, ConfigSchemaTable* SchemaTable, Str8 RootPath, Str8 String);
ConfigInputMap* ConfigInputMapFromConfig(Arena* MemPool);
ConfigInputMapNodePtrList ConfigInputMapNodePtrListFromName(Arena* MemPool, ConfigInputMap* InputMap, Str8 String);
ConfigInputMapNodePtrList ConfigInputMapNodePtrListFromBinding(Arena* MemPool, ConfigInputMap* InputMap, ConfigBinding Binding);
ConfigNode* ConfigWindowFromConfig(ConfigNode* Config);
ConfigPanelTree ConfigPanelTreeFromConfig(Arena* MemPool, ConfigNode* CfgRoot);
ConfigPanelNodeRecord ConfigPanelNodeRecDepthFirst(ConfigPanelNode* Root, ConfigPanelNode* Panel, u64 SiblingOffset, u64 ChildOffset);
ConfigPanelNode* ConfigPanelNodeFromConfigTree(ConfigPanelNode* Root, ConfigNode* Cfg);
r2f32 ConfigTargetRectFromPanelNodeChild(r2f32 ParentRect, ConfigPanelNode* Parent, ConfigPanelNode* Panel);
r2f32 ConfigTargetRectFromPanelNode(r2f32 RootRect, ConfigPanelNode* Root, ConfigPanelNode* Panel);

#endif // __NC_CONFIG_H__
