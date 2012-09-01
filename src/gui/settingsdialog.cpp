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
const wxString compadModeHelp =_(
"Compatibility mode launches Minecraft\
using the old Minecraft.main() method, rather than using MultiMC's\
AppletWrapper. This should help fix a few issues with certain mods,\
but it will disable certain features such as setting Minecraft's window\
size and icon."
);

SettingsDialog::SettingsDialog(wxWindow *parent, wxWindowID id)
	: wxDialog(parent, id, _T("Settings"), wxDefaultPosition,
		wxSize(500, 450))
{
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

void SettingsDialog::OnButtonClicked(wxCommandEvent& event)
{
	this->Close();
}

void SettingsDialog::ApplySettings(AppSettings *s /* = settings */)
{
	s->SetShowConsole(showConsoleCheck->IsChecked());
	s->SetAutoCloseConsole(autoCloseConsoleCheck->IsChecked());
	
	s->SetAutoUpdate(autoUpdateCheck->IsChecked());
	
	wxFileName newInstDir = wxFileName::DirName(instDirTextBox->GetValue());
	if (!s->GetInstDir().SameAs(newInstDir))
	{
		wxFileName oldInstDir = s->GetInstDir();
		
		int response = wxMessageBox(
			_T("You've changed your instance directory, would you like to transfer all of your instances?"),
			_T("Instance directory changed."), 
			wxYES | wxNO | wxCANCEL | wxCENTER, this);
		
		if (response != wxCANCEL)
		{
			s->SetInstDir(newInstDir);
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
						wxLogError(_("Failed to move instance folder %s->"), oldDirName.c_str());
					}
				} while (instDir.GetNext(&oldDirName));
			}
		}
	}

	s->SetMinMemAlloc(minMemorySpin->GetValue());
	s->SetMaxMemAlloc(maxMemorySpin->GetValue());

	s->SetJavaPath(javaPathTextBox->GetValue());
	s->SetJvmArgs(jvmArgsTextBox->GetValue());
	
	GUIMode newGUIMode;
	if (guiStyleDropDown->GetValue() == guiModeDefault)
		newGUIMode = GUI_Default;
	else if (guiStyleDropDown->GetValue() == guiModeSimple)
		newGUIMode = GUI_Simple;
	
	if (newGUIMode != s->GetGUIMode())
	{
		s->SetGUIMode(newGUIMode);
		wxMessageBox(_("Changing the GUI style requires a restart in order to take effect. Please restart MultiMC."),
			_("Restart Required"));
	}

//	if (devBuildCheck->GetValue() && !s->GetUseDevBuilds())
//	{
//		// Display a warning.
//		if (wxMessageBox(_("Warning: Dev builds contain incomplete, experimental, and possibly unstable features-> \
//Some may be extremely buggy, and others may not work at all. Use these at your own risk. \
//Are you sure you want to use dev builds?"), 
//			_("Are you sure?"), wxOK | wxCANCEL) == wxOK)
//		{
//			s->SetUseDevBuilds(devBuildCheck->GetValue());
//		}
//	}
//	else
//	{
//		s->SetUseDevBuilds(devBuildCheck->GetValue());
//	}

	s->SetUseAppletWrapper(!compatCheckbox->IsChecked());

	s->SetMCWindowMaximize(winMaxCheckbox->IsChecked());
	s->SetMCWindowWidth(winWidthSpin->GetValue());
	s->SetMCWindowHeight(winHeightSpin->GetValue());

	s->SetConsoleSysMsgColor(sysMsgColorCtrl->GetColour());
	s->SetConsoleStdoutColor(stdoutColorCtrl->GetColour());
	s->SetConsoleStderrColor(stderrColorCtrl->GetColour());
}

void SettingsDialog::LoadSettings(AppSettings *s /* = settings */)
{
	showConsoleCheck->SetValue(s->GetShowConsole());
	autoCloseConsoleCheck->SetValue(s->GetAutoCloseConsole());

	autoUpdateCheck->SetValue(s->GetAutoUpdate());

	//devBuildCheck->SetValue(s->GetUseDevBuilds());

	instDirTextBox->SetValue(s->GetInstDir().GetFullPath());

	minMemorySpin->SetValue(s->GetMinMemAlloc());
	maxMemorySpin->SetValue(s->GetMaxMemAlloc());

	javaPathTextBox->SetValue(s->GetJavaPath());
	jvmArgsTextBox->SetValue(s->GetJvmArgs());

	compatCheckbox->SetValue(!s->GetUseAppletWrapper());

	winMaxCheckbox->SetValue(s->GetMCWindowMaximize());
	winWidthSpin->SetValue(s->GetMCWindowWidth());
	winHeightSpin->SetValue(s->GetMCWindowHeight());
	UpdateCheckboxStuff();

	sysMsgColorCtrl->SetColour(s->GetConsoleSysMsgColor());
	stdoutColorCtrl->SetColour(s->GetConsoleStdoutColor());
	stderrColorCtrl->SetColour(s->GetConsoleStderrColor());
	
	switch (s->GetGUIMode())
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

bool SettingsDialog::GetForceUpdateMultiMC() const
{
	return forceUpdateToggle->GetValue();
}


BEGIN_EVENT_TABLE(SettingsDialog, wxDialog)
	EVT_BUTTON(ID_BrowseInstDir, SettingsDialog::OnBrowseInstDirClicked)
	EVT_BUTTON(ID_DetectJavaPath, SettingsDialog::OnDetectJavaPathClicked)

	EVT_CHECKBOX(ID_MCMaximizeCheckbox, SettingsDialog::OnUpdateMCTabCheckboxes)
	EVT_CHECKBOX(ID_CompatModeCheckbox, SettingsDialog::OnUpdateMCTabCheckboxes)
END_EVENT_TABLE()
