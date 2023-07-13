#pragma once
#include "PandorAPI.h"

#include "Math/Maths.h"

#include <unordered_map>

struct GLFWwindow;


#pragma region Flags & Enumerations
enum class PANDOR_API WindowFlags
{
	None = 0,
	NoTitleBar = 1 << 0,   // Disable title-bar
	NoResize = 1 << 1,   // Disable user resizing with the lower-right grip
	NoMove = 1 << 2,   // Disable user moving the window
	NoScrollbar = 1 << 3,   // Disable scrollbars (window can still scroll with mouse or programmatically)
	NoScrollWithMouse = 1 << 4,   // Disable user vertically scrolling with mouse wheel. On child window, mouse wheel will be forwarded to the parent unless NoScrollbar is also set.
	NoCollapse = 1 << 5,   // Disable user collapsing window by double-clicking on it. Also referred to as Window Menu Button (e.g. within a docking node).
	AlwaysAutoResize = 1 << 6,   // Resize every window to its content every frame
	NoBackground = 1 << 7,   // Disable drawing background color (WindowBg, etc.) and outside border. Similar as using SetNextWindowBgAlpha(0.0f).
	NoSavedSettings = 1 << 8,   // Never load/save settings in .ini file
	NoMouseInputs = 1 << 9,   // Disable catching mouse, hovering test with pass through.
	MenuBar = 1 << 10,  // Has a menu-bar
	HorizontalScrollbar = 1 << 11,  // Allow horizontal scrollbar to appear (off by default). You may use SetNextWindowContentSize(Math::Vector2(width,0.0f)); prior to calling Begin() to specify width. Read code in imgui_demo in the "Horizontal Scrolling" section.
	NoFocusOnAppearing = 1 << 12,  // Disable taking focus when transitioning from hidden to visible state
	NoBringToFrontOnFocus = 1 << 13,  // Disable bringing window to front when taking focus (e.g. clicking on it or programmatically giving it focus)
	AlwaysVerticalScrollbar = 1 << 14,  // Always show vertical scrollbar (even if ContentSize.y < Size.y)
	AlwaysHorizontalScrollbar = 1 << 15,  // Always show horizontal scrollbar (even if ContentSize.x < Size.x)
	AlwaysUseWindowPadding = 1 << 16,  // Ensure child windows without border uses style.WindowPadding (ignored by default for non-bordered child windows, because more convenient)
	NoNavInputs = 1 << 18,  // No gamepad/keyboard navigation within the window
	NoNavFocus = 1 << 19,  // No focusing toward this window with gamepad/keyboard navigation (e.g. skipped by CTRL+TAB)
	UnsavedDocument = 1 << 20,  // Display a dot next to the title. When used in a tab/docking context, tab is selected when clicking the X + closure is not assumed (will wait for user to stop submitting the tab). Otherwise closure is assumed when pressing the X, so if you keep submitting the tab may reappear at end of tab bar.
	NoDocking = 1 << 21,  // Disable docking of this window

	NoNav = NoNavInputs | NoNavFocus,
	NoDecoration = NoTitleBar | NoResize | NoScrollbar | NoCollapse,
	NoInputs = NoMouseInputs | NoNavInputs | NoNavFocus,

	// [Internal]
	NavFlattened = 1 << 23,  // [BETA] On child window: allow gamepad/keyboard navigation to cross over parent border to this child or between sibling child windows.
	ChildWindow = 1 << 24,  // Don't use! For internal use by BeginChild()
	Tooltip = 1 << 25,  // Don't use! For internal use by BeginTooltip()
	Popup = 1 << 26,  // Don't use! For internal use by BeginPopup()
	Modal = 1 << 27,  // Don't use! For internal use by BeginPopupModal()
	ChildMenu = 1 << 28,  // Don't use! For internal use by BeginMenu()
	DockNodeHost = 1 << 29,  // Don't use! For internal use by Begin()/NewFrame()
};

enum class PANDOR_API InputTextFlags
{
	None = 0,
	CharsDecimal = 1 << 0,   // Allow 0123456789.+-*/
	CharsHexadecimal = 1 << 1,   // Allow 0123456789ABCDEFabcdef
	CharsUppercase = 1 << 2,   // Turn a..z into A..Z
	CharsNoBlank = 1 << 3,   // Filter out spaces, tabs
	AutoSelectAll = 1 << 4,   // Select entire text when first taking mouse focus
	EnterReturnsTrue = 1 << 5,   // Return 'true' when Enter is pressed (as opposed to every time the value was modified). Consider looking at the IsItemDeactivatedAfterEdit() function.
	CallbackCompletion = 1 << 6,   // Callback on pressing TAB (for completion handling)
	CallbackHistory = 1 << 7,   // Callback on pressing Up/Down arrows (for history handling)
	CallbackAlways = 1 << 8,   // Callback on each iteration. User code may query cursor position, modify text buffer.
	CallbackCharFilter = 1 << 9,   // Callback on character inputs to replace or discard them. Modify 'EventChar' to replace or discard, or return 1 in callback to discard.
	AllowTabInput = 1 << 10,  // Pressing TAB input a '\t' character into the text field
	CtrlEnterForNewLine = 1 << 11,  // In multi-line mode, unfocus with Enter, add new line with Ctrl+Enter (default is opposite: unfocus with Ctrl+Enter, add line with Enter).
	NoHorizontalScroll = 1 << 12,  // Disable following the cursor horizontally
	AlwaysOverwrite = 1 << 13,  // Overwrite mode
	ReadOnly = 1 << 14,  // Read-only mode
	Password = 1 << 15,  // Password mode, display all characters as '*'
	NoUndoRedo = 1 << 16,  // Disable undo/redo. Note that input text owns the text data while active, if you want to provide your own undo/redo stack you need e.g. to call ClearActiveID().
	CharsScientific = 1 << 17,  // Allow 0123456789.+-*/eE (Scientific notation input)
	CallbackResize = 1 << 18,  // Callback on buffer capacity changes request (beyond 'buf_size' parameter value), allowing the string to grow. Notify when the string wants to be resized (for string types which hold a cache of their Size). You will be provided a new BufSize in the callback and NEED to honor it. (see misc/cpp/imgui_stdlib.h for an example of using this)
	CallbackEdit = 1 << 19,  // Callback on any edit (note that InputText() already returns true on edit, the callback is useful mainly to manipulate the underlying buffer while focus is active)
	EscapeClearsAll = 1 << 20,  // Escape key clears content if not empty, and deactivate otherwise (contrast to default behavior of Escape to revert)

	// Obsolete names (will be removed soon)
#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
	AlwaysInsertMode = AlwaysOverwrite   // [renamed in 1.82] name was not matching behavior
#endif
};

enum class PANDOR_API TreeNodeFlags
{
	None = 0,
	Selected = 1 << 0,   // Draw as selected
	Framed = 1 << 1,   // Draw frame with background (e.g. for CollapsingHeader)
	AllowItemOverlap = 1 << 2,   // Hit testing to allow subsequent widgets to overlap this one
	NoTreePushOnOpen = 1 << 3,   // Don't do a TreePush() when open (e.g. for CollapsingHeader) = no extra indent nor pushing on ID stack
	NoAutoOpenOnLog = 1 << 4,   // Don't automatically and temporarily open node when Logging is active (by default logging will automatically open tree nodes)
	DefaultOpen = 1 << 5,   // Default node to be open
	OpenOnDoubleClick = 1 << 6,   // Need double-click to open node
	OpenOnArrow = 1 << 7,   // Only open when clicking on the arrow part. If OpenOnDoubleClick is also set, single-click arrow or double-click all box to open.
	Leaf = 1 << 8,   // No collapsing, no arrow (use as a convenience for leaf nodes).
	Bullet = 1 << 9,   // Display a bullet instead of arrow
	FramePadding = 1 << 10,  // Use FramePadding (even for an unframed text node) to vertically align text baseline to regular widget height. Equivalent to calling AlignTextToFramePadding().
	SpanAvailWidth = 1 << 11,  // Extend hit box to the right-most edge, even if not framed. This is not the default in order to allow adding other items on the same line. In the future we may refactor the hit system to be front-to-back, allowing natural overlaps and then this can become the default.
	SpanFullWidth = 1 << 12,  // Extend hit box to the left-most and right-most edges (bypass the indented area).
	NavLeftJumpsBackHere = 1 << 13,  // (WIP) Nav: left direction may move to this TreeNode() from any of its child (items submitted between TreeNode and TreePop)
	//NoScrollOnOpen     = 1 << 14,  // FIXME: TODO: Disable automatic scroll on TreePop() if node got just open and contents is not visible
	CollapsingHeader = Framed | NoTreePushOnOpen | NoAutoOpenOnLog,
};

enum class PANDOR_API PopupFlags
{
	None = 0,
	MouseButtonLeft = 0,        // For BeginPopupContext*(): open on Left Mouse release. Guaranteed to always be == 0 (same as MouseButton_Left)
	MouseButtonRight = 1,        // For BeginPopupContext*(): open on Right Mouse release. Guaranteed to always be == 1 (same as MouseButton_Right)
	MouseButtonMiddle = 2,        // For BeginPopupContext*(): open on Middle Mouse release. Guaranteed to always be == 2 (same as MouseButton_Middle)
	MouseButtonMask_ = 0x1F,
	MouseButtonDefault_ = 1,
	NoOpenOverExistingPopup = 1 << 5,   // For OpenPopup*(), BeginPopupContext*(): don't open if there's already a popup at the same level of the popup stack
	NoOpenOverItems = 1 << 6,   // For BeginPopupContextWindow(): don't return true when hovering items, only when hovering empty space
	AnyPopupId = 1 << 7,   // For IsPopupOpen(): ignore the ID parameter and test for any popup.
	AnyPopupLevel = 1 << 8,   // For IsPopupOpen(): search/test at any level of the popup stack (default test in the current level)
	AnyPopup = AnyPopupId | AnyPopupLevel,
};

enum class PANDOR_API SelectableFlags
{
	None = 0,
	DontClosePopups = 1 << 0,   // Clicking this doesn't close parent popup window
	SpanAllColumns = 1 << 1,   // Selectable frame can span all columns (text will still fit in current column)
	AllowDoubleClick = 1 << 2,   // Generate press events on double clicks too
	Disabled = 1 << 3,   // Cannot be selected, display grayed out text
	AllowItemOverlap = 1 << 4,   // (WIP) Hit testing to allow subsequent widgets to overlap this one
};

enum class PANDOR_API SelectableFlagsPrivate
{
	// NB: need to be in sync with last value of SelectableFlags_
	NoHoldingActiveID = 1 << 20,
	SelectOnNav = 1 << 21,  // (WIP) Auto-select when moved into. This is not exposed in public API as to handle multi-select and modifiers we will need user to explicitly control focus scope. May be replaced with a BeginSelection() API.
	SelectOnClick = 1 << 22,  // Override button behavior to react on Click (default is Click+Release)
	SelectOnRelease = 1 << 23,  // Override button behavior to react on Release (default is Click+Release)
	SpanAvailWidth = 1 << 24,  // Span all avail width even if we declared less for layout purpose. FIXME: We may be able to remove this (added in 6251d379, 2bcafc86 for menus)
	DrawHoveredWhenHeld = 1 << 25,  // Always show active when held, even is not hovered. This concept could probably be renamed/formalized somehow.
	SetNavIdOnHover = 1 << 26,  // Set Nav/Focus ID on mouse hover (used by MenuItem)
	NoPadWithHalfSpacing = 1 << 27,  // Disable padding each side with ItemSpacing * 0.5f
};

enum class PANDOR_API ComboFlags
{
	None = 0,
	PopupAlignLeft = 1 << 0,   // Align the popup toward the left by default
	HeightSmall = 1 << 1,   // Max ~4 items visible. Tip: If you want your combo popup to be a specific size you can use SetNextWindowSizeConstraints() prior to calling BeginCombo()
	HeightRegular = 1 << 2,   // Max ~8 items visible (default)
	HeightLarge = 1 << 3,   // Max ~20 items visible
	HeightLargest = 1 << 4,   // As many fitting items as possible
	NoArrowButton = 1 << 5,   // Display on the preview box without the square arrow button
	NoPreview = 1 << 6,   // Display only a square arrow button
	HeightMask_ = HeightSmall | HeightRegular | HeightLarge | HeightLargest,
};

enum class PANDOR_API TabBarFlags
{
	None = 0,
	Reorderable = 1 << 0,   // Allow manually dragging tabs to re-order them + New tabs are appended at the end of list
	AutoSelectNewTabs = 1 << 1,   // Automatically select new tabs when they appear
	TabListPopupButton = 1 << 2,   // Disable buttons to open the tab list popup
	NoCloseWithMiddleMouseButton = 1 << 3,   // Disable behavior of closing tabs (that are submitted with p_open != NULL) with middle mouse button. You can still repro this behavior on user's side with if (IsItemHovered() && IsMouseClicked(2)) *p_open = false.
	NoTabListScrollingButtons = 1 << 4,   // Disable scrolling buttons (apply when fitting policy is FittingPolicyScroll)
	NoTooltip = 1 << 5,   // Disable tooltips when hovering a tab
	FittingPolicyResizeDown = 1 << 6,   // Resize tabs when they don't fit
	FittingPolicyScroll = 1 << 7,   // Add scroll buttons when tabs don't fit
	FittingPolicyMask_ = FittingPolicyResizeDown | FittingPolicyScroll,
	FittingPolicyDefault_ = FittingPolicyResizeDown,
};

enum class PANDOR_API TabItemFlags
{
	None = 0,
	UnsavedDocument = 1 << 0,   // Display a dot next to the title + tab is selected when clicking the X + closure is not assumed (will wait for user to stop submitting the tab). Otherwise closure is assumed when pressing the X, so if you keep submitting the tab may reappear at end of tab bar.
	SetSelected = 1 << 1,   // Trigger flag to programmatically make the tab selected when calling BeginTabItem()
	NoCloseWithMiddleMouseButton = 1 << 2,   // Disable behavior of closing tabs (that are submitted with p_open != NULL) with middle mouse button. You can still repro this behavior on user's side with if (IsItemHovered() && IsMouseClicked(2)) *p_open = false.
	NoPushId = 1 << 3,   // Don't call PushID(tab->ID)/PopID() on BeginTabItem()/EndTabItem()
	NoTooltip = 1 << 4,   // Disable tooltip for the given tab
	NoReorder = 1 << 5,   // Disable reordering this tab or having another tab cross over this tab
	Leading = 1 << 6,   // Enforce the tab position to the left of the tab bar (after the tab list popup button)
	Trailing = 1 << 7,   // Enforce the tab position to the right of the tab bar (before the scrolling buttons)
};

enum class PANDOR_API TableFlags
{
	// Features
	None = 0,
	Resizable = 1 << 0,   // Enable resizing columns.
	Reorderable = 1 << 1,   // Enable reordering columns in header row (need calling TableSetupColumn() + TableHeadersRow() to display headers)
	Hideable = 1 << 2,   // Enable hiding/disabling columns in context menu.
	Sortable = 1 << 3,   // Enable sorting. Call TableGetSortSpecs() to obtain sort specs. Also see SortMulti and SortTristate.
	NoSavedSettings = 1 << 4,   // Disable persisting columns order, width and sort settings in the .ini file.
	ContextMenuInBody = 1 << 5,   // Right-click on columns body/contents will display table context menu. By default it is available in TableHeadersRow().
	// Decorations
	RowBg = 1 << 6,   // Set each RowBg color with TableRowBg or TableRowBgAlt (equivalent of calling TableSetBgColor with TableBgFlags_RowBg0 on each row manually)
	BordersInnerH = 1 << 7,   // Draw horizontal borders between rows.
	BordersOuterH = 1 << 8,   // Draw horizontal borders at the top and bottom.
	BordersInnerV = 1 << 9,   // Draw vertical borders between columns.
	BordersOuterV = 1 << 10,  // Draw vertical borders on the left and right sides.
	BordersH = BordersInnerH | BordersOuterH, // Draw horizontal borders.
	BordersV = BordersInnerV | BordersOuterV, // Draw vertical borders.
	BordersInner = BordersInnerV | BordersInnerH, // Draw inner borders.
	BordersOuter = BordersOuterV | BordersOuterH, // Draw outer borders.
	Borders = BordersInner | BordersOuter,   // Draw all borders.
	NoBordersInBody = 1 << 11,  // [ALPHA] Disable vertical borders in columns Body (borders will always appear in Headers). -> May move to style
	NoBordersInBodyUntilResize = 1 << 12,  // [ALPHA] Disable vertical borders in columns Body until hovered for resize (borders will always appear in Headers). -> May move to style
	// Sizing Policy (read above for defaults)
	SizingFixedFit = 1 << 13,  // Columns default to _WidthFixed or _WidthAuto (if resizable or not resizable), matching contents width.
	SizingFixedSame = 2 << 13,  // Columns default to _WidthFixed or _WidthAuto (if resizable or not resizable), matching the maximum contents width of all columns. Implicitly enable NoKeepColumnsVisible.
	SizingStretchProp = 3 << 13,  // Columns default to _WidthStretch with default weights proportional to each columns contents widths.
	SizingStretchSame = 4 << 13,  // Columns default to _WidthStretch with default weights all equal, unless overridden by TableSetupColumn().
	// Sizing Extra Options
	NoHostExtendX = 1 << 16,  // Make outer width auto-fit to columns, overriding outer_size.x value. Only available when ScrollX/ScrollY are disabled and Stretch columns are not used.
	NoHostExtendY = 1 << 17,  // Make outer height stop exactly at outer_size.y (prevent auto-extending table past the limit). Only available when ScrollX/ScrollY are disabled. Data below the limit will be clipped and not visible.
	NoKeepColumnsVisible = 1 << 18,  // Disable keeping column always minimally visible when ScrollX is off and table gets too small. Not recommended if columns are resizable.
	PreciseWidths = 1 << 19,  // Disable distributing remainder width to stretched columns (width allocation on a 100-wide table with 3 columns: Without this flag: 33,33,34. With this flag: 33,33,33). With larger number of columns, resizing will appear to be less smooth.
	// Clipping
	NoClip = 1 << 20,  // Disable clipping rectangle for every individual columns (reduce draw command count, items will be able to overflow into other columns). Generally incompatible with TableSetupScrollFreeze().
	// Padding
	PadOuterX = 1 << 21,  // Default if BordersOuterV is on. Enable outermost padding. Generally desirable if you have headers.
	NoPadOuterX = 1 << 22,  // Default if BordersOuterV is off. Disable outermost padding.
	NoPadInnerX = 1 << 23,  // Disable inner padding between columns (double inner padding if BordersOuterV is on, single inner padding if BordersOuterV is off).
	// Scrolling
	ScrollX = 1 << 24,  // Enable horizontal scrolling. Require 'outer_size' parameter of BeginTable() to specify the container size. Changes default sizing policy. Because this creates a child window, ScrollY is currently generally recommended when using ScrollX.
	ScrollY = 1 << 25,  // Enable vertical scrolling. Require 'outer_size' parameter of BeginTable() to specify the container size.
	// Sorting
	SortMulti = 1 << 26,  // Hold shift when clicking headers to sort on multiple column. TableGetSortSpecs() may return specs where (SpecsCount > 1).
	SortTristate = 1 << 27,  // Allow no sorting, disable default sorting. TableGetSortSpecs() may return specs where (SpecsCount == 0).

	// [Internal] Combinations and masks
	SizingMask_ = SizingFixedFit | SizingFixedSame | SizingStretchProp | SizingStretchSame,
};

enum class PANDOR_API TableColumnFlags
{
	// Input configuration flags
	None = 0,
	Disabled = 1 << 0,   // Overriding/master disable flag: hide column, won't show in context menu (unlike calling TableSetColumnEnabled() which manipulates the user accessible state)
	DefaultHide = 1 << 1,   // Default as a hidden/disabled column.
	DefaultSort = 1 << 2,   // Default as a sorting column.
	WidthStretch = 1 << 3,   // Column will stretch. Preferable with horizontal scrolling disabled (default if table sizing policy is _SizingStretchSame or _SizingStretchProp).
	WidthFixed = 1 << 4,   // Column will not stretch. Preferable with horizontal scrolling enabled (default if table sizing policy is _SizingFixedFit and table is resizable).
	NoResize = 1 << 5,   // Disable manual resizing.
	NoReorder = 1 << 6,   // Disable manual reordering this column, this will also prevent other columns from crossing over this column.
	NoHide = 1 << 7,   // Disable ability to hide/disable this column.
	NoClip = 1 << 8,   // Disable clipping for this column (all NoClip columns will render in a same draw command).
	NoSort = 1 << 9,   // Disable ability to sort on this field (even if TableFlags_Sortable is set on the table).
	NoSortAscending = 1 << 10,  // Disable ability to sort in the ascending direction.
	NoSortDescending = 1 << 11,  // Disable ability to sort in the descending direction.
	NoHeaderLabel = 1 << 12,  // TableHeadersRow() will not submit label for this column. Convenient for some small columns. Name will still appear in context menu.
	NoHeaderWidth = 1 << 13,  // Disable header text width contribution to automatic column width.
	PreferSortAscending = 1 << 14,  // Make the initial sort direction Ascending when first sorting on this column (default).
	PreferSortDescending = 1 << 15,  // Make the initial sort direction Descending when first sorting on this column.
	IndentEnable = 1 << 16,  // Use current Indent value when entering cell (default for column 0).
	IndentDisable = 1 << 17,  // Ignore current Indent value when entering cell (default for columns > 0). Indentation changes _within_ the cell will still be honored.

	// Output status flags, read-only via TableGetColumnFlags()
	IsEnabled = 1 << 24,  // Status: is enabled == not hidden by user/api (referred to as "Hide" in _DefaultHide and _NoHide) flags.
	IsVisible = 1 << 25,  // Status: is visible == is enabled AND not clipped by scrolling.
	IsSorted = 1 << 26,  // Status: is currently part of the sort specs
	IsHovered = 1 << 27,  // Status: is hovered by mouse

	// [Internal] Combinations and masks
	WidthMask_ = WidthStretch | WidthFixed,
	IndentMask_ = IndentEnable | IndentDisable,
	StatusMask_ = IsEnabled | IsVisible | IsSorted | IsHovered,
	NoDirectResize_ = 1 << 30,  // [Internal] Disable user resizing this column directly (it may however we resized indirectly from its left edge)
};

enum class PANDOR_API TableRowFlags
{
	None = 0,
	Headers = 1 << 0,   // Identify header row (set default background color + width of its contents accounted differently for auto column width)
};

enum class PANDOR_API TableBgTarget
{
	None = 0,
	RowBg0 = 1,        // Set row background color 0 (generally used for background, automatically set when TableFlags_RowBg is used)
	RowBg1 = 2,        // Set row background color 1 (generally used for selection marking)
	CellBg = 3,        // Set cell background color (top-most color)
};

enum class PANDOR_API FocusedFlags
{
	None = 0,
	ChildWindows = 1 << 0,   // Return true if any children of the window is focused
	RootWindow = 1 << 1,   // Test from root window (top most parent of the current hierarchy)
	AnyWindow = 1 << 2,   // Return true if any window is focused. Important: If you are trying to tell how to dispatch your low-level inputs, do NOT use this. Use 'io.WantCaptureMouse' instead! Please read the FAQ!
	NoPopupHierarchy = 1 << 3,   // Do not consider popup hierarchy (do not treat popup emitter as parent of popup) (when used with _ChildWindows or _RootWindow)
	DockHierarchy = 1 << 4,   // Consider docking hierarchy (treat dockspace host as parent of docked window) (when used with _ChildWindows or _RootWindow)
	RootAndChildWindows = RootWindow | ChildWindows,
};

enum class PANDOR_API HoveredFlags
{
	None = 0,        // Return true if directly over the item/window, not obstructed by another window, not obstructed by an active popup or modal blocking inputs under them.
	ChildWindows = 1 << 0,   // IsWindowHovered() only: Return true if any children of the window is hovered
	RootWindow = 1 << 1,   // IsWindowHovered() only: Test from root window (top most parent of the current hierarchy)
	AnyWindow = 1 << 2,   // IsWindowHovered() only: Return true if any window is hovered
	NoPopupHierarchy = 1 << 3,   // IsWindowHovered() only: Do not consider popup hierarchy (do not treat popup emitter as parent of popup) (when used with _ChildWindows or _RootWindow)
	DockHierarchy = 1 << 4,   // IsWindowHovered() only: Consider docking hierarchy (treat dockspace host as parent of docked window) (when used with _ChildWindows or _RootWindow)
	AllowWhenBlockedByPopup = 1 << 5,   // Return true even if a popup window is normally blocking access to this item/window
	//AllowWhenBlockedByModal     = 1 << 6,   // Return true even if a modal popup window is normally blocking access to this item/window. FIXME-TODO: Unavailable yet.
	AllowWhenBlockedByActiveItem = 1 << 7,   // Return true even if an active item is blocking access to this item/window. Useful for Drag and Drop patterns.
	AllowWhenOverlapped = 1 << 8,   // IsItemHovered() only: Return true even if the position is obstructed or overlapped by another window
	AllowWhenDisabled = 1 << 9,   // IsItemHovered() only: Return true even if the item is disabled
	NoNavOverride = 1 << 10,  // Disable using gamepad/keyboard navigation state when active, always query mouse.
	RectOnly = AllowWhenBlockedByPopup | AllowWhenBlockedByActiveItem | AllowWhenOverlapped,
	RootAndChildWindows = RootWindow | ChildWindows,

	// Hovering delays (for tooltips)
	DelayNormal = 1 << 11,  // Return true after io.HoverDelayNormal elapsed (~0.30 sec)
	DelayShort = 1 << 12,  // Return true after io.HoverDelayShort elapsed (~0.10 sec)
	NoSharedDelay = 1 << 13,  // Disable shared delay system where moving from one item to the next keeps the previous timer for a short time (standard for tooltips with long delays)
};

enum class PANDOR_API DockNodeFlags
{
	None = 0,
	KeepAliveOnly = 1 << 0,   // Shared       // Don't display the dockspace node but keep it alive. Windows docked into this dockspace node won't be undocked.
	//NoCentralNode              = 1 << 1,   // Shared       // Disable Central Node (the node which can stay empty)
	NoDockingInCentralNode = 1 << 2,   // Shared       // Disable docking inside the Central Node, which will be always kept empty.
	PassthruCentralNode = 1 << 3,   // Shared       // Enable passthru dockspace: 1) DockSpace() will render a WindowBg background covering everything excepted the Central Node when empty. Meaning the host window should probably use SetNextWindowBgAlpha(0.0f) prior to Begin() when using this. 2) When Central Node is empty: let inputs pass-through + won't display a DockingEmptyBg background. See demo for details.
	NoSplit = 1 << 4,   // Shared/Local // Disable splitting the node into smaller nodes. Useful e.g. when embedding dockspaces into a main root one (the root one may have splitting disabled to reduce confusion). Note: when turned off, existing splits will be preserved.
	NoResize = 1 << 5,   // Shared/Local // Disable resizing node using the splitter/separators. Useful with programmatically setup dockspaces.
	AutoHideTabBar = 1 << 6,   // Shared/Local // Tab bar will automatically hide when there is a single window in the dock node.
};

enum class PANDOR_API DragDropFlags
{
	None = 0,
	// BeginDragDropSource() flags
	SourceNoPreviewTooltip = 1 << 0,   // Disable preview tooltip. By default, a successful call to BeginDragDropSource opens a tooltip so you can display a preview or description of the source contents. This flag disables this behavior.
	SourceNoDisableHover = 1 << 1,   // By default, when dragging we clear data so that IsItemHovered() will return false, to avoid subsequent user code submitting tooltips. This flag disables this behavior so you can still call IsItemHovered() on the source item.
	SourceNoHoldToOpenOthers = 1 << 2,   // Disable the behavior that allows to open tree nodes and collapsing header by holding over them while dragging a source item.
	SourceAllowNullID = 1 << 3,   // Allow items such as Text(), Image() that have no unique identifier to be used as drag source, by manufacturing a temporary identifier based on their window-relative position. This is extremely unusual within the dear imgui ecosystem and so we made it explicit.
	SourceExtern = 1 << 4,   // External source (from outside of dear imgui), won't attempt to read current item/window info. Will always return true. Only one Extern source can be active simultaneously.
	SourceAutoExpirePayload = 1 << 5,   // Automatically expire the payload if the source cease to be submitted (otherwise payloads are persisting while being dragged)
	// AcceptDragDropPayload() flags
	AcceptBeforeDelivery = 1 << 10,  // AcceptDragDropPayload() will returns true even before the mouse button is released. You can then call IsDelivery() to test if the payload needs to be delivered.
	AcceptNoDrawDefaultRect = 1 << 11,  // Do not draw the default highlight rectangle when hovering over target.
	AcceptNoPreviewTooltip = 1 << 12,  // Request hiding the BeginDragDropSource tooltip from the BeginDragDropTarget site.
	AcceptPeekOnly = AcceptBeforeDelivery | AcceptNoDrawDefaultRect, // For peeking ahead and inspecting the payload before delivery.
};

enum class PANDOR_API DataType
{
	S8,       // signed char / char (with sensible compilers)
	U8,       // unsigned char
	S16,      // short
	U16,      // unsigned short
	S32,      // int
	U32,      // unsigned int
	S64,      // long long / __int64
	U64,      // unsigned long long / unsigned __int64
	Float,    // float
	Double,   // double
	COUNT
};

enum class PANDOR_API Dir
{
	None = -1,
	Left = 0,
	Right = 1,
	Up = 2,
	Down = 3,
	COUNT
};

enum class PANDOR_API SortDirection
{
	None = 0,
	Ascending = 1,    // Ascending = 0->9, A->Z etc.
	Descending = 2     // Descending = 9->0, Z->A etc.
};

enum class PANDOR_API Key : int
{
	// Keyboard
	Key_None = 0,
	Key_Tab = 512,             // == Key_NamedKey_BEGIN
	Key_LeftArrow,
	Key_RightArrow,
	Key_UpArrow,
	Key_DownArrow,
	Key_PageUp,
	Key_PageDown,
	Key_Home,
	Key_End,
	Key_Insert,
	Key_Delete,
	Key_Backspace,
	Key_Space,
	Key_Enter,
	Key_Escape,
	Key_LeftCtrl, Key_LeftShift, Key_LeftAlt, Key_LeftSuper,
	Key_RightCtrl, Key_RightShift, Key_RightAlt, Key_RightSuper,
	Key_Menu,
	Key_0, Key_1, Key_2, Key_3, Key_4, Key_5, Key_6, Key_7, Key_8, Key_9,
	Key_A, Key_B, Key_C, Key_D, Key_E, Key_F, Key_G, Key_H, Key_I, Key_J,
	Key_K, Key_L, Key_M, Key_N, Key_O, Key_P, Key_Q, Key_R, Key_S, Key_T,
	Key_U, Key_V, Key_W, Key_X, Key_Y, Key_Z,
	Key_F1, Key_F2, Key_F3, Key_F4, Key_F5, Key_F6,
	Key_F7, Key_F8, Key_F9, Key_F10, Key_F11, Key_F12,
	Key_Apostrophe,        // '
	Key_Comma,             // ,
	Key_Minus,             // -
	Key_Period,            // .
	Key_Slash,             // /
	Key_Semicolon,         // ;
	Key_Equal,             // =
	Key_LeftBracket,       // [
	Key_Backslash,         // \ (this text inhibit multiline comment caused by backslash)
	Key_RightBracket,      // ]
	Key_GraveAccent,       // `
	Key_CapsLock,
	Key_ScrollLock,
	Key_NumLock,
	Key_PrintScreen,
	Key_Pause,
	Key_Keypad0, Key_Keypad1, Key_Keypad2, Key_Keypad3, Key_Keypad4,
	Key_Keypad5, Key_Keypad6, Key_Keypad7, Key_Keypad8, Key_Keypad9,
	Key_KeypadDecimal,
	Key_KeypadDivide,
	Key_KeypadMultiply,
	Key_KeypadSubtract,
	Key_KeypadAdd,
	Key_KeypadEnter,
	Key_KeypadEqual,

	// Gamepad (some of those are analog values, 0.0f to 1.0f)                          // NAVIGATION ACTION
	// (download controller mapping PNG/PSD at http://dearimgui.org/controls_sheets)
	Key_GamepadStart,          // Menu (Xbox)      + (Switch)   Start/Options (PS)
	Key_GamepadBack,           // View (Xbox)      - (Switch)   Share (PS)
	Key_GamepadFaceLeft,       // X (Xbox)         Y (Switch)   Square (PS)        // Tap: Toggle Menu. Hold: Windowing mode (Focus/Move/Resize windows)
	Key_GamepadFaceRight,      // B (Xbox)         A (Switch)   Circle (PS)        // Cancel / Close / Exit
	Key_GamepadFaceUp,         // Y (Xbox)         X (Switch)   Triangle (PS)      // Text Input / On-screen Keyboard
	Key_GamepadFaceDown,       // A (Xbox)         B (Switch)   Cross (PS)         // Activate / Open / Toggle / Tweak
	Key_GamepadDpadLeft,       // D-pad Left                                       // Move / Tweak / Resize Window (in Windowing mode)
	Key_GamepadDpadRight,      // D-pad Right                                      // Move / Tweak / Resize Window (in Windowing mode)
	Key_GamepadDpadUp,         // D-pad Up                                         // Move / Tweak / Resize Window (in Windowing mode)
	Key_GamepadDpadDown,       // D-pad Down                                       // Move / Tweak / Resize Window (in Windowing mode)
	Key_GamepadL1,             // L Bumper (Xbox)  L (Switch)   L1 (PS)            // Tweak Slower / Focus Previous (in Windowing mode)
	Key_GamepadR1,             // R Bumper (Xbox)  R (Switch)   R1 (PS)            // Tweak Faster / Focus Next (in Windowing mode)
	Key_GamepadL2,             // L Trig. (Xbox)   ZL (Switch)  L2 (PS) [Analog]
	Key_GamepadR2,             // R Trig. (Xbox)   ZR (Switch)  R2 (PS) [Analog]
	Key_GamepadL3,             // L Stick (Xbox)   L3 (Switch)  L3 (PS)
	Key_GamepadR3,             // R Stick (Xbox)   R3 (Switch)  R3 (PS)
	Key_GamepadLStickLeft,     // [Analog]                                         // Move Window (in Windowing mode)
	Key_GamepadLStickRight,    // [Analog]                                         // Move Window (in Windowing mode)
	Key_GamepadLStickUp,       // [Analog]                                         // Move Window (in Windowing mode)
	Key_GamepadLStickDown,     // [Analog]                                         // Move Window (in Windowing mode)
	Key_GamepadRStickLeft,     // [Analog]
	Key_GamepadRStickRight,    // [Analog]
	Key_GamepadRStickUp,       // [Analog]
	Key_GamepadRStickDown,     // [Analog]

	// Aliases: Mouse Buttons (auto-submitted from AddMouseButtonEvent() calls)
	// - This is mirroring the data also written to io.MouseDown[], io.MouseWheel, in a format allowing them to be accessed via standard key API.
	Key_MouseLeft, Key_MouseRight, Key_MouseMiddle, Key_MouseX1, Key_MouseX2, Key_MouseWheelX, Key_MouseWheelY,

	// [Internal] Reserved for mod storage
	Key_ReservedForModCtrl, Key_ReservedForModShift, Key_ReservedForModAlt, Key_ReservedForModSuper,
	Key_COUNT,

	// Keyboard Modifiers (explicitly submitted by backend via AddKeyEvent() calls)
	// - This is mirroring the data also written to io.KeyCtrl, io.KeyShift, io.KeyAlt, io.KeySuper, in a format allowing
	//   them to be accessed via standard key API, allowing calls such as IsKeyPressed(), IsKeyReleased(), querying duration etc.
	// - Code polling every key (e.g. an interface to detect a key press for input mapping) might want to ignore those
	//   and prefer using the real keys (e.g. Key_LeftCtrl, Key_RightCtrl instead of Mod_Ctrl).
	// - In theory the value of keyboard modifiers should be roughly equivalent to a logical or of the equivalent left/right keys.
	//   In practice: it's complicated; mods are often provided from different sources. Keyboard layout, IME, sticky keys and
	//   backends tend to interfere and break that equivalence. The safer decision is to relay that ambiguity down to the end-user...
	Mod_None = 0,
	Mod_Ctrl = 1 << 12, // Ctrl
	Mod_Shift = 1 << 13, // Shift
	Mod_Alt = 1 << 14, // Option/Menu
	Mod_Super = 1 << 15, // Cmd/Super/Windows
	Mod_Shortcut = 1 << 11, // Alias for Ctrl (non-macOS) _or_ Super (macOS).
	Mod_Mask_ = 0xF800,  // 5-bits

	// [Internal] Prior to 1.87 we required user to fill io.KeysDown[512] using their own native index + the io.KeyMap[] array.
	// We are ditching this method but keeping a legacy path for user code doing e.g. IsKeyPressed(MY_NATIVE_KEY_CODE)
	Key_NamedKey_BEGIN = 512,
	Key_NamedKey_END = Key_COUNT,
	Key_NamedKey_COUNT = Key_NamedKey_END - Key_NamedKey_BEGIN,
#ifdef IMGUI_DISABLE_OBSOLETE_KEYIO
	Key_KeysData_SIZE = Key_NamedKey_COUNT,          // Size of KeysData[]: only hold named keys
	Key_KeysData_OFFSET = Key_NamedKey_BEGIN,          // First key stored in io.KeysData[0]. Accesses to io.KeysData[] must use (key - Key_KeysData_OFFSET).
#else
	Key_KeysData_SIZE = Key_COUNT,                   // Size of KeysData[]: hold legacy 0..512 keycodes + named keys
	Key_KeysData_OFFSET = 0,                                // First key stored in io.KeysData[0]. Accesses to io.KeysData[] must use (key - Key_KeysData_OFFSET).
#endif

#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
	Key_ModCtrl = Mod_Ctrl, Key_ModShift = Mod_Shift, Key_ModAlt = Mod_Alt, Key_ModSuper = Mod_Super, // Renamed in 1.89
	Key_KeyPadEnter = Key_KeypadEnter,    // Renamed in 1.87
#endif
};

enum class PANDOR_API ConfigFlags
{
	None = 0,
	NavEnableKeyboard = 1 << 0,   // Master keyboard navigation enable flag.
	NavEnableGamepad = 1 << 1,   // Master gamepad navigation enable flag. Backend also needs to set BackendFlags_HasGamepad.
	NavEnableSetMousePos = 1 << 2,   // Instruct navigation to move the mouse cursor. May be useful on TV/console systems where moving a virtual mouse is awkward. Will update io.MousePos and set io.WantSetMousePos=true. If enabled you MUST honor io.WantSetMousePos requests in your backend, otherwise  will react as if the mouse is jumping around back and forth.
	NavNoCaptureKeyboard = 1 << 3,   // Instruct navigation to not set the io.WantCaptureKeyboard flag when io.NavActive is set.
	NoMouse = 1 << 4,   // Instruct imgui to clear mouse position/buttons in NewFrame(). This allows ignoring the mouse information set by the backend.
	NoMouseCursorChange = 1 << 5,   // Instruct backend to not alter mouse cursor shape and visibility. Use if the backend cursor changes are interfering with yours and you don't want to use SetMouseCursor() to change mouse cursor. You may want to honor requests from imgui by reading GetMouseCursor() yourself instead.

	// [BETA] Docking
	DockingEnable = 1 << 6,   // Docking enable flags.

	// [BETA] Viewports
	// When using viewports it is recommended that your default value for WindowBg is opaque (Alpha=1.0) so transition to a viewport won't be noticeable.
	ViewportsEnable = 1 << 10,  // Viewport enable flags (require both BackendFlags_PlatformHasViewports + BackendFlags_RendererHasViewports set by the respective backends)
	DpiEnableScaleViewports = 1 << 14,  // [BETA: Don't use] FIXME-DPI: Reposition and resize imgui windows when the DpiScale of a viewport changed (mostly useful for the main viewport hosting other window). Note that resizing the main window itself is up to your application.
	DpiEnableScaleFonts = 1 << 15,  // [BETA: Don't use] FIXME-DPI: Request bitmap-scaled fonts to match DpiScale. This is a very low-quality workaround. The correct way to handle DPI is _currently_ to replace the atlas and/or fonts in the Platform_OnChangedViewport callback, but this is all early work in progress.

	// User storage (to allow your backend/engine to communicate to code that may be shared between multiple projects. Those flags are NOT used by core Dear )
	IsSRGB = 1 << 20,  // Application is SRGB-aware.
	IsTouchScreen = 1 << 21,  // Application is using a touch screen instead of a mouse.
};

enum class PANDOR_API BackendFlags
{
	None = 0,
	HasGamepad = 1 << 0,   // Backend Platform supports gamepad and currently has one connected.
	HasMouseCursors = 1 << 1,   // Backend Platform supports honoring GetMouseCursor() value to change the OS cursor shape.
	HasSetMousePos = 1 << 2,   // Backend Platform supports io.WantSetMousePos requests to reposition the OS mouse position (only used if NavEnableSetMousePos is set).
	RendererHasVtxOffset = 1 << 3,   // Backend Renderer supports ImDrawCmd::VtxOffset. This enables output of large meshes (64K+ vertices) while still using 16-bit indices.

	// [BETA] Viewports
	PlatformHasViewports = 1 << 10,  // Backend Platform supports multiple viewports.
	HasMouseHoveredViewport = 1 << 11,  // Backend Platform supports calling io.AddMouseViewportEvent() with the viewport under the mouse. IF POSSIBLE, ignore viewports with the ViewportFlags_NoInputs flag (Win32 backend, GLFW 3.30+ backend can do this, SDL backend cannot). If this cannot be done, Dear  needs to use a flawed heuristic to find the viewport under.
	RendererHasViewports = 1 << 12,  // Backend Renderer supports multiple viewports.
};

enum class PANDOR_API Col
{
	Text,
	TextDisabled,
	WindowBg,              // Background of normal windows
	ChildBg,               // Background of child windows
	PopupBg,               // Background of popups, menus, tooltips windows
	Border,
	BorderShadow,
	FrameBg,               // Background of checkbox, radio button, plot, slider, text input
	FrameBgHovered,
	FrameBgActive,
	TitleBg,
	TitleBgActive,
	TitleBgCollapsed,
	MenuBarBg,
	ScrollbarBg,
	ScrollbarGrab,
	ScrollbarGrabHovered,
	ScrollbarGrabActive,
	CheckMark,
	SliderGrab,
	SliderGrabActive,
	Button,
	ButtonHovered,
	ButtonActive,
	Header,                // Header* colors are used for CollapsingHeader, TreeNode, Selectable, MenuItem
	HeaderHovered,
	HeaderActive,
	Separator,
	SeparatorHovered,
	SeparatorActive,
	ResizeGrip,            // Resize grip in lower-right and lower-left corners of windows.
	ResizeGripHovered,
	ResizeGripActive,
	Tab,                   // TabItem in a TabBar
	TabHovered,
	TabActive,
	TabUnfocused,
	TabUnfocusedActive,
	DockingPreview,        // Preview overlay color when about to docking something
	DockingEmptyBg,        // Background color for empty node (e.g. CentralNode with no window docked into it)
	PlotLines,
	PlotLinesHovered,
	PlotHistogram,
	PlotHistogramHovered,
	TableHeaderBg,         // Table header background
	TableBorderStrong,     // Table outer and header borders (prefer using Alpha=1.0 here)
	TableBorderLight,      // Table inner borders (prefer using Alpha=1.0 here)
	TableRowBg,            // Table row background (even rows)
	TableRowBgAlt,         // Table row background (odd rows)
	TextSelectedBg,
	DragDropTarget,        // Rectangle highlighting a drop target
	NavHighlight,          // Gamepad/keyboard: current highlighted item
	NavWindowingHighlight, // Highlight window when using CTRL+TAB
	NavWindowingDimBg,     // Darken/colorize entire screen behind the CTRL+TAB window list, when active
	ModalWindowDimBg,      // Darken/colorize entire screen behind a modal window, when one is active
	COUNT
};

enum class PANDOR_API StyleVar
{
	// Enum name --------------------- // Member in Style structure (see Style for descriptions)
	Alpha,               // float     Alpha
	DisabledAlpha,       // float     DisabledAlpha
	WindowPadding,       // Math::Vector2    WindowPadding
	WindowRounding,      // float     WindowRounding
	WindowBorderSize,    // float     WindowBorderSize
	WindowMinSize,       // Math::Vector2    WindowMinSize
	WindowTitleAlign,    // Math::Vector2    WindowTitleAlign
	ChildRounding,       // float     ChildRounding
	ChildBorderSize,     // float     ChildBorderSize
	PopupRounding,       // float     PopupRounding
	PopupBorderSize,     // float     PopupBorderSize
	FramePadding,        // Math::Vector2    FramePadding
	FrameRounding,       // float     FrameRounding
	FrameBorderSize,     // float     FrameBorderSize
	ItemSpacing,         // Math::Vector2    ItemSpacing
	ItemInnerSpacing,    // Math::Vector2    ItemInnerSpacing
	IndentSpacing,       // float     IndentSpacing
	CellPadding,         // Math::Vector2    CellPadding
	ScrollbarSize,       // float     ScrollbarSize
	ScrollbarRounding,   // float     ScrollbarRounding
	GrabMinSize,         // float     GrabMinSize
	GrabRounding,        // float     GrabRounding
	TabRounding,         // float     TabRounding
	ButtonTextAlign,     // Math::Vector2    ButtonTextAlign
	SelectableTextAlign, // Math::Vector2    SelectableTextAlign
	SeparatorTextBorderSize,// float  SeparatorTextBorderSize
	SeparatorTextAlign,  // Math::Vector2    SeparatorTextAlign
	SeparatorTextPadding,// Math::Vector2    SeparatorTextPadding
	COUNT
};

enum class PANDOR_API ButtonFlags
{
	None = 0,
	MouseButtonLeft = 1 << 0,   // React on left mouse button (default)
	MouseButtonRight = 1 << 1,   // React on right mouse button
	MouseButtonMiddle = 1 << 2,   // React on center mouse button

	// [Internal]
	MouseButtonMask_ = MouseButtonLeft | MouseButtonRight | MouseButtonMiddle,
	MouseButtonDefault_ = MouseButtonLeft,
};

enum class PANDOR_API ColorEditFlags
{
	None = 0,
	NoAlpha = 1 << 1,   //              // ColorEdit, ColorPicker, ColorButton: ignore Alpha component (will only read 3 components from the input pointer).
	NoPicker = 1 << 2,   //              // ColorEdit: disable picker when clicking on color square.
	NoOptions = 1 << 3,   //              // ColorEdit: disable toggling options menu when right-clicking on inputs/small preview.
	NoSmallPreview = 1 << 4,   //              // ColorEdit, ColorPicker: disable color square preview next to the inputs. (e.g. to show only the inputs)
	NoInputs = 1 << 5,   //              // ColorEdit, ColorPicker: disable inputs sliders/text widgets (e.g. to show only the small preview color square).
	NoTooltip = 1 << 6,   //              // ColorEdit, ColorPicker, ColorButton: disable tooltip when hovering the preview.
	NoLabel = 1 << 7,   //              // ColorEdit, ColorPicker: disable display of inline text label (the label is still forwarded to the tooltip and picker).
	NoSidePreview = 1 << 8,   //              // ColorPicker: disable bigger color preview on right side of the picker, use small color square preview instead.
	NoDragDrop = 1 << 9,   //              // ColorEdit: disable drag and drop target. ColorButton: disable drag and drop source.
	NoBorder = 1 << 10,  //              // ColorButton: disable border (which is enforced by default)

	// User Options (right-click on widget to change some of them).
	AlphaBar = 1 << 16,  //              // ColorEdit, ColorPicker: show vertical alpha bar/gradient in picker.
	AlphaPreview = 1 << 17,  //              // ColorEdit, ColorPicker, ColorButton: display preview as a transparent color over a checkerboard, instead of opaque.
	AlphaPreviewHalf = 1 << 18,  //              // ColorEdit, ColorPicker, ColorButton: display half opaque / half checkerboard, instead of opaque.
	HDR = 1 << 19,  //              // (WIP) ColorEdit: Currently only disable 0.0f..1.0f limits in RGBA edition (note: you probably want to use Float flag as well).
	DisplayRGB = 1 << 20,  // [Display]    // ColorEdit: override _display_ type among RGB/HSV/Hex. ColorPicker: select any combination using one or more of RGB/HSV/Hex.
	DisplayHSV = 1 << 21,  // [Display]    // "
	DisplayHex = 1 << 22,  // [Display]    // "
	Uint8 = 1 << 23,  // [DataType]   // ColorEdit, ColorPicker, ColorButton: _display_ values formatted as 0..255.
	Float = 1 << 24,  // [DataType]   // ColorEdit, ColorPicker, ColorButton: _display_ values formatted as 0.0f..1.0f floats instead of 0..255 integers. No round-trip of value via integers.
	PickerHueBar = 1 << 25,  // [Picker]     // ColorPicker: bar for Hue, rectangle for Sat/Value.
	PickerHueWheel = 1 << 26,  // [Picker]     // ColorPicker: wheel for Hue, triangle for Sat/Value.
	InputRGB = 1 << 27,  // [Input]      // ColorEdit, ColorPicker: input and output data in RGB format.
	InputHSV = 1 << 28,  // [Input]      // ColorEdit, ColorPicker: input and output data in HSV format.

	// Defaults Options. You can set application defaults using SetColorEditOptions(). The intent is that you probably don't want to
	// override them in most of your calls. Let the user choose via the option menu and/or call SetColorEditOptions() once during startup.
	DefaultOptions_ = Uint8 | DisplayRGB | InputRGB | PickerHueBar,

	// [Internal] Masks
	DisplayMask_ = DisplayRGB | DisplayHSV | DisplayHex,
	DataTypeMask_ = Uint8 | Float,
	PickerMask_ = PickerHueWheel | PickerHueBar,
	InputMask_ = InputRGB | InputHSV,

	// Obsolete names (will be removed)
	// RGB = DisplayRGB, HSV = DisplayHSV, HEX = DisplayHex  // [renamed in 1.69]
};

enum class PANDOR_API SliderFlags
{
	None = 0,
	AlwaysClamp = 1 << 4,       // Clamp value to min/max bounds when input manually with CTRL+Click. By default CTRL+Click allows going out of bounds.
	Logarithmic = 1 << 5,       // Make the widget logarithmic (linear otherwise). Consider using NoRoundToFormat with this if using a format-string with small amount of digits.
	NoRoundToFormat = 1 << 6,       // Disable rounding underlying value to match precision of the display format string (e.g. %.3f values are rounded to those 3 digits)
	NoInput = 1 << 7,       // Disable CTRL+Click or Enter key allowing to input text directly into the widget
	InvalidMask_ = 0x7000000F,   // [Internal] We treat using those bits as being potentially a 'float power' argument from the previous API that has got miscast to this enum class, and will trigger an assert if needed.

	// Obsolete names (will be removed)
#ifndef IMGUI_DISABLE_OBSOLETE_FUNCTIONS
	ClampOnInput = AlwaysClamp, // [renamed in 1.79]
#endif
};

enum class PANDOR_API MouseButton
{
	Left = 0,
	Right = 1,
	Middle = 2,
	COUNT = 5
};

enum class PANDOR_API MouseCursor
{
	None = -1,
	Arrow = 0,
	TextInput,         // When hovering over InputText, etc.
	ResizeAll,         // (Unused by Dear  functions)
	ResizeNS,          // When hovering over a horizontal border
	ResizeEW,          // When hovering over a vertical border or a column
	ResizeNESW,        // When hovering over the bottom-left corner of a window
	ResizeNWSE,        // When hovering over the bottom-right corner of a window
	Hand,              // (Unused by Dear  functions. Use for e.g. hyperlinks)
	NotAllowed,        // When hovering something with disallowed interaction. Usually a crossed circle.
	COUNT
};

enum class PANDOR_API Cond
{
	None = 0,        // No condition (always set the variable), same as _Always
	Always = 1 << 0,   // No condition (always set the variable), same as _None
	Once = 1 << 1,   // Set the variable once per runtime session (only the first call will succeed)
	FirstUseEver = 1 << 2,   // Set the variable if the object/window has no persistently saved data (no entry in .ini file)
	Appearing = 1 << 3,   // Set the variable if the object/window is appearing after being hidden/inactive (or the first time)
};

enum ViewportFlags
{
	None = 0,
	IsPlatformWindow = 1 << 0,   // Represent a Platform Window
	IsPlatformMonitor = 1 << 1,   // Represent a Platform Monitor (unused yet)
	OwnedByApp = 1 << 2,   // Platform Window: is created/managed by the application (rather than a dear imgui backend)
	NoDecoration = 1 << 3,   // Platform Window: Disable platform decorations: title bar, borders, etc. (generally set all windows, but if ConfigFlags_ViewportsDecoration is set we only set this on popups/tooltips)
	NoTaskBarIcon = 1 << 4,   // Platform Window: Disable platform task bar icon (generally set on popups/tooltips, or all windows if ConfigFlags_ViewportsNoTaskBarIcon is set)
	NoFocusOnAppearing = 1 << 5,   // Platform Window: Don't take focus when created.
	NoFocusOnClick = 1 << 6,   // Platform Window: Don't take focus when clicked on.
	NoInputs = 1 << 7,   // Platform Window: Make mouse pass through so we can drag this window while peaking behind it.
	NoRendererClear = 1 << 8,   // Platform Window: Renderer doesn't need to clear the framebuffer ahead (because we will fill it entirely).
	TopMost = 1 << 9,   // Platform Window: Display on top (for tooltips only).
	Minimized = 1 << 10,  // Platform Window: Window is minimized, can skip render. When minimized we tend to avoid using the viewport pos/size for clipping window or testing if they are contained in the viewport.
	NoAutoMerge = 1 << 11,  // Platform Window: Avoid merging this window into another host window. This can only be set via WindowClass viewport flags override (because we need to now ahead if we are going to create a viewport in the first place!).
	CanHostOtherWindows = 1 << 12,  // Main viewport: can host multiple imgui windows (secondary viewports are associated to a single window).
};

enum class PANDOR_API SeparatorFlags
{
	None = 0,
	Horizontal = 1 << 0,   // Axis default to current layout type, so generally Horizontal unless e.g. in a menu bar
	Vertical = 1 << 1,
	SpanAllColumns = 1 << 2,
};
#pragma endregion

#pragma region Structs
struct PANDOR_API SizeCallbackData
{
	void* UserData;       // Read-only.   What user passed to SetNextWindowSizeConstraints(). Generally store an integer or float in here (need reinterpret_cast<>).
	Math::Vector2  Pos;            // Read-only.   Window position, for reference.
	Math::Vector2  CurrentSize;    // Read-only.   Current window size.
	Math::Vector2  DesiredSize;    // Read-write.  Desired size, based on user's mouse position. Write to this field to restrain resizing.
};

struct PANDOR_API InputTextCallbackData
{
	InputTextFlags EventFlag;      // One InputTextFlags_Callback*    // Read-only
	InputTextFlags Flags;          // What user passed to InputText()      // Read-only
	void* UserData;       // What user passed to InputText()      // Read-only

	// Arguments for the different callback events
	// - To modify the text buffer in a callback, prefer using the InsertChars() / DeleteChars() function. InsertChars() will take care of calling the resize callback if necessary.
	// - If you know your edits are not going to resize the underlying buffer allocation, you may modify the contents of 'Buf[]' directly. You need to update 'BufTextLen' accordingly (0 <= BufTextLen < BufSize) and set 'BufDirty'' to true so InputText can update its internal state.
	wchar_t             EventChar;      // Character input                      // Read-write   // [CharFilter] Replace character with another one, or set to zero to drop. return 1 is equivalent to setting EventChar=0;
	Key            EventKey;       // Key pressed (Up/Down/TAB)            // Read-only    // [Completion,History]
	char* Buf;            // Text buffer                          // Read-write   // [Resize] Can replace pointer / [Completion,History,Always] Only write to pointed data, don't replace the actual pointer!
	int                 BufTextLen;     // Text length (in bytes)               // Read-write   // [Resize,Completion,History,Always] Exclude zero-terminator storage. In C land: == strlen(some_text), in C++ land: string.length()
	int                 BufSize;        // Buffer size (in bytes) = capacity+1  // Read-only    // [Resize,Completion,History,Always] Include zero-terminator storage. In C land == ARRAYSIZE(my_char_array), in C++ land: string.capacity()+1
	bool                BufDirty;       // Set if you modify Buf/BufTextLen!    // Write        // [Completion,History,Always]
	int                 CursorPos;      //                                      // Read-write   // [Completion,History,Always]
	int                 SelectionStart; //                                      // Read-write   // [Completion,History,Always] == to SelectionEnd when no selection)
	int                 SelectionEnd;   //                                      // Read-write   // [Completion,History,Always]

	// Helper functions for text manipulation.
	// Use those function to benefit from the CallbackResize behaviors. Calling those function reset the selection.
	InputTextCallbackData();
	void      DeleteChars(int pos, int bytes_count);
	void      InsertChars(int pos, const char* text, const char* text_end = NULL);
	void                SelectAll() { SelectionStart = 0; SelectionEnd = BufTextLen; }
	void                ClearSelection() { SelectionStart = SelectionEnd = BufTextLen; }
	bool                HasSelection() const { return SelectionStart != SelectionEnd; }
};

struct PANDOR_API TableColumnSortSpecs
{
	uint32_t                     ColumnUserID;       // User id of the column (if specified by a TableSetupColumn() call)
	signed short                       ColumnIndex;        // Index of the column
	signed short                       SortOrder;          // Index within parent ImGuiTableSortSpecs (always stored in order starting from 0, tables sorted on a single criteria will always have a 0 here)
	int          SortDirection : 8;  // ImGuiSortDirection_Ascending or ImGuiSortDirection_Descending (you can use this or SortSign, whichever is more convenient for your sort function)

	TableColumnSortSpecs() { memset(this, 0, sizeof(*this)); }
};

struct PANDOR_API TableSortSpecs
{
	const TableColumnSortSpecs* Specs;     // Pointer to sort spec array.
	int                         SpecsCount;     // Sort spec count. Most often 1. May be > 1 when TableFlags_SortMulti is enabled. May be == 0 when TableFlags_SortTristate is enabled.
	bool                        SpecsDirty;     // Set to true when specs have changed since last time! Use this to sort again, then clear the flag.

	TableSortSpecs() { memset(this, 0, sizeof(*this)); }
};

struct PANDOR_API WindowClass
{
	uint32_t             ClassId;                    // User data. 0 = Default class (unclassed). Windows of different classes cannot be docked with each others.
	uint32_t             ParentViewportId;           // Hint for the platform backend. -1: use default. 0: request platform backend to not parent the platform. != 0: request platform backend to create a parent<>child relationship between the platform windows. Not conforming backends are free to e.g. parent every viewport to the main viewport or not.
	ViewportFlags  ViewportFlagsOverrideSet;   // Viewport flags to set when a window of this class owns a viewport. This allows you to enforce OS decoration or task bar icon, override the defaults on a per-window basis.
	ViewportFlags  ViewportFlagsOverrideClear; // Viewport flags to clear when a window of this class owns a viewport. This allows you to enforce OS decoration or task bar icon, override the defaults on a per-window basis.
	TabItemFlags   TabItemFlagsOverrideSet;    // [EXPERIMENTAL] TabItem flags to set when a window of this class gets submitted into a dock node tab bar. May use with ImGuiTabItemFlags_Leading or ImGuiTabItemFlags_Trailing.
	DockNodeFlags  DockNodeFlagsOverrideSet;   // [EXPERIMENTAL] Dock node flags to set when a window of this class is hosted by a dock node (it doesn't have to be selected!)
	bool               DockingAlwaysTabBar;        // Set to true to enforce single floating windows of this class always having their own docking node (equivalent of setting the global io.ConfigDockingAlwaysTabBar)
	bool                DockingAllowUnclassed;      // Set to true to allow windows of this class to be docked/merged with an unclassed window. // FIXME-DOCK: Move to DockNodeFlags override?

	WindowClass() { memset(this, 0, sizeof(*this)); ParentViewportId = (uint32_t)-1; DockingAllowUnclassed = true; }
};

struct PANDOR_API Payload
{
	// Members
	void* Data;               // Data (copied and owned by dear imgui)
	int             DataSize;           // Data size

	// [Internal]
	uint32_t         SourceId;           // Source item id
	uint32_t         SourceParentId;     // Source parent id (if available)
	int             DataFrameCount;     // Data timestamp
	char            DataType[32 + 1];   // Data type tag (short user-supplied string, 32 characters max)
	bool            Preview;            // Set when AcceptDragDropPayload() was called and mouse has been hovering the target item (nb: handle overlapping drag targets)
	bool            Delivery;           // Set when AcceptDragDropPayload() was called and mouse button is released over the target item.

	Payload() { Clear(); }
	void Clear() { SourceId = SourceParentId = 0; Data = NULL; DataSize = 0; memset(DataType, 0, sizeof(DataType)); DataFrameCount = -1; Preview = Delivery = false; }
	bool IsDataType(const char* type) const { return DataFrameCount != -1 && strcmp(type, DataType) == 0; }
	bool IsPreview() const { return Preview; }
	bool IsDelivery() const { return Delivery; }
};
#include <vector>
struct PANDOR_API TextFilter
{
	          TextFilter(const char* default_filter = "");
	bool      Draw(const char* label = "Filter (inc,-exc)", float width = 0.0f);  // Helper calling InputText+Build
	bool      PassFilter(const char* text, const char* text_end = NULL) const;
	void      Build();
	void                Clear() { InputBuf[0] = 0; Build(); }
	bool                IsActive() const { return !Filters.empty(); }

	// [Internal]
	struct TextRange
	{
		const char* b;
		const char* e;

		TextRange() { b = e = NULL; }
		TextRange(const char* _b, const char* _e) { b = _b; e = _e; }
		bool            empty() const { return b == e; }
		void  split(char separator, std::vector<TextRange>* out) const;
	};
	char                    InputBuf[256];
	std::vector<TextRange> Filters;
	int                     CountGrep;
};


#pragma endregion

#pragma region Typedefs
typedef void    (*SizeCallback)(SizeCallbackData* data);
typedef int     (*InputTextCallback)(InputTextCallbackData* data);    // Callback function for ::InputText()
#pragma endregion

namespace Resources
{
	class IResources;
	class Texture;
	class AnimationController;
	struct StateRect;
	struct Link;
}
namespace Core
{
	class GameObject;
}

namespace Component::UI
{
	class UIImage;
	class RectTransform;
	class Button;
	class Text;
}

struct ImFont;

namespace Core::Wrapper
{
	namespace WrapperUI {
		static ImFont* defaultFont = nullptr;
		static ImFont* arialFont = nullptr;
		static std::unordered_map<std::string, ImFont*> fonts;
		static std::vector<std::pair<std::string, int>> waitingFonts;
		// Transform Draw
		PANDOR_API bool DrawVec3Control(const std::string& label, float* values, float resetValue = 0.0f, bool lockButton = false, float columnWidth = 100.0f);
		PANDOR_API void Initialize(GLFWwindow* GlfwWindow);
		PANDOR_API void NewFrame();
		PANDOR_API void EndFrame();
		PANDOR_API void Destroy();
		PANDOR_API void ShowDemoWindow();

		PANDOR_API void ShowPerformanceWindow(const char* name, bool* open);

		PANDOR_API void SetupTheme(int themeID);

		PANDOR_API bool ChangeTextureButton(Resources::Texture*& texture, const std::string& buttonName = "Texture");

		PANDOR_API void ShowMainDocking();

		PANDOR_API float GetDeltaTime();

		PANDOR_API ImFont* GetOrAddFont(std::string path, int size);
		PANDOR_API void PopulateFonts();

		PANDOR_API void DrawCanvas(const Math::Vector2 CanvasSize, Core::GameObject* Canvas);
		PANDOR_API void DrawGameObjectOnCanvas(GameObject* gameObject, const Math::Vector2& origin, float zoom);
		PANDOR_API void DrawImageOnCanvas(Component::UI::UIImage* image, const Math::Vector2& origin, float zoom);
		PANDOR_API void DrawButtonOnCanvas(Component::UI::Button* button, const Math::Vector2& origin, float zoom);
		PANDOR_API void DrawTextOnCanvas(Component::UI::Text* text, const Math::Vector2& origin, float zoom);
		PANDOR_API void DrawAnchor(Component::UI::RectTransform* transform, const Math::Vector2& origin, float zoom);

		PANDOR_API void DrawAnimatorBackGround(Resources::AnimationController* controller, Resources::StateRect*& stateSelected, Resources::Link*& linkSelected);
		PANDOR_API void DrawAnimatorList(Resources::AnimationController* controller);
		PANDOR_API bool DrawStateRect(Resources::StateRect* state, float zoom, Math::Vector2 origin, Math::Vector2 mousePos, Resources::StateRect*& selected);
		PANDOR_API void DrawLink(Resources::Link* link, const Math::Vector2& origin, float zoom, const Math::Vector2& mousePos, Resources::Link*& selected);

		PANDOR_API bool IsWindowVisible();
		PANDOR_API bool IsWindowAppearing();
		PANDOR_API bool IsWindowCollapsed();
		PANDOR_API bool IsWindowFocused(FocusedFlags flags = FocusedFlags::None); // is current window focused? or its root/child, depending on flags. see flags for options.
		PANDOR_API bool IsWindowHovered(HoveredFlags flags = HoveredFlags::None); // is current window hovered (and typically: not blocked by a popup/modal)? see flags for options. NB: If you are trying to check whether your mouse should be dispatched to imgui or to your app, you should use the 'io.WantCaptureMouse' boolean for that! Please read the FAQ!
		PANDOR_API float GetWindowDpiScale();                        // get DPI scale currently associated to the current window's viewport.
		PANDOR_API Math::Vector2 GetWindowPos();                             // get current window position in screen space (useful if you want to do your own drawing via the DrawList API)
		PANDOR_API Math::Vector2 GetWindowSize();                            // get current window size
		PANDOR_API float GetWindowWidth();                           // get current window width (shortcut for GetWindowSize().x)
		PANDOR_API float GetWindowHeight();                          // get current window height (shortcut for GetWindowSize().y)

		PANDOR_API bool Begin(const char* name, bool* p_open = NULL, WindowFlags flags = WindowFlags::None);
		PANDOR_API void End();

		PANDOR_API bool BeginChild(const char* str_id, const Math::Vector2& size = Math::Vector2(0, 0), bool border = false, WindowFlags flags = WindowFlags::None);
		PANDOR_API bool BeginChild(uint32_t id, const Math::Vector2& size = Math::Vector2(0, 0), bool border = false, WindowFlags flags = WindowFlags::None);
		PANDOR_API void EndChild();

		PANDOR_API void SetNextWindowPos(const Math::Vector2& pos, Cond cond = Cond::None, const Math::Vector2& pivot = Math::Vector2(0, 0)); // set next window position. call before Begin(). use pivot=(0.5f,0.5f) to center on given point, etc.
		PANDOR_API void SetNextWindowSize(const Math::Vector2& size, Cond cond = Cond::None);                  // set next window size. set axis to 0.0f to force an auto-fit on this axis. call before Begin()
		PANDOR_API void SetNextWindowSizeConstraints(const Math::Vector2& size_min, const Math::Vector2& size_max, SizeCallback custom_callback = NULL, void* custom_callback_data = NULL); // set next window size limits. use -1,-1 on either X/Y axis to preserve the current size. Sizes will be rounded down. Use callback to apply non-trivial programmatic constraints.
		PANDOR_API void SetNextWindowContentSize(const Math::Vector2& size);                               // set next window content size (~ scrollable client area, which enforce the range of scrollbars). Not including window decorations (title bar, menu bar, etc.) nor WindowPadding. set an axis to 0.0f to leave it automatic. call before Begin()
		PANDOR_API void SetNextWindowCollapsed(bool collapsed, Cond cond = Cond::None);                 // set next window collapsed state. call before Begin()
		PANDOR_API void SetNextWindowFocus();                                                       // set next window to be focused / top-most. call before Begin()
		PANDOR_API void SetNextWindowScroll(const Math::Vector2& scroll);                                  // set next window scrolling value (use < 0.0f to not affect a given axis).
		PANDOR_API void SetNextWindowBgAlpha(float alpha);                                          // set next window background color alpha. helper to easily override the Alpha component of ImGuiCol_WindowBg/ChildBg/PopupBg. you may also use ImGuiWindowFlags_NoBackground.
		PANDOR_API void SetNextWindowViewport(uint32_t viewport_id);                                 // set next window viewport
		PANDOR_API void SetWindowPos(const Math::Vector2& pos, Cond cond = Cond::None);                        // (not recommended) set current window position - call within Begin()/End(). prefer using SetNextWindowPos(), as this may incur tearing and side-effects.
		PANDOR_API void SetWindowSize(const Math::Vector2& size, Cond cond = Cond::None);                      // (not recommended) set current window size - call within Begin()/End(). set to Math::Vector2(0, 0) to force an auto-fit. prefer using SetNextWindowSize(), as this may incur tearing and minor side-effects.
		PANDOR_API void SetWindowCollapsed(bool collapsed, Cond cond = Cond::None);                     // (not recommended) set current window collapsed state. prefer using SetNextWindowCollapsed().
		PANDOR_API void SetWindowFocus();                                                           // (not recommended) set current window to be focused / top-most. prefer using SetNextWindowFocus().
		PANDOR_API void SetWindowFontScale(float scale);                                            // [OBSOLETE] set font scale. Adjust IO.FontGlobalScale if you want to scale all windows. This is an old API! For correct scaling, prefer to reload font + rebuild ImFontAtlas + call style.ScaleAllSizes().
		PANDOR_API void SetWindowPos(const char* name, const Math::Vector2& pos, Cond cond = Cond::None);      // set named window position.
		PANDOR_API void SetWindowSize(const char* name, const Math::Vector2& size, Cond cond = Cond::None);    // set named window size. set axis to 0.0f to force an auto-fit on this axis.
		PANDOR_API void SetWindowCollapsed(const char* name, bool collapsed, Cond cond = Cond::None);   // set named window collapsed state
		PANDOR_API void SetWindowFocus(const char* name);                                           // set named window to be focused / top-most. use NULL to remove focus.

		PANDOR_API Math::Vector2 GetContentRegionAvail();                                        // == GetContentRegionMax() - GetCursorPos()
		PANDOR_API Math::Vector2 GetContentRegionMax();                                          // current content boundaries (typically window boundaries including scrolling, or current column boundaries), in windows coordinates
		PANDOR_API Math::Vector2 GetWindowContentRegionMin();                                    // content boundaries min for the full window (roughly (0,0)-Scroll), in window coordinates
		PANDOR_API Math::Vector2 GetWindowContentRegionMax();                                    // content boundaries max for the full window (roughly (0,0)+Size-Scroll) where Size can be overridden with SetNextWindowContentSize(), in window coordinates

		PANDOR_API float GetScrollX();                                                   // get scrolling amount [0 .. GetScrollMaxX()]
		PANDOR_API float GetScrollY();                                                   // get scrolling amount [0 .. GetScrollMaxY()]
		PANDOR_API void SetScrollX(float scroll_x);                                     // set scrolling amount [0 .. GetScrollMaxX()]
		PANDOR_API void SetScrollY(float scroll_y);                                     // set scrolling amount [0 .. GetScrollMaxY()]
		PANDOR_API float GetScrollMaxX();                                                // get maximum scrolling amount ~~ ContentSize.x - WindowSize.x - DecorationsSize.x
		PANDOR_API float GetScrollMaxY();                                                // get maximum scrolling amount ~~ ContentSize.y - WindowSize.y - DecorationsSize.y
		PANDOR_API void SetScrollHereX(float center_x_ratio = 0.5f);                    // adjust scrolling amount to make current cursor position visible. center_x_ratio=0.0: left, 0.5: center, 1.0: right. When using to make a "default/current item" visible, consider using SetItemDefaultFocus() instead.
		PANDOR_API void SetScrollHereY(float center_y_ratio = 0.5f);                    // adjust scrolling amount to make current cursor position visible. center_y_ratio=0.0: top, 0.5: center, 1.0: bottom. When using to make a "default/current item" visible, consider using SetItemDefaultFocus() instead.
		PANDOR_API void SetScrollFromPosX(float local_x, float center_x_ratio = 0.5f);  // adjust scrolling amount to make given position visible. Generally GetCursorStartPos() + offset to compute a valid position.
		PANDOR_API void SetScrollFromPosY(float local_y, float center_y_ratio = 0.5f);  // adjust scrolling amount to make given position visible. Generally GetCursorStartPos() + offset to compute a valid position.

		PANDOR_API void PushStyleColor(Col idx, uint32_t col);                        // modify a style color. always use this if you modify the style after NewFrame().
		PANDOR_API void PushStyleColor(Col idx, const Math::Vector4& col);
		PANDOR_API void PopStyleColor(int count = 1);
		PANDOR_API void PushStyleVar(StyleVar idx, float val);                     // modify a style float variable. always use this if you modify the style after NewFrame().
		PANDOR_API void PushStyleVar(StyleVar idx, const Math::Vector2& val);             // modify a style Math::Vector2 variable. always use this if you modify the style after NewFrame().
		PANDOR_API void PopStyleVar(int count = 1);
		PANDOR_API void PushAllowKeyboardFocus(bool allow_keyboard_focus);              // == tab stop enable. Allow focusing using TAB/Shift-TAB, enabled by default but you can disable it for certain widgets
		PANDOR_API void PopAllowKeyboardFocus();
		PANDOR_API void PushButtonRepeat(bool repeat);                                  // in 'repeat' mode, Button*() functions return repeated true in a typematic manner (using io.KeyRepeatDelay/io.KeyRepeatRate setting). Note that you can call IsItemActive() after any Button() to tell if the button is held in the current frame.
		PANDOR_API void PopButtonRepeat();

		PANDOR_API void PushItemWidth(float item_width);                                // push width of items for common large "item+label" widgets. >0.0f: width in pixels, <0.0f align xx pixels to the right of window (so -FLT_MIN always align width to the right side).
		PANDOR_API void PopItemWidth();
		PANDOR_API void SetNextItemWidth(float item_width);                             // set width of the _next_ common large "item+label" widget. >0.0f: width in pixels, <0.0f align xx pixels to the right of window (so -FLT_MIN always align width to the right side)
		PANDOR_API float CalcItemWidth();                                                // width of item given pushed settings and current cursor position. NOT necessarily the width of last item unlike most 'Item' functions.
		PANDOR_API void PushTextWrapPos(float wrap_local_pos_x = 0.0f);                 // push word-wrapping position for Text*() commands. < 0.0f: no wrapping; 0.0f: wrap to end of window (or column); > 0.0f: wrap at 'wrap_pos_x' position in window local space
		PANDOR_API void PopTextWrapPos();

		PANDOR_API float GetFontSize();                                                  // get current font size (= height in pixels) of current font with current scale applied
		PANDOR_API Math::Vector2 GetFontTexUvWhitePixel();                                       // get UV coordinate for a while pixel, useful to draw custom shapes via the ImDrawList API
		PANDOR_API uint32_t GetColorU32(Col idx, float alpha_mul = 1.0f);              // retrieve given style color with style alpha applied and optional extra alpha multiplier, packed as a 32-bit value suitable for ImDrawList
		PANDOR_API uint32_t GetColorU32(const Math::Vector4& col);                                 // retrieve given color with style alpha applied, packed as a 32-bit value suitable for ImDrawList
		PANDOR_API uint32_t GetColorU32(uint32_t col);                                         // retrieve given color with style alpha applied, packed as a 32-bit value suitable for ImDrawList
		PANDOR_API const Math::Vector4 GetStyleColorVec4(Col idx);                                // retrieve style color as stored in ImGuiStyle structure. use to feed back into PushStyleColor(), otherwise use GetColorU32() to get style color with style alpha baked in.

		//Widgets
		PANDOR_API void Separator();                                                    // separator, generally horizontal. inside a menu bar or in horizontal layout mode, this becomes a vertical separator.
		PANDOR_API void SeparatorEx(SeparatorFlags flags);
		PANDOR_API void SameLine(float offset_from_start_x = 0.0f, float spacing = -1.0f);  // call between widgets or groups to layout them horizontally. X position given in window coordinates.
		PANDOR_API void NewLine();                                                      // undo a SameLine() or force a new line when in a horizontal-layout context.
		PANDOR_API void Spacing();                                                      // add vertical spacing.
		PANDOR_API void Dummy(const Math::Vector2& size);                                      // add a dummy item of given size. unlike InvisibleButton(), Dummy() won't take the mouse click or be navigable into.
		PANDOR_API void Indent(float indent_w = 0.0f);                                  // move content position toward the right, by indent_w, or style.IndentSpacing if indent_w <= 0
		PANDOR_API void Unindent(float indent_w = 0.0f);                                // move content position back to the left, by indent_w, or style.IndentSpacing if indent_w <= 0
		PANDOR_API void BeginGroup();                                                   // lock horizontal starting position
		PANDOR_API void EndGroup();                                                     // unlock horizontal starting position + capture the whole group bounding box into one "item" (so you can use IsItemHovered() or layout primitives such as SameLine() on whole group, etc.)
		PANDOR_API Math::Vector2 GetCursorPos();                                                 // cursor position in window coordinates (relative to window position)
		PANDOR_API float GetCursorPosX();                                                //   (some functions are using window-relative coordinates, such as: GetCursorPos, GetCursorStartPos, GetContentRegionMax, GetWindowContentRegion* etc.
		PANDOR_API float GetCursorPosY();                                                //    other functions such as GetCursorScreenPos or everything in ImDrawList::
		PANDOR_API void SetCursorPos(const Math::Vector2& local_pos);                          //    are using the main, absolute coordinate system.
		PANDOR_API void SetCursorPosX(float local_x);                                   //    GetWindowPos() + GetCursorPos() == GetCursorScreenPos() etc.)
		PANDOR_API void SetCursorPosY(float local_y);                                   //
		PANDOR_API Math::Vector2 GetCursorStartPos();                                            // initial cursor position in window coordinates
		PANDOR_API Math::Vector2 GetCursorScreenPos();                                           // cursor position in absolute coordinates (useful to work with ImDrawList API). generally top-left == GetMainViewport()->Pos == (0,0) in single viewport mode, and bottom-right == GetMainViewport()->Pos+Size == io.DisplaySize in single-viewport mode.
		PANDOR_API void SetCursorScreenPos(const Math::Vector2& pos);                          // cursor position in absolute coordinates
		PANDOR_API void AlignTextToFramePadding();                                      // vertically align upcoming text baseline to FramePadding.y so that it will align properly to regularly framed items (call if you have text on a line before a framed item)
		PANDOR_API float GetTextLineHeight();                                            // ~ FontSize
		PANDOR_API float GetTextLineHeightWithSpacing();                                 // ~ FontSize + style.ItemSpacing.y (distance in pixels between 2 consecutive lines of text)
		PANDOR_API float GetFrameHeight();                                               // ~ FontSize + style.FramePadding.y * 2
		PANDOR_API float GetFrameHeightWithSpacing();                                    // ~ FontSize + style.FramePadding.y * 2 + style.ItemSpacing.y (distance in pixels between 2 consecutive lines of framed widgets)

		PANDOR_API void PushID(const char* str_id);                                     // push string into the ID stack (will hash string).
		PANDOR_API void PushID(const char* str_id_begin, const char* str_id_end);       // push string into the ID stack (will hash string).
		PANDOR_API void PushID(const void* ptr_id);                                     // push pointer into the ID stack (will hash pointer).
		PANDOR_API void PushID(int int_id);                                             // push integer into the ID stack (will hash integer).
		PANDOR_API void PopID();                                                        // pop from the ID stack.
		PANDOR_API uint32_t GetID(const char* str_id);                                      // calculate unique ID (hash of whole ID stack + given parameter). e.g. if you want to query into ImGuiStorage yourself
		PANDOR_API uint32_t GetID(const char* str_id_begin, const char* str_id_end);
		PANDOR_API uint32_t GetID(const void* ptr_id);

		PANDOR_API void TextUnformatted(const char* text, const char* text_end = NULL);
		PANDOR_API void Text(const char* fmt, ...);
		PANDOR_API void TextColored(const Math::Vector4& col, const char* fmt, ...);
		PANDOR_API void Image(unsigned int user_texture_id, const Math::Vector2& size, const Math::Vector2& uv0 = Math::Vector2(0, 0), const Math::Vector2& uv1 = Math::Vector2(1, 1), const Math::Vector4& tint_col = Math::Vector4(1, 1, 1, 1), const Math::Vector4& border_col = Math::Vector4(0, 0, 0, 0));
		PANDOR_API bool ImageButton(const char* str_id, void* user_texture_id, const Math::Vector2& size, const Math::Vector2& uv0 = Math::Vector2(0, 0), const Math::Vector2& uv1 = Math::Vector2(1, 1), const Math::Vector4& bg_col = Math::Vector4(0, 0, 0, 0), const Math::Vector4& tint_col = Math::Vector4(1, 1, 1, 1));
		PANDOR_API bool ImageButton(unsigned int user_texture_id, const Math::Vector2& size, const Math::Vector2& uv0 = Math::Vector2(0, 0), const Math::Vector2& uv1 = Math::Vector2(1, 1), int frame_padding = -1, const Math::Vector4& bg_col = Math::Vector4(0, 0, 0, 0), const Math::Vector4& tint_col = Math::Vector4(1, 1, 1, 1)); // Use new ImageButton() signature (explicit item id, regular FramePadding)
		PANDOR_API void TextDisabled(const char* fmt, ...);                              // shortcut for PushStyleColor(ImGuiCol_Text, style.Colors[ImGuiCol_TextDisabled]); Text(fmt, ...); PopStyleColor();
		PANDOR_API void TextWrapped(const char* fmt, ...);                               // shortcut for PushTextWrapPos(0.0f); Text(fmt, ...); PopTextWrapPos();. Note that this won't work on an auto-resizing window if there's no other widgets to extend the window width, yoy may need to set a size using SetNextWindowSize().
		PANDOR_API void LabelText(const char* label, const char* fmt, ...);              // display text+label aligned the same way as value+label widgets
		PANDOR_API void BulletText(const char* fmt, ...);                                // shortcut for Bullet()+Text()
		PANDOR_API void SeparatorText(const char* label);                               // currently: formatted text with an horizontal line

		PANDOR_API bool Button(const char* label, const Math::Vector2& size_arg = Math::Vector2(0, 0));
		PANDOR_API bool SmallButton(const char* label);                                 // button with FramePadding=(0,0) to easily embed within text
		PANDOR_API bool InvisibleButton(const char* str_id, const Math::Vector2& size, ButtonFlags flags = ButtonFlags::None); // flexible button behavior without the visuals, frequently useful to build custom behaviors using the public api (along with IsItemActive, IsItemHovered, etc.)
		PANDOR_API bool ArrowButton(const char* str_id, Dir dir);                  // square button with an arrow shape
		PANDOR_API bool Checkbox(const char* label, bool* v);
		PANDOR_API bool CheckboxFlags(const char* label, int* flags, int flags_value);
		PANDOR_API bool CheckboxFlags(const char* label, unsigned int* flags, unsigned int flags_value);
		PANDOR_API bool RadioButton(const char* label, bool active);                    // use with e.g. if (RadioButton("one", my_value==1)) { my_value = 1; }
		PANDOR_API bool RadioButton(const char* label, int* v, int v_button);           // shortcut to handle the above pattern when value is an integer
		PANDOR_API void ProgressBar(float fraction, const Math::Vector2& size_arg = Math::Vector2(-FLT_MIN, 0), const char* overlay = NULL);
		PANDOR_API void Bullet();

		PANDOR_API bool BeginCombo(const char* label, const char* preview_value, ComboFlags flags = ComboFlags::None);
		PANDOR_API void EndCombo(); // only call EndCombo() if BeginCombo() returns true!
		PANDOR_API bool Combo(const char* label, int* current_item, const char* const items[], int items_count, int popup_max_height_in_items = -1);
		PANDOR_API bool Combo(const char* label, int* current_item, const char* items_separated_by_zeros, int popup_max_height_in_items = -1);      // Separate items with \0 within a string, end item-list with \0\0. e.g. "One\0Two\0Three\0"
		PANDOR_API bool Combo(const char* label, int* current_item, bool(*items_getter)(void* data, int idx, const char** out_text), void* data, int items_count, int popup_max_height_in_items = -1);

		//Drags
		PANDOR_API bool DragFloat(const char* label, float* v, float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const char* format = "%.3f", SliderFlags flags = SliderFlags::None);     // If v_min >= v_max we have no bound
		PANDOR_API bool DragFloat2(const char* label, float v[2], float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const char* format = "%.3f", SliderFlags flags = SliderFlags::None);
		PANDOR_API bool DragFloat3(const char* label, float v[3], float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const char* format = "%.3f", SliderFlags flags = SliderFlags::None);
		PANDOR_API bool DragFloat4(const char* label, float v[4], float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const char* format = "%.3f", SliderFlags flags = SliderFlags::None);
		PANDOR_API bool DragFloatRange2(const char* label, float* v_current_min, float* v_current_max, float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const char* format = "%.3f", const char* format_max = NULL, SliderFlags flags = SliderFlags::None);
		PANDOR_API bool DragInt(const char* label, int* v, float v_speed = 1.0f, int v_min = 0, int v_max = 0, const char* format = "%d", SliderFlags flags = SliderFlags::None);  // If v_min >= v_max we have no bound
		PANDOR_API bool DragInt2(const char* label, int v[2], float v_speed = 1.0f, int v_min = 0, int v_max = 0, const char* format = "%d", SliderFlags flags = SliderFlags::None);
		PANDOR_API bool DragInt3(const char* label, int v[3], float v_speed = 1.0f, int v_min = 0, int v_max = 0, const char* format = "%d", SliderFlags flags = SliderFlags::None);
		PANDOR_API bool DragInt4(const char* label, int v[4], float v_speed = 1.0f, int v_min = 0, int v_max = 0, const char* format = "%d", SliderFlags flags = SliderFlags::None);
		PANDOR_API bool DragIntRange2(const char* label, int* v_current_min, int* v_current_max, float v_speed = 1.0f, int v_min = 0, int v_max = 0, const char* format = "%d", const char* format_max = NULL, SliderFlags flags = SliderFlags::None);
		PANDOR_API bool DragScalar(const char* label, DataType data_type, void* p_data, float v_speed = 1.0f, const void* p_min = NULL, const void* p_max = NULL, const char* format = NULL, SliderFlags flags = SliderFlags::None);
		PANDOR_API bool DragScalarN(const char* label, DataType data_type, void* p_data, int components, float v_speed = 1.0f, const void* p_min = NULL, const void* p_max = NULL, const char* format = NULL, SliderFlags flags = SliderFlags::None);

		PANDOR_API bool SliderFloat(const char* label, float* v, float v_min, float v_max, const char* format = "%.3f", SliderFlags flags = SliderFlags::None);     // adjust format to decorate the value with a prefix or a suffix for in-slider labels or unit display.
		PANDOR_API bool SliderFloat2(const char* label, float v[2], float v_min, float v_max, const char* format = "%.3f", SliderFlags flags = SliderFlags::None);
		PANDOR_API bool SliderFloat3(const char* label, float v[3], float v_min, float v_max, const char* format = "%.3f", SliderFlags flags = SliderFlags::None);
		PANDOR_API bool SliderFloat4(const char* label, float v[4], float v_min, float v_max, const char* format = "%.3f", SliderFlags flags = SliderFlags::None);
		PANDOR_API bool SliderAngle(const char* label, float* v_rad, float v_degrees_min = -360.0f, float v_degrees_max = +360.0f, const char* format = "%.0f deg", SliderFlags flags = SliderFlags::None);
		PANDOR_API bool SliderInt(const char* label, int* v, int v_min, int v_max, const char* format = "%d", SliderFlags flags = SliderFlags::None);
		PANDOR_API bool SliderInt2(const char* label, int v[2], int v_min, int v_max, const char* format = "%d", SliderFlags flags = SliderFlags::None);
		PANDOR_API bool SliderInt3(const char* label, int v[3], int v_min, int v_max, const char* format = "%d", SliderFlags flags = SliderFlags::None);
		PANDOR_API bool SliderInt4(const char* label, int v[4], int v_min, int v_max, const char* format = "%d", SliderFlags flags = SliderFlags::None);
		PANDOR_API bool SliderScalar(const char* label, DataType data_type, void* p_data, const void* p_min, const void* p_max, const char* format = NULL, SliderFlags flags = SliderFlags::None);
		PANDOR_API bool SliderScalarN(const char* label, DataType data_type, void* p_data, int components, const void* p_min, const void* p_max, const char* format = NULL, SliderFlags flags = SliderFlags::None);
		PANDOR_API bool VSliderFloat(const char* label, const Math::Vector2& size, float* v, float v_min, float v_max, const char* format = "%.3f", SliderFlags flags = SliderFlags::None);
		PANDOR_API bool VSliderInt(const char* label, const Math::Vector2& size, int* v, int v_min, int v_max, const char* format = "%d", SliderFlags flags = SliderFlags::None);
		PANDOR_API bool VSliderScalar(const char* label, const Math::Vector2& size, DataType data_type, void* p_data, const void* p_min, const void* p_max, const char* format = NULL, SliderFlags flags = SliderFlags::None);
		PANDOR_API bool SliderMat4(const char* label, float v[16], float v_min, float v_max, const char* format = "%.3f", SliderFlags flags = SliderFlags::None);

		PANDOR_API bool InputText(const char* label, char* buf, size_t buf_size, InputTextFlags flags = InputTextFlags::None, InputTextCallback callback = NULL, void* user_data = NULL);
		PANDOR_API bool InputTextMultiline(const char* label, char* buf, size_t buf_size, const Math::Vector2& size = Math::Vector2(0, 0), InputTextFlags flags = InputTextFlags::None, InputTextCallback callback = NULL, void* user_data = NULL);
		PANDOR_API bool InputTextWithHint(const char* label, const char* hint, char* buf, size_t buf_size, InputTextFlags flags = InputTextFlags::None, InputTextCallback callback = NULL, void* user_data = NULL);
		PANDOR_API bool InputFloat(const char* label, float* v, float step = 0.0f, float step_fast = 0.0f, const char* format = "%.3f", InputTextFlags flags = InputTextFlags::None);
		PANDOR_API bool InputFloat2(const char* label, float v[2], const char* format = "%.3f", InputTextFlags flags = InputTextFlags::None);
		PANDOR_API bool InputFloat3(const char* label, float v[3], const char* format = "%.3f", InputTextFlags flags = InputTextFlags::None);
		PANDOR_API bool InputFloat4(const char* label, float v[4], const char* format = "%.3f", InputTextFlags flags = InputTextFlags::None);
		PANDOR_API bool InputInt(const char* label, int* v, int step = 1, int step_fast = 100, InputTextFlags flags = InputTextFlags::None);
		PANDOR_API bool InputInt2(const char* label, int v[2], InputTextFlags flags = InputTextFlags::None);
		PANDOR_API bool InputInt3(const char* label, int v[3], InputTextFlags flags = InputTextFlags::None);
		PANDOR_API bool InputInt4(const char* label, int v[4], InputTextFlags flags = InputTextFlags::None);
		PANDOR_API bool InputDouble(const char* label, double* v, double step = 0.0, double step_fast = 0.0, const char* format = "%.6f", InputTextFlags flags = InputTextFlags::None);
		PANDOR_API bool InputScalar(const char* label, DataType data_type, void* p_data, const void* p_step = NULL, const void* p_step_fast = NULL, const char* format = NULL, InputTextFlags flags = InputTextFlags::None);
		PANDOR_API bool InputScalarN(const char* label, DataType data_type, void* p_data, int components, const void* p_step = NULL, const void* p_step_fast = NULL, const char* format = NULL, InputTextFlags flags = InputTextFlags::None);

		PANDOR_API bool ColorEdit3(const char* label, float col[3], ColorEditFlags flags = ColorEditFlags::None);
		PANDOR_API bool ColorEdit4(const char* label, float col[4], ColorEditFlags flags = ColorEditFlags::None);
		PANDOR_API bool ColorPicker3(const char* label, float col[3], ColorEditFlags flags = ColorEditFlags::None);
		PANDOR_API bool ColorPicker4(const char* label, float col[4], ColorEditFlags flags = ColorEditFlags::None, const float* ref_col = NULL);
		PANDOR_API bool ColorButton(const char* desc_id, const Math::Vector4& col, ColorEditFlags flags = ColorEditFlags::None, const Math::Vector2& size = Math::Vector2(0, 0)); // display a color square/button, hover for details, return true when pressed.
		PANDOR_API void SetColorEditOptions(ColorEditFlags flags);                     // initialize current options (generally on application startup) if you want to select a default format, picker type, etc. User will be able to change many settings, unless you pass the _NoOptions flag to your calls.

		PANDOR_API bool TreeNode(const char* label);
		PANDOR_API bool TreeNode(const char* str_id, const char* fmt, ...);   // helper variation to easily decorelate the id from the displayed string. Read the FAQ about why and how to use ID. to align arbitrary text at the same level as a TreeNode() you can use Bullet().
		PANDOR_API bool TreeNode(const void* ptr_id, const char* fmt, ...);   // "
		PANDOR_API bool TreeNodeEx(const char* label, TreeNodeFlags flags = TreeNodeFlags::None);
		PANDOR_API bool TreeNodeEx(const char* str_id, TreeNodeFlags flags, const char* fmt, ...);
		PANDOR_API bool TreeNodeEx(const void* ptr_id, TreeNodeFlags flags, const char* fmt, ...);
		PANDOR_API void TreePush(const char* str_id);                                       // ~ Indent()+PushId(). Already called by TreeNode() when returning true, but you can call TreePush/TreePop yourself if desired.
		PANDOR_API void TreePush(const void* ptr_id);                                       // "
		PANDOR_API void TreePop();                                                          // ~ Unindent()+PopId()
		PANDOR_API float GetTreeNodeToLabelSpacing();                                        // horizontal distance preceding label when using TreeNode*() or Bullet() == (g.FontSize + style.FramePadding.x*2) for a regular unframed TreeNode
		PANDOR_API bool CollapsingHeader(const char* label, TreeNodeFlags flags = TreeNodeFlags::None);  // if returning 'true' the header is open. doesn't indent nor push on ID stack. user doesn't have to call TreePop().
		PANDOR_API bool CollapsingHeader(const char* label, bool* p_visible, TreeNodeFlags flags = TreeNodeFlags::None); // when 'p_visible != NULL': if '*p_visible==true' display an additional small close button on upper right of the header which will set the bool to false when clicked, if '*p_visible==false' don't display the header.
		PANDOR_API void SetNextItemOpen(bool is_open, Cond cond = Cond::None);                  // set next TreeNode/CollapsingHeader open state.

		PANDOR_API bool Selectable(const char* label, bool selected = false, SelectableFlags flags = SelectableFlags::None, const Math::Vector2& size = Math::Vector2(0, 0)); // "bool selected" carry the selection state (read-only). Selectable() is clicked is returns true so you can modify your selection state. size.x==0.0: use remaining width, size.x>0.0: specify width. size.y==0.0: use label height, size.y>0.0: specify height
		PANDOR_API bool Selectable(const char* label, bool* p_selected, SelectableFlags flags = SelectableFlags::None, const Math::Vector2& size = Math::Vector2(0, 0));      // "bool* p_selected" point to the selection state (read-write), as a convenient helper.

		PANDOR_API bool BeginListBox(const char* label, const Math::Vector2& size = Math::Vector2(0, 0)); // open a framed scrolling region
		PANDOR_API void EndListBox();                                                       // only call EndListBox() if BeginListBox() returned true!
		PANDOR_API bool ListBox(const char* label, int* current_item, const char* const items[], int items_count, int height_in_items = -1);
		PANDOR_API bool ListBox(const char* label, int* current_item, bool (*items_getter)(void* data, int idx, const char** out_text), void* data, int items_count, int height_in_items = -1);

		PANDOR_API void PlotLines(const char* label, const float* values, int values_count, int values_offset = 0, const char* overlay_text = NULL, float scale_min = FLT_MAX, float scale_max = FLT_MAX, Math::Vector2 graph_size = Math::Vector2(0, 0), int stride = sizeof(float));
		PANDOR_API void PlotLines(const char* label, float(*values_getter)(void* data, int idx), void* data, int values_count, int values_offset = 0, const char* overlay_text = NULL, float scale_min = FLT_MAX, float scale_max = FLT_MAX, Math::Vector2 graph_size = Math::Vector2(0, 0));
		PANDOR_API void PlotHistogram(const char* label, const float* values, int values_count, int values_offset = 0, const char* overlay_text = NULL, float scale_min = FLT_MAX, float scale_max = FLT_MAX, Math::Vector2 graph_size = Math::Vector2(0, 0), int stride = sizeof(float));
		PANDOR_API void PlotHistogram(const char* label, float(*values_getter)(void* data, int idx), void* data, int values_count, int values_offset = 0, const char* overlay_text = NULL, float scale_min = FLT_MAX, float scale_max = FLT_MAX, Math::Vector2 graph_size = Math::Vector2(0, 0));

		PANDOR_API bool BeginMenuBar();                                                     // append to menu-bar of current window (requires ImGuiWindowFlags_MenuBar flag set on parent window).
		PANDOR_API void EndMenuBar();                                                       // only call EndMenuBar() if BeginMenuBar() returns true!
		PANDOR_API bool BeginMainMenuBar();                                                 // create and append to a full screen menu-bar.
		PANDOR_API void EndMainMenuBar();                                                   // only call EndMainMenuBar() if BeginMainMenuBar() returns true!
		PANDOR_API bool BeginMenu(const char* label, bool enabled = true);                  // create a sub-menu entry. only call EndMenu() if this returns true!
		PANDOR_API void EndMenu();                                                          // only call EndMenu() if BeginMenu() returns true!
		PANDOR_API bool MenuItem(const char* label, const char* shortcut = NULL, bool selected = false, bool enabled = true);  // return true when activated.
		PANDOR_API bool MenuItem(const char* label, const char* shortcut, bool* p_selected, bool enabled = true);              // return true when activated + toggle (*p_selected) if p_selected != NULL

		PANDOR_API void BeginTooltip();                                                     // begin/append a tooltip window. to create full-featured tooltip (with any kind of items).
		PANDOR_API void EndTooltip();
		PANDOR_API void SetTooltip(const char* fmt, ...);

		PANDOR_API bool BeginPopup(const char* str_id, WindowFlags flags = WindowFlags::None);                         // return true if the popup is open, and you can start outputting to it.
		PANDOR_API bool BeginPopupModal(const char* name, bool* p_open = NULL, WindowFlags flags = WindowFlags::None); // return true if the modal is open, and you can start outputting to it.
		PANDOR_API void EndPopup();

		PANDOR_API void OpenPopup(const char* str_id, PopupFlags popup_flags = PopupFlags::None);                     // call to mark popup as open (don't call every frame!).
		PANDOR_API void OpenPopup(uint32_t id, PopupFlags popup_flags = PopupFlags::None);                             // id overload to facilitate calling from nested stacks
		PANDOR_API void OpenPopupOnItemClick(const char* str_id = NULL, PopupFlags popup_flags = PopupFlags::MouseButtonRight);   // helper to open popup when clicked on last item. Default to ImGuiPopupFlags_MouseButtonRight == 1. (note: actually triggers on the mouse _released_ event to be consistent with popup behaviors)
		PANDOR_API void CloseCurrentPopup();

		PANDOR_API bool BeginPopupContextItem(const char* str_id = NULL, PopupFlags popup_flags = PopupFlags::MouseButtonRight);  // open+begin popup when clicked on last item. Use str_id==NULL to associate the popup to previous item. If you want to use that on a non-interactive item such as Text() you need to pass in an explicit ID here. read comments in .cpp!
		PANDOR_API bool BeginPopupContextWindow(const char* str_id = NULL, PopupFlags popup_flags = PopupFlags::MouseButtonRight);// open+begin popup when clicked on current window.
		PANDOR_API bool BeginPopupContextVoid(const char* str_id = NULL, PopupFlags popup_flags = PopupFlags::MouseButtonRight);  // open+begin popup when clicked in void (where there are no windows).

		PANDOR_API bool IsPopupOpen(const char* str_id, PopupFlags flags = PopupFlags::None);                         // return true if the popup is open.

		// Tables Function
		PANDOR_API bool BeginTable(const char* str_id, int column, TableFlags flags = TableFlags::None, const Math::Vector2& outer_size = Math::Vector2(0.0f, 0.0f), float inner_width = 0.0f);
		PANDOR_API void EndTable();                                         // only call EndTable() if BeginTable() returns true!
		PANDOR_API void TableNextRow(TableFlags row_flags = TableFlags::None, float min_row_height = 0.0f); // append into the first cell of a new row.
		PANDOR_API bool TableNextColumn();                                  // append into the next column (or first column of next row if currently in last column). Return true when column is visible.
		PANDOR_API bool TableSetColumnIndex(int column_n);                  // append into the specified column. Return true when column is visible.

		PANDOR_API void TableSetupColumn(const char* label, TableColumnFlags flags = TableColumnFlags::None, float init_width_or_weight = 0.0f, uint32_t user_id = 0);
		PANDOR_API void TableSetupScrollFreeze(int cols, int rows);         // lock columns/rows so they stay visible when scrolled.
		PANDOR_API void TableHeadersRow();                                  // submit all headers cells based on data provided to TableSetupColumn() + submit context menu
		PANDOR_API void TableHeader(const char* label);                     // submit one header cell manually (rarely used)

		PANDOR_API TableSortSpecs* TableGetSortSpecs();                        // get latest sort specs for the table (NULL if not sorting).  Lifetime: don't hold on this pointer over multiple frames or past any subsequent call to BeginTable().
		PANDOR_API int TableGetColumnCount();                      // return number of columns (value passed to BeginTable)
		PANDOR_API int TableGetColumnIndex();                      // return current column index.
		PANDOR_API int TableGetRowIndex();                         // return current row index.
		PANDOR_API const char* TableGetColumnName(int column_n = -1);      // return "" if column didn't have a name declared by TableSetupColumn(). Pass -1 to use current column.
		PANDOR_API TableColumnFlags TableGetColumnFlags(int column_n = -1);     // return column flags so you can query their Enabled/Visible/Sorted/Hovered status flags. Pass -1 to use current column.
		PANDOR_API void TableSetColumnEnabled(int column_n, bool v);// change user accessible enabled/disabled state of a column. Set to false to hide the column. User can use the context menu to change this themselves (right-click in headers, or right-click in columns body with ImGuiTableFlags_ContextMenuInBody)
		PANDOR_API void TableSetBgColor(TableBgTarget target, uint32_t color, int column_n = -1);  // change the color of a cell, row, or column. See ImGuiTableBgTarget_ flags for details.

		PANDOR_API void Columns(int count = 1, const char* id = NULL, bool border = true);
		PANDOR_API void NextColumn();                                                       // next column, defaults to current row or next row if the current row is finished
		PANDOR_API int GetColumnIndex();                                                   // get current column index
		PANDOR_API float GetColumnWidth(int column_index = -1);                              // get column width (in pixels). pass -1 to use current column
		PANDOR_API void SetColumnWidth(int column_index, float width);                      // set column width (in pixels). pass -1 to use current column
		PANDOR_API float GetColumnOffset(int column_index = -1);                             // get position of column line (in pixels, from the left side of the contents region). pass -1 to use current column, otherwise 0..GetColumnsCount() inclusive. column 0 is typically 0.0f
		PANDOR_API void SetColumnOffset(int column_index, float offset_x);                  // set position of column line (in pixels, from the left side of the contents region). pass -1 to use current column
		PANDOR_API int GetColumnsCount();

		PANDOR_API bool BeginTabBar(const char* str_id, TabBarFlags flags = TabBarFlags::None);        // create and append into a TabBar
		PANDOR_API void EndTabBar();                                                        // only call EndTabBar() if BeginTabBar() returns true!
		PANDOR_API bool BeginTabItem(const char* label, bool* p_open = NULL, TabItemFlags flags = TabItemFlags::None); // create a Tab. Returns true if the Tab is selected.
		PANDOR_API void EndTabItem();                                                       // only call EndTabItem() if BeginTabItem() returns true!
		PANDOR_API bool TabItemButton(const char* label, TabItemFlags flags = TabItemFlags::None);      // create a Tab behaving like a button. return true when clicked. cannot be selected in the tab bar.
		PANDOR_API void SetTabItemClosed(const char* tab_or_docked_window_label);           // notify TabBar or Docking system of a closed tab/window ahead (useful to reduce visual flicker on reorderable tab bars). For tab-bar: call after BeginTabBar() and before Tab submissions. Otherwise call with a window name.

		PANDOR_API uint32_t DockSpace(uint32_t id, const Math::Vector2& size = Math::Vector2(0, 0), DockNodeFlags flags = DockNodeFlags::None, const WindowClass* window_class = NULL);
		PANDOR_API void SetNextWindowDockID(uint32_t dock_id, Cond cond = Cond::None);           // set next window dock id
		PANDOR_API void SetNextWindowClass(const WindowClass* window_class);           // set next window class (control docking compatibility + provide hints to platform backend via custom viewport flags and platform parent/child relationship)
		PANDOR_API uint32_t GetWindowDockID();
		PANDOR_API bool IsWindowDocked();

		PANDOR_API bool BeginDragDropSource(DragDropFlags flags = DragDropFlags::None);                                      // call after submitting an item which may be dragged. when this return true, you can call SetDragDropPayload() + EndDragDropSource()
		PANDOR_API bool SetDragDropPayload(const char* type, const void* data, size_t sz, Cond cond = Cond::None);  // type is a user defined string of maximum 32 characters. Strings starting with '_' are reserved for dear imgui internal types. Data is copied and held by imgui. Return true when payload has been accepted.
		PANDOR_API void EndDragDropSource();                                                                    // only call EndDragDropSource() if BeginDragDropSource() returns true!
		PANDOR_API bool BeginDragDropTarget();                                                          // call after submitting an item that may receive a payload. If this returns true, you can call AcceptDragDropPayload() + EndDragDropTarget()
		PANDOR_API const Payload* AcceptDragDropPayload(const char* type, DragDropFlags flags = DragDropFlags::None);          // accept contents of a given type. If ImGuiDragDropFlags_AcceptBeforeDelivery is set you can peek into the payload before the mouse button is released.
		PANDOR_API void EndDragDropTarget();                                                            // only call EndDragDropTarget() if BeginDragDropTarget() returns true!
		PANDOR_API const Payload* GetDragDropPayload();                                                           // peek directly into the current payload from anywhere. may return NULL. use ImGuiPayload::IsDataType() to test for the payload type.

		PANDOR_API void BeginDisabled(bool disabled = true);
		PANDOR_API void EndDisabled();

		PANDOR_API void SetItemDefaultFocus();                                              // make last item the default focused item of a window.
		PANDOR_API void SetKeyboardFocusHere(int offset = 0);                               // focus keyboard on the next widget. Use positive 'offset' to access sub components of a multiple component widget. Use -1 to access previous widget.

		PANDOR_API bool IsItemHovered(HoveredFlags flags = HoveredFlags::None);                         // is the last item hovered? (and usable, aka not blocked by a popup, etc.). See ImGuiHoveredFlags for more options.
		PANDOR_API bool IsItemActive();                                                     // is the last item active? (e.g. button being held, text field being edited. This will continuously return true while holding mouse button on an item. Items that don't interact will always return false)
		PANDOR_API bool IsItemFocused();                                                    // is the last item focused for keyboard/gamepad navigation?
		PANDOR_API bool IsItemClicked(MouseButton mouse_button = MouseButton::Left);                   // is the last item hovered and mouse clicked on? (**)  == IsMouseClicked(mouse_button) && IsItemHovered()Important. (**) this is NOT equivalent to the behavior of e.g. Button(). Read comments in function definition.
		PANDOR_API bool IsItemVisible();                                                    // is the last item visible? (items may be out of sight because of clipping/scrolling)
		PANDOR_API bool IsItemEdited();                                                     // did the last item modify its underlying value this frame? or was pressed? This is generally the same as the "bool" return value of many widgets.
		PANDOR_API bool IsItemActivated();                                                  // was the last item just made active (item was previously inactive).
		PANDOR_API bool IsItemDeactivated();                                                // was the last item just made inactive (item was previously active). Useful for Undo/Redo patterns with widgets that require continuous editing.
		PANDOR_API bool IsItemDeactivatedAfterEdit();                                       // was the last item just made inactive and made a value change when it was active? (e.g. Slider/Drag moved). Useful for Undo/Redo patterns with widgets that require continuous editing. Note that you may get false positives (some widgets such as Combo()/ListBox()/Selectable() will return true even when clicking an already selected item).
		PANDOR_API bool IsItemToggledOpen();                                                // was the last item open state toggled? set by TreeNode().
		PANDOR_API bool IsAnyItemHovered();                                                 // is any item hovered?
		PANDOR_API bool IsAnyItemActive();                                                  // is any item active?
		PANDOR_API bool IsAnyItemFocused();                                                 // is any item focused?
		PANDOR_API uint32_t GetItemID();                                                        // get ID of last item (~~ often same ImGui::GetID(label) beforehand)
		PANDOR_API Math::Vector2 GetItemRectMin();                                                   // get upper-left bounding rectangle of the last item (screen space)
		PANDOR_API Math::Vector2 GetItemRectMax();                                                   // get lower-right bounding rectangle of the last item (screen space)
		PANDOR_API Math::Vector2 GetItemRectSize();                                                  // get size of last item
		PANDOR_API void SetItemAllowOverlap();                                              // allow last item to be overlapped by a subsequent item. sometimes useful with invisible buttons, selectables, etc. to catch unused area.

		PANDOR_API double GetTime();                                                          // get global imgui time. incremented by io.DeltaTime every frame.
		PANDOR_API int GetFrameCount();                                                    // get global imgui frame count. incremented by 1 every frame.

		// Color Utilities
		PANDOR_API Math::Vector4 ColorConvertU32ToFloat4(uint32_t in);
		PANDOR_API uint32_t ColorConvertFloat4ToU32(const Math::Vector4& in);
		PANDOR_API void ColorConvertRGBtoHSV(float r, float g, float b, float& out_h, float& out_s, float& out_v);
		PANDOR_API void ColorConvertHSVtoRGB(float h, float s, float v, float& out_r, float& out_g, float& out_b);

		PANDOR_API bool IsKeyDown(Key key);                                            // is key being held.
		PANDOR_API bool IsKeyPressed(Key key, bool repeat = true);                     // was key pressed (went from !Down to Down)? if repeat=true, uses io.KeyRepeatDelay / KeyRepeatRate
		PANDOR_API bool IsKeyReleased(Key key);                                        // was key released (went from Down to !Down)?
		PANDOR_API int GetKeyPressedAmount(Key key, float repeat_delay, float rate);  // uses provided repeat rate/delay. return a count, most often 0 or 1 but might be >1 if RepeatRate is small enough that DeltaTime > RepeatRate
		PANDOR_API const char* GetKeyName(Key key);                                           // [DEBUG] returns English name of the key. Those names a provided for debugging purpose and are not meant to be saved persistently not compared.
		PANDOR_API void SetNextFrameWantCaptureKeyboard(bool want_capture_keyboard);        // Override io.WantCaptureKeyboard flag next frame (said flag is left for your application to handle, typically when true it instructs your app to ignore inputs). e.g. force capture keyboard when your widget is being hovered. This is equivalent to setting "io.WantCaptureKeyboard = want_capture_keyboard"; after the next NewFrame() call.

		PANDOR_API bool IsMouseDown(MouseButton button);                               // is mouse button held?
		PANDOR_API bool IsMouseClicked(MouseButton button, bool repeat = false);       // did mouse button clicked? (went from !Down to Down). Same as GetMouseClickedCount() == 1.
		PANDOR_API bool IsMouseReleased(MouseButton button);                           // did mouse button released? (went from Down to !Down)
		PANDOR_API bool IsMouseDoubleClicked(MouseButton button);                      // did mouse button double-clicked? Same as GetMouseClickedCount() == 2. (note that a double-click will also report IsMouseClicked() == true)
		PANDOR_API int GetMouseClickedCount(MouseButton button);                      // return the number of successive mouse-clicks at the time where a click happen (otherwise 0).
		PANDOR_API bool IsMouseHoveringRect(const Math::Vector2& r_min, const Math::Vector2& r_max, bool clip = true);// is mouse hovering given bounding rect (in screen space). clipped by current clipping settings, but disregarding of other consideration of focus/window ordering/popup-block.
		PANDOR_API bool IsMousePosValid(const Math::Vector2* mouse_pos = NULL);                    // by convention we use (-FLT_MAX,-FLT_MAX) to denote that there is no mouse available
		PANDOR_API bool IsAnyMouseDown();                                                   // [WILL OBSOLETE] is any mouse button held? This was designed for backends, but prefer having backend maintain a mask of held mouse buttons, because upcoming input queue system will make this invalid.
		PANDOR_API Math::Vector2 GetMousePos();                                                      // shortcut to ::GetIO().MousePos provided by user, to be consistent with other calls
		PANDOR_API Math::Vector2 GetMousePosOnOpeningCurrentPopup();                                 // retrieve mouse position at the time of opening popup we have BeginPopup() into (helper to avoid user backing that value themselves)
		PANDOR_API bool IsMouseDragging(MouseButton button, float lock_threshold = -1.0f);         // is mouse dragging? (if lock_threshold < -1.0f, uses io.MouseDraggingThreshold)
		PANDOR_API Math::Vector2 GetMouseDragDelta(MouseButton button = MouseButton::Left, float lock_threshold = -1.0f);   // return the delta from the initial clicking position while the mouse button is pressed or was just released. This is locked and return 0.0f until the mouse moves past a distance threshold at least once (if lock_threshold < -1.0f, uses io.MouseDraggingThreshold)
		PANDOR_API void ResetMouseDragDelta(MouseButton button = MouseButton::Left);                   //
		PANDOR_API MouseCursor GetMouseCursor();                                                // get desired mouse cursor shape. Important: reset in ::NewFrame(), this is updated during the frame. valid before Render(). If you use software rendering by setting io.MouseDrawCursor  will render those for you
		PANDOR_API void SetMouseCursor(MouseCursor cursor_type);                       // set desired mouse cursor shape
		PANDOR_API void SetNextFrameWantCaptureMouse(bool want_capture_mouse);              // Override io.WantCaptureMouse flag next frame (said flag is left for your application to handle, typical when true it instucts your app to ignore inputs). This is equivalent to setting "io.WantCaptureMouse = want_capture_mouse;" after the next NewFrame() call.

		PANDOR_API const char* GetClipboardText();
		PANDOR_API void SetClipboardText(const char* text);

		PANDOR_API Math::Vector2 GetItemSpacing();
	}
}