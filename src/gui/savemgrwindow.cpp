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

#include "savemgrwindow.h"

#include <wx/sizer.h>
#include <wx/gbsizer.h>
#include <wx/wfstream.h>

#include "mainwindow.h"

#include "apputils.h"
#include "fsutils.h"

#include "ziptask.h"
#include "taskprogressdialog.h"

enum
{
	ID_Explore,

	ID_ExportZip,
};

SaveMgrWindow::SaveMgrWindow(MainWindow *parent, Instance *inst)
	: wxFrame(parent, -1, wxT("Manage Saves"), wxDefaultPosition, wxSize(500, 400))
{
	m_parent = parent;
	m_inst = inst;

	wxPanel *mainPanel = new wxPanel(this);

	SetTitle(wxString::Format(_("Manage Saves for %s"), inst->GetName().c_str()));

	wxGridBagSizer *mainBox = new wxGridBagSizer();
	mainPanel->SetSizer(mainBox);

	saveList = new SaveListCtrl(mainPanel, inst);
	saveList->AppendColumn(_("World Name"), wxLIST_FORMAT_LEFT, 250);
	mainBox->Add(saveList, wxGBPosition(0, 0), wxGBSpan(1, 1), wxEXPAND | wxALL, 4);

	// Buttons in the side panel beside the world list.
	{
		wxBoxSizer *sideBtnSz = new wxBoxSizer(wxVERTICAL);
		mainBox->Add(sideBtnSz, wxGBPosition(0, 1), wxGBSpan(1, 1), wxALL, 4);

		const wxSizerFlags sideBtnSzFlags = wxSizerFlags().Border(wxBOTTOM, 4);

		exportZip = new wxButton(mainPanel, ID_ExportZip, _("Export to Zip"));
		sideBtnSz->Add(exportZip, sideBtnSzFlags);

		EnableSideButtons(false);
	}

	// Buttons on the bottom of the dialog
	{
		wxBoxSizer *btnBox = new wxBoxSizer(wxHORIZONTAL);
		mainBox->Add(btnBox, wxGBPosition(1, 0), wxGBSpan(1, 2), 
			wxEXPAND | wxBOTTOM | wxRIGHT | wxLEFT, 8);


		wxButton *viewFolderBtn = new wxButton(mainPanel, ID_Explore, _("View Folder"));
		btnBox->Add(viewFolderBtn, wxSizerFlags().Align(wxALIGN_LEFT).Border(wxTOP | wxBOTTOM, 4));

		btnBox->AddStretchSpacer();

		wxButton *btnClose = new wxButton(mainPanel, wxID_CLOSE, _("&Close"));
		btnBox->Add(btnClose, wxSizerFlags().Align(wxALIGN_RIGHT).Border(wxTOP | wxBOTTOM, 4));
	}

	mainBox->AddGrowableCol(0);
	mainBox->AddGrowableRow(0);

	CenterOnParent();
}

SaveMgrWindow::SaveListCtrl::SaveListCtrl(wxWindow *parent, Instance *inst)
	: wxListCtrl(parent, -1, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxLC_VIRTUAL | wxLC_VRULES)
{
	m_inst = inst;
	UpdateListItems();
}

void SaveMgrWindow::SaveListCtrl::UpdateListItems()
{
	SetItemCount(m_inst->GetWorldList()->size());
}

wxString SaveMgrWindow::SaveListCtrl::OnGetItemText(long item, long col) const
{
	WorldList *worldList = m_inst->GetWorldList();

	if (item >= worldList->size())
		return wxT("Error: Index out of bounds!");

	switch (col)
	{
	case 0:
		return worldList->at(item).GetSaveName();

	default:
		return wxEmptyString;
	}
}

World *SaveMgrWindow::SaveListCtrl::GetSelectedSave()
{
	long item = -1;
	while (true)
	{
		item = GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

		if (item == -1)
			break;

		return &m_inst->GetWorldList()->at(item);
	}
	return nullptr;
}

void SaveMgrWindow::EnableSideButtons(bool enable)
{
	exportZip->Enable(enable);
}

void SaveMgrWindow::OnSelChanged(wxListEvent &event)
{
	EnableSideButtons(saveList->GetSelectedSave() != nullptr);
}

void SaveMgrWindow::OnViewFolderClicked(wxCommandEvent& event)
{
	Utils::OpenFolder(m_inst->GetSavesDir().GetFullPath());
}

void SaveMgrWindow::OnCloseClicked(wxCommandEvent& event)
{
	Close();
}

void SaveMgrWindow::OnExportZipClicked(wxCommandEvent& event)
{
	World *world = saveList->GetSelectedSave();
	wxFileDialog exportZipDialog(this, "Choose a file to add.",
		wxEmptyString, wxEmptyString, "*.zip" , 
		wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

	if (exportZipDialog.ShowModal() == wxID_OK)
	{
		wxString fileDest = exportZipDialog.GetPath();

		wxFFileOutputStream outStream(fileDest);
		ZipTask *task = new ZipTask(&outStream, world->GetSaveDir());

		TaskProgressDialog dlg(this);
		dlg.ShowModal(task);

		delete task;
	}
}

BEGIN_EVENT_TABLE(SaveMgrWindow, wxFrame)
	EVT_BUTTON(wxID_CLOSE, SaveMgrWindow::OnCloseClicked)
	EVT_BUTTON(ID_Explore, SaveMgrWindow::OnViewFolderClicked)

	EVT_BUTTON(ID_ExportZip, SaveMgrWindow::OnExportZipClicked)

	EVT_LIST_ITEM_SELECTED(-1, SaveMgrWindow::OnSelChanged)
	EVT_LIST_ITEM_DESELECTED(-1, SaveMgrWindow::OnSelChanged)
END_EVENT_TABLE()
