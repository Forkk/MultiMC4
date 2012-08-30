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

#include "mainwindow.h"
#include "logindialog.h"
#include "consolewindow.h"
#include "modeditwindow.h"
#include "changeicondialog.h"

#include "resources/toolbaricons.h"

#include "multimc.h"
#include "gameupdatetask.h"
#include "logintask.h"
#include "moddertask.h"
#include <checkupdatetask.h>
#include <filedownloadtask.h>
#include "filecopytask.h"
#include "exportpacktask.h"
#include "version.h"
#include "configpack.h"
#include "importpackwizard.h"
#include "downgradedialog.h"
#include "downgradetask.h"
#include "fsutils.h"

#include <wx/filesys.h>
#include <wx/dir.h>
#include <wx/fs_arc.h>
#include <wx/fs_mem.h>
#include <wx/aboutdlg.h>
#include <wx/cmdline.h>
#include <wx/stdpaths.h>
#include <wx/listbook.h>
#include <wx/gbsizer.h>
#include <wx/filedlg.h>

#include "buildtag.h"

const int instNameLengthLimit = 25;

const wxSize minSize = wxSize(620, 400);

// Main window
MainWindow::MainWindow(void)
	: wxFrame(NULL, -1, 
		wxString::Format(_("MultiMC %d.%d.%d %s"), 
			AppVersion.GetMajor(), AppVersion.GetMinor(), AppVersion.GetRevision(),
			AppBuildTag.ToString().c_str()),
		wxPoint(0, 0), minSize),
		centralModList(settings->GetModsDir().GetFullPath())
{
	// initialize variables to sane values
	closeOnTaskEnd = false;
	editingNotes = false;
	renamingInst = false;
	instActionsEnabled = true;
	instMenu = nullptr;
	instListCtrl = nullptr;

	instPanel = nullptr;
	instSz = nullptr;
	
	btnPlay = nullptr;
	btnRename = nullptr;
	btnChangeIcon = nullptr;
	btnCopyInst = nullptr;
	btnEditMods = nullptr;
	btnDowngrade = nullptr;
	btnRebuildJar = nullptr;
	btnViewFolder = nullptr;
	
	instNotesEditor = nullptr;
	editNotesBtn = nullptr;
	cancelEditNotesBtn = nullptr;
	
	instNameSz = nullptr;
	instNameLabel = nullptr;
	instNameEditor = nullptr;
	m_currentInstance = nullptr;
	m_currentInstanceIdx = -1;
	
	//SetMinSize(minSize);
	
	SetIcons(wxGetApp().GetAppIcons());
	
	wxToolBar *mainToolBar = CreateToolBar();
	
	// Load toolbar icons
	wxBitmap newInstIcon = wxMEMORY_IMAGE(newinsticon);
	wxBitmap importCPIcon = wxMEMORY_IMAGE(importcpicon);
	wxBitmap reloadIcon = wxMEMORY_IMAGE(refreshinsticon);
	wxBitmap viewFolderIcon = wxMEMORY_IMAGE(viewfoldericon);
	wxBitmap settingsIcon = wxMEMORY_IMAGE(settingsicon);
	wxBitmap checkUpdateIcon = wxMEMORY_IMAGE(checkupdateicon);
	wxBitmap helpIcon = wxMEMORY_IMAGE(helpicon);
	wxBitmap aboutIcon = wxMEMORY_IMAGE(abouticon);
	
	// Build the toolbar
	#if (defined __WXMSW__ || defined __WXGTK__) && wxCHECK_VERSION(2, 9, 4) 
	{
		auto tool = mainToolBar->AddTool(ID_AddInst, _("Add instance"), newInstIcon, _("Add a new instance."),wxITEM_DROPDOWN);
		wxMenu* newInstanceMenu = new wxMenu();
		
		wxMenuItem* create = new wxMenuItem(0, ID_AddInst, _("Add a new instance."));
		create->SetBitmap(newInstIcon);
		((wxMenuBase*)newInstanceMenu)->Append(create);
		
		//wxMenuItem* copy = new wxMenuItem(0, ID_CopyInst, _("Copy selected instance."));
		//copy->SetBitmap(newInstIcon);
		//((wxMenuBase*)newInstanceMenu)->Append(copy);
		//
		//wxMenuItem* import = new wxMenuItem(0, ID_ImportInst, _("Import existing .minecraft folder"));
		//import->SetBitmap(newInstIcon);
		//((wxMenuBase*)newInstanceMenu)->Append(import);

		wxMenuItem* importPack = new wxMenuItem(0, ID_ImportInst, _("Import config pack"));
		importPack->SetBitmap(newInstIcon);
		((wxMenuBase*)newInstanceMenu)->Append(importPack);
		
		tool->SetDropdownMenu(newInstanceMenu);
	}
	#else
	{
		mainToolBar->AddTool(ID_AddInst, _("Add instance"), newInstIcon, _("Add a new instance."));
		//mainToolBar->AddTool(ID_CopyInst, _("Copy instance"), newInstIcon, _("Copy selected instance."));
		//mainToolBar->AddTool(ID_ImportInst, _("Import .minecraft"), newInstIcon, _("Import existing .minecraft folder"));
		mainToolBar->AddTool(ID_ImportCP, _("Import config pack"), importCPIcon, _("Import a config pack."));
	}
	#endif
	mainToolBar->AddTool(ID_Refresh, _("Refresh"), reloadIcon, _("Reload ALL the instances!"));
	mainToolBar->AddTool(ID_ViewFolder, _("View folder"), viewFolderIcon, _("Open the instance folder."));
	mainToolBar->AddSeparator();
	mainToolBar->AddTool(ID_Settings, _("Settings"), settingsIcon, _("Settings"));
	mainToolBar->AddTool(ID_CheckUpdate, _("Check for updates"), checkUpdateIcon, _("Check for MultiMC updates."));
	mainToolBar->AddSeparator();
	mainToolBar->AddTool(ID_Help, _("Help"), helpIcon, _("Help"));
	mainToolBar->AddTool(ID_About, _("About"), aboutIcon, _("About MultiMC"));
	
	mainToolBar->Realize();
	
	
	// Create the status bar
	CreateStatusBar(1);
	SetStatusBarPane(0);
	
	// Set up the main panel and sizers
	wxBoxSizer *box = new wxBoxSizer(wxVERTICAL);
	SetSizer(box);
	
	m_guiMode = settings->GetGUIMode();
	// Initialize the GUI
	switch (GetGUIMode())
	{
	case GUI_Simple:
		InitBasicGUI(box);
		break;
	case GUI_Default:
		InitAdvancedGUI(box);
		break;
	}
	CenterOnScreen();
}

MainWindow::~MainWindow(void)
{
	
}

void MainWindow::OnStartup()
{
	LoadInstanceList();
	LoadCentralModList();

	// Automatically auto-detect the Java path.
	if (settings->GetJavaPath() == _("java"))
	{
		settings->SetJavaPath(FindJavaPath());
	}

	if (settings->GetAutoUpdate())
	{
		CheckUpdateTask *task = new CheckUpdateTask();
		StartTask(*task);
	}
}

void MainWindow::InitBasicGUI(wxBoxSizer *mainSz)
{
	instListCtrl = new wxInstanceCtrl(this, ID_InstListCtrl,wxDefaultPosition,wxDefaultSize);
	instListCtrl->SetImageSize(wxSize(32,32));
	InitInstMenu();
	
	instNotesEditor = nullptr;
	
	mainSz->Add(instListCtrl, 1, wxEXPAND);
}

void MainWindow::InitInstMenu()
{
	// Build the instance context menu
	instMenu = new wxMenu();
	instMenu->Append(ID_Play, _T("&Play"), _T("Launch the instance."));
	instMenu->AppendSeparator();
	instMenu->Append(ID_Rename, _T("&Rename"), _T("Change the instance's name."));
	instMenu->Append(ID_ChangeIcon, _T("&Change Icon"), _T("Change this instance's icon."));
	instMenu->Append(ID_EditNotes, _T("&Notes"), _T("View / edit this instance's notes."));
	instMenu->AppendSeparator();
	instMenu->Append(ID_ManageSaves, _T("&Manage Saves"), _T("Backup / restore your saves."));
	instMenu->Append(ID_EditMods, _T("&Edit Mods"), _T("Install or remove mods."));
	instMenu->Append(ID_DowngradeInst, _T("Downgrade"), _T("Use MCNostalgia to downgrade this instance."));
	instMenu->Append(ID_RebuildJar, _T("Re&build Jar"), _T("Reinstall all the instance's jar mods."));
	instMenu->Append(ID_ViewInstFolder, _T("&View Folder"), _T("Open the instance's folder."));
	instMenu->AppendSeparator();
	instMenu->Append(ID_DeleteInst, _T("Delete"), _T("Delete this instance."));
}

void MainWindow::InitAdvancedGUI(wxBoxSizer *mainSz)
{
	InitInstMenu();

	instPanel = new wxPanel(this);
	mainSz->Add(instPanel, 1, wxEXPAND);
	wxGridBagSizer *instSz = new wxGridBagSizer();
	instPanel->SetSizer(instSz);
	
	const int cols = 5;
	const int rows = 3;
	
	wxFont titleFont(18, wxSWISS, wxNORMAL, wxNORMAL);
	wxFont nameEditFont(14, wxSWISS, wxNORMAL, wxNORMAL);
	
	instListCtrl = new wxInstanceCtrl(instPanel, ID_InstListCtrl,wxDefaultPosition,wxDefaultSize,wxINST_SINGLE_COLUMN);
	instListCtrl->SetImageSize(wxSize(32,32));
	instSz->Add(instListCtrl,wxGBPosition(0, 0), wxGBSpan(rows, 1),wxEXPAND);
	
	instNameSz = new wxBoxSizer(wxVERTICAL);
	instSz->Add(instNameSz, wxGBPosition(0, 1), wxGBSpan(1, cols - 2), 
		wxEXPAND | wxALL, 4);
	
	instNameEditor = new wxTextCtrl(instPanel, ID_InstNameEditor, wxEmptyString,
		wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
	instNameEditor->SetMaxLength(instNameLengthLimit);
	instNameEditor->SetFont(nameEditFont);
	instNameSz->Add(instNameEditor, wxSizerFlags(0).Align(wxALIGN_CENTER).Expand());
	
	instNameLabel = new wxStaticText(instPanel, -1, _("InstName"), 
		wxDefaultPosition, wxDefaultSize);
	instNameLabel->SetFont(titleFont);
	instNameSz->Add(instNameLabel, wxSizerFlags(0).Align(wxALIGN_CENTER));
	
	
	instNotesEditor = new wxTextCtrl(instPanel, -1, wxEmptyString,
		wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_RICH);
	instSz->Add(instNotesEditor, wxGBPosition(1, 1), wxGBSpan(rows - 2, cols - 2), 
		wxEXPAND | wxALL, 4);
	
	
	wxPanel *btnPanel = new wxPanel(instPanel);
	wxBoxSizer *btnSz = new wxBoxSizer(wxVERTICAL);
	instSz->Add(btnPanel, wxGBPosition(1, cols - 1), wxGBSpan(rows - 2, 1),
		wxALIGN_RIGHT | wxLEFT | wxRIGHT, 8);
	btnPanel->SetSizer(btnSz);
	
	const int spacerSize = 4;

	btnPlay = new wxButton(btnPanel, ID_Play, _("&Play"));
	wxSizerFlags szflags(0);
	szflags.Border(wxTOP | wxBOTTOM, 4).Expand();
	
	btnSz->Add(btnPlay, szflags);
	btnSz->AddSpacer(spacerSize);
	btnRename = new wxButton(btnPanel, ID_Rename, _("&Rename"));
	btnSz->Add(btnRename, szflags);
	btnChangeIcon = new wxButton(btnPanel, ID_ChangeIcon, _("Change &Icon"));
	btnSz->Add(btnChangeIcon, szflags);
	btnCopyInst = new wxButton(btnPanel, ID_CopyInst, _("Copy Instance"));
	btnSz->Add(btnCopyInst, szflags);
	btnSz->AddSpacer(spacerSize);
	btnEditMods = new wxButton(btnPanel, ID_EditMods, _("Edit &Mods"));
	btnSz->Add(btnEditMods, szflags);
	btnDowngrade = new wxButton(btnPanel, ID_DowngradeInst, _("Downgrade"));
	btnSz->Add(btnDowngrade, szflags);
	btnRebuildJar = new wxButton(btnPanel, ID_RebuildJar, _("Re&build Jar"));
	btnSz->Add(btnRebuildJar, szflags);
	btnViewFolder = new wxButton(btnPanel, ID_ViewInstFolder, _("&View Folder"));
	btnSz->Add(btnViewFolder, szflags);
	
	instSz->AddGrowableCol(1, 0);
	instSz->AddGrowableRow(1, 0);
	SetMinSize(instSz->ComputeFittingWindowSize(this));
	UpdateInstPanel();
}

void MainWindow::UpdateInstPanel()
{
	instPanel->Show();
	if(m_currentInstance && !renamingInst)
		EnableInstActions();
	else
		DisableInstActions();
	//UpdateInstNameLabel(inst);
	UpdateNotesBox();
	CancelRename();

	instPanel->Layout();
}

void MainWindow::UpdateInstNameLabel(Instance *inst)
{
	if(inst)
	{
		wxString instName = inst->GetName();
		if (instName.Len() > instNameLengthLimit)
		{
			instName.Truncate(instNameLengthLimit);
			instName.Trim();
			instName.Append(_("..."));
		}
		instNameLabel->SetLabel(instName);
	}
	else
	{
		instNameLabel->SetLabel(_("Select an instance"));
	}
}

void MainWindow::OnInstSelected(wxInstanceCtrlEvent &event)
{
	if(GetGUIMode() == GUI_Default)
		SaveNotesBox();
	m_currentInstanceIdx = event.GetIndex();
	m_currentInstance = GetLinkedInst(m_currentInstanceIdx);
	if(GetGUIMode() == GUI_Default)
		UpdateInstPanel();
}

void MainWindow::LoadInstanceList(wxFileName instDir)
{
	GetStatusBar()->PushStatusText(_("Loading instances..."), 0);
	
	instListCtrl->Clear();
	instItems.clear();
	
	wxDir dir(instDir.GetFullPath());
	if (!dir.IsOpened())
	{
		return;
	}
	
	Enable(false);
	wxString subFolder;
	int ctr = 0;
	bool cont = dir.GetFirst(&subFolder, wxEmptyString, wxDIR_DIRS);
	instListCtrl->Freeze();
	while (cont)
	{
		wxFileName dirName(instDir.GetFullPath(), subFolder);
		if (IsValidInstance(dirName))
		{
			Instance *inst = Instance::LoadInstance(dirName);
			if (inst != NULL)
			{
				AddInstance(inst);
			}
			
			ctr++;
		}
		cont = dir.GetNext(&subFolder);
	}
	instListCtrl->Thaw();
	GetStatusBar()->SetStatusText(wxString::Format(_("Loaded %i instances..."), ctr), 0);
	wxGetApp().Yield();
	Enable(true);
	
	
	if (GetGUIMode() == GUI_Default)
	{
		UpdateInstPanel();
	}
	
	GetStatusBar()->PopStatusText(0);
}

void MainWindow::AddInstance(Instance *inst)
{
	wxString instName = inst->GetName();
	if (instName.Len() > instNameLengthLimit)
	{
		instName.Truncate(instNameLengthLimit - 3);
		instName.Append(_("..."));
	}
	instListCtrl->Append(new wxInstanceItem(inst));
	instItems.push_back(inst);
	wxSizer * sz = GetSizer();
	if(sz)
		sz->Layout();
}

Instance* MainWindow::GetLinkedInst(int id)
{
	if(id == -1)
		return nullptr;
	return instItems[id];
}

Instance* MainWindow::GetSelectedInst()
{
	long item = instListCtrl->GetSelection();
		
	if (item == -1)
		return nullptr;
	return GetLinkedInst(item);
}

bool MainWindow::GetNewInstName(wxString *instName, wxString *instDirName, const wxString title)
{
	wxString newInstName = wxEmptyString;
Retry:
	newInstName = wxGetTextFromUser(_T("Instance name:"), 
		title, newInstName, this);

	if (newInstName.empty())
	{
		return false;
	}
	else if (newInstName.Len() > instNameLengthLimit)
	{
		wxMessageBox(_T("Sorry, that name is too long."), _T("Error"), wxOK | wxCENTER, this);
		goto Retry;
	}

	int num = 0;
	wxString dirName = Utils::RemoveInvalidPathChars(newInstName);
	while (wxDirExists(Path::Combine(settings->GetInstDir(), dirName)))
	{
		num++;
		dirName = Utils::RemoveInvalidPathChars(newInstName) + wxString::Format(_("_%i"), num);

		// If it's over 9000
		if (num > 9000)
		{
			wxLogError(_T("Couldn't create instance folder: %s"),
				Path::Combine(settings->GetInstDir(), dirName).c_str());
			goto Retry;
		}
	}

	*instName = newInstName;
	*instDirName = dirName;
	return true;
}


// Toolbar
void MainWindow::OnAddInstClicked(wxCommandEvent& event)
{
	wxString instName;
	wxString instDirName;
	if (!GetNewInstName(&instName, &instDirName))
		return;

	wxFileName instDir = wxFileName::DirName(Path::Combine(settings->GetInstDir(), instDirName));
	
	Instance *inst = new Instance(instDir);
	inst->SetName(instName);
	AddInstance(inst);
}

void MainWindow::OnImportCPClicked(wxCommandEvent& event)
{
	wxFileDialog *fileDlg = new wxFileDialog(this, _("Choose a pack to import."),
		wxEmptyString, wxEmptyString, _("*.zip"), wxFD_OPEN);
	if (fileDlg->ShowModal() == wxID_OK)
	{
		ConfigPack cp(fileDlg->GetPath());
		if (cp.IsValid())
		{
			ImportPackWizard importWiz(this, &cp);
			importWiz.Start();
		}
	}
}

void MainWindow::OnViewFolderClicked(wxCommandEvent& event)
{
	Utils::OpenFile(settings->GetInstDir());
}

void MainWindow::OnRefreshClicked(wxCommandEvent& event)
{
	LoadInstanceList();
}

void MainWindow::OnSettingsClicked(wxCommandEvent& event)
{
	SettingsDialog *settingsDlg = new SettingsDialog(this, -1);
	int response = settingsDlg->ShowModal();

	if (response == wxID_OK)
		settingsDlg->ApplySettings();
}

void MainWindow::OnCheckUpdateClicked(wxCommandEvent& event)
{
	CheckUpdateTask *task = new CheckUpdateTask();
	StartModalTask(*task);
}

void MainWindow::OnCheckUpdateComplete(CheckUpdateEvent &event)
{
#ifdef __WXMSW__
	wxString updaterFileName = _("MultiMCUpdate.exe");
#else
	wxString updaterFileName = _("MultiMCUpdate");
#endif

	if (event.m_latestBuildNumber > AppVersion.GetBuild())
	{
		if (wxMessageBox(wxString::Format(_("Build #%i is available. Would you like to download and install it?"), 
				event.m_latestBuildNumber), 
				_("Update Available"), wxYES_NO) == wxYES)
		{
			FileDownloadTask dlTask(event.m_downloadURL, 
				wxFileName(updaterFileName), _("Downloading updates..."));
			wxGetApp().updateOnExit = true;
			StartModalTask(dlTask);
			
			// Give the task dialogs some time to close.
			for (int i = 0; i < 100; i++)
			{
				wxSafeYield();
			}
			
			Close(false);
		}
	}
}

void MainWindow::OnHelpClicked(wxCommandEvent& event)
{
	NotImplemented();
}

void MainWindow::OnAboutClicked(wxCommandEvent& event)
{
#ifndef __WXMSW__
	wxAboutDialogInfo info;
	info.SetName(_("MultiMC"));
	info.SetVersion(wxString::Format(_("%s - %s"), AppVersion.ToString().c_str(), AppBuildTag.ToString().c_str()));
	info.SetDescription(_("MultiMC is a custom launcher that makes managing Minecraft easier by allowing you to have multiple installations of Minecraft at once."));
	info.SetCopyright(_("(C) 2012 Andrew Okin"));
	
#ifdef __WXGTK__
	info.SetWebSite(_("http://forkk.net/MultiMC"));
	info.SetLicense(_("Licensed under the Apache License, Version 2.0 (the \"License\");\n\
you may not use this file except in compliance with the License.\n\
You may obtain a copy of the License at\n\
\n\
\thttp://www.apache.org/licenses/LICENSE-2.0\n\
\n\
Unless required by applicable law or agreed to in writing, software\n\
distributed under the License is distributed on an \"AS IS\" BASIS,\n\
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.\n\
See the License for the specific language governing permissions and\n\
limitations under the License.\n\
\n\
MultiMC uses bspatch, \n\
Copyright 2003-2005 Colin Percival\n\
All rights reserved\n\
\n\
Redistribution and use in source and binary forms, with or without\n\
modification, are permitted providing that the following conditions\n\
are met: \n\
1. Redistributions of source code must retain the above copyright\n\
   notice, this list of conditions and the following disclaimer.\n\
2. Redistributions in binary form must reproduce the above copyright\n\
   notice, this list of conditions and the following disclaimer in the\n\
   documentation and/or other materials provided with the distribution.\n\
\n\
THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR\n\
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED\n\
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE\n\
ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY\n\
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL\n\
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS\n\
OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)\n\
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,\n\
STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING\n\
IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE\n\
POSSIBILITY OF SUCH DAMAGE."));
	
	info.SetIcon(wxGetApp().GetAppIcons().GetIcon(wxSize(128,128)));
#endif
	info.AddDeveloper(_("Andrew Okin <forkk@forkk.net>"));
	info.AddDeveloper(_("Petr Mrázek <peterix@gmail.com>"));
	
	wxAboutBox(info);
#else
	wxMessageBox(wxString::Format(_("The about dialog is currently not supported in Windows.\nYou are using MultiMC version %s.\nThis build's tag is %s."), AppVersion.ToString().c_str(), AppBuildTag.ToString().c_str()));
#endif
}


void MainWindow::NotImplemented()
{
	wxMessageBox(_T("This feature has not yet been implemented."), 
		_T("Not implemented"), wxOK | wxCENTER, this);
}


// Instance menu
void MainWindow::OnPlayClicked(wxCommandEvent& event)
{
	ShowLoginDlg(_(""));
}

void MainWindow::OnInstActivated(wxInstanceCtrlEvent &event)
{
	ShowLoginDlg(_(""));
}

void MainWindow::ShowLoginDlg(wxString errorMsg)
{
	if(!m_currentInstance)
	{
		// FIXME: what if the instance somehow becomes deselected while playing? is that possible?
		//        what does it mean to the state of the GUI
		return;
	}
	UserInfo lastLogin;
	if (wxFileExists(_("lastlogin4")))
	{
		lastLogin.LoadFromFile("lastlogin4");
	}
	
	LoginDialog loginDialog(this, errorMsg, lastLogin);
	int response = loginDialog.ShowModal();
	
	bool playOffline = response == ID_PLAY_OFFLINE;
	if (response == wxID_OK || playOffline)
	{
		UserInfo info(loginDialog);
		
		info.SaveToFile("lastlogin4");
		
		if (!playOffline)
		{
			LoginTask *task = new LoginTask(info,m_currentInstance , loginDialog.ShouldForceUpdate());
			StartModalTask(*task, true);
		}
		else
		{
			LoginCompleteEvent event(nullptr, LoginResult::PlayOffline(info.username), m_currentInstance);
			OnLoginComplete(event);
		}
	}
}

void MainWindow::OnLoginComplete(LoginCompleteEvent& event)
{
	LoginResult result = event.m_loginResult;

	if (!result.loginFailed)
	{
		// Login success
		Instance *inst = event.m_inst;

		// If the session ID is empty, the game updater will not be run.
		if (!result.playOffline && !result.sessionID.IsEmpty() && 
			!result.sessionID.Trim().IsEmpty() && result.sessionID != _("Offline"))
		{
			GameUpdateTask task(inst, result.latestVersion, _("minecraft.jar"), event.m_forceUpdate);
			if (!StartModalTask(task))
			{
				return;
			}
		}
		
		if (inst->ShouldRebuild())
		{
			ModderTask modTask(inst);
			StartModalTask(modTask);
		}
		
		if(inst->Launch(result.username, result.sessionID, true))
		{
			Show(false);
			InstConsoleWindow *cwin = new InstConsoleWindow(inst, this);
			cwin->Start();
		}
		else
		{
			ShowLoginDlg(_("Minecraft failed to launch. Check your Java path."));
		}
	}
	else
	{
		// Login failed
		ShowLoginDlg(result.errorMessage);
	}
}

void MainWindow::RenameEvent()
{
	switch (GetGUIMode())
	{
	case GUI_Default:
		StartRename();
		break;
		
	case GUI_Simple:
	{
		if(!m_currentInstance)
			break;
		wxTextEntryDialog textDlg(this, _("Enter a new name for this instance."), 
			_("Rename Instance"), m_currentInstance->GetName());
		while(1)
		{
			int response = textDlg.ShowModal();
			if(response == wxID_CANCEL)
				break;
			wxString str = textDlg.GetValue();
			if(str.length() > 25)
			{
				wxMessageBox(_T("Sorry, that name is too long. 25 characters is the limit."), _T("Error"), wxOK | wxCENTER, this);
				continue;
			}
			m_currentInstance->SetName(str);
			instListCtrl->UpdateItem(m_currentInstanceIdx);
			break;
		}
	}
	}
}

void MainWindow::OnInstRenameKey ( wxInstanceCtrlEvent& event )
{
	RenameEvent();
}

void MainWindow::OnRenameClicked(wxCommandEvent& event)
{
	RenameEvent();
}

void MainWindow::OnChangeIconClicked(wxCommandEvent& event)
{
	ChangeIconDialog iconDlg(this);
	if (iconDlg.ShowModal() == wxID_OK)
	{
		if(!m_currentInstance)
			return;
		m_currentInstance->SetIconKey(iconDlg.GetSelectedIconKey());
		instListCtrl->Refresh();
	}
}

void MainWindow::OnCopyInstClicked(wxCommandEvent &event)
{
	if(!m_currentInstance)
		return;

	wxString instName;
	wxString instDirName;
	if (!GetNewInstName(&instName, &instDirName, _("Copy existing instance")))
		return;

	instDirName = Path::Combine(settings->GetInstDir(), instDirName);

	wxMkdir(instDirName);
	FileCopyTask task(m_currentInstance->GetRootDir().GetFullPath(), wxFileName::DirName(instDirName));
	StartModalTask(task);

	Instance *newInst = new Instance(instDirName);
	newInst->SetName(instName);
	AddInstance(newInst);
}

void MainWindow::OnNotesClicked(wxCommandEvent& event)
{
	switch (GetGUIMode())
	{
	case GUI_Simple:
	{
		if(!m_currentInstance)
			return;
		wxTextEntryDialog textDlg(this, _("Instance notes"), _("Notes"), m_currentInstance->GetNotes(), 
			wxOK | wxCANCEL | wxTE_MULTILINE);
		textDlg.SetSize(600, 400);
		if (textDlg.ShowModal() == wxID_OK)
		{
			m_currentInstance->SetNotes(textDlg.GetValue());
		}
		break;
	}
	}
}

void MainWindow::SaveNotesBox()
{
	if (m_currentInstance != nullptr)
	{
		wxString notes = instNotesEditor->GetValue();
		m_currentInstance->SetNotes(notes);
		//FIXME: is this some some sort of magic?
		instNotesEditor->SetValue(m_currentInstance->GetNotes());
	}
}

void MainWindow::UpdateNotesBox()
{
	if (m_currentInstance)
		instNotesEditor->SetValue(m_currentInstance->GetNotes());
	else
		instNotesEditor->SetValue(wxString());
}


void MainWindow::StartRename()
{
	if(!m_currentInstance)
		return;
	DisableInstActions();
	renamingInst = true;
	
	GetStatusBar()->PushStatusText(wxString::Format(
		_("Renaming instance '%s'... (Press enter to finish)"), 
		m_currentInstance->GetName().c_str()), 0);
	
	instNameEditor->SetValue(m_currentInstance->GetName());
	
	instNameLabel->Show(false);
	instNameSz->Hide(instNameLabel);
	
	instNameEditor->Show(true);
	instNameSz->Show(instNameEditor);
	instNameEditor->SetFocus();
	
	instNameSz->Layout();
}

void MainWindow::FinishRename()
{
	if(m_currentInstance)
	{
		if (!instNameEditor->IsEmpty())
		{
			m_currentInstance->SetName(instNameEditor->GetValue());
			instListCtrl->UpdateItem(m_currentInstanceIdx);
		}
	}
	CancelRename();
}

void MainWindow::CancelRename()
{
	if (renamingInst)
	{
		EnableInstActions();
		renamingInst = false;
		GetStatusBar()->PopStatusText(0);
	}
	
	UpdateInstNameLabel(m_currentInstance);
	instNameEditor->Show(false);
	instNameSz->Hide(instNameEditor);
	
	instNameLabel->Show(true);
	instNameSz->Show(instNameLabel);
	
	instNameSz->Layout();
}

void MainWindow::OnRenameEnterPressed(wxCommandEvent &event)
{
	FinishRename();
}


void MainWindow::EnableInstActions(bool enabled)
{
	instActionsEnabled = enabled;
	switch (GetGUIMode())
	{
	case GUI_Default:
		btnPlay->Enable(enabled);
		btnRename->Enable(enabled);
		btnChangeIcon->Enable(enabled);
		btnEditMods->Enable(enabled);
		btnRebuildJar->Enable(enabled);
		btnViewFolder->Enable(enabled);
		btnCopyInst->Enable(enabled);
		btnDowngrade->Enable(enabled);
		break;
		
	case GUI_Simple:
		break;
	}
}

void MainWindow::DisableInstActions()
{
	EnableInstActions(false);
}


void MainWindow::OnManageSavesClicked(wxCommandEvent& event)
{
	NotImplemented();
}

void MainWindow::OnEditModsClicked(wxCommandEvent& event)
{
	if(m_currentInstance == nullptr)
		return;
	ModEditWindow *editDlg = new ModEditWindow(this, m_currentInstance);
	editDlg->Show();
}

void MainWindow::OnDowngradeInstClicked(wxCommandEvent& event)
{
	if(m_currentInstance == nullptr)
		return;

	if (m_currentInstance->GetVersionFile().FileExists())
	{
		DowngradeDialog *downDlg = new DowngradeDialog(this);
		if (downDlg->ShowModal() == wxID_OK && !downDlg->GetSelectedVersion().IsEmpty())
		{
			DowngradeTask dgTask(m_currentInstance, downDlg->GetSelectedVersion());
			StartModalTask(dgTask);
		}
	}
	else
	{
		wxLogError(_("You must run this instance at least once to download minecraft before you can downgrade it!"));
	}
}

void MainWindow::OnRebuildJarClicked(wxCommandEvent& event)
{
	if(m_currentInstance == nullptr)
		return;
	ModderTask *modTask = new ModderTask(m_currentInstance);
	StartModalTask(*modTask);
}

void MainWindow::OnViewInstFolderClicked(wxCommandEvent& event)
{
	if(m_currentInstance == nullptr)
		return;
	Utils::OpenFile(m_currentInstance->GetRootDir());
}

bool MainWindow::DeleteSelectedInstance()
{
	if(m_currentInstance == nullptr)
		return false;

	wxMessageDialog *dlg = new wxMessageDialog(this, 
		_("Are you sure you want to delete this instance?\n\
Deleted instances are lost FOREVER! (a really long time)"),
		_("Confirm deletion."),
		wxYES_NO | wxNO_DEFAULT | wxICON_QUESTION | wxCENTRE | wxSTAY_ON_TOP);
	if (dlg->ShowModal() == wxID_YES)
	{
		RecursiveDelete(m_currentInstance->GetRootDir().GetFullPath());
		instListCtrl->Delete(m_currentInstanceIdx);
		delete m_currentInstance;
		
		m_currentInstance = nullptr;
		instItems.erase(instItems.begin() + m_currentInstanceIdx);
		
		if(m_currentInstanceIdx > 0)
			instListCtrl->Select(m_currentInstanceIdx - 1);
		else if(instListCtrl->GetCount())
			instListCtrl->Select(0);
		
		m_currentInstanceIdx = instListCtrl->GetSelection();
		m_currentInstance = GetLinkedInst(m_currentInstanceIdx);
		
		if(GetGUIMode() == GUI_Default)
		{
			UpdateInstPanel();
		}
		return true;
	}
	return false;
}

void MainWindow::OnInstDeleteKey ( wxInstanceCtrlEvent& event )
{
	DeleteSelectedInstance();
}

void MainWindow::OnDeleteClicked(wxCommandEvent& event)
{
	DeleteSelectedInstance();
}

void MainWindow::OnInstMenuOpened(wxInstanceCtrlEvent& event)
{
	if(event.GetIndex() != -1)
	{
		if (instActionsEnabled)
			PopupMenu(instMenu, event.GetPosition());
	}
	else
	{
		//TODO: A menu for the instance control itself could be spawned there (with stuff like 'create instance', etc.)
	}
}


void MainWindow::OnTaskStart(TaskEvent& event)
{
	
}

void MainWindow::OnTaskEnd(TaskEvent& event)
{
	
}

void MainWindow::OnTaskProgress(TaskProgressEvent& event)
{
	
}

void MainWindow::OnTaskStatus(TaskStatusEvent& event)
{
	
}

void MainWindow::OnTaskError(TaskErrorEvent& event)
{
	wxLogError(event.m_errorMsg);
}


void MainWindow::StartTask(Task& task)
{
	task.SetEvtHandler(this);
	task.Start();
}

bool MainWindow::StartModalTask(Task& task, bool forceModal)
{
	wxYield();
	
	int style = wxPD_APP_MODAL;
	if (task.CanUserCancel())
		style = style | wxPD_CAN_ABORT;
	
	wxProgressDialog *progDialog = new wxProgressDialog(_("Please wait..."), task.GetStatus(), 100, this, style);
	progDialog->SetMinSize(wxSize(400, 80));
	progDialog->Update(0);
	progDialog->Fit();
	progDialog->CenterOnParent();
	task.SetProgressDialog(progDialog);
	task.SetEvtHandler(this);
	modalTaskRunning = true;
	task.Start();
	
	progDialog->Show();
	bool cancelled = false;
	while (!task.HasEnded())
	{
		bool retVal = true;
		if (task.GetProgress() == 0)
			retVal = progDialog->Pulse(task.GetStatus());
		else
			retVal = progDialog->Update(task.GetProgress(), task.GetStatus());
		
		if (!retVal)
		{
			if (task.CanUserCancel())
				task.Cancel();
			else
				progDialog->Resume();
		}
		progDialog->Fit();
		wxYield();
		
		wxMilliSleep(100);
	}
	//progDialog->Close(false);
	progDialog->Destroy();
	return !cancelled;
}

void MainWindow::OnWindowClosed(wxCloseEvent& event)
{
	if(instNotesEditor)
	{
		// Save instance notes on exit.
		SaveNotesBox();
	}
	wxTheApp->Exit();
}

void MainWindow::BuildConfPack(Instance *inst, const wxString &packName, 
	const wxString &packNotes, const wxString &filename, wxArrayString &includedConfigs)
{
	ExportPackTask task(inst, packName, packNotes, filename, includedConfigs);
	StartModalTask(task);
}

void MainWindow::LoadCentralModList()
{
	GetStatusBar()->PushStatusText(_("Loading central mods list..."), 0);
	centralModList.UpdateModList(true);
	GetStatusBar()->PopStatusText(0);
}

ModList* MainWindow::GetCentralModList()
{
	return &centralModList;
}


BEGIN_EVENT_TABLE(MainWindow, wxFrame)
	EVT_TOOL(ID_AddInst, MainWindow::OnAddInstClicked)
	EVT_TOOL(ID_ImportCP, MainWindow::OnImportCPClicked)
	EVT_TOOL(ID_ViewFolder, MainWindow::OnViewFolderClicked)
	EVT_TOOL(ID_Refresh, MainWindow::OnRefreshClicked)

	EVT_TOOL(ID_Settings, MainWindow::OnSettingsClicked)
	EVT_TOOL(ID_CheckUpdate, MainWindow::OnCheckUpdateClicked)

	EVT_TOOL(ID_Help, MainWindow::OnHelpClicked)
	EVT_TOOL(ID_About, MainWindow::OnAboutClicked)


	EVT_MENU(ID_Play, MainWindow::OnPlayClicked)
	
	EVT_MENU(ID_Rename, MainWindow::OnRenameClicked)
	EVT_MENU(ID_ChangeIcon, MainWindow::OnChangeIconClicked)
	EVT_MENU(ID_EditNotes, MainWindow::OnNotesClicked)
	
	EVT_MENU(ID_ManageSaves, MainWindow::OnManageSavesClicked)
	EVT_MENU(ID_EditMods, MainWindow::OnEditModsClicked)
	EVT_MENU(ID_DowngradeInst, MainWindow::OnDowngradeInstClicked)
	EVT_MENU(ID_RebuildJar, MainWindow::OnRebuildJarClicked)
	EVT_MENU(ID_ViewInstFolder, MainWindow::OnViewInstFolderClicked)
	
	EVT_MENU(ID_DeleteInst, MainWindow::OnDeleteClicked)
	
	
	EVT_BUTTON(ID_Play, MainWindow::OnPlayClicked)
	
	EVT_BUTTON(ID_Rename, MainWindow::OnRenameClicked)
	EVT_BUTTON(ID_ChangeIcon, MainWindow::OnChangeIconClicked)
	EVT_BUTTON(ID_CopyInst, MainWindow::OnCopyInstClicked)
	EVT_BUTTON(ID_EditNotes, MainWindow::OnNotesClicked)
	
	EVT_BUTTON(ID_ManageSaves, MainWindow::OnManageSavesClicked)
	EVT_BUTTON(ID_EditMods, MainWindow::OnEditModsClicked)
	EVT_BUTTON(ID_DowngradeInst, MainWindow::OnDowngradeInstClicked)
	EVT_BUTTON(ID_RebuildJar, MainWindow::OnRebuildJarClicked)
	EVT_BUTTON(ID_ViewInstFolder, MainWindow::OnViewInstFolderClicked)
	
	EVT_BUTTON(ID_DeleteInst, MainWindow::OnDeleteClicked)
	
	
	EVT_INST_LEFT_DCLICK(ID_InstListCtrl, MainWindow::OnInstActivated)
	EVT_INST_RETURN(ID_InstListCtrl, MainWindow::OnInstActivated)
	EVT_INST_DELETE(ID_InstListCtrl, MainWindow::OnInstDeleteKey)
	EVT_INST_RENAME(ID_InstListCtrl, MainWindow::OnInstRenameKey)
	EVT_INST_RIGHT_CLICK(ID_InstListCtrl, MainWindow::OnInstMenuOpened)
	EVT_INST_ITEM_SELECTED(ID_InstListCtrl, MainWindow::OnInstSelected)
	
	EVT_TASK_START(MainWindow::OnTaskStart)
	EVT_TASK_END(MainWindow::OnTaskEnd)
	EVT_TASK_STATUS(MainWindow::OnTaskStatus)
	EVT_TASK_PROGRESS(MainWindow::OnTaskProgress)
	EVT_TASK_ERRORMSG(MainWindow::OnTaskError)
	
	EVT_LOGIN_COMPLETE(MainWindow::OnLoginComplete)
	EVT_CHECK_UPDATE(MainWindow::OnCheckUpdateComplete)
	
	EVT_TEXT_ENTER(ID_InstNameEditor, MainWindow::OnRenameEnterPressed)

	EVT_CLOSE(MainWindow::OnWindowClosed)
END_EVENT_TABLE()
