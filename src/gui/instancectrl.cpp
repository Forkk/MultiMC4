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
#include "wx/sstream.h"

#include "instancemodel.h"

WX_DEFINE_OBJARRAY(InstanceItemArray);
WX_DEFINE_OBJARRAY(InstanceGroupArray);

DEFINE_EVENT_TYPE(wxEVT_COMMAND_INST_ITEM_SELECTED)
DEFINE_EVENT_TYPE(wxEVT_COMMAND_INST_ITEM_DESELECTED)
DEFINE_EVENT_TYPE(wxEVT_COMMAND_INST_ACTIVATE)
DEFINE_EVENT_TYPE(wxEVT_COMMAND_INST_MENU)
DEFINE_EVENT_TYPE(wxEVT_COMMAND_INST_DELETE)
DEFINE_EVENT_TYPE(wxEVT_COMMAND_INST_RENAME)
DEFINE_EVENT_TYPE(wxEVT_COMMAND_INST_DRAG)

IMPLEMENT_CLASS(InstanceCtrl, wxScrolledCanvas)
IMPLEMENT_CLASS(InstanceCtrlEvent, wxNotifyEvent)

BEGIN_EVENT_TABLE(InstanceCtrl, wxScrolledCanvas)
	EVT_PAINT(InstanceCtrl::OnPaint)
	EVT_ERASE_BACKGROUND(InstanceCtrl::OnEraseBackground)
	EVT_LEFT_DOWN(InstanceCtrl::OnLeftClick)
	EVT_RIGHT_DOWN(InstanceCtrl::OnRightClick)
	EVT_MIDDLE_DOWN(InstanceCtrl::OnMiddleClick)
	EVT_LEFT_DCLICK(InstanceCtrl::OnLeftDClick)
	EVT_MOTION(InstanceCtrl::OnMouseMotion)
	EVT_CHAR(InstanceCtrl::OnChar)
	EVT_SIZE(InstanceCtrl::OnSize)
	EVT_SET_FOCUS(InstanceCtrl::OnSetFocus)
	EVT_KILL_FOCUS(InstanceCtrl::OnKillFocus)

	EVT_INST_DRAG(wxID_ANY, InstanceCtrl::OnInstDragged)
END_EVENT_TABLE()

/*!
 * InstanceCtrl
 */
InstanceCtrl::InstanceCtrl()
{
	Init();
}

InstanceCtrl::InstanceCtrl(wxWindow* parent, InstanceModel *instList, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
{
	Init();
	Create(parent, instList, id, pos, size, style);
}

/// Creation
bool InstanceCtrl::Create(wxWindow* parent, InstanceModel *instList, wxWindowID id, const wxPoint& pos, const wxSize& size, long style)
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
void InstanceCtrl::Init()
{
	m_itemWidth = -1;
	m_ImageSize = wxINST_DEFAULT_IMAGE_SIZE;
	m_freezeCount = 0;
	m_spacing = wxINST_DEFAULT_SPACING;
	m_itemMargin = wxINST_DEFAULT_MARGIN;
	m_selectedItem = -1;
	m_focusItem = -1;
}

/// Call Freeze to prevent refresh
void InstanceCtrl::Freeze()
{
	m_freezeCount ++;
}

/// Call Thaw to refresh
void InstanceCtrl::Thaw()
{
	m_freezeCount --;
	
	if (m_freezeCount == 0)
	{
		ReflowAll();
		SetupScrollbars();
		Refresh();
	}
}

/// Get the visual item by coord
InstanceVisual* InstanceCtrl::GetItem( VisualCoord n ) const
{
	if(!n.isItem())
		return nullptr;
	if (n.groupIndex < GetCount())
	{
		GroupVisual & gv = m_groups[n.groupIndex];
		if(n.itemIndex < gv.items.size())
			return & gv.items[n.itemIndex];
	}
	
	return nullptr;
}

/// Get the visual group by coord
GroupVisual* InstanceCtrl::GetGroup(VisualCoord n) const
{
	if(!n.isGroup())
		return nullptr;

	if (n.groupIndex < GetCount())
	{
		return &m_groups[n.groupIndex];
	}

	return nullptr;
}

/// Get the overall rect of the given item
bool InstanceCtrl::GetItemRect( VisualCoord item, wxRect& rect, bool view_relative )
{
	if (item.groupIndex < GetCount())
	{
		int row, col;
		if (!GetRowCol(item, row, col))
			return false;
		GroupVisual & gv = m_groups[item.groupIndex];
		
		wxSize bsz = GetWindowBorderSize();
		int x = col * (m_itemWidth + m_spacing) + m_spacing + bsz.GetWidth() / 2;
		int y = gv.y_position + gv.row_ys[row] + m_spacing;
		
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
		rect.height = GetItemHeight(item);
		return true;
	}
	return false;
}

bool InstanceCtrl::GetGroupRect ( int group, wxRect& rect, bool view_relative )
{
	if (group < GetCount() && group >= 0)
	{
		int w,h;
		GetClientSize(&w, &h);
		GroupVisual & gv = m_groups[group];
		rect.x = 0;
		rect.y = gv.y_position;
		rect.width = w; // always fills entire width
		rect.height = gv.total_height;

		if (view_relative)
		{
			int startX, startY;
			int xppu, yppu;
			GetScrollPixelsPerUnit(& xppu, & yppu);
			GetViewStart(& startX, & startY);
			rect.x = rect.x - startX * xppu;
			rect.y = rect.y - startY * yppu;
		}
		return true;
	}
}


/// Calculate the outer item size based
/// on font used for text and inner size
void InstanceCtrl::CalculateOverallItemSize()
{
	wxCoord w;
	wxClientDC dc(this);
	dc.SetFont(GetFont());
	dc.GetTextExtent(wxT("X"), & w, & m_itemTextHeight);
	
	// FIXME: base padding on font metrics.
	// From left to right: padding, image, padding...
	m_itemWidth = m_ImageSize.x + m_itemMargin * 22;
}

int InstanceCtrl::CalculateItemsPerRow()
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
bool InstanceCtrl::GetRowCol( VisualCoord item, int& row, int& col )
{
	if(!GetItem(item))
		return false;
	int perRow = GetItemsPerRow();
	row = item.itemIndex / perRow;
	col = item.itemIndex % perRow;
	
	return true;
}


/// Select or deselect an item
void InstanceCtrl::Select( VisualCoord n, bool select )
{
	VisualCoord oldFocusItem = m_focusItem;
	m_focusItem = n;
	m_selectedItem = n;
	
	if (m_freezeCount == 0)
	{
		wxRect rect;
		GetItemRect(n, rect);
		RefreshRect(rect);
		
		if (!oldFocusItem.isVoid() && oldFocusItem != n)
		{
			GetItemRect(oldFocusItem, rect);
			RefreshRect(rect);
		}
	}
}

/// Do (de)selection
void InstanceCtrl::DoSelection(VisualCoord n)
{
	if(n == m_selectedItem)
		return;
	
	VisualCoord oldSelected = m_selectedItem;
	
	// refresh the old selection
	if(!oldSelected.isVoid())
	{
		wxRect rect;
		GetItemRect(oldSelected, rect);
		RefreshRect(rect);
	}
	
	m_selectedItem.makeVoid();
	Select(n, true);
	
	// Now notify the app of any selection changes
	if(!oldSelected.isVoid())
	{
		InstanceCtrlEvent eventDeselect(wxEVT_COMMAND_INST_ITEM_DESELECTED,GetId());
		eventDeselect.SetEventObject(this);
		int clickedID = IDFromIndex(oldSelected);
		m_instList->CtrlSelectInstance( clickedID );
		eventDeselect.SetItemID(clickedID);
		eventDeselect.SetItemIndex(oldSelected);
		GetEventHandler()->ProcessEvent(eventDeselect);
	}
	
	InstanceCtrlEvent eventSelect(wxEVT_COMMAND_INST_ITEM_SELECTED,GetId());
	eventSelect.SetEventObject(this);
	int clickedID = IDFromIndex(m_selectedItem);
	m_instList->CtrlSelectInstance( clickedID );
	eventSelect.SetItemID(clickedID);
	eventSelect.SetItemIndex(m_selectedItem);
	GetEventHandler()->ProcessEvent(eventSelect);
}

/// Returns -1 if there is no selection.
VisualCoord InstanceCtrl::GetSelection() const
{
	return m_selectedItem;
}

/// Returns true if the item is selected
bool InstanceCtrl::IsSelected(VisualCoord n) const
{
	return m_selectedItem == n;
}

/// Clears all selections
void InstanceCtrl::ClearSelections()
{
	m_selectedItem.makeVoid();
	m_focusItem.makeVoid();
	
	if (m_freezeCount == 0)
	{
		Refresh();
	}
}

int InstanceCtrl::GetSuggestedPostRemoveID ( int removedID )
{
	/*
	if(m_items.size() == 1)
		return -1;
	int deletedIndex = m_itemIndexes[removedID];
	// deleting last item, go back one
	if(deletedIndex == m_items.size() - 1)
	{
		deletedIndex --;
	}
	else
	{
		// otherwise go forward
		deletedIndex++;
	}
	if(deletedIndex < 0)
		deletedIndex = 0;
	// translate back to ID
	return m_items[deletedIndex].GetID();
	*/
	return -1;
}

/// Painting
void InstanceCtrl::OnPaint(wxPaintEvent& WXUNUSED(event))
{
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
	wxRect rect, untransformedRect;
	for (i = 0; i < count; i++)
	{
		GetGroupRect(i, rect);
		
		wxRegionContain c = dirtyRegion.Contains(rect);
		if (c != wxOutRegion)
		{
			style = 0;
			GetGroupRect(i, untransformedRect, false);
			GroupVisual & gv = m_groups[i];
			gv.Draw(dc, this, untransformedRect,
			        m_selectedItem.groupIndex == i, m_selectedItem.itemIndex,
			        m_focusItem.groupIndex == i, m_focusItem.itemIndex, 
					i == highlightedGroup.groupIndex);
		}
	}
}

void GroupVisual::Draw ( wxDC& dc, InstanceCtrl* parent, wxRect limitingRect, bool hasSelection, int selectionIndex, bool hasFocus, int focusIndex, bool highlight )
{
	int i;
	int count = items.size();
	int style = 0;
	wxRect rect;
	
	// Draw the header
	if(!no_header)
	{
		wxColour textColor = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT);

		if (highlight)
		{
			textColor = wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT);
		}

		wxBrush brush(textColor);
		wxPen pen(textColor);
		dc.SetBrush(brush);
		dc.SetPen(pen);
		wxSize sz = dc.GetTextExtent(GetName());
		dc.SetTextForeground(textColor);
		
		dc.DrawText( GetName() , 20, y_position + 5 );
		int atheight = y_position + header_height / 2;
		if(sz.x + 30 < limitingRect.width - 10)
			dc.DrawLine(sz.x + 30,atheight, limitingRect.width - 10, atheight);
		
		dc.SetBrush(*wxTRANSPARENT_BRUSH);
		dc.SetPen(textColor);
		
		// Ungrouped can't be hidden, so don't draw the box.
		if (m_group)
		{
			dc.DrawRectangle(5,atheight -5, 10,10);
			dc.DrawRectangle(7,atheight -1, 6,2);
			if(!IsExpanded())
			{
				dc.DrawRectangle(9,atheight -3, 2,6);
			}
		}
	}
	
	if(IsExpanded()) for (i = 0; i < count; i++)
	{
		parent->GetItemRect(VisualCoord(index,i), rect, false);

		if (!limitingRect.Intersects(rect))
			continue;
		style = 0;
		if (hasSelection && selectionIndex == i)
			style |= wxINST_SELECTED;
		if (hasFocus && i == focusIndex)
			style |= wxINST_IS_FOCUS;

		InstanceVisual& item = items[i];
		item.Draw(dc, parent, rect, style);
	}
}


void InstanceCtrl::OnSetFocus(wxFocusEvent& WXUNUSED(event))
{
	if (GetCount() > 0)
		Refresh();
}

void InstanceCtrl::OnKillFocus(wxFocusEvent& WXUNUSED(event))
{
	if (GetCount() > 0)
		Refresh();
}

/// Left-click
void InstanceCtrl::OnLeftClick(wxMouseEvent& event)
{
	SetFocus();
	VisualCoord clickedIndex;
	HitTest(event.GetPosition(), clickedIndex);
	if(clickedIndex.isItem())
	{
		int flags = 0;
		if (event.ControlDown())
			flags |= wxINST_CTRL_DOWN;
		if (event.ShiftDown())
			flags |= wxINST_SHIFT_DOWN;
		if (event.AltDown())
			flags |= wxINST_ALT_DOWN;
			
		//EnsureVisible(clickedIndex);
		DoSelection(clickedIndex);
	}
	else if(clickedIndex.isHeaderTicker())
	{
		ToggleGroup(clickedIndex.groupIndex);
	}
	else ClearSelections();
}

/// Right-click
void InstanceCtrl::OnRightClick(wxMouseEvent& event)
{
	SetFocus();
	VisualCoord clickedIndex;
	int flags = 0;
	if (event.ControlDown())
		flags |= wxINST_CTRL_DOWN;
	if (event.ShiftDown())
		flags |= wxINST_SHIFT_DOWN;
	if (event.AltDown())
		flags |= wxINST_ALT_DOWN;
	HitTest(event.GetPosition(), clickedIndex);
	if (clickedIndex.isItem())
	{
		//EnsureVisible(clickedIndex);
		DoSelection(clickedIndex);
	}
	else
	{
		ClearSelections();
	}
	
	InstanceCtrlEvent cmdEvent(wxEVT_COMMAND_INST_MENU, GetId());
	cmdEvent.SetEventObject(this);
	cmdEvent.SetItemIndex(clickedIndex);
	int clickedID = IDFromIndex(clickedIndex);
	cmdEvent.SetItemID(clickedID);
	cmdEvent.SetFlags(flags);
	cmdEvent.SetPosition(event.GetPosition());
	if (clickedIndex.isGroup())
	{
		cmdEvent.SetGroup(m_groups[clickedIndex.groupIndex].m_group);
	}
	GetEventHandler()->ProcessEvent(cmdEvent);
}

/// Left-double-click
void InstanceCtrl::OnLeftDClick(wxMouseEvent& event)
{
	VisualCoord clickedIndex;
	HitTest(event.GetPosition(), clickedIndex);
	if (clickedIndex.isItem())
	{
		int flags = 0;
		if (event.ControlDown())
			flags |= wxINST_CTRL_DOWN;
		if (event.ShiftDown())
			flags |= wxINST_SHIFT_DOWN;
		if (event.AltDown())
			flags |= wxINST_ALT_DOWN;
			
		InstanceCtrlEvent cmdEvent(
		    wxEVT_COMMAND_INST_ACTIVATE,
		    GetId());
		cmdEvent.SetEventObject(this);
		cmdEvent.SetItemIndex(clickedIndex);
		int clickedID = IDFromIndex(clickedIndex);
		cmdEvent.SetItemID(clickedID);
		cmdEvent.SetFlags(flags);
		cmdEvent.SetPosition(event.GetPosition());
		GetEventHandler()->ProcessEvent(cmdEvent);
	}
	else if(clickedIndex.isHeader())
	{
		ToggleGroup(clickedIndex.groupIndex);
	}
}

void InstanceCtrl::OnMouseMotion(wxMouseEvent& event)
{
	// Only start DnD if the mouse is over the selected instance.
	VisualCoord coord;
	HitTest(event.GetPosition(), coord);
	InstanceVisual *instVisual = GetItem(coord);

	if (event.Dragging() && instVisual && m_instList->GetSelectedInstance() == instVisual->GetInstance())
	{
		int flags = 0;
		if (event.ControlDown())
			flags |= wxINST_CTRL_DOWN;
		if (event.ShiftDown())
			flags |= wxINST_SHIFT_DOWN;
		if (event.AltDown())
			flags |= wxINST_ALT_DOWN;

		InstanceCtrlEvent cmdEvent(
			wxEVT_COMMAND_INST_DRAG,
			GetId());
		cmdEvent.SetEventObject(this);
		cmdEvent.SetFlags(flags);
		GetEventHandler()->ProcessEvent(cmdEvent);
	}
	else
	{
		event.Skip();
	}
}

void InstanceCtrl::InstCtrlDropTarget::OnLeave()
{
	m_parent->HighlightGroup(VisualCoord());
}

bool InstanceCtrl::InstCtrlDropTarget::OnDropText(wxCoord x, wxCoord y, const wxString& data)
{
	// Unhighlight group.
	m_parent->HighlightGroup(VisualCoord());

	VisualCoord coord;
	m_parent->HitTest(wxPoint(x, y), coord);

	// Make sure we target a group, always.
	if (coord.itemIndex >= 0)
		coord.itemIndex = -1;
	GroupVisual *gv = m_parent->GetGroup(coord);
	if (gv)
	{
		for (int i = 0; i < m_parent->m_instList->size(); i++)
		{
			Instance * inst = m_parent->m_instList->at(i);
			if (inst->GetInstID() == data)
			{
				if(gv->isUngrouped() && inst->GetGroup() == "" && coord.isHeader() || coord.isHeaderTicker())
				{
					inst->SetGroup("New Group");
				}
				else if(gv->isUngrouped())
				{
					inst->SetGroup(wxEmptyString);
				}
				else
				{
					inst->SetGroup(gv->GetName());
				}
				m_parent->ReloadAll();

				return true;
			}
		}
	}

	return false;
}

wxDragResult InstanceCtrl::InstCtrlDropTarget::OnDragOver(wxCoord x, wxCoord y, wxDragResult def)
{
	VisualCoord coord;
	m_parent->HitTest(wxPoint(x, y), coord);

	// Make sure we target a group, always.
	if (coord.itemIndex >= 0)
		coord.itemIndex = -1;
	GroupVisual *gv = m_parent->GetGroup(coord);
	if (gv)
	{
		// Give DnD feedback
		m_parent->HighlightGroup(coord);

		return wxDragResult::wxDragMove;
	}
	else
	{
		m_parent->HighlightGroup(VisualCoord());
	}

	return wxDragResult::wxDragNone;
}

/// Key press
void InstanceCtrl::OnChar(wxKeyEvent& event)
{
	int flags = 0;
	if (event.ControlDown())
		flags |= wxINST_CTRL_DOWN;
	if (event.ShiftDown())
		flags |= wxINST_SHIFT_DOWN;
	if (event.AltDown())
		flags |= wxINST_ALT_DOWN;

	if (event.GetKeyCode() == WXK_RETURN)
	{
		InstanceCtrlEvent cmdEvent(
		    wxEVT_COMMAND_INST_ACTIVATE,
		    GetId());
		cmdEvent.SetEventObject(this);
		cmdEvent.SetFlags(flags);
		GetEventHandler()->ProcessEvent(cmdEvent);
	}
	else if (event.GetKeyCode() == WXK_DELETE)
	{
		InstanceCtrlEvent cmdEvent(
		    wxEVT_COMMAND_INST_DELETE,
		    GetId());
		cmdEvent.SetEventObject(this);
		cmdEvent.SetFlags(flags);
		GetEventHandler()->ProcessEvent(cmdEvent);
	}
	else if (event.GetKeyCode() == WXK_F2)
	{
		InstanceCtrlEvent cmdEvent(
		    wxEVT_COMMAND_INST_RENAME,
		    GetId());
		cmdEvent.SetEventObject(this);
		cmdEvent.SetFlags(flags);
		GetEventHandler()->ProcessEvent(cmdEvent);
	}
	else
		event.Skip();
}

/// Sizing
void InstanceCtrl::OnSize(wxSizeEvent& event)
{
	int old_rows = GetItemsPerRow();
	int new_rows = CalculateItemsPerRow();
	if (old_rows != new_rows)
	{
		SetItemsPerRow(new_rows);
		ReflowAll();
	}
	SetupScrollbars();
	RecreateBuffer();
	event.Skip();
}

/// Set up scrollbars, e.g. after a resize
void InstanceCtrl::SetupScrollbars()
{
	if (m_freezeCount)
		return;
	
	if (GetCount() == 0)
	{
		SetScrollbars(0, 0, 0, 0, 0, 0);
		return;
	}
	
	int pixelsPerUnit = 10;
	wxSize clientSize = GetClientSize();
	
	int maxHeight = GetTotalHeight();
	
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

int InstanceCtrl::IDFromIndex ( VisualCoord index ) const
{
	if (!index.isItem())
		return -1;
	
	auto & grp = m_groups[index.groupIndex];
	auto & item = grp.items[index.itemIndex];
	return item.GetID();
}
VisualCoord InstanceCtrl::IndexFromID ( int ID ) const
{
	if(ID == -1)
		return VisualCoord();
	return m_itemIndexes[ID];
}

void InstanceCtrl::ToggleGroup ( int index )
{
	GroupVisual & gv = m_groups[index];
	gv.SetExpanded(!gv.IsExpanded());
	ReflowAll();
	SetupScrollbars();
	Refresh();
}

/// Find the item under the given point
void InstanceCtrl::HitTest( const wxPoint& pt, VisualCoord& n )
{
	wxSize clientSize = GetClientSize();
	int startX, startY;
	int ppuX, ppuY;
	n.makeVoid();
	
	GetViewStart(& startX, & startY);
	GetScrollPixelsPerUnit(& ppuX, & ppuY);
	
	int perRow = GetItemsPerRow();
	
	int colPos = (int)(pt.x / (m_itemWidth + m_spacing));
	int rowPos = 0;
	int actualY = pt.y + startY * ppuY;
	
	GroupVisual * found = nullptr;
	int grpIdx = 0;
	for(; grpIdx < m_groups.size(); grpIdx++)
	{
		GroupVisual & gv = m_groups[grpIdx];
		if (actualY >= gv.y_position && actualY <= gv.y_position + gv.total_height)
		{
			found = &gv;
			n.makeGroup(grpIdx);
			if (!gv.no_header && actualY <= gv.y_position + gv.header_height)
			{
				// it's a header
				if(pt.x >= 5 && pt.x <= 15)
				{
					// it's the ticker thing
					n.makeHeaderTicker(grpIdx);
				}
				else
				{
					// it's the header in general
					n.makeHeader(grpIdx);
				}
				return;
			}
			break;
		}
	}
	// it's not even a group
	if(!found)
		return;
	
	while (rowPos < found->row_ys.size() && found->y_position + found->row_ys[rowPos] < actualY)
		rowPos++;
	rowPos--;
	
	int itemN = (rowPos * perRow + colPos);
	if (itemN >= found->items.size() || itemN < 0)
		return;
		
	wxRect rect;
	VisualCoord coord(grpIdx, itemN);
	GetItemRect(coord, rect);
	if (rect.Contains(pt))
	{
		n = coord;
	}
}

/// Paint the background
void InstanceCtrl::PaintBackground(wxDC& dc)
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
bool InstanceCtrl::RecreateBuffer(const wxSize& size)
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

void GroupVisual::Reflow ( int perRow, int spacing, int margin, int lineHeight, int imageSize, int & progressive_y )
{
	y_position = progressive_y;
	int numitems = items.size();
	int numrows = (numitems / perRow) + (numitems % perRow != 0);
	row_ys.clear();
	row_ys.resize(numrows);
	row_heights.clear();
	row_heights.resize(numrows);
	header_height = lineHeight + 10;
	
	int oldrow = 0;
	int row = 0;
	int row_y = spacing;
	if(!no_header)
	{
		row_y += header_height;
	}
	if(IsExpanded())
	{
		int rheight = 0;
		for (int n = 0; n < numitems; n++)
		{
			row = n / perRow;
			if (row != oldrow)
			{
				row_ys[oldrow] = row_y;
				row_y += rheight + spacing;
				row_heights[oldrow] = rheight;
				rheight = 0;
				oldrow = row;
			}
			InstanceVisual& item = items[n];
			// icon, margin, margin (highlight), text, margin (end highlight)
			int iheight = margin * 3 + lineHeight * item.GetNumLines() + imageSize;
			if (iheight > rheight)
				rheight = iheight;
		}
		if (rheight)
		{
			row_heights[row] = rheight;
			row_ys[row] = row_y;
		}
		total_height = row_ys[numrows - 1] + row_heights[numrows - 1];
	}
	else
	{
		total_height = header_height;
	}
	progressive_y += total_height;
}


void InstanceCtrl::ReflowAll()
{
	int progressive_y = 0;
	for(int i = 0; i < m_groups.size(); i++)
	{
		GroupVisual & gv = m_groups[i];
		gv.Reflow(m_itemsPerRow,m_spacing,m_itemMargin,m_itemTextHeight,m_ImageSize.GetHeight(), progressive_y);
	}
}

/*!
 * wxInstanceItem
 */
void InstanceVisual::updateName()
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

/// Draw the item
bool InstanceVisual::Draw(wxDC& dc, InstanceCtrl* ctrl, const wxRect& rect, int style)
{
	wxColour backgroundColor = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOW);
	wxColour textColor = wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT);
	wxColour highlightTextColor = wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHTTEXT);
	wxColour focus_color = wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT);
	wxColour focussedSelection = wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT);
	
	wxRect imageRect(rect.x + (rect.width - 32) / 2, rect.y, 32, 32);
	
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

wxSize InstanceCtrl::DoGetBestSize() const
{
	wxSize bsz = GetWindowBorderSize();
	int best_width = m_spacing + (m_itemWidth + m_spacing) * GetItemsPerRow() +
	                 bsz.GetWidth() * 2 +
	                 wxSystemSettings::GetMetric(wxSYS_VSCROLL_X);
	wxSize sz(best_width, 0);
	return sz;
}

int NameSort(InstanceVisual **first, InstanceVisual **second)
{
	return (*first)->GetName().CmpNoCase((*second)->GetName());
};

int NameSort(GroupVisual **first, GroupVisual **second)
{
	if (!(*first)->m_group)
		return 1;

	if (!(*second)->m_group)
		return -1;

	return (*first)->GetName().CmpNoCase((*second)->GetName());
};

//FIXME: doing this for every single change seems like a waste :/
// Also, pure fail :D
void InstanceCtrl::ReloadAll()
{
	// nuke all... lol
	m_groups.Clear();
	
	int totalitems = m_instList->size();
	
	/// sort items into groups
	std::map<InstanceGroup *, InstanceItemArray> sorter;
	for (int i = 0; i < totalitems ; i++)
	{
		auto inst = m_instList->operator[](i);
		InstanceGroup *group =  m_instList->GetInstanceGroup(inst);

		if (sorter.count(group))
		{
			auto & list = sorter[group];
			list.Add(InstanceVisual(inst, i));
		}
		else
		{
			InstanceItemArray arr;
			arr.Add(InstanceVisual(inst, i));
			sorter[group] = arr;
		}
	}
	
	m_itemIndexes.resize(totalitems);

	// sort items in each group
	auto iter = sorter.begin();
	while (iter != sorter.end())
	{
		InstanceGroup* group = (*iter).first;
		
		GroupVisual grpv(group);
		grpv.items = (*iter).second;
		grpv.items.Sort(NameSort);
		m_groups.Add(grpv);
		iter++;
	}
	
	// sort the groups and construct a mapping from IDs to indexes
	m_groups.Sort(NameSort);
	for(int i = 0; i < m_groups.size(); i++)
	{
		GroupVisual & grp = m_groups[i];
		grp.SetIndex(i);
		for(int j = 0; j < grp.items.size(); j++)
		{
			InstanceVisual & iv = grp.items[j];
			int ID = iv.GetID();
			m_itemIndexes[ID] = VisualCoord(i,j);
		}
	}
	
	int selectedIdx = m_instList->GetSelectedIndex();
	if(selectedIdx == -1)
	{
		if(m_focusItem == m_selectedItem)
			m_focusItem.makeVoid();
		m_selectedItem.makeVoid();
	}
	else
	{
		VisualCoord selectedIndex = m_itemIndexes[selectedIdx];
		if(m_focusItem == m_selectedItem)
			m_focusItem = selectedIndex;
		m_selectedItem = selectedIndex;
	}
	
	// Disable group headers in single column mode. Groups are still there, but their existence is hidden.
	if (GetWindowStyle() & wxINST_SINGLE_COLUMN)
	{
		for (int i = 0; i < m_groups.size(); i++)
		{
			m_groups[i].no_header = true;
			m_groups[i].always_show = true;
		}
	}
	
	// everything got changed (probably not, but we can't know that). Redo all of the layout stuff.
	ReflowAll();
	SetupScrollbars();
	Refresh();
}

void InstanceCtrl::HighlightGroup(const VisualCoord& coord)
{
	VisualCoord prevHighlight = highlightedGroup;

	if (coord.isGroup())
	{
		highlightedGroup = coord;

		wxRect gRect;
		GetGroupRect(coord.groupIndex, gRect);
		RefreshRect(gRect);
	}
	else
	{
		highlightedGroup.makeVoid();
	}

	if (prevHighlight.isGroup())
	{
		wxRect gRect;
		GetGroupRect(prevHighlight.groupIndex, gRect);
		RefreshRect(gRect);
	}
}

void InstanceCtrl::OnInstDragged(InstanceCtrlEvent& event)
{
	// No DnD in single column mode.
	if (GetWindowStyle() & wxINST_SINGLE_COLUMN)
		return;

	Instance *selectedInst = m_instList->GetSelectedInstance();
	if (!selectedInst)
		return;

	wxTextDataObject instDataObj(selectedInst->GetInstID());

	wxDropSource dragSource(instDataObj, this);
	dragSource.DoDragDrop(wxDrag_AllowMove);
	
	// Make sure we reset the group highlighting when the DnD operation is done.
	HighlightGroup(VisualCoord());
}


GroupVisual::GroupVisual(InstanceGroup *group, bool no_header)
	: no_header(no_header)
{
	m_group = group;
	total_height = 0;
	header_height = 0;
	y_position = 0;
	index = -1;
	always_show = false;
}

wxString GroupVisual::GetName() const
{
	if (m_group)
		return m_group->GetName();
	else
		return "Ungrouped";
}

bool GroupVisual::isUngrouped() const
{
	return (m_group == nullptr);
}


void GroupVisual::SetExpanded(bool expanded)
{
	if (m_group)
		m_group->SetHidden(!expanded);
}

bool GroupVisual::IsExpanded() const
{
	return !m_group || !m_group->IsHidden() || always_show;
}
