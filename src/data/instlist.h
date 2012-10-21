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

#include <wx/list.h>
#include <map>

class Instance;

WX_DECLARE_LIST(Instance, InstListBase);

typedef std::map<wxString, wxString> GroupMap;

class InstList : public InstListBase
{
public:
	InstList();

	// Gets the given instance's group.
	wxString GetGroup(Instance* inst);

	// Sets the given instance's group.
	void SetGroup(Instance* inst, const wxString& group);

	bool LoadGroupInfo(const wxString& file);
	bool SaveGroupInfo(const wxString& file) const;

protected:
	GroupMap m_groupMap;
};
