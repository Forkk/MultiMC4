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

#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>

#include <string>
#include <sstream>

#include "apputils.h"
#include "httputils.h"
#include "taskprogressdialog.h"
#include "lambdatask.h"

enum
{
	ID_RefreshList,
};

inline void SetControlEnable(wxWindow *parentWin, int id, bool state)
{
	wxWindow *win = parentWin->FindWindowById(id);
	if (win) win->Enable(state);
}

ListSelectDialog::ListSelectDialog(wxWindow *parent, const wxString& title)
	: wxDialog(parent, wxID_ANY, title, wxDefaultPosition, wxSize(400, 420))
{
	dlgSizer = new wxBoxSizer(wxVERTICAL);
	SetSizer(dlgSizer);

	mainPanel = new wxPanel(this);
	dlgSizer->Add(mainPanel, wxSizerFlags(1).Expand().Border(wxALL, 8));
	mainSz = new wxGridBagSizer();
	mainSz->AddGrowableCol(0, 0);
	mainSz->AddGrowableRow(0, 0);
	mainPanel->SetSizer(mainSz);

	list = new wxListBox(mainPanel, -1, wxDefaultPosition, wxDefaultSize,
		wxArrayString(), wxLB_SINGLE);
	mainSz->Add(list, wxGBPosition(0, 0), wxGBSpan(1, 2), wxEXPAND | wxALL, 4);

	refreshButton = new wxButton(mainPanel, ID_RefreshList, _("&Refresh"));
	mainSz->Add(refreshButton, wxGBPosition(1, 1), wxGBSpan(1, 1), wxALL, 3);

	btnSz = CreateButtonSizer(wxOK | wxCANCEL);
	dlgSizer->Add(btnSz, wxSizerFlags(0).Border(wxBOTTOM | wxRIGHT, 8).
		Align(wxALIGN_RIGHT | wxALIGN_BOTTOM));

	SetControlEnable(this, wxID_OK, false);
}

int ListSelectDialog::ShowModal()
{
	LoadList();
	return wxDialog::ShowModal();
}

void ListSelectDialog::LoadList()
{
	wxArrayString sList;
	LambdaTask::TaskFunc func = [&] (LambdaTask *task) -> wxThread::ExitCode
	{
		task->DoSetStatus("Loading list...");
		return (wxThread::ExitCode)DoLoadList(sList);
	};

	LambdaTask *lTask = new LambdaTask(func);
	TaskProgressDialog taskDlg(this);
	taskDlg.ShowModal(lTask);
	delete lTask;

	list->Set(sList);

	UpdateOKBtn();
}

void ListSelectDialog::OnRefreshListClicked(wxCommandEvent& event)
{
	LoadList();
}

void ListSelectDialog::OnListBoxSelChange(wxCommandEvent& event)
{
	UpdateOKBtn();
}

void ListSelectDialog::UpdateOKBtn()
{
	SetControlEnable(this, wxID_OK, list->GetSelection() != wxNOT_FOUND);
}

wxString ListSelectDialog::GetSelection()
{
	if (list->GetSelection() != wxNOT_FOUND)
		return list->GetStringSelection();
	else
		return wxEmptyString;
}

BEGIN_EVENT_TABLE(ListSelectDialog, wxDialog)
	EVT_BUTTON(ID_RefreshList, ListSelectDialog::OnRefreshListClicked)
	EVT_LISTBOX(-1, ListSelectDialog::OnListBoxSelChange)
END_EVENT_TABLE()
