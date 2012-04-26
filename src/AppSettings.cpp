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

#include "AppSettings.h"

AppSettings settings;

bool InitAppSettings(void)
{
	settings.Load();
	return true;
}

void AppSettings::Save(fs::path file /* = iniConfigFile */)
{
	using boost::property_tree::ptree;
	ptree pt;

	pt.put<int>("MinMemoryAlloc", minMemAlloc);
	pt.put<int>("MaxMemoryAlloc", maxMemAlloc);

	pt.put<fs::path>("JavaPath", javaPath);
	pt.put<fs::path>("InstDir", instanceDir);
	pt.put<fs::path>("ModsDir", modsDir);

	pt.put<bool>("ShowConsole", showConsole);
	pt.put<bool>("AutoCloseConsole", autoCloseConsole);
	pt.put<bool>("AutoUpdate", autoUpdate);
	pt.put<bool>("QuitIfProblem", quitIfProblem);

	write_ini((char *)file.native().c_str(), pt);
}

void AppSettings::Load(fs::path file /* = iniConfigFile */)
{
	using boost::property_tree::ptree;
	ptree pt;

	if (fs::exists(file) && is_regular_file(file))
	{
		read_ini((char *)file.native().c_str(), pt);
	}

	minMemAlloc = pt.get<int>("MinMemoryAlloc", 512);
	minMemAlloc = pt.get<int>("MaxMemoryAlloc", 1024);

	javaPath = pt.get<fs::path>("JavaPath", defJavaPath);
	instanceDir = pt.get<fs::path>("InstDir", defInstDir);
	modsDir = pt.get<fs::path>("ModsDir", defModsDir);

	showConsole = pt.get<bool>("ShowConsole", false);
	autoCloseConsole = pt.get<bool>("AutoCloseConsole", false);
	autoUpdate = pt.get<bool>("AutoUpdate", true);
	quitIfProblem = pt.get<bool>("QuitIfProblem", false);
}