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
class InstanceCtrl;

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
	void SetLinkedControl( InstanceCtrl * ctrl );
	
	/// clear all instances
	void Clear();
	
	/// Add a new instance and return its index. if do_select is true, the instance will be selected in the linked control
	std::size_t Add (Instance * inst, bool do_select = false);
	
	/// remove instance by index (and destroy the object)
	void Remove (std::size_t index);
	
	/// Delete instance by index (this includes all its files)
	void Delete (std::size_t index);
	
	/// delete the currently selected instance and all its files
	void DeleteCurrent ();
	
	/// Prevent the model from updating the control until thawed again (for batching changes)
	void Freeze();
	/// Allow updating the control again
	void Thaw();

	bool LoadGroupInfo(wxString file = wxEmptyString);
	bool SaveGroupInfo(wxString file = wxEmptyString) const;

	Instance *GetSelectedInstance()
	{
		if(m_selectedIndex == -1)
			return nullptr;
		return m_instances[m_selectedIndex];
	};
	
	Instance *GetPreviousInstance()
	{
		if(m_previousIndex == -1)
			return nullptr;
		return m_instances[m_previousIndex];
	};
	
	/// Set the selection from the linked control side. We do not notify the originating control.
	void CtrlSelectInstance ( int clickedID )
	{
		m_previousIndex = m_selectedIndex;
		m_selectedIndex = clickedID;
	};
	int GetSelectedIndex()
	{
		return m_selectedIndex;
	};
	int GetPreviousIndex()
	{
		return m_previousIndex;
	};
	void InstanceRenamed ( Instance* renamedInstance );
	void InstanceGroupChanged ( Instance* changedInstance );

	void SetGroupFile(const wxString& groupFile);
	
protected:
	// mapping between instances and groups...
	GroupMap m_groupMap;
	// our list of instances :D
	std::vector <Instance *> m_instances;
	// previously selected instance (index)
	int m_previousIndex;
	// currently selected instance (index)
	int m_selectedIndex;
	// the control to notify about changes
	InstanceCtrl * m_control;
	// determines if updates to the control should be postponed
	unsigned int m_freeze_level;

	wxString m_groupFile;
};
