// 
//  Copyright 2012 Andrew Okin
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

#include "snapshotlist.h"

#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>

#include <sstream>

#include <wx/regex.h>

#include "httputils.h"
#include "apputils.h"

SnapshotList::SnapshotList()
{

}

bool SnapshotList::LoadFromURL(wxString url)
{
	// Parse XML from the given URL.
	wxString assetsXML = wxEmptyString;
	if (!DownloadString(url, &assetsXML))
	{
		wxLogError(_("Failed to get snapshot list. Check your internet connection."));
		return false;
	}

	using namespace boost::property_tree;
	try
	{
		ptree pt;
		std::stringstream inStream(stdStr(assetsXML), std::ios::in);
		read_xml(inStream, pt);

		wxRegEx snapshotRegex(wxT("^[0-9][0-9]w[0-9][0-9][a-z]/$"));

		BOOST_FOREACH(const ptree::value_type& v, pt.get_child("ListBucketResult"))
		{
			if (v.first == "Contents")
			{
				wxString keyName = wxStr(v.second.get<std::string>("Key"));

				if (snapshotRegex.Matches(keyName))
				{
					if (keyName.EndsWith(wxT("/")))
						keyName = keyName.Left(keyName.Len() - 1);
					push_back(keyName);
				}
			}
		}
	}
	catch (xml_parser_error e)
	{
		wxLogError(_("Failed to parse snapshot list.\nXML parser error at line %i: %s"), 
			e.line(), wxStr(e.message()).c_str());
	}

	// ^[0-9][0-9]w[0-9][0-9][a-z]/minecraft.jar$
}
