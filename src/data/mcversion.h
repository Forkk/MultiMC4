#pragma once
#include <wx/string.h>

enum VersionType
{
	OldSnapshot,
	Stable,
	CurrentStable,
	Snapshot,
	MCRewind,
	MetaCustom,
	MetaLatestSnapshot,
	MetaLatestStable,
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
	static MCVersion getMCRVersion (wxString raw_name, wxString nice_name, wxString ptVersion, wxString md5sum)
	{
		MCVersion ver;
		ver.descriptor = raw_name;
		ver.name = nice_name;
		ver.patchTargetVersion = ptVersion;
		ver.etag = md5sum;
		ver.SetVersionType(MCRewind);
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

	// If this is an MCRewind version, gets the version that this version's
	// patches should be applied to.
	// Otherwise, just returns empty string.
	wxString GetPatchTargetVersion() const
	{
		return patchTargetVersion;
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
	
private:
	// raw version string as used in configs
	wxString descriptor;
	// translated name
	wxString name;
	// checksum
	wxString etag;
	// last changed unix time in seconds. nice for sorting :3
	uint64_t unixTimestamp;
	// base URL for download
	wxString dlURL;
	// version that this MCRW version's patches should be applied to to get the desired version
	wxString patchTargetVersion;
	// this version has a mojang lwjgl associated with it
	bool has_lwjgl;
	VersionType type;
	MCVersion * linkedVersion;
};
