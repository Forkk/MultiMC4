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
#include "mainwindow.h"

#include <wx/gbsizer.h>
#include <wx/dir.h>
#include <wx/valnum.h>

#include <utils/apputils.h>
#include <utils/fsutils.h>

#include "multimc.h"
#include "instance.h"

const wxString guiModeFancy = _("Fancy");
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

const wxString sortModeName = _("By Name");
const wxString sortModeLastLaunch = _("By Last Launched");

SettingsDialog::SettingsDialog( wxWindow* parent, wxWindowID id, SettingsBase* s /* = settings */)
	: wxDialog(parent, id, _("Settings"), wxDefaultPosition, wxSize(500, 450))
{
	parent_w = (MainWindow *) parent;
	m_shouldRestartMMC = false;
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
	
	
	if(!instanceMode)
	{
		// MultiMC tab
		{
			auto multimcPanel = new wxPanel(tabCtrl, -1);
			auto multimcSizer = new wxBoxSizer(wxVERTICAL);
			multimcPanel->SetSizer(multimcSizer);
			tabCtrl->AddPage(multimcPanel, _T("MultiMC"), true);

			// Visual settings group box
			{
				wxArrayString guiModeChoices;
				guiModeChoices.Add(guiModeSimple);
				guiModeChoices.Add(guiModeFancy);
				guiStyleBox = new wxRadioBox(multimcPanel, -1, 
					_("GUI Style (requires restart)"), wxDefaultPosition, 
					wxDefaultSize, guiModeChoices);
				multimcSizer->Add(guiStyleBox, staticBoxOuterFlags);
			}

			// Sort mode group box
			{
				wxArrayString sortModeChoices;
				sortModeChoices.Add(sortModeName);
				sortModeChoices.Add(sortModeLastLaunch);
				sortModeBox = new wxRadioBox(multimcPanel, -1, _("Sorting Mode"), 
					wxDefaultPosition, wxDefaultSize, sortModeChoices);
				multimcSizer->Add(sortModeBox, staticBoxOuterFlags);
			}

			// Language combo box
			{
				auto box = new wxStaticBoxSizer(wxVERTICAL, multimcPanel, _("Language"));
				useSystemLangCheck = new wxCheckBox(box->GetStaticBox(), ID_UseSystemLang, 
					_("Use the system language?"));
				box->Add(useSystemLangCheck, itemsFlags);

				langSelectorBox = new wxComboBox(box->GetStaticBox(), -1, 
					wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, nullptr, 
					wxCB_DROPDOWN | wxCB_READONLY);
				box->Add(langSelectorBox, expandingItemFlags);

				multimcSizer->Add(box, staticBoxOuterFlags);
			}

			// Update settings group box
			{
				auto box = new wxStaticBoxSizer(wxVERTICAL, multimcPanel, _("Update Settings"));
				useDevBuildsCheck = new wxCheckBox(box->GetStaticBox(), -1, _("Use development builds."));
				box->Add(useDevBuildsCheck, itemFlags);
				autoUpdateCheck = new wxCheckBox(box->GetStaticBox(), -1, _("Check for updates when MultiMC starts?"));
				box->Add(autoUpdateCheck, itemFlags);
				multimcSizer->Add(box, staticBoxOuterFlags);
			}

			// Directory group box
			{
				auto box = new wxStaticBoxSizer(wxVERTICAL, multimcPanel, _("Folders"));
				auto dirsSizer = new wxGridBagSizer();
				int row = 0;
				{
					auto instDirLabel = new wxStaticText(box->GetStaticBox(), -1, _("Instances: "));
					dirsSizer->Add(instDirLabel, wxGBPosition(row, 0), wxGBSpan(1, 1), GBitemsFlags);

					instDirTextBox = new wxTextCtrl(box->GetStaticBox(), -1);
					dirsSizer->Add(instDirTextBox, wxGBPosition(row, 1), wxGBSpan(1, 1), GBexpandingItemsFlags);

					wxButton *instDirBrowseButton = new wxButton(box->GetStaticBox(), ID_BrowseInstDir, _("Browse..."));
					dirsSizer->Add(instDirBrowseButton, wxGBPosition(row, 2), wxGBSpan(1, 1), GBitemsFlags);
				}
				row++;
				{
					auto modsDirLabel = new wxStaticText(box->GetStaticBox(), -1, _("Mods: "));
					dirsSizer->Add(modsDirLabel, wxGBPosition(row, 0), wxGBSpan(1, 1), GBitemsFlags);

					modsDirTextBox = new wxTextCtrl(box->GetStaticBox(), -1);
					dirsSizer->Add(modsDirTextBox, wxGBPosition(row, 1), wxGBSpan(1, 1), GBexpandingItemsFlags);

					auto modDirBrowseButton = new wxButton(box->GetStaticBox(), ID_BrowseModDir, _("Browse..."));
					dirsSizer->Add(modDirBrowseButton, wxGBPosition(row, 2), wxGBSpan(1, 1), GBitemsFlags);
				}
				row++;
				{
					auto lwjglDirLabel = new wxStaticText(box->GetStaticBox(), -1, _("LWJGL: "));
					dirsSizer->Add(lwjglDirLabel, wxGBPosition(row, 0), wxGBSpan(1, 1), GBitemsFlags);

					lwjglDirTextBox = new wxTextCtrl(box->GetStaticBox(), -1);
					dirsSizer->Add(lwjglDirTextBox, wxGBPosition(row, 1), wxGBSpan(1, 1), GBexpandingItemsFlags);

					auto lwjglDirBrowseButton = new wxButton(box->GetStaticBox(), ID_BrowseLwjglDir, _("Browse..."));
					dirsSizer->Add(lwjglDirBrowseButton, wxGBPosition(row, 2), wxGBSpan(1, 1), GBitemsFlags);
				}
				dirsSizer->AddGrowableCol(1);

				box->Add(dirsSizer, staticBoxInnerFlags);
				multimcSizer->Add(box, staticBoxOuterFlags);
			}
			multimcSizer->SetSizeHints(multimcPanel);
		}
	}

	// Network tab
	if (!instanceMode)
	{
		auto networkPanel = new wxPanel(tabCtrl, -1);
		auto networkSz = new wxBoxSizer(wxVERTICAL);
		networkPanel->SetSizer(networkSz);
		tabCtrl->AddPage(networkPanel, _("Network"), false);

		// Proxy settings box
		{
			auto box = new wxStaticBoxSizer(wxVERTICAL, networkPanel, _("Proxy"));

			// Proxy type
			box->AddSpacer(4);
			wxStaticText* proxyTypeLabel = new wxStaticText(box->GetStaticBox(), 
				-1, _("Proxy Type: "));
			box->Add(proxyTypeLabel, wxSizerFlags().Border(wxLEFT, 4));

			wxBoxSizer* pTypeSz = new wxBoxSizer(wxHORIZONTAL);
			box->Add(pTypeSz, wxSizerFlags(1).Border(wxBOTTOM | wxLEFT | wxRIGHT, 4).Expand());

			auto rbtnSzFlags = itemFlags.Align(wxALIGN_TOP);
			noProxyRBtn = new wxRadioButton(box->GetStaticBox(), ID_UseProxy, _("None"), 
				wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
			pTypeSz->Add(noProxyRBtn, rbtnSzFlags);
			httpProxyRBtn = new wxRadioButton(box->GetStaticBox(), ID_UseProxy, _("HTTP"));
			pTypeSz->Add(httpProxyRBtn, rbtnSzFlags);
			socks4ProxyRBtn = new wxRadioButton(box->GetStaticBox(), ID_UseProxy, _("SOCKS4"));
			pTypeSz->Add(socks4ProxyRBtn, rbtnSzFlags);
			socks5ProxyRBtn = new wxRadioButton(box->GetStaticBox(), ID_UseProxy, _("SOCKS5"));
			pTypeSz->Add(socks5ProxyRBtn, rbtnSzFlags);


			// Host / port
			box->AddSpacer(4);
			wxStaticText* hostPortLabel = new wxStaticText(box->GetStaticBox(), 
				-1, _("Hostname / IP and Port:"));
			box->Add(hostPortLabel, wxSizerFlags().Border(wxLEFT, 4));

			wxBoxSizer* hostPortSz = new wxBoxSizer(wxHORIZONTAL);
			box->Add(hostPortSz, wxSizerFlags(1).Border(wxBOTTOM | wxLEFT | wxRIGHT, 4).Expand());

			proxyHostTextbox = new wxTextCtrl(box->GetStaticBox(), -1);
			hostPortSz->Add(proxyHostTextbox, expandingItemsFlags);

			proxyPortTextbox = new wxTextCtrl(box->GetStaticBox(), -1);
			wxIntegerValidator<long> portValidator(&proxyPortValue);
			portValidator.SetMin(1);
			portValidator.SetMax(65535);
			proxyPortTextbox->SetValidator(portValidator);
			hostPortSz->Add(proxyPortTextbox, itemsFlags);

			
			// Username / password
			box->AddSpacer(4);
			wxGridBagSizer* credentialsSz = new wxGridBagSizer();
			box->Add(credentialsSz, staticBoxInnerFlags);

			wxStaticText* proxyUserLabel = new wxStaticText(box->GetStaticBox(), 
				-1, _("Username:"));
			credentialsSz->Add(proxyUserLabel, wxGBPosition(0, 0), wxGBSpan(1, 1), GBitemsFlags);
			proxyUserTextbox = new wxTextCtrl(box->GetStaticBox(), -1);
			credentialsSz->Add(proxyUserTextbox, wxGBPosition(0, 1), wxGBSpan(1, 1), GBexpandingItemsFlags);

			wxStaticText* proxyPassLabel = new wxStaticText(box->GetStaticBox(), 
				-1, _("Password:"));
			credentialsSz->Add(proxyPassLabel, wxGBPosition(1, 0), wxGBSpan(1, 1), GBitemsFlags);
			proxyPassTextbox = new wxTextCtrl(box->GetStaticBox(), -1, wxEmptyString, 
				wxDefaultPosition, wxDefaultSize, wxTE_PASSWORD);
			credentialsSz->Add(proxyPassTextbox, wxGBPosition(1, 1), wxGBSpan(1, 1), GBexpandingItemsFlags);

			wxStaticText* passwarnLabel = new wxStaticText(box->GetStaticBox(), -1, 
				_("Warning: Proxy password is stored in plaintext!"));
			passwarnLabel->SetForegroundColour(wxColor("red"));
			credentialsSz->Add(passwarnLabel, wxGBPosition(2, 0), wxGBSpan(1, 2), GBitemsFlags);

			credentialsSz->AddGrowableCol(1);

			networkSz->Add(box, staticBoxOuterFlags);
		}
	}

	// Console tab
	if(!instanceMode)
	{
		auto consolePanel = new wxPanel(tabCtrl, -1);
		auto consoleSizer = new wxBoxSizer(wxVERTICAL);
		consolePanel->SetSizer(consoleSizer);
		tabCtrl->AddPage(consolePanel, _("Console"), false);
		// Console settings group box
		{
			auto box = new wxStaticBoxSizer(wxVERTICAL, consolePanel, _("Console Settings"));
			showConsoleCheck = new wxCheckBox(box->GetStaticBox(), -1, _("Show console while an instance is running."));
			box->Add(showConsoleCheck, itemFlags);
			autoCloseConsoleCheck = new wxCheckBox(box->GetStaticBox(), -1, _("Automatically close console when the game quits."));
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
		tabCtrl->AddPage(mcPanel, _("Minecraft"), false);

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

		// Login group box
		{
			auto box = new wxStaticBoxSizer(wxVERTICAL, mcPanel, _("Login"));

			if (instanceMode)
			{
				loginUseDefs = new wxCheckBox(box->GetStaticBox(), ID_OverrideLogin, 
					_("Use defaults?"));
				box->Add(loginUseDefs, itemsFlags);
			}

			autoLoginCheck = new wxCheckBox(box->GetStaticBox(), -1, 
				_("Log in automatically when I launch an instance."));
			box->Add(autoLoginCheck, itemsFlags);

			mcBox->Add(box, staticBoxOuterFlags);
		}
	}
	{
		auto mcPanel = new wxPanel(tabCtrl, -1);
		auto mcBox = new wxBoxSizer(wxVERTICAL);
		mcPanel->SetSizer(mcBox);
		tabCtrl->AddPage(mcPanel, _("Java"), false);
		// Memory group box
		{
			auto box = new wxStaticBoxSizer(wxVERTICAL, mcPanel, _("Memory"));
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
			minMemLabel = new wxStaticText(mcPanel, -1, _("Minimum memory allocation: "));
			sizer->Add(minMemLabel, wxGBPosition(row,0), wxGBSpan(1,1),GBitemsFlags);
			minMemorySpin = new wxSpinCtrl(mcPanel, -1);
			minMemorySpin->SetRange(256, Utils::GetMaxAllowedMemAlloc());
			sizer->Add(minMemorySpin, wxGBPosition(row,1), wxGBSpan(1,1),GBexpandingItemsFlags);
			
			row++;
			
			// Max memory
			maxMemLabel = new wxStaticText(mcPanel, -1, _("Maximum memory allocation: "));
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
			auto box = new wxStaticBoxSizer(wxVERTICAL, mcPanel, _("Java Settings"));
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

		// Launch commands
		{
			auto box = new wxStaticBoxSizer(wxVERTICAL, mcPanel, _("Custom Commands"));
			auto sizer = new wxGridBagSizer();

			int row = 0;

			if(instanceMode)
			{
				cCmdsUseDefs = new wxCheckBox(mcPanel, ID_OverrideCCmds, _("Use defaults?"));
				sizer->Add(cCmdsUseDefs, wxGBPosition(row, 0), wxGBSpan(1, 3), wxALL, 4);
				row++;
			}

			auto preLaunchCmdLabel = new wxStaticText(mcPanel, -1, _("Pre-launch command: "));
			sizer->Add(preLaunchCmdLabel, wxGBPosition(row, 0), wxGBSpan(1, 1), GBitemsFlags);
			preLaunchCmdBox = new wxTextCtrl(mcPanel, -1);
			sizer->Add(preLaunchCmdBox, wxGBPosition(row, 1), wxGBSpan(1, 1), GBexpandingItemsFlags);

			row++;

			auto postExitCmdLabel = new wxStaticText(mcPanel, -1, _("Post-exit command: "));
			sizer->Add(postExitCmdLabel, wxGBPosition(row, 0), wxGBSpan(1, 1), GBitemsFlags);
			postExitCmdBox = new wxTextCtrl(mcPanel, -1);
			sizer->Add(postExitCmdBox, wxGBPosition(row, 1), wxGBSpan(1, 1), GBexpandingItemsFlags);

			row++;

			auto infoLabel = new wxStaticText(mcPanel, -1, 
				_("Pre-launch command runs before the instance launches and "
				"post-exit command runs after it exits. Both will be run in "
				"MultiMC's working directory, with INST_ID, INST_NAME, and "
				"INST_DIR as environment variables."));
			sizer->Add(infoLabel, wxGBPosition(row, 0), wxGBSpan(1, 2), GBexpandingItemsFlags);
			infoLabel->Wrap(320);

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
	else
	{
		if(!newDir.Mkdir(0777,wxPATH_MKDIR_FULL))
		{
			wxLogError(_("Failed to create the new folder: %s"), newDir.GetFullPath().c_str());
			return false;
		}
	}
	return true;
}

bool SettingsDialog::ApplySettings()
{
	// True if MultiMC needs to restart to apply the settings.
	bool needsRestart = false;

	if(!instanceMode)
	{
		wxFileName newInstDir = wxFileName::DirName(instDirTextBox->GetValue());
		wxFileName oldInstDir = currentSettings->GetInstDir();
		wxFileName test = newInstDir;
		test.MakeAbsolute();
		wxString tests = test.GetFullPath();
		if(tests.Contains("!"))
		{
			wxLogError(_("The chosen instance path contains a ! character.\nThis would make Minecraft crash. Please change it."), tests.c_str());
			return false;
		}
		if (!oldInstDir.SameAs(newInstDir))
		{
			if(!FolderMove(oldInstDir, newInstDir,
				_("You've changed your instance directory, would you like to transfer all of your instances?"),
				_("Instance directory changed.")))
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
				_("You've changed your central mods directory, would you like to transfer all of your mods?"),
				_("Central mods directory changed.")))
			{
				return false;
			}
			else
			{
				// NUKE ALL MODS, FOLDER CHANGED
				auto mlist = parent_w->GetCentralModList();
				mlist->clear();
				mlist->SetDir(newModDir.GetFullPath());
				mlist->UpdateModList();
			}
		}
		currentSettings->SetModsDir(newModDir);
		
		wxFileName newLwjglDir = wxFileName::DirName(lwjglDirTextBox->GetValue());
		wxFileName oldLwjglDir = currentSettings->GetLwjglDir();
		if (!oldLwjglDir.SameAs(newLwjglDir))
		{
			if(!FolderMove(oldLwjglDir, newLwjglDir,
				_("You've changed your lwjgl directory, would you like to transfer all of your lwjgl versions?"),
				_("Lwjgl directory changed.")))
			{
				return false;
			}
		}
		currentSettings->SetLwjglDir(newLwjglDir);
		
		GUIMode newGUIMode;
		if (guiStyleBox->GetStringSelection() == guiModeFancy)
			newGUIMode = GUI_Fancy;
		else if (guiStyleBox->GetStringSelection() == guiModeSimple)
			newGUIMode = GUI_Simple;
		
		if (newGUIMode != currentSettings->GetGUIMode())
		{
			currentSettings->SetGUIMode(newGUIMode);
			needsRestart = true;
		}

		if (sortModeBox->GetStringSelection() == sortModeName)
			currentSettings->SetInstSortMode(Sort_Name);
		else if (sortModeBox->GetStringSelection() == sortModeLastLaunch)
			currentSettings->SetInstSortMode(Sort_LastLaunch);
		
		currentSettings->SetShowConsole(showConsoleCheck->IsChecked());
		currentSettings->SetAutoCloseConsole(autoCloseConsoleCheck->IsChecked());
		
		currentSettings->SetAutoUpdate(autoUpdateCheck->IsChecked());
		
		currentSettings->SetConsoleSysMsgColor(sysMsgColorCtrl->GetColour());
		currentSettings->SetConsoleStdoutColor(stdoutColorCtrl->GetColour());
		currentSettings->SetConsoleStderrColor(stderrColorCtrl->GetColour());
		
		// apply instance settings to global
		currentSettings->SetAutoLogin(autoLoginCheck->GetValue());
		
		currentSettings->SetMinMemAlloc(minMemorySpin->GetValue());
		currentSettings->SetMaxMemAlloc(maxMemorySpin->GetValue());

		currentSettings->SetJavaPath(javaPathTextBox->GetValue());
		currentSettings->SetJvmArgs(jvmArgsTextBox->GetValue());

		currentSettings->SetPreLaunchCmd(preLaunchCmdBox->GetValue());
		currentSettings->SetPostExitCmd(postExitCmdBox->GetValue());
		
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


		// Apply language settings.
		if (currentSettings->GetUseSystemLang() != useSystemLangCheck->GetValue())
			needsRestart = true;

		currentSettings->SetUseSystemLang(useSystemLangCheck->GetValue());
		if (!useSystemLangCheck->GetValue())
		{
			bool languageSet = false;
			wxString langName = langSelectorBox->GetStringSelection();
			const LanguageArray* langs = wxGetApp().localeHelper.GetLanguages();
			for (unsigned i = 0; i < langs->size(); i++)
			{
				if (langs->operator[](i).m_name == langName &&
					langs->operator[](i).m_canonicalName != currentSettings->GetLanguage())
				{
					// Set the language.
					currentSettings->SetLanguage(langs->operator[](i).m_canonicalName);
					needsRestart = true;
				}
			}
		}

		// Proxy settings
		if (noProxyRBtn->GetValue())
			currentSettings->SetProxyType(Proxy_None);
		else if (httpProxyRBtn->GetValue())
			currentSettings->SetProxyType(Proxy_HTTP);
		else if (socks4ProxyRBtn->GetValue())
			currentSettings->SetProxyType(Proxy_SOCKS4);
		else if (socks5ProxyRBtn->GetValue())
			currentSettings->SetProxyType(Proxy_SOCKS5);

		currentSettings->SetProxyHostName(proxyHostTextbox->GetValue());
		proxyPortTextbox->GetValidator()->TransferFromWindow();
		currentSettings->SetProxyPort(proxyPortValue);

		currentSettings->SetProxyUsername(proxyUserTextbox->GetValue());
		currentSettings->SetProxyPassword(proxyPassTextbox->GetValue());
	}
	else
	{
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

		bool haveCCmds = !cCmdsUseDefs->GetValue();
		if (haveCCmds)
		{
			currentSettings->SetPreLaunchCmd(preLaunchCmdBox->GetValue());
			currentSettings->SetPostExitCmd(postExitCmdBox->GetValue());
		}
		else
		{
			currentSettings->ResetPreLaunchCmd();
			currentSettings->ResetPostExitCmd();
		}
		currentSettings->SetLaunchCmdOverride(haveCCmds);
		
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

		bool haveLogin = !loginUseDefs->GetValue();
		if (haveLogin)
		{
			currentSettings->SetAutoLogin(autoLoginCheck->GetValue());
		}
		else
		{
			currentSettings->ResetAutoLogin();
		}
	}
	
	if (needsRestart)
	{
		if (wxMessageBox(
			_("Some settings were changed that require MultiMC to restart before they take effect. "
			  "Would you like to restart MultiMC now?"),
			_("Restart Required"), wxYES_NO) == wxYES)
		{
			m_shouldRestartMMC = true;
		}
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
		lwjglDirTextBox->SetValue(currentSettings->GetLwjglDir().GetFullPath());

		sysMsgColorCtrl->SetColour(currentSettings->GetConsoleSysMsgColor());
		stdoutColorCtrl->SetColour(currentSettings->GetConsoleStdoutColor());
		stderrColorCtrl->SetColour(currentSettings->GetConsoleStderrColor());
		
		switch (currentSettings->GetGUIMode())
		{
		case GUI_Simple:
			guiStyleBox->SetStringSelection(guiModeSimple);
			break;
			
		case GUI_Fancy:
			guiStyleBox->SetStringSelection(guiModeFancy);
			break;
		}

		switch (currentSettings->GetInstSortMode())
		{
		case Sort_Name:
			sortModeBox->SetStringSelection(sortModeName);
			break;

		case Sort_LastLaunch:
			sortModeBox->SetStringSelection(sortModeLastLaunch);
			break;
		}

		int selectedIndex = -1;
		const LanguageArray* langs = wxGetApp().localeHelper.GetLanguages();
		for (unsigned i = 0; i < langs->size(); i++)
		{
			wxString langCName = langs->operator[](i).m_canonicalName;
			langSelectorBox->Append(langs->operator[](i).m_name);
			if (langCName == currentSettings->GetLanguage())
				selectedIndex = i;
		}
		if (selectedIndex == -1)
		{
			langSelectorBox->Insert(_("Unsupported Language"), 0);
			selectedIndex = 0;
		}
		langSelectorBox->SetSelection(selectedIndex);

		useSystemLangCheck->SetValue(currentSettings->GetUseSystemLang());

		// Proxy settings
		switch (currentSettings->GetProxyType())
		{
		case Proxy_None:
			noProxyRBtn->SetValue(true);
			break;

		case Proxy_HTTP:
			httpProxyRBtn->SetValue(true);
			break;

		case Proxy_SOCKS4:
			socks4ProxyRBtn->SetValue(true);
			break;

		case Proxy_SOCKS5:
			socks5ProxyRBtn->SetValue(true);
			break;
		}

		proxyHostTextbox->SetValue(currentSettings->GetProxyHostName());
		proxyPortValue = currentSettings->GetProxyPort();
		proxyPortTextbox->GetValidator()->TransferToWindow();

		proxyUserTextbox->SetValue(currentSettings->GetProxyUsername());
		proxyPassTextbox->SetValue(currentSettings->GetProxyPassword());
	}
	else
	{
		javaUseDefs->SetValue(!currentSettings->GetJavaOverride());
		cCmdsUseDefs->SetValue(!currentSettings->GetLaunchCmdOverride());
		memoryUseDefs->SetValue(!currentSettings->GetMemoryOverride());
		winUseDefs->SetValue(!currentSettings->GetWindowOverride());
		loginUseDefs->SetValue(!currentSettings->GetLoginOverride());
	}
	
	minMemorySpin->SetValue(currentSettings->GetMinMemAlloc());
	maxMemorySpin->SetValue(currentSettings->GetMaxMemAlloc());

	javaPathTextBox->SetValue(currentSettings->GetJavaPath());
	jvmArgsTextBox->SetValue(currentSettings->GetJvmArgs());

	preLaunchCmdBox->SetValue(currentSettings->GetPreLaunchCmd());
	postExitCmdBox->SetValue(currentSettings->GetPostExitCmd());

	compatCheckbox->SetValue(!currentSettings->GetUseAppletWrapper());

	winMaxCheckbox->SetValue(currentSettings->GetMCWindowMaximize());
	winWidthSpin->SetValue(currentSettings->GetMCWindowWidth());
	winHeightSpin->SetValue(currentSettings->GetMCWindowHeight());

	autoLoginCheck->SetValue(currentSettings->GetAutoLogin());

	UpdateCheckboxStuff();
}

void SettingsDialog::OnBrowseInstDirClicked(wxCommandEvent& event)
{
	wxDirDialog dirDlg (this, "Select a new instance folder.", instDirTextBox->GetValue());
	if (dirDlg.ShowModal() == wxID_OK)
	{
		wxFileName a = dirDlg.GetPath();
		if(fsutils::isSubsetOf(a,wxGetCwd()))
			a.MakeRelativeTo();
		if(a.SameAs(wxGetCwd()))
		{
			instDirTextBox->ChangeValue(".");
		}
		else
		{
			instDirTextBox->ChangeValue(a.GetFullPath());
		}
	}
}

void SettingsDialog::OnBrowseModsDirClicked(wxCommandEvent& event)
{
	wxDirDialog dirDlg (this, "Select a new central mods folder.", modsDirTextBox->GetValue());
	if (dirDlg.ShowModal() == wxID_OK)
	{
		wxFileName a = dirDlg.GetPath();
		if(fsutils::isSubsetOf(a,wxGetCwd()))
			a.MakeRelativeTo();
		if(a.SameAs(wxGetCwd()))
		{
			modsDirTextBox->ChangeValue(".");
		}
		else
		{
			modsDirTextBox->ChangeValue(a.GetFullPath());
		}
	}
}



void SettingsDialog::OnBrowseLwjglDirClicked(wxCommandEvent& event)
{
	wxDirDialog dirDlg (this, "Select a new folder for storing LWJGL versions.", lwjglDirTextBox->GetValue());
	if (dirDlg.ShowModal() == wxID_OK)
	{
		wxFileName a = dirDlg.GetPath();
		if(fsutils::isSubsetOf(a,wxGetCwd()))
			a.MakeRelativeTo();
		if(a.SameAs(wxGetCwd()))
		{
			lwjglDirTextBox->ChangeValue(".");
		}
		else
		{
			lwjglDirTextBox->ChangeValue(a.GetFullPath());
		}
	}
}

void SettingsDialog::OnDetectJavaPathClicked(wxCommandEvent& event)
{
	wxString newJPath = FindJavaPath(javaPathTextBox->GetValue());
	javaPathTextBox->SetValue(newJPath);
}


void SettingsDialog::OnUpdateCheckboxes(wxCommandEvent& event)
{
	UpdateCheckboxStuff();
}

void SettingsDialog::UpdateCheckboxStuff()
{
	if(instanceMode)
	{
		bool enableJava = !javaUseDefs->GetValue();
		bool enableCCmds = !cCmdsUseDefs->GetValue();
		bool enableMemory = !memoryUseDefs->GetValue();
		bool enableWindow = !winUseDefs->GetValue();
		bool enableLogin = !loginUseDefs->GetValue();
		
		// java tab stuff
		javaPathTextBox->Enable(enableJava);
		jvmArgsTextBox->Enable(enableJava);
		autoDetectButton->Enable(enableJava);
		jvmArgsLabel->Enable(enableJava);
		javaPathLabel->Enable(enableJava);

		preLaunchCmdBox->Enable(enableCCmds);
		postExitCmdBox->Enable(enableCCmds);
		
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
		
		autoLoginCheck->Enable(enableLogin);
	}
	else
	{
		winMaxCheckbox->Enable(!compatCheckbox->GetValue());

		winWidthSpin->Enable(!(winMaxCheckbox->GetValue() || compatCheckbox->GetValue()));
		winHeightSpin->Enable(!(winMaxCheckbox->GetValue() || compatCheckbox->GetValue()));

		langSelectorBox->Enable(!useSystemLangCheck->GetValue());

		proxyHostTextbox->Enable(!noProxyRBtn->GetValue());
		proxyPortTextbox->Enable(!noProxyRBtn->GetValue());
		proxyUserTextbox->Enable(!noProxyRBtn->GetValue());
		proxyPassTextbox->Enable(!noProxyRBtn->GetValue());
	}
}

bool SettingsDialog::ShouldRestartNow() const
{
	return m_shouldRestartMMC;
}


BEGIN_EVENT_TABLE(SettingsDialog, wxDialog)
	EVT_BUTTON(ID_BrowseInstDir, SettingsDialog::OnBrowseInstDirClicked)
	EVT_BUTTON(ID_BrowseModDir, SettingsDialog::OnBrowseModsDirClicked)
	EVT_BUTTON(ID_BrowseLwjglDir, SettingsDialog::OnBrowseLwjglDirClicked)
	EVT_BUTTON(ID_DetectJavaPath, SettingsDialog::OnDetectJavaPathClicked)
	EVT_BUTTON(wxID_OK, SettingsDialog::OnOKClicked)

	EVT_CHECKBOX(ID_MCMaximizeCheckbox, SettingsDialog::OnUpdateCheckboxes)
	EVT_CHECKBOX(ID_CompatModeCheckbox, SettingsDialog::OnUpdateCheckboxes)
	EVT_CHECKBOX(ID_UseSystemLang, SettingsDialog::OnUpdateCheckboxes)
	EVT_CHECKBOX(ID_OverrideJava, SettingsDialog::OnUpdateCheckboxes)
	EVT_CHECKBOX(ID_OverrideWindow, SettingsDialog::OnUpdateCheckboxes)
	EVT_CHECKBOX(ID_OverrideUpdate, SettingsDialog::OnUpdateCheckboxes)
	EVT_CHECKBOX(ID_OverrideMemory, SettingsDialog::OnUpdateCheckboxes)
	EVT_CHECKBOX(ID_OverrideLogin, SettingsDialog::OnUpdateCheckboxes)
	EVT_CHECKBOX(ID_OverrideCCmds, SettingsDialog::OnUpdateCheckboxes)
	EVT_RADIOBUTTON(ID_UseProxy, SettingsDialog::OnUpdateCheckboxes)
END_EVENT_TABLE()
