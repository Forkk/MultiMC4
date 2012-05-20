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
#include <wx/notebook.h>
#include <apputils.h>

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
	
	jarModList = new ModListCtrl(jarModPanel, ID_JAR_MOD_LIST, inst);
	jarModList->InsertColumn(0, _("Mod Name"));
	jarModList->InsertColumn(1, _("Mod Version"), wxLIST_FORMAT_RIGHT);
	jarModList->SetDropTarget(new JarModsDropTarget(jarModList, inst));
	jarModSizer->Add(jarModList, wxSizerFlags(1).Expand().Border(wxALL, 8));
	
	wxBoxSizer *jarListBtnBox = new wxBoxSizer(wxVERTICAL);
	jarModSizer->Add(jarListBtnBox, wxSizerFlags(0).Border(wxTOP | wxBOTTOM, 4));
	
	wxButton *addJarModBtn = new wxButton(jarModPanel, ID_ADD_JAR_MOD, _("&Add"));
	jarListBtnBox->Add(addJarModBtn, wxSizerFlags(0).Border(wxTOP | wxBOTTOM | wxRIGHT, 4));
	
	delJarModBtn = new wxButton(jarModPanel, ID_DEL_JAR_MOD, _("&Remove"));
	jarListBtnBox->Add(delJarModBtn, wxSizerFlags(0).Border(wxTOP | wxBOTTOM | wxRIGHT, 4));
	
	jarModUpBtn = new wxButton(jarModPanel, ID_MOVE_JAR_MOD_UP, _("Move &Up"));
	jarListBtnBox->Add(jarModUpBtn, wxSizerFlags(0).Border(wxTOP | wxBOTTOM | wxRIGHT, 4).Align(wxALIGN_BOTTOM));
	
	jarModDownBtn = new wxButton(jarModPanel, ID_MOVE_JAR_MOD_DOWN, _("Move &Down"));
	jarListBtnBox->Add(jarModDownBtn, wxSizerFlags(0).Border(wxTOP | wxBOTTOM | wxRIGHT, 4).Align(wxALIGN_BOTTOM));
	
	
	wxPanel *mlModPanel = new wxPanel(modEditNotebook, -1);
	wxBoxSizer *mlModSizer = new wxBoxSizer(wxHORIZONTAL);
	mlModPanel->SetSizer(mlModSizer);
	modEditNotebook->AddPage(mlModPanel, _("Mods Folder"));
	
	mlModList = new ModListCtrl(mlModPanel, ID_ML_MOD_LIST, inst);
	mlModList->InsertColumn(0, _("Mod Name"));
	mlModList->InsertColumn(1, _("Mod Version"), wxLIST_FORMAT_RIGHT);
	mlModList->SetDropTarget(new MLModsDropTarget(mlModList, inst));
	mlModSizer->Add(mlModList, wxSizerFlags(1).Expand().Border(wxALL, 8));
	
	wxBoxSizer *mlModListBtnBox = new wxBoxSizer(wxVERTICAL);
	mlModSizer->Add(mlModListBtnBox, wxSizerFlags(0).Border(wxTOP | wxBOTTOM, 4));
	
	wxButton *addMLModBtn = new wxButton(mlModPanel, ID_ADD_ML_MOD, _("&Add"));
	mlModListBtnBox->Add(addMLModBtn, wxSizerFlags(0).Border(wxTOP | wxBOTTOM | wxRIGHT, 4));
	
	wxButton *delMLModBtn = new wxButton(mlModPanel, ID_DEL_ML_MOD, _("&Remove"));
	mlModListBtnBox->Add(delMLModBtn, wxSizerFlags(0).Border(wxTOP | wxBOTTOM | wxRIGHT, 4));
	
	
	wxSizer *btnBox = CreateButtonSizer(wxOK);
	mainBox->Add(btnBox, 0, wxALIGN_RIGHT | wxBOTTOM | wxRIGHT | wxLEFT, 8);
	
	CenterOnParent();
	
	LoadJarMods();
	LoadMLMods();
}

void ModEditDialog::LoadJarMods()
{
	jarModList->UpdateItems();
}

void ModEditDialog::ModListCtrl::UpdateItems()
{
	SetItemCount(GetModList()->size());
}

void ModEditDialog::LoadMLMods()
{
	mlModList->UpdateItems();
}

ModEditDialog::ModListCtrl::ModListCtrl(wxWindow *parent, int id, Instance *inst)
	: wxListCtrl(parent, id, wxDefaultPosition, wxDefaultSize, 
				 wxLC_REPORT | wxLC_VIRTUAL | wxLC_VRULES)
{
	m_inst = inst;
	m_insMarkIndex = -1;
}

wxString ModEditDialog::ModListCtrl::OnGetItemText(long int item, long int column) const
{
	switch (column)
	{
	case 0:
		return (*GetModList())[item].GetName();
	case 1:
		return (*GetModList())[item].GetModVersion();
	default:
		return wxEmptyString;
	}
}

ModList *ModEditDialog::ModListCtrl::GetModList() const
{
	if (GetId() == ID_ML_MOD_LIST)
		return m_inst->GetMLModList();
	else
		return m_inst->GetModList();
}

void ModEditDialog::UpdateColSizes()
{
	const int versionColumnWidth = 100;
	
	int width = jarModList->GetClientSize().GetWidth();
	
	jarModList->SetColumnWidth(0, width - versionColumnWidth);
	jarModList->SetColumnWidth(1, versionColumnWidth);
	
	mlModList->SetColumnWidth(0, width - versionColumnWidth);
	mlModList->SetColumnWidth(1, versionColumnWidth);
}



bool ModEditDialog::Show(bool show)
{
	int response = wxDialog::Show(show);
	UpdateColSizes();
	return response;
}

ModEditDialog::JarModsDropTarget::JarModsDropTarget(ModEditDialog::ModListCtrl *owner, Instance *inst)
{
	m_owner = owner;
	m_inst = inst;
}

wxDragResult ModEditDialog::JarModsDropTarget::OnDragOver(wxCoord x, wxCoord y, wxDragResult def)
{
	int flags = wxLIST_HITTEST_BELOW;
	long index = 0;
	
	if (m_owner->GetItemCount() > 0)
	{
		index = m_owner->HitTest(wxPoint(x, y), flags);
	}
	
	def = wxDragResult::wxDragCopy;
	if (index != wxNOT_FOUND)
	{
		m_owner->SetInsertMark(index);
	}
	else
	{
		m_owner->SetInsertMark(m_owner->GetItemCount());
	}
	
	return def;
}

void ModEditDialog::JarModsDropTarget::OnLeave()
{
	m_owner->SetInsertMark(-1);
}

bool ModEditDialog::JarModsDropTarget::OnDropFiles(wxCoord x, wxCoord y, const wxArrayString &filenames)
{
	int flags = wxLIST_HITTEST_ONITEM;
	
	long index = 0;
	
	if (m_owner->GetItemCount() > 0)
	{
		index = m_owner->HitTest(wxPoint(x, y), flags);
	}
	
	m_owner->SetInsertMark(-1);
	
	for (wxArrayString::const_iterator iter = filenames.begin(); iter != filenames.end(); iter++)
	{
		wxFileName modFileName(*iter);
		m_inst->InsertMod(index, modFileName);
		m_owner->UpdateItems();
	}
}

void ModEditDialog::OnDeleteJarMod()
{
	if (jarModList->GetItemCount() <= 0)
		return;
	
	wxArrayInt indices = jarModList->GetSelectedItems();
	for (int i = indices.GetCount() -1; i >= 0; i--)
	{
		m_inst->DeleteMod(indices[i]);
	}
	jarModList->UpdateItems();
}

void ModEditDialog::OnDeleteMLMod()
{
	if (mlModList->GetItemCount() <= 0)
		return;
	
	wxArrayInt indices;
	long item = -1;
	while (true)
	{
		item = mlModList->GetNextItem(item, wxLIST_NEXT_ALL, 
									  wxLIST_STATE_SELECTED);
		
		if (item == -1)
			break;
		
		indices.Add(item);
	}
	
	for (int i = indices.GetCount() -1; i >= 0; i--)
	{
		m_inst->DeleteMLMod(indices[i]);
	}
	m_inst->LoadMLModList();
	mlModList->UpdateItems();
}

void ModEditDialog::OnJarListKeyDown(wxListEvent &event)
{
	if (event.GetKeyCode() == WXK_DELETE)
	{
		OnDeleteJarMod();
	}
}

void ModEditDialog::OnMLListKeyDown(wxListEvent &event)
{
	if (event.GetKeyCode() == WXK_DELETE)
	{
		OnDeleteMLMod();
	}
}

void ModEditDialog::ModListCtrl::SetInsertMark(const int index)
{
	m_insMarkIndex = index;
	Refresh();
	Update();
	DrawInsertMark(index);
}

void ModEditDialog::ModListCtrl::DrawInsertMark(int index)
{
	if (index < 0)
		return;
#ifdef WIN32
	wxWindow *listMainWin = (wxWindow*)this;
#else
	wxWindow *listMainWin = (wxWindow*)this->m_mainWin;
#endif
	
	int lineY = 0;
	wxRect itemRect;
	this->GetItemRect(index, itemRect);
	lineY = itemRect.GetTop() - listMainWin->GetPosition().y;
	
	if (index == GetItemCount() && GetItemCount() > 0)
	{
		this->GetItemRect(index - 1, itemRect);
		lineY = itemRect.GetBottom() - listMainWin->GetPosition().y;
	}
	
	wxWindowDC dc(listMainWin);
	dc.SetPen(wxPen(wxColour(_("white")), 2, wxSOLID));
	
	const wxBrush *brush = wxTRANSPARENT_BRUSH;
	dc.SetBrush(*brush);
	dc.SetLogicalFunction(wxXOR);
	dc.DrawLine(0, lineY, this->GetClientSize().GetWidth(), lineY);
}

ModEditDialog::MLModsDropTarget::MLModsDropTarget(ModEditDialog::ModListCtrl *owner, Instance *inst)
{
	m_owner = owner;
	m_inst = inst;
}

wxDragResult ModEditDialog::MLModsDropTarget::OnDragOver(wxCoord x, wxCoord y, wxDragResult def)
{
	return wxDragResult::wxDragCopy;
}

bool ModEditDialog::MLModsDropTarget::OnDropFiles(wxCoord x, wxCoord y, const wxArrayString &filenames)
{
	for (wxArrayString::const_iterator iter = filenames.begin(); iter != filenames.end(); ++iter)
	{
		wxFileName dest(Path::Combine(m_inst->GetMLModsDir().GetFullPath(), *iter));
		wxCopyFile(*iter, dest.GetFullPath());
	}
	
	m_inst->LoadMLModList();
	m_owner->UpdateItems();
}

void ModEditDialog::OnAddJarMod(wxCommandEvent &event)
{
	wxFileDialog *addModDialog = new wxFileDialog(this, _("Choose a file to add."), 
		wxGetCwd(), wxEmptyString);
	if (addModDialog->ShowModal() == wxID_OK)
	{
		wxFileName file(addModDialog->GetPath());
		m_inst->InsertMod(m_inst->GetModList()->size(), file);
		jarModList->UpdateItems();
	}
}

void ModEditDialog::OnDeleteJarMod(wxCommandEvent &event)
{
	OnDeleteJarMod();
}

void ModEditDialog::OnMoveJarModUp(wxCommandEvent &event)
{
	if (jarModList->GetItemCount() <= 0)
		return;
	
	wxArrayInt indices = jarModList->GetSelectedItems();
	ModList *mods= m_inst->GetModList();
	for (int i = 0; i < indices.GetCount(); ++i)
	{
		if (indices[i] == 0)
			continue;
		
		Mod mod = mods->at(indices[i]);
		mods->erase(mods->begin() + indices[i]);
		mods->insert(mods->begin() + indices[i] - 1, mod);
		
		jarModList->SetItemState(indices[i], 0, wxLIST_STATE_SELECTED);
		jarModList->SetItemState(indices[i] - 1, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
	}
	
	jarModList->UpdateItems();
}

void ModEditDialog::OnMoveJarModDown(wxCommandEvent &event)
{
	if (jarModList->GetItemCount() <= 0)
		return;
	
	wxArrayInt indices = jarModList->GetSelectedItems();
	ModList *mods= m_inst->GetModList();
	for (int i = indices.GetCount() - 1; i >= 0; --i)
	{
		if (indices[i] + 1 >= mods->size())
			continue;
		
		Mod mod = mods->at(indices[i]);
		mods->erase(mods->begin() + indices[i]);
		mods->insert(mods->begin() + indices[i] + 1, mod);
		
		jarModList->SetItemState(indices[i], 0, wxLIST_STATE_SELECTED);
		jarModList->SetItemState(indices[i] + 1, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
	}
}

void ModEditDialog::OnJarModSelChanged(wxListEvent &event)
{
	int selCount = jarModList->GetSelectedItemCount();
	delJarModBtn->Enable(selCount > 0);
	jarModUpBtn->Enable(selCount > 0);
	jarModDownBtn->Enable(selCount > 0);
}


void ModEditDialog::OnAddMLMod(wxCommandEvent &event)
{
	wxFileDialog *addModDialog = new wxFileDialog(this, _("Choose a file to add."), 
												  wxGetCwd(), wxEmptyString);
	if (addModDialog->ShowModal() == wxID_OK)
	{
		wxFileName file(addModDialog->GetPath());
		wxCopyFile(file.GetFullPath(), Path::Combine(m_inst->GetMLModsDir(), file.GetFullName()));
		jarModList->UpdateItems();
	}
}

void ModEditDialog::OnDeleteMLMod(wxCommandEvent &event)
{
	OnDeleteMLMod();
}

wxArrayInt ModEditDialog::ModListCtrl::GetSelectedItems()
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

void ModEditDialog::OnDragJarMod(wxListEvent &event)
{
	ModList *mods = m_inst->GetModList();
	wxFileDataObject modFileObj;
	
	wxArrayInt indices = jarModList->GetSelectedItems();
	for (wxArrayInt::const_iterator iter = indices.begin(); iter != indices.end(); ++iter)
	{
		wxFileName modFile = mods->at(*iter).GetFileName();
		modFile.MakeAbsolute();
		modFileObj.AddFile(modFile.GetFullPath());
	}
	
	wxDropSource modsDropSource(modFileObj, jarModList);
	modsDropSource.DoDragDrop(wxDrag_CopyOnly);
}


BEGIN_EVENT_TABLE(ModEditDialog, wxDialog)
	EVT_LIST_KEY_DOWN(ID_JAR_MOD_LIST, ModEditDialog::OnJarListKeyDown)
	EVT_LIST_KEY_DOWN(ID_ML_MOD_LIST, ModEditDialog::OnMLListKeyDown)
	
	EVT_BUTTON(ID_ADD_JAR_MOD, ModEditDialog::OnAddJarMod)
	EVT_BUTTON(ID_DEL_JAR_MOD, ModEditDialog::OnDeleteJarMod)
	EVT_BUTTON(ID_MOVE_JAR_MOD_UP, ModEditDialog::OnMoveJarModUp)
	EVT_BUTTON(ID_MOVE_JAR_MOD_DOWN, ModEditDialog::OnMoveJarModDown)
	
	EVT_LIST_ITEM_SELECTED(ID_JAR_MOD_LIST, ModEditDialog::OnJarModSelChanged)
	EVT_LIST_ITEM_DESELECTED(ID_JAR_MOD_LIST, ModEditDialog::OnJarModSelChanged)
	
	EVT_LIST_BEGIN_DRAG(ID_JAR_MOD_LIST, ModEditDialog::OnDragJarMod)
END_EVENT_TABLE()
