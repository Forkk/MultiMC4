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

#include "mod.h"

#define READ_MODINFO

#ifdef READ_MODINFO
#include <boost/property_tree/json_parser.hpp>

#include <wx/wfstream.h>
#include <wx/zipstrm.h>
#include <wx/sstream.h>

#include <string>
#include <sstream>

#include <memory>

#include "apputils.h"

#endif

Mod::Mod(const wxFileName& file)
{
	modFile = file;
	
	modName = modFile.GetName();

#ifdef READ_MODINFO
	if (IsZipMod())
	{
		wxFFileInputStream fileIn(modFile.GetFullPath());
		wxZipInputStream zipIn(fileIn);

		std::auto_ptr<wxZipEntry> entry;

		do 
		{
			entry.reset(zipIn.GetNextEntry());
		} while (entry.get() != nullptr && !entry->GetInternalName().EndsWith(_(".info")));

		if (entry.get() != nullptr)
		{
			// Read the info file into text
			wxString infoFileData;
			wxStringOutputStream stringOut(&infoFileData);
			zipIn.Read(stringOut);
		
			using namespace boost::property_tree;

			// Read the data
			ptree ptRoot;

			wxString entryName = entry->GetInternalName();

			std::stringstream stringIn(cStr(infoFileData));
			try
			{
				read_json(stringIn, ptRoot);

				ptree pt = ptRoot.get_child("").begin()->second;

				modID = wxStr(pt.get<std::string>("modid"));
				modName = wxStr(pt.get<std::string>("name"));
				modVersion = wxStr(pt.get<std::string>("version"));
			}
			catch (json_parser_error e)
			{
				// Silently fail...
			}
			catch (ptree_error e)
			{
				// Silently fail...
			}
		}
	}
#endif
}

Mod::Mod(const Mod& mod)
{
	modFile = mod.GetFileName();
	modName = mod.GetName();
	modVersion = mod.GetModVersion();
	mcVersion = mod.GetMCVersion();
}

wxFileName Mod::GetFileName() const
{
	return modFile;
}

wxString Mod::GetName() const
{
	return modName;
}

wxString Mod::GetModID() const
{
	return GetFileName().GetFullName();
}

wxString Mod::GetModVersion() const
{
	return modVersion;
}

wxString Mod::GetMCVersion() const
{
	return mcVersion;
}

bool Mod::IsZipMod() const
{
	return GetFileName().GetExt() == _("zip") || GetFileName().GetExt() == _("jar");
}

bool Mod::operator ==(const Mod &other) const
{
	return GetFileName().SameAs(other.GetFileName());
}
