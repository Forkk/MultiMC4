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

#include "settingsdialog.h"
#include <wx/gbsizer.h>
#include <wx/dir.h>
#include <apputils.h>
#include <fsutils.h>
#include "instance.h"

const wxString guiModeDefault = _("Default");
const wxString guiModeSimple = _("Simple");
const wxString dontUpdate = "Never update Minecraft";
const wxString doUpdate = "Update Minecraft automatically";
const wxString compadModeHelp =_(
"Compatibility mode launches Minecraft\
using the old Minecraft.main() method, rather than using MultiMC's\
AppletWrapper. This should help fix a few issues with certain mods,\
but it will disable certain features such as setting Minecraft's window\
size and icon."
);

SettingsDialog::SettingsDialog( wxWindow* parent, wxWindowID id, SettingsBase* s /* = settings */)
	: wxDialog(parent, id, _T("Settings"), wxDefaultPosition, wxSize(500, 450))
{
	currentSettings = s;
	wxBoxSizer *mainBox = new wxBoxSizer(wxVERTICAL);
	SetSizer(mainBox);
	
	// Tab control
	tabCtrl = new wxNotebook(this, -1);
	mainBox->Add(tabCtrl, 1, wxEXPAND | wxALL, 8);
	
	instanceMode = !currentSettings->IsConfigGlobal();
	if(instanceMode)
	{
		Instance * i = (Instance *)s;
		wxString title = wxT("Settings for ");
		title.Append(i->GetName());
		SetTitle(title);
	}
	
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
	
	
	// MultiMC tab
	if(!instanceMode)
	{
		auto multimcPanel = new wxPanel(tabCtrl, -1);
		auto multimcSizer = new wxBoxSizer(wxVERTICAL);
		multimcPanel->SetSizer(multimcSizer);
		tabCtrl->AddPage(multimcPanel, _T("MultiMC"), true);
		// Visual settings group box
		{
			auto box = new wxStaticBoxSizer(wxVERTICAL, multimcPanel, _T("Visual Settings"));
			auto styleSz = new wxBoxSizer(wxHORIZONTAL);
			auto styleLabel = new wxStaticText(box->GetStaticBox(), -1, _("GUI Style (requires restart): "));
			styleSz->Add(styleLabel, itemsFlags);
			
			wxArrayString choices;
			choices.Add(guiModeDefault);
			choices.Add(guiModeSimple);
			guiStyleDropDown = new wxComboBox(box->GetStaticBox(), -1, wxEmptyString,
				wxDefaultPosition, wxDefaultSize, choices, wxCB_DROPDOWN | wxCB_READONLY);
			
			styleSz->Add(guiStyleDropDown, expandingItemsFlags);
			box->Add(styleSz, staticBoxInnerFlags);
			multimcSizer->Add(box, staticBoxOuterFlags);
		}
		// Update settings group box
		{
			auto box = new wxStaticBoxSizer(wxVERTICAL, multimcPanel, _T("Update Settings"));
			useDevBuildsCheck = new wxCheckBox(box->GetStaticBox(), -1, _T("Use development builds."));
			box->Add(useDevBuildsCheck, itemFlags);
			autoUpdateCheck = new wxCheckBox(box->GetStaticBox(), -1, _T("Check for updates when MultiMC starts?"));
			box->Add(autoUpdateCheck, itemFlags);
			forceUpdateToggle = new wxToggleButton(box->GetStaticBox(), -1, _T("Force-update MultiMC"));
			box->Add(forceUpdateToggle, itemFlags);
			multimcSizer->Add(box, staticBoxOuterFlags);
		}
		// Directory group box
		{
			auto box = new wxStaticBoxSizer(wxVERTICAL, multimcPanel, _T("Folders"));
			auto dirsSizer = new wxGridBagSizer();
			int row = 0;
			{
				auto instDirLabel = new wxStaticText(box->GetStaticBox(), -1, _("Instances: "));
				dirsSizer->Add(instDirLabel, wxGBPosition(row, 0), wxGBSpan(1, 1), GBitemsFlags);

				instDirTextBox = new wxTextCtrl(box->GetStaticBox(), -1);
				dirsSizer->Add(instDirTextBox, wxGBPosition(row, 1), wxGBSpan(1, 1), GBexpandingItemsFlags);

				wxButton *instDirBrowseButton = new wxButton(box->GetStaticBox(), ID_BrowseInstDir, _T("Browse..."));
				dirsSizer->Add(instDirBrowseButton, wxGBPosition(row, 2), wxGBSpan(1, 1), GBitemsFlags);
			}
			row++;
			{
				auto modsDirLabel = new wxStaticText(box->GetStaticBox(), -1, _("Mods: "));
				dirsSizer->Add(modsDirLabel, wxGBPosition(row, 0), wxGBSpan(1, 1), GBitemsFlags);

				modsDirTextBox = new wxTextCtrl(box->GetStaticBox(), -1);
				dirsSizer->Add(modsDirTextBox, wxGBPosition(row, 1), wxGBSpan(1, 1), GBexpandingItemsFlags);

				auto modDirBrowseButton = new wxButton(box->GetStaticBox(), ID_BrowseModDir, _T("Browse..."));
				dirsSizer->Add(modDirBrowseButton, wxGBPosition(row, 2), wxGBSpan(1, 1), GBitemsFlags);
			}

			dirsSizer->AddGrowableCol(1);

			box->Add(dirsSizer, staticBoxInnerFlags);
			multimcSizer->Add(box, staticBoxOuterFlags);
		}
		multimcSizer->SetSizeHints(multimcPanel);
	}

	// Console tab
	if(!instanceMode)
	{
		auto consolePanel = new wxPanel(tabCtrl, -1);
		auto consoleSizer = new wxBoxSizer(wxVERTICAL);
		consolePanel->SetSizer(consoleSizer);
		tabCtrl->AddPage(consolePanel, _T("Error console"), false);
		// Console settings group box
		{
			auto box = new wxStaticBoxSizer(wxVERTICAL, consolePanel, _T("Console Settings"));
			showConsoleCheck = new wxCheckBox(box->GetStaticBox(), -1, _T("Show console while an instance is running."));
			box->Add(showConsoleCheck, itemFlags);
			autoCloseConsoleCheck = new wxCheckBox(box->GetStaticBox(), -1, _T("Automatically close console when the game quits."));
			box->Add(autoCloseConsoleCheck, itemFlags);
			consoleSizer->Add(box, staticBoxOuterFlags);
		}
		// Console colors box
		{
			auto consoleColorsBox = new wxStaticBoxSizer(wxVERTICAL, consolePanel, _("Instance Console Colors"));
			auto consoleColorSz = new wxGridBagSizer();
			consoleColorsBox->Add(consoleColorSz, staticBoxInnerFlags);

			// System message color
			wxStaticText *sysMsgColorLabel = new wxStaticText(consolePanel, -1, _("System message color: "));
			consoleColorSz->Add(sysMsgColorLabel, wxGBPosition(0, 0), wxGBSpan(1, 1), GBitemsFlags);
			sysMsgColorCtrl = new wxColourPickerCtrl(consolePanel, -1);
			consoleColorSz->Add(sysMsgColorCtrl, wxGBPosition(0, 1), wxGBSpan(1, 1), GBexpandingItemsFlags);

			// Stdout message color
			wxStaticText *stdoutColorLabel = new wxStaticText(consolePanel, -1, _("Output message color: "));
			consoleColorSz->Add(stdoutColorLabel, wxGBPosition(1, 0), wxGBSpan(1, 1), GBitemsFlags);
			stdoutColorCtrl = new wxColourPickerCtrl(consolePanel, -1);
			consoleColorSz->Add(stdoutColorCtrl, wxGBPosition(1, 1), wxGBSpan(1, 1), GBexpandingItemsFlags);

			// Stderr message color
			wxStaticText *stderrColorLabel = new wxStaticText(consolePanel, -1, _("Error message color: "));
			consoleColorSz->Add(stderrColorLabel, wxGBPosition(2, 0), wxGBSpan(1, 1), GBitemsFlags);
			stderrColorCtrl = new wxColourPickerCtrl(consolePanel, -1);
			consoleColorSz->Add(stderrColorCtrl, wxGBPosition(2, 1), wxGBSpan(1, 1), GBexpandingItemsFlags);
			
			consoleColorSz->AddGrowableCol(0);

			consoleSizer->Add(consoleColorsBox, staticBoxOuterFlags);
		}
	}

	// Minecraft tab
	{
		auto mcPanel = new wxPanel(tabCtrl, -1);
		auto mcBox = new wxBoxSizer(wxVERTICAL);
		mcPanel->SetSizer(mcBox);
		tabCtrl->AddPage(mcPanel, _T("Minecraft"), false);

		// Updates group box
		{
			auto box = new wxStaticBoxSizer(wxVERTICAL, mcPanel, _T("Updates"));
			
			// Override
			if(instanceMode)
			{
				updateUseDefs = new wxCheckBox(box->GetStaticBox(), ID_OverrideUpdate, _("Use defaults?"));
				box->Add(updateUseDefs, 0, wxALL, 4);
			}
			
			wxArrayString choices;
			{
				choices.Add(dontUpdate);
				choices.Add(doUpdate);
			}
			mcUpdateDropDown = new wxComboBox(box->GetStaticBox(), -1, wxEmptyString,
				wxDefaultPosition, wxDefaultSize, choices, wxCB_DROPDOWN | wxCB_READONLY);
			box->Add(mcUpdateDropDown, expandingItemFlags);
			mcBox->Add(box, staticBoxOuterFlags);
		}
		
		// Window size group box
		{
			auto box = new wxStaticBoxSizer(wxVERTICAL, mcPanel, _("Minecraft Window Size"));
			auto sizer = new wxGridBagSizer();
			
			int row = 0;
			
			if(instanceMode)
			{
				// Override
				winUseDefs = new wxCheckBox(mcPanel, ID_OverrideWindow, _("Use defaults?"));
				sizer->Add(winUseDefs, wxGBPosition(row, 0), wxGBSpan(1, 2), wxALL, 4);
				row++;
			}
			
			compatCheckbox = new wxCheckBox(mcPanel, ID_CompatModeCheckbox, _("Compatibility mode?"));
			compatCheckbox->SetHelpText(compadModeHelp);
			sizer->Add(compatCheckbox,wxGBPosition(row, 0), wxGBSpan(1, 2) ,GBitemFlags);
			row++;
			
			// Maximize
			winMaxCheckbox = new wxCheckBox(mcPanel, ID_MCMaximizeCheckbox, _("Start Minecraft maximized?"));
			sizer->Add(winMaxCheckbox, wxGBPosition(row, 0), wxGBSpan(1, 2), GBitemFlags);
			row++;

			// Window width
			winWidthLabel = new wxStaticText(mcPanel, -1, _("Window width: "));
			sizer->Add(winWidthLabel, wxGBPosition(row, 0), wxGBSpan(1, 1), GBitemsFlags);
			winWidthSpin = new wxSpinCtrl(mcPanel, -1);
			winWidthSpin->SetRange(854, wxSystemSettings::GetMetric (wxSYS_SCREEN_X));
			sizer->Add(winWidthSpin, wxGBPosition(row, 1), wxGBSpan(1, 1), GBexpandingItemsFlags);
			row++;
			
			// Window height
			winHeightLabel = new wxStaticText(mcPanel, -1, _("Window height: "));
			sizer->Add(winHeightLabel, wxGBPosition(row, 0), wxGBSpan(1, 1), GBitemsFlags);
			winHeightSpin = new wxSpinCtrl(mcPanel, -1);
			winHeightSpin->SetRange(480, wxSystemSettings::GetMetric (wxSYS_SCREEN_Y));
			sizer->Add(winHeightSpin, wxGBPosition(row, 1), wxGBSpan(1, 1), GBexpandingItemsFlags);
			
			sizer->AddGrowableCol(1);

			box->Add(sizer, staticBoxInnerFlags);
			mcBox->Add(box, staticBoxOuterFlags);
		}
	}
	{
		auto mcPanel = new wxPanel(tabCtrl, -1);
		auto mcBox = new wxBoxSizer(wxVERTICAL);
		mcPanel->SetSizer(mcBox);
		tabCtrl->AddPage(mcPanel, _T("Java"), false);
		// Memory group box
		{
			auto box = new wxStaticBoxSizer(wxVERTICAL, mcPanel, _T("Memory"));
			auto sizer = new wxGridBagSizer();
			int row = 0;
			
			if(instanceMode)
			{
				// Override
				memoryUseDefs = new wxCheckBox(mcPanel, ID_OverrideMemory, _("Use defaults?"));
				sizer->Add(memoryUseDefs, wxGBPosition(row, 0), wxGBSpan(1, 2), wxALL, 4);
				row++;
			}
			
			// Min memory
			minMemLabel = new wxStaticText(mcPanel, -1, _T("Minimum memory allocation: "));
			sizer->Add(minMemLabel, wxGBPosition(row,0), wxGBSpan(1,1),GBitemsFlags);
			minMemorySpin = new wxSpinCtrl(mcPanel, -1);
			minMemorySpin->SetRange(256, Utils::GetMaxAllowedMemAlloc());
			sizer->Add(minMemorySpin, wxGBPosition(row,1), wxGBSpan(1,1),GBexpandingItemsFlags);
			
			row++;
			
			// Max memory
			maxMemLabel = new wxStaticText(mcPanel, -1, _T("Maximum memory allocation: "));
			sizer->Add(maxMemLabel,wxGBPosition(row,0), wxGBSpan(1,1),GBitemsFlags);
			maxMemorySpin = new wxSpinCtrl(mcPanel, -1);
			maxMemorySpin->SetRange(512, Utils::GetMaxAllowedMemAlloc());
			sizer->Add(maxMemorySpin, wxGBPosition(row,1), wxGBSpan(1,1),GBexpandingItemsFlags);
			
			sizer->AddGrowableCol(1);
			
			box->Add(sizer, staticBoxInnerFlags);
			mcBox->Add(box, staticBoxOuterFlags);
		}
		
		// Java path
		{
			auto box = new wxStaticBoxSizer(wxVERTICAL, mcPanel, _T("Java Settings"));
			auto sizer = new wxGridBagSizer();

			int row = 0;

			if(instanceMode)
			{
				javaUseDefs = new wxCheckBox(mcPanel, ID_OverrideJava, _("Use defaults?"));
				sizer->Add(javaUseDefs, wxGBPosition(row, 0), wxGBSpan(1, 3), wxALL, 4);
				row++;
			}
			
			javaPathLabel = new wxStaticText(mcPanel, -1, _("Java Path: "));
			sizer->Add(javaPathLabel, wxGBPosition(row, 0), wxGBSpan(1, 1), GBitemsFlags);
			javaPathTextBox = new wxTextCtrl(mcPanel, -1);
			sizer->Add(javaPathTextBox, wxGBPosition(row, 1), wxGBSpan(1, 1), GBexpandingItemsFlags);
			autoDetectButton = new wxButton(mcPanel, ID_DetectJavaPath, _("Auto-detect"));
			sizer->Add(autoDetectButton, wxGBPosition(row, 2), wxGBSpan(1, 1), GBitemsFlags);
			row++;
			
			auto jvmArgsBox = new wxBoxSizer(wxHORIZONTAL);

			jvmArgsLabel = new wxStaticText(mcPanel, -1, _("JVM Arguments: "));
			sizer->Add(jvmArgsLabel, wxGBPosition(row, 0), wxGBSpan(1, 1), GBitemsFlags);

			jvmArgsTextBox = new wxTextCtrl(mcPanel, -1);
			sizer->Add(jvmArgsTextBox, wxGBPosition(row, 1), wxGBSpan(1, 2), GBexpandingItemsFlags);
			
			sizer->AddGrowableCol(1);

			box->Add(sizer, staticBoxInnerFlags);
			mcBox->Add(box, staticBoxOuterFlags);
		}
	}
	
	// Buttons
	wxSizer *btnBox = CreateButtonSizer(wxOK | wxCANCEL);
	mainBox->Add(btnBox, 0, wxALIGN_RIGHT | wxBOTTOM | wxRIGHT, 8);

	mainBox->Fit(this);
	mainBox->SetSizeHints(this);
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
	if(!instanceMode)
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
		
		currentSettings->SetShowConsole(showConsoleCheck->IsChecked());
		currentSettings->SetAutoCloseConsole(autoCloseConsoleCheck->IsChecked());
		
		currentSettings->SetAutoUpdate(autoUpdateCheck->IsChecked());
		
		currentSettings->SetConsoleSysMsgColor(sysMsgColorCtrl->GetColour());
		currentSettings->SetConsoleStdoutColor(stdoutColorCtrl->GetColour());
		currentSettings->SetConsoleStderrColor(stderrColorCtrl->GetColour());
		
		// apply instance settings to global
		UpdateMode newUpdateMode;
		if (mcUpdateDropDown->GetValue() == doUpdate)
			newUpdateMode = Update_Auto;
		else if (mcUpdateDropDown->GetValue() == dontUpdate)
			newUpdateMode = Update_Never;
		currentSettings->SetUpdateMode(newUpdateMode);
		
		currentSettings->SetMinMemAlloc(minMemorySpin->GetValue());
		currentSettings->SetMaxMemAlloc(maxMemorySpin->GetValue());

		currentSettings->SetJavaPath(javaPathTextBox->GetValue());
		currentSettings->SetJvmArgs(jvmArgsTextBox->GetValue());
		
		currentSettings->SetUseAppletWrapper(!compatCheckbox->IsChecked());

		currentSettings->SetMCWindowMaximize(winMaxCheckbox->IsChecked());
		currentSettings->SetMCWindowWidth(winWidthSpin->GetValue());
		currentSettings->SetMCWindowHeight(winHeightSpin->GetValue());

		if (useDevBuildsCheck->GetValue() && !currentSettings->GetUseDevBuilds())
		{
			// Display a warning.
			if (wxMessageBox(_("Warning: Dev builds contain incomplete, experimental, and possibly unstable features. \
Some may be extremely buggy, and others may not work at all. Use these at your own risk. \
Are you sure you want to use dev builds?"), 
				_("Are you sure?"), wxOK | wxCANCEL) == wxOK)
			{
				currentSettings->SetUseDevBuilds(useDevBuildsCheck->GetValue());
			}
		}
		else
		{
			currentSettings->SetUseDevBuilds(useDevBuildsCheck->GetValue());
		}
	}
	else
	{
		// apply instance settings to the instance
		bool haveUpdate = !updateUseDefs->GetValue();
		if(haveUpdate)
		{
			UpdateMode newUpdateMode = Update_Never;
			if (mcUpdateDropDown->GetValue() == doUpdate)
				newUpdateMode = Update_Auto;
			else if (mcUpdateDropDown->GetValue() == dontUpdate)
				newUpdateMode = Update_Never;
			currentSettings->SetUpdateMode(newUpdateMode);
		}
		else
		{
			currentSettings->ResetUpdateMode();
		}
		currentSettings->SetUpdatesOverride(haveUpdate);
		
		bool haveMemory = !memoryUseDefs->GetValue();
		if(haveMemory)
		{
			currentSettings->SetMinMemAlloc(minMemorySpin->GetValue());
			currentSettings->SetMaxMemAlloc(maxMemorySpin->GetValue());
		}
		else
		{
			currentSettings->ResetMinMemAlloc();
			currentSettings->ResetMaxMemAlloc();
		}
		currentSettings->SetMemoryOverride(haveMemory);

		bool haveJava = !javaUseDefs->GetValue();
		if(haveJava)
		{
			currentSettings->SetJavaPath(javaPathTextBox->GetValue());
			currentSettings->SetJvmArgs(jvmArgsTextBox->GetValue());
		}
		else
		{
			currentSettings->ResetJavaPath();
			currentSettings->ResetJvmArgs();
		}
		currentSettings->SetJavaOverride(haveJava);
		
		bool haveWindow = !winUseDefs->GetValue();
		if(haveWindow)
		{
			currentSettings->SetUseAppletWrapper(!compatCheckbox->GetValue());
			currentSettings->SetMCWindowMaximize(winMaxCheckbox->GetValue());
			currentSettings->SetMCWindowWidth(winWidthSpin->GetValue());
			currentSettings->SetMCWindowHeight(winHeightSpin->GetValue());
		}
		else
		{
			currentSettings->ResetMCWindowHeight();
			currentSettings->ResetMCWindowMaximize();
			currentSettings->ResetMCWindowWidth();
			currentSettings->ResetUseAppletWrapper();
		}
		currentSettings->SetWindowOverride(haveWindow);
	}
	return true;
}

void SettingsDialog::LoadSettings()
{
	if(!instanceMode)
	{
		showConsoleCheck->SetValue(currentSettings->GetShowConsole());
		autoCloseConsoleCheck->SetValue(currentSettings->GetAutoCloseConsole());

		useDevBuildsCheck->SetValue(currentSettings->GetUseDevBuilds());
		autoUpdateCheck->SetValue(currentSettings->GetAutoUpdate());

		instDirTextBox->SetValue(currentSettings->GetInstDir().GetFullPath());
		modsDirTextBox->SetValue(currentSettings->GetModsDir().GetFullPath());

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
	else
	{
		javaUseDefs->SetValue(!currentSettings->GetJavaOverride());
		memoryUseDefs->SetValue(!currentSettings->GetMemoryOverride());
		updateUseDefs->SetValue(!currentSettings->GetUpdatesOverride());
		winUseDefs->SetValue(!currentSettings->GetWindowOverride());
	}
	
	switch (currentSettings->GetUpdateMode())
	{
	case Update_Auto:
		mcUpdateDropDown->SetValue(doUpdate);
		break;
		
	case Update_Never:
		mcUpdateDropDown->SetValue(dontUpdate);
		break;
	}
	minMemorySpin->SetValue(currentSettings->GetMinMemAlloc());
	maxMemorySpin->SetValue(currentSettings->GetMaxMemAlloc());

	javaPathTextBox->SetValue(currentSettings->GetJavaPath());
	jvmArgsTextBox->SetValue(currentSettings->GetJvmArgs());

	compatCheckbox->SetValue(!currentSettings->GetUseAppletWrapper());

	winMaxCheckbox->SetValue(currentSettings->GetMCWindowMaximize());
	winWidthSpin->SetValue(currentSettings->GetMCWindowWidth());
	winHeightSpin->SetValue(currentSettings->GetMCWindowHeight());
	UpdateCheckboxStuff();
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
	if(instanceMode)
	{
		bool enableUpdates = !updateUseDefs->GetValue();
		bool enableJava = !javaUseDefs->GetValue();
		bool enableMemory = !memoryUseDefs->GetValue();
		bool enableWindow = !winUseDefs->GetValue();
		
		// java tab stuff
		javaPathTextBox->Enable(enableJava);
		jvmArgsTextBox->Enable(enableJava);
		autoDetectButton->Enable(enableJava);
		jvmArgsLabel->Enable(enableJava);;
		javaPathLabel->Enable(enableJava);;
		
		minMemorySpin->Enable(enableMemory);
		maxMemorySpin->Enable(enableMemory);
		minMemLabel->Enable(enableMemory);
		maxMemLabel->Enable(enableMemory);

		// minecraft tab stuff
		compatCheckbox->Enable(enableWindow);
		bool enableWindow2 = !compatCheckbox->GetValue() && enableWindow;
		winMaxCheckbox->Enable(enableWindow2);
		bool enableWindowSize = !winMaxCheckbox->GetValue() && enableWindow2;
		winWidthSpin->Enable(enableWindowSize);
		winHeightSpin->Enable(enableWindowSize);
		winWidthLabel->Enable(enableWindowSize);
		winHeightLabel->Enable(enableWindowSize);
		
		mcUpdateDropDown->Enable(enableUpdates);
	}
	else
	{
		winMaxCheckbox->Enable(!compatCheckbox->GetValue());

		winWidthSpin->Enable(!(winMaxCheckbox->GetValue() || compatCheckbox->GetValue()));
		winHeightSpin->Enable(!(winMaxCheckbox->GetValue() || compatCheckbox->GetValue()));
	}
}

bool SettingsDialog::GetForceUpdateMultiMC() const
{
	return !instanceMode && forceUpdateToggle->GetValue();
}


BEGIN_EVENT_TABLE(SettingsDialog, wxDialog)
	EVT_BUTTON(ID_BrowseInstDir, SettingsDialog::OnBrowseInstDirClicked)
	EVT_BUTTON(ID_BrowseModDir, SettingsDialog::OnBrowseModsDirClicked)
	EVT_BUTTON(ID_DetectJavaPath, SettingsDialog::OnDetectJavaPathClicked)
	EVT_BUTTON(wxID_OK, SettingsDialog::OnOKClicked)

	EVT_CHECKBOX(ID_MCMaximizeCheckbox, SettingsDialog::OnUpdateMCTabCheckboxes)
	EVT_CHECKBOX(ID_CompatModeCheckbox, SettingsDialog::OnUpdateMCTabCheckboxes)
	EVT_CHECKBOX(ID_OverrideJava, SettingsDialog::OnUpdateMCTabCheckboxes)
	EVT_CHECKBOX(ID_OverrideWindow, SettingsDialog::OnUpdateMCTabCheckboxes)
	EVT_CHECKBOX(ID_OverrideUpdate, SettingsDialog::OnUpdateMCTabCheckboxes)
	EVT_CHECKBOX(ID_OverrideMemory, SettingsDialog::OnUpdateMCTabCheckboxes)
END_EVENT_TABLE()
