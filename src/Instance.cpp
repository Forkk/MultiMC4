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

const wxString cfgFileName = _("instance.cfg");

bool IsValidInstance(wxFileName rootDir)
{
	return rootDir.DirExists() && rootDir.FileExists(cfgFileName);
}

Instance::Instance(wxFileName rootDir, wxString name)
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
	if (!GetRootDir().DirExists())
	{
		GetRootDir().Mkdir();
	}
	
	config.Save(GetConfigPath());
}

void Instance::Load()
{
	wxFileName cfgPath = GetConfigPath();
	wxFileName oldCfgPath = rootDir.FileName(_("instance.xml"));

	try
	{
		if (cfgPath.FileExists())
		{
			config.Load(cfgPath);
		}
		else if (oldCfgPath.FileExists())
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

wxFileName Instance::GetRootDir()
{
	return rootDir;
}

wxFileName Instance::GetConfigPath()
{
	return wxFileName(rootDir.GetFullPath(), cfgFileName);
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
void InstConfig::Load(const wxFileName &filename)
{
	using boost::property_tree::ptree;
	ptree pt;

	read_ini(Utils::stdStr(filename.GetPath()).c_str(), pt);

	name = pt.get<std::string>("name", "Unnamed Instance");
	iconKey = pt.get<std::string>("iconKey", "default");
	notes = pt.get<std::string>("notes", "");
	needsRebuild = pt.get<bool>("NeedsRebuild", false);
	askUpdate = pt.get<bool>("AskUpdate", true);
}

void InstConfig::Save(const wxFileName &filename)
{
	using boost::property_tree::ptree;
	ptree pt;
	
	pt.put<std::string>("name", name);
	pt.put<std::string>("iconKey", iconKey);
	pt.put<std::string>("notes", notes);
	pt.put<bool>("NeedsRebuild", needsRebuild);
	pt.put<bool>("AskUpdate", askUpdate);
	
	write_ini(Utils::stdStr(filename.GetFullPath()).c_str(), pt);
}

void InstConfig::LoadXML(wxFileName &filename)
{
	using boost::property_tree::ptree;
	ptree pt;
	
	read_xml(Utils::stdStr(filename.GetFullPath()).c_str(), pt);

	name = pt.get<std::string>("instance.name", "Unnamed Instance");
	iconKey = pt.get<std::string>("instance.iconKey", "default");
	notes = pt.get<std::string>("instance.notes", "");
	needsRebuild = pt.get<bool>("instance.NeedsRebuild", false);
	askUpdate = pt.get<bool>("instance.AskUpdate", true);
}
