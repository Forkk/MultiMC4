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
#include "instancectrl.h"
#include "fsutils.h"

#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>

InstanceModel::InstanceModel()
{
	m_control = nullptr;
	m_selectedIndex = -1;
	m_previousIndex = -1;
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
	m_previousIndex = -1;
	m_selectedIndex = -1;
	
	if(!m_freeze_level && m_control)
		m_control->ReloadAll();
	
}

std::size_t InstanceModel::Add (Instance * inst, bool do_select)
{
	auto idx = size();
	m_instances.push_back(inst);
	
	if(!m_freeze_level && m_control)
		m_control->ReloadAll();
	
	wxString ID = inst->GetInstID();
	if(m_groupMap.count(ID))
	{
		inst->SetGroup(m_groupMap[ID]);
	}
	inst->SetParentModel(this);
	return idx;
}

void InstanceModel::Remove (std::size_t index)
{
	auto inst = m_instances[index];
	delete inst;
	
	if(index == m_selectedIndex)
	{
		m_previousIndex = -1; // we are destroying it after all :)
		if(m_control)
			m_selectedIndex = m_control->GetSuggestedPostRemoveID(m_selectedIndex);
		else
			m_selectedIndex = -1;
	}
	
	m_instances.erase(m_instances.begin() + index);
	
	if(!m_freeze_level && m_control)
		m_control->ReloadAll();
}

void InstanceModel::Delete ( std::size_t index )
{
	auto inst = m_instances[index];
	fsutils::RecursiveDelete(inst->GetRootDir().GetFullPath());
	Remove(index);
}

void InstanceModel::DeleteCurrent()
{
	if(m_selectedIndex != -1)
		Delete(m_selectedIndex);
}

void InstanceModel::InstanceRenamed ( Instance* renamedInstance )
{
	if(m_freeze_level == 0 && m_control)
		m_control->ReloadAll();
}

void InstanceModel::SetLinkedControl ( InstanceCtrl* ctrl )
{
	m_control = ctrl;
	
	if(!m_freeze_level && m_control)
		m_control->ReloadAll();
}

void InstanceModel::Freeze()
{
	m_freeze_level ++;
};

void InstanceModel::Thaw()
{
	m_freeze_level --;
	if(m_freeze_level == 0 && m_control)
		m_control->ReloadAll();
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

void InstanceModel::InstanceGroupChanged ( Instance* changedInstance )
{
	wxString group = changedInstance->GetGroup();
	//TODO: notify control of group change
	if(group.empty())
		m_groupMap.erase(changedInstance->GetInstID());
	else
		m_groupMap[changedInstance->GetInstID()] = group;
	if(m_freeze_level == 0 && m_control)
		m_control->ReloadAll();
}
