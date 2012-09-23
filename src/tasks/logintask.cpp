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

#include "logintask.h"
#include <apputils.h>
#include <wx/tokenzr.h>
#include <wx/url.h>
#include <wx/sstream.h>
#include <sstream>
#include "curlutils.h"

#include "config.h"

LoginTask::LoginTask(UserInfo& uInfo, Instance* inst, bool forceUpdate)
	: Task()
{
	m_userInfo = uInfo;
	m_inst = inst;
	m_forceUpdate = forceUpdate;
}

wxThread::ExitCode LoginTask::TaskStart()
{
	SetStatus(_("Logging in..."));

	CURL *curl = curl_easy_init();
	// Get http://login.minecraft.net/?username=<username>&password=<password>&version=1337
	wxCharBuffer login = m_userInfo.username.ToUTF8();
	wxCharBuffer passwd = m_userInfo.password.ToUTF8();
	std::ostringstream sst;
	char * encodedLogin = curl_easy_escape(curl,login.data(), strlen(login));
	char * encodedPasswd = curl_easy_escape(curl,passwd.data(), strlen(passwd));
#ifndef MSVC
#if USE_HTTPS == true
	sst << "https";
#else
	sst << "http";
#endif
#else
	sst << "http";
#endif
	sst << "://login.minecraft.net/?user=" << encodedLogin << "&password=" << encodedPasswd << "&version=1337";
	char errorBuffer[CURL_ERROR_SIZE];
	
	
	curl_easy_setopt(curl, CURLOPT_URL, sst.str().c_str());
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlLambdaCallback);
	curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, &errorBuffer);
	
	wxString outString;
	wxStringOutputStream outStream(&outString);
	CurlLambdaCallbackFunction curlWrite = [&] (void *buffer, size_t size) -> size_t
	{
		outStream.Write(buffer, size);
		return outStream.LastWrite();
	};
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &curlWrite);
	
	int status = curl_easy_perform(curl);
	
	long response = 0;
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response);
	
	curl_easy_cleanup(curl);
	
	if (status != 0)
	{
		SetLoginResult(wxStr(errorBuffer));
		return (void *)0;
	}
	
	if (response == 200)
	{
		SetLoginResult(outString);
		return (void *)1;
	}
	else if (response == 503)
	{
		SetLoginResult(wxString::Format(_("503 - login servers unavailable. Check help.mojang.com!")));
		return (void *)0;
	}
	else
	{
		SetLoginResult(wxString::Format(_("Unknown HTTP error %i occurred."), response));
		return (void *)0;
	}
}

void LoginTask::SetLoginResult(LoginResult result)
{
	SetProgress(100);
	m_result = new LoginResult(result);
	m_result.forceUpdate = m_forceUpdate;
}

LoginResult::LoginResult(wxString loginResponse)
{
	loginFailed = false;
	playOffline = false;
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

LoginResult::LoginResult(const wxString username, 
						 const wxString sessionID, 
						 const wxString downloadTicket, 
						 const wxString latestVersion,
						 bool loginFailed,
						 bool playOffline,
						 bool forceUpdate)
{
	this->username = username;
	this->sessionID = sessionID;
	this->downloadTicket = downloadTicket;
	this->latestVersion = latestVersion;
	this->loginFailed = loginFailed;
	this->playOffline = playOffline;
	this->forceUpdate = forceUpdate;
}

LoginResult::LoginResult(const LoginResult *result)
{
	this->downloadTicket = result->downloadTicket;
	this->latestVersion = result->latestVersion;
	this->errorMessage = result->errorMessage;
	this->playOffline = result->playOffline;
	this->loginFailed = result->loginFailed;
	this->sessionID = result->sessionID;
	this->username = result->username;
	this->forceUpdate = result->forceUpdate;
}

LoginResult LoginResult::PlayOffline(const wxString username)
{
	return LoginResult(username, wxEmptyString, wxEmptyString, wxEmptyString, false, true, false);
}
