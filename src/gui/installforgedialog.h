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
#include "listselectdialog.h"
#include "mcversionlist.h"
#include <vector>

struct ForgeVersionItem
{
	ForgeVersionItem(wxString _Url, wxString _MCVersion, wxString _ForgeVersion, wxString _ChangelogUrl, wxString _Filename)
	:Url(_Url),MCVersion(_MCVersion),ForgeVersion(_ForgeVersion),ChangelogUrl(_ChangelogUrl),Filename(_Filename){};
	wxString Url;
	wxString MCVersion;
	wxString ForgeVersion;
	wxString ChangelogUrl;
	wxString Filename;
	//TODO: maybe define a sorting predicate.
};

class InstallForgeDialog : public ListSelectDialog
{
public:
	InstallForgeDialog( wxWindow* parent, wxString jarVersion = MCVer_Unknown );
	ForgeVersionItem & GetSelectedItem();
protected:
	bool ParseForgeJson( wxString file );
	virtual bool DoLoadList();
	virtual void OnSelectionChange();
	virtual wxString OnGetItemText(long item, long column);
	void OnChangelog(wxCommandEvent& event);
	
	virtual void UpdateListCount();
	
	wxButton * m_changeLogButton;
	std::vector<ForgeVersionItem> m_items;
	wxString m_jarVersion;
	bool m_filterVersions;
	enum
	{
		ID_RefreshList,
		ID_ChangelogBtn,
	};
	DECLARE_EVENT_TABLE();
};
