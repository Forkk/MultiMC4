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

#include "apputils.h"
#include "httputils.h"

const wxString mcnwebURL = "http://sonicrules.org/mcnweb.py";

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
	if (DownloadString(mcnwebURL + "?pversion=1&list=True", &vlistJSON))
	{
		using namespace boost::property_tree;

		try
		{
			// Parse the JSON
			ptree pt;
			std::stringstream jsonStream(stdStr(vlistJSON), std::ios::in);
			read_json(jsonStream, pt);
			wxRegEx snapshotRegex("^[0-9][0-9]w[0-9][0-9][a-z]$");
			wxRegEx indevRegex("in(f)?dev");
			wxRegEx preRegex("pre");
			wxRegEx rcRegex("rc");
			BOOST_FOREACH(const ptree::value_type& v, pt.get_child("order"))
			{
				auto str = wxStr(v.second.data());
				if(snapshotRegex.Matches(str) || indevRegex.Matches(str)
					|| preRegex.Matches(str) || rcRegex.Matches(str))
					continue;
				sList.Insert(str, 0);
			}
		}
		catch (json_parser_error e)
		{
			wxLogError(_("Failed to read version list.\nJSON parser error at line %i: %s"), 
				e.line(), wxStr(e.message()).c_str());
			return false;
		}
	}
	else
	{
		wxLogError(_("Failed to get version list. Check your internet connection and try again later."));
		return false;
	}
}
