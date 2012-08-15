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

#include "changeicondialog.h"

ChangeIconDialog::ChangeIconDialog(wxWindow *parent)
	: wxDialog(parent, -1, _("Change Icon"), wxDefaultPosition, wxSize(500, 400))
{
	wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);
	
	iconListCtrl = new InstIconListCtrl(this);
	mainSizer->Add(iconListCtrl, wxSizerFlags(1).Border(wxALL, 8).Expand());
	
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
	return _("default");
}

ChangeIconDialog::InstIconListCtrl::InstIconListCtrl(wxWindow *parent)
	: wxListCtrl(parent, -1, wxDefaultPosition, wxDefaultSize, wxLC_LIST | wxLC_SINGLE_SEL)
{
	UpdateItems();
}

void ChangeIconDialog::InstIconListCtrl::UpdateItems()
{
	auto iconList = InstIconList::Instance();
	wxImageList *imgList = iconList->CreateImageList();
	AssignImageList(imgList, wxIMAGE_LIST_NORMAL);
	SetImageList(imgList, wxIMAGE_LIST_SMALL);
	
	const IconListIndexMap &indexMap = iconList->GetIndexMap();
	for (IconListIndexMap::const_iterator iter = indexMap.begin(); iter != indexMap.end(); ++iter)
	{
		wxString key = iter->first;
		int index = iter->second;
		InsertItem(index, key, index);
	}
}

void ChangeIconDialog::OnItemActivated(wxListEvent &event)
{
	EndModal(wxID_OK);
}

BEGIN_EVENT_TABLE(ChangeIconDialog, wxDialog)
	EVT_LIST_ITEM_ACTIVATED(-1, ChangeIconDialog::OnItemActivated)
END_EVENT_TABLE()

