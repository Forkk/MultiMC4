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

const int instNameLengthLimit = 25;

const wxSize minSize = wxSize(620, 400);

// Main window
MainWindow::MainWindow(void)
	: wxFrame(NULL, -1, 
		wxString::Format(_("MultiMC %d.%d.%d Build %d %s"), 
			AppVersion.GetMajor(), AppVersion.GetMinor(), AppVersion.GetRevision(), AppVersion.GetBuild(),
			(AppVersion.IsDevBuild() ? _("Dev") : _("Stable"))),
		wxPoint(0, 0), minSize),
		centralModList(settings.GetModsDir().GetFullPath())
{
	// initialize variables to sane values
	closeOnTaskEnd = false;
	editingNotes = false;
	renamingInst = false;
	instActionsEnabled = true;
	instMenu = nullptr;
	instListCtrl = nullptr;

	instListbook = nullptr;
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
	
	SetMinSize(minSize);
	
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
// 	wxPanel *panel = new wxPanel(this, -1);
	wxBoxSizer *box = new wxBoxSizer(wxVERTICAL);
	SetSizer(box);
	
	// Initialize the GUI
	switch (settings.GetGUIMode())
	{
	case GUI_Simple:
		InitBasicGUI(box);
		break;
	case GUI_Default:
		InitAdvancedGUI(box);
		break;
	}
	
	// Load instance icons
	InstIconList * instIcons = InstIconList::Instance();
	switch (GetGUIMode())
	{
	case GUI_Simple:
		instListCtrl->AssignImageList(instIcons->CreateImageList(), wxIMAGE_LIST_NORMAL);
		break;
		
	case GUI_Default:
		instListbook->AssignImageList(instIcons->CreateImageList());
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
	if (settings.GetJavaPath() == _("java"))
	{
		settings.SetJavaPath(FindJavaPath());
	}

	if (settings.GetAutoUpdate())
	{
		CheckUpdateTask *task = new CheckUpdateTask();
		StartTask(*task);
	}
}

GUIMode MainWindow::GetGUIMode() const
{
	if (instListCtrl == nullptr)
		return GUI_Default;
	else
		return GUI_Simple;
}

void MainWindow::InitBasicGUI(wxBoxSizer *mainSz)
{
	instListCtrl = new wxListCtrl(this, ID_InstListCtrl, wxDefaultPosition, wxDefaultSize,
		wxLC_SINGLE_SEL | wxLC_ICON | wxLC_ALIGN_LEFT);
	
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
	instNotesEditor = nullptr;
	
	
	mainSz->Add(instListCtrl, 1, wxEXPAND);
}

void MainWindow::InitAdvancedGUI(wxBoxSizer *mainSz)
{
	instListbook = new wxListbook(this, ID_InstListCtrl, 
		wxDefaultPosition, wxDefaultSize, wxLB_LEFT);
	instListbook->GetListView()->SetMinSize(wxSize(80, -1));
	
	mainSz->Add(instListbook, 1, wxEXPAND);
	
	instPanel = new wxPanel(instListbook);
	wxGridBagSizer *instSz = new wxGridBagSizer();
	instPanel->SetSizer(instSz);
	
	instSz->AddGrowableCol(1, 0);
	instSz->AddGrowableRow(1, 0);
	
	const int cols = 4;
	const int rows = 3;
	
	wxFont titleFont(18, wxSWISS, wxNORMAL, wxNORMAL);
	wxFont nameEditFont(14, wxSWISS, wxNORMAL, wxNORMAL);
	
	instNameSz = new wxBoxSizer(wxVERTICAL);
	instSz->Add(instNameSz, wxGBPosition(0, 0), wxGBSpan(1, cols - 1), 
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
	instSz->Add(instNotesEditor, wxGBPosition(1, 0), wxGBSpan(rows - 2, cols - 1), 
		wxEXPAND | wxALL, 4);
	
	
	wxPanel *btnPanel = new wxPanel(instPanel);
	wxBoxSizer *btnSz = new wxBoxSizer(wxVERTICAL);
	instSz->Add(btnPanel, wxGBPosition(1, cols - 1), wxGBSpan(rows - 2, 1),
		wxALIGN_RIGHT | wxLEFT | wxRIGHT, 8);
	btnPanel->SetSizer(btnSz);
	
	const int spacerSize = 4;

	btnPlay = new wxButton(btnPanel, ID_Play, _("&Play"));
	btnSz->Add(btnPlay, wxSizerFlags(0).Border(wxTOP | wxBOTTOM, 4).Expand());
	btnSz->AddSpacer(spacerSize);
	btnRename = new wxButton(btnPanel, ID_Rename, _("&Rename"));
	btnSz->Add(btnRename, wxSizerFlags(0).Border(wxTOP | wxBOTTOM, 4).Expand());
	btnChangeIcon = new wxButton(btnPanel, ID_ChangeIcon, _("Change &Icon"));
	btnSz->Add(btnChangeIcon, wxSizerFlags(0).Border(wxTOP | wxBOTTOM, 4).Expand());
	btnCopyInst = new wxButton(btnPanel, ID_CopyInst, _("Copy Instance"));
	btnSz->Add(btnCopyInst, wxSizerFlags(0).Border(wxTOP | wxBOTTOM, 4).Expand());
	btnSz->AddSpacer(spacerSize);
	btnEditMods = new wxButton(btnPanel, ID_EditMods, _("Edit &Mods"));
	btnSz->Add(btnEditMods, wxSizerFlags(0).Border(wxTOP | wxBOTTOM, 4).Expand());
	btnDowngrade = new wxButton(btnPanel, ID_DowngradeInst, _("Downgrade"));
	btnSz->Add(btnDowngrade, wxSizerFlags(0).Border(wxTOP | wxBOTTOM, 4).Expand());
	btnRebuildJar = new wxButton(btnPanel, ID_RebuildJar, _("Re&build Jar"));
	btnSz->Add(btnRebuildJar, wxSizerFlags(0).Border(wxTOP | wxBOTTOM, 4).Expand());
	btnViewFolder = new wxButton(btnPanel, ID_ViewInstFolder, _("&View Folder"));
	btnSz->Add(btnViewFolder, wxSizerFlags(0).Border(wxTOP | wxBOTTOM, 4).Expand());
	
	UpdateNotesBox();
	CancelRename();
}

void MainWindow::UpdateInstPanel()
{
	bool showInstPanel = instListbook->GetPageCount() > 0;
	instPanel->Show(showInstPanel);
	if (showInstPanel)
	{
		Instance *inst = GetSelectedInst();

		UpdateInstNameLabel(inst);
		UpdateNotesBox();
		CancelRename();

		instPanel->Layout();
	}
}

void MainWindow::UpdateInstNameLabel(Instance *inst)
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

#if wxCHECK_VERSION(2, 9, 0)
void MainWindow::OnPageChanged(wxBookCtrlEvent &event)
#else
void MainWindow::OnPageChanged(wxListbookEvent &event)
#endif
{
	if (GetLinkedInst(event.GetOldSelection()) != nullptr)
	{
		SaveNotesBox(GetLinkedInst(event.GetOldSelection()));
	}
	UpdateInstPanel();
}

void MainWindow::LoadInstanceList(wxFileName instDir)
{
	GetStatusBar()->PushStatusText(_("Loading instances..."), 0);
	
	switch (GetGUIMode())
	{
	case GUI_Simple:
		instListCtrl->ClearAll();
		break;
		
	case GUI_Default:
		for (int i = instListbook->GetPageCount() - 1; i >= 0; i--)
		{
			instListbook->RemovePage(i);
		}
		break;
	}
	
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
		
		GetStatusBar()->SetStatusText(
			wxString::Format(_("Loaded %i instances..."), ctr), 0);
		wxGetApp().Yield();
		cont = dir.GetNext(&subFolder);
	}
	Enable(true);
	
	if (GetGUIMode() == GUI_Default)
	{
		instListbook->SetSelection(0);
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
	
	int item;
	InstIconList * instIcons = InstIconList::Instance();
	switch (GetGUIMode())
	{
	case GUI_Simple:
		item = instListCtrl->InsertItem(instListCtrl->GetItemCount(), 
			instName, instIcons->getIndexForKey(inst->GetIconKey()));
		instItems[item] = inst;
		break;
		
	case GUI_Default:
		item = instListbook->GetPageCount();
		instItems[item] = inst;
		instListbook->InsertPage(item, instPanel, inst->GetName(), true, instIcons->getIndexForKey(inst->GetIconKey()));
		break;
	}
}

Instance* MainWindow::GetLinkedInst(int id)
{
	return instItems[id];
}

Instance* MainWindow::GetSelectedInst()
{
	switch (GetGUIMode())
	{
	case GUI_Simple:
	{
		long item = -1;
		while (true)
		{
			item = instListCtrl->GetNextItem(item, wxLIST_NEXT_ALL, 
				wxLIST_STATE_SELECTED);
			
			if (item == -1)
				break;
			
			return GetLinkedInst(item);
		}
		return nullptr;
	}
	
	case GUI_Default:
		if (instListbook->GetPageCount() <= 0)
			return nullptr;
		return GetLinkedInst(instListbook->GetSelection());

	default:
		return nullptr;
	}
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
	while (wxDirExists(Path::Combine(settings.GetInstDir(), dirName)))
	{
		num++;
		dirName = Utils::RemoveInvalidPathChars(newInstName) + wxString::Format(_("_%i"), num);

		// If it's over 9000
		if (num > 9000)
		{
			wxLogError(_T("Couldn't create instance folder: %s"),
				Path::Combine(settings.GetInstDir(), dirName).c_str());
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

	wxFileName instDir = wxFileName::DirName(Path::Combine(settings.GetInstDir(), instDirName));
	
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
	Utils::OpenFile(settings.GetInstDir());
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

	if (event.m_latestBuildNumber > AppVersion.GetBuild() || 
		settings.GetUseDevBuilds() != AppVersion.IsDevBuild())
	{
		if (wxMessageBox(wxString::Format(_("%s build #%i is available. Would you like to download and install it?"), 
				(settings.GetUseDevBuilds() ? _("Dev") : _("Stable")), event.m_latestBuildNumber), 
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
	info.SetVersion(AppVersion.ToString());
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
	wxMessageBox(wxString::Format(_("The about dialog is currently not supported in Windows.\nYou are using MultiMC version %s."), AppVersion.ToString().c_str()));
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

void MainWindow::OnInstActivated(wxListEvent &event)
{
	ShowLoginDlg(_(""));
}

void MainWindow::ShowLoginDlg(wxString errorMsg)
{
	Instance *selected = GetSelectedInst();
	if(!selected)
	{
		// FIXME: what if the instance somehow becomes deselected while playing? is that possible?
		//        what does it mean to the state of the GUI
		return;
	}
	UserInfo lastLogin;
	if (wxFileExists(_("lastlogin")))
	{
		lastLogin.LoadFromFile("lastlogin");
	}
	
	LoginDialog loginDialog(this, errorMsg, lastLogin);
	int response = loginDialog.ShowModal();
	
	bool playOffline = response == ID_PLAY_OFFLINE;
	if (response == wxID_OK || playOffline)
	{
		UserInfo info(loginDialog);
		
		info.SaveToFile("lastlogin");
		
		if (!playOffline)
		{
			LoginTask *task = new LoginTask(info,selected , loginDialog.ShouldForceUpdate());
			StartModalTask(*task, true);
		}
		else
		{
			LoginCompleteEvent event(nullptr, LoginResult::PlayOffline(info.username), selected);
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


void MainWindow::OnRenameClicked(wxCommandEvent& event)
{
	switch (GetGUIMode())
	{
	case GUI_Default:
		StartRename();
		break;
		
	case GUI_Simple:
	{
		Instance *inst = GetSelectedInst();
		if(!inst)
			break;
		wxTextEntryDialog textDlg(this, _("Enter a new name for this instance."), 
			_("Rename Instance"), inst->GetName());
		if (textDlg.ShowModal() == wxID_OK)
		{
			inst->SetName(textDlg.GetValue());
			LoadInstanceList();
		}
		break;
	}
	}
}

void MainWindow::OnChangeIconClicked(wxCommandEvent& event)
{
	ChangeIconDialog iconDlg(this);
	if (iconDlg.ShowModal() == wxID_OK)
	{
		Instance *inst = GetSelectedInst();
		if(!inst)
			return;
		inst->SetIconKey(iconDlg.GetSelectedIconKey());
		LoadInstanceList();
	}
}

void MainWindow::OnCopyInstClicked(wxCommandEvent &event)
{
	Instance *srcInst = GetSelectedInst();
	if(!srcInst)
		return;

	wxString instName;
	wxString instDirName;
	if (!GetNewInstName(&instName, &instDirName, _("Copy existing instance")))
		return;

	instDirName = Path::Combine(settings.GetInstDir(), instDirName);

	wxMkdir(instDirName);
	FileCopyTask task(srcInst->GetRootDir().GetFullPath(), wxFileName::DirName(instDirName));
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
		Instance *inst = GetSelectedInst();
		if(!inst)
			return;
		wxTextEntryDialog textDlg(this, _("Instance notes"), _("Notes"), inst->GetNotes(), 
			wxOK | wxCANCEL | wxTE_MULTILINE);
		textDlg.SetSize(600, 400);
		if (textDlg.ShowModal() == wxID_OK)
		{
			inst->SetNotes(textDlg.GetValue());
			LoadInstanceList();
		}
		break;
	}
	}
}

void MainWindow::SaveNotesBox(Instance *inst)
{
	if (inst != nullptr)
	{
		wxString notes = instNotesEditor->GetValue();
		inst->SetNotes(notes);
		instNotesEditor->SetValue(inst->GetNotes());
	}
}

void MainWindow::UpdateNotesBox()
{
	if (GetSelectedInst() != nullptr)
		instNotesEditor->SetValue(GetSelectedInst()->GetNotes());
}


void MainWindow::StartRename()
{
	Instance * selected = GetSelectedInst();
	if(!selected)
		return;
	DisableInstActions();
	renamingInst = true;
	
	GetStatusBar()->PushStatusText(wxString::Format(
		_("Renaming instance '%s'... (Press enter to finish)"), 
		selected->GetName().c_str()), 0);
	
	instNameEditor->SetValue(selected->GetName());
	
	instNameLabel->Show(false);
	instNameSz->Hide(instNameLabel);
	
	instNameEditor->Show(true);
	instNameSz->Show(instNameEditor);
	instNameEditor->SetFocus();
	
	instNameSz->Layout();
}

void MainWindow::FinishRename()
{
	Instance * selected = GetSelectedInst();
	if(selected)
	{
		if (!instNameEditor->IsEmpty())
			GetSelectedInst()->SetName(instNameEditor->GetValue());
	}
	CancelRename();
	LoadInstanceList();
}

void MainWindow::CancelRename()
{
	EnableInstActions();
	
	if (renamingInst)
	{
		renamingInst = false;
		GetStatusBar()->PopStatusText(0);
	}
	
	instNameEditor->Show(false);
	instNameSz->Hide(instNameEditor);
	
	instNameLabel->Show(true);
	instNameSz->Show(instNameLabel);
	
	instNameSz->Layout();
	
	if (GetSelectedInst() != nullptr)
		UpdateInstNameLabel(GetSelectedInst());
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
	Instance *selected = GetSelectedInst();
	if(selected == nullptr)
		return;
	ModEditWindow *editDlg = new ModEditWindow(this, selected);
	editDlg->Show();
}

void MainWindow::OnDowngradeInstClicked(wxCommandEvent& event)
{
	Instance *selected = GetSelectedInst();
	if(selected == nullptr)
		return;

	if (selected->GetVersionFile().FileExists())
	{
		DowngradeDialog *downDlg = new DowngradeDialog(this);
		if (downDlg->ShowModal() == wxID_OK && !downDlg->GetSelectedVersion().IsEmpty())
		{
			DowngradeTask dgTask(selected, downDlg->GetSelectedVersion());
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
	Instance *selected = GetSelectedInst();
	if(selected == nullptr)
		return;
	ModderTask *modTask = new ModderTask(selected);
	StartModalTask(*modTask);
}

void MainWindow::OnViewInstFolderClicked(wxCommandEvent& event)
{
	Instance *selected = GetSelectedInst();
	if(selected == nullptr)
		return;
	Utils::OpenFile(selected->GetRootDir());
}

void MainWindow::OnDeleteClicked(wxCommandEvent& event)
{
	NotImplemented();
}

void MainWindow::OnInstMenuOpened(wxListEvent& event)
{
	if (instActionsEnabled)
		PopupMenu(instMenu, event.GetPoint());
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
		SaveNotesBox(GetSelectedInst());
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
	
	
	EVT_LIST_ITEM_ACTIVATED(ID_InstListCtrl, MainWindow::OnInstActivated)
	EVT_LIST_ITEM_RIGHT_CLICK(ID_InstListCtrl, MainWindow::OnInstMenuOpened)
	
	EVT_LISTBOOK_PAGE_CHANGED(ID_InstListCtrl, MainWindow::OnPageChanged)
	
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