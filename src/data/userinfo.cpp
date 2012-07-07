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

#include "userinfo.h"
#include "logindialog.h"

#include <wx/sstream.h>

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

void UserInfo::SaveToStream(wxOutputStream &output) const
{
	wxString outString;
	if (rememberUsername)
	{
		outString.Append(username);
		if (rememberPassword)
			outString.Append(_("=") + password);
	}
	
	for (size_t i = 0; i < outString.Length(); i++)
	{
		outString[i] = -outString[i];
	}
	
	wxStringInputStream stringInput(outString);
	output.Write(stringInput);
}

void UserInfo::LoadFromStream(wxInputStream &input)
{
	wxString inString;
	wxStringOutputStream outStream(&inString);
	outStream.Write(input);
	
	for (size_t i = 0; i < inString.Length(); i++)
	{
		inString[i] = -inString[i];
	}
	
	if (inString.Length() == 0)
		return;
	
	username = inString.SubString(0, inString.Last('=') - 1);
	rememberUsername = true;
	
	if (inString.Last('=') != wxString::npos)
	{
		password = inString.Mid(inString.Last('=') + 1);
		rememberPassword = true;
	}
}
