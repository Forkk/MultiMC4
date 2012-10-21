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

#include "instlist.h"

#include "data/instance.h"

#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>

int CompareInstances(Instance *first, Instance *second)
{
	return wxStricmp(first->GetName(), second->GetName());
}

InstList::InstList()
	//: InstListBase(&CompareInstances)
{

}

bool InstList::LoadGroupInfo(const wxString& file)
{
	m_groupMap.clear();

	using namespace boost::property_tree;
	ptree pt;

	try
	{
		read_json(stdStr(file), pt);

		BOOST_FOREACH(const ptree::value_type& v, pt.get_child("groups"))
		{
			m_groupMap[wxStr(v.first.data())] = wxStr(v.second.data());
		}
	}
	catch (json_parser_error e)
	{
		wxLogError(_("Failed to read group list.\nJSON parser error at line %i: %s"), 
			e.line(), wxStr(e.message()).c_str());
		return false;
	}
	return true;
}

bool InstList::SaveGroupInfo(const wxString& file) const
{
	using namespace boost::property_tree;
	ptree pt;

	try
	{
		ptree subPT;
		for (GroupMap::const_iterator iter = m_groupMap.begin(); iter != m_groupMap.end(); iter++)
		{
			subPT.push_back(std::make_pair(stdStr(iter->first), stdStr(iter->second)));
		}
		pt.add_child("groups", subPT);

		write_json(stdStr(file), pt);
	}
	catch (json_parser_error e)
	{
		wxLogError(_("Failed to read group list.\nJSON parser error at line %i: %s"), 
			e.line(), wxStr(e.message()).c_str());
		return false;
	}

	return true;
}

wxString InstList::GetGroup(Instance* inst)
{
	return m_groupMap[inst->GetInstID()];
}

void InstList::SetGroup(Instance *inst, const wxString& group)
{
	m_groupMap[inst->GetInstID()] = group;
}

#include <wx/listimpl.cpp>
WX_DEFINE_LIST(InstListBase)
