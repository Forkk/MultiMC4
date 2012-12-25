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

#include "lwjglversionlist.h"
#include "appsettings.h"

#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>

#include <sstream>

#include <wx/regex.h>
#include <wx/numformatter.h>
#include <wx/dir.h>

#include "utils/httputils.h"
#include "utils/apputils.h"

const wxString rssURL = "http://sourceforge.net/api/file/index/project-id/58488/mtime/desc/rss";

LWJGLVersionList* LWJGLVersionList::pInstance = 0;

LWJGLVersionList::LWJGLVersionList() {}

LWJGLVersion* LWJGLVersionList::GetVersion ( wxString name )
{
	for(auto iter = versions.begin(); iter != versions.end() ; iter++)
	{
		LWJGLVersion & v = *iter;
		wxString descr = v.GetName();
		if( descr == name )
			return &v;
	}
	return nullptr;
}

bool LWJGLVersionList::LoadIfNeeded()
{
	if(NeedsLoad())
	{
		return Reload();
	}
	return true;
}

bool LWJGLVersionList::Reload()
{
	versions.push_back(LWJGLVersion("Mojang",""));
	
	// Parse XML from the given URL.
	wxString rssXML = wxEmptyString;
	bool failed = false;
	if (!DownloadString(rssURL, &rssXML))
	{
		//wxLogError(_("Failed to get RSS feed. Check your internet connection."));
		failed = true;
	}
	using namespace boost::property_tree;
	if(!failed)
	try
	{
		ptree pt;
		std::stringstream inStream(stdStr(rssXML), std::ios::in);
		read_xml(inStream, pt);

		wxRegEx lwjglRegex("^lwjgl-(([0-9]\\.?)+)\\.zip$");

		if(pt.count("rss")) BOOST_FOREACH(const ptree::value_type& v, pt.get_child("rss.channel"))
		{
			if (v.first == "item")
			{
				auto val = v.second.get_optional<std::string>("link");
				if(!val)
					continue;
				wxString link = wxStr(val.get());

				// Look for download links.
				if (link.EndsWith("/download"))
				{
					wxString name = link.BeforeLast('/');
					name = name.AfterLast('/');

					if (lwjglRegex.Matches(name))
					{
						wxString version = lwjglRegex.GetMatch(name,1);
						versions.push_back(LWJGLVersion(version, link));
					}
				}
			}
		}
		else
		{
			failed = true;
		}
	}
	catch (ptree_error)
	{
		failed = true;
	}
	if(failed)
	{
		wxDir dir(settings->GetLwjglDir().GetFullPath());
		if ( !dir.IsOpened() )
			return false;

		wxString dirName = dir.GetName();

		wxString str;
		if(dir.GetFirst(&str, wxEmptyString, wxDIR_DIRS))
		{
			versions.push_back(LWJGLVersion(str,wxEmptyString));
		}
		while(dir.GetNext(&str))
		{
			versions.push_back(LWJGLVersion(str,wxEmptyString));
		}
		std::sort(versions.begin(),versions.end(),
			[](const LWJGLVersion & left, const LWJGLVersion & right)
			{
				return left.GetName().Lower() > right.GetName().Lower();
			}
		);
		return false;
	}
	return true;
}
