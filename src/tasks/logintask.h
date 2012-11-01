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
#include "task.h"
#include "logindialog.h"

#include <wx/string.h>

#include "instance.h"

// Stores a response from login.minecraft.net
struct LoginResult
{
	LoginResult(wxString loginResponse = wxEmptyString);
	LoginResult(const wxString username, 
				const wxString sessionID, 
				const wxString downloadTicket, 
				const int64_t latestVersion,
				bool loginFailed = false,
				bool playOffline = false,
				bool forceUpdate = false
			);
	LoginResult(const LoginResult *result);

	static LoginResult PlayOffline(const wxString username);
	
	bool loginFailed;
	bool playOffline;
	bool forceUpdate;
	wxString errorMessage;
	/// User login
	wxString username;
	/// ID of this Minecraft session
	wxString sessionID;
	/// Deprecated
	wxString downloadTicket;
	/// A unix timestamp of the latest Minecraft version from Mojang
	int64_t latestVersion;
	/// User ID?
	wxString UID;
};

class LoginTask : public Task
{
public:
	LoginTask(UserInfo& uInfo, Instance *inst, bool forceUpdate);
	
	Instance *m_inst;
	bool m_forceUpdate;
	const LoginResult & GetLoginResult()
	{
		return m_result;
	};
	
protected:
	virtual ExitCode TaskStart();
	
	virtual void SetLoginResult(LoginResult result);
	
	UserInfo m_userInfo;
	LoginResult m_result;
};
