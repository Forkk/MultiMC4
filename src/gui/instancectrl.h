/*
 * Derived from the thumbnail control example by Julian Smart
 */
#pragma once

#include "wx/dynarray.h"
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
	bool operator!=(VisualCoord& other) const
	{
		return other.groupIndex != groupIndex || other.itemIndex != itemIndex;
	};
	
	bool isGroup() const
	{
		return groupIndex >= 0 && itemIndex == -1;
	};
	bool isItem() const
	{
		return groupIndex >= 0 && itemIndex >= 0;
	};
	bool isVoid() const
	{
		return groupIndex == -1 && itemIndex == -1;
	}
	void makeVoid()
	{
		groupIndex = itemIndex = -1;
	};
	
	int groupIndex;
	int itemIndex;
};

struct GroupVisual
{
	GroupVisual(wxString & name, bool no_header = false):name(name),no_header(no_header)
	{
		expanded = true;
		total_height = 0;
		header_height = 0;
		y_position = 0;
		index = -1;
	};
	void Reflow ( int perRow, int spacing, int margin, int lineHeight, int imageSize, int & progressive_y );
	void Draw ( wxDC & dc, InstanceCtrl* parent, wxRect untransformedRect,
	            bool hasSelection, int selectionIndex,
	            bool hasFocus, int focusIndex );
	void SetIndex (int index)
	{
		this->index = index;
	}
	
	wxString name;
	InstanceItemArray items;
	
	/// if the header is enabled, this decides if the rest of the group should be drawn and incluced in the height
	bool expanded;
	
	/// don't draw header and don't include it in height if true
	bool no_header;
	
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
		for(int i = 0; i < m_groups.size(); i++)
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
	
	/// Left-click
	void OnLeftClick(wxMouseEvent& event);
	
	/// Left-double-click
	void OnLeftDClick(wxMouseEvent& event);
	
	/// Middle-click
	void OnMiddleClick(wxMouseEvent& event);
	
	/// Right-click
	void OnRightClick(wxMouseEvent& event);
	
	/// Key press
	void OnChar(wxKeyEvent& event);
	
	/// Sizing
	void OnSize(wxSizeEvent& event);
	
	/// Setting/losing focus
	void OnSetFocus(wxFocusEvent& event);
	void OnKillFocus(wxFocusEvent& event);
	
// Implementation
private:

	/// Update the row heights for layouting.
	void ReflowAll( );
	void ReflowGroup( GroupVisual & group );
	
	/// Set up scrollbars, e.g. after a resize
	void SetupScrollbars();
	
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
	
	/// Get the current items per row value
	void SetItemsPerRow(int itpw)
	{
		m_itemsPerRow = itpw;
	}
	
	/// Do (de)selection
	void DoSelection(VisualCoord n);
	
	/// Find the item under the given point
	bool HitTest(const wxPoint& pt, VisualCoord& n);
	
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
	
protected:
	VisualCoord   m_itemIndex;
	int           m_itemID;
	int           m_flags;
	wxPoint       m_position;
	
private:
	DECLARE_DYNAMIC_CLASS_NO_ASSIGN(InstanceCtrlEvent)
};

/*!
 * wxInstanceCtrl event macros
 */

BEGIN_DECLARE_EVENT_TYPES()
DECLARE_EVENT_TYPE(wxEVT_COMMAND_INST_ITEM_SELECTED, 2600)
DECLARE_EVENT_TYPE(wxEVT_COMMAND_INST_ITEM_DESELECTED, 2601)
DECLARE_EVENT_TYPE(wxEVT_COMMAND_INST_LEFT_CLICK, 2602)
DECLARE_EVENT_TYPE(wxEVT_COMMAND_INST_RIGHT_CLICK, 2603)
DECLARE_EVENT_TYPE(wxEVT_COMMAND_INST_MIDDLE_CLICK, 2604)
DECLARE_EVENT_TYPE(wxEVT_COMMAND_INST_LEFT_DCLICK, 2605)
DECLARE_EVENT_TYPE(wxEVT_COMMAND_INST_RETURN, 2606)
DECLARE_EVENT_TYPE(wxEVT_COMMAND_INST_DELETE, 2607)
DECLARE_EVENT_TYPE(wxEVT_COMMAND_INST_RENAME, 2608)
END_DECLARE_EVENT_TYPES()

typedef void (wxEvtHandler::*wxInstanceCtrlEventFunction)(InstanceCtrlEvent&);

#define EVT_INST_ITEM_SELECTED(id, fn) DECLARE_EVENT_TABLE_ENTRY( wxEVT_COMMAND_INST_ITEM_SELECTED, id, -1, (wxObjectEventFunction) (wxEventFunction)  wxStaticCastEvent( wxInstanceCtrlEventFunction, & fn ), NULL ),
#define EVT_INST_ITEM_DESELECTED(id, fn) DECLARE_EVENT_TABLE_ENTRY( wxEVT_COMMAND_INST_ITEM_DESELECTED, id, -1, (wxObjectEventFunction) (wxEventFunction)  wxStaticCastEvent( wxInstanceCtrlEventFunction, & fn ), NULL ),
#define EVT_INST_LEFT_CLICK(id, fn) DECLARE_EVENT_TABLE_ENTRY( wxEVT_COMMAND_INST_LEFT_CLICK, id, -1, (wxObjectEventFunction) (wxEventFunction)  wxStaticCastEvent( wxInstanceCtrlEventFunction, & fn ), NULL ),
#define EVT_INST_RIGHT_CLICK(id, fn) DECLARE_EVENT_TABLE_ENTRY( wxEVT_COMMAND_INST_RIGHT_CLICK, id, -1, (wxObjectEventFunction) (wxEventFunction)  wxStaticCastEvent( wxInstanceCtrlEventFunction, & fn ), NULL ),
#define EVT_INST_MIDDLE_CLICK(id, fn) DECLARE_EVENT_TABLE_ENTRY( wxEVT_COMMAND_INST_MIDDLE_CLICK, id, -1, (wxObjectEventFunction) (wxEventFunction)  wxStaticCastEvent( wxInstanceCtrlEventFunction, & fn ), NULL ),
#define EVT_INST_LEFT_DCLICK(id, fn) DECLARE_EVENT_TABLE_ENTRY( wxEVT_COMMAND_INST_LEFT_DCLICK, id, -1, (wxObjectEventFunction) (wxEventFunction)  wxStaticCastEvent( wxInstanceCtrlEventFunction, & fn ), NULL ),
#define EVT_INST_RETURN(id, fn) DECLARE_EVENT_TABLE_ENTRY( wxEVT_COMMAND_INST_RETURN, id, -1, (wxObjectEventFunction) (wxEventFunction)  wxStaticCastEvent( wxInstanceCtrlEventFunction, & fn ), NULL ),
#define EVT_INST_DELETE(id, fn) DECLARE_EVENT_TABLE_ENTRY( wxEVT_COMMAND_INST_DELETE, id, -1, (wxObjectEventFunction) (wxEventFunction)  wxStaticCastEvent( wxInstanceCtrlEventFunction, & fn ), NULL ),
#define EVT_INST_RENAME(id, fn) DECLARE_EVENT_TABLE_ENTRY( wxEVT_COMMAND_INST_RENAME, id, -1, (wxObjectEventFunction) (wxEventFunction)  wxStaticCastEvent( wxInstanceCtrlEventFunction, & fn ), NULL ),

