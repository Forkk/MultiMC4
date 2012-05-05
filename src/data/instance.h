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

bool IsValidInstance(wxFileName rootDir);

class Instance
{
public:
	static Instance *LoadInstance(wxFileName rootDir);
	Instance(wxFileName rootDir, wxString name);
	~Instance(void);
	
	bool Save() const;
	bool Load(bool loadDefaults = false);
	
	// Directories
	wxFileName GetRootDir() const;
	wxFileName GetMCDir() const;
	wxFileName GetBinDir() const;
	
	// Files
	wxFileName GetConfigPath() const;
	wxFileName GetVersionFile() const;
	wxFileName GetMCBackup() const;
	
	
	wxString ReadVersionFile();
	void WriteVersionFile(const wxString& contents);
	
	wxString GetName() const;
	void SetName(wxString name);
	
	wxString GetIconKey() const;
	void SetIconKey(wxString iconKey);

protected:
	wxFileName rootDir;

	wxString name;
	wxString iconKey;
	wxString notes;
	bool needsRebuild;
	bool askUpdate;
};
