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
#include <wx/numformatter.h>

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
	return true;
}

void SnapshotList::Sort(bool descending)
{
	if (descending)
		wxArrayString::Sort(CompareSnapshotsDescending);
	else
		wxArrayString::Sort(CompareSnapshotsAscending);
}

struct SnapshotDef
{
	SnapshotDef()
	{
		m_year = 0;
		m_week = 0;
		m_char = 0;
	}

	SnapshotDef(long year, long week, char letter)
	{
		m_year = year;
		m_week = week;
		m_char = letter;
	}

	bool ParseString(wxString str)
	{
		wxString yearStr = str.BeforeFirst(wxT('w'));
		wxString weekStr = str.AfterFirst(wxT('w')).RemoveLast();
		m_char = str.Right(1)[0];

		return wxNumberFormatter::FromString(yearStr, &m_year) &&
			wxNumberFormatter::FromString(weekStr, &m_week);
	}

	int CompareTo(SnapshotDef *other)
	{
		if (this->m_year > other->m_year)
			return 1;
		else if (this->m_year < other->m_year)
			return -1;

		else if (this->m_week > other->m_week)
			return 1;
		else if (this->m_week < other->m_week)
			return -1;

		else if (this->m_char > other->m_char)
			return 1;
		else if (this->m_char < other->m_char)
			return -1;

		else return 0;
	}

	long m_year; // The first numbers in the snapshot's string.
	long m_week; // The second two numbers in the snapshot's string.
	wxChar m_char; // The letter at the end of the snapshot's string.
};

int SnapshotList::CompareSnapshots(wxString *first, wxString *second, bool reverse)
{
	SnapshotDef firstDef;
	SnapshotDef secondDef;

	bool parseSuccess = firstDef.ParseString(*first) && secondDef.ParseString(*second);
	wxASSERT_MSG(parseSuccess, "Failed to parse snapshot strings.");
	if (!parseSuccess)
		return 0;

	if (reverse)
		return -firstDef.CompareTo(&secondDef);
	else
		return firstDef.CompareTo(&secondDef);
}
