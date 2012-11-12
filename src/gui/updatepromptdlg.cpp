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

#include "updatepromptdlg.h"
#include <wx/gbsizer.h>
#include <wx/stattext.h>
#include <wx/button.h>

#include "apputils.h"
#include "config.h"

UpdatePromptDialog::UpdatePromptDialog(wxWindow *parent, const wxString &updateMsg)
	: wxDialog(parent, -1, _("Update Available"), wxDefaultPosition, wxDefaultSize)
{
	wxBoxSizer *mainSz = new wxBoxSizer(wxVERTICAL);
	SetSizer(mainSz);

	wxStaticText *msgLabel = new wxStaticText(this, -1, updateMsg);
	mainSz->Add(msgLabel, wxSizerFlags(1).Expand().
		Align(wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL).Border(wxTOP | wxLEFT | wxRIGHT, 8));

	wxBoxSizer *btnSz = new wxBoxSizer(wxHORIZONTAL);

	wxButton *changelogBtn = new wxButton(this, ID_Changelog, _("&Changelog"));
	btnSz->Add(changelogBtn, wxSizerFlags().Align(wxALIGN_LEFT).Border(wxLEFT | wxRIGHT, 4));

	btnSz->AddStretchSpacer();

	wxButton* cancelButton = new wxButton(this, wxID_CANCEL, _("Cancel"));
	btnSz->Add(cancelButton);

	wxButton* updateLaterBtn = new wxButton(this, ID_UpdateLater, _("When MultiMC Exits"));
	btnSz->Add(updateLaterBtn, wxSizerFlags().Align(wxALIGN_RIGHT).Border(wxLEFT | wxRIGHT, 4));

	wxButton* updateNowBtn = new wxButton(this, ID_UpdateNow, _("Update Now"));
	btnSz->Add(updateNowBtn, wxSizerFlags().Align(wxALIGN_RIGHT).Border(wxLEFT | wxRIGHT, 4));

	mainSz->Add(btnSz, wxSizerFlags().Align(wxALIGN_RIGHT).Border(wxBOTTOM | wxRIGHT | wxLEFT | wxTOP, 8));

	SetAffirmativeId(ID_UpdateNow);
	updateNowBtn->SetFocus();

	Fit();
}

void UpdatePromptDialog::OnChangelogClicked(wxCommandEvent &event)
{
	wxString url = wxT(JENKINS_JOB_URL);
	Utils::OpenURL(url.Append(wxT("changes")));
}

void UpdatePromptDialog::OnUpdateNowClicked(wxCommandEvent& event)
{
	EndModal(ID_UpdateNow);
}

void UpdatePromptDialog::OnUpdateLaterClicked(wxCommandEvent& event)
{
	EndModal(ID_UpdateLater);
}

BEGIN_EVENT_TABLE(UpdatePromptDialog, wxDialog)
	EVT_BUTTON(ID_Changelog, UpdatePromptDialog::OnChangelogClicked)
	EVT_BUTTON(ID_UpdateNow, UpdatePromptDialog::OnUpdateNowClicked)
	EVT_BUTTON(ID_UpdateLater, UpdatePromptDialog::OnUpdateLaterClicked)
END_EVENT_TABLE()
