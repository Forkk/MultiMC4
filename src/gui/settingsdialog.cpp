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

	if (devBuildCheck->GetValue() && !settings.GetUseDevBuilds())
	{
		// Display a warning.
		if (wxMessageBox(_("Warning: Dev builds contain incomplete, experimental, and possibly unstable features. \
Some may be extremely buggy, and others may not work at all. Use these at your own risk. \
Are you sure you want to use dev builds?"), 
			_("Are you sure?"), wxOK | wxCANCEL) == wxOK)
		{
			settings.SetUseDevBuilds(devBuildCheck->GetValue());
		}
	}
	else
	{
		settings.SetUseDevBuilds(devBuildCheck->GetValue());
	}
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


BEGIN_EVENT_TABLE(SettingsDialog, wxDialog)
	EVT_BUTTON(ID_BrowseInstDir, SettingsDialog::OnBrowseInstDirClicked)
	EVT_BUTTON(ID_DetectJavaPath, SettingsDialog::OnDetectJavaPathClicked)
END_EVENT_TABLE()
