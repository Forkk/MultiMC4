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

#include "textdisplaydialog.h"

#include <wx/gbsizer.h>

TextDisplayDialog::TextDisplayDialog(wxWindow *parent, wxString text, wxString title)
	: wxDialog(parent, wxID_ANY, title, wxDefaultPosition, wxSize(520, 420))
{
	wxBoxSizer *dlgSizer = new wxBoxSizer(wxVERTICAL);
	SetSizer(dlgSizer);

	textBox = new wxTextCtrl(this, -1, text, wxDefaultPosition, wxDefaultSize, 
		wxTE_MULTILINE | wxTE_RICH | wxTE_READONLY);
	dlgSizer->Add(textBox, wxSizerFlags(1).Border(wxALL, 8).Expand());

	wxSizer *btnSz = CreateButtonSizer(wxOK);
	dlgSizer->Add(btnSz, wxSizerFlags(0).Border(wxBOTTOM | wxRIGHT, 8).
		Align(wxALIGN_RIGHT | wxALIGN_BOTTOM));

	CenterOnParent();
}
