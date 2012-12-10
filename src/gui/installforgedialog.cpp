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

#include "installforgedialog.h"

#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>

#include <wx/gbsizer.h>
#include <wx/regex.h>

#include "utils/apputils.h"
#include "utils/httputils.h"
#include "forgeversions.h"

InstallForgeDialog::InstallForgeDialog(wxWindow *parent)
	: ListSelectDialog(parent, _("Install Minecraft Forge"))
{
	// Custom GUI stuff.

	wxClientDC dc(this);
	dc.SetFont(listCtrl->GetFont());
	int h,typeColumnWidth = 120;
	dc.GetTextExtent(_("Minecraft Version"), & typeColumnWidth, & h);
	typeColumnWidth += 10;
	
	// Clear columns and add our own.
	listCtrl->DeleteAllColumns();
	listCtrl->AppendColumn(_("Forge Version"), wxLIST_FORMAT_LEFT);
	listCtrl->AppendColumn(_("Minecraft Version"), wxLIST_FORMAT_RIGHT, typeColumnWidth);

	// Show column headers
	ShowHeader(true);
}

bool InstallForgeDialog::ParseForgeJson(wxString file)
{
	using namespace boost::property_tree;
	try
	{
		ptree pt;
		std::stringstream jsonStream(stdStr(file), std::ios::in);
		read_json(jsonStream, pt);

		// for each build
		if(pt.count("builds")) BOOST_FOREACH(const ptree::value_type& v, pt.get_child("builds"))
		{
			const ptree & build = v.second;
			// for each file
			if(build.count("files")) BOOST_FOREACH(const ptree::value_type& v, build.get_child("files"))
			{
				const ptree & file = v.second;
				wxString buildtype = wxStr(file.get<std::string>("buildtype"));
				if(buildtype != "client" && buildtype != "universal")
					continue;
				wxString url = wxStr(file.get<std::string>("url"));
				wxString jobbuildver = wxStr(file.get<std::string>("jobbuildver"));
				wxString mcver = wxStr(file.get<std::string>("mcver"));
				items.push_back(ForgeVersionItem(url,mcver,jobbuildver));
			}
		}
	}
	catch (json_parser_error e)
	{
		wxLogError(_("Failed to read the forge version list.\nError on line %i: %s"),
			e.line(), wxStr(e.message()).c_str());
		return false;
	}
	catch (ptree_error)
	{
		wxLogError(_("Failed to read the forge version list."));
		return false;
	}
	return true;
}

ForgeVersionItem& InstallForgeDialog::GetSelectedItem()
{
	return items[GetSelectedIndex()];
}

bool InstallForgeDialog::DoLoadList()
{
	wxString dlURL = "http://files.minecraftforge.net/minecraftforge/json";

	items.clear();
	
	wxString buildListText;
	if (DownloadString(dlURL, &buildListText))
	{
		return ParseForgeJson(buildListText);
	}
	else
	{
		wxLogError(_("Failed to load forge version list. Check your internet connection."));
		return false;
	}

	return true;
}

void InstallForgeDialog::UpdateListCount()
{
	listCtrl->SetItemCount(items.size());
}


wxString InstallForgeDialog::OnGetItemText(long item, long column)
{
	switch (column)
	{
	case 1:
		return items[item].MCVersion;

	default:
		return items[item].ForgeVersion;
	}
}
