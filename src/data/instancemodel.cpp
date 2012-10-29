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

bool InstanceModel::LoadGroupInfo(wxString file)
{
	if (file.IsEmpty())
		file = m_groupFile;

	m_groupMap.clear();

	using namespace boost::property_tree;
	ptree pt;

	try
	{
		read_json(stdStr(file), pt);

		BOOST_FOREACH(const ptree::value_type& vp, pt.get_child("groups"))
		{
			ptree gPt = vp.second;
			wxString groupName = wxStr(vp.first);

			m_groups.push_back(new InstanceGroup(groupName, this));

			wxArrayString groupInstances;
			BOOST_FOREACH(const ptree::value_type& v, gPt.get_child("instances"))
			{
				groupInstances.Add(wxStr(v.second.data()));
			}

			for (InstVector::iterator iter = m_instances.begin(); iter < m_instances.end(); iter++)
			{
				if (groupInstances.Index((*iter)->GetInstID()) != wxNOT_FOUND)
					SetInstanceGroup(*iter, groupName);
			}
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

bool InstanceModel::SaveGroupInfo(wxString file) const
{
	if (file.IsEmpty())
		file = m_groupFile;

	using namespace boost::property_tree;
	ptree pt;

	try
	{
		typedef std::map<InstanceGroup *, InstVector> GroupListMap;

		GroupListMap groupLists;
		for (InstVector::const_iterator iter = m_instances.begin(); iter != m_instances.end(); iter++)
		{
			InstanceGroup *group = GetInstanceGroup(*iter);
			
			if (group != nullptr)
				groupLists[group].push_back(*iter);
		}

		ptree groupsPtree;
		for (GroupListMap::iterator iter = groupLists.begin(); iter != groupLists.end(); iter++)
		{
			InstanceGroup *group = iter->first;
			InstVector gList = iter->second;

			ptree groupTree;

			ptree instList;
			for (InstVector::iterator iter = gList.begin(); iter != gList.end(); iter++)
			{
				instList.push_back(std::make_pair("", stdStr((*iter)->GetInstID())));
			}
			groupTree.put_child("instances", instList);

			groupsPtree.push_back(std::make_pair(stdStr(group->GetName()), groupTree));
		}
		pt.put_child("groups", groupsPtree);

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
	SaveGroupInfo();
	if(m_freeze_level == 0 && m_control)
		m_control->ReloadAll();
}

void InstanceModel::SetGroupFile(const wxString& groupFile)
{
	m_groupFile = groupFile;
}

void InstanceModel::SetInstanceGroup(Instance *inst, wxString groupName)
{
	InstanceGroup *prevGroup = GetInstanceGroup(inst);

	if (prevGroup != nullptr)
	{
		m_groupMap.erase(inst);
	}

	if (!groupName.IsEmpty())
	{
		InstanceGroup *newGroup = nullptr;

		for (GroupVector::iterator iter = m_groups.begin(); iter != m_groups.end(); iter++)
		{
			if ((*iter)->GetName() == groupName)
			{
				newGroup = *iter;
			}
		}

		if (newGroup == nullptr)
		{
			newGroup = new InstanceGroup(groupName, this);
			m_groups.push_back(newGroup);
		}

		m_groupMap[inst] = newGroup;
	}

	InstanceGroupChanged(inst);
}

InstanceGroup *InstanceModel::GetInstanceGroup(Instance *inst) const
{
	if (m_groupMap.count(inst))
		return m_groupMap.at(inst);
	else
		return nullptr;
}


InstanceGroup::InstanceGroup(const wxString& name, InstanceModel *parent)
{
	m_name = name;
	m_parent = parent;
}

wxString InstanceGroup::GetName() const
{
	return m_name;
}

void InstanceGroup::SetName(const wxString& name)
{
	m_name = name;
}

InstanceModel* InstanceGroup::GetParent() const
{
	return m_parent;
}
