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

#include "modeditdialog.h"

ModEditDialog::ModEditDialog(wxWindow *parent, Instance *inst)
	: wxDialog(parent, -1, _("Edit Mods"), wxDefaultPosition, wxSize(500, 400))
{
	m_inst = inst;
	
	wxBoxSizer *mainBox = new wxBoxSizer(wxVERTICAL);
	SetSizer(mainBox);
	
	wxNotebook *modEditNotebook = new wxNotebook(this, -1);
	mainBox->Add(modEditNotebook, wxSizerFlags(1).Expand().Border(wxALL, 8));
	
	wxPanel *jarModPanel = new wxPanel(modEditNotebook, -1);
	wxBoxSizer *jarModSizer = new wxBoxSizer(wxHORIZONTAL);
	jarModPanel->SetSizer(jarModSizer);
	modEditNotebook->AddPage(jarModPanel, _("Jar Mods"), true);
	
	jarModList = new JarModListCtrl(jarModPanel, inst);
	jarModList->InsertColumn(0, _("Mod Name"));
	jarModList->InsertColumn(1, _("Mod Version"), wxLIST_FORMAT_RIGHT);
	jarModList->SetDropTarget(new ModsDropTarget(jarModList, inst));
	jarModSizer->Add(jarModList, wxSizerFlags(1).Expand().Border(wxALL, 8));
	
	
	wxPanel *mlModPanel = new wxPanel(modEditNotebook, -1);
	wxBoxSizer *mlModSizer = new wxBoxSizer(wxHORIZONTAL);
	mlModPanel->SetSizer(mlModSizer);
	modEditNotebook->AddPage(mlModPanel, _("Mods Folder"));
	
	mlModList = new wxListCtrl(mlModPanel, -1);
	mlModSizer->Add(mlModList, wxSizerFlags(1).Expand().Border(wxALL, 8));
	
	wxSizer *btnBox = CreateButtonSizer(wxOK);
	mainBox->Add(btnBox, 0, wxALIGN_RIGHT | wxBOTTOM | wxRIGHT, 8);
	
	CenterOnParent();
	
	LoadJarMods();
	LoadMLMods();
}

void ModEditDialog::LoadJarMods()
{
	jarModList->Update();
}

void ModEditDialog::JarModListCtrl::Update()
{
	SetItemCount(m_inst->GetModList()->size());
	wxGenericListCtrl::Update();
}

void ModEditDialog::LoadMLMods()
{
	
}

ModEditDialog::JarModListCtrl::JarModListCtrl(wxWindow *parent, Instance *inst)
	: wxListCtrl(parent, -1, wxDefaultPosition, wxDefaultSize, 
				 wxLC_REPORT | wxLC_VIRTUAL | wxLC_VRULES)
{
	m_inst = inst;
}

wxString ModEditDialog::JarModListCtrl::OnGetItemText(long int item, long int column) const
{
	switch (column)
	{
	case 0:
		return (*m_inst->GetModList())[item].GetName();
	case 1:
		return (*m_inst->GetModList())[item].GetModVersion();
	default:
		return wxEmptyString;
	}
}

void ModEditDialog::UpdateColSizes()
{
	const int versionColumnWidth = 100;
	
	int width = jarModList->GetClientSize().GetWidth();
	
	jarModList->SetColumnWidth(0, width - versionColumnWidth);
	jarModList->SetColumnWidth(1, versionColumnWidth);
}



bool ModEditDialog::Show(bool show)
{
	int response = wxDialog::Show(show);
	UpdateColSizes();
	return response;
}

ModsDropTarget::ModsDropTarget(wxListCtrl *owner, Instance *inst)
{
	m_owner = owner;
	m_inst = inst;
}

wxDragResult ModsDropTarget::OnDragOver(wxCoord x, wxCoord y, wxDragResult def)
{
	def = wxDragResult::wxDragCopy;
	return def;
}

bool ModsDropTarget::OnDropFiles(wxCoord x, wxCoord y, const wxArrayString &filenames)
{
	int flags = wxLIST_HITTEST_ONITEM;
	long index = m_owner->HitTest(wxPoint(x, y), flags);
	
	for (wxArrayString::const_iterator iter = filenames.begin(); iter != filenames.end(); iter++)
	{
		wxFileName modFileName(*iter);
		m_inst->InsertMod(index, modFileName);
		m_owner->Update();
	}
}

BEGIN_EVENT_TABLE(ModEditDialog, wxDialog)
	
END_EVENT_TABLE()
