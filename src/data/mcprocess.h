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

#include <wx/process.h>

class InstConsoleWindow;
class Instance;

class MinecraftProcess : public wxProcess
{
public:
	static wxProcess *Launch(Instance * source, InstConsoleWindow* parent, wxString username, wxString sessionID);

public:
	bool ProcessInput();
	void KillMinecraft();
	bool AlreadyKilled() const
	{
		return m_wasKilled;
	}
protected:
	MinecraftProcess(Instance * source, InstConsoleWindow* parent);
	void OnTerminate ( int pid, int status );
	bool m_wasKilled;
	InstConsoleWindow* m_parent;
	Instance * m_source;
};
