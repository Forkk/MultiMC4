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

#include "updatepromptdlg.h"
#include <wx/gbsizer.h>
#include <wx/stattext.h>
#include <wx/button.h>

#include "apputils.h"
#include "config.h"

enum
{
	ID_Changelog,
};

UpdatePromptDialog::UpdatePromptDialog(wxWindow *parent, const wxString &updateMsg)
	: wxDialog(parent, -1, _("Update Avaliable"), wxDefaultPosition, wxDefaultSize)
{
	wxBoxSizer *mainSz = new wxBoxSizer(wxVERTICAL);
	SetSizer(mainSz);

	wxStaticText *msgLabel = new wxStaticText(this, -1, updateMsg);
	mainSz->Add(msgLabel, wxSizerFlags(1).Expand().
		Align(wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL).Border(wxTOP | wxLEFT | wxRIGHT, 8));

	wxSizer *btnSz = CreateButtonSizer(wxOK | wxCANCEL);

	wxButton *changelogBtn = new wxButton(this, ID_Changelog, _("&Changelog"));
	btnSz->Insert(0, changelogBtn, wxSizerFlags().Align(wxALIGN_RIGHT).Border(wxLEFT | wxRIGHT, 4));

	mainSz->Add(btnSz, wxSizerFlags().Align(wxALIGN_RIGHT).Border(wxBOTTOM | wxRIGHT | wxTOP, 8));

	Fit();
}

void UpdatePromptDialog::OnChangelogClicked(wxCommandEvent &event)
{
	wxString url = wxT(JENKINS_JOB_URL);
	Utils::OpenURL(url.Append(wxT("changes")));
}

BEGIN_EVENT_TABLE(UpdatePromptDialog, wxDialog)
	EVT_BUTTON(ID_Changelog, UpdatePromptDialog::OnChangelogClicked)
END_EVENT_TABLE()
