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

#include "appsettings.h"

AppSettings settings;

bool InitAppSettings(void)
{
	settings.Load();
	return true;
}

void AppSettings::Save(const wxFileName &filename /* = iniConfigFile */)
{
	using boost::property_tree::ptree;
	ptree pt;
	
	pt.put<int>("MinMemoryAlloc", minMemAlloc);
	pt.put<int>("MaxMemoryAlloc", maxMemAlloc);
	
	pt.put<std::string>("JavaPath", stdStr(javaPath.GetFullPath()));
	pt.put<std::string>("InstanceDir", stdStr(instanceDir.GetFullPath()));
	pt.put<std::string>("ModsDir", stdStr(modsDir.GetFullPath()));
	
	pt.put<bool>("ShowConsole", showConsole);
	pt.put<bool>("AutoCloseConsole", autoCloseConsole);
	pt.put<bool>("AutoUpdate", autoUpdate);
	pt.put<bool>("QuitIfProblem", quitIfProblem);
	
	wxString str = filename.GetFullPath();
	write_ini(stdStr(filename.GetFullPath()).c_str(), pt);
}

void AppSettings::Load(const wxFileName &filename /* = iniConfigFile */)
{
	using boost::property_tree::ptree;
	ptree pt;
	
	if (filename.FileExists())
		read_ini(stdStr(filename.GetFullPath()).c_str(), pt);
	
	minMemAlloc = pt.get<int>("MinMemoryAlloc", 512);
	maxMemAlloc = pt.get<int>("MaxMemoryAlloc", 1024);
	
	javaPath = GetPathSetting(pt, "JavaPath", "java", false);
	instanceDir = GetPathSetting(pt, "InstanceDir", "instances");
	modsDir = GetPathSetting(pt, "ModsDir", "mods");
	
	showConsole = pt.get<bool>("ShowConsole", false);
	autoCloseConsole = pt.get<bool>("AutoCloseConsole", true);
	autoUpdate = pt.get<bool>("AutoUpdate", true);
	quitIfProblem = pt.get<bool>("QuitIfProblem", false);
}

wxFileName AppSettings::GetPathSetting(boost::property_tree::ptree &pt, 
									   const std::string &key, 
									   const std::string &def, 
									   bool isDir)
{
	std::string path = pt.get<std::string>(key, def);
	if (path.empty())
	{
		path = def;
	}
	if (isDir)
		return wxFileName(wxStr(path), wxEmptyString);
	else
		return wxFileName(wxStr(path));
}
