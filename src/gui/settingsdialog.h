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

#pragma once
#include <wx/wx.h>
#include <wx/spinctrl.h>
#include <wx/tglbtn.h>
#include <wx/notebook.h>

#include "appsettings.h"

class SettingsDialog : public wxDialog
{
public:
	SettingsDialog(wxWindow *parent, wxWindowID id);
	
	void OnButtonClicked(wxCommandEvent& event);
	
	void ApplySettings(AppSettings &s = settings);
	void LoadSettings(AppSettings &s = settings);
	
protected:
	void OnBrowseInstDirClicked(wxCommandEvent &event);
	
	wxNotebook *tabCtrl;
	
	wxCheckBox *showConsoleCheck;
	wxCheckBox *autoCloseConsoleCheck;
	
	wxCheckBox *autoUpdateCheck;
	wxToggleButton *forceUpdateToggle;
	
	wxTextCtrl *instDirTextBox;
	
	wxSpinCtrl *minMemorySpin;
	wxSpinCtrl *maxMemorySpin;
	
	wxTextCtrl *javaPathTextBox;
	
	wxComboBox *guiStyleDropDown;
	
	DECLARE_EVENT_TABLE()
};

enum
{
	ID_BrowseInstDir,
};
