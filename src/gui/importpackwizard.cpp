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

#include "importpackwizard.h"

#include <wx/gbsizer.h>
#include <wx/wfstream.h>
#include <wx/zipstrm.h>

#include "mainwindow.h"
#include "apputils.h"
#include "fsutils.h"

#include "stdinstance.h"

#include <memory>

ImportPackWizard::ImportPackWizard(MainWindow *parent, ConfigPack *pack)
	: wxWizard(parent, -1, _("Import Config Pack"))
{
	auto sizer = GetPageAreaSizer();
	this->m_pack = pack;
	this->m_mainWin = parent;

	this->SetPageSize(wxSize(400, 300));

	wxFont titleFont(12, wxSWISS, wxNORMAL, wxNORMAL);
	
	packInfoPage = new wxWizardPageSimple(this);
	sizer->Add(packInfoPage);
	auto infoPageSz = new wxBoxSizer(wxVERTICAL);
	packInfoPage->SetSizer(infoPageSz);

	wxStaticText *infoTitleLabel = new wxStaticText(packInfoPage, -1, m_pack->GetPackName(), 
		wxDefaultPosition, wxDefaultSize);
	infoTitleLabel->SetFont(titleFont);
	infoPageSz->Add(infoTitleLabel, 0, wxALIGN_CENTER | wxALL, 4);

	packNotesTextbox = new wxTextCtrl(packInfoPage, -1, m_pack->GetPackNotes(),
		wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY);
	infoPageSz->Add(packNotesTextbox, 1, wxEXPAND | wxALL, 4);


	findModFilesPage = new wxWizardPageSimple(this, packInfoPage);
	packInfoPage->SetNext(findModFilesPage);
	wxGridBagSizer *configPageSz = new wxGridBagSizer();
	
	findModFilesPage->SetSizer(configPageSz);

	wxStaticText *configTitleLabel = new wxStaticText(findModFilesPage, -1, 
		_("Please add the mods listed below to your central mods folder and click refresh."));
	configPageSz->Add(configTitleLabel, wxGBPosition(0, 0), wxGBSpan(1, 3), wxALIGN_CENTER | wxALL, 4);

	wxArrayString configList;
	missingModsList = new wxListBox(findModFilesPage, -1,
		wxDefaultPosition, wxDefaultSize, configList);
	configPageSz->Add(missingModsList, wxGBPosition(1, 0), wxGBSpan(1, 3), wxALIGN_CENTER | wxALL | wxEXPAND, 4);
	
	wxButton *viewCentralModsFolderButton = new wxButton(findModFilesPage, ID_ViewCentralModsFolder,
		_("&View Central Mods Folder"));
	configPageSz->Add(viewCentralModsFolderButton, wxGBPosition(2, 0), wxGBSpan(1, 1), wxALIGN_CENTER | wxALL, 4);

	wxButton *refreshButton = new wxButton(findModFilesPage, ID_RefreshList, _("&Refresh"));
	configPageSz->Add(refreshButton, wxGBPosition(2, 2), wxGBSpan(1, 1), wxALIGN_CENTER | wxALL | wxEXPAND, 4);
	
	configPageSz->AddGrowableCol(1, 0);
	configPageSz->AddGrowableRow(1, 0);
	UpdateMissingModList();
}

bool ImportPackWizard::Start()
{
	if (RunWizard(packInfoPage))
	{
		// Install the mod pack.
		wxString instName;
		wxString instDirName;
		if (!m_mainWin->GetNewInstName(&instName, &instDirName, _("Import config pack")))
			return false;

		wxFileName instDir = wxFileName::DirName(Path::Combine(settings->GetInstDir(), instDirName));

		Instance *inst = new StdInstance(instDir);
		inst->SetName(instName);

		m_mainWin->AddInstance(inst);

		ModList *centralModList = m_mainWin->GetCentralModList();

		// Add jar mods...
		for (auto iter = m_pack->GetJarModList()->begin(); iter != m_pack->GetJarModList()->end(); ++iter)
		{
			Mod *mod = centralModList->FindByID(iter->m_id, iter->m_version);
			if (mod == nullptr)
			{
				wxLogError(_("Missing jar mod %s."), iter->m_id.c_str());
			}
			else
			{
				inst->GetModList()->InsertMod(inst->GetModList()->size(), mod->GetFileName().GetFullPath());
			}
		}

		// Add mod loader mods...
		for (auto iter = m_pack->GetMLModList()->begin(); iter != m_pack->GetMLModList()->end(); ++iter)
		{
			Mod *mod = centralModList->FindByID(iter->m_id, iter->m_version);
			if (mod == nullptr)
			{
				wxLogError(_("Missing modloader mod %s."), iter->m_id.c_str());
			}
			else
			{
				inst->GetMLModList()->InsertMod(0, mod->GetFileName().GetFullPath());
			}
		}

		// Add core mods...
		for (auto iter = m_pack->GetCoreModList()->begin(); iter != m_pack->GetCoreModList()->end(); ++iter)
		{
			Mod *mod = centralModList->FindByID(iter->m_id, iter->m_version);
			if (mod == nullptr)
			{
				wxLogError(_("Missing modloader mod %s."), iter->m_id.c_str());
			}
			else
			{
				inst->GetCoreModList()->InsertMod(0, mod->GetFileName().GetFullPath());
			}
		}
		
		// Extract config files
		wxFFileInputStream fileIn(m_pack->GetFileName());
		fsutils::ExtractZipArchive(fileIn, instDir.GetFullPath());

		return true;
	}
	return false;
}

void ImportPackWizard::UpdateMissingModList()
{
	missingModsList->Clear();

	m_mainWin->LoadCentralModList();
	ModList *centralModList = m_mainWin->GetCentralModList();

	for (auto iter = m_pack->GetJarModList()->begin(); iter != m_pack->GetJarModList()->end(); ++iter)
	{
		if (centralModList->FindByID(iter->m_id, iter->m_version) == nullptr)
		{
			missingModsList->Append(wxString::Format(_("%s %s"), iter->m_id.c_str(), iter->m_version.c_str()));
		}
	}

	for (auto iter = m_pack->GetMLModList()->begin(); iter != m_pack->GetMLModList()->end(); ++iter)
	{
		if (centralModList->FindByID(iter->m_id, iter->m_version) == nullptr)
		{
			missingModsList->Append(wxString::Format(_("%s %s"), iter->m_id.c_str(), iter->m_version.c_str()));
		}
	}
	
	for (auto iter = m_pack->GetCoreModList()->begin(); iter != m_pack->GetCoreModList()->end(); ++iter)
	{
		if (centralModList->FindByID(iter->m_id, iter->m_version) == nullptr)
		{
			missingModsList->Append(wxString::Format(_("%s %s"), iter->m_id.c_str(), iter->m_version.c_str()));
		}
	}
}

void ImportPackWizard::UpdateMissingModList(wxCommandEvent& event)
{
	UpdateMissingModList();
}

void ImportPackWizard::ViewFolderClicked(wxCommandEvent& event)
{
	Utils::OpenFolder(settings->GetModsDir());
}

BEGIN_EVENT_TABLE(ImportPackWizard, wxWizard)
	EVT_BUTTON(ID_RefreshList, ImportPackWizard::UpdateMissingModList)
	EVT_BUTTON(ID_ViewCentralModsFolder, ImportPackWizard::ViewFolderClicked)
END_EVENT_TABLE()