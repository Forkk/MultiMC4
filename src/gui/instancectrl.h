/*
 * Derived from the thumbnail control example by Julian Smart
 */
#pragma once

#include "wx/dynarray.h"
#include "wx/dnd.h"
#include <instance.h>
#include <insticonlist.h>

#define wxINST_SINGLE_COLUMN      0x0020

/* Flags
 */

#define wxINST_SHIFT_DOWN  0x01
#define wxINST_CTRL_DOWN   0x02
#define wxINST_ALT_DOWN    0x04

/* Defaults
 */

#define wxINST_DEFAULT_IMAGE_SIZE wxSize(32, 32)
#define wxINST_DEFAULT_SPACING 3
#define wxINST_DEFAULT_MARGIN 3

/*!
 * Forward declarations
 */

class InstanceModel;

class InstanceCtrl;

/*!
 * wxInstanceItem class declaration
 */

// Drawing styles/states
#define wxINST_SELECTED    0x01
// The control is focussed
#define wxINST_FOCUSED    0x04
// The item itself has the focus
#define wxINST_IS_FOCUS    0x08

class InstanceCtrlEvent;

class InstanceVisual
{
public:
// Constructors

	InstanceVisual()
	{
		m_inst = nullptr;
		m_id = -1;
		updateName();
	}

	InstanceVisual(Instance* inst, int ID)
	{
		SetInstance(inst, ID);
	}

	Instance *GetInstance() const
	{
		return m_inst;
	}

	void SetInstance(Instance* inst, int ID)
	{
		m_inst = inst;
		m_id = ID;
		updateName();
	}
	
	const wxString GetName() const
	{
		return m_inst->GetName();
	}
	
	int GetID() const
	{
		return m_id;
	}
	
	void updateName();
	
	int GetNumLines()
	{
		return text_lines;
	};
	
	/// Draw the item
	bool Draw(wxDC& dc, InstanceCtrl* ctrl, const wxRect& rect, int style);
	
protected:
	Instance*   m_inst;
	int         m_id;
	int         text_width;
	wxString    name_wrapped;
	int         text_lines;
};

WX_DECLARE_OBJARRAY(InstanceVisual, InstanceItemArray);

struct VisualCoord
{
public:
	VisualCoord(int groupIndex = -1, int itemIndex = -1):itemIndex(itemIndex), groupIndex(groupIndex){};
	
	bool operator==(VisualCoord& other) const
	{
		return other.groupIndex == groupIndex && other.itemIndex == itemIndex;
	};

	void operator=(const VisualCoord& other)
	{
		groupIndex = other.groupIndex;
		itemIndex = other.itemIndex;
	};

	bool operator!=(VisualCoord& other) const
	{
		return other.groupIndex != groupIndex || other.itemIndex != itemIndex;
	};
	
	bool isGroup() const
	{
		return groupIndex >= 0 && itemIndex < 0;
	};
	bool isItem() const
	{
		return groupIndex >= 0 && itemIndex >= 0;
	};
	bool isVoid() const
	{
		return groupIndex < 0 && itemIndex < 0;
	}
	bool isHeader() const
	{
		return groupIndex >= 0 && itemIndex == -2;
	}
	bool isHeaderTicker() const
	{
		return groupIndex >= 0 && itemIndex == -3;
	}
	void makeGroup(int group)
	{
		groupIndex = group;
		itemIndex = -1;
	}
	void makeHeader(int group)
	{
		groupIndex = group;
		itemIndex = -2;
	}
	void makeHeaderTicker(int group)
	{
		groupIndex = group;
		itemIndex = -3;
	}
	void makeVoid()
	{
		groupIndex = itemIndex = -1;
	};
	
	int groupIndex;
	int itemIndex;
};

class InstanceGroup;

struct GroupVisual
{
	GroupVisual(InstanceGroup *group, bool no_header = false);
	void Reflow ( int perRow, int spacing, int margin, int lineHeight, int imageSize, int & progressive_y );
	void Draw ( wxDC & dc, InstanceCtrl* parent, wxRect untransformedRect,
	            bool hasSelection, int selectionIndex,
	            bool hasFocus, int focusIndex, bool highlight = false );
	void SetIndex (int index)
	{
		this->index = index;
	}

	bool IsExpanded() const;
	void SetExpanded(bool expanded);

	wxString GetName() const;
	bool isUngrouped() const;

	InstanceGroup *m_group;
	InstanceItemArray items;
	
	/// don't draw header and don't include it in height if true
	bool no_header;

	/// ignore whether or not the group is marked as hidden
	bool always_show;
	
	/// absolute y-position of the top of the group
	int y_position;
	
	/// total height in pixels
	int total_height;
	
	/// height of the header in pixels
	int header_height;
	
	/// y positions where each row starts
	wxArrayInt               row_ys;
	
	/// height of each row (spacing not included)
	wxArrayInt               row_heights;
	
	/// sorted index of this group
	int index;
};
WX_DECLARE_OBJARRAY(GroupVisual, InstanceGroupArray);

class InstanceCtrl: public wxScrolledCanvas
{
	DECLARE_CLASS(InstanceCtrl)
	DECLARE_EVENT_TABLE()
	
public:
// Constructors

	InstanceCtrl();
	InstanceCtrl(wxWindow* parent, InstanceModel *instList, wxWindowID id = -1, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0);
	
// Operations

	/// Creation
	bool Create(wxWindow* parent, InstanceModel *instList, wxWindowID id = -1, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0);
	
	/// Member initialisation
	void Init();
	
	/// Call Freeze to prevent refresh
	void Freeze();
	
	/// Call Thaw to refresh
	void Thaw();

	/// Reloads the items from the instance model
	void ReloadAll();
	
// Accessing items

	/// Get the number of groups in the control
	virtual int GetCount() const
	{
		return m_groups.GetCount();
	}
	
	/// Is the control empty?
	bool IsEmpty() const
	{
		return GetCount() == 0;
	}
	
	/// Get the nth item
	InstanceVisual* GetItem(VisualCoord n) const;

	/// Get the nth group
	GroupVisual* GetGroup(VisualCoord n) const;
	
	/// Get the overall rect of the given item
	/// If view_relative is true, rect is relative to the scroll viewport
	/// (i.e. may be negative)
	bool GetItemRect(VisualCoord item, wxRect& rect, bool view_relative = true);
	
	/// Get the overall rect of the given group
	/// If view_relative is true, rect is relative to the scroll viewport
	/// (i.e. may be negative)
	bool GetGroupRect(int group, wxRect& rect, bool view_relative = true);
	
	/// Return the row and column given the client size
	bool GetRowCol(VisualCoord item, int& row, int& col);
	
// Selection

	/// Select or deselect an item
	void Select(VisualCoord n, bool select = true) ;
	
	/// Get the index of the single selection, if not multi-select.
	/// Returns -1 if there is no selection.
	VisualCoord GetSelection() const ;
	
	/// Returns true if the item is selected
	bool IsSelected(VisualCoord n) const ;
	
	/// Clears all selections
	void ClearSelections();
	
	int GetSuggestedPostRemoveID(int removedID);
	
// Visual properties

	const wxSize& GetImageSize() const
	{
		return m_ImageSize;
	}
	
	/// The inter-item spacing
	void SetSpacing(int spacing)
	{
		m_spacing = spacing;
	}
	int GetSpacing() const
	{
		return m_spacing;
	}
	
	/// The margin between elements within the item
	void SetItemMargin(int margin)
	{
		m_itemMargin = margin;
	}
	int GetItemMargin() const
	{
		return m_itemMargin;
	}
	
	/// get height of item n
	int GetItemHeight(VisualCoord n) const
	{
		InstanceVisual* item = GetItem(n);
		return m_itemMargin * 3 + m_ImageSize.y + item->GetNumLines() * m_itemTextHeight;
	}
	
	/// get total height of the control
	int GetTotalHeight() const
	{
		int total = 0;
		for(unsigned i = 0; i < m_groups.size(); i++)
		{
			GroupVisual & grp = m_groups[i];
			total += grp.total_height;
		}
		return total;
	}
// Event handlers

	/// Painting
	void OnPaint(wxPaintEvent& event);
	void OnEraseBackground(wxEraseEvent& event){};
	
	/// input event handlers
	void OnLeftClick(wxMouseEvent& event);
	void OnLeftDClick(wxMouseEvent& event);
	void OnRightClick(wxMouseEvent& event);
	void OnMouseMotion(wxMouseEvent& event);
	void OnChar(wxKeyEvent& event);

	// Other
	void OnInstDragged(InstanceCtrlEvent& event);

	class InstCtrlDropTarget : public wxTextDropTarget
	{
	public:
		InstCtrlDropTarget(InstanceCtrl* parent)
			: wxTextDropTarget()
		{
			m_parent = parent;
		}

		virtual bool OnDropText(wxCoord x, wxCoord y, const wxString& data);
		virtual wxDragResult OnDragOver(wxCoord x, wxCoord y, wxDragResult def);
		virtual void OnLeave();

	protected:
		InstanceCtrl* m_parent;
	} *dtarget;
	
	/// Sizing
	void OnSize(wxSizeEvent& event);
	
	/// Setting/losing focus
	void OnSetFocus(wxFocusEvent& event);
	void OnKillFocus(wxFocusEvent& event);

	void HighlightGroup(const VisualCoord& coord);

	VisualCoord highlightedGroup;
	
// Implementation
private:

	/// Update the row heights for layouting.
	void ReflowAll( );
	void ReflowGroup( GroupVisual & group );
	
	/// Set up scrollbars, e.g. after a resize
	void SetupScrollbars();
	
	/// Scroll an item into the view by VisualCoord
	void EnsureVisible(VisualCoord coord);
	
	/// Scroll a rectangle in absolute coordinates into view
	void EnsureRectVisible(wxRect rect);
	
	/// Keyboard input navigation
	bool Navigate(int keyCode, int flags);
	
	/// Set the intended navigation column based on coord
	void SetIntendedColumn( VisualCoord coord );
	
	/// Calculate the outer item size based
	/// on font used for text and inner size
	void CalculateOverallItemSize();
	
	/// Calculate items per row based on current size
	int CalculateItemsPerRow();
	
	/// Get the current items per row value
	int GetItemsPerRow() const
	{
		if ((GetWindowStyle() & wxINST_SINGLE_COLUMN) != 0)
			return 1;
		return m_itemsPerRow;
	}
	
	/// Set the current items per row value
	void SetItemsPerRow(int itpw)
	{
		m_itemsPerRow = itpw;
		if(m_intended_column != -1)
		{
			int row, col;
			if(GetRowCol(m_selectedItem, row, col))
			{
				m_intended_column = col;
			}
		}
	}
	
	/// Do (de)selection
	void DoSelection(VisualCoord n);
	
	/// Find the item under the given point
	void HitTest(const wxPoint& pt, VisualCoord& n);
	
	/// Toggle the expansion of a group (user input driven)
	void ToggleGroup(int index);
	
	/// Paint the background
	void PaintBackground(wxDC& dc);
	
	/// Recreate buffer bitmap if necessary
	bool RecreateBuffer(const wxSize& size = wxDefaultSize);
	
	/// Get item index from ID
	VisualCoord IndexFromID(int ID) const;
	/// Get item ID from index
	int IDFromIndex(VisualCoord index) const;
	
// Overrides
	wxSize DoGetBestSize() const ;
	
// Data members
private:
	/// The items
	//InstanceItemArray        m_items;
	InstanceGroupArray       m_groups;
	
	/// Mapping from IDs to indexes
	std::vector<VisualCoord> m_itemIndexes;
	
	/// The currently selected item
	VisualCoord              m_selectedItem;
	
	/// Which column is intended for navigation (up/down)
	int                      m_intended_column;
	
	/// Focus item - doesn't have to be a real item (can be group header, etc.)
	VisualCoord              m_focusItem;
	
	/// Instance list pointer
	InstanceModel*           m_instList;
	
	/// Outer size of the item
	int                      m_itemWidth;
	
	/// Image size of the item
	wxSize                   m_ImageSize;
	
	/// The inter-item spacing
	int                      m_spacing;
	
	/// The margin between the image/text and the edge of the item
	int                      m_itemMargin;
	
	/// The height of item text in the current font
	int                      m_itemTextHeight;
	
	/// Allows nested Freeze/Thaw
	int                      m_freezeCount;
	
	/// Buffer bitmap
	wxBitmap                 m_bufferBitmap;
	
	/// items per row - cached
	int                      m_itemsPerRow;
};

/*!
 * wxInstanceCtrlEvent - the event class for wxInstanceCtrl notifications
 */

class InstanceCtrlEvent : public wxNotifyEvent
{
public:
	InstanceCtrlEvent(wxEventType commandType = wxEVT_NULL, int winid = 0, wxPoint position = wxPoint(-1, -1))
		: wxNotifyEvent(commandType, winid),
		  m_itemIndex(-1), m_itemID(-1), m_flags(0), m_position(position)
	{ }
	
	InstanceCtrlEvent(const InstanceCtrlEvent& event)
		: wxNotifyEvent(event),
		  m_itemIndex(event.m_itemIndex), m_itemID(event.m_itemID), m_flags(event.m_flags), m_position(event.m_position)
	{ }
	
	VisualCoord GetItemIndex() const
	{
		return m_itemIndex;
	}
	void SetItemIndex(VisualCoord n)
	{
		m_itemIndex = n;
	}
	
	int GetItemID() const
	{
		return m_itemID;
	}
	void SetItemID(int n)
	{
		m_itemID = n;
	}
	
	int GetFlags() const
	{
		return m_flags;
	}
	void SetFlags(int flags)
	{
		m_flags = flags;
	}
	
	wxPoint GetPosition() const
	{
		return m_position;
	};
	void SetPosition(wxPoint position)
	{
		m_position = position;
	};
	virtual wxEvent* Clone() const
	{
		return new InstanceCtrlEvent(*this);
	}

	InstanceGroup *GetGroup() const
	{
		return m_group;
	}

	void SetGroup(InstanceGroup *group)
	{
		m_group = group;
	}
	
protected:
	VisualCoord   m_itemIndex;
	int           m_itemID;
	int           m_flags;
	wxPoint       m_position;

	InstanceGroup *m_group;
	
private:
	DECLARE_DYNAMIC_CLASS_NO_ASSIGN(InstanceCtrlEvent)
};

/*!
 * wxInstanceCtrl event macros
 */

BEGIN_DECLARE_EVENT_TYPES()
DECLARE_EVENT_TYPE(wxEVT_COMMAND_INST_ITEM_SELECTED, 2600)
DECLARE_EVENT_TYPE(wxEVT_COMMAND_INST_ITEM_DESELECTED, 2601)
DECLARE_EVENT_TYPE(wxEVT_COMMAND_INST_MENU, 2602)
DECLARE_EVENT_TYPE(wxEVT_COMMAND_INST_ACTIVATE, 2603)
DECLARE_EVENT_TYPE(wxEVT_COMMAND_INST_DELETE, 2604)
DECLARE_EVENT_TYPE(wxEVT_COMMAND_INST_RENAME, 2605)
DECLARE_EVENT_TYPE(wxEVT_COMMAND_INST_DRAG, 2606)
END_DECLARE_EVENT_TYPES()

typedef void (wxEvtHandler::*wxInstanceCtrlEventFunction)(InstanceCtrlEvent&);

#define EVT_INST_ITEM_SELECTED(id, fn)\
	DECLARE_EVENT_TABLE_ENTRY\
	(\
		wxEVT_COMMAND_INST_ITEM_SELECTED,\
		id,\
		-1,\
		(wxObjectEventFunction) (wxEventFunction)  wxStaticCastEvent( wxInstanceCtrlEventFunction, & fn ),\
		NULL\
	),
#define EVT_INST_ITEM_DESELECTED(id, fn)\
	DECLARE_EVENT_TABLE_ENTRY\
	(\
		wxEVT_COMMAND_INST_ITEM_DESELECTED,\
		id,\
		-1,\
		(wxObjectEventFunction) (wxEventFunction)  wxStaticCastEvent( wxInstanceCtrlEventFunction, & fn ),\
		NULL\
	),
#define EVT_INST_ACTIVATE(id, fn)\
	DECLARE_EVENT_TABLE_ENTRY\
	(\
		wxEVT_COMMAND_INST_ACTIVATE,\
		id,\
		-1,\
		(wxObjectEventFunction) (wxEventFunction)  wxStaticCastEvent( wxInstanceCtrlEventFunction, & fn ),\
		NULL\
	),
#define EVT_INST_MENU(id, fn)\
	DECLARE_EVENT_TABLE_ENTRY\
	(\
		wxEVT_COMMAND_INST_MENU,\
		id,\
		-1,\
		(wxObjectEventFunction) (wxEventFunction)  wxStaticCastEvent( wxInstanceCtrlEventFunction, & fn ),\
		NULL\
	),
#define EVT_INST_DELETE(id, fn)\
	DECLARE_EVENT_TABLE_ENTRY\
	(\
		wxEVT_COMMAND_INST_DELETE,\
		id,\
		-1,\
		(wxObjectEventFunction) (wxEventFunction)  wxStaticCastEvent( wxInstanceCtrlEventFunction, & fn ),\
		NULL\
	),
#define EVT_INST_RENAME(id, fn)\
	DECLARE_EVENT_TABLE_ENTRY\
	(\
		wxEVT_COMMAND_INST_RENAME,\
		id,\
		-1,\
		(wxObjectEventFunction) (wxEventFunction)  wxStaticCastEvent( wxInstanceCtrlEventFunction, & fn ),\
		NULL\
	),

#define EVT_INST_DRAG(id, fn)\
	DECLARE_EVENT_TABLE_ENTRY\
	(\
		wxEVT_COMMAND_INST_DRAG,\
		id,\
		-1,\
		(wxObjectEventFunction) (wxEventFunction)  wxStaticCastEvent( wxInstanceCtrlEventFunction, & fn ),\
		NULL\
	),

