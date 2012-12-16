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

#include "exportinstwizard.h"

#include <wx/gbsizer.h>

ExportInstWizard::ExportInstWizard(wxWindow *parent, Instance *inst)
	: wxWizard(parent, -1, _("Export Instance"))
{
	this->m_inst = inst;

	this->SetPageSize(wxSize(400, 300));

	wxFont titleFont(12, wxSWISS, wxNORMAL, wxNORMAL);

	enterInfoPage = new wxWizardPageSimple(this);
	wxGridBagSizer *infoPageSz = new wxGridBagSizer();
	enterInfoPage->SetSizer(infoPageSz);

	wxStaticText *infoTitleLabel = new wxStaticText(enterInfoPage, -1, _("Config Pack Info"), wxDefaultPosition, wxDefaultSize);
	infoTitleLabel->SetFont(titleFont);
	infoPageSz->Add(infoTitleLabel, wxGBPosition(0, 0), wxGBSpan(1, 2), wxALIGN_CENTER | wxALL, 4);

	wxStaticText *nameLabel = new wxStaticText(enterInfoPage, -1, _("Name: "));
	infoPageSz->Add(nameLabel, wxGBPosition(1, 0), wxGBSpan(1, 1), wxALL | wxALIGN_CENTER_VERTICAL, 4);

	packNameTextbox = new wxTextCtrl(enterInfoPage, -1, inst->GetName());
	infoPageSz->Add(packNameTextbox, wxGBPosition(1, 1), wxGBSpan(1, 1), wxEXPAND | wxALIGN_CENTER_VERTICAL | wxALL, 4);

	wxStaticText *notesLabel = new wxStaticText(enterInfoPage, -1, _("Notes: "));
	infoPageSz->Add(notesLabel, wxGBPosition(2, 0), wxGBSpan(1, 2), wxALL | wxALIGN_CENTER_VERTICAL, 4);

	packNotesTextbox = new wxTextCtrl(enterInfoPage, -1, inst->GetNotes(), wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE);
	infoPageSz->Add(packNotesTextbox, wxGBPosition(3, 0), wxGBSpan(1, 2), wxEXPAND | wxALL, 4);
	infoPageSz->AddGrowableCol(1);
	infoPageSz->AddGrowableRow(3);


	chooseConfigFilesPage = new wxWizardPageSimple(this, enterInfoPage);
	enterInfoPage->SetNext(chooseConfigFilesPage);
	wxGridBagSizer *configPageSz = new wxGridBagSizer();
	configPageSz->AddGrowableCol(0, 0);
	chooseConfigFilesPage->SetSizer(configPageSz);

	wxStaticText *configTitleLabel = new wxStaticText(chooseConfigFilesPage, -1, _("Choose Included Configs"), 
		wxDefaultPosition, wxDefaultSize);
	configTitleLabel->SetFont(titleFont);
	configPageSz->Add(configTitleLabel, wxGBPosition(0, 0), wxGBSpan(1, 1), wxALIGN_CENTER | wxALL, 4);

	wxArrayString configList;
	inst->GetPossibleConfigFiles(&configList);
	cfgListCtrl = new wxCheckListBox(chooseConfigFilesPage, -1,
		wxDefaultPosition, wxDefaultSize, configList);
	for (unsigned i = 0; i < cfgListCtrl->GetCount(); i++)
	{
		if (!cfgListCtrl->GetString(i).Contains("options.txt") &&
			!cfgListCtrl->GetString(i).Contains("optionsof.txt"))
				cfgListCtrl->Check(i, true);
	}
	configPageSz->Add(cfgListCtrl, wxGBPosition(1, 0), wxGBSpan(1, 1), wxALIGN_CENTER | wxALL | wxEXPAND, 4);
	configPageSz->AddGrowableRow(1, 0);
}

bool ExportInstWizard::Start()
{
	return RunWizard(enterInfoPage);
}

wxString ExportInstWizard::GetPackName()
{
	return packNameTextbox->GetValue();
}

wxString ExportInstWizard::GetPackNotes()
{
	return packNotesTextbox->GetValue();
}

void ExportInstWizard::GetIncludedConfigs(wxArrayString *out)
{
	for (unsigned i = 0; i < cfgListCtrl->GetCount(); i++)
	{
		if (cfgListCtrl->IsChecked(i))
		{
			wxFileName cfgFile(cfgListCtrl->GetString(i));
			cfgFile.Normalize(wxPATH_NORM_ALL, m_inst->GetRootDir().GetFullPath());
			cfgFile.MakeRelativeTo();
			out->Add(cfgFile.GetFullPath());
		}
	}
}
