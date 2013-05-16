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
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>

#include <string>
#include <sstream>
#include <map>

#include <wx/regex.h>
#include <wx/numformatter.h>
#include <wx/datetime.h>

#include "utils/httputils.h"
#include "utils/apputils.h"

//#define PRINT_CRUD

const wxString mcrwIndexURL = "http://mcrw.forkk.net/index.json";

class initme
{
public:
	/// mapping from MCRewind versions to real versions
	std::map <wxString, wxString> mapping;
	/// mapping from current version etags to snapshot etags
	std::map <wxString, wxString> etagmapping;
	initme()
	{
		// wxEmptyString means that it should be ignored
		mapping["1.4.6_pre"] = wxEmptyString;
		mapping["1.4.5_pre"] = wxEmptyString;
		mapping["1.4.3_pre"] = "1.4.3";
		mapping["1.4.2_pre"] = wxEmptyString;
		mapping["1.4.1_pre"] = "1.4.1";
		mapping["1.4_pre"] = "1.4";
		mapping["1.3.2_pre"] = wxEmptyString;
		mapping["1.3.1_pre"] = wxEmptyString;
		mapping["1.3_pre"] = wxEmptyString;
		mapping["1.2_pre"] = "1.2";
		etagmapping["\"fd11cbc5b01aae1d62cff0145171f3d9\""] = "\"cb57b15e2038b533ef8527f132e17d9e-2\"";
		etagmapping["\"5c1219d869b87d233de3033688ec7567\""] = "\"e8a1f7a2f2ca9607a6ed3ab1494bb328-2\"";
	}
    bool MatchEtags(wxString snapshots_etag, wxString current_v_etag)
	{
		if(snapshots_etag.IsSameAs(current_v_etag))
			return true;
		auto iter = etagmapping.find(current_v_etag);
		if(iter != etagmapping.end())
		{
			if((*iter).second.IsSameAs(snapshots_etag))
				return true;
		}
		return false;
	};
} ver;

wxString MCRWVersionToAssetsVersion(wxString mcrw_version)
{
	auto iter = ver.mapping.find(mcrw_version);
	if(iter != ver.mapping.end())
	{
		return (*iter).second;
	}
	return mcrw_version;
}

MCVersionList* MCVersionList::pInstance = 0;

MCVersionList::MCVersionList()
{
	stableVersionIndex = -1;
	includesMCRW = false;
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
	return left.GetTimestamp() > right.GetTimestamp();
}

MCVersion * MCVersionList::GetVersion ( wxString descriptor )
{
	if(descriptor == MCVer_Unknown)
		return nullptr;
	else
	{
		auto descriptorPredicate = [&descriptor](MCVersion & ver){ return ver.GetDescriptor() == descriptor;};
		auto found = std::find_if(versions.begin(), versions.end(), descriptorPredicate);
		if(found != versions.end())
		{
			return &*found;
		}
		found = std::find_if(mcrw_versions.begin(), mcrw_versions.end(), descriptorPredicate);
		if(found != mcrw_versions.end())
		{
			return &*found;
		}
		return nullptr;
	}
}

MCVersion* MCVersionList::GetCurrentStable()
{
	if(versions.empty() || stableVersionIndex == -1)
		return nullptr;
	return &versions[stableVersionIndex];
}


bool MCVersionList::LoadIfNeeded()
{
	bool OK = true;
	if(NeedsMojangLoad())
	{
		OK &= LoadMojang();
	}
	if(NeedsMCRWLoad())
	{
		OK &= LoadMCRW();
	}
	return OK;
}

bool MCVersionList::LoadMCRW()
{
	mcrw_versions.clear();
	if(!versions.size())
		return false;
	wxString vlistJSON;
	if (DownloadString(mcrwIndexURL, &vlistJSON))
	{
		using namespace boost::property_tree;

		try
		{
			// Parse the JSON
			ptree pt;
			std::stringstream jsonStream(stdStr(vlistJSON), std::ios::in);
			read_json(jsonStream, pt);
			wxRegEx indevRegex("in(f)?dev");
			wxString mcVersion = wxStr(pt.get<std::string>("mcversion"));
			BOOST_FOREACH(const ptree::value_type& v, pt.get_child("versions"))
			{
				wxString rawVersion = wxStr(v.second.get<std::string>("name"));
				if(indevRegex.Matches(rawVersion))
					continue;
				wxString niceVersion = MCRWVersionToAssetsVersion(rawVersion);
				if(niceVersion.empty())
					continue;
				if(GetVersion(niceVersion))
					continue;

				wxString md5sum = wxStr(v.second.get<std::string>("md5"));

				MCVersion ver = MCVersion::getMCRVersion(rawVersion, niceVersion, mcVersion, md5sum);
				mcrw_versions.insert(mcrw_versions.begin(),ver);
			}
		}
		catch (json_parser_error e)
		{
			wxLogError(_("Failed to read MCRewind list.\nJSON parser error at line %i: %s"), 
				e.line(), wxStr(e.message()).c_str());
			return false;
		}
		catch (ptree_error)
		{
			wxLogError(_("Failed to read MCRewind list.\nThe format either changed or the server returned something else."));
			return false;
		}
		return true;
	}
	else
	{
		wxLogError(_("Failed to get MCRewind list. Check your internet connection and try again later."));
		return false;
	}
}

bool MCVersionList::LoadMojang()
{
	using namespace boost::property_tree;
	versions.clear();
	stableVersionIndex = -1;
	MCVersion currentStable;
	bool currentStableFound = false;
	
	wxString mainXML = wxEmptyString;
	
	wxString MojangURL = "http://s3.amazonaws.com/MinecraftDownload/";
	// ``broken'' URL for testing
	//wxString MojangURL = "http://dethware.org/dl/";
	bool suppress_error = false;
	
	if (DownloadString(MojangURL, &mainXML))
	{
		try
		{
			ptree pt;
			std::stringstream inStream(stdStr(mainXML), std::ios::in);
			read_xml(inStream, pt);
			BOOST_FOREACH(const ptree::value_type& v, pt.get_child("ListBucketResult"))
			{
				if (v.first != "Contents")
					continue;
				wxString keyName = wxStr(v.second.get<std::string>("Key"));
				if(keyName != "minecraft.jar")
					continue;
				wxString datetimeStr = wxStr(v.second.get<std::string>("LastModified"));
				wxString etag = wxStr(v.second.get<std::string>("ETag"));
				// use some kind of sensible time if we fail to parse it
				wxDateTime dtt;
				if(!TimeFromS3Time(datetimeStr, dtt))
				{
					wxLogError(_("Failed to parse date/time: %s %s"), keyName.c_str() , datetimeStr.c_str());
					dtt.SetToCurrent();
				}
				MCVersion version("LatestStable",_("Current"),dtt.GetTicks(),"http://s3.amazonaws.com/MinecraftDownload/",true,etag);
				currentStable = version;
				currentStableFound = true;
				break;
			}
		}
		catch (xml_parser_error e)
		{
			wxLogError(_("Failed to get the current stable version.\nEncountered a parser error at line %i: %s"), e.line(), e.message().c_str());
			suppress_error = true;
		}
		catch (ptree_error e)
		{
			wxLogError(_("Failed to get the current stable version.\nThe list format might have changed.\nPlease report this as a bug."));
			suppress_error = true;
		}
	}
	if(!currentStableFound && !suppress_error)
		wxLogError(_("Failed to get the current stable version.\nCheck your internet connection."));
	
	// Getting snapshots from the assets site is optional.
	// If it stops working, it shouldn't affect getting the current version
	bool have_snapshots = false;
	bool found_current_in_assets = false;
	wxString assetsXML = wxEmptyString;
	if (DownloadString("http://assets.minecraft.net/", &assetsXML))
	{
		try
		{
			ptree pt;
			std::stringstream inStream(stdStr(assetsXML), std::ios::in);
			read_xml(inStream, pt);

			wxRegEx mcRegex("/minecraft.jar$");
			wxRegEx snapshotRegex("[0-9][0-9]w[0-9][0-9][a-z]|pre|rc");
			BOOST_FOREACH(const ptree::value_type& v, pt.get_child("ListBucketResult"))
			{
				if (v.first != "Contents")
					continue;
				
				wxString Key = wxStr(v.second.get<std::string>("Key"));
				wxString datetimeStr = wxStr(v.second.get<std::string>("LastModified"));
				wxString etag = wxStr(v.second.get<std::string>("ETag"));
				
				if (!mcRegex.Matches(Key))
					continue;
				wxString versionID = Key.Left(Key.Len() - 14);
				
				wxString dlUrl;
				dlUrl << "http://assets.minecraft.net/" << versionID << "/";
				
				wxString versionName = versionID;
				for(unsigned i = 0; i < versionName.size();i++)
				{
					if(versionName[i] == '_')
						versionName[i] = '.';
				}
				
				wxDateTime dtt;
				if(!TimeFromS3Time(datetimeStr, dtt))
				{
					wxLogError(_("Failed to parse date/time: %s %s"), versionName.c_str() , datetimeStr.c_str());
					dtt.SetToCurrent();
				}
				
				if(currentStableFound && ver.MatchEtags(etag, currentStable.GetEtag()))
				{
					MCVersion version(versionName,versionName,dtt.GetTicks(),currentStable.GetDLUrl(),true,etag);
					version.SetVersionType(CurrentStable);
					versions.push_back(version);
					found_current_in_assets = true;
				}
				else if (currentStableFound)
				{
					bool older = dtt.GetTicks() < currentStable.GetTimestamp();
					bool newer = dtt.GetTicks() > currentStable.GetTimestamp();
					bool isSnapshot = snapshotRegex.Matches(versionName);
					MCVersion version(versionName, versionName,dtt.GetTicks(),dlUrl,false,etag);
					if(newer)
					{
						version.SetVersionType(Snapshot);
					}
					else if(older && isSnapshot)
					{
						version.SetVersionType(OldSnapshot);
					}
					else if(older)
					{
						version.SetVersionType(Stable);
					}
					else
					{
						// shouldn't happen, right? we handle this above
						version.SetVersionType(CurrentStable);
					}
					versions.push_back(version);
				}
				else // there is no current stable :<
				{
					bool isSnapshot = snapshotRegex.Matches(versionName);
					MCVersion version(versionName,versionName,dtt.GetTicks(),dlUrl,false,etag);
					if(isSnapshot)
					{
						version.SetVersionType(Snapshot);
					}
					else
					{
						version.SetVersionType(Stable);
					}
					versions.push_back(version);
				}
			}
		}
		catch (xml_parser_error e)
		{
			wxLogError(_("Failed to parse snapshot list.\nXML parser error at line %i: %s"), 
				e.line(), wxStr(e.message()).c_str());
		}
		catch (ptree_error e)
		{
			wxLogError(_("Failed to parse snapshot list fully.\nThe format might have changed.\nPlease report this as a bug."));
		}
		
	}
	// if this ever happens, we need to inject the current version into the list, if it exists
	if(!found_current_in_assets && currentStableFound)
	{
		currentStable.SetVersionType(CurrentStable);
		versions.push_back(currentStable);
		wxLogError(_("For some reason, MultiMC couldn't determine the current Minecraft version number.\nYou can still get the current version, but you will have to run it at least once to get the version number.\nIt is possible that the current version is doubled in the list."));
	}
	if(!versions.size())
	{
		wxLogError(_("There are no Minecraft versions available at this time."));
	}
	
	std::sort(versions.begin(), versions.end(),compareVersions);
	stableVersionIndex = -1;
	for(unsigned i = 0; i < versions.size();i++)
	{
		if(versions[i].GetVersionType() == CurrentStable)
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
	return versions.size() != 0;
}

std::size_t MCVersionList::size() const
{
	return versions.size() + mcrw_versions.size();
}

MCVersion& MCVersionList::operator[] ( std::size_t index )
{
	if(index < versions.size())
		return versions[index];
	else
		return mcrw_versions[index - versions.size()];
}
