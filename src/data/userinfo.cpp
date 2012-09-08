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
#include <stdio.h>

#include <wx/sstream.h>

UserInfo::UserInfo(LoginDialog& loginDlg)
{
	this->username = loginDlg.GetUsername();
	this->password = loginDlg.GetPassword();
	this->rememberUsername = loginDlg.GetRememberUsername();
	this->rememberPassword = loginDlg.GetRememberPassword();
}

UserInfo::UserInfo(const UserInfo& uInfo)
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

void UserInfo::SaveToFile(const char * filename) const
{
	wxString outString;
	if (rememberUsername)
	{
		outString.Append(username);
		if (rememberPassword)
			outString.Append(_("=") + password);
	}
	FILE* passfile = 0;
	passfile = fopen(filename,"wb");
	if(!passfile)
		return;
	
	auto utf8_data = outString.ToUTF8();
	for(size_t i = 0; utf8_data[i] != 0; i++)
	{
		short datapt = utf8_data[i];
		datapt = -datapt;
		fwrite(&datapt,sizeof(short),1,passfile);
	}
	fflush(passfile);
	fclose(passfile);
}

void UserInfo::LoadFromFile(const char * filename)
{
	FILE* passfile = 0;
	passfile = fopen(filename,"rb");
	if(!passfile)
		return;
	std::string utf8_str;
	while (!feof(passfile))
	{
		short datapt = 0;
		if(fread(&datapt,sizeof(short),1,passfile) != 1)
			break;
		datapt = -datapt;
		utf8_str.append(1, (char)datapt);
	}
	if (utf8_str.size() == 0)
		return;
	fclose(passfile);
	wxString inString = wxString::FromUTF8(utf8_str.c_str(),utf8_str.size());
	rememberUsername = true;
	int split_pos = inString.Find(wxT('='),true);
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
