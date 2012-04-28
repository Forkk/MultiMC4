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

#include "Instance.h"

const char cfgFileName[] = "instance.cfg";

bool IsValidInstance(fs::path rootDir)
{
	return fs::exists(rootDir) && fs::is_directory(rootDir) &&
		fs::exists(rootDir / cfgFileName) && fs::is_regular_file(rootDir / cfgFileName);
}

Instance::Instance(fs::path rootDir, wxString name)
{
	this->rootDir = rootDir;

	if (!name.IsNull() && !name.IsEmpty())
		this->SetName(name);

	Load();
}

Instance::~Instance(void)
{

}

void Instance::Save()
{
	if (!fs::exists(GetRootDir()))
	{
		fs::create_directories(GetRootDir());
	}
	
	config.Save(GetConfigPath());
}

void Instance::Load()
{
	fs::path cfgPath = rootDir / cfgFileName;
	fs::path oldCfgPath = rootDir / "instance.xml";

	try
	{
		if (exists(cfgPath) && is_regular_file(cfgPath))
		{
			config.Load(cfgPath);
		}
		else if (exists(oldCfgPath) && is_regular_file(oldCfgPath))
		{
			config.LoadXML(oldCfgPath);
		}
	}
	catch (boost::property_tree::ini_parser_error e)
	{

	}
	catch (boost::property_tree::xml_parser_error e)
	{

	}
}

fs::path Instance::GetRootDir()
{
	return rootDir;
}

fs::path Instance::GetConfigPath()
{
	return rootDir / cfgFileName;
}

wxString Instance::GetName()
{
	return Utils::wxStr(config.name);
}

void Instance::SetName(wxString name)
{
	config.name = Utils::stdStr(name);
}

wxString Instance::GetIconKey()
{
	return Utils::wxStr(config.iconKey);
}

void Instance::SetIconKey(wxString iconKey)
{
	config.iconKey = Utils::stdStr(iconKey);
}

// InstConfig
void InstConfig::Load(const fs::path &filename)
{
	using boost::property_tree::ptree;
	ptree pt;

	read_ini(filename.string(), pt);

	name = pt.get<std::string>("name", "Unnamed Instance");
	iconKey = pt.get<std::string>("iconKey", "default");
	notes = pt.get<std::string>("notes", "");
	needsRebuild = pt.get<bool>("NeedsRebuild", false);
	askUpdate = pt.get<bool>("AskUpdate", true);
}

void InstConfig::Save(const fs::path &filename)
{
	using boost::property_tree::ptree;
	ptree pt;

	pt.put<std::string>("name", name);
	pt.put<std::string>("iconKey", iconKey);
	pt.put<std::string>("notes", notes);
	pt.put<bool>("NeedsRebuild", needsRebuild);
	pt.put<bool>("AskUpdate", askUpdate);

	write_ini(filename.string(), pt);
}

void InstConfig::LoadXML(fs::path &filename)
{
	using boost::property_tree::ptree;
	ptree pt;

	read_xml(filename.string(), pt);

	name = pt.get<std::string>("instance.name", "Unnamed Instance");
	iconKey = pt.get<std::string>("instance.iconKey", "default");
	notes = pt.get<std::string>("instance.notes", "");
	needsRebuild = pt.get<bool>("instance.NeedsRebuild", false);
	askUpdate = pt.get<bool>("instance.AskUpdate", true);
}
