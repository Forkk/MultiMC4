#include "forgeversions.h"
#include <map>
#include <wx/regex.h>

// map between forge version prefixes and minecraft versions
struct initme
{
	// FIXME: replace with some trie implementation if bored
	// MARISA-trie looks good :3
	initme()
	{
		valid = false;
		if (!forgeRegex.Compile("[0-9]+.[0-9]+.[0-9]+.[0-9]+"))
		{
			return;
		}
		// good versioning
		bm["6*"] = "1.4.2";
		bm["5*"] = "1.4";
		bm["4*"] = "1.3.2";
		bm["3*"] = "1.2.5";
		bm["2*"] = "1.2.4";
		// messy versioning
		bm["1.4*"] = "1.2.3";
		bm["1.3.4.4*"] = "1.2.3"; // 1.3.4.40 and 1.3.4.42
		bm["1.3.4.3*"] = "1.2.3"; // 1.3.4.30 - 1.3.4.39
		bm["1.3.4.29*"] = "1.1"; // last 1.1 version
		bm["1.3.3*"] = "1.1";
		bm["1.3.2*"] = "1.1";
		valid = true;
	};
	std::map <wxString, wxString> bm;
	wxRegEx forgeRegex;
	bool valid;
} versions;

wxString forgeutils::MCVersionFromForgeFilename ( wxString filename )
{
	if(versions.valid && versions.forgeRegex.Matches(filename))
	{
		wxString version = versions.forgeRegex.GetMatch(filename,0);
		// Really, a vector would suffice... but ... whatever.
		// I like the smell of abused red-black trees in the morning :P
		for(auto iter = versions.bm.begin(); iter != versions.bm.end(); iter++)
		{
			if(version.Matches((*iter).first))
			{
				return (*iter).second;
			}
		}
	}
	return "Unknown";
}
