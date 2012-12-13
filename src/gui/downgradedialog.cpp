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

#include "downgradedialog.h"

#include <wx/gbsizer.h>
#include <wx/hyperlink.h>
#include <wx/regex.h>

#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>

#include <string>
#include <sstream>
#include <map>

#include "utils/apputils.h"
#include "utils/httputils.h"
#include <mcversionlist.h>

const wxString mcnwebURL = "http://sonicrules.org/mcnweb.py";

class initme
{
public:
	std::map <wxString, wxString> mapping;
	initme()
	{
		// wxEmptyString means that it should be ignored
		mapping["1.4.5_pre"] = wxEmptyString;
		mapping["1.4.3_pre"] = "1.4.3";
		mapping["1.4.2_pre"] = wxEmptyString;
		mapping["1.4.1_pre"] = "1.4.1";
		mapping["1.4_pre"] = "1.4";
		mapping["1.3.2_pre"] = wxEmptyString;
		mapping["1.3.1_pre"] = wxEmptyString;
		mapping["1.3_pre"] = wxEmptyString;
		mapping["1.2_pre"] = "1.2";
	}
} ver;

wxString NostalgiaVersionToAssetsVersion(wxString nostalgia_version)
{
	auto iter = ver.mapping.find(nostalgia_version);
	if(iter != ver.mapping.end())
	{
		return (*iter).second;
	}
	return nostalgia_version;
}

DowngradeDialog::DowngradeDialog(wxWindow *parent)
	: ListSelectDialog(parent, _("Downgrade Instance"))
{
	// GUI customizations.
	wxHyperlinkCtrl *mcnLink = new wxHyperlinkCtrl(this, -1, _("Powered by MCNostalgia"),
		"http://www.minecraftforum.net/topic/800346-");
	auto cnt = dlgSizer->GetItemCount();
	dlgSizer->Insert(cnt-1,mcnLink,0, wxALL | wxEXPAND | wxALIGN_CENTER_VERTICAL, 4);
}

bool DowngradeDialog::DoLoadList()
{
	wxString vlistJSON;
	MCVersionList & ver_list = MCVersionList::Instance();
	ver_list.LoadIfNeeded();
	nice_names.Clear();
	if (DownloadString(mcnwebURL + "?pversion=1&list=True", &vlistJSON))
	{
		using namespace boost::property_tree;

		try
		{
			// Parse the JSON
			ptree pt;
			std::stringstream jsonStream(stdStr(vlistJSON), std::ios::in);
			read_json(jsonStream, pt);
			wxRegEx indevRegex("in(f)?dev");
			BOOST_FOREACH(const ptree::value_type& v, pt.get_child("order"))
			{
				auto rawVersion = wxStr(v.second.data());
				if(indevRegex.Matches(rawVersion))
					continue;
				auto niceVersion = NostalgiaVersionToAssetsVersion(rawVersion);
				if(niceVersion.empty())
					continue;
				if(ver_list.GetVersion(niceVersion))
					continue;
				sList.Insert(rawVersion, 0);
				nice_names.Insert(niceVersion,0);
			}
		}
		catch (json_parser_error e)
		{
			wxLogError(_("Failed to read version list.\nJSON parser error at line %i: %s"), 
				e.line(), wxStr(e.message()).c_str());
			return false;
		}
		return true;
	}
	else
	{
		wxLogError(_("Failed to get version list. Check your internet connection and try again later."));
		return false;
	}
}
MCVersion DowngradeDialog::GetSelectedVersion()
{
	int idx = GetSelectedIndex();
	if(idx == -1)
		return MCVersion();
	return MCVersion::getMCNVersion(sList[idx], nice_names[idx], wxEmptyString);
}
