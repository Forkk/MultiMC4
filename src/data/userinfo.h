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

#pragma once

#include <wx/wx.h>

class LoginDialog;

struct UserInfo
{
	UserInfo()
	{
		this->username = wxEmptyString;
		this->password = wxEmptyString;
		this->rememberUsername = false;
		this->rememberPassword = false;
	}

	UserInfo(LoginDialog &loginDlg);
	UserInfo(UserInfo &uInfo);
	UserInfo(wxString &username, wxString &password, 
		bool rememberUsername, bool rememberPassword);
	
	wxString username;
	wxString password;
	bool rememberUsername;
	bool rememberPassword;
	
	void SaveToFile(const char * filename) const;
	void LoadFromFile(const char * filename);
};

class UserInfoList
{
	
};
