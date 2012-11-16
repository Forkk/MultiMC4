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

#include "lwjgldialog.h"

#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>

#include <sstream>

#include <wx/regex.h>
#include <wx/numformatter.h>

#include "httputils.h"
#include "apputils.h"

const wxString rssURL = "http://sourceforge.net/api/file/index/project-id/58488/mtime/desc/rss";

ChooseLWJGLDialog::ChooseLWJGLDialog(wxWindow *parent)
	: ListSelectDialog(parent, _("Choose LWJGL Version"))
{

}

void ChooseLWJGLDialog::LoadList()
{
	linkList.Clear();

	ListSelectDialog::LoadList();
}

bool ChooseLWJGLDialog::DoLoadList()
{
	// Parse XML from the given URL.
	wxString rssXML = wxEmptyString;
	if (!DownloadString(rssURL, &rssXML))
	{
		wxLogError(_("Failed to get RSS feed. Check your internet connection."));
		return false;
	}

	using namespace boost::property_tree;
	try
	{
		ptree pt;
		std::stringstream inStream(stdStr(rssXML), std::ios::in);
		read_xml(inStream, pt);

		wxRegEx lwjglRegex(wxT("^lwjgl-([0-9]\\.?)+\\.zip$"));

		BOOST_FOREACH(const ptree::value_type& v, pt.get_child("rss.channel"))
		{
			if (v.first == "item")
			{
				wxString link = wxStr(v.second.get<std::string>("link"));

				// Look for download links.
				if (link.EndsWith("/download"))
				{
					wxString name = link.BeforeLast('/');
					name = name.AfterLast('/');

					if (lwjglRegex.Matches(name))
					{
						sList.Add(name);
						linkList.Add(link);
					}
				}
			}
		}
	}
	catch (xml_parser_error e)
	{
		wxLogError(_("Failed to parse LWJGL list.\nXML parser error at line %i: %s"), 
			e.line(), wxStr(e.message()).c_str());
		return false;
	}

	return true;
}

wxString ChooseLWJGLDialog::GetSelectedURL()
{
	return linkList[GetSelectedIndex()];
}
