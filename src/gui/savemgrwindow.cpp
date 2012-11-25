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
#include <wx/clipbrd.h>

#include "mainwindow.h"

#include "utils/apputils.h"
#include "utils/fsutils.h"

#include "ziptask.h"
#include "filecopytask.h"
#include "taskprogressdialog.h"

enum
{
	ID_Explore,
	ID_ReloadList,

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
	saveList->SetDropTarget(new SaveListDropTarget(saveList, inst));
	mainBox->Add(saveList, wxGBPosition(0, 0), wxGBSpan(1, 1), wxEXPAND | wxALL, 4);

	// Buttons in the side panel beside the world list.
	{
		wxBoxSizer *sideBtnSz = new wxBoxSizer(wxVERTICAL);
		mainBox->Add(sideBtnSz, wxGBPosition(0, 1), wxGBSpan(1, 1), wxALL | wxEXPAND, 4);

		const wxSizerFlags topSideBtnFlags = wxSizerFlags(0).Border(wxBOTTOM, 4).Align(wxALIGN_TOP).Expand();
		const wxSizerFlags bottomSideBtnFlags = wxSizerFlags(0).Border(wxTOP, 4).Align(wxALIGN_BOTTOM).Expand();

		addBtn = new wxButton(mainPanel, wxID_ADD, _("&Add"));
		sideBtnSz->Add(addBtn, topSideBtnFlags);

		removeBtn = new wxButton(mainPanel, wxID_REMOVE, _("&Delete"));
		sideBtnSz->Add(removeBtn, topSideBtnFlags);

		sideBtnSz->AddStretchSpacer();

		exportZip = new wxButton(mainPanel, ID_ExportZip, _("Export to Zip"));
		sideBtnSz->Add(exportZip, bottomSideBtnFlags);

		EnableSideButtons(false);
	}

	// Buttons on the bottom of the dialog
	{
		wxBoxSizer *btnBox = new wxBoxSizer(wxHORIZONTAL);
		mainBox->Add(btnBox, wxGBPosition(1, 0), wxGBSpan(1, 2), 
			wxEXPAND | wxBOTTOM | wxRIGHT | wxLEFT, 4);


		wxSizerFlags leftBtnFlags = wxSizerFlags().Align(wxALIGN_LEFT).Border(wxTOP | wxBOTTOM | wxRIGHT, 4);

		wxButton *viewFolderBtn = new wxButton(mainPanel, ID_Explore, _("View Folder"));
		btnBox->Add(viewFolderBtn, leftBtnFlags);

		wxButton *refreshListBtn = new wxButton(mainPanel, ID_ReloadList, _("&Refresh"));
		btnBox->Add(refreshListBtn, leftBtnFlags);

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

	const int accelCount = 3;
	wxAcceleratorEntry entries[accelCount];
	entries[0].Set(wxACCEL_NORMAL,	WXK_DELETE,	wxID_DELETE);
	entries[1].Set(wxACCEL_CTRL,	(int) 'C',	wxID_COPY);
	entries[2].Set(wxACCEL_CTRL,	(int) 'V',	wxID_PASTE);
	wxAcceleratorTable accel(accelCount, entries);
	SetAcceleratorTable(accel);
}

void SaveMgrWindow::SaveListCtrl::UpdateListItems()
{
	SetItemCount(m_inst->GetWorldList()->size());
	Refresh();
	Update();
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

SaveMgrWindow::SaveListDropTarget::SaveListDropTarget(SaveListCtrl *owner, Instance *inst)
{
	m_inst = inst;
	m_owner = owner;
}

wxDragResult SaveMgrWindow::SaveListDropTarget::OnDragOver(wxCoord x, wxCoord y, wxDragResult def)
{
	return wxDragCopy;
}

bool SaveMgrWindow::SaveListDropTarget::OnDropFiles(wxCoord x, wxCoord y, const wxArrayString &filenames)
{
	for (wxArrayString::const_iterator iter = filenames.begin(); iter != filenames.end(); iter++)
	{
		m_owner->AddSaveFromPath(*iter);
	}
	return true;
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

void SaveMgrWindow::OnDragSave(wxListEvent &event)
{
	WorldList *worlds = m_inst->GetWorldList();
	wxFileDataObject worldFileObj;

	wxArrayInt indices = saveList->GetSelectedItems();
	for (wxArrayInt::const_iterator iter = indices.begin(); iter != indices.end(); ++iter)
	{
		wxFileName saveDir = worlds->at(*iter).GetSaveDir();
		saveDir.MakeAbsolute();
		worldFileObj.AddFile(saveDir.GetFullPath());
	}

	wxDropSource savesDropSource(worldFileObj, saveList);
	savesDropSource.DoDragDrop(wxDrag_CopyOnly);
}

wxArrayInt SaveMgrWindow::SaveListCtrl::GetSelectedItems()
{
	wxArrayInt indices;
	long item = -1;
	while (true)
	{
		item = GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

		if (item == -1)
			break;

		indices.Add(item);
	}
	return indices;
}

void SaveMgrWindow::SaveListCtrl::RefreshList()
{
	m_inst->GetWorldList()->UpdateWorldList();
	UpdateListItems();
}

void SaveMgrWindow::RefreshList()
{
	saveList->RefreshList();
}

void SaveMgrWindow::OnRefreshClicked(wxCommandEvent& event)
{
	RefreshList();
}

void SaveMgrWindow::OnAddClicked(wxCommandEvent& event)
{
	wxDirDialog dirDlg (this, "Choose a world to add.", wxEmptyString);
	if (dirDlg.ShowModal() == wxID_OK)
	{
		saveList->AddSaveFromPath(dirDlg.GetPath());
	}
}

void SaveMgrWindow::OnRemoveClicked(wxCommandEvent& event)
{
	saveList->DeleteSelected();
}

void SaveMgrWindow::SaveListCtrl::AddSaveFromPath(wxString path)
{
	if (!wxDirExists(path))
	{
		return;
	}

	wxFileName srcPath(path);
	wxFileName destPath(Path::Combine(m_inst->GetSavesDir(), srcPath.GetFullName()));

	// Skip if the destination is the same as the source.
	if (srcPath.SameAs(destPath))
	{
		return;
	}

	if (!wxFileExists(Path::Combine(path, "level.dat")) &&
		wxMessageBox(_("This folder does not contain a level.dat file. Continue?"), 
		_("Not a valid save."), wxOK | wxCANCEL | wxCENTER, GetParent()) == wxID_CANCEL)
	{
		return;
	}

	if (wxDirExists(destPath.GetFullPath()))
	{
		int ctr = 1;
		wxString dPathName = destPath.GetName();
		while (wxDirExists(destPath.GetFullPath()) && ctr < 9000)
		{
			destPath.SetName(wxString::Format("%s (%i)", dPathName.c_str(), ctr));
			ctr++;
		}

		if (wxMessageBox(wxString::Format(
			_("There's already a save with the filename '%s'. Copy to '%s' instead?"), 
			dPathName.c_str(), destPath.GetFullName().c_str()), 
			_("File already exists."), wxOK | wxCANCEL | wxCENTER, GetParent()) == wxCANCEL)
		{
			return;
		}
	}

	if (wxFileExists(destPath.GetFullPath()))
	{
		wxLogError("Failed to copy world. File already exists.");
		return;
	}

	FileCopyTask *task = new FileCopyTask(path, destPath.GetFullPath());
	TaskProgressDialog dlg(GetParent());
	dlg.ShowModal(task);
	delete task;

	RefreshList();
}

void SaveMgrWindow::SaveListCtrl::DeleteSelected()
{
	if (GetSelectedSave() == nullptr)
		return;

	wxMessageDialog dlg(GetParent(), "Are you sure you want to delete this world?\n"
		"Deleted worlds are lost FOREVER! (a really long time)",
		"Confirm deletion.",
		wxYES_NO | wxNO_DEFAULT | wxICON_QUESTION | wxCENTRE | wxSTAY_ON_TOP);
	dlg.CenterOnParent();
	if (dlg.ShowModal() == wxID_YES)
	{
		fsutils::RecursiveDelete(GetSelectedSave()->GetSaveDir());
		RefreshList();
	}
}

void SaveMgrWindow::SaveListCtrl::OnCopy(wxCommandEvent& event)
{
	WorldList *worlds = m_inst->GetWorldList();
	wxFileDataObject *worldFileObj = new wxFileDataObject;

	wxArrayInt indices = GetSelectedItems();
	for (wxArrayInt::const_iterator iter = indices.begin(); iter != indices.end(); ++iter)
	{
		wxFileName worldFile = worlds->at(*iter).GetSaveDir();
		worldFile.MakeAbsolute();
		worldFileObj->AddFile(worldFile.GetFullPath());
	}

	if (wxTheClipboard->Open())
	{
		wxTheClipboard->SetData(worldFileObj);
		wxTheClipboard->Close();
	}
}

void SaveMgrWindow::SaveListCtrl::OnPaste(wxCommandEvent& event)
{
	wxFileDataObject data;

	// Get data from the clipboard.
	if (wxTheClipboard->Open())
	{
		if (wxTheClipboard->IsSupported(wxDF_FILENAME))
		{
			wxTheClipboard->GetData(data);
		}
		else
		{
			wxTheClipboard->Close();
			return;
		}
		wxTheClipboard->Close();
	}

	wxArrayString filenames = data.GetFilenames();
	for (wxArrayString::const_iterator iter = filenames.begin(); iter != filenames.end(); iter++)
	{
		AddSaveFromPath(*iter);
	}
}

void SaveMgrWindow::SaveListCtrl::OnDelete(wxCommandEvent& event)
{
	DeleteSelected();
}

BEGIN_EVENT_TABLE(SaveMgrWindow, wxFrame)
	EVT_BUTTON(wxID_ADD, SaveMgrWindow::OnAddClicked)
	EVT_BUTTON(wxID_REMOVE, SaveMgrWindow::OnRemoveClicked)

	EVT_BUTTON(wxID_CLOSE, SaveMgrWindow::OnCloseClicked)
	EVT_BUTTON(ID_Explore, SaveMgrWindow::OnViewFolderClicked)
	EVT_BUTTON(ID_ReloadList, SaveMgrWindow::OnRefreshClicked)

	EVT_BUTTON(ID_ExportZip, SaveMgrWindow::OnExportZipClicked)

	EVT_LIST_ITEM_SELECTED(-1, SaveMgrWindow::OnSelChanged)
	EVT_LIST_ITEM_DESELECTED(-1, SaveMgrWindow::OnSelChanged)

	EVT_LIST_BEGIN_DRAG(-1, SaveMgrWindow::OnDragSave)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(SaveMgrWindow::SaveListCtrl, wxListCtrl)
	EVT_MENU(wxID_COPY, SaveMgrWindow::SaveListCtrl::OnCopy)
	EVT_MENU(wxID_PASTE, SaveMgrWindow::SaveListCtrl::OnPaste)
	EVT_MENU(wxID_DELETE, SaveMgrWindow::SaveListCtrl::OnDelete)
END_EVENT_TABLE()
