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

#include "logindialog.h"
#include <wx/gbsizer.h>

LoginDialog::LoginDialog (wxWindow *parent, wxString errorMsg, UserInfo info)
	: wxDialog(parent, -1, _T("Login"), wxDefaultPosition, wxSize(520, 140))
{
	wxGridBagSizer *mainBox = new wxGridBagSizer();
	int offset = (errorMsg.empty() ? 0 : 1);
	const int cols = 3;
	const int padding = 4;
	
	// Error message label
	if (!errorMsg.empty())
	{
		wxStaticText *errorMsgLabel = new wxStaticText(this, -1, errorMsg);
		mainBox->Add(errorMsgLabel, wxGBPosition(0, 0), wxGBSpan(1, cols), 
					 wxTOP | wxLEFT | wxALIGN_CENTER_VERTICAL | wxALIGN_LEFT, padding);
	}
	
	// Username text field
	wxStaticText *usernameLabel = new wxStaticText(this, -1, _T("Username: "));
	mainBox->Add(usernameLabel, wxGBPosition(0 + offset, 0), wxGBSpan(1, 1), 
				 wxALL | wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT, padding);
	
	usernameTextBox = new wxTextCtrl(this, ID_USERNAME_TEXTCTRL, info.username,
		wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
	mainBox->Add(usernameTextBox, wxGBPosition(0 + offset, 1), wxGBSpan(1, cols - 1), 
				 wxALL | wxEXPAND, padding);
	
	
	// Password text field
	wxStaticText *passwordLabel = new wxStaticText(this, -1, _T("Password: "));
	mainBox->Add(passwordLabel, wxGBPosition(1 + offset, 0), wxGBSpan(1, 1), 
		wxALL | wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT, padding);
	
	passwordTextBox = new wxTextCtrl(this, ID_PASSWORD_TEXTCTRL, info.password, 
		wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER | wxTE_PASSWORD);
	mainBox->Add(passwordTextBox, wxGBPosition(1 + offset, 1), wxGBSpan(1, cols - 1), 
		wxALL | wxEXPAND, padding);
	
	
	// Checkboxes
	forceUpdateToggle = new wxToggleButton(this, -1, _T("&Force update"));
	mainBox->Add(forceUpdateToggle, wxGBPosition(2 + offset, 0), wxGBSpan(1, 1), wxALL, padding);
	
	rememberUsernameCheck = new wxCheckBox(this, -1, _T("&Remember username?"));
	rememberUsernameCheck->SetValue(info.rememberUsername);
	mainBox->Add(rememberUsernameCheck, wxGBPosition(2 + offset, 1), wxGBSpan(1, 1), wxALL, padding);
	
	rememberPasswordCheck = new wxCheckBox(this, -1, _T("R&emember password?"));
	rememberPasswordCheck->SetValue(info.rememberPassword);
	mainBox->Add(rememberPasswordCheck, wxGBPosition(2 + offset, 2), wxGBSpan(1, 1), wxALL, padding);
	
	wxBoxSizer *playOfflineSizer = new wxBoxSizer(wxHORIZONTAL);
	playOfflineButton = new wxButton(this, ID_PLAY_OFFLINE, _("Play &Offline"));
	playOfflineSizer->Add(playOfflineButton, 1, wxTOP | wxBOTTOM | wxEXPAND, 8);
	mainBox->Add(playOfflineSizer, wxGBPosition(3 + offset, 0), wxGBSpan(1, 1), 
		wxLEFT | wxRIGHT | wxEXPAND, padding);
	
	wxSizer *btnBox = CreateButtonSizer(wxOK | wxCANCEL);
	mainBox->Add(btnBox, wxGBPosition(3 + offset, 1), wxGBSpan(1, cols - 1), 
		wxRIGHT | wxTOP | wxBOTTOM | wxEXPAND, 8);
	
	usernameTextBox->SetFocus();
	
	SetSizer(mainBox);
	mainBox->SetSizeHints(this);
}

LoginDialog::~LoginDialog()
{
	
}

void LoginDialog::OnUsernameEnter(wxCommandEvent& event)
{
	if (passwordTextBox->GetValue().empty())
	{
		passwordTextBox->SetFocus();
	}
	else
	{
		EndModal(GetAffirmativeId());
	}
}

void LoginDialog::OnPasswordEnter(wxCommandEvent& event)
{
	if (usernameTextBox->GetValue().empty())
	{
		usernameTextBox->SetFocus();
	}
	else
	{
		EndModal(GetAffirmativeId());
	}
}

wxString LoginDialog::GetUsername() const
{
	wxString username = usernameTextBox->GetValue();
	return username.Left(username.First(wxT(':')));
}

void LoginDialog::SetUsername(wxString& username)
{
	usernameTextBox->SetValue(username);
}

wxString LoginDialog::GetPassword() const
{
	return passwordTextBox->GetValue();
}

void LoginDialog::SetPassword(wxString& password)
{
	passwordTextBox->SetValue(password);
}

bool LoginDialog::GetRememberUsername() const
{
	return rememberUsernameCheck->GetValue();
}

void LoginDialog::SetRememberUsername(bool rememberUsername)
{
	rememberUsernameCheck->SetValue(rememberUsername);
}

bool LoginDialog::GetRememberPassword() const
{
	return rememberPasswordCheck->GetValue();
}

void LoginDialog::SetRememberPassword(bool rememberPassword)
{
	rememberPasswordCheck->SetValue(rememberPassword);
}

void LoginDialog::SetForceUpdate(bool value)
{
	forceUpdateToggle->SetValue(value);
}

bool LoginDialog::ShouldForceUpdate() const
{
	return forceUpdateToggle->GetValue();
}

void LoginDialog::OnPlayOffline(wxCommandEvent &event)
{
	EndModal(ID_PLAY_OFFLINE);
}

BEGIN_EVENT_TABLE(LoginDialog, wxDialog)
	EVT_TEXT_ENTER(ID_USERNAME_TEXTCTRL, LoginDialog::OnUsernameEnter)
	EVT_TEXT_ENTER(ID_PASSWORD_TEXTCTRL, LoginDialog::OnPasswordEnter)

	EVT_BUTTON(ID_PLAY_OFFLINE, LoginDialog::OnPlayOffline)
END_EVENT_TABLE()
