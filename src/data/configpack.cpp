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

#include "configpack.h"

#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>

#include <wx/wfstream.h>
#include <wx/zipstrm.h>
#include <wx/sstream.h>

#include <string>
#include <sstream>

#include "utils/apputils.h"

#include <memory>

ConfigPack::ConfigPack(const wxString& fileName)
	: m_fileName(fileName)
{
	m_valid = false;

	// Read the zip file.
	wxFFileInputStream fileIn(fileName);
	wxZipInputStream zipIn(fileIn);

	std::auto_ptr<wxZipEntry> entry;

	do 
	{
		entry.reset(zipIn.GetNextEntry());
	} while (entry.get() != nullptr && entry->GetInternalName() != "modpack.json");
	
	// Read the file into a stringstream so boost can parse it
	auto e = entry.get();
	if(!e)
		return;
	char * raw_buf = new char[e->GetSize()+1];
	zipIn.Read(raw_buf,e->GetSize());
	raw_buf[e->GetSize()] = 0;
	std::stringstream jsonIn(raw_buf);
	delete[] raw_buf;
	
	using namespace boost::property_tree;
	try
	{
		ptree pt;
		read_json(jsonIn, pt);

		m_packName = wxStr(pt.get<std::string>("name"));
		if(pt.count("MCversion"))
		{
			m_minecraftVersion = wxStr(pt.get<std::string>("MCversion"));
		}
		else
		{
			m_minecraftVersion = "Unknown";
		}
		m_packNotes = wxStr(pt.get<std::string>("notes"));

		// Load the jar mod list.
		BOOST_FOREACH(const ptree::value_type& v, pt.get_child("jarmods"))
		{
			wxString id = wxStr(v.second.get<std::string>("id"));
			wxString version = wxStr(v.second.get<std::string>("version"));

			jarModInfoList.push_back(CPModInfo(id, version));
		}
		
		// Load the ML mod list.
		BOOST_FOREACH(const ptree::value_type& v, pt.get_child("mlmods"))
		{
			std::string idstr =v.second.get<std::string>("id");
			std::string vstr =v.second.get<std::string>("version");
			wxString id = wxStr(idstr);
			wxString version = wxStr(vstr);

			mlModInfoList.push_back(CPModInfo(id, version));
		}
		// older config packs don't have to have core mods...
		try
		{
			auto coremods = pt.get_child("coremods");
			// Load the core mod list.
			BOOST_FOREACH(const ptree::value_type& v, coremods)
			{
				wxString id = wxStr(v.second.get<std::string>("id"));
				wxString version = wxStr(v.second.get<std::string>("version"));

				coreModInfoList.push_back(CPModInfo(id, version));
			}
		}
		catch(ptree_bad_path e){};
	}
	catch (json_parser_error e)
	{
		wxLogError(_("Invalid config pack. Failed to parse JSON. At line %i: %s"),
			e.line(), wxStr(e.message()).c_str());
		return;
	}
	catch (ptree_error)
	{
		wxLogError(_("Invalid config pack."));
		return;
	}
	m_valid = true;
}

bool ConfigPack::IsValid() const
{
	return m_valid;
}

wxString ConfigPack::GetFileName() const
{
	return m_fileName;
}

wxString ConfigPack::GetPackName() const
{
	return m_packName;
}

wxString ConfigPack::GetPackNotes() const
{
	return m_packNotes;
}

const std::vector<ConfigPack::CPModInfo>* ConfigPack::GetJarModList() const
{
	return &jarModInfoList;
}

const std::vector<ConfigPack::CPModInfo>* ConfigPack::GetMLModList() const
{
	return &mlModInfoList;
}

const std::vector<ConfigPack::CPModInfo>* ConfigPack::GetCoreModList() const
{
	return &coreModInfoList;
}

ConfigPack::CPModInfo::CPModInfo(const wxString& id, const wxString& version)
{
	m_id = id;
	m_version = version;
}
