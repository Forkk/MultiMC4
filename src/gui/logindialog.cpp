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

BEGIN_EVENT_TABLE(LoginDialog, wxDialog)
	
END_EVENT_TABLE()

LoginDialog::LoginDialog (wxWindow* parent, wxString errorMsg)
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
	
	usernameTextBox = new wxTextCtrl(this, -1);
	mainBox->Add(usernameTextBox, wxGBPosition(0 + offset, 1), wxGBSpan(1, cols - 1), 
				 wxALL | wxEXPAND, padding);
	
	
	// Password text field
	wxStaticText *passwordLabel = new wxStaticText(this, -1, _T("Password: "));
	mainBox->Add(passwordLabel, wxGBPosition(1 + offset, 0), wxGBSpan(1, 1), 
				 wxALL | wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT, padding);
	
	passwordTextBox = new wxTextCtrl(this, -1, wxEmptyString, wxDefaultPosition, 
									 wxDefaultSize, wxTE_PASSWORD);
	mainBox->Add(passwordTextBox, wxGBPosition(1 + offset, 1), wxGBSpan(1, cols - 1), 
				 wxALL | wxEXPAND, padding);
	
	
	// Checkboxes
	forceUpdateToggle = new wxToggleButton(this, -1, _T("&Force update"));
	mainBox->Add(forceUpdateToggle, wxGBPosition(2 + offset, 0), wxGBSpan(1, 1), wxALL, padding);
	
	rememberUsernameCheck = new wxCheckBox(this, -1, _T("&Remember username?"));
	mainBox->Add(rememberUsernameCheck, wxGBPosition(2 + offset, 1), wxGBSpan(1, 1), wxALL, padding);
	
	rememberPasswordCheck = new wxCheckBox(this, -1, _T("&Remember password?"));
	mainBox->Add(rememberPasswordCheck, wxGBPosition(2 + offset, 2), wxGBSpan(1, 1), wxALL, padding);
	
	
	wxSizer *btnBox = CreateButtonSizer(wxOK | wxCANCEL);
	mainBox->Add(btnBox, wxGBPosition(3 + offset, 0), wxGBSpan(1, cols), 
				 wxBOTTOM | wxRIGHT | wxALIGN_RIGHT, 8);
	
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
		usernameTextBox->SetFocus();
	}
	else
	{
		
	}
}

void LoginDialog::OnPasswordEnter(wxCommandEvent& event)
{
	if (usernameTextBox->GetValue().empty())
	{
		passwordTextBox->SetFocus();
	}
	else
	{
		
	}
}

void LoginDialog::OnLoginClicked(wxCommandEvent& event)
{
	
}

wxString LoginDialog::GetUsername()
{
	return usernameTextBox->GetValue();
}

void LoginDialog::SetUsername(wxString& username)
{
	usernameTextBox->SetValue(username);
}

wxString LoginDialog::GetPassword()
{
	return passwordTextBox->GetValue();
}

void LoginDialog::SetPassword(wxString& password)
{
	passwordTextBox->SetValue(password);
}

bool LoginDialog::GetRememberUsername()
{
	return rememberUsernameCheck->GetValue();
}

void LoginDialog::SetRememberUsername(bool rememberUsername)
{
	rememberUsernameCheck->SetValue(rememberUsername);
}

bool LoginDialog::GetRememberPassword()
{
	return rememberPasswordCheck->GetValue();
}

void LoginDialog::SetRememberPassword(bool rememberPassword)
{
	rememberPasswordCheck->SetValue(rememberPassword);
}

UserInfo::UserInfo()
{
	this->username = wxEmptyString;
	this->password = wxEmptyString;
	this->rememberUsername = false;
	this->rememberPassword = false;
}

UserInfo::UserInfo(LoginDialog& loginDlg)
{
	this->username = loginDlg.GetUsername();
	this->password = loginDlg.GetPassword();
	this->rememberUsername = loginDlg.GetRememberUsername();
	this->rememberPassword = loginDlg.GetRememberPassword();
}

UserInfo::UserInfo(UserInfo& uInfo)
{
	this->username = uInfo.username;
	this->password = uInfo.password;
	this->rememberUsername = uInfo.rememberUsername;
	this->rememberPassword = uInfo.rememberPassword;
}

UserInfo::UserInfo(wxString& username, wxString& password, bool rememberUsername, bool rememberPassword)
{
	this->username = username;
	this->password = password;
	this->rememberUsername = rememberUsername;
	this->rememberPassword = rememberPassword;
}


