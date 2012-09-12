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
#include "aboutdlg.h"

#include <wx/filesys.h>
#include <wx/dir.h>
#include <wx/fs_arc.h>
#include <wx/fs_mem.h>
#ifdef __WXGTK__
#include <wx/aboutdlg.h>
#endif

#include <wx/cmdline.h>
#include <wx/stdpaths.h>
#include <wx/gbsizer.h>
#include <wx/filedlg.h>

#include <wx/utils.h>
#include <wx/toolbar.h>
#include <wx/tbarbase.h>
#include <wx/image.h>

#if (defined __WXMSW__ || defined __WXGTK__) && wxCHECK_VERSION(2, 9, 4) 
#define USE_DROPDOWN_MENU
#endif

#include "config.h"
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
	renamingInst = false;
	instActionsEnabled = true;
	instMenu = nullptr;
	instListCtrl = nullptr;

	instPanel = nullptr;
	
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
	
	wxToolBar *mainToolBar = CreateToolBar(/*wxTB_HORIZONTAL| wxTB_NO_TOOLTIPS*/);
	
	
	// Load toolbar icons
	wxBitmap newInstIcon = wxMEMORY_IMAGE(newinsticon);
	wxBitmap importCPIcon = wxMEMORY_IMAGE(importcpicon);
	wxBitmap reloadIcon = wxMEMORY_IMAGE(refreshinsticon);
	wxBitmap viewFolderIcon = wxMEMORY_IMAGE(viewfoldericon);
	wxBitmap viewCMFolderIcon = wxMEMORY_IMAGE(centralmodsfolder);
	wxBitmap settingsIcon = wxMEMORY_IMAGE(settingsicon);
	wxBitmap checkUpdateIcon = wxMEMORY_IMAGE(checkupdateicon);
	wxBitmap helpIcon = wxMEMORY_IMAGE(helpicon);
	wxBitmap aboutIcon = wxMEMORY_IMAGE(abouticon);
	wxBitmap bugIcon = wxMEMORY_IMAGE(reportbug);

	
	
	// Build the toolbar
	#ifdef USE_DROPDOWN_MENU
	{
		wxMenu *addInstMenu = new wxMenu();
		addInstMenu->Append(ID_NewInst, _("Add a new instance."));
		addInstMenu->Append(ID_CopyInst, _("Copy selected instance."));
		addInstMenu->Append(ID_ImportInst, _("Import existing .minecraft folder"));
		addInstMenu->Append(ID_ImportCP, _("Import config pack"));

		auto tool = mainToolBar->AddTool(ID_AddInst, _("Add instance"), newInstIcon, _("Add a new instance."), wxITEM_DROPDOWN);
		tool->SetDropdownMenu(newInstanceMenu);
	}
	#else
	{
		mainToolBar->AddTool(ID_AddInst, _("Add"),
			newInstIcon, wxNullBitmap, wxITEM_NORMAL,
			_("Add a new instance."), _("Add a new Minecraft instance."));
		
		mainToolBar->AddSeparator();
	}
	#endif
	mainToolBar->AddTool(ID_Refresh, _("Refresh"),
		reloadIcon, wxNullBitmap, wxITEM_NORMAL,
		_("Reload ALL the instances!"),_("Reload ALL the instances!"));
	
	mainToolBar->AddTool(ID_ViewFolder, _("View folder"),
		viewFolderIcon, wxNullBitmap, wxITEM_NORMAL,
		_("Open the instance folder."), _("Open the instance folder."));
	
	mainToolBar->AddTool(ID_ViewCMFolder, _("View Central mods folder"),
		viewCMFolderIcon, wxNullBitmap, wxITEM_NORMAL,
		_("Open the central mods folder."),_("Open the central mods folder."));
	
	mainToolBar->AddSeparator();
	
	mainToolBar->AddTool(ID_Settings, _("Settings"),
		settingsIcon, wxNullBitmap, wxITEM_NORMAL,
		_("Settings"), _("Change MultiMC or Minecraft settings."));
	
	mainToolBar->AddTool(ID_CheckUpdate, _("Check for updates"),
		checkUpdateIcon, wxNullBitmap, wxITEM_NORMAL,
		_("Check for MultiMC updates."), _("Check for MultiMC updates."));
	
	mainToolBar->AddSeparator();

	mainToolBar->AddTool(ID_BugReport, _("Report bug"),
		bugIcon, wxNullBitmap, wxITEM_NORMAL,
		_("Report bug"), _("Report bug"));
	mainToolBar->AddTool(ID_Help, _("Help"),
		helpIcon, wxNullBitmap, wxITEM_NORMAL,
		_("Help"),_("Help"));
	// interestingly, calling tool->Enable(false) won't disable it reliably. This works:
	mainToolBar->EnableTool(ID_Help,false);
	
	mainToolBar->AddTool(ID_About, _("About"),
		aboutIcon, wxNullBitmap, wxITEM_NORMAL,
		_("About MultiMC"), _("About MultiMC"));
	
	mainToolBar->Realize();
	
	// Create the status bar
	auto sbar = CreateStatusBar(1);
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

	launchInstance = _("");

	CenterOnScreen();
}

MainWindow::~MainWindow(void)
{
	
}

void MainWindow::OnStartup()
{
	LoadInstanceList();

	// Automatically auto-detect the Java path.
	if (settings->GetJavaPath() == _("java"))
	{
		settings->SetJavaPath(FindJavaPath());
	}

	if (settings->GetAutoUpdate())
	{
		CheckUpdateTask *task = new CheckUpdateTask();
		StartTask(task,TASK_BACKGROUND);
	}

	if(!launchInstance.empty())
	{
		wxFileName instanceDir = settings->GetInstDir();
		instanceDir.AppendDir(launchInstance);
		m_currentInstance = Instance::LoadInstance(instanceDir);

		if(m_currentInstance == nullptr)
		{
			wxString output = _("Couldn't find the instance you tried to load: ");
			output.append(launchInstance);
			output.append(_(". Make sure it exists!"));
			wxLogError(output);
		}
		else
		{
			ShowLoginDlg(_(""));
		}
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
	#ifdef __WXMSW__
	int borderstyle = wxWindow::GetThemedBorderStyle();
	#else
	int borderstyle = wxBORDER_SUNKEN;
	#endif
	instListCtrl = new wxInstanceCtrl(instPanel, ID_InstListCtrl,wxDefaultPosition,wxDefaultSize,wxINST_SINGLE_COLUMN|borderstyle);
	instListCtrl->SetImageSize(wxSize(32,32));
	instSz->Add(instListCtrl,wxGBPosition(0, 0), wxGBSpan(rows, 1),wxEXPAND/* | wxALL, 4*/);
	
	instNameSz = new wxBoxSizer(wxVERTICAL);
	instSz->Add(instNameSz, wxGBPosition(0, 1), wxGBSpan(1, cols - 2), wxEXPAND | wxALL, 4);
	
	instNameEditor = new wxTextCtrl(instPanel, ID_InstNameEditor, wxEmptyString,
		wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
	instNameEditor->SetMaxLength(instNameLengthLimit);
	instNameEditor->SetFont(nameEditFont);
	instNameSz->Add(instNameEditor, wxSizerFlags(0).Align(wxALIGN_CENTER).Expand());
	
	instNameLabel = new wxStaticText(instPanel, -1, _("InstName"), 
		wxDefaultPosition, wxDefaultSize);
	instNameLabel->SetFont(titleFont);
	instNameSz->Add(instNameLabel, wxSizerFlags(0).Align(wxALIGN_CENTER));
	
	
	instNotesEditor = new wxTextCtrl(instPanel, -1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_RICH);
	instSz->Add(instNotesEditor, wxGBPosition(1, 1), wxGBSpan(rows - 2, cols - 2), wxEXPAND | wxLEFT | wxTOP | wxRIGHT, 4);
	
	
	wxPanel *btnPanel = new wxPanel(instPanel);
	wxBoxSizer *btnSz = new wxBoxSizer(wxVERTICAL);
	instSz->Add(btnPanel, wxGBPosition(1, cols - 1), wxGBSpan(rows - 2, 1), wxALIGN_RIGHT | wxLEFT | wxRIGHT, 8);
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

	if (!instDir.DirExists())
	{
		if (!instDir.Mkdir())
		{
			wxLogError(_T("Failed to create instance directory."));
			return;
		}
	}
	
	instListCtrl->Clear();
	for(int i = 0; i < instItems.size(); i++)
	{
		delete instItems[i];
	}
	instItems.clear();
	m_currentInstance = nullptr;
	m_currentInstanceIdx = -1;
	
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
	
	//GetStatusBar()->PopStatusText(0);
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
#ifdef USE_DROPDOWN_MENU
	OnNewInstance(event);
#else
	wxMenu *addInstMenu = new wxMenu();
	addInstMenu->Append(ID_NewInst, _("Add a new instance."));
	addInstMenu->Append(ID_CopyInst, _("Copy selected instance."));
	addInstMenu->Append(ID_ImportInst, _("Import existing .minecraft folder"));
	addInstMenu->Append(ID_ImportCP, _("Import config pack"));
	PopupMenu(addInstMenu);
	wxDELETE(addInstMenu);
#endif
}

void MainWindow::OnNewInstance(wxCommandEvent& event)
{
	wxString instName;
	wxString instDirName;
	if (!GetNewInstName(&instName, &instDirName))
		return;

	wxFileName instDir = wxFileName::DirName(Path::Combine(settings->GetInstDir(), instDirName));

	Instance *inst = new Instance(instDir);
	UserInfo lastLogin;
	if (wxFileExists(_("lastlogin4")))
	{
		lastLogin.LoadFromFile("lastlogin4");
		if(lastLogin.username.Lower().Contains(_("direwolf")))
		{
			inst->SetIconKey(_("enderman"));
		}
	}
	inst->SetName(instName);
	AddInstance(inst);
}

void MainWindow::OnImportMCFolder(wxCommandEvent& event)
{
	wxDirDialog *dirDlg = new wxDirDialog(this, _("Select a Minecraft folder to import"));
	if (dirDlg->ShowModal() != wxID_OK)
		return;

	wxString existingMCDir = dirDlg->GetPath();

	wxString instName;
	wxString instDirName;
	if (!GetNewInstName(&instName, &instDirName, _("Import existing Minecraft folder")))
		return;

	instDirName = Path::Combine(settings->GetInstDir(), Utils::RemoveInvalidPathChars(instDirName));

	wxMkdir(instDirName);

	Instance *inst = new Instance(instDirName);
	inst->SetName(instName);
	StartTask(new FileCopyTask(existingMCDir, inst->GetMCDir()));
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
		else
		{
			wxLogError(_("This is not a valid config pack!"));
		}
	}
}

void MainWindow::OnViewFolderClicked(wxCommandEvent& event)
{
	if (!settings->GetInstDir().DirExists())
		settings->GetInstDir().Mkdir();

	Utils::OpenFolder(settings->GetInstDir());
}

void MainWindow::OnViewCMFolderClicked(wxCommandEvent& event)
{
	if (!settings->GetModsDir().DirExists())
		settings->GetModsDir().Mkdir();

	Utils::OpenFolder(settings->GetModsDir());
}

void MainWindow::OnRefreshClicked(wxCommandEvent& event)
{
	LoadInstanceList();
}

void MainWindow::OnSettingsClicked(wxCommandEvent& event)
{
	SettingsDialog *settingsDlg = new SettingsDialog(this, -1);
	auto oldInstDir = settings->GetInstDir();
	int response = settingsDlg->ShowModal();
	if (response == wxID_OK)
	{
		// if the instance folder changed, reload the instance list
		if(!oldInstDir.SameAs(settings->GetInstDir()))
		{
			LoadInstanceList();
		}
		
		if (settingsDlg->GetForceUpdateMultiMC())
		{
			wxString ciURL(_T(JENKINS_JOB_URL));

			wxString dlFileName;
			if (IS_WINDOWS())
				dlFileName = _("MultiMC.exe");
			else if (IS_LINUX() || IS_MAC())
				dlFileName = _("MultiMC");

			wxString dlURL = wxString::Format(
				_("%s/lastStableBuild/artifact/%s"),
				ciURL.c_str(), dlFileName.c_str());
			DownloadInstallUpdates(dlURL);
		}
	}
}

void MainWindow::OnCheckUpdateClicked(wxCommandEvent& event)
{
	CheckUpdateTask *task = new CheckUpdateTask();
	StartTask(task);
}

void MainWindow::OnCheckUpdateComplete(CheckUpdateEvent &event)
{
	if (event.m_latestBuildNumber > AppVersion.GetBuild())
	{
		if (wxMessageBox(wxString::Format(_("Build #%i is available. Would you like to download and install it?"), 
				event.m_latestBuildNumber), 
				_("Update Available"), wxYES_NO) == wxYES)
		{
			DownloadInstallUpdates(event.m_downloadURL);
		}
	}
}

void MainWindow::DownloadInstallUpdates(const wxString &downloadURL)
{
#ifdef __WXMSW__
	wxString updaterFileName = _("MultiMCUpdate.exe");
#else
	wxString updaterFileName = _("MultiMCUpdate");
#endif

	auto dlTask = new FileDownloadTask(downloadURL, wxFileName(updaterFileName), _("Downloading updates..."));
	wxGetApp().updateOnExit = true;
	StartTask(dlTask, TASK_MODAL, false, wxSize(400, 120));

	/*
	// Give the task dialogs some time to close.
	for (int i = 0; i < 100; i++)
	{
		wxSafeYield();
	}
	*/

	Close(false);
}

void MainWindow::OnHelpClicked(wxCommandEvent& event)
{
	NotImplemented();
}

void MainWindow::OnAboutClicked(wxCommandEvent& event)
{
#ifdef __WXGTK__
	wxAboutDialogInfo info;
	info.SetName(_("MultiMC"));
	info.SetVersion(wxString::Format(_("%s - %s"), AppVersion.ToString().c_str(), AppBuildTag.ToString().c_str()));
	info.SetDescription(_("MultiMC is a custom launcher that makes managing Minecraft easier by allowing you to have multiple installations of Minecraft at once."));
	info.SetCopyright(_("(C) 2012 Andrew Okin"));
	
	info.SetWebSite(_("http://forkk.net/MultiMC4"));
	info.SetLicense(licenseText);
	
	info.SetIcon(wxGetApp().GetAppIcons().GetIcon(wxSize(64, 64)));

	info.AddDeveloper(_("Andrew Okin <forkk@forkk.net>"));
	info.AddDeveloper(_("Petr Mrázek <peterix@gmail.com>"));
	
	wxAboutBox(info);
#else
	AboutDlgInfo info;

	info.name = _("MultiMC");
	info.version = wxString::Format(_("%s - %s"), AppVersion.ToString().c_str(), AppBuildTag.ToString().c_str());
	info.description = _("MultiMC is a custom launcher that makes managing Minecraft easier by allowing you to have multiple installations of Minecraft at once.");
	info.copyright = _("(C) 2012 Andrew Okin");

	info.website = _("http://forkk.net/MultiMC4");
	info.license = licenseText;

	info.icon = wxGetApp().GetAppIcons().GetIcon(wxSize(64, 64));

	AboutDlg aboutDlg(this, info);
	aboutDlg.ShowModal();
#endif
}

void MainWindow::OnBugReportClicked ( wxCommandEvent& event )
{
	if(!Utils::OpenURL(_("http://bugs.forkk.net/")))
	{
		wxMessageBox(_T("MultiMC was unable to run your web browser.\n\nTo report bugs, visit:\nhttp://bugs.forkk.net/"), 
		_T("Error"), wxOK | wxCENTER | wxICON_ERROR, this);
	}
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
	bool canPlayOffline = m_currentInstance->HasBinaries();
	LoginDialog loginDialog(this, errorMsg, lastLogin, canPlayOffline);
	int response = loginDialog.ShowModal();
	
	bool playOffline = response == ID_PLAY_OFFLINE;
	if (response == wxID_OK || playOffline)
	{
		UserInfo info(loginDialog);
		
		info.SaveToFile("lastlogin4");
		
		if (!playOffline)
		{
			LoginTask *task = new LoginTask(info,m_currentInstance , loginDialog.ShouldForceUpdate());
			StartTask(task);
		}
		else
		{
			LoginCompleteEvent event(nullptr, LoginResult::PlayOffline(info.username), m_currentInstance);
			OnLoginComplete(event);
		}
	}
	else if (!launchInstance.IsEmpty() && response == wxID_CANCEL)
	{
		this->Destroy();
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
		if (!result.playOffline && !result.sessionID.Trim().IsEmpty() && result.sessionID != _("Offline"))
		{
			StartTask(new GameUpdateTask(inst, result.latestVersion, _("minecraft.jar"), event.m_forceUpdate));
		}
		
		if (inst->ShouldRebuild())
		{
			StartTask(new ModderTask (inst));
		}
		
		if(inst->Launch(result.username, result.sessionID, true))
		{
			Show(false);
			InstConsoleWindow *cwin = new InstConsoleWindow(inst, this, 
				!launchInstance.IsEmpty());
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

	instDirName = Path::Combine(settings->GetInstDir(), Utils::RemoveInvalidPathChars(instDirName));

	wxMkdir(instDirName);
	StartTask(new FileCopyTask (m_currentInstance->GetRootDir().GetFullPath(), wxFileName::DirName(instDirName)));

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
			if (downDlg->GetSelectedVersion().Contains(wxT("indev")) ||
				downDlg->GetSelectedVersion().Contains(wxT("infdev")))
			{
				if (wxMessageBox(_("MultiMC is currently incompatible with \
indev and infdev. Are you sure you would like to downgrade to this version?"), 
						_("Continue?"), wxYES_NO) == wxNO)
				{
					return;
				}
			}

			StartTask(new DowngradeTask (m_currentInstance, downDlg->GetSelectedVersion()));
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
	StartTask(new ModderTask(m_currentInstance));
}

void MainWindow::OnViewInstFolderClicked(wxCommandEvent& event)
{
	if(m_currentInstance == nullptr)
		return;
	Utils::OpenFolder(m_currentInstance->GetRootDir());
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
		fsutils::RecursiveDelete(m_currentInstance->GetRootDir().GetFullPath());
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
	/*
	Task * t = event.m_task;
	if(!t->isModal())
	{
		t->Wait();
		delete t;
	}*/
}

void MainWindow::OnTaskProgress(TaskProgressEvent& event)
{
	
}

void MainWindow::OnTaskError(TaskErrorEvent& event)
{
	wxLogError(event.m_errorMsg);
}

void MainWindow::StartTask ( Task* task, MainWindow::task_type type, bool shouldFit, wxSize size )
{
	if(type == TASK_BACKGROUND)
	{
		task->Start(this,false);
		// FIXME: there is only one such task. it is leaked. Sadly, the event system sucks and it's using it
	}
	else
	{
		int style = wxPD_APP_MODAL;
		
		wxProgressDialog *progDialog = new wxProgressDialog(_("Please wait..."), task->GetStatus(), 100, this, style);
		if(shouldFit)
		{
			progDialog->SetMinSize(size);
			progDialog->Fit();
		}
		else
		{
			progDialog->SetSize(size);
		}

		progDialog->Update(0);
		progDialog->CenterOnParent();
		task->Start(this,true);
		
		progDialog->Show();
		while (!task->hasEnded())
		{
			bool retVal = true;
			wxString status = task->GetStatus();
			int progress = task->GetProgress();
			if (progress == 0)
			{
				retVal = progDialog->Pulse(status);
			}
			//HACK: the dialog gets stuck when set to max. no idea why
			else if (progress == 100)
			{
				progDialog->Update(99, status);
			}
			else
			{
				progDialog->Update(progress, status);
			}
			if(shouldFit) progDialog->Fit();
			wxYield();
			wxMilliSleep(100);
		}
		task->Wait();
		delete task;
		progDialog->Destroy();
	}
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
	StartTask(new ExportPackTask (inst, packName, packNotes, filename, includedConfigs));
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
	EVT_TOOL(ID_ViewCMFolder, MainWindow::OnViewCMFolderClicked)
	EVT_TOOL(ID_Refresh, MainWindow::OnRefreshClicked)

	EVT_TOOL(ID_Settings, MainWindow::OnSettingsClicked)
	EVT_TOOL(ID_CheckUpdate, MainWindow::OnCheckUpdateClicked)

	EVT_TOOL(ID_Help, MainWindow::OnHelpClicked)
	EVT_TOOL(ID_About, MainWindow::OnAboutClicked)
	EVT_TOOL(ID_BugReport, MainWindow::OnBugReportClicked)

	EVT_MENU(ID_NewInst, MainWindow::OnNewInstance)
	EVT_MENU(ID_CopyInst, MainWindow::OnCopyInstClicked)
	EVT_MENU(ID_ImportInst, MainWindow::OnImportMCFolder)
	EVT_MENU(ID_ImportCP, MainWindow::OnImportCPClicked)

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
	EVT_TASK_PROGRESS(MainWindow::OnTaskProgress)
	EVT_TASK_ERRORMSG(MainWindow::OnTaskError)
	
	EVT_LOGIN_COMPLETE(MainWindow::OnLoginComplete)
	EVT_CHECK_UPDATE(MainWindow::OnCheckUpdateComplete)
	
	EVT_TEXT_ENTER(ID_InstNameEditor, MainWindow::OnRenameEnterPressed)

	EVT_CLOSE(MainWindow::OnWindowClosed)
END_EVENT_TABLE()
