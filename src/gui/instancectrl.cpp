/*
 * Derived from the thumbnail control example by Julian Smart
 */

// For compilers that support precompilation, includes "wx.h".
#include "wx/wxprec.h"

#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif

#include "instancectrl.h"

#include "wx/settings.h"
#include "wx/arrimpl.cpp"
#include "wx/image.h"
#include "wx/dcbuffer.h"

#include "instancemodel.h"

WX_DEFINE_OBJARRAY(wxInstanceItemArray);

DEFINE_EVENT_TYPE(wxEVT_COMMAND_INST_ITEM_SELECTED)
DEFINE_EVENT_TYPE(wxEVT_COMMAND_INST_ITEM_DESELECTED)
DEFINE_EVENT_TYPE(wxEVT_COMMAND_INST_LEFT_CLICK)
DEFINE_EVENT_TYPE(wxEVT_COMMAND_INST_MIDDLE_CLICK)
DEFINE_EVENT_TYPE(wxEVT_COMMAND_INST_RIGHT_CLICK)
DEFINE_EVENT_TYPE(wxEVT_COMMAND_INST_LEFT_DCLICK)
DEFINE_EVENT_TYPE(wxEVT_COMMAND_INST_RETURN)
DEFINE_EVENT_TYPE(wxEVT_COMMAND_INST_DELETE)
DEFINE_EVENT_TYPE(wxEVT_COMMAND_INST_RENAME)

IMPLEMENT_CLASS(wxInstanceCtrl, wxScrolledCanvas)
IMPLEMENT_CLASS(wxInstanceItem, wxObject)
IMPLEMENT_CLASS(wxInstanceCtrlEvent, wxNotifyEvent)

BEGIN_EVENT_TABLE(wxInstanceCtrl, wxScrolledCanvas)
	EVT_PAINT(wxInstanceCtrl::OnPaint)
	EVT_ERASE_BACKGROUND(wxInstanceCtrl::OnEraseBackground)
	EVT_LEFT_DOWN(wxInstanceCtrl::OnLeftClick)
	EVT_RIGHT_DOWN(wxInstanceCtrl::OnRightClick)
	EVT_MIDDLE_DOWN(wxInstanceCtrl::OnMiddleClick)
	EVT_LEFT_DCLICK(wxInstanceCtrl::OnLeftDClick)
	EVT_CHAR(wxInstanceCtrl::OnChar)
	EVT_SIZE(wxInstanceCtrl::OnSize)
	EVT_SET_FOCUS(wxInstanceCtrl::OnSetFocus)
	EVT_KILL_FOCUS(wxInstanceCtrl::OnKillFocus)
END_EVENT_TABLE()

/*!
 * wxInstanceCtrl
 */
wxInstanceCtrl::wxInstanceCtrl()
{
	Init();
}

wxInstanceCtrl::wxInstanceCtrl(wxWindow* parent, InstanceModel *instList, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
{
	Init();
	Create(parent, instList, id, pos, size, style);
}

/// Creation
bool wxInstanceCtrl::Create(wxWindow* parent, InstanceModel *instList, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
{
	m_instList = instList;

	if (!wxScrolledCanvas::Create(parent, id, pos, size, style | wxFULL_REPAINT_ON_RESIZE | wxWANTS_CHARS))
		return false;
		
	SetFont(wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT));
	CalculateOverallItemSize();
	m_itemsPerRow = CalculateItemsPerRow();
	
	SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW));
	SetBackgroundStyle(wxBG_STYLE_CUSTOM);
	DisableKeyboardScrolling();
	ShowScrollbars(wxSHOW_SB_NEVER, wxSHOW_SB_ALWAYS);
	
	// Tell the sizers to use the given or best size
	SetInitialSize(size);
	
	// Create a buffer
	RecreateBuffer(size);
	
	return true;
}

/// Member initialisation
void wxInstanceCtrl::Init()
{
	m_itemWidth = -1;
	m_ImageSize = wxINST_DEFAULT_IMAGE_SIZE;
	m_freezeCount = 0;
	m_spacing = wxINST_DEFAULT_SPACING;
	m_itemMargin = wxINST_DEFAULT_MARGIN;
	m_firstSelection = -1;
	m_lastSelection = -1;
	m_focusItem = -1;
}

/// Call Freeze to prevent refresh
void wxInstanceCtrl::Freeze()
{
	m_freezeCount ++;
}

/// Call Thaw to refresh
void wxInstanceCtrl::Thaw()
{
	m_freezeCount --;
	
	if (m_freezeCount == 0)
	{
		UpdateRows();
		SetupScrollbars();
		Refresh();
	}
}

/// Get the nth item
wxInstanceItem* wxInstanceCtrl::GetItem(int n)
{
	wxASSERT(n < GetCount());
	
	if (n < GetCount())
	{
		return & m_items[(size_t) n];
	}
	else
		return NULL;
}

/// Get the overall rect of the given item
bool wxInstanceCtrl::GetItemRect(int n, wxRect& rect, bool view_relative)
{
	//wxASSERT(n < GetCount());
	if (n < GetCount())
	{
		int row, col;
		if (!GetRowCol(n, GetClientSize(), row, col))
			return false;
			
		wxSize bsz = GetWindowBorderSize();
		int x = col * (m_itemWidth + m_spacing) + m_spacing + bsz.GetWidth() / 2;
		int y = m_row_ys[row] + m_spacing;
		
		if (view_relative)
		{
			int startX, startY;
			int xppu, yppu;
			GetScrollPixelsPerUnit(& xppu, & yppu);
			GetViewStart(& startX, & startY);
			x = x - startX * xppu;
			y = y - startY * yppu;
		}
		
		rect.x = x;
		rect.y = y;
		rect.width = m_itemWidth;
		rect.height = GetItemHeight(n);
		return true;
	}
	
	return false;
}

/// Get the image rect of the given item
bool wxInstanceCtrl::GetItemRectImage(int n, wxRect& rect, bool view_relative)
{
	wxASSERT(n < GetCount());
	
	wxRect outerRect;
	if (!GetItemRect(n, outerRect, view_relative))
		return false;
		
	rect.width = m_ImageSize.x;
	rect.height = m_ImageSize.y;
	rect.x = outerRect.x + (outerRect.width - rect.width) / 2;
	rect.y = outerRect.y;
	
	return true;
}

/// Calculate the outer item size based
/// on font used for text and inner size
void wxInstanceCtrl::CalculateOverallItemSize()
{
	wxCoord w;
	wxClientDC dc(this);
	dc.SetFont(GetFont());
	dc.GetTextExtent(wxT("X"), & w, & m_itemTextHeight);
	
	// FIXME: base padding on font metrics.
	// From left to right: padding, image, padding...
	m_itemWidth = m_ImageSize.x + m_itemMargin * 22;
}

int wxInstanceCtrl::CalculateItemsPerRow()
{
	wxSize clientSize = GetClientSize();
	int perRow = clientSize.x / (m_itemWidth + m_spacing);
	if (perRow < 1)
		perRow = 1;
	return perRow;
}

/// Return the row and column given the client
/// size and a left-to-right, top-to-bottom layout
/// assumption
bool wxInstanceCtrl::GetRowCol(int item, const wxSize& clientSize, int& row, int& col)
{
	wxASSERT(item < GetCount());
	if (item >= GetCount())
		return false;
		
	// How many can we fit in a row?
	
	int perRow = GetItemsPerRow();
	
	row = item / perRow;
	col = item % perRow;
	
	return true;
}


/// Select or deselect an item
void wxInstanceCtrl::Select(int n, bool select)
{
	wxASSERT(n < GetCount());
	
	if (select)
	{
		if (m_selections.Index(n) == wxNOT_FOUND)
			m_selections.Add(n);
	}
	else
	{
		if (m_selections.Index(n) != wxNOT_FOUND)
			m_selections.Remove(n);
	}
	
	m_firstSelection = n;
	m_lastSelection = n;
	int oldFocusItem = m_focusItem;
	m_focusItem = n;
	
	if (m_freezeCount == 0)
	{
		wxRect rect;
		GetItemRect(n, rect);
		RefreshRect(rect);
		
		if (oldFocusItem != -1 && oldFocusItem != n && oldFocusItem < GetCount())
		{
			GetItemRect(oldFocusItem, rect);
			RefreshRect(rect);
		}
	}
}

/// Get the index of the single selection, if not multi-select.
/// Returns -1 if there is no selection.
int wxInstanceCtrl::GetSelection() const
{
	if (m_selections.GetCount() > 0)
		return m_selections[0u];
	else
		return -1;
}

/// Returns true if the item is selected
bool wxInstanceCtrl::IsSelected(int n) const
{
	return (m_selections.Index(n) != wxNOT_FOUND) ;
}

/// Clears all selections
void wxInstanceCtrl::ClearSelections()
{
	int count = GetCount();
	
	m_selections.Clear();
	m_firstSelection = -1;
	m_lastSelection = -1;
	m_focusItem = -1;
	
	if (count > 0 && m_freezeCount == 0)
	{
		Refresh();
	}
}

/// Set the focus item
void wxInstanceCtrl::SetFocusItem(int item)
{
	wxASSERT(item < GetCount());
	if (item < GetCount())
	{
		int oldFocusItem = m_focusItem;
		m_focusItem = item;
		
		if (m_freezeCount == 0)
		{
			wxRect rect;
			if (oldFocusItem != -1)
			{
				GetItemRect(oldFocusItem, rect);
				RefreshRect(rect);
			}
			if (m_focusItem != -1)
			{
				GetItemRect(m_focusItem, rect);
				RefreshRect(rect);
			}
		}
	}
}

/// Painting
void wxInstanceCtrl::OnPaint(wxPaintEvent& WXUNUSED(event))
{
	// Set this to 0 to compare it with the
	// unbuffered implementation
	wxBufferedPaintDC dc(this, m_bufferBitmap);
	
	PrepareDC(dc);
	
	if (m_freezeCount > 0)
		return;
		
	// Paint the background
	PaintBackground(dc);
	
	if (GetCount() == 0)
		return;
		
	wxRegion dirtyRegion = GetUpdateRegion();
	bool isFocused = (FindFocus() == this);
	
	int i;
	int count = GetCount();
	int style = 0;
	wxRect rect, untransformedRect, imageRect, untransformedImageRect;
	for (i = 0; i < count; i++)
	{
		GetItemRect(i, rect);
		
		wxRegionContain c = dirtyRegion.Contains(rect);
		if (c != wxOutRegion)
		{
			GetItemRectImage(i, imageRect);
			style = 0;
			if (IsSelected(i))
				style |= wxINST_SELECTED;
			if (isFocused)
				style |= wxINST_FOCUSED;
			if (isFocused && i == m_focusItem)
				style |= wxINST_IS_FOCUS;
			
			wxInstanceItem* item = GetItem(i);
			if(item)
			{
				GetItemRect(i, untransformedRect, false);
				GetItemRectImage(i, untransformedImageRect, false);
				item->Draw(dc, this, untransformedRect, untransformedImageRect, style);
			}
		}
	}
}

// Empty implementation, to prevent flicker
void wxInstanceCtrl::OnEraseBackground(wxEraseEvent& WXUNUSED(event))
{
}

void wxInstanceCtrl::OnSetFocus(wxFocusEvent& WXUNUSED(event))
{
	if (GetCount() > 0)
		Refresh();
}

void wxInstanceCtrl::OnKillFocus(wxFocusEvent& WXUNUSED(event))
{
	if (GetCount() > 0)
		Refresh();
}

/// Left-click
void wxInstanceCtrl::OnLeftClick(wxMouseEvent& event)
{
	SetFocus();
	int n;
	if (HitTest(event.GetPosition(), n))
	{
		int flags = 0;
		if (event.ControlDown())
			flags |= wxINST_CTRL_DOWN;
		if (event.ShiftDown())
			flags |= wxINST_SHIFT_DOWN;
		if (event.AltDown())
			flags |= wxINST_ALT_DOWN;
			
		EnsureVisible(n);
		DoSelection(n, flags);
		
		wxInstanceCtrlEvent cmdEvent(
		    wxEVT_COMMAND_INST_LEFT_CLICK,
		    GetId());
		cmdEvent.SetEventObject(this);
		cmdEvent.SetIndex(n);
		cmdEvent.SetFlags(flags);
		cmdEvent.SetPosition(event.GetPosition());
		GetEventHandler()->ProcessEvent(cmdEvent);
	}
	else
		ClearSelections();
}

/// Right-click
void wxInstanceCtrl::OnRightClick(wxMouseEvent& event)
{
	SetFocus();
	int n;
	int flags = 0;
	if (event.ControlDown())
		flags |= wxINST_CTRL_DOWN;
	if (event.ShiftDown())
		flags |= wxINST_SHIFT_DOWN;
	if (event.AltDown())
		flags |= wxINST_ALT_DOWN;
	
	if (HitTest(event.GetPosition(), n))
	{
		EnsureVisible(n);
		DoSelection(n, flags);
	}
	else
	{
		ClearSelections();
	}
	
	wxInstanceCtrlEvent cmdEvent(wxEVT_COMMAND_INST_RIGHT_CLICK, GetId());
	cmdEvent.SetEventObject(this);
	cmdEvent.SetIndex(n);
	cmdEvent.SetFlags(flags);
	cmdEvent.SetPosition(event.GetPosition());
	GetEventHandler()->ProcessEvent(cmdEvent);

}

/// Left-double-click
void wxInstanceCtrl::OnLeftDClick(wxMouseEvent& event)
{
	int n;
	if (HitTest(event.GetPosition(), n))
	{
		int flags = 0;
		if (event.ControlDown())
			flags |= wxINST_CTRL_DOWN;
		if (event.ShiftDown())
			flags |= wxINST_SHIFT_DOWN;
		if (event.AltDown())
			flags |= wxINST_ALT_DOWN;
			
		wxInstanceCtrlEvent cmdEvent(
		    wxEVT_COMMAND_INST_LEFT_DCLICK,
		    GetId());
		cmdEvent.SetEventObject(this);
		cmdEvent.SetIndex(n);
		cmdEvent.SetFlags(flags);
		cmdEvent.SetPosition(event.GetPosition());
		GetEventHandler()->ProcessEvent(cmdEvent);
	}
}

/// Middle-click
void wxInstanceCtrl::OnMiddleClick(wxMouseEvent& event)
{
	int n;
	if (HitTest(event.GetPosition(), n))
	{
		int flags = 0;
		if (event.ControlDown())
			flags |= wxINST_CTRL_DOWN;
		if (event.ShiftDown())
			flags |= wxINST_SHIFT_DOWN;
		if (event.AltDown())
			flags |= wxINST_ALT_DOWN;
			
		wxInstanceCtrlEvent cmdEvent(
		    wxEVT_COMMAND_INST_MIDDLE_CLICK,
		    GetId());
		cmdEvent.SetEventObject(this);
		cmdEvent.SetIndex(n);
		cmdEvent.SetFlags(flags);
		cmdEvent.SetPosition(event.GetPosition());
		GetEventHandler()->ProcessEvent(cmdEvent);
	}
}

/// Key press
void wxInstanceCtrl::OnChar(wxKeyEvent& event)
{
	int flags = 0;
	if (event.ControlDown())
		flags |= wxINST_CTRL_DOWN;
	if (event.ShiftDown())
		flags |= wxINST_SHIFT_DOWN;
	if (event.AltDown())
		flags |= wxINST_ALT_DOWN;
		
	if (event.GetKeyCode() == WXK_LEFT ||
	        event.GetKeyCode() == WXK_RIGHT ||
	        event.GetKeyCode() == WXK_UP ||
	        event.GetKeyCode() == WXK_DOWN ||
	        event.GetKeyCode() == WXK_HOME ||
	        event.GetKeyCode() == WXK_PAGEUP ||
	        event.GetKeyCode() == WXK_PAGEDOWN ||
	        event.GetKeyCode() == WXK_END)
	{
		Navigate(event.GetKeyCode(), flags);
	}
	else if (event.GetKeyCode() == WXK_RETURN)
	{
		wxInstanceCtrlEvent cmdEvent(
		    wxEVT_COMMAND_INST_RETURN,
		    GetId());
		cmdEvent.SetEventObject(this);
		cmdEvent.SetFlags(flags);
		GetEventHandler()->ProcessEvent(cmdEvent);
	}
	else if (event.GetKeyCode() == WXK_TAB)
	{
		int focus = m_focusItem;
		if (focus == -1)
			focus = m_lastSelection;
		bool next = !event.ShiftDown();
		
		if (focus <= 0 && !next)
		{
			wxWindow::Navigate(wxNavigationKeyEvent::FromTab | wxNavigationKeyEvent::IsBackward);
		}
		else if (m_items.size() && focus == m_items.size() - 1 && next)
		{
			wxWindow::Navigate(wxNavigationKeyEvent::FromTab | wxNavigationKeyEvent::IsForward);
		}
		else
		{
			Navigate(event.GetKeyCode(), flags);
		}
	}
	else if (event.GetKeyCode() == WXK_DELETE)
	{
		wxInstanceCtrlEvent cmdEvent(
		    wxEVT_COMMAND_INST_DELETE,
		    GetId());
		cmdEvent.SetEventObject(this);
		cmdEvent.SetFlags(flags);
		GetEventHandler()->ProcessEvent(cmdEvent);
	}
	else if (event.GetKeyCode() == WXK_F2)
	{
		wxInstanceCtrlEvent cmdEvent(
		    wxEVT_COMMAND_INST_RENAME,
		    GetId());
		cmdEvent.SetEventObject(this);
		cmdEvent.SetFlags(flags);
		GetEventHandler()->ProcessEvent(cmdEvent);
	}
	else
		event.Skip();
}

/// Keyboard navigation
bool wxInstanceCtrl::Navigate(int keyCode, int flags)
{
	if (GetCount() == 0)
		return false;
		
	wxSize clientSize = GetClientSize();
	int perRow = GetItemsPerRow();
	
	int focus = m_focusItem;
	if (focus == -1)
		focus = m_lastSelection;
		
	if (focus == -1 || focus >= GetCount())
	{
		m_lastSelection = 0;
		DoSelection(m_lastSelection, flags);
		ScrollIntoView(m_lastSelection, keyCode);
		return true;
	}
	
	if (keyCode == WXK_RIGHT)
	{
		int next = focus + 1;
		if (next < GetCount())
		{
			DoSelection(next, flags);
			ScrollIntoView(next, keyCode);
		}
	}
	else if (keyCode == WXK_LEFT)
	{
		int next = focus - 1;
		if (next >= 0)
		{
			DoSelection(next, flags);
			ScrollIntoView(next, keyCode);
		}
	}
	else if (keyCode == WXK_UP)
	{
		int next = focus - perRow;
		if (next >= 0)
		{
			DoSelection(next, flags);
			ScrollIntoView(next, keyCode);
		}
	}
	else if (keyCode == WXK_DOWN)
	{
		int next = focus + perRow;
		if (next < GetCount())
		{
			DoSelection(next, flags);
			ScrollIntoView(next, keyCode);
		}
	}
	// FIXME: this is crap. going one page up or down should be more predictable
	// this solution kinda jumps around too much
	else if (keyCode == WXK_PAGEUP)
	{
		wxRect orig;
		GetItemRect(focus, orig, false);
		int Ynew = orig.y - clientSize.y;
		int row = focus / perRow;
		int next = focus;
		while (next > 0 && (Ynew + m_row_heights[row]) < m_row_ys[row])
		{
			next -= perRow;
			row--;
		}
		if (next < 0)
			next += perRow;
		DoSelection(next, flags);
		ScrollIntoView(next, keyCode);
	}
	else if (keyCode == WXK_PAGEDOWN)
	{
		wxRect orig;
		GetItemRect(focus, orig, false);
		int Ynew = orig.y + clientSize.y;
		int row = focus / perRow;
		int next = focus;
		while (next < m_items.size() && Ynew > m_row_ys[row])
		{
			next += perRow;
			row++;
		}
		if (next >= m_items.size())
			next -= perRow;
		DoSelection(next, flags);
		ScrollIntoView(next, keyCode);
	}
	else if (keyCode == WXK_HOME)
	{
		DoSelection(0, flags);
		ScrollIntoView(0, keyCode);
	}
	else if (keyCode == WXK_END)
	{
		DoSelection(GetCount() - 1, flags);
		ScrollIntoView(GetCount() - 1, keyCode);
	}
	else if (keyCode == WXK_TAB)
	{
		int next = focus;
		if (flags & wxINST_SHIFT_DOWN)
			next --;
		else
			next ++;
			
		if (next >= 0 && next < GetCount())
		{
			DoSelection(next, flags);
			ScrollIntoView(next, keyCode);
		}
	}
	return true;
}

/// Scroll to see the image
void wxInstanceCtrl::ScrollIntoView(int n, int keyCode)
{
	wxRect rect;
	GetItemRect(n, rect, false);    // _Not_ relative to scroll start
	
	int ppuX, ppuY;
	GetScrollPixelsPerUnit(& ppuX, & ppuY);
	
	int startX, startY;
	GetViewStart(& startX, & startY);
	startX = 0;
	startY = startY * ppuY;
	
	int sx, sy;
	GetVirtualSize(& sx, & sy);
	sx = 0;
	if (ppuY != 0)
		sy = sy / ppuY;
		
	wxSize clientSize = GetClientSize();
	
	// Going down
	if (keyCode == WXK_DOWN || keyCode == WXK_RIGHT || keyCode == WXK_END || keyCode == WXK_PAGEDOWN)
	{
		if ((rect.y + rect.height) > (clientSize.y + startY))
		{
			// Make it scroll so this item is at the bottom
			// of the window
			int y = rect.y - (clientSize.y - rect.height - m_spacing) ;
			SetScrollbars(ppuX, ppuY, sx, sy, 0, (int)(0.5 + y / ppuY));
		}
		else if (rect.y < startY)
		{
			// Make it scroll so this item is at the top
			// of the window
			int y = rect.y ;
			SetScrollbars(ppuX, ppuY, sx, sy, 0, (int)(0.5 + y / ppuY));
		}
	}
	// Going up
	else if (keyCode == WXK_UP || keyCode == WXK_LEFT || keyCode == WXK_HOME || keyCode == WXK_PAGEUP)
	{
		if (rect.y < startY)
		{
			// Make it scroll so this item is at the top
			// of the window
			int y = rect.y ;
			SetScrollbars(ppuX, ppuY, sx, sy, 0, (int)(0.5 + y / ppuY));
		}
		else if ((rect.y + rect.height) > (clientSize.y + startY))
		{
			// Make it scroll so this item is at the bottom
			// of the window
			int y = rect.y - (clientSize.y - rect.height - m_spacing) ;
			SetScrollbars(ppuX, ppuY, sx, sy, 0, (int)(0.5 + y / ppuY));
		}
	}
}

/// Scrolls the item into view if necessary
void wxInstanceCtrl::EnsureVisible(int n)
{
	wxRect rect;
	GetItemRect(n, rect, false);    // _Not_ relative to scroll start
	
	int ppuX, ppuY;
	GetScrollPixelsPerUnit(& ppuX, & ppuY);
	
	if (ppuY == 0)
		return;
		
	int startX, startY;
	GetViewStart(& startX, & startY);
	startX = 0;
	startY = startY * ppuY;
	
	int sx, sy;
	GetVirtualSize(& sx, & sy);
	sx = 0;
	if (ppuY != 0)
		sy = sy / ppuY;
		
	wxSize clientSize = GetClientSize();
	
	if ((rect.y + rect.height) > (clientSize.y + startY))
	{
		// Make it scroll so this item is at the bottom
		// of the window
		int y = rect.y - (clientSize.y - rect.height - m_spacing) ;
		SetScrollbars(ppuX, ppuY, sx, sy, 0, (int)(0.5 + y / ppuY));
	}
	else if (rect.y < startY)
	{
		// Make it scroll so this item is at the top
		// of the window
		int y = rect.y ;
		SetScrollbars(ppuX, ppuY, sx, sy, 0, (int)(0.5 + y / ppuY));
	}
}

/// Sizing
void wxInstanceCtrl::OnSize(wxSizeEvent& event)
{
	int old_rows = GetItemsPerRow();
	int new_rows = CalculateItemsPerRow();
	if (old_rows != new_rows)
	{
		SetItemsPerRow(new_rows);
		UpdateRows();
	}
	SetupScrollbars();
	RecreateBuffer();
	event.Skip();
}

/// Set up scrollbars, e.g. after a resize
void wxInstanceCtrl::SetupScrollbars()
{
	if (m_freezeCount)
		return;
		
	if (GetCount() == 0)
	{
		SetScrollbars(0, 0, 0, 0, 0, 0);
		return;
	}
	
	int lastItem = wxMax(0, GetCount() - 1);
	int pixelsPerUnit = 10;
	wxSize clientSize = GetClientSize();
	
	int row, col;
	GetRowCol(lastItem, clientSize, row, col);
	
	//int maxHeight = ( row+1 ) * ( m_itemOverallSize.y + m_spacing ) + m_spacing;
	int lastrow = m_row_ys.size() - 1;
	int maxHeight = m_row_ys[lastrow] + m_row_heights[lastrow] + m_spacing;
	
	int unitsY = maxHeight / pixelsPerUnit;
	
	int startX, startY;
	GetViewStart(& startX, & startY);
	
	int maxtop = maxHeight - clientSize.y;
	if (maxtop % pixelsPerUnit)
	{
		maxtop = ((maxtop / pixelsPerUnit) + 1) * pixelsPerUnit;
	}
	int maxPositionY = (wxMax(maxtop , 0)) / pixelsPerUnit;
	
	// Move to previous scroll position if
	// possible
	SetScrollbars(0, pixelsPerUnit,
	              0, unitsY,
	              0, wxMin(maxPositionY, startY));
}

/// Do (de)selection
void wxInstanceCtrl::DoSelection(int n, int flags)
{
	bool isSelected = IsSelected(n);
	
	wxArrayInt stateChanged;
	{
		size_t i = 0;
		for (i = 0; i < m_selections.GetCount(); i++)
		{
			wxRect rect;
			GetItemRect(m_selections[i], rect);
			RefreshRect(rect);
			
			stateChanged.Add(i);
		}
		
		m_selections.Clear();
		Select(n, true);
		if (stateChanged.Index(n) == wxNOT_FOUND)
			stateChanged.Add(n);
	}
	
	// Now notify the app of any selection changes
	size_t i = 0;
	for (i = 0; i < stateChanged.GetCount(); i++)
	{
		wxInstanceCtrlEvent event(
		    m_selections.Index(stateChanged[i]) != wxNOT_FOUND ? wxEVT_COMMAND_INST_ITEM_SELECTED : wxEVT_COMMAND_INST_ITEM_DESELECTED,
		    GetId());
		event.SetEventObject(this);
		event.SetIndex(stateChanged[i]);
		GetEventHandler()->ProcessEvent(event);
	}
}

/// Find the item under the given point
bool wxInstanceCtrl::HitTest(const wxPoint& pt, int& n)
{
	wxSize clientSize = GetClientSize();
	int startX, startY;
	int ppuX, ppuY;
	n = -1;
	GetViewStart(& startX, & startY);
	GetScrollPixelsPerUnit(& ppuX, & ppuY);
	
	int perRow = GetItemsPerRow();
	
	int colPos = (int)(pt.x / (m_itemWidth + m_spacing));
	int rowPos = 0;
	int actualY = pt.y + startY * ppuY;
	if (0 == m_row_ys.size())
		return false;
	//FIXME: use binary search
	while (rowPos < m_row_ys.size() && m_row_ys[rowPos] < actualY)
		rowPos++;
	rowPos--;
	
	int itemN = (rowPos * perRow + colPos);
	if (itemN >= GetCount())
		return false;
	if (itemN < 0)
		return false;
		
	wxRect rect;
	GetItemRect(itemN, rect);
	if (rect.Contains(pt))
	{
		n = itemN;
		return true;
	}
	return false;
}

/// Paint the background
void wxInstanceCtrl::PaintBackground(wxDC& dc)
{
	wxColour backgroundColour = GetBackgroundColour();
	if (!backgroundColour.Ok())
		backgroundColour = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW);
		
	// Clear the background
	dc.SetBrush(wxBrush(backgroundColour));
	dc.SetPen(*wxTRANSPARENT_PEN);
	wxRect windowRect(wxPoint(0, 0), GetClientSize());
	
	// We need to shift the rectangle to take into account
	// scrolling. Converting device to logical coordinates.
	CalcUnscrolledPosition(windowRect.x, windowRect.y, & windowRect.x, & windowRect.y);
	dc.DrawRectangle(windowRect);
}

/// Recreate buffer bitmap if necessary
bool wxInstanceCtrl::RecreateBuffer(const wxSize& size)
{
	wxSize sz = size;
	if (sz == wxDefaultSize)
		sz = GetClientSize();
		
	if (sz.x < 1 || sz.y < 1)
		return false;
		
	if (!m_bufferBitmap.Ok() || m_bufferBitmap.GetWidth() < sz.x || m_bufferBitmap.GetHeight() < sz.y)
		m_bufferBitmap = wxBitmap(sz.x, sz.y);
	return m_bufferBitmap.Ok();
}

void wxInstanceCtrl::UpdateRows()
{
	wxSize clientSize = GetClientSize();
	int perRow = GetItemsPerRow();
	int numitems = m_items.size();
	int numrows = (numitems / perRow) + (numitems % perRow != 0);
	m_row_ys.clear();
	m_row_ys.resize(numrows);
	m_row_heights.clear();
	m_row_heights.resize(numrows);
	
	int oldrow = 0;
	int row = 0;
	int row_y = m_spacing;
	int rheight = 0;
	for (int n = 0; n < m_items.size(); n++)
	{
		row = n / perRow;
		if (row != oldrow)
		{
			m_row_ys[oldrow] = row_y;
			row_y += rheight + m_spacing;
			m_row_heights[oldrow] = rheight;
			rheight = 0;
			oldrow = row;
		}
		wxInstanceItem& item = m_items[n];
		// icon, margin, margin (highlight), text, margin (end highlight)
		int iheight = m_itemMargin * 3 + m_itemTextHeight * item.GetNumLines() + m_ImageSize.y;
		if (iheight > rheight)
			rheight = iheight;
	}
	if (rheight)
	{
		m_row_heights[row] = rheight;
		m_row_ys[row] = row_y;
	}
}

void wxInstanceCtrl::UpdateItem(int n)
{
	if (n < 0 || n >= GetCount())
		return;
	auto& item = m_items[n];
	item.updateName();
	Refresh();
	UpdateRows();
	SetupScrollbars();
}

/*!
 * wxInstanceItem
 */
void wxInstanceItem::updateName()
{
	wxDC* dc = new wxScreenDC();
	wxString raw_name = wxEmptyString;
	if (m_inst)
		raw_name = m_inst->GetName();

	dc->SetFont(wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT));
	wxArrayInt extents;
	dc->GetPartialTextExtents(raw_name, extents);
	int line = 0;
	int limit = 60 + 32; //FIXME: pass this in from somewhere...
	int accum = 0;
	int linestart = 0;
	int lastspace = -1;
	int lastprocessed = 0;
	text_width = 0;
	text_lines = 0;
	name_wrapped = wxString();
	
	for (int i = 0; i < extents.size(); i++)
	{
		if (raw_name[i] == wxT(' '))
		{
			lastspace = i;
		}
		if ((extents[i] - accum) > limit)
		{
			if (lastspace != -1)
			{
				int size = extents[lastspace - 1] - accum;
				
				name_wrapped.Append(raw_name.SubString(linestart, lastspace - 1).Strip());
				name_wrapped.Append("\n");
				text_lines++;
				
				if (size > text_width)
					text_width = size;
				line++;
				linestart = lastspace + 1;
				
				accum = extents[lastspace];
				lastprocessed = lastspace;
				i = lastspace + 1;
				lastspace = -1;
			}
			else
			{
				int size = extents[i - 1] - accum;
				
				name_wrapped.Append(raw_name.SubString(linestart, i - 1));
				text_lines++;
				if (i + 1 != extents.size())
					name_wrapped.Append(_("\n"));
					
				if (size > text_width)
					text_width = size;
				line++;
				lastspace = -1;
				linestart = i;
				accum = extents[i - 1];
				lastprocessed = i;
			}
		}
	}
	if (lastprocessed != extents.size())
	{
		name_wrapped.Append(raw_name.SubString(linestart, extents.size() - 1));
		text_lines++;
	}
	delete dc;
}

/// Draw the item background
bool wxInstanceItem::Draw(wxDC& dc, wxInstanceCtrl* ctrl, const wxRect& rect, const wxRect& imageRect, int style)
{
	wxColour backgroundColor = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW);
	wxColour textColor = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT);
	wxColour highlightTextColor = wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHTTEXT);
	wxColour focus_color = wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT);
	wxColour focussedSelection = wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT);
	
	if (style & wxINST_SELECTED)
	{
		wxBrush brush(focus_color);
		wxPen pen(focus_color);
		dc.SetBrush(brush);
		dc.SetPen(pen);
	}
	else
	{
		wxBrush brush(backgroundColor);
		wxPen pen(backgroundColor);
		dc.SetBrush(brush);
		dc.SetPen(pen);
	}
	
	// Draw the label
	wxString name = m_inst->GetName();
	if (!name.IsEmpty())
	{
		int margin = ctrl->GetItemMargin();
		
		wxRect textRect;
		textRect.x = rect.x + margin;
		//fRect.y = rect.y + rect.height - ( rect.height - imageRect.height ) /2 + margin;
		textRect.y = rect.y + imageRect.height + 2 * margin;
		textRect.width = rect.width - 2 * margin;
		
		dc.SetFont(ctrl->GetFont());
		if (style & wxINST_SELECTED)
			dc.SetTextForeground(highlightTextColor);
		else
			dc.SetTextForeground(textColor);
		dc.SetBackgroundMode(wxTRANSPARENT);
		
		int yoffset = 0;
		wxSize textsize = dc.GetMultiLineTextExtent(name_wrapped);
		textRect.height = textsize.GetHeight();
		if (style & wxINST_SELECTED)
		{
			wxRect hiRect;
			hiRect.x = rect.x + (rect.width - textsize.x) / 2 - margin;
			hiRect.y = textRect.y - margin;
			hiRect.SetSize(textsize + wxSize(2 * margin, 2 * margin));
			dc.DrawRectangle(hiRect);
		}
		dc.DrawLabel(name_wrapped, textRect, wxALIGN_TOP | wxALIGN_CENTER_HORIZONTAL);
	}
	
	// Draw the icon
	auto list = InstIconList::Instance();
	wxImage icon;
	if (style & wxINST_SELECTED)
		icon = list->getHLImageForKey(m_inst->GetIconKey());
	else
		icon = list->getImageForKey(m_inst->GetIconKey());
	wxBitmap bmap = wxBitmap(icon);
	int x = imageRect.x + (imageRect.width - bmap.GetWidth()) / 2;
	int y = imageRect.y + (imageRect.height - bmap.GetHeight()) / 2;
	dc.DrawBitmap(bmap , x, y, true);
	
	return true;
}

wxSize wxInstanceCtrl::DoGetBestSize() const
{
	wxSize bsz = GetWindowBorderSize();
	int best_width = m_spacing + (m_itemWidth + m_spacing) * GetItemsPerRow() +
	                 bsz.GetWidth() * 2 +
	                 wxSystemSettings::GetMetric(wxSYS_VSCROLL_X);
	wxSize sz(best_width, 0);
	return sz;
}

void wxInstanceCtrl::UpdateItems()
{
	Freeze();

	while (m_items.GetCount() > m_instList->size())
	{
		m_items.RemoveAt(m_items.GetCount() - 1);
	}

	{
		for (int i = 0; i < m_instList->size() ; i++)
		{
			auto inst = m_instList->operator[](i);
			if (i >= m_items.GetCount())
				m_items.Add(wxInstanceItem(inst));
			else
				m_items[i].SetInstance(inst);
		}
	}

	Thaw();
}
