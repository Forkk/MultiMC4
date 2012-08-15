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

#include "configpack.h"

#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>

#include <wx/wfstream.h>
#include <wx/zipstrm.h>
#include <wx/sstream.h>

#include <string>
#include <sstream>

#include "apputils.h"

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
	} while (entry.get() != nullptr && entry->GetInternalName() != _("modpack.json"));

	// Read the file into a string so boost can parse it
	wxStringOutputStream jsonOut;
	jsonOut.Write(zipIn);

	// Create a std::stringstream for boost to read from.
	std::stringstream jsonIn(cStr(jsonOut.GetString()));

	using namespace boost::property_tree;
	try
	{
		ptree pt;
		read_json(jsonIn, pt);

		m_packName = wxStr(pt.get<std::string>("name"));
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
			wxString id = wxStr(v.second.get<std::string>("id"));
			wxString version = wxStr(v.second.get<std::string>("version"));

			mlModInfoList.push_back(CPModInfo(id, version));
		}
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

ConfigPack::CPModInfo::CPModInfo(const wxString& id, const wxString& version)
{
	m_id = id;
	m_version = version;
}
