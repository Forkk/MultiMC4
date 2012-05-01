/*
    Copyright 2012 <copyright holder> <email>

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

LoginTask::LoginTask(UserInfo &uInfo)
{
	m_userInfo = uInfo;
}

void LoginTask::TaskStart()
{
	// Get http://login.minecraft.net/?username=<username>&password=<password>&version=1337
	wxString loginURI = wxString::Format(
		_("http://login.minecraft.net/?username=%s&password=%s&version=1337"), 
		stdStr(m_userInfo.username).c_str(), stdStr(m_userInfo.password).c_str());
	
	
}

