/*
    Copyright 2012 Andrew Okin

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/


#include "logintask.h"
#include <wx/tokenzr.h>
#include <wx/url.h>
#include <wx/sstream.h>

DEFINE_EVENT_TYPE(wxEVT_LOGIN_COMPLETE)

LoginTask::LoginTask(UserInfo &uInfo)
{
	m_userInfo = uInfo;
}

void LoginTask::TaskStart()
{
	// Get http://login.minecraft.net/?username=<username>&password=<password>&version=1337
	wxURL loginURL = _("http://login.minecraft.net/?user=") + m_userInfo.username + 
		_("&password=") + m_userInfo.password + _("&version=1337");
	
	if (loginURL.GetError() == wxURL_NOERR)
	{
		wxInputStream *inputStream = loginURL.GetInputStream();
		
		wxString response;
		wxStringOutputStream outStream(&response);
		outStream.Write(*inputStream);
		
		wxDELETE(inputStream);
		
		OnLoginComplete(response);
	}
	else
	{
		switch (loginURL.GetError())
		{
		case wxURL_CONNERR:
			OnLoginComplete(LoginResult(_("Can't connect to login.minecraft.net.")));
			break;
			
		default:
			OnLoginComplete(LoginResult(_("An unknown error occurred.")));
			break;
		}
	}
}

void LoginTask::OnLoginComplete(LoginResult result)
{
	LoginCompleteEvent event(this, new LoginResult(result));
	m_evtHandler->AddPendingEvent(event);
}

LoginResult::LoginResult(wxString loginResponse)
{
	loginFailed = false;
	errorMessage = wxEmptyString;
	
	latestVersion = wxEmptyString;
	downloadTicket = wxEmptyString;
	username = wxEmptyString;
	sessionID = wxEmptyString;
	
	wxArrayString strings = wxStringTokenize(loginResponse, _(":"));
	if (strings.Count() >= 4)
	{
		// Login succeeded
		latestVersion = strings[0];
		downloadTicket = strings[1];
		username = strings[2];
		sessionID = strings[3];
	}
	else
	{
		// Login failed
		loginFailed = true;
		
		if (loginResponse.Lower() == _("bad login"))
			errorMessage = _T("Invalid username or password.");
		else if (loginResponse.Lower() == _("old version"))
			errorMessage = _T("Launcher outdated, please update.");
		else
			errorMessage = _("Login failed: ") + loginResponse;
	}
}

LoginResult::LoginResult(const LoginResult *result)
{
	this->downloadTicket = result->downloadTicket;
	this->latestVersion = result->latestVersion;
	this->errorMessage = result->errorMessage;
	this->loginFailed = result->loginFailed;
	this->sessionID = result->sessionID;
	this->username = result->username;
}
