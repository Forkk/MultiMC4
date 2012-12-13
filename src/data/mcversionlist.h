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

#pragma once
#include <wx/string.h>
#include <vector>
#include <stdint.h>

enum VersionType
{
	OldSnapshot,
	Stable,
	CurrentStable,
	Snapshot,
	MetaCustom,
	MetaLatestSnapshot,
	MetaLatestStable,
	MCNostalgia
};

// Some handy constants for often used version descriptors
#define MCVer_Latest_Stable    "LatestStable"
#define MCVer_Current_Stable   "CurrentStable"
#define MCVer_Latest_Snapshot  "LatestSnapshot"
#define MCVer_Current_Snapshot "CurrentSnapshot"
#define MCVer_Unknown          "Unknown"

class MCVersion
{
public:
	MCVersion(wxString _descriptor, wxString _name, uint64_t _unixTimestamp, wxString _dlURL, bool _has_lwjgl, wxString _etag)
	: descriptor(_descriptor), name(_name), unixTimestamp(_unixTimestamp), dlURL(_dlURL), has_lwjgl(_has_lwjgl), etag(_etag)
	{
		linkedVersion = nullptr;
	}
	MCVersion()
	{
		unixTimestamp = 0;
		has_lwjgl = false;
		linkedVersion = nullptr;
	}
	MCVersion(MCVersion * linked)
	{
		linkedVersion = linked;
	}
	static MCVersion getMCNVersion (wxString raw_name, wxString nice_name, wxString required)
	{
		MCVersion ver;
		ver.descriptor = raw_name;
		ver.name = nice_name;
		ver.mcn_dependency = required;
		ver.SetVersionType(MCNostalgia);
		return ver;
	}
	wxString GetDLUrl() const
	{
		if(linkedVersion)
			return linkedVersion->GetDLUrl();
		return dlURL;
	}
	wxString GetDescriptor() const
	{
		return descriptor;
	}
	wxString GetName() const
	{
		if(linkedVersion)
			return linkedVersion->GetName();
		return name;
	}
	wxString GetEtag() const
	{
		if(linkedVersion)
			return linkedVersion->GetEtag();
		return etag;
	}
	uint64_t GetTimestamp() const
	{
		if(linkedVersion)
			return linkedVersion->GetTimestamp();
		return unixTimestamp;
	}
	bool IsMeta() const
	{
		return type == MetaCustom || type == MetaLatestSnapshot || type == MetaLatestStable;
	}
	VersionType GetVersionType() const
	{
		if(linkedVersion)
			return linkedVersion->GetVersionType();
		return type;
	}
	void SetVersionType(VersionType newType)
	{
		type = newType;
	}
	
public:
	static const char * descriptors[];
	
private:
	// raw version string as used in configs
	wxString descriptor;
	// translated name
	wxString name;
	// checksum
	wxString etag;
	// last changed unix time in seconds. nice for sorting :3
	uint64_t unixTimestamp;
	// required normal version for MCNostalgia synthetic MCVersion
	wxString mcn_dependency;
	// base URL for download
	wxString dlURL;
	// this version has a mojang lwjgl associated with it
	bool has_lwjgl;
	VersionType type;
	MCVersion * linkedVersion;
};

class MCVersionList
{
public:
	static MCVersionList& Instance()
	{
		if (pInstance == 0)
			pInstance = new MCVersionList();
		return *pInstance;
	};
	

	bool Reload();
	bool LoadIfNeeded();
	bool NeedsLoad()
	{
		return versions.size() == 0;
	}
	MCVersion * GetVersion ( wxString descriptor );

	std::vector <MCVersion> versions;
	int stableVersionIndex;
private:
	static MCVersionList * pInstance;
	MCVersionList();
};
