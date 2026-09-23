#if !defined(__NC_UI_COMPONENTS_H__)
#define __NC_UI_COMPONENTS_H__

#include "nc_types.h"
#include "nc_ui.h"

// @defines____________________________________________________________________
typedef u32 UIScrollListKind;
enum : u32 {
    UI_SCROLL_LIST_KIND_NAV  = (1 << 0),
    UI_SCROLL_LIST_KIND_SNAP = (1 << 1),
    UI_SCROLL_LIST_KIND_ALL  = U32_MAX
};

typedef u32 UIToggleKind;
enum : u32 {
    UI_TOGGLE_KIND_CHECKBOX,
    UI_TOGGLE_KIND_SWITCH,
    UI_TOGGLE_KIND_COUNT
};

typedef u32 UISliderKind;
enum : u32 {
    UI_SLIDER_KIND_HORZ,
    UI_SLIDER_KIND_VERT,
    UI_SLIDER_KIND_COUNT
};

typedef u32 UIProgressKind;
enum : u32 {
    UI_PROGRESS_KIND_BAR,
    UI_PROGRESS_KIND_SPINNER,
    UI_PROGRESS_KIND_COUNT
};

#define UIRow()                        DEFER(UIRowBegin(), UIRowEnd())
#define UIColumn()                     DEFER(UIColumnBegin(), UIColumnEnd())
#define UINamedRow(X)                  DEFER(UINamedRowBegin(X), UINamedRowEnd())
#define UINamedColumn(X)               DEFER(UINamedColumnBegin(X), UINamedColumnEnd())
#define UIPane(X, Y)                   DEFER(UIPaneBegin(X, Y), UIPaneEnd())
#define UIPaneFmt(X, ...)              DEFER(UIPaneBegin(X, __VA_ARGS__), UIPaneEnd())
#define UIPadding(X)                   DEFER(UISpacer(X), UISpacer(X))
#define UICentre()                     UIPadding(UI_PERCENT(1.0f, 0.0f))
#define UITable(X, Y, Z)               DEFER(UITableBegin(X, Y, Z), UITableEnd())
#define UITableFmt(X, Y, ...)          DEFER(UITableBegin(X, Y, __VA_ARGS__), UITableEnd())
#define UINamedTableVector(X)          DEFER(UINamedTableVectorBegin(X), UINamedTableVectorEnd())
#define UINamedTableVectorFmt(...)     DEFER(UINamedTableVectorBegin(__VA_ARGS__), UINamedTableVectorEnd())
#define UITableVector()                DEFER(UITableVectorBegin(), UITableVectorEnd())
#define UITableCell()                  DEFER(UITableCellBegin(), UITableCellEnd())
#define UITableCellSized(X)            DEFER(UITableCellSizedBegin(), UITableCellSizedEnd())
#define UIScrollList(X, Y, Z, W, I, J) DEFER(UIScrollListBegin((X), (Y), (Z), (W), (I), (J)), UIScrollListEnd())
#define UIRadioGroup(X, Y, Z)          DEFER(UIRadioGroupBegin((X), (Y), (Z)), UIRadioGroupEnd())
#define UITreeNode(X, Y, Z)            DEFER(UITreeNodeBegin((X), (Y), (Z)), UITreeNodeEnd())
#define UIBreadCrumb(X, Y)             DEFER(UIBreadCrumBegin((X), (Y)), UIBreadCrumbEnd())

// @types______________________________________________________________________
struct UIScrollListRowBlock {
    u64 RowCount;
    u64 ItemCount;
};

struct UIScrollListRowBlockChunkNode {
    UIScrollListRowBlockChunkNode* Next;
    UIScrollListRowBlock*          V;
    u64                            Count;
    u64                            Capacity;
};

struct UIScrollListRowBlockChunkList {
    UIScrollListRowBlockChunkNode* Head;
    UIScrollListRowBlockChunkNode* Tail;
    u64                            ChunkCount;
    u64                            TotalCount;
};

struct UIScrollListRowBlockArray {
    UIScrollListRowBlock* V;
    u64                   Count;
};

struct UIScrollListParameters {
    UIScrollListKind          Kind;
    v2f32                     DimensionsPX;
    f32                       RowHeightPX;
    UIScrollListRowBlockArray RowBlocks;
    r2i64                     CursorRange;
    r1i64                     ItemRange;
    b32                       CursorMinIsEmptySelection[AXIS_2D_COUNT];
};

struct UIScrollListSignal {
    b32 CursorMoved;
};

struct UISearchBoxResult {
    UISignal Sig;
    b32      Comitted;
    i64      SelectedIndex;
};

struct UILineEditDrawData {
    Str8      EdittedString;
    TextPoint Cursor;
    TextPoint Mark;
    b32       Trail;
};

struct UISliderResult {
    UISignal Sig;
    f32      Value;
    b32      Changed;
};

struct UIDropDownResult {
    UISignal Sig;
    i64      SelectedIndex;
    b32      Changed;
};

struct UINumericEditResult {
    UISignal Sig;
    f64      Value;
    b32      Changed;
    b32      Comitted;
};

struct UITreeNodeState {
    b32 IsExpanded;
    b32 WasToggled;
};

struct UIDividerDrawData {
    Axis2D Axis;
};

struct UIScrollBarDrawData {
    Axis2D Axis;
    f32    BeforeFrac;
    f32    ScrollerFrac;
    UIBox* ScrollerBox;
};

struct UIToggleSwitchDrawData {
    f32 EnabledT;
};

struct UISliderDrawData {
    f32          Value;
    r1f32        Range;
    UISliderKind Kind;
};

struct UIProgressBarDrawData {
    f32 Progress;
};

struct UIScrollBarDragData {
    UIScrollPoint StartPoint;
    f32           ScrollSpacePX;
};

// @runtime____________________________________________________________________


// @functions__________________________________________________________________
void UIDivider(UISize Size);
UISignal UILabel(Str8 String);
UISignal UILabel(char* Fmt, ...);
void UILabelMultiline(f32 Max, Str8 String);
void UILabelMultiline(f32 Max, char* Fmt, ...);
UISignal UIButton(Str8 String);
UISignal UIButton(char* Fmt, ...);
UISignal UIHoverLabel(Str8 String);
UISignal UIHoverLabel(char* Fmt, ...);
UISignal UILineEdit(TextPoint* Cursor, TextPoint* Mark, u8* EditBuffer, u64 EditBufferSize, OUT u64* EditStrSize, Str8 PreEditValue, Str8 String);
UISignal UILineEdit(TextPoint* Cursor, TextPoint* Mark, u8* EditBuffer, u64 EditBufferSize, OUT u64* EditStrSize, Str8 PreEditValue, char* Fmt, ...);
UISignal UIExpander(b32 IsExpanded, Str8 String);
UISignal UIExpander(b32 IsExpanded, char* Fmt, ...);
UISignal UISortHeader(b32 Sorting, b32 Ascending, Str8 String);
UISignal UISortHeader(b32 Sorting, b32 Ascending, char* Fmt, ...);
UIBox* UIRowBegin(void);
UISignal UIRowEnd(void);
UIBox* UIColumnBegin(void);
UISignal UIColumnEnd(void);
UIBox* UINamedRowBegin(Str8 String);
UISignal UINamedRowEnd(void);
UIBox* UINamedColumnBegin(Str8 String);
UISignal UINamedColumnEnd(void);
UIBox* UIPaneBegin(r2f32 Rect, Str8 String);
UIBox* UIPaneBegin(r2f32 Rect, char* Fmt, ...);
UISignal UIPaneEnd(void);
void UITableBegin(u64 ColumnPercentsCount, f32** ColumnPercents, Str8 String);
void UITableBegin(u64 ColumnPercentsCount, f32** ColumnPercents, char* Fmt, ...);
void UITableEnd(void);
UIBox* UINamedTableVectorBegin(Str8 String);
UIBox* UINamedTableVectorBegin(char* Fmt, ...);
UIBox* UITableVectorBegin(void);
UISignal UITableVectorEnd(void);
UIBox* UITableCellBegin(void);
UISignal UITableCellEnd(void);
UIBox* UITableCellSizedBegin(UISize Size);
void ListPush(Arena* MemPool, UIScrollListRowBlockChunkList* List, u64 Capacity, UIScrollListRowBlock* Block);
UIScrollListRowBlockArray UIScrollListRowBlockArrayFromChunkList(Arena* MemPool, UIScrollListRowBlockChunkList* List);
u64 UIScrollListRowFromItem(UIScrollListRowBlockArray* Blocks, u64 Item);
u64 UIScrollListItemFromRow(UIScrollListRowBlockArray* Blocks, u64 Row);
UIScrollPoint UIScrollBar(Axis2D Axis, UISize OffsetAxisSize, UIScrollPoint Point, r1i64 IndexRange, i64 ViewNumIndices);
void UIScrollListBegin(UIScrollListParameters* Params, OUT UIScrollPoint* ScrollPoint, OUT v2i64* Cursor, OUT v2i64* Mark, OUT r1i64* VisibleRowRange, OUT UIScrollListSignal* Signal);
void UIScrollListEnd(void);
UISignal UICheckBox(b32* Checked, Str8 String);
UISignal UICheckBox(b32* Checked, char* Fmt, ...);
UISignal UIToggle(b32* Enabled, UIToggleKind Kind, Str8 String);
UISignal UIToggle(b32* Enabled, UIToggleKind Kind, char* Fmt, ...);
UISliderResult UISlider(f32* Value, r1f32 Range, UISliderKind Kind, Str8 String);
UISliderResult UISlider(f32* Value, r1f32 Range, UISliderKind Kind, char* Fmt, ...);
UISliderResult UISlider(i32* Value, r1i32 Range, UISliderKind Kind, Str8 String);
UISliderResult UISlider(i32* Value, r1i32 Range, UISliderKind Kind, char* Fmt, ...);
UIDropDownResult UIDropDown(i64* SelectedIndex, Str8Array Options, Str8 String);
UIDropDownResult UIDropDown(i64* SelectedIndex, Str8Array Options, char* Fmt, ...);
UIDropDownResult UIComboBox(i64* SelectedIndex, Str8Array Options, b32* IsOpen, Str8 String);
UIDropDownResult UIComboBox(i64* SelectedIndex, Str8Array Options, b32* IsOpen, char* Fmt, ...);
UINumericEditResult UINumericEdit(f64* Value, r1f64 Range, f64 Step, Str8 String);
UINumericEditResult UINumericEdit(f64* Value, r1f64 Range, f64 Step, char* Fmt, ...);
UINumericEditResult UINumericEdit(i64* Value, r1i64 Range, i64 Step, Str8 String);
UINumericEditResult UINumericEdit(i64* Value, r1i64 Range, i64 Step, char* Fmt, ...);
UISignal UIProgressBar(f32 Progress, Str8 String);
UISignal UIProgressBar(f32 Progress, char* Fmt, ...);
UISignal UIProgressSpinner(f32 SpinRate, Str8 String);
UISignal UIProgressSpinner(f32 SpinRate, char* Fmt, ...);
b32 UITreeNodeBegin(b32* IsExpanded, b32 HasChildren, Str8 String);
b32 UITreeNodeBegin(b32* IsExpanded, b32 HasChildren, char* Fmt, ...);
void UITreeNodeEnd(void);
void UITreeNodeLeaf(Str8 String);
void UITreeNodeLeaf(char* Fmt, ...);
void UIRadioGroupBegin(i64* SelectedIndex, i64 OptionCount, Str8 String);
void UIRadioGroupBegin(i64* SelectedIndex, i64 OptionCount, char* Fmt, ...);
UISignal UIRadioOption(i64 Index, Str8 String);
UISignal UIRadioOption(i64 Index, char* Fmt, ...);
void UIRadioGroupEnd(void);
UIBox* UIBreadCrumbBegin(i64 Count, Str8 String);
UIBox* UIBreadCrumbBegin(i64 Count, char* Fmt, ...);
UISignal UIBreadCrumbItem(i64 Index, Str8 String);
UISignal UIBreadCrumbItem(i64 Index, char* Fmt, ...);
void UIBreadCrumbEnd(void);

INTERNAL UI_BOX_CUSTOM_DRAW(UIDividerDraw);
INTERNAL UI_BOX_CUSTOM_DRAW(UILineEditDraw);
INTERNAL UI_BOX_CUSTOM_DRAW(UISliderDraw);
INTERNAL UI_BOX_CUSTOM_DRAW(UIToggleSwitchDraw);
INTERNAL UI_BOX_CUSTOM_DRAW(UIProgressBarDraw);
INTERNAL UI_BOX_CUSTOM_DRAW(UIScrollBarDraw);

#endif // __NC_UI_COMPONENTS_H__
