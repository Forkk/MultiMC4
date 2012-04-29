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

const wxFileName iniConfigFile(_("multimc.cfg"));

bool InitAppSettings(void);

// Default values
const wxString defJavaPath = _("java");
const wxString defInstDir = _("instances");
const wxString defModsDir = _("mods");

struct AppSettings
{
	int minMemAlloc;
	int maxMemAlloc;
	
	wxFileName javaPath;
	wxFileName instanceDir;
	wxFileName modsDir;
	
	bool showConsole;
	bool autoCloseConsole;
	bool autoUpdate;
	bool quitIfProblem;
	
	void Save(const wxFileName& filename = iniConfigFile);
	void Load(const wxFileName& filename = iniConfigFile);
	
	wxFileName GetPathSetting(boost::property_tree::ptree& pt, 
							  const std::string& key, 
							  const std::string& def, 
							  bool isDir = true);
};

extern AppSettings settings;