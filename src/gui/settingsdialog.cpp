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

const wxString guiModeDefault = _("Default");
const wxString guiModeSimple = _("Simple");

SettingsDialog::SettingsDialog(wxWindow *parent, wxWindowID id)
	: wxDialog(parent, id, _T("Settings"), wxDefaultPosition,
		wxSize(500, 450))
{
	wxBoxSizer *mainBox = new wxBoxSizer(wxVERTICAL);
	SetSizer(mainBox);
	
	// Tab control
	tabCtrl = new wxNotebook(this, -1, wxDefaultPosition, 
							 wxDefaultSize);
	mainBox->Add(tabCtrl, 1, wxEXPAND | wxALL, 8);
	
	int row = 0;
	
	// General tab
	wxPanel *generalPanel = new wxPanel(tabCtrl, -1);
	wxGridBagSizer *generalBox = new wxGridBagSizer();
	generalBox->AddGrowableCol(0, 0);
	generalPanel->SetSizer(generalBox);
	tabCtrl->AddPage(generalPanel, _T("General"), true);
	
	// Visual settings group box
	wxStaticBoxSizer *visualBox = new wxStaticBoxSizer(wxVERTICAL, 
		generalPanel, _T("Visual Settings"));
	wxBoxSizer *styleSz = new wxBoxSizer(wxHORIZONTAL);
	wxStaticText *styleLabel = new wxStaticText(generalPanel, -1, _("GUI Style (requires restart): "));
	styleSz->Add(styleLabel, wxSizerFlags(0).Align(wxALIGN_CENTER_VERTICAL).Border(wxALL, 4));
	wxArrayString choices; choices.Add(guiModeDefault); choices.Add(guiModeSimple);
	guiStyleDropDown = new wxComboBox(generalPanel, -1, wxEmptyString,
		wxDefaultPosition, wxDefaultSize, choices, wxCB_DROPDOWN | wxCB_READONLY);
	styleSz->Add(guiStyleDropDown, wxSizerFlags(1).Align(wxALIGN_CENTER_VERTICAL).Border(wxALL, 4));
	visualBox->Add(styleSz, wxSizerFlags(0).Expand());
	generalBox->Add(visualBox, wxGBPosition(row++, 0), wxGBSpan(1, 1), wxALL | wxEXPAND, 4);
	
	// Console settings group box
	wxStaticBoxSizer *consoleBox = new wxStaticBoxSizer(wxVERTICAL, 
		generalPanel, _T("Console Settings"));
	showConsoleCheck = new wxCheckBox(generalPanel, -1, 
		_T("Show console while an instance is running."));
	consoleBox->Add(showConsoleCheck, 0, wxALL, 4);
	autoCloseConsoleCheck = new wxCheckBox(generalPanel, -1,
		_T("Automatically close console when the game quits."));
	consoleBox->Add(autoCloseConsoleCheck, 0, wxALL, 4);
	generalBox->Add(consoleBox, wxGBPosition(row++, 0), wxGBSpan(1, 1), wxALL | wxEXPAND, 4);
	
	// Update settings group box
	wxStaticBoxSizer *updateBox = new wxStaticBoxSizer(wxVERTICAL,
		generalPanel, _T("Update Settings"));
	autoUpdateCheck = new wxCheckBox(generalPanel, -1,
		_T("Check for updates when MultiMC starts?"));
	updateBox->Add(autoUpdateCheck, 0, wxALL, 4);
	devBuildCheck = new wxCheckBox(generalPanel, -1, _("Use development builds?"));
	updateBox->Add(devBuildCheck, 0, wxALL | wxEXPAND, 4);
	forceUpdateToggle = new wxToggleButton(generalPanel, -1,
		_T("Force-update MultiMC"));
	updateBox->Add(forceUpdateToggle, 0, wxALL, 4);
	generalBox->Add(updateBox, wxGBPosition(row++, 0), wxGBSpan(1, 1), wxALL | wxEXPAND, 4);
	
	// Instance directory group box
	wxStaticBoxSizer *instDirBox = new wxStaticBoxSizer(wxHORIZONTAL,
		generalPanel, _T("Instance Folder"));
	instDirTextBox = new wxTextCtrl(generalPanel, -1);
	instDirBox->Add(instDirTextBox, 1, wxEXPAND | wxALL, 4);
	wxButton *instDirBrowseButton = new wxButton(generalPanel, 
		ID_BrowseInstDir, _T("Browse..."));
	instDirBox->Add(instDirBrowseButton, 0, wxALL, 4);
	generalBox->Add(instDirBox, wxGBPosition(row++, 0), wxGBSpan(1, 1), wxALL | wxEXPAND, 4);
	
	generalBox->SetSizeHints(generalPanel);


	// Minecraft tab
	wxPanel *mcPanel = new wxPanel(tabCtrl, -1);
	wxGridBagSizer *mcBox = new wxGridBagSizer();
	mcBox->AddGrowableCol(0, 0);
	mcPanel->SetSizer(mcBox);
	tabCtrl->AddPage(mcPanel, _T("Minecraft"), false);

	// Compatibility mode check box.
	compatCheckbox = new wxCheckBox(mcPanel, ID_CompatModeCheckbox, 
		_("Compatibility mode?"));
	compatCheckbox->SetHelpText(_("Compatibility mode launches Minecraft \
using the old Minecraft.main() method, rather than using MultiMC's \
AppletWrapper. This should help fix a few issues with certain mods, \
but it will disable certain features such as setting Minecraft's window \
size and icon."));
	mcBox->Add(compatCheckbox, wxGBPosition(0, 0), wxGBSpan(1, 1), wxALL | wxEXPAND, 4);

	// Window size group box
	wxStaticBoxSizer *windowSizeBox = new wxStaticBoxSizer(wxVERTICAL,
		mcPanel, _("Minecraft Window Size"));
	wxGridBagSizer *winSizeSz = new wxGridBagSizer();
	windowSizeBox->Add(winSizeSz, wxSizerFlags(0).Expand());
	mcBox->Add(windowSizeBox, wxGBPosition(1, 0), wxGBSpan(1, 1), wxALL | wxEXPAND, 4);

	// Maximize
	winMaxCheckbox = new wxCheckBox(mcPanel, ID_MCMaximizeCheckbox, 
		_("Start Minecraft maximized?"));
	winSizeSz->Add(winMaxCheckbox, wxGBPosition(0, 0), wxGBSpan(1, 2), 
		wxALL | wxALIGN_CENTER_VERTICAL, 4);

	// Window width
	wxStaticText *winWidthLabel = new wxStaticText(mcPanel, -1, 
		_("Window width: "));
	winSizeSz->Add(winWidthLabel, wxGBPosition(1, 0), wxGBSpan(1, 1), 
		wxALL | wxALIGN_CENTER_VERTICAL, 4);
	winWidthSpin = new wxSpinCtrl(mcPanel, -1);
	winWidthSpin->SetRange(854, wxSystemSettings::GetMetric (wxSYS_SCREEN_X));
	winSizeSz->Add(winWidthSpin, wxGBPosition(1, 1), wxGBSpan(1, 1), 
		wxALL | wxALIGN_CENTER_VERTICAL | wxEXPAND, 4);

	// Window height
	wxStaticText *winHeightLabel = new wxStaticText(mcPanel, -1, 
		_("Window height: "));
	winSizeSz->Add(winHeightLabel, wxGBPosition(2, 0), wxGBSpan(1, 1), 
		wxALL | wxALIGN_CENTER_VERTICAL, 4);
	winHeightSpin = new wxSpinCtrl(mcPanel, -1);
	winHeightSpin->SetRange(480, wxSystemSettings::GetMetric (wxSYS_SCREEN_Y));
	winSizeSz->Add(winHeightSpin, wxGBPosition(2, 1), wxGBSpan(1, 1), 
		wxALL | wxALIGN_CENTER_VERTICAL | wxEXPAND, 4);


	// Console colors box
	wxStaticBoxSizer *consoleColorsBox = new wxStaticBoxSizer(wxVERTICAL,
		mcPanel, _("Instance Console Colors"));
	wxGridBagSizer *consoleColorSz = new wxGridBagSizer();
	consoleColorsBox->Add(consoleColorSz, wxSizerFlags(0).Expand());
	mcBox->Add(consoleColorsBox, wxGBPosition(2, 0), wxGBSpan(1, 1), wxALL | wxEXPAND, 4);

	// System message color
	wxStaticText *sysMsgColorLabel = new wxStaticText(mcPanel, -1, _("System message color: "));
	consoleColorSz->Add(sysMsgColorLabel, wxGBPosition(0, 0), wxGBSpan(1, 1), 
		wxALL | wxALIGN_CENTER_VERTICAL, 4);
	sysMsgColorCtrl = new wxColourPickerCtrl(mcPanel, -1, *wxBLACK, 
		wxDefaultPosition, wxDefaultSize, wxCLRP_SHOW_LABEL);
	consoleColorSz->Add(sysMsgColorCtrl, wxGBPosition(0, 1), wxGBSpan(1, 1), wxALL | wxEXPAND, 4);

	// Stdout message color
	wxStaticText *stdoutColorLabel = new wxStaticText(mcPanel, -1, _("Output message color: "));
	consoleColorSz->Add(stdoutColorLabel, wxGBPosition(1, 0), wxGBSpan(1, 1), 
		wxALL | wxALIGN_CENTER_VERTICAL, 4);
	stdoutColorCtrl = new wxColourPickerCtrl(mcPanel, -1, *wxBLACK, 
		wxDefaultPosition, wxDefaultSize, wxCLRP_SHOW_LABEL);
	consoleColorSz->Add(stdoutColorCtrl, wxGBPosition(1, 1), wxGBSpan(1, 1), wxALL | wxEXPAND, 4);

	// Stderr message color
	wxStaticText *stderrColorLabel = new wxStaticText(mcPanel, -1, _("Error message color: "));
	consoleColorSz->Add(stderrColorLabel, wxGBPosition(2, 0), wxGBSpan(1, 1), 
		wxALL | wxALIGN_CENTER_VERTICAL, 4);
	stderrColorCtrl = new wxColourPickerCtrl(mcPanel, -1, *wxBLACK, 
		wxDefaultPosition, wxDefaultSize, wxCLRP_SHOW_LABEL);
	consoleColorSz->Add(stderrColorCtrl, wxGBPosition(2, 1), wxGBSpan(1, 1), wxALL | wxEXPAND, 4);

	
	// Advanced tab
	wxPanel *advancedPanel = new wxPanel(tabCtrl, -1);
	wxGridBagSizer *advancedBox = new wxGridBagSizer();
	advancedBox->AddGrowableCol(0, 0);
	advancedPanel->SetSizer(advancedBox);
	tabCtrl->AddPage(advancedPanel, _T("Advanced"), false);
	
	// Memory group box
	wxStaticBoxSizer *memoryBox = new wxStaticBoxSizer(wxVERTICAL,
		advancedPanel, _T("Memory"));
	advancedBox->Add(memoryBox, wxGBPosition(0, 0), wxGBSpan(1, 1), wxALL | wxEXPAND, 4);
	
	// Min memory
	wxBoxSizer *minMemBox = new wxBoxSizer(wxHORIZONTAL);
	memoryBox->Add(minMemBox, 0, wxALL, 4);
	wxStaticText *minMemLabel = new wxStaticText(advancedPanel, -1, 
		_T("Minimum memory allocation: "));
	minMemBox->Add(minMemLabel, 1, wxEXPAND);
	minMemorySpin = new wxSpinCtrl(advancedPanel, -1);
	minMemorySpin->SetRange(256, Utils::GetMaxAllowedMemAlloc());
	minMemBox->Add(minMemorySpin);
	
	// Max memory
	wxBoxSizer *maxMemBox = new wxBoxSizer(wxHORIZONTAL);
	memoryBox->Add(maxMemBox, 0, wxALL, 4);
	wxStaticText *maxMemLabel = new wxStaticText(advancedPanel, -1, 
		_T("Maximum memory allocation: "));
	maxMemBox->Add(maxMemLabel, 1, wxEXPAND);
	maxMemorySpin = new wxSpinCtrl(advancedPanel, -1);
	maxMemorySpin->SetRange(512, Utils::GetMaxAllowedMemAlloc());
	maxMemBox->Add(maxMemorySpin);
	
	// Java path
	wxStaticBoxSizer *javaPathBox = new wxStaticBoxSizer(wxHORIZONTAL, 
		advancedPanel, _T("Java path"));
	javaPathTextBox = new wxTextCtrl(advancedPanel, -1);
	javaPathBox->Add(javaPathTextBox, 1, wxALL | wxEXPAND, 4);
	wxButton *autoDetectButton = new wxButton(advancedPanel, ID_DetectJavaPath, _("Auto-detect"));
	javaPathBox->Add(autoDetectButton, 0, wxALL | wxEXPAND, 4);
	advancedBox->Add(javaPathBox, wxGBPosition(1, 0), wxGBSpan(1, 1), wxALL | wxEXPAND, 4);
	
	// JVM Arguments
	wxStaticBoxSizer *jvmArgsBox = new wxStaticBoxSizer(wxHORIZONTAL,
		advancedPanel, _T("Additional JVM Arguments"));
	jvmArgsTextBox = new wxTextCtrl(advancedPanel, -1);
	jvmArgsBox->Add(jvmArgsTextBox, 1, wxALL | wxEXPAND, 4);
	advancedBox->Add(jvmArgsBox, wxGBPosition(2, 0), wxGBSpan(1, 1), wxALL | wxEXPAND, 4);

	
	// Buttons
	wxSizer *btnBox = CreateButtonSizer(wxOK | wxCANCEL);
	mainBox->Add(btnBox, 0, wxALIGN_RIGHT | wxBOTTOM | wxRIGHT, 8);

	LoadSettings();
}

void SettingsDialog::OnButtonClicked(wxCommandEvent& event)
{
	this->Close();
}

void SettingsDialog::ApplySettings(AppSettings &s /* = settings */)
{
	s.SetShowConsole(showConsoleCheck->IsChecked());
	s.SetAutoCloseConsole(autoCloseConsoleCheck->IsChecked());
	
	s.SetAutoUpdate(autoUpdateCheck->IsChecked());
	
	wxFileName newInstDir = wxFileName::DirName(instDirTextBox->GetValue());
	if (!s.GetInstDir().SameAs(newInstDir))
	{
		wxFileName oldInstDir = s.GetInstDir();
		
		int response = wxMessageBox(
			_T("You've changed your instance directory, would you like to transfer all of your instances?"),
			_T("Instance directory changed."), 
			wxYES | wxNO | wxCANCEL | wxCENTER, this);
		
	RetryTransfer:
		if (response != wxCANCEL)
		{
			s.SetInstDir(newInstDir);
		}
		
		if (response == wxYES)
		{
			wxDir instDir(oldInstDir.GetFullPath());

			wxString oldDirName;
			if (instDir.GetFirst(&oldDirName))
			{
				do 
				{
					oldDirName = Path::Combine(oldInstDir, oldDirName);
					wxFileName newDirName(oldDirName);
					newDirName.MakeRelativeTo(oldInstDir.GetFullPath());
					newDirName.Normalize(wxPATH_NORM_ALL, newInstDir.GetFullPath());
					if (!wxRenameFile(oldDirName, newDirName.GetFullPath(), false))
					{
						wxLogError(_("Failed to move instance folder %s."), oldDirName.c_str());
					}
				} while (instDir.GetNext(&oldDirName));
			}
		}
	}

	s.SetMinMemAlloc(minMemorySpin->GetValue());
	s.SetMaxMemAlloc(maxMemorySpin->GetValue());

	s.SetJavaPath(javaPathTextBox->GetValue());
	s.SetJvmArgs(jvmArgsTextBox->GetValue());
	
	GUIMode newGUIMode;
	if (guiStyleDropDown->GetValue() == guiModeDefault)
		newGUIMode = GUI_Default;
	else if (guiStyleDropDown->GetValue() == guiModeSimple)
		newGUIMode = GUI_Simple;
	
	if (newGUIMode != s.GetGUIMode())
	{
		s.SetGUIMode(newGUIMode);
		wxMessageBox(_("Changing the GUI style requires a restart in order to take effect. Please restart MultiMC."),
			_("Restart Required"));
	}

	if (devBuildCheck->GetValue() && !s.GetUseDevBuilds())
	{
		// Display a warning.
		if (wxMessageBox(_("Warning: Dev builds contain incomplete, experimental, and possibly unstable features. \
Some may be extremely buggy, and others may not work at all. Use these at your own risk. \
Are you sure you want to use dev builds?"), 
			_("Are you sure?"), wxOK | wxCANCEL) == wxOK)
		{
			s.SetUseDevBuilds(devBuildCheck->GetValue());
		}
	}
	else
	{
		s.SetUseDevBuilds(devBuildCheck->GetValue());
	}

	s.SetUseAppletWrapper(!compatCheckbox->IsChecked());

	s.SetMCWindowMaximize(winMaxCheckbox->IsChecked());
	s.SetMCWindowWidth(winWidthSpin->GetValue());
	s.SetMCWindowHeight(winHeightSpin->GetValue());

	s.SetConsoleSysMsgColor(sysMsgColorCtrl->GetColour());
	s.SetConsoleStdoutColor(stdoutColorCtrl->GetColour());
	s.SetConsoleStderrColor(stderrColorCtrl->GetColour());
}

void SettingsDialog::LoadSettings(AppSettings &s /* = settings */)
{
	showConsoleCheck->SetValue(s.GetShowConsole());
	autoCloseConsoleCheck->SetValue(s.GetAutoCloseConsole());

	autoUpdateCheck->SetValue(s.GetAutoUpdate());

	devBuildCheck->SetValue(s.GetUseDevBuilds());

	instDirTextBox->SetValue(s.GetInstDir().GetFullPath());

	minMemorySpin->SetValue(s.GetMinMemAlloc());
	maxMemorySpin->SetValue(s.GetMaxMemAlloc());

	javaPathTextBox->SetValue(s.GetJavaPath());
	jvmArgsTextBox->SetValue(s.GetJvmArgs());

	compatCheckbox->SetValue(!s.GetUseAppletWrapper());

	winMaxCheckbox->SetValue(s.GetMCWindowMaximize());
	winWidthSpin->SetValue(s.GetMCWindowWidth());
	winHeightSpin->SetValue(s.GetMCWindowHeight());
	UpdateCheckboxStuff();

	sysMsgColorCtrl->SetColour(s.GetConsoleSysMsgColor());
	stdoutColorCtrl->SetColour(s.GetConsoleStdoutColor());
	stderrColorCtrl->SetColour(s.GetConsoleStderrColor());
	
	switch (s.GetGUIMode())
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
	wxDirDialog *dirDlg = new wxDirDialog(this, _("Select a new instance folder."), 
		instDirTextBox->GetValue());
	if (dirDlg->ShowModal() == wxID_OK)
		instDirTextBox->ChangeValue(dirDlg->GetPath());
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


BEGIN_EVENT_TABLE(SettingsDialog, wxDialog)
	EVT_BUTTON(ID_BrowseInstDir, SettingsDialog::OnBrowseInstDirClicked)
	EVT_BUTTON(ID_DetectJavaPath, SettingsDialog::OnDetectJavaPathClicked)

	EVT_CHECKBOX(ID_MCMaximizeCheckbox, SettingsDialog::OnUpdateMCTabCheckboxes)
	EVT_CHECKBOX(ID_CompatModeCheckbox, SettingsDialog::OnUpdateMCTabCheckboxes)
END_EVENT_TABLE()
