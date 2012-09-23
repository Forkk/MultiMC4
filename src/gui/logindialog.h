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

#pragma once
#include <wx/wx.h>
#include <wx/tglbtn.h>

#include "instance.h"
#include "userinfo.h"

class LoginDialog : public wxDialog
{
public:
	LoginDialog(wxWindow* parent, wxString errorMsg = _(""), UserInfo info = UserInfo(), bool canGoOffline = false);
	~LoginDialog();
	
	void OnUsernameEnter(wxCommandEvent &event);
	void OnPasswordEnter(wxCommandEvent &event);
	
	wxString GetUsername() const;
	void SetUsername(wxString &username);
	
	wxString GetPassword() const;
	void SetPassword(wxString &password);
	
	bool GetRememberUsername() const;
	void SetRememberUsername(bool rememberUsername);
	
	bool GetRememberPassword() const;
	void SetRememberPassword(bool rememberPassword);
	
	bool ShouldForceUpdate() const;
	void SetForceUpdate(bool value);
	
	DECLARE_EVENT_TABLE()

protected:
	wxTextCtrl *usernameTextBox;
	wxTextCtrl *passwordTextBox;
	
	wxCheckBox *rememberUsernameCheck;
	wxCheckBox *rememberPasswordCheck;
	
	wxButton *playOfflineButton;
	
	wxToggleButton *forceUpdateToggle;

	void OnPlayOffline(wxCommandEvent &event);
};

enum
{
	ID_USERNAME_TEXTCTRL = 1,
	ID_PASSWORD_TEXTCTRL,

	ID_PLAY_OFFLINE,
};
