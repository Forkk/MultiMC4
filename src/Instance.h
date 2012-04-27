// 
//  Copyright 2012 Andrew Okin
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
#include "includes.h"

namespace fs = boost::filesystem;

bool IsValidInstance(fs::path rootDir);

struct InstConfig
{
	std::string name;
	std::string iconKey;
	std::string notes;
	bool needsRebuild;
	bool askUpdate;

	void Load(const fs::path &filename);
	void Save(const fs::path &filename);

	void LoadXML(fs::path &filename);
};

class Instance
{
public:
	Instance(fs::path rootDir, wxString name = _T(""));
	~Instance(void);

	void Save();
	void Load();

	fs::path GetRootDir();
	fs::path GetConfigPath();

	wxString GetName() { return wxString(config.name.c_str(), wxConvUTF8); }
	void SetName(wxString name) { config.name = std::string(name.mb_str()); }

	wxString GetIconKey() { return wxString(config.iconKey.c_str(), wxConvUTF8); }
	void SetIconKey(wxString iconKey) { config.iconKey = std::string(iconKey.mb_str()); }

protected:
	InstConfig config;

	fs::path rootDir;
};
