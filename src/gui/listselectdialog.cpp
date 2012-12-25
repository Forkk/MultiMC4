// 
//  Copyright 2012 MultiMC Contributors
// 
//    Licensed under the Apache License, Version 2.0 (the "License");
//    you may not use this file except in compliance with the License.
//    You may obtain a copy of the License at
// 
//        http://www.apache.org/licenses/LICENSE-2.0
// 
//    Unless required by applicable law or agreed to in writing, software
//    distributed under the License is distributed on an "AS IS" BASIS,
//    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//    See the License for the specific language governing permissions and
//    limitations under the License.
//

#include "listselectdialog.h"

#include <wx/gbsizer.h>

#include <boost/foreach.hpp>

#include <string>
#include <sstream>

#include "utils/apputils.h"
#include "utils/httputils.h"
#include "taskprogressdialog.h"
#include "lambdatask.h"

inline void SetControlEnable(wxWindow *parentWin, int id, bool state)
{
	wxWindow *win = parentWin->FindWindowById(id);
	if (win) win->Enable(state);
}

ListSelectDialog::ListSelectDialog(wxWindow *parent, const wxString& title)
	: wxDialog(parent, wxID_ANY, title, wxDefaultPosition, wxSize(400, 420), 
		wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
	SetMinSize(wxSize(400, 420));

	dlgSizer = new wxBoxSizer(wxVERTICAL);
	SetSizer(dlgSizer);

	listCtrl = new ListSelectCtrl(this, this);
	listCtrl->AppendColumn(wxEmptyString, wxLIST_FORMAT_LEFT);
	dlgSizer->Add(listCtrl,1, wxEXPAND | wxALL, 4);

	wxSizerFlags btnSzFlags = wxSizerFlags(0).Border(wxBOTTOM, 4);
	btnSz = new wxBoxSizer(wxHORIZONTAL);
	refreshButton = new wxButton(this, ID_RefreshList, _("&Refresh"));
	btnSz->Add(refreshButton, btnSzFlags.Align(wxALIGN_LEFT));
	btnSz->AddStretchSpacer();
	btnSz->Add(new wxButton(this, wxID_CANCEL));
	btnSz->Add(new wxButton(this, wxID_OK));
	dlgSizer->Add(btnSz, wxSizerFlags(0).Border(wxBOTTOM | wxRIGHT | wxLEFT, 4).
		Align(wxALIGN_BOTTOM).Expand());

	SetControlEnable(this, wxID_OK, false);
}

int ListSelectDialog::ShowModal()
{
	LoadList();
	SetupColumnSizes();
	return wxDialog::ShowModal();
}

void ListSelectDialog::LoadList()
{
	sList.Clear();
	LambdaTask::TaskFunc func = [&] (LambdaTask *task) -> wxThread::ExitCode
	{
		task->DoSetStatus("Loading list...");
		return (wxThread::ExitCode)DoLoadList();
	};

	LambdaTask *lTask = new LambdaTask(func);
	TaskProgressDialog taskDlg(this);
	taskDlg.ShowModal(lTask);
	delete lTask;

	UpdateListCount();
	listCtrl->Refresh();
	listCtrl->Update();

	UpdateOKBtn();
}

void ListSelectDialog::UpdateListCount()
{
	listCtrl->SetItemCount(sList.GetCount());
}


void ListSelectDialog::OnRefreshListClicked(wxCommandEvent& event)
{
	LoadList();
}

void ListSelectDialog::OnListBoxSelChange(wxListEvent& event)
{
	UpdateOKBtn();
}

void ListSelectDialog::UpdateOKBtn()
{
	SetControlEnable(this, wxID_OK, GetSelectedIndex() != -1);
}

wxString ListSelectDialog::GetSelection() const
{
	int index = GetSelectedIndex();
	if (index != -1)
		return listCtrl->GetItemText(index);
	else
		return wxEmptyString;
}

int ListSelectDialog::GetSelectedIndex() const
{
	long item = -1;
	while (true)
	{
		item = listCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

		if (item == -1)
			break;

		return item;
	}
	return -1;
}

wxString ListSelectDialog::OnGetItemText(long item, long column)
{
	return sList[item];
}

void ListSelectDialog::ShowHeader(bool show)
{
	listCtrl->SetSingleStyle(wxLC_NO_HEADER, !show);
}

void ListSelectDialog::SetupColumnSizes()
{
	wxSize listSize = listCtrl->GetClientSize();

	int otherColsWidth = 0;
	for (int i = 1; i < listCtrl->GetColumnCount(); i++)
	{
		otherColsWidth += listCtrl->GetColumnWidth(i);
	}

	if (otherColsWidth >= listSize.GetWidth())
		listCtrl->SetColumnWidth(0, wxLIST_AUTOSIZE_USEHEADER);
	else
		listCtrl->SetColumnWidth(0, listSize.GetWidth() - otherColsWidth);
}

BEGIN_EVENT_TABLE(ListSelectDialog, wxDialog)
	EVT_BUTTON(ID_RefreshList, ListSelectDialog::OnRefreshListClicked)
	EVT_LIST_ITEM_SELECTED(-1, ListSelectDialog::OnListBoxSelChange)
END_EVENT_TABLE()


////////////// LIST CONTROL ////////////////

ListSelectDialog::ListSelectCtrl::ListSelectCtrl(wxWindow* parent, ListSelectDialog* owner)
	: wxListCtrl(parent, -1, wxDefaultPosition, wxDefaultSize, 
		wxLC_REPORT | wxLC_VIRTUAL | wxLC_SINGLE_SEL | wxLC_NO_HEADER)
{
	m_owner = owner;
}

wxString ListSelectDialog::ListSelectCtrl::OnGetItemText(long item, long column) const
{
	return m_owner->OnGetItemText(item, column);
}

void ListSelectDialog::ListSelectCtrl::OnResize(wxSizeEvent& event)
{
	m_owner->SetupColumnSizes();
	event.Skip();
}

BEGIN_EVENT_TABLE(ListSelectDialog::ListSelectCtrl, wxListCtrl)
	EVT_SIZE(ListSelectDialog::ListSelectCtrl::OnResize)
END_EVENT_TABLE()
