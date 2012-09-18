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

#include <wx/dialog.h>
#include <wx/string.h>
#include <wx/icon.h>

class AboutDlgInfo
{
public:
	wxString name;
	wxString version;
	wxString description;
	wxString copyright;

	wxString website;
	wxString license;

	wxIcon icon;
};

class AboutDlg : public wxDialog
{
public:
	AboutDlg(wxWindow *parent, const AboutDlgInfo &info);

protected:
	void OnViewLicense(wxCommandEvent& event);

	wxString m_licenseText;

	DECLARE_EVENT_TABLE();
};

enum
{
	ID_License,
};
