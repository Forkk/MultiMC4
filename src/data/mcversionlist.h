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
#include "mcversion.h"

class MCVersionList
{
public:
	static MCVersionList& Instance()
	{
		if (pInstance == 0)
			pInstance = new MCVersionList();
		return *pInstance;
	};
	
	bool LoadNostalgia();
	bool LoadMojang();
	
	bool LoadIfNeeded();
	bool NeedsLoad()
	{
		return NeedsMojangLoad() || NeedsNostalgiaLoad();
	}
	bool NeedsMojangLoad()
	{
		return versions.size() == 0;
	}
	bool NeedsNostalgiaLoad()
	{
		return includesNostalgia && nostalgia_versions.size() == 0;
	}
	void SetNeedsNostalgia()
	{
		includesNostalgia = true;
	}
	
	MCVersion & operator[](std::size_t index);
	std::size_t size() const;
	
	MCVersion * GetVersion ( wxString descriptor );
	MCVersion * GetCurrentStable ();
	int GetStableVersionIndex()
	{
		return stableVersionIndex;
	};
private:
	std::vector <MCVersion> versions;
	std::vector <MCVersion> nostalgia_versions;
	int stableVersionIndex;
	bool includesNostalgia;
	static MCVersionList * pInstance;
	MCVersionList();
};
