// 
//  Copyright 2012 Andrew Okin
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

#include "snapshotdialog.h"

#include <wx/gbsizer.h>

#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>

#include <string>
#include <sstream>

#include "apputils.h"
#include "httputils.h"
#include "snapshotlist.h"

enum
{
	ID_RefreshSnapshotList,
};

inline void SetControlEnable(wxWindow *parentWin, int id, bool state)
{
	wxWindow *win = parentWin->FindWindowById(id);
	if (win) win->Enable(state);
}

SnapshotDialog::SnapshotDialog(wxWindow *parent)
	: wxDialog(parent, wxID_ANY, _("Choose a snapshot"), wxDefaultPosition, wxSize(400, 420))
{
	wxFont titleFont(12, wxSWISS, wxNORMAL, wxNORMAL);

	wxBoxSizer *dlgSizer = new wxBoxSizer(wxVERTICAL);
	SetSizer(dlgSizer);

	wxPanel *mainPanel = new wxPanel(this);
	dlgSizer->Add(mainPanel, wxSizerFlags(1).Expand().Border(wxALL, 8));
	wxGridBagSizer *mainSz = new wxGridBagSizer();
	mainSz->AddGrowableCol(0, 0);
	mainSz->AddGrowableRow(1, 0);
	mainPanel->SetSizer(mainSz);

	wxStaticText *versionPageTitle = new wxStaticText(mainPanel, -1, _("Choose Version"));
	versionPageTitle->SetFont(titleFont);
	mainSz->Add(versionPageTitle, wxGBPosition(0, 0), wxGBSpan(1, 2), wxALL | wxALIGN_CENTER, 4);

	snapshotList = new wxListBox(mainPanel, -1, wxDefaultPosition, wxDefaultSize,
		wxArrayString(), wxLB_SINGLE);
	mainSz->Add(snapshotList, wxGBPosition(1, 0), wxGBSpan(1, 2), wxEXPAND | wxALL, 4);

	wxButton *versionRefreshBtn = new wxButton(mainPanel, ID_RefreshSnapshotList, _("&Refresh"));
	mainSz->Add(versionRefreshBtn, wxGBPosition(2, 1), wxGBSpan(1, 1), wxALL, 3);

	wxSizer *btnSz = CreateButtonSizer(wxOK | wxCANCEL);
	dlgSizer->Add(btnSz, wxSizerFlags(0).Border(wxBOTTOM | wxRIGHT, 8).
		Align(wxALIGN_RIGHT | wxALIGN_BOTTOM));

	SetControlEnable(this, wxID_OK, false);

	LoadSnapshotList();
}

void SnapshotDialog::LoadSnapshotList()
{
	SnapshotList snapList;
	snapList.LoadFromURL(wxT("assets.minecraft.net"));
	snapshotList->Set(snapList);

	UpdateOKBtn();
}

void SnapshotDialog::OnRefreshSListClicked(wxCommandEvent& event)
{
	LoadSnapshotList();
}

void SnapshotDialog::OnListBoxSelChange(wxCommandEvent& event)
{
	UpdateOKBtn();
}

void SnapshotDialog::UpdateOKBtn()
{
	SetControlEnable(this, wxID_OK, snapshotList->GetSelection() != wxNOT_FOUND);
}

wxString SnapshotDialog::GetSelectedSnapshot()
{
	if (snapshotList->GetSelection() != wxNOT_FOUND)
		return snapshotList->GetStringSelection();
	else
		return wxEmptyString;
}

BEGIN_EVENT_TABLE(SnapshotDialog, wxDialog)
	EVT_BUTTON(ID_RefreshSnapshotList, SnapshotDialog::OnRefreshSListClicked)
	EVT_LISTBOX(-1, SnapshotDialog::OnListBoxSelChange)
END_EVENT_TABLE()
