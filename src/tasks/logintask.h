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

#pragma once

#include "task.h"
#include <logindialog.h>

#include <wx/wx.h>

DECLARE_EVENT_TYPE(wxEVT_LOGIN_COMPLETE, -1)

// Stores a response from login.minecraft.net
struct LoginResult
{
	LoginResult(wxString loginResponse = wxEmptyString);
	LoginResult(const LoginResult *result);
	
	bool loginFailed;
	wxString errorMessage;
	
	wxString username;
	wxString sessionID;
	wxString downloadTicket;
	wxString latestVersion;
};

class LoginTask : public Task
{
public:
	LoginTask(UserInfo& uInfo);
	
protected:
	virtual void TaskStart();
	
	virtual void OnLoginComplete(LoginResult result);
	
	UserInfo m_userInfo;
};

struct LoginCompleteEvent : TaskEvent
{
	LoginCompleteEvent(const Task* task, const LoginResult &loginResult) 
		: TaskEvent(wxEVT_LOGIN_COMPLETE, task) { m_loginResult = LoginResult(loginResult); }
	
	LoginResult m_loginResult;
	
	virtual wxEvent *Clone() const
	{
		return new LoginCompleteEvent(m_task, m_loginResult);
	}
};

typedef void (wxEvtHandler::*LoginCompleteEventFunction)(LoginCompleteEvent&);

#define EVT_LOGIN_COMPLETE(fn) EVT_TASK_CUSTOM(wxEVT_LOGIN_COMPLETE, fn, LoginCompleteEventFunction)
