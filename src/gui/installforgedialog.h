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
#include <vector>

struct ForgeVersionItem
{
	ForgeVersionItem(wxString _Filename, wxString _MCVersion, wxString _ForgeVersion)
	:Filename(_Filename),MCVersion(_MCVersion),ForgeVersion(_ForgeVersion){};
	wxString Filename;
	wxString MCVersion;
	wxString ForgeVersion;
	//TODO: maybe define a sorting predicate.
};

class InstallForgeDialog : public ListSelectDialog
{
public:
	InstallForgeDialog(wxWindow *parent);

protected:
	virtual bool DoLoadList();
	virtual wxString OnGetItemText(long item, long column);
	
	virtual void UpdateListCount();
	std::vector<ForgeVersionItem> items;
};
