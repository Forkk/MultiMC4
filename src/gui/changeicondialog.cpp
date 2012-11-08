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

#include "changeicondialog.h"

#include <wx/filename.h>

#include "apputils.h"
#include "appsettings.h"

enum
{
	ID_AddIcon,
	ID_RemoveIcon,
	ID_ReloadIcons,
};

ChangeIconDialog::ChangeIconDialog(wxWindow *parent)
	: wxDialog(parent, -1, _("Change Icon"), wxDefaultPosition, wxSize(500, 400))
{
	wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);
	
	wxBoxSizer *hSz = new wxBoxSizer(wxHORIZONTAL);
	iconListCtrl = new InstIconListCtrl(this);
	hSz->Add(iconListCtrl, wxSizerFlags(1).Expand().Border(wxALL, 8));

	wxBoxSizer *sideBtnSz = new wxBoxSizer(wxVERTICAL);

	wxButton *addIconBtn = new wxButton(this, ID_AddIcon, _("&Add Icon"));
	sideBtnSz->Add(addIconBtn, wxSizerFlags(0).Border(wxTOP | wxBOTTOM | wxRIGHT, 4).Expand());

	wxButton *removeIconBtn = new wxButton(this, ID_RemoveIcon, _("&Remove Icon"));
	sideBtnSz->Add(removeIconBtn, wxSizerFlags(0).Border(wxTOP | wxBOTTOM | wxRIGHT, 4).Expand());

	wxButton *reloadIconBtn = new wxButton(this, ID_ReloadIcons, _("&Reload Icons"));
	reloadIconBtn->Enable(false);
	sideBtnSz->Add(reloadIconBtn, wxSizerFlags(0).Border(wxTOP | wxBOTTOM | wxRIGHT, 4).Expand());

	hSz->Add(sideBtnSz, wxSizerFlags().Border(wxTOP | wxBOTTOM | wxRIGHT, 4));

	mainSizer->Add(hSz, wxSizerFlags(1).Expand());
	
	wxSizer *btnSizer = CreateButtonSizer(wxOK | wxCANCEL);
	mainSizer->Add(btnSizer, wxSizerFlags(0).Border(wxBOTTOM | wxRIGHT, 8).
		Align(wxALIGN_RIGHT | wxALIGN_BOTTOM));
	
	SetSizer(mainSizer);
// 	mainSizer->SetSizeHints(this);
}

wxString ChangeIconDialog::GetSelectedIconKey() const
{
	long item = -1;
	while (true)
	{
		item = iconListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
		
		if (item == -1)
			break;
		
		return iconListCtrl->GetItemText(item);
	}
	return "default";
}

ChangeIconDialog::InstIconListCtrl::InstIconListCtrl(wxWindow *parent)
	: wxListCtrl(parent, -1, wxDefaultPosition, wxDefaultSize, wxLC_LIST | wxLC_SINGLE_SEL)
{
	UpdateItems();
}

void ChangeIconDialog::InstIconListCtrl::UpdateItems()
{
	this->ClearAll();
	auto iconList = InstIconList::Instance();
	wxImageList *imgList = iconList->CreateImageList();
	AssignImageList(imgList, wxIMAGE_LIST_NORMAL);
	SetImageList(imgList, wxIMAGE_LIST_SMALL);
	
	const InstIconMap &iconMap = iconList->GetIconMap();
	int i = 0;
	for (InstIconMap::const_iterator iter = iconMap.begin(); iter != iconMap.end(); ++iter)
	{
		wxString key = iter->first;
		InsertItem(i, key, i);
		i++;
	}
}

void ChangeIconDialog::OnItemActivated(wxListEvent &event)
{
	EndModal(wxID_OK);
}

void ChangeIconDialog::OnAddIcon(wxCommandEvent &event)
{
	wxFileDialog addIconDialog (this, _("Choose an icon to add."), 
		wxGetCwd(), wxEmptyString, wxT("*.png"));
	if (addIconDialog.ShowModal() == wxID_OK)
	{
		auto iconList = InstIconList::Instance();

		if (!wxDirExists(settings->GetIconsDir().GetFullPath()))
			wxMkdir(settings->GetIconsDir().GetFullPath());

		wxFileName src(addIconDialog.GetPath());
		wxFileName dest = Path::Combine(settings->GetIconsDir(), src.GetFullName());

		wxCopyFile(src.GetFullPath(), dest.GetFullPath());
		iconList->AddFile(dest.GetFullPath());
	}
	iconListCtrl->UpdateItems();
}

void ChangeIconDialog::OnRemoveIcon(wxCommandEvent &event)
{
	auto iconList = InstIconList::Instance();
	wxString removeIcon = GetSelectedIconKey();
	wxString iconFilename = iconList->getFileNameForKey(removeIcon);

	if (wxFileExists(iconFilename))
	{
		wxRemoveFile(iconFilename);
	}
	else
	{
		return;
	}

	iconList->RemoveIcon(removeIcon);
	iconListCtrl->UpdateItems();
}

void ChangeIconDialog::OnReloadIcons(wxCommandEvent &event)
{
	// TODO Make this work :P
	iconListCtrl->UpdateItems();
}

BEGIN_EVENT_TABLE(ChangeIconDialog, wxDialog)
	EVT_LIST_ITEM_ACTIVATED(-1, ChangeIconDialog::OnItemActivated)

	EVT_BUTTON(ID_AddIcon, ChangeIconDialog::OnAddIcon)
	EVT_BUTTON(ID_RemoveIcon, ChangeIconDialog::OnRemoveIcon)
	EVT_BUTTON(ID_ReloadIcons, ChangeIconDialog::OnReloadIcons)
END_EVENT_TABLE()

