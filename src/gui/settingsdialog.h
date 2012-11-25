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
#include <wx/wx.h>
#include <wx/spinctrl.h>
#include <wx/tglbtn.h>
#include <wx/notebook.h>
#include <wx/clrpicker.h>

#include "appsettings.h"

class SettingsDialog : public wxDialog
{
public:
	SettingsDialog(wxWindow *parent, wxWindowID id, SettingsBase *s = settings);
	bool GetForceUpdateMultiMC() const;

	bool ShouldRestartNow() const;

protected:
	void OnBrowseInstDirClicked(wxCommandEvent& event);
	void OnBrowseModsDirClicked(wxCommandEvent& event);
	void OnDetectJavaPathClicked(wxCommandEvent& event);
	void OnUpdateCheckboxes(wxCommandEvent& event);
	void OnOKClicked(wxCommandEvent& event);

	void UpdateCheckboxStuff();
	void LoadSettings();
	bool ApplySettings();
	bool FolderMove( wxFileName oldDir, wxFileName newDir, wxString message, wxString title);
	
	SettingsBase * currentSettings;
	wxNotebook *tabCtrl;
	bool instanceMode;
	
	// console tab stuff
	wxCheckBox *showConsoleCheck;
	wxCheckBox *autoCloseConsoleCheck;
	wxColourPickerCtrl *sysMsgColorCtrl;
	wxColourPickerCtrl *stdoutColorCtrl;
	wxColourPickerCtrl *stderrColorCtrl;
	
	// multimc tab stuff
	wxCheckBox *autoUpdateCheck;
	wxCheckBox *useDevBuildsCheck;
	wxToggleButton *forceUpdateToggle;
	wxRadioBox *guiStyleBox;
	wxRadioBox *sortModeBox;
	wxComboBox *langSelectorBox;
	wxCheckBox *useSystemLangCheck;
	wxTextCtrl *instDirTextBox;
	wxTextCtrl *modsDirTextBox;
	

	// java tab stuff
	wxCheckBox *javaUseDefs;
	wxTextCtrl *javaPathTextBox;
	wxTextCtrl *jvmArgsTextBox;
	wxButton *autoDetectButton;
	wxStaticText *jvmArgsLabel;
	wxStaticText *javaPathLabel;
	
	wxCheckBox *memoryUseDefs;
	wxSpinCtrl *minMemorySpin;
	wxSpinCtrl *maxMemorySpin;
	wxStaticText *minMemLabel;
	wxStaticText *maxMemLabel;

	// minecraft tab stuff
	wxCheckBox *winUseDefs;
	wxCheckBox *compatCheckbox;
	wxCheckBox *winMaxCheckbox;
	wxSpinCtrl *winWidthSpin;
	wxSpinCtrl *winHeightSpin;
	wxStaticText *winHeightLabel;
	wxStaticText *winWidthLabel;
	
	wxCheckBox *updateUseDefs;
	wxComboBox *mcUpdateDropDown;

	wxCheckBox *loginUseDefs;
	wxCheckBox *autoLoginCheck;


	// Network tab stuff
	wxRadioButton* noProxyRBtn;
	wxRadioButton* httpProxyRBtn;
	wxRadioButton* socks4ProxyRBtn;
	wxRadioButton* socks5ProxyRBtn;
	wxTextCtrl* proxyHostTextbox;
	wxTextCtrl* proxyPortTextbox;
	wxTextCtrl* proxyUserTextbox;
	wxTextCtrl* proxyPassTextbox;

	long proxyPortValue;


	// Other stuff
	bool m_shouldRestartMMC;

	DECLARE_EVENT_TABLE()
};

enum
{
	ID_BrowseInstDir,
	ID_BrowseModDir,
	ID_DetectJavaPath,

	ID_MCMaximizeCheckbox,
	ID_CompatModeCheckbox,
	ID_OverrideJava,
	ID_OverrideWindow,
	ID_OverrideUpdate,
	ID_OverrideMemory,
	ID_OverrideLogin,

	ID_UseSystemLang,

	ID_UseProxy,
};
