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

#include "mcversionlist.h"

#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>

#include <sstream>

#include <wx/regex.h>
#include <wx/numformatter.h>

#include "httputils.h"
#include "apputils.h"

//#define PRINT_CRUD

MCVersionList* MCVersionList::pInstance = 0;

MCVersionList::MCVersionList()
{
	stableVersionIndex = -1;
}

bool TimeFromS3Time(wxString str, wxDateTime & datetime)
{
	wxString::const_iterator end;
	const wxString fmt("%Y-%m-%dT%H:%M:%S.000Z");
	///FIXME: The date is in UTC, this reads it as some kind of local time... bleh. there could be bugs.
	return datetime.ParseFormat(str, fmt, &end);
}

bool compareVersions(const MCVersion & left, const MCVersion & right)
{
	return left.unixTimestamp > right.unixTimestamp;
}

bool MCVersionList::LoadIfNeeded()
{
	if(versions.empty())
	{
		return Reload();
	}
	return true;
}

bool MCVersionList::Reload()
{
	using namespace boost::property_tree;
	versions.clear();
	
	MCVersion currentStable;
	bool currentStableFound = false;
	
	wxString mainXML = wxEmptyString;
	if (!DownloadString("http://s3.amazonaws.com/MinecraftDownload/", &mainXML))
	{
		wxLogError(_("Failed to get snapshot list. Check your internet connection."));
		return false;
	}

	try
	{
		ptree pt;
		std::stringstream inStream(stdStr(mainXML), std::ios::in);
		read_xml(inStream, pt);
		BOOST_FOREACH(const ptree::value_type& v, pt.get_child("ListBucketResult"))
		{
			if (v.first == "Contents")
			{
				wxString keyName = wxStr(v.second.get<std::string>("Key"));
				if(keyName == "minecraft.jar")
				{
					wxString datetimeStr = wxStr(v.second.get<std::string>("LastModified"));
					wxString etag = wxStr(v.second.get<std::string>("ETag"));
					wxDateTime dtt;
					if(!TimeFromS3Time(datetimeStr, dtt))
					{
						wxLogError(_("Failed to parse date/time."));
						return false;
					}
					MCVersion version("current",dtt.GetTicks(),"http://s3.amazonaws.com/MinecraftDownload/",true,etag);
					currentStable = version;
					currentStableFound = true;
					break;
				}
			}
		}
	}
	catch (xml_parser_error e)
	{
		wxLogError(_("Failed to parse snapshot list.\nXML parser error at line %i: %s"), 
			e.line(), wxStr(e.message()).c_str());
		return false;
	}
	
	// Parse XML from the given URL.
	wxString assetsXML = wxEmptyString;
	if (!DownloadString("http://assets.minecraft.net/", &assetsXML))
	{
		wxLogError(_("Failed to get snapshot list. Check your internet connection."));
		return false;
	}

	try
	{
		bool found_current_in_assets = false;
		ptree pt;
		std::stringstream inStream(stdStr(assetsXML), std::ios::in);
		read_xml(inStream, pt);

		wxRegEx mcRegex("/minecraft.jar$");
		wxRegEx snapshotRegex("[0-9][0-9]w[0-9][0-9][a-z]|pre|rc");
		BOOST_FOREACH(const ptree::value_type& v, pt.get_child("ListBucketResult"))
		{
			if (v.first != "Contents")
				continue;
			wxString keyName = wxStr(v.second.get<std::string>("Key"));
			wxString datetimeStr = wxStr(v.second.get<std::string>("LastModified"));
			wxString etag = wxStr(v.second.get<std::string>("ETag"));
			wxDateTime dtt;
			if(!TimeFromS3Time(datetimeStr, dtt))
			{
				wxLogError(_("Failed to parse date/time."));
				return false;
			}
			if (mcRegex.Matches(keyName))
			{
				keyName = keyName.Left(keyName.Len() - 14);
				wxString dlUrl;
				dlUrl << "http://assets.minecraft.net/" << keyName << "/";
				for(unsigned i = 0; i < keyName.size();i++)
				{
					if(keyName[i] == '_')
						keyName[i] = '.';
				}
				if(currentStableFound && etag == currentStable.etag)
				{
					MCVersion version(keyName,dtt.GetTicks(),currentStable.dlURL,true,etag);
					version.type = CurrentStable;
					versions.push_back(version);
					found_current_in_assets = true;
				}
				else if (currentStableFound)
				{
					bool older = dtt.GetTicks() < currentStable.unixTimestamp;
					bool newer = dtt.GetTicks() > currentStable.unixTimestamp;
					bool isSnapshot = snapshotRegex.Matches(keyName);
					MCVersion version(keyName,dtt.GetTicks(),dlUrl,false,etag);
					if(newer)
					{
						version.type = Snapshot;
					}
					else if(older && isSnapshot)
					{
						version.type = OldSnapshot;
					}
					else if(older)
					{
						version.type = Stable;
					}
					else
					{
						// shouldn't happen, right? we handle this above
						version.type = CurrentStable;
					}
					versions.push_back(version);
				}
				else // there is no current stable :<
				{
					bool isSnapshot = snapshotRegex.Matches(keyName);
					MCVersion version(keyName,dtt.GetTicks(),dlUrl,false,etag);
					if(isSnapshot)
					{
						version.type = Snapshot;
					}
					else
					{
						version.type = Stable;
					}
					versions.push_back(version);
				}
			}
		}
		// if this ever happens, we need to inject the current version into the list, if it exists
		if(!found_current_in_assets && currentStableFound)
		{
			versions.push_back(currentStable);
		}
	}
	catch (xml_parser_error e)
	{
		wxLogError(_("Failed to parse snapshot list.\nXML parser error at line %i: %s"), 
			e.line(), wxStr(e.message()).c_str());
		return false;
	}
	std::sort(versions.begin(), versions.end(),compareVersions);
	for(int i = 0; i < versions.size();i++)
	{
		if(versions[i].type == CurrentStable)
		{
			stableVersionIndex = i;
			break;
		}
	}
#ifdef PRINT_CRUD
	std::ofstream out("test.txt");
	if(out.is_open())
	{
		for(unsigned i = 0; i < versions.size();i++)
		{
			MCVersion & v = versions[i];
			out << "Version " << v.name << " DLURL: " << v.dlURL << std::endl;
			out << "timestamp " << v.unixTimestamp << std::endl;
			switch(v.type)
			{
				case OldSnapshot:
					out << "Old snapshot" << std::endl;
					break;
				case Stable:
					out << "Old stable" << std::endl;
					break;
				case CurrentStable:
					out << "Current stable" << std::endl;
					break;
				case Snapshot:
					out << "Snapshot" << std::endl;
					break;
			}
			out << "-----------------------------------------" << std::endl;
		}
	}
	out.close();
#endif
	return true;
}
