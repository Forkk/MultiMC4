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

#pragma once

#include <wx/list.h>
#include <map>
#include <vector>

class Instance;
class wxInstanceCtrl;

typedef std::map<wxString, wxString> GroupMap;

class InstanceModel
{
public:
	InstanceModel();
	~InstanceModel();

	/// get the number of stored instances
	std::size_t size() const
	{
		return m_instances.size();
	}
	
	/// get an instance given its index...
	Instance * operator[](const std::size_t index) const;
	
	/// Link an instance control with this model
	void SetLinkedControl( wxInstanceCtrl * ctrl );
	
	/// clear all instances
	void Clear();
	
	/// Add a new instance and return its index
	std::size_t Add (Instance * inst);
	
	/// remove instance by index (and destroy it)
	void Remove (std::size_t index);
	
	/// Prevent the model from updating the control until thawed again (for batching changes)
	void Freeze();
	/// Allow updating the control again
	void Thaw();
	
	/// Gets the given instance's group.
	wxString GetGroup(Instance* inst);

	/// Sets the given instance's group.
	void SetGroup(Instance* inst, const wxString& group);

	bool LoadGroupInfo(const wxString& file);
	bool SaveGroupInfo(const wxString& file) const;

protected:
	// mapping between instances and groups...
	GroupMap m_groupMap;
	// our list of instances :D
	std::vector <Instance *> m_instances;
	// the control to notify about changes
	wxInstanceCtrl * m_control;
	// determines if updates to the control should be postponed
	unsigned int m_freeze_level;
};
