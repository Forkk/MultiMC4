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

#include "instancemodel.h"

#include "data/instance.h"
#include <instancectrl.h>

#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>

InstanceModel::InstanceModel()
{
	m_control = nullptr;
	m_freeze_level = 0;
}

InstanceModel::~InstanceModel()
{
	Clear();
}

Instance * InstanceModel::operator[](const std::size_t index) const
{
	if(index >= size())
	{
		return nullptr;
	}
	return m_instances[index];
};

void InstanceModel::Clear()
{
	for(int i = 0; i < size(); i++)
		delete m_instances[i];
	m_instances.clear();
	
	if(!m_freeze_level && m_control)
		m_control->UpdateItems();
	
}

std::size_t InstanceModel::Add (Instance * inst)
{
	auto idx = size();
	m_instances.push_back(inst);
	
	if(!m_freeze_level && m_control)
		m_control->UpdateItems();
	
	return idx;
}

void InstanceModel::Remove (std::size_t index)
{
	auto inst = m_instances[index];
	delete inst;
	m_instances.erase(m_instances.begin() + index);
	
	if(!m_freeze_level && m_control)
		m_control->UpdateItems();
}

void InstanceModel::SetLinkedControl ( wxInstanceCtrl* ctrl )
{
	m_control = ctrl;
	
	if(!m_freeze_level && m_control)
		m_control->UpdateItems();
}

void InstanceModel::Freeze()
{
	m_freeze_level ++;
	if(m_control)
	{
		m_control->Freeze();
	}
};

void InstanceModel::Thaw()
{
	m_freeze_level --;
	if(m_freeze_level == 0 && m_control)
		m_control->UpdateItems();
	if(m_control)
	{
		m_control->Thaw();
	}
};

bool InstanceModel::LoadGroupInfo(const wxString& file)
{
	m_groupMap.clear();

	using namespace boost::property_tree;
	ptree pt;

	try
	{
		read_json(stdStr(file), pt);

		BOOST_FOREACH(const ptree::value_type& v, pt.get_child("groups"))
		{
			auto value = v.second.data();
			auto key = v.first.data();
			if(!value.empty())
				m_groupMap[key] = wxStr(value);
		}
	}
	catch (json_parser_error e)
	{
		//FIXME: control is left in weird state/desynchronized
		wxLogError(_("Failed to read group list.\nJSON parser error at line %i: %s"), 
			e.line(), wxStr(e.message()).c_str());
		return false;
	}
	return true;
}

bool InstanceModel::SaveGroupInfo(const wxString& file) const
{
	using namespace boost::property_tree;
	ptree pt;

	try
	{
		ptree subPT;
		for (GroupMap::const_iterator iter = m_groupMap.begin(); iter != m_groupMap.end(); iter++)
		{
			ptree leaf (stdStr(iter->second));
			auto pair = std::make_pair(stdStr(iter->first), leaf);
			subPT.push_back(pair);
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

wxString InstanceModel::GetGroup(Instance* inst)
{
	return m_groupMap[inst->GetInstID()];
}

void InstanceModel::SetGroup(Instance *inst, const wxString& group)
{
	//TODO: notify control of group change
	if(group.empty())
		m_groupMap.erase(inst->GetInstID());
	else
		m_groupMap[inst->GetInstID()] = group;
}
