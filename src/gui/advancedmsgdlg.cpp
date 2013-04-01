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

#include "advancedmsgdlg.h"
#include <wx/gbsizer.h>
#include <wx/stattext.h>
#include <wx/button.h>

#include "utils/apputils.h"
#include "config.h"

AdvancedMsgDialog::AdvancedMsgDialog(wxWindow *parent, const wxString &msg, 
	ButtonDefList btns, const wxString &title)
	: wxDialog(parent, -1, title, wxDefaultPosition, wxDefaultSize)
{
	SetAprilFonts(this);

	wxBoxSizer *mainSz = new wxBoxSizer(wxVERTICAL);
	SetSizer(mainSz);

	wxStaticText *msgLabel = new wxStaticText(this, -1, msg);
	mainSz->Add(msgLabel, wxSizerFlags(1).Expand().
		Align(wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL).Border(wxTOP | wxLEFT | wxRIGHT, 8));

	wxBoxSizer *btnSz = new wxBoxSizer(wxHORIZONTAL);
	mainSz->Add(btnSz, wxSizerFlags().Align(wxALIGN_RIGHT).Border(wxALL, 8));

	for (ButtonDefList::iterator iter = btns.begin(); iter != btns.end(); iter++)
	{
		wxButton *btn = new wxButton(this, iter->m_id, iter->m_text);
		btnSz->Add(btn, wxSizerFlags().Align(wxALIGN_RIGHT).Border(wxLEFT | wxRIGHT, 4));
		Bind(wxEVT_COMMAND_BUTTON_CLICKED, &AdvancedMsgDialog::OnButtonClicked, this);
	}

	Fit();
}

void AdvancedMsgDialog::OnButtonClicked(wxCommandEvent &event)
{
	EndModal(event.GetId());
}

AdvancedMsgDialog::ButtonDef::ButtonDef(wxString text, int id)
{
	m_text = text;
	m_id = id;
}

BEGIN_EVENT_TABLE(AdvancedMsgDialog, wxDialog)
END_EVENT_TABLE()
