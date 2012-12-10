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
#include <wx/string.h>
#include <vector>
#include <stdint.h>

class LWJGLVersion
{
public:
	LWJGLVersion(wxString _name, wxString _dlURL) : name(_name), dlURL(_dlURL) {}
	wxString GetDLUrl() const
	{
		return dlURL;
	}
	wxString GetName() const
	{
		return name;
	}

private:
	// version string
	wxString name;
	// base URL for download
	wxString dlURL;
};

class LWJGLVersionList
{
public:
	static LWJGLVersionList& Instance()
	{
		if (pInstance == 0)
			pInstance = new LWJGLVersionList();
		return *pInstance;
	};

	bool Reload();
	bool LoadIfNeeded();
	bool NeedsLoad()
	{
		return versions.size() == 0;
	}
	LWJGLVersion * GetVersion ( wxString name );

	std::vector <LWJGLVersion> versions;
private:
	static LWJGLVersionList * pInstance;
	LWJGLVersionList();
};
