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

InstallForgeDialog::InstallForgeDialog(wxWindow *parent, wxString intendedVersion)
	: ListSelectDialog(parent, _("Install Minecraft Forge"))
{
	// version filtering based on instance intended version
	m_intendedVersion = intendedVersion;
	m_filterVersions = m_intendedVersion != MCVer_Unknown;
	// Custom GUI stuff.
	wxClientDC dc(this);
	dc.SetFont(listCtrl->GetFont());
	int h,typeColumnWidth = 120;
	dc.GetTextExtent(_("Minecraft Version"), & typeColumnWidth, & h);
	typeColumnWidth += 10;
	
	wxSizerFlags btnSzFlags = wxSizerFlags(0).Border(wxBOTTOM, 4);
	m_changeLogButton = new wxButton(this, ID_ChangelogBtn, _("C&hangelog"));
	btnSz->Insert(1,m_changeLogButton,btnSzFlags.Align(wxALIGN_LEFT));
	// Clear columns and add our own.
	listCtrl->DeleteAllColumns();
	listCtrl->AppendColumn(_("Forge Version"), wxLIST_FORMAT_LEFT);
	listCtrl->AppendColumn(_("Minecraft Version"), wxLIST_FORMAT_RIGHT, typeColumnWidth);

	// Show column headers
	ShowHeader(true);
	Layout();
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
			bool valid = false;
			wxString url, changelogurl, jobbuildver, mcver;
			// for each file
			if(build.count("files")) BOOST_FOREACH(const ptree::value_type& v, build.get_child("files"))
			{
				const ptree & file = v.second;
				wxString buildtype = wxStr(file.get<std::string>("buildtype"));
				if(buildtype == "client" || buildtype == "universal")
				{
					mcver = wxStr(file.get<std::string>("mcver"));
					// if we are filtering based on MC versions and the version doesn't match
					if( m_filterVersions && mcver != m_intendedVersion )
					{
						break; // skip to next build
					}
					url = wxStr(file.get<std::string>("url"));
					jobbuildver = wxStr(file.get<std::string>("jobbuildver"));
					valid = true;
				}
				else if(buildtype == "changelog")
				{
					changelogurl = wxStr(file.get<std::string>("url"));
				}
			}
			if(valid)
				m_items.push_back(ForgeVersionItem(url,mcver,jobbuildver,changelogurl));
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
	return m_items[GetSelectedIndex()];
}

bool InstallForgeDialog::DoLoadList()
{
	wxString dlURL = "http://files.minecraftforge.net/minecraftforge/json";

	m_items.clear();
	
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
	listCtrl->SetItemCount(m_items.size());
}


wxString InstallForgeDialog::OnGetItemText(long item, long column)
{
	switch (column)
	{
	case 1:
		return m_items[item].MCVersion;

	default:
		return m_items[item].ForgeVersion;
	}
}

void InstallForgeDialog::OnChangelog ( wxCommandEvent& event )
{
	if(m_items.size() == 0)
		return;
	int selected = GetSelectedIndex();
	if(selected == -1)
	{
		selected = 0;
	}
	ForgeVersionItem & item = m_items[selected];
	Utils::OpenURL(item.ChangelogUrl);
}


BEGIN_EVENT_TABLE(InstallForgeDialog, ListSelectDialog)
	EVT_BUTTON(ID_ChangelogBtn, InstallForgeDialog::OnChangelog)
END_EVENT_TABLE()