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

#include "newschecktask.h"

#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>

#include <sstream>

#include "utils/httputils.h"
#include "utils/apputils.h"

const wxString rssURL = "http://news.forkk.net/feed/rss2";

NewsCheckTask::NewsCheckTask()
	: Task()
{

}

wxThread::ExitCode NewsCheckTask::TaskStart()
{
	m_latestPostTitle = _("Failed to load news RSS feed. Click here to go to the dev blog's homepage.");
	m_latestPostURL = _("http://forkk.net/mmcnews.php");

	// Attempt to get the RSS feed.
	wxString rssXML = wxEmptyString;
	bool failed = false;
	if (!DownloadString(rssURL, &rssXML))
	{
		failed = true;
	}
	using namespace boost::property_tree;
	if (!failed)
		try
	{
		ptree pt;
		std::stringstream inStream(stdStr(rssXML), std::ios::in);
		read_xml(inStream, pt);

		if (pt.count("rss")) BOOST_FOREACH(const ptree::value_type& v, pt.get_child("rss.channel"))
		{
			if (v.first == "item")
			{
				auto _title = v.second.get_optional<std::string>("title");
				auto _link = v.second.get_optional<std::string>("link");
				if (!_title || !_link)
					continue;

				m_latestPostTitle = wxStr(_title.get());
				m_latestPostURL = wxStr(_link.get());

				break;
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

	return (ExitCode) failed;
}

wxString NewsCheckTask::GetLatestPostTitle() const
{
	return m_latestPostTitle;
}

wxString NewsCheckTask::GetLatestPostURL() const
{
	return m_latestPostURL;
}
