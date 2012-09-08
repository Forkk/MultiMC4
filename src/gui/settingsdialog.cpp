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

#include "settingsdialog.h"
#include <wx/gbsizer.h>
#include <wx/dir.h>
#include <apputils.h>
#include <fsutils.h>

const wxString guiModeDefault = _("Default");
const wxString guiModeSimple = _("Simple");
const wxString compadModeHelp =_(
"Compatibility mode launches Minecraft\
using the old Minecraft.main() method, rather than using MultiMC's\
AppletWrapper. This should help fix a few issues with certain mods,\
but it will disable certain features such as setting Minecraft's window\
size and icon."
);

SettingsDialog::SettingsDialog(wxWindow *parent, wxWindowID id, AppSettings *_settings /* = settings */)
	: wxDialog(parent, id, _T("Settings"), wxDefaultPosition, wxSize(500, 450))
{
	currentSettings = _settings;
	wxBoxSizer *mainBox = new wxBoxSizer(wxVERTICAL);
	SetSizer(mainBox);
	
	// Tab control
	tabCtrl = new wxNotebook(this, -1);
	mainBox->Add(tabCtrl, 1, wxEXPAND | wxALL, 8);
	
	// common sizer flags
	auto staticBoxInnerFlags = wxSizerFlags().Expand();
	auto staticBoxOuterFlags = wxSizerFlags().Border(wxALL,4).Expand();
	// single item in a row
	auto expandingItemFlags  = wxSizerFlags(1).Border(wxALL,4).Expand();
	auto itemFlags           = wxSizerFlags().Border(wxALL,4);
	// two items in a row - vertically aligned
	auto itemsFlags          = wxSizerFlags().Border(wxALL,4).Align(wxALIGN_CENTER_VERTICAL);
	auto expandingItemsFlags = wxSizerFlags(1).Border(wxALL,4).Expand().Align(wxALIGN_CENTER_VERTICAL);
	// crappy gridbag hax
	#define GBitemFlags wxALL, 4
	#define GBitemsFlags wxALL | wxALIGN_CENTER_VERTICAL, 4
	#define GBexpandingItemsFlags wxALL | wxALIGN_CENTER_VERTICAL | wxEXPAND, 4
	
	
	// General tab
	{
		auto generalPanel = new wxPanel(tabCtrl, -1);
		auto generalBox = new wxBoxSizer(wxVERTICAL);
		generalPanel->SetSizer(generalBox);
		tabCtrl->AddPage(generalPanel, _T("General"), true);
		// Visual settings group box
		{
			auto visualBox = new wxStaticBoxSizer(wxVERTICAL, generalPanel, _T("Visual Settings"));
			auto styleSz = new wxBoxSizer(wxHORIZONTAL);
			auto styleLabel = new wxStaticText(generalPanel, -1, _("GUI Style (requires restart): "));
			styleSz->Add(styleLabel, itemsFlags);
			
			wxArrayString choices;
			choices.Add(guiModeDefault);
			choices.Add(guiModeSimple);
			guiStyleDropDown = new wxComboBox(generalPanel, -1, wxEmptyString,
				wxDefaultPosition, wxDefaultSize, choices, wxCB_DROPDOWN | wxCB_READONLY);
			
			styleSz->Add(guiStyleDropDown, expandingItemsFlags);
			visualBox->Add(styleSz, staticBoxInnerFlags);
			generalBox->Add(visualBox, staticBoxOuterFlags);
		}
		// Console settings group box
		{
			auto consoleBox = new wxStaticBoxSizer(wxVERTICAL, generalPanel, _T("Console Settings"));
			showConsoleCheck = new wxCheckBox(generalPanel, -1, _T("Show console while an instance is running."));
			consoleBox->Add(showConsoleCheck, itemFlags);
			autoCloseConsoleCheck = new wxCheckBox(generalPanel, -1, _T("Automatically close console when the game quits."));
			consoleBox->Add(autoCloseConsoleCheck, itemFlags);
			generalBox->Add(consoleBox, staticBoxOuterFlags);
		}
		// Update settings group box
		{
			auto updateBox = new wxStaticBoxSizer(wxVERTICAL, generalPanel, _T("Update Settings"));
			autoUpdateCheck = new wxCheckBox(generalPanel, -1, _T("Check for updates when MultiMC starts?"));
			updateBox->Add(autoUpdateCheck, itemFlags);
			forceUpdateToggle = new wxToggleButton(generalPanel, -1, _T("Force-update MultiMC"));
			updateBox->Add(forceUpdateToggle, itemFlags);
			generalBox->Add(updateBox, staticBoxOuterFlags);
		}
		// Instance directory group box
		{
			auto instDirBox = new wxStaticBoxSizer(wxHORIZONTAL, generalPanel, _T("Instance Folder"));
			instDirTextBox = new wxTextCtrl(generalPanel, -1);
			instDirBox->Add(instDirTextBox, expandingItemFlags);
			wxButton *instDirBrowseButton = new wxButton(generalPanel, ID_BrowseInstDir, _T("Browse..."));
			instDirBox->Add(instDirBrowseButton, itemFlags);
			generalBox->Add(instDirBox, staticBoxOuterFlags);
		}
		// Instance directory group box
		{
			auto modDirBox = new wxStaticBoxSizer(wxHORIZONTAL, generalPanel, _T("Central Mods Folder"));
			modsDirTextBox = new wxTextCtrl(generalPanel, -1);
			modDirBox->Add(modsDirTextBox, expandingItemFlags);
			auto instDirBrowseButton = new wxButton(generalPanel, ID_BrowseModDir, _T("Browse..."));
			modDirBox->Add(instDirBrowseButton, itemFlags);
			generalBox->Add(modDirBox, staticBoxOuterFlags);
		}
		generalBox->SetSizeHints(generalPanel);
	}

	// Minecraft tab
	{
		auto mcPanel = new wxPanel(tabCtrl, -1);
		auto mcBox = new wxBoxSizer(wxVERTICAL);
		mcPanel->SetSizer(mcBox);
		
		tabCtrl->AddPage(mcPanel, _T("Minecraft"), false);

		// Compatibility mode check box.
		{
			compatCheckbox = new wxCheckBox(mcPanel, ID_CompatModeCheckbox, _("Compatibility mode?"));
			compatCheckbox->SetHelpText(compadModeHelp);
			mcBox->Add(compatCheckbox, staticBoxOuterFlags);
		}

		// Window size group box
		{
			auto windowSizeBox = new wxStaticBoxSizer(wxVERTICAL, mcPanel, _("Minecraft Window Size"));
			auto winSizeSz = new wxGridBagSizer();
			winSizeSz->AddGrowableCol(1);

			// Maximize
			winMaxCheckbox = new wxCheckBox(mcPanel, ID_MCMaximizeCheckbox, _("Start Minecraft maximized?"));
			winSizeSz->Add(winMaxCheckbox, wxGBPosition(0, 0), wxGBSpan(1, 2), GBitemFlags);

			// Window width
			wxStaticText *winWidthLabel = new wxStaticText(mcPanel, -1, _("Window width: "));
			winSizeSz->Add(winWidthLabel, wxGBPosition(1, 0), wxGBSpan(1, 1), GBitemsFlags);
			winWidthSpin = new wxSpinCtrl(mcPanel, -1);
			winWidthSpin->SetRange(854, wxSystemSettings::GetMetric (wxSYS_SCREEN_X));
			winSizeSz->Add(winWidthSpin, wxGBPosition(1, 1), wxGBSpan(1, 1), GBexpandingItemsFlags);

			// Window height
			wxStaticText *winHeightLabel = new wxStaticText(mcPanel, -1, _("Window height: "));
			winSizeSz->Add(winHeightLabel, wxGBPosition(2, 0), wxGBSpan(1, 1), GBitemsFlags);
			winHeightSpin = new wxSpinCtrl(mcPanel, -1);
			winHeightSpin->SetRange(480, wxSystemSettings::GetMetric (wxSYS_SCREEN_Y));
			winSizeSz->Add(winHeightSpin, wxGBPosition(2, 1), wxGBSpan(1, 1), GBexpandingItemsFlags);
			
			windowSizeBox->Add(winSizeSz, staticBoxInnerFlags);
			mcBox->Add(windowSizeBox, staticBoxOuterFlags);
		}

		// Console colors box
		{
			auto consoleColorsBox = new wxStaticBoxSizer(wxVERTICAL, mcPanel, _("Instance Console Colors"));
			auto consoleColorSz = new wxGridBagSizer();
			consoleColorSz->AddGrowableCol(0);
			consoleColorsBox->Add(consoleColorSz, staticBoxInnerFlags);

			// System message color
			wxStaticText *sysMsgColorLabel = new wxStaticText(mcPanel, -1, _("System message color: "));
			consoleColorSz->Add(sysMsgColorLabel, wxGBPosition(0, 0), wxGBSpan(1, 1), GBitemsFlags);
			sysMsgColorCtrl = new wxColourPickerCtrl(mcPanel, -1);
			consoleColorSz->Add(sysMsgColorCtrl, wxGBPosition(0, 1), wxGBSpan(1, 1), GBexpandingItemsFlags);

			// Stdout message color
			wxStaticText *stdoutColorLabel = new wxStaticText(mcPanel, -1, _("Output message color: "));
			consoleColorSz->Add(stdoutColorLabel, wxGBPosition(1, 0), wxGBSpan(1, 1), GBitemsFlags);
			stdoutColorCtrl = new wxColourPickerCtrl(mcPanel, -1);
			consoleColorSz->Add(stdoutColorCtrl, wxGBPosition(1, 1), wxGBSpan(1, 1), GBexpandingItemsFlags);

			// Stderr message color
			wxStaticText *stderrColorLabel = new wxStaticText(mcPanel, -1, _("Error message color: "));
			consoleColorSz->Add(stderrColorLabel, wxGBPosition(2, 0), wxGBSpan(1, 1), GBitemsFlags);
			stderrColorCtrl = new wxColourPickerCtrl(mcPanel, -1);
			consoleColorSz->Add(stderrColorCtrl, wxGBPosition(2, 1), wxGBSpan(1, 1), GBexpandingItemsFlags);
			
			mcBox->Add(consoleColorsBox, staticBoxOuterFlags);
		}
	}
	
	// Advanced tab
	{
		auto advancedPanel = new wxPanel(tabCtrl, -1);
		auto advancedBox = new wxBoxSizer(wxVERTICAL);
		advancedPanel->SetSizer(advancedBox);
		tabCtrl->AddPage(advancedPanel, _T("Advanced"), false);
		
		// Memory group box
		{
			auto memoryBox = new wxStaticBoxSizer(wxVERTICAL, advancedPanel, _T("Memory"));
			auto memorySz = new wxGridSizer(2,2,0,0);
			// Min memory
			wxStaticText *minMemLabel = new wxStaticText(advancedPanel, -1, _T("Minimum memory allocation: "));
			memorySz->Add(minMemLabel, itemsFlags);
			minMemorySpin = new wxSpinCtrl(advancedPanel, -1);
			minMemorySpin->SetRange(256, Utils::GetMaxAllowedMemAlloc());
			memorySz->Add(minMemorySpin, expandingItemsFlags);
			
			// Max memory
			auto maxMemLabel = new wxStaticText(advancedPanel, -1, _T("Maximum memory allocation: "));
			memorySz->Add(maxMemLabel,itemsFlags);
			maxMemorySpin = new wxSpinCtrl(advancedPanel, -1);
			maxMemorySpin->SetRange(512, Utils::GetMaxAllowedMemAlloc());
			memorySz->Add(maxMemorySpin, expandingItemsFlags);
			
			memoryBox->Add(memorySz, staticBoxInnerFlags);
			advancedBox->Add(memoryBox, staticBoxOuterFlags);
		}
		
		// Java path
		{
			auto javaPathBox = new wxStaticBoxSizer(wxHORIZONTAL, advancedPanel, _T("Java path"));
			javaPathTextBox = new wxTextCtrl(advancedPanel, -1);
			javaPathBox->Add(javaPathTextBox, 1, wxALL | wxEXPAND, 4);
			auto autoDetectButton = new wxButton(advancedPanel, ID_DetectJavaPath, _("Auto-detect"));
			javaPathBox->Add(autoDetectButton, 0, wxALL | wxEXPAND, 4);
			
			advancedBox->Add(javaPathBox, staticBoxOuterFlags);
		}
		
		// JVM Arguments
		{
			auto jvmArgsBox = new wxStaticBoxSizer(wxHORIZONTAL, advancedPanel, _T("Additional JVM Arguments"));
			jvmArgsTextBox = new wxTextCtrl(advancedPanel, -1);
			jvmArgsBox->Add(jvmArgsTextBox, 1, wxALL | wxEXPAND, 4);
			
			advancedBox->Add(jvmArgsBox, staticBoxOuterFlags);
		}
	}
	
	// Buttons
	wxSizer *btnBox = CreateButtonSizer(wxOK | wxCANCEL);
	mainBox->Add(btnBox, 0, wxALIGN_RIGHT | wxBOTTOM | wxRIGHT, 8);

	mainBox->Fit(this);
	LoadSettings();
}

void SettingsDialog::OnOKClicked(wxCommandEvent& event)
{
	if(ApplySettings())
		EndModal(wxID_OK);
}

bool SettingsDialog::FolderMove ( wxFileName oldDir, wxFileName newDir, wxString message, wxString title )
{
	int response = wxMessageBox(
	message,
	title,
	wxYES | wxNO | wxCANCEL | wxCENTER, this);

	if(response == wxCANCEL)
	{
		return false;
	}
	
	if (response == wxYES)
	{
		wxDir srcDir(oldDir.GetFullPath());

		if(!newDir.Mkdir(0777,wxPATH_MKDIR_FULL))
		{
			wxLogError(_("Failed to create the new folder: %s"), newDir.GetFullPath().c_str());
			return false;
		}
		wxString oldName;
		if (srcDir.GetFirst(&oldName))
		{
			do 
			{
				oldName = Path::Combine(oldDir, oldName);
				wxFileName newDirName(oldName);
				newDirName.MakeRelativeTo(oldDir.GetFullPath());
				newDirName.Normalize(wxPATH_NORM_ALL, newDir.GetFullPath());
				if (!wxRenameFile(oldName, newDirName.GetFullPath(), false))
				{
					wxLogError(_("Failed to move: %s"), oldName.c_str());
				}
			} while (srcDir.GetNext(&oldName));
		}
	}
	return true;
}

bool SettingsDialog::ApplySettings()
{
	wxFileName newInstDir = wxFileName::DirName(instDirTextBox->GetValue());
	wxFileName oldInstDir = currentSettings->GetInstDir();
	wxFileName test = newInstDir;
	test.MakeAbsolute();
	wxString tests = test.GetFullPath();
	if(tests.Contains(_("!")))
	{
		wxLogError(_("The chosen instance path contains a ! character.\nThis would make Minecraft crash. Please change it."), tests.c_str());
		return false;
	}
	if (!oldInstDir.SameAs(newInstDir))
	{
		if(!FolderMove(oldInstDir, newInstDir,
			_T("You've changed your instance directory, would you like to transfer all of your instances?"),
			_T("Instance directory changed.")))
		{
			return false;
		}
	}
	currentSettings->SetInstDir(newInstDir);
	wxFileName newModDir = wxFileName::DirName(modsDirTextBox->GetValue());
	wxFileName oldModDir = currentSettings->GetModsDir();
	if (!oldModDir.SameAs(newModDir))
	{
		if(!FolderMove(oldModDir, newModDir,
			_T("You've changed your central mods directory, would you like to transfer all of your mods?"),
			_T("Central mods directory changed.")))
		{
			return false;
		}
	}
	currentSettings->SetModsDir(newModDir);
	
	currentSettings->SetShowConsole(showConsoleCheck->IsChecked());
	currentSettings->SetAutoCloseConsole(autoCloseConsoleCheck->IsChecked());
	currentSettings->SetAutoUpdate(autoUpdateCheck->IsChecked());

	currentSettings->SetMinMemAlloc(minMemorySpin->GetValue());
	currentSettings->SetMaxMemAlloc(maxMemorySpin->GetValue());

	currentSettings->SetJavaPath(javaPathTextBox->GetValue());
	currentSettings->SetJvmArgs(jvmArgsTextBox->GetValue());
	
	GUIMode newGUIMode;
	if (guiStyleDropDown->GetValue() == guiModeDefault)
		newGUIMode = GUI_Default;
	else if (guiStyleDropDown->GetValue() == guiModeSimple)
		newGUIMode = GUI_Simple;
	
	if (newGUIMode != currentSettings->GetGUIMode())
	{
		currentSettings->SetGUIMode(newGUIMode);
		wxMessageBox(_("Changing the GUI style requires a restart in order to take effect. Please restart MultiMC."),
			_("Restart Required"));
	}

	currentSettings->SetUseAppletWrapper(!compatCheckbox->IsChecked());

	currentSettings->SetMCWindowMaximize(winMaxCheckbox->IsChecked());
	currentSettings->SetMCWindowWidth(winWidthSpin->GetValue());
	currentSettings->SetMCWindowHeight(winHeightSpin->GetValue());

	currentSettings->SetConsoleSysMsgColor(sysMsgColorCtrl->GetColour());
	currentSettings->SetConsoleStdoutColor(stdoutColorCtrl->GetColour());
	currentSettings->SetConsoleStderrColor(stderrColorCtrl->GetColour());
	return true;
}

void SettingsDialog::LoadSettings()
{
	showConsoleCheck->SetValue(currentSettings->GetShowConsole());
	autoCloseConsoleCheck->SetValue(currentSettings->GetAutoCloseConsole());

	autoUpdateCheck->SetValue(currentSettings->GetAutoUpdate());

	instDirTextBox->SetValue(currentSettings->GetInstDir().GetFullPath());
	modsDirTextBox->SetValue(currentSettings->GetModsDir().GetFullPath());

	minMemorySpin->SetValue(currentSettings->GetMinMemAlloc());
	maxMemorySpin->SetValue(currentSettings->GetMaxMemAlloc());

	javaPathTextBox->SetValue(currentSettings->GetJavaPath());
	jvmArgsTextBox->SetValue(currentSettings->GetJvmArgs());

	compatCheckbox->SetValue(!currentSettings->GetUseAppletWrapper());

	winMaxCheckbox->SetValue(currentSettings->GetMCWindowMaximize());
	winWidthSpin->SetValue(currentSettings->GetMCWindowWidth());
	winHeightSpin->SetValue(currentSettings->GetMCWindowHeight());
	UpdateCheckboxStuff();

	sysMsgColorCtrl->SetColour(currentSettings->GetConsoleSysMsgColor());
	stdoutColorCtrl->SetColour(currentSettings->GetConsoleStdoutColor());
	stderrColorCtrl->SetColour(currentSettings->GetConsoleStderrColor());
	
	switch (currentSettings->GetGUIMode())
	{
	case GUI_Simple:
		guiStyleDropDown->SetValue(guiModeSimple);
		break;
		
	case GUI_Default:
		guiStyleDropDown->SetValue(guiModeDefault);
		break;
	}
}

void SettingsDialog::OnBrowseInstDirClicked(wxCommandEvent& event)
{
	wxDirDialog *dirDlg = new wxDirDialog(this, _("Select a new instance folder."), instDirTextBox->GetValue());
	if (dirDlg->ShowModal() == wxID_OK)
	{
		wxFileName a = dirDlg->GetPath();
		if(fsutils::isSubsetOf(a,wxGetCwd()))
			a.MakeRelativeTo();
		if(a.SameAs(wxGetCwd()))
		{
			instDirTextBox->ChangeValue(_("."));
		}
		else
		{
			instDirTextBox->ChangeValue(a.GetFullPath());
		}
	}
}

void SettingsDialog::OnBrowseModsDirClicked(wxCommandEvent& event)
{
	wxDirDialog *dirDlg = new wxDirDialog(this, _("Select a new central mods folder."), modsDirTextBox->GetValue());
	if (dirDlg->ShowModal() == wxID_OK)
	{
		wxFileName a = dirDlg->GetPath();
		if(fsutils::isSubsetOf(a,wxGetCwd()))
			a.MakeRelativeTo();
		if(a.SameAs(wxGetCwd()))
		{
			modsDirTextBox->ChangeValue(_("."));
		}
		else
		{
			modsDirTextBox->ChangeValue(a.GetFullPath());
		}
	}
}

void SettingsDialog::OnDetectJavaPathClicked(wxCommandEvent& event)
{
	wxString newJPath = FindJavaPath(javaPathTextBox->GetValue());
	javaPathTextBox->SetValue(newJPath);
}


void SettingsDialog::OnUpdateMCTabCheckboxes(wxCommandEvent& event)
{
	UpdateCheckboxStuff();
}

void SettingsDialog::UpdateCheckboxStuff()
{
	winMaxCheckbox->Enable(!compatCheckbox->IsChecked());

	winWidthSpin->Enable(!(winMaxCheckbox->IsChecked() || compatCheckbox->IsChecked()));
	winHeightSpin->Enable(!(winMaxCheckbox->IsChecked() || compatCheckbox->IsChecked()));
}

bool SettingsDialog::GetForceUpdateMultiMC() const
{
	return forceUpdateToggle->GetValue();
}


BEGIN_EVENT_TABLE(SettingsDialog, wxDialog)
	EVT_BUTTON(ID_BrowseInstDir, SettingsDialog::OnBrowseInstDirClicked)
	EVT_BUTTON(ID_BrowseModDir, SettingsDialog::OnBrowseModsDirClicked)
	EVT_BUTTON(ID_DetectJavaPath, SettingsDialog::OnDetectJavaPathClicked)
	EVT_BUTTON(wxID_OK, SettingsDialog::OnOKClicked)

	EVT_CHECKBOX(ID_MCMaximizeCheckbox, SettingsDialog::OnUpdateMCTabCheckboxes)
	EVT_CHECKBOX(ID_CompatModeCheckbox, SettingsDialog::OnUpdateMCTabCheckboxes)
END_EVENT_TABLE()
