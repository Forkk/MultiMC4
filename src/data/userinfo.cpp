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
	/*
	for (size_t i = 0; i < outString.Length(); i++)
	{
		short c = outString[i];
		outString[i] = -c;
	}
	*/
	wxStringInputStream stringInput(outString);
	output.Write(stringInput);
}

void UserInfo::LoadFromStream(wxInputStream &input)
{
	wxString inString;
	wxStringOutputStream outStream(&inString);
	outStream.Write(input);
	/*
	for (size_t i = 0; i < inString.Length(); i++)
	{
		// the short here is a HACK to make it work with linux builds.
		// wxString uses UTF-16 internally, but on linux, wchar_t is bigger, making the kind of nasty crap
		// this code does to strings not break the strings (unused, bad numbers are trimmed away)
		wxChar c = inString[i];
		inString[i] = short(-c);
	}
	*/
	if (inString.Length() == 0)
		return;
	rememberUsername = true;
	int split_pos = inString.Find(L'=',true);
	if ( split_pos != wxString::npos)
	{
		username = inString.SubString(0, split_pos - 1);
		password = inString.Mid(split_pos + 1);
		rememberPassword = true;
	}
	else
	{
		username = inString;
	}
}
