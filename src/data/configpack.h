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

class ConfigPack
{
public:
	ConfigPack(const wxString& fileName);

	struct CPModInfo
	{
		CPModInfo(const wxString& id, const wxString& version);

		wxString m_id;
		wxString m_version;
	};

	bool IsValid() const;

	wxString GetFileName() const;
	wxString GetPackName() const;
	wxString GetPackNotes() const;
	wxString GetMinecraftVersion() const;

	const std::vector<CPModInfo>* GetJarModList() const;
	const std::vector<CPModInfo>* GetMLModList() const;
	const std::vector<CPModInfo>* GetCoreModList() const;

protected:
	bool m_valid;

	wxString m_fileName;
	
	wxString m_packName;
	wxString m_packNotes;
	wxString m_minecraftVersion;

	std::vector<CPModInfo> jarModInfoList;
	std::vector<CPModInfo> mlModInfoList;
	std::vector<CPModInfo> coreModInfoList;
};
