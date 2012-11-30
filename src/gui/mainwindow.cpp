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
#include "utils/fsutils.h"
#include "aboutdlg.h"
#include "updatepromptdlg.h"
#include "taskprogressdialog.h"
#include "snapshotdialog.h"
#include "lwjgldialog.h"
#include "savemgrwindow.h"
#include "stdinstance.h"
#include <mcversionlist.h>
#include <mcprocess.h>
#include "lwjglinstalltask.h"
#include "ftbselectdialog.h"

#include "instancectrl.h"

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

#ifdef wx29
#include <wx/persist/toplevel.h>
#endif

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
	m_guiState = STATE_IDLE;
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
		addInstMenu->Append(ID_ImportFTB, _("Import from FTB launcher."));

		auto tool = mainToolBar->AddTool(ID_AddInst, _("Add instance"), newInstIcon, _("Add a new instance."), wxITEM_DROPDOWN);
		tool->SetDropdownMenu(addInstMenu);
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
	sbar->SetFieldsCount(2);
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
	case GUI_Fancy:
		InitAdvancedGUI(box);
		break;
	}

	launchInstance = wxEmptyString;

	// This breaks the windows version and is pretty much alien on linux.
	// AWAY WITH YOU, FOUL CODE, INTO THE LANDS OF IFDEF AND SPINNING BEACHBALLS!
#ifndef __WXMSW__
#ifndef __WXGTK__
	// Keyboard accelerators.
	wxAcceleratorEntry entries[1];
	entries[0].Set(wxACCEL_CTRL,	(int) 'Q',	wxID_EXIT);
	wxAcceleratorTable accel(sizeof(entries), entries);
	SetAcceleratorTable(accel);
#endif
#endif

	// Is it November 30th?
	const wxDateTime originalReleaseDate(30, wxDateTime::Month::Nov, 2011, 17, 54);
	if (wxDateTime::Now().GetDay() == originalReleaseDate.GetDay() &&
		wxDateTime::Now().GetMonth() == originalReleaseDate.GetMonth())
	{
		// Calculate how many years old MultiMC is.
		int yearsOld = wxDateTime::Now().GetYear() - originalReleaseDate.GetYear();

		wxString yearStr = (yearsOld == 1 ? _("year") : _("years"));
		wxString titleMsg = wxString::Format(_("Happy Birthday, MultiMC! %i %s old!"),
			yearsOld, yearStr.c_str());
		SetTitle(titleMsg);
	}
	
	CenterOnScreen();
}

MainWindow::~MainWindow(void)
{
	
}

void MainWindow::OnStartup()
{
	LoadInstanceList();

	// Automatically auto-detect the Java path.
	if (settings->GetJavaPath() == "java")
	{
		settings->SetJavaPath(FindJavaPath());
	}

	if (settings->GetAutoUpdate())
	{
		CheckUpdateTask *task = new CheckUpdateTask();
		task->Start(this,false);
	}

	if(!launchInstance.empty())
	{
		instItems.SelectInstanceByID(launchInstance);
		Instance * inst = instItems.GetSelectedInstance();
		if(inst == nullptr)
		{
			wxString output = _("Couldn't find the instance you tried to load: ");
			output.append(launchInstance);
			output.append(_(". Make sure it exists!"));
			wxLogError(output);
		}
		else
		{
			LoginClicked();
		}
	}
}

void MainWindow::InitBasicGUI(wxBoxSizer *mainSz)
{
	instListCtrl = new InstanceCtrl(this, &instItems, ID_InstListCtrl,wxDefaultPosition,wxDefaultSize);
	instListCtrl->SetDropTarget(new InstanceCtrl::InstCtrlDropTarget(instListCtrl));
	instItems.SetLinkedControl(instListCtrl);
	InitInstMenu();
	
	instNotesEditor = nullptr;
	
	mainSz->Add(instListCtrl, 1, wxEXPAND);
}

void MainWindow::InitInstMenu()
{
	// Build the instance context menu
	instMenu = new wxMenu();
	instMenu->Append(ID_Play, _("&Play"), _("Launch the instance."));
	instMenu->AppendSeparator();
	instMenu->Append(ID_Rename, _("&Rename"), _("Change the instance's name."));
	instMenu->Append(ID_SetGroup, _("Change Group"), _("Change the instance's group."));
	instMenu->Append(ID_ChangeIcon, _("&Change Icon"), _("Change this instance's icon."));
	instMenu->Append(ID_EditNotes, _("&Notes"), _("View / edit this instance's notes."));
	instMenu->Append(ID_Configure, _("&Settings"), _("Change instance settings."));
#if WINDOWS
	// Only works on Windows.
	instMenu->Append(ID_MakeDesktopLink, _("Make Desktop Shortcut"), _("Makes a shortcut on the desktop to launch this instance."));
#endif
	instMenu->AppendSeparator();
	instMenu->Append(ID_ManageSaves, _("&Manage Saves"), _("Backup / restore your saves."));
	instMenu->Append(ID_EditMods, _("&Edit Mods"), _("Install or remove mods."));
	instMenu->Append(ID_DowngradeInst, _("Downgrade"), _("Use MCNostalgia to downgrade this instance."));
	instMenu->Append(ID_UseSnapshot, _("Snapshot"), _("Install a snapshot."));
	instMenu->Append(ID_ChangeLWJGL, _("Change LWJGL"), _("Use a different version of LWJGL with this instance."));
	instMenu->Append(ID_RebuildJar, _("Re&build Jar"), _("Reinstall all the instance's jar mods."));
	instMenu->Append(ID_ViewInstFolder, _("&View Folder"), _("Open the instance's folder."));
	instMenu->AppendSeparator();
	instMenu->Append(ID_DeleteInst, _("Delete"), _("Delete this instance."));

	// Build the group context menu.
	groupMenu = new wxMenu();
	groupMenu->Append(ID_RenameGroup, _("&Rename"), _("Rename the group."));
	groupMenu->Append(ID_DeleteGroup, _("Delete"), _("Delete this group, ungrouping all instances in it."));
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

	// create the instance list and link it to the model
	instListCtrl = new InstanceCtrl(instPanel, &instItems, ID_InstListCtrl, 
		wxDefaultPosition, wxDefaultSize, wxINST_SINGLE_COLUMN | wxBORDER_SUNKEN);
	instItems.SetLinkedControl(instListCtrl);
	
	instSz->Add(instListCtrl,wxGBPosition(0, 0), wxGBSpan(rows, 1),wxEXPAND/* | wxALL, 4*/);
	
	instNameSz = new wxBoxSizer(wxVERTICAL);
	instSz->Add(instNameSz, wxGBPosition(0, 1), wxGBSpan(1, cols - 2), wxEXPAND | wxALL, 4);
	
	instNameEditor = new wxTextCtrl(instPanel, ID_InstNameEditor, wxEmptyString,
		wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
	instNameEditor->SetMaxLength(instNameLengthLimit);
	instNameEditor->SetFont(nameEditFont);
	instNameSz->Add(instNameEditor, wxSizerFlags(0).Align(wxALIGN_CENTER).Expand());
	
	instNameLabel = new wxStaticText(instPanel, -1, "InstName", 
		wxDefaultPosition, wxDefaultSize);
	instNameLabel->SetFont(titleFont);
	instNameSz->Add(instNameLabel, wxSizerFlags(0).Align(wxALIGN_CENTER));
	
	instNotesEditor = new wxTextCtrl(instPanel, ID_NotesCtrl, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_RICH);
	//instNotesEditor->Bind(wxEVT_KILL_FOCUS, &MainWindow::OnNotesLostFocus, this, ID_NotesCtrl);

	instSz->Add(instNotesEditor, wxGBPosition(1, 1), wxGBSpan(rows - 1, cols - 2), wxEXPAND | wxLEFT | wxTOP | wxRIGHT, 4);
	
	wxPanel *btnPanel = new wxPanel(instPanel);
	wxBoxSizer *btnSz = new wxBoxSizer(wxVERTICAL);
	instSz->Add(btnPanel, wxGBPosition(1, cols - 1), wxGBSpan(rows - 1, 1), wxALIGN_RIGHT | wxLEFT | wxRIGHT, 8);
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
	btnInstSettings = new wxButton(btnPanel, ID_Configure, _("Instance Settings"));
	btnSz->Add(btnInstSettings, szflags);
	btnSz->AddSpacer(spacerSize);
	btnEditMods = new wxButton(btnPanel, ID_EditMods, _("Edit &Mods"));
	btnSz->Add(btnEditMods, szflags);
	btnManageSaves = new wxButton(btnPanel, ID_ManageSaves, _("Manage Saves"));
	btnSz->Add(btnManageSaves, szflags);
	btnDowngrade = new wxButton(btnPanel, ID_DowngradeInst, _("Downgrade"));
	btnSz->Add(btnDowngrade, szflags);
	btnSnapshot = new wxButton(btnPanel, ID_UseSnapshot, _("Snapshot"));
	btnSz->Add(btnSnapshot, szflags);
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
	if(GetGUIMode() != GUI_Fancy)
		return;
	instPanel->Show();
	if(instItems.GetSelectedInstance() && !renamingInst)
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
			instName.Append("...");
		}
		instNameLabel->SetLabel(instName);
	}
	else
	{
		instNameLabel->SetLabel(_("Select an instance"));
	}
}

void MainWindow::OnInstSelected(InstanceCtrlEvent &event)
{
	if(GetGUIMode() == GUI_Fancy)
		SaveNotesBox(false);
	auto currentInstance = instItems.GetSelectedInstance();
	SetStatusText(wxT("Minecraft Version: ") + currentInstance->GetJarVersion());
	SetStatusText(wxT("Instance ID: ") + currentInstance->GetInstID(), 1);

	if(GetGUIMode() == GUI_Fancy)
		UpdateInstPanel();
}

void MainWindow::LoadInstanceList(wxFileName instDir)
{
	GetStatusBar()->PushStatusText(_("Loading instances..."), 0);

	if (!instDir.DirExists())
	{
		if (!instDir.Mkdir())
		{
			wxLogError(_("Failed to create instance directory."));
			return;
		}
	}
	
	int ctr = 0;
	instItems.Freeze();
	{
		instItems.Clear();
		
		wxDir dir(instDir.GetFullPath());
		if (!dir.IsOpened())
		{
			return;
		}
		
		Enable(false);
		wxString subFolder;
		
		bool cont = dir.GetFirst(&subFolder, wxEmptyString, wxDIR_DIRS);
		while (cont)
		{
			wxString dirName = Path::Combine(instDir, subFolder);
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


		wxString groupFile = Path::Combine(settings->GetInstDir(), "instgroups.json");
		instItems.SetGroupFile(groupFile);
		if (wxFileExists(groupFile))
			instItems.LoadGroupInfo();
	}
	instItems.Thaw();
	GetStatusBar()->SetStatusText(wxString::Format(_("Loaded %i instances..."), ctr), 0);
	Enable(true);
	
	if (GetGUIMode() == GUI_Fancy)
	{
		UpdateInstPanel();
	}
	
	//GetStatusBar()->PopStatusText(0);
}

void MainWindow::AddInstance(Instance *inst)
{
	instItems.Add(inst);
	wxSizer * sz = GetSizer();
	if(sz)
		sz->Layout();
}

Instance* MainWindow::GetLinkedInst(int id)
{
	if (id < 0)
		return nullptr;
	return instItems[id];
}

bool MainWindow::GetNewInstName(wxString *instName, wxString *instDirName, const wxString title)
{
	wxString newInstName = wxEmptyString;
Retry:
	newInstName = wxGetTextFromUser(_("Instance name:"), 
		title, newInstName, this);

	if (newInstName.empty())
	{
		return false;
	}
	else if (newInstName.Len() > instNameLengthLimit)
	{
		wxMessageBox(_("Sorry, that name is too long."), _("Error"), wxOK | wxCENTER, this);
		goto Retry;
	}

	int num = 0;
	wxString dirName = Utils::RemoveInvalidPathChars(newInstName, '-', false);
	while (wxDirExists(Path::Combine(settings->GetInstDir(), dirName)))
	{
		num++;
		dirName = Utils::RemoveInvalidPathChars(newInstName, '-', false) + wxString::Format("_%i", num);

		// If it's over 9000
		if (num > 9000)
		{
			wxLogError(_("Couldn't create instance folder: %s"),
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
	addInstMenu->Append(ID_ImportFTB, _("Import from FTB launcher."));
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

	wxString instDir = Path::Combine(settings->GetInstDir(), instDirName);
	
	Instance *inst = new StdInstance(instDir);
	UserInfo lastLogin;
	if (wxFileExists("lastlogin4"))
	{
		lastLogin.LoadFromFile("lastlogin4");
		if(lastLogin.username.Lower().Contains("direwolf"))
		{
			inst->SetIconKey("enderman");
		}
		else if (lastLogin.username.Lower().Contains("rootbear75"))
		{
			inst->SetIconKey("derp");
		}
	}
	inst->SetName(instName);
	AddInstance(inst);
}

void MainWindow::OnImportMCFolder(wxCommandEvent& event)
{
	wxDirDialog dirDlg(this, _("Select a Minecraft folder to import"));
	dirDlg.CenterOnParent();
	if (dirDlg.ShowModal() != wxID_OK)
		return;

	wxString existingMCDir = dirDlg.GetPath();

	wxString instName;
	wxString instDirName;
	if (!GetNewInstName(&instName, &instDirName, _("Import existing Minecraft folder")))
		return;

	instDirName = Path::Combine(settings->GetInstDir(), Utils::RemoveInvalidPathChars(instDirName, '-', false));

	wxMkdir(instDirName);

	Instance *inst = new StdInstance(instDirName);
	inst->SetName(instName);
	
	auto task = new FileCopyTask(existingMCDir, inst->GetMCDir());
	StartTask(task);
	delete task;
	
	AddInstance(inst);
}

void MainWindow::OnImportCPClicked(wxCommandEvent& event)
{
	wxFileDialog fileDlg(this, _("Choose a pack to import."),
		wxEmptyString, wxEmptyString, "*.zip", wxFD_OPEN);
	fileDlg.CenterOnParent();
	if (fileDlg.ShowModal() == wxID_OK)
	{
		ConfigPack cp(fileDlg.GetPath());
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

void MainWindow::OnImportFTBClicked(wxCommandEvent& event)
{
	// Select the launcher folder.
	wxDirDialog dirDlg(this, _("Please select your FTB launcher folder."));
	dirDlg.CenterOnParent();
	if (dirDlg.ShowModal() == wxID_OK && wxDirExists(dirDlg.GetPath()))
	{
		// Choose a pack to import.
		SelectFTBDialog selDialog(this, dirDlg.GetPath());
		selDialog.CenterOnParent();
		if (selDialog.ShowModal() == wxID_OK)
		{
			// Name the instance.
			wxString instName;
			wxString instDirName;
			if (GetNewInstName(&instName, &instDirName, _("Import FTB pack.")))
			{
				instDirName = Path::Combine(settings->GetInstDir(), instDirName);

				// Create the instance.
				wxMkdir(instDirName);

				Instance *inst = new StdInstance(instDirName);
				inst->SetName(instName);
				
				// Just to be safe...
				wxRmDir(inst->GetMCDir().GetFullPath());

				// Copy the pack to its new instance folder.
				FileCopyTask *copyTask = new FileCopyTask(
					selDialog.GetSelectedFolder(), instDirName);
				StartTask(copyTask);
				delete copyTask;

				// Make some corrections
				if (!wxFileExists(inst->GetVersionFile().GetFullPath()) &&
					wxFileExists(Path::Combine(inst->GetRootDir().GetFullPath(), "version")))
				{
					wxRenameFile(Path::Combine(inst->GetRootDir().GetFullPath(), "version"),
						inst->GetVersionFile().GetFullPath());
				}

				// Set needs rebuild.
				inst->SetNeedsRebuild();

				// Add the instance.
				AddInstance(inst);
			}
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
	SettingsDialog settingsDlg(this, -1);
	auto oldInstDir = settings->GetInstDir();
	settingsDlg.CenterOnParent();
	int response = settingsDlg.ShowModal();
	if (response == wxID_OK)
	{
		// if the instance folder changed, reload the instance list
		if(!oldInstDir.SameAs(settings->GetInstDir()))
		{
			LoadInstanceList();
		}
		instListCtrl->ReloadAll();
		
		if (settingsDlg.GetForceUpdateMultiMC())
		{
			wxString ciURL(_T(JENKINS_JOB_URL));

#if WINDOWS
			wxString dlFileName = "MultiMC.exe";
#else
			wxString dlFileName = "MultiMC";
#endif

			wxString dlURL = wxString::Format(
				"%s/lastStableBuild/artifact/%s",
				ciURL.c_str(), dlFileName.c_str());
			DownloadInstallUpdates(dlURL);
		}

		if (settingsDlg.ShouldRestartNow())
		{
			wxGetApp().exitAction = MultiMC::EXIT_RESTART;
			Close(false);
		}
	}
}

void MainWindow::OnCheckUpdateClicked(wxCommandEvent& event)
{
	auto task = new CheckUpdateTask();
	// if task was successful (this is modal, unlike the autoupdate check on start)
	if(StartTask(task))
	{
		CheckUpdateEvent event(task, task->m_buildNumber, task->m_downloadURL);
		OnCheckUpdateComplete(event);
	}
	delete task;
}

void MainWindow::OnCheckUpdateComplete(CheckUpdateEvent &event)
{
	// Clone the event so that we can keep it. We need to do this because 
	// if an instance is running, by the time processLater() is called, the 
	// original event will have been deleted.
	CheckUpdateEvent* newEvent = (CheckUpdateEvent*)event.Clone();

	DeferredEventFunc processLater = [&, newEvent] ()
	{
		if (newEvent->m_latestBuildNumber != AppVersion.GetBuild())
		{
			wxString updateMsg = wxString::Format(_("Build #%i is available. Would you like to download and install it?"), 
				newEvent->m_latestBuildNumber);

			UpdatePromptDialog updatePrompt (this, updateMsg);
			updatePrompt.CenterOnParent();
			int response = updatePrompt.ShowModal();
			if (response == ID_UpdateNow)
			{
				DownloadInstallUpdates(newEvent->m_downloadURL);
			}
			else if (response == ID_UpdateLater)
			{
				DownloadInstallUpdates(newEvent->m_downloadURL, false);
			}
			delete newEvent;
		}
	};
	CallWhenIdle(processLater);
}

void MainWindow::DownloadInstallUpdates(const wxString &downloadURL, bool installNow)
{
#if WINDOWS
	wxString updaterFileName = "MultiMCUpdate.exe";
#else
	wxString updaterFileName = "MultiMCUpdate";
#endif

	auto dlTask = new FileDownloadTask(downloadURL, wxFileName(updaterFileName), _("Downloading updates..."));

	if (installNow)
	{
		// Download and install in the foreground.
		StartTask(dlTask);
		delete dlTask;
		wxGetApp().exitAction = MultiMC::EXIT_UPDATE_RESTART;
		Close(false);
	}
	else
	{
		// Download in the background and install on exit.
		// FIXME: If MultiMC closes before the download finishes, the update will not install.
		dlTask->Start(this, false);
		wxGetApp().exitAction = MultiMC::EXIT_UPDATE;
	}
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
	info.SetVersion(wxString::Format("%s - %s", AppVersion.ToString().c_str(), AppBuildTag.ToString().c_str()));
	info.SetDescription(_("MultiMC is a custom launcher that makes managing Minecraft easier by allowing you to have multiple installations of Minecraft at once."));
	info.SetCopyright(_("(C) 2012 MultiMC Contributors"));
	
	info.SetWebSite("http://forkk.net/MultiMC4");
	info.SetLicense(licenseText);
	
	info.SetIcon(wxGetApp().GetAppIcons().GetIcon(wxSize(64, 64)));

	info.AddDeveloper(_("Andrew Okin <forkk@forkk.net>"));
	info.AddDeveloper(_("Petr Mrázek <peterix@gmail.com>"));
	
	wxAboutBox(info);
#else
	AboutDlgInfo info;

	info.name = _("MultiMC");
	info.version = wxString::Format("%s - %s", AppVersion.ToString().c_str(), AppBuildTag.ToString().c_str());
	info.description = _("MultiMC is a custom launcher that makes managing Minecraft easier by allowing you to have multiple installations of Minecraft at once.");
	info.copyright = _("(C) 2012 MultiMC Contributors");

	info.website = "http://forkk.net/MultiMC4";
	info.license = licenseText;

	info.icon = wxGetApp().GetAppIcons().GetIcon(wxSize(64, 64));

	AboutDlg aboutDlg(this, info);
	aboutDlg.CenterOnParent();
	aboutDlg.ShowModal();
#endif
}

void MainWindow::OnBugReportClicked ( wxCommandEvent& event )
{
	if(!Utils::OpenURL("http://bugs.forkk.net/"))
	{
		wxMessageBox(_("MultiMC was unable to run your web browser.\n\nTo report bugs, visit:\nhttp://bugs.forkk.net/"), 
		_("Error"), wxOK | wxCENTER | wxICON_ERROR, this);
	}
}


void MainWindow::NotImplemented()
{
	wxMessageBox(_("This feature has not yet been implemented."), 
		_("Not implemented"), wxOK | wxCENTER, this);
}


// Instance menu
void MainWindow::OnPlayClicked(wxCommandEvent& event)
{
	LoginClicked();
}

void MainWindow::OnInstActivated(InstanceCtrlEvent &event)
{
	LoginClicked();
}

void MainWindow::LoginClicked()
{
	auto currentInstance = instItems.GetSelectedInstance();
	if (!currentInstance)
	{
		return;
	}

	if (currentInstance->GetAutoLogin() && wxFileExists("lastlogin4"))
	{
		UserInfo lastLogin;
		lastLogin.LoadFromFile("lastlogin4");
		DoLogin(lastLogin);
	}
	else
	{
		ShowLoginDlg(wxEmptyString);
	}
}

void MainWindow::DoLogin(UserInfo info, bool playOffline, bool forceUpdate)
{
	auto currentInstance = instItems.GetSelectedInstance();
	info.SaveToFile("lastlogin4");

	if (!playOffline)
	{
		LoginTask *task = new LoginTask(info, currentInstance, forceUpdate);
		StartTask(task);
		OnLoginComplete(task->GetLoginResult());
		delete task;
	}
	else
	{
		LoginResult lr = LoginResult::PlayOffline(info.username);
		OnLoginComplete(lr);
	}
}

void MainWindow::ShowLoginDlg(wxString errorMsg)
{
	auto currentInstance = instItems.GetSelectedInstance();
	if(!currentInstance)
	{
		// FIXME: what if the instance somehow becomes deselected while playing? is that possible?
		//        what does it mean to the state of the GUI
		return;
	}

	UserInfo lastLogin;
	if (wxFileExists("lastlogin4"))
	{
		lastLogin.LoadFromFile("lastlogin4");
	}

	bool canPlayOffline = currentInstance->HasBinaries();
	LoginDialog loginDialog(this, errorMsg, lastLogin, canPlayOffline);
	loginDialog.CenterOnParent();
	int response = loginDialog.ShowModal();
	
	bool playOffline = response == ID_PLAY_OFFLINE;
	if (response == wxID_OK || playOffline)
	{
		UserInfo info(loginDialog);
		DoLogin(info, playOffline, loginDialog.ShouldForceUpdate());
	}
	else if (!launchInstance.IsEmpty() && response == wxID_CANCEL)
	{
		this->Destroy();
	}
}

void MainWindow::OnLoginComplete( const LoginResult& result )
{
	auto currentInstance = instItems.GetSelectedInstance();
	if (!result.loginFailed)
	{
		// Login success
		Instance *inst = currentInstance;

		// If the session ID is empty, the game updater will not be run.
		wxString sessionID = result.sessionID;
		sessionID.Trim();
		if (!result.playOffline && !sessionID.IsEmpty() && sessionID != "Offline")
		{
			auto task = new GameUpdateTask(inst, result.latestVersion, result.forceUpdate);
			StartTask(task);
			delete task;
			if(GetGUIMode() == GUI_Fancy)
				UpdateInstPanel();
		}
		
		if (inst->ShouldRebuild())
		{
			auto task = new ModderTask (inst);
			StartTask(task);
			delete task;
		}
		
		InstConsoleWindow *cwin = new InstConsoleWindow(inst, this, !launchInstance.IsEmpty());
		cwin->SetUserInfo(result.username, result.sessionID);
		cwin->SetName(wxT("InstConsoleWindow"));
		if (!wxPersistenceManager::Get().RegisterAndRestore(cwin))
			cwin->CenterOnScreen();
		
		if (MinecraftProcess::Launch(inst, cwin, result.username, result.sessionID) != nullptr)
		{
			Show(false);
			cwin->Show(settings->GetShowConsole());
			instListCtrl->ReloadAll();
			SetGUIState(STATE_INST_RUNNING);
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

void MainWindow::ReturnToMainWindow()
{
	Show();
	Raise();
	SetGUIState(STATE_IDLE);
}

void MainWindow::RenameEvent()
{
	auto currentInstance = instItems.GetSelectedInstance();
	switch (GetGUIMode())
	{
	case GUI_Fancy:
		StartRename();
		break;
		
	case GUI_Simple:
	{
		if(!currentInstance)
			break;
		wxTextEntryDialog textDlg(this, _("Enter a new name for this instance."), 
			_("Rename Instance"), currentInstance->GetName());
		textDlg.CenterOnParent();
		while(1)
		{
			int response = textDlg.ShowModal();
			if(response == wxID_CANCEL)
				break;
			wxString str = textDlg.GetValue();
			if(str.length() > 25)
			{
				wxMessageBox(_("Sorry, that name is too long. 25 characters is the limit."), _("Error"), wxOK | wxCENTER, this);
				continue;
			}
			//FIXME: this should be handled and passed on by the model
			currentInstance->SetName(str);
			break;
		}
	}
	}
}

void MainWindow::OnInstRenameKey ( InstanceCtrlEvent& event )
{
	RenameEvent();
}

void MainWindow::OnRenameClicked(wxCommandEvent& event)
{
	RenameEvent();
}

void MainWindow::OnChangeGroupClicked(wxCommandEvent& event)
{
	auto currentInstance = instItems.GetSelectedInstance();
	if (!currentInstance)
		return;

	wxTextEntryDialog textDlg(this, _("Enter a new group for this instance."), 
		_("Change Group"), currentInstance->GetGroup());
	textDlg.CenterOnParent();
	if (textDlg.ShowModal() == wxID_OK)
	{
		currentInstance->SetGroup(textDlg.GetValue());
	}
}

void MainWindow::OnChangeIconClicked(wxCommandEvent& event)
{
	ChangeIconDialog iconDlg(this);
	iconDlg.CenterOnParent();
	if (iconDlg.ShowModal() == wxID_OK)
	{
		auto currentInstance = instItems.GetSelectedInstance();
		if(!currentInstance)
			return;
		currentInstance->SetIconKey(iconDlg.GetSelectedIconKey());
		instListCtrl->Refresh();
	}
}

void MainWindow::OnCopyInstClicked(wxCommandEvent &event)
{
	auto currentInstance = instItems.GetSelectedInstance();
	if(!currentInstance)
		return;

	wxString instName;
	wxString instDirName;
	if (!GetNewInstName(&instName, &instDirName, _("Copy existing instance")))
		return;

	instDirName = Path::Combine(settings->GetInstDir(), Utils::RemoveInvalidPathChars(instDirName, '-', false));

	wxMkdir(instDirName);
	auto task = new FileCopyTask (currentInstance->GetRootDir().GetFullPath(), wxFileName::DirName(instDirName));
	StartTask(task);
	delete task;

	//FIXME: I wouldn't be so sure about *THIS*
	Instance *newInst = new StdInstance(instDirName);
	newInst->SetName(instName);
	AddInstance(newInst);
}

void MainWindow::OnMakeDesktopLinkClicked(wxCommandEvent& event)
{
	auto currentInst = instItems.GetSelectedInstance();
	if (!currentInst)
		return;

#if WINDOWS
	// Find the Desktop folder.
	wxString desktopDir = Path::GetDesktopDir();

	wxString shortcutName;
AskAgain:
	shortcutName = wxGetTextFromUser(_("Enter a name for the shortcut: "), 
		_("Name Shortcut"), shortcutName, this);

	if (shortcutName.IsEmpty())
	{
		return;
	}
	else if (Utils::ContainsInvalidPathChars(shortcutName, true))
	{
		wxMessageBox(wxString::Format(
			_("Shortcut name cannot contain any invalid characters (such as '%s')."), wxFileName::GetForbiddenChars()),
			_("Invalid Shortcut Name"));
		goto AskAgain;
	}

	wxString exePath = wxStandardPaths::Get().GetExecutablePath();
	if (exePath.IsEmpty())
		exePath = wxGetApp().argv[0];

	if (!CreateShortcut(Path::Combine(desktopDir, shortcutName + ".lnk"), 
		exePath, wxString::Format("-l \"%s\"", currentInst->GetInstID().c_str())))
	{
		wxLogError(_("Failed to create desktop shortcut. An unknown error occurred."));
		return;
	}
#else
	wxMessageBox(_("Sorry, desktop shortcuts are only supported on Windows."), _("Not Supported"));
#endif
}

void MainWindow::OnInstanceSettingsClicked ( wxCommandEvent& event )
{
	auto currentInstance = instItems.GetSelectedInstance();
	SettingsDialog settingsDlg(this, -1, currentInstance);
	settingsDlg.CenterOnParent();
	settingsDlg.ShowModal();
}

void MainWindow::OnNotesClicked(wxCommandEvent& event)
{
	auto currentInstance = instItems.GetSelectedInstance();
	switch (GetGUIMode())
	{
	case GUI_Simple:
	{
		if(!currentInstance)
			return;
		wxTextEntryDialog textDlg(this, _("Instance notes"), _("Notes"), currentInstance->GetNotes(), 
			wxOK | wxCANCEL | wxTE_MULTILINE);
		textDlg.SetSize(600, 400);
		textDlg.CenterOnParent();
		if (textDlg.ShowModal() == wxID_OK)
		{
			currentInstance->SetNotes(textDlg.GetValue());
		}
		break;
	}
	}
}

void MainWindow::SaveNotesBox(bool current)
{
	Instance * inst = nullptr;
	if(current)
		inst = instItems.GetSelectedInstance();
	else
		inst = instItems.GetPreviousInstance();
	if (inst != nullptr)
	{
		wxString notes = instNotesEditor->GetValue();
		inst->SetNotes(notes);
	}
}

void MainWindow::UpdateNotesBox()
{
	auto currentInstance = instItems.GetSelectedInstance();
	if (currentInstance)
		instNotesEditor->SetValue(currentInstance->GetNotes());
	else
		instNotesEditor->SetValue(wxString());
}


void MainWindow::StartRename()
{
	auto currentInstance = instItems.GetSelectedInstance();
	if(!currentInstance)
		return;
	DisableInstActions();
	renamingInst = true;
	
	GetStatusBar()->PushStatusText(wxString::Format(
		_("Renaming instance '%s'... (Press enter to finish)"), 
		currentInstance->GetName().c_str()), 0);
	
	instNameEditor->SetValue(currentInstance->GetName());
	
	instNameLabel->Show(false);
	instNameSz->Hide(instNameLabel);
	
	instNameEditor->Show(true);
	instNameSz->Show(instNameEditor);
	instNameEditor->SetFocus();
	
	instNameSz->Layout();
}

void MainWindow::FinishRename()
{
	auto currentInstance = instItems.GetSelectedInstance();
	if(currentInstance)
	{
		if (!instNameEditor->IsEmpty())
		{
			currentInstance->SetName(instNameEditor->GetValue());
		}
	}
	CancelRename();
}

void MainWindow::CancelRename()
{
	auto currentInstance = instItems.GetSelectedInstance();
	if (renamingInst)
	{
		EnableInstActions();
		renamingInst = false;
		GetStatusBar()->PopStatusText(0);
	}
	
	UpdateInstNameLabel(currentInstance);
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
	case GUI_Fancy:
		btnPlay->Enable(enabled);
		btnRename->Enable(enabled);
		btnChangeIcon->Enable(enabled);
		btnManageSaves->Enable(enabled);
		btnEditMods->Enable(enabled);
		btnInstSettings->Enable(enabled);
		btnRebuildJar->Enable(enabled);
		btnViewFolder->Enable(enabled);
		btnCopyInst->Enable(enabled);
		btnDowngrade->Enable(enabled);
		btnSnapshot->Enable(enabled);
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
	auto currentInstance = instItems.GetSelectedInstance();
	if (!currentInstance)
		return;

	SaveMgrWindow *saveMgr = new SaveMgrWindow(this, currentInstance);
	saveMgr->Show();
}

void MainWindow::OnEditModsClicked(wxCommandEvent& event)
{
	auto currentInstance = instItems.GetSelectedInstance();
	if(currentInstance == nullptr)
		return;
	ModEditWindow *editDlg = new ModEditWindow(this, currentInstance);
	editDlg->Show();
}

void MainWindow::OnDowngradeInstClicked(wxCommandEvent& event)
{
	auto currentInstance = instItems.GetSelectedInstance();
	if(currentInstance == nullptr)
		return;

	if (currentInstance->GetVersionFile().FileExists())
	{
		DowngradeDialog downDlg(this);
		downDlg.CenterOnParent();
		if (downDlg.ShowModal() == wxID_OK && !downDlg.GetSelection().IsEmpty())
		{
			if (downDlg.GetSelection().Contains(wxT("indev")) ||
				downDlg.GetSelection().Contains(wxT("infdev")))
			{
				if (wxMessageBox(_("MultiMC is currently incompatible with \
indev and infdev. Are you sure you would like to downgrade to this version?"), 
						_("Continue?"), wxYES_NO) == wxNO)
				{
					return;
				}
			}

			auto task = new DowngradeTask (currentInstance, downDlg.GetSelection());
			StartTask(task);
			delete task;
			UpdateInstPanel();
		}
	}
	else
	{
		wxLogError(_("You must run this instance at least once to download minecraft before you can downgrade it!"));
	}
}

void MainWindow::OnSnapshotClicked(wxCommandEvent& event)
{
	auto currentInstance = instItems.GetSelectedInstance();
	if(currentInstance == nullptr)
		return;

	if (currentInstance->GetVersionFile().FileExists())
	{
		SnapshotDialog snapDlg(this);
		snapDlg.CenterOnParent();
		MCVersion ver;
		if (snapDlg.ShowModal() == wxID_OK && snapDlg.GetSelectedVersion(ver))
		{
			wxString snapURL = ver.dlURL + "minecraft.jar";

			wxString snapshotJar = Path::Combine(currentInstance->GetBinDir(), wxT("snapshot.jar"));
			FileDownloadTask task(snapURL, snapshotJar);
			if (StartTask(&task))
			{
				if (wxFileExists(currentInstance->GetMCBackup().GetFullPath()) &&
					!wxRemoveFile(currentInstance->GetMCBackup().GetFullPath()))
				{
					wxLogError(_("MultiMC was unable to replace the old .jar with the new snapshot."));
					return;
				}

				if (wxCopyFile(snapshotJar, currentInstance->GetMCJar().GetFullPath()))
				{
					currentInstance->UpdateVersion();
					UpdateInstPanel();
				}
			}
		}
	}
	else
	{
		wxLogError(_("You must run this instance at least once to download minecraft before you can downgrade it!"));
	}
}

void MainWindow::OnChangeLWJGLClicked(wxCommandEvent& event)
{
	ChooseLWJGLDialog lwjglDlg(this);
	lwjglDlg.CenterOnParent();
	if (lwjglDlg.ShowModal() == wxID_OK && !lwjglDlg.GetSelection().IsEmpty())
	{
		// Download LWJGL
		FileDownloadTask *dlTask = new FileDownloadTask(lwjglDlg.GetSelectedURL(),
			wxFileName("lwjgl.zip"), _("Downloading LWJGL..."));
		TaskProgressDialog tDialog(this);
		if (!tDialog.ShowModal(dlTask))
			return;
		delete dlTask;

		LWJGLInstallTask *installTask = new LWJGLInstallTask(
			instItems.GetSelectedInstance(), "lwjgl.zip");
		tDialog.ShowModal(installTask);
		delete installTask;
	}
}

void MainWindow::OnRebuildJarClicked(wxCommandEvent& event)
{
	auto currentInstance = instItems.GetSelectedInstance();
	if(currentInstance == nullptr)
		return;
	auto task = new ModderTask(currentInstance);
	StartTask(task);
	delete task;
}

void MainWindow::OnViewInstFolderClicked(wxCommandEvent& event)
{
	auto currentInstance = instItems.GetSelectedInstance();
	if(currentInstance == nullptr)
		return;
	Utils::OpenFolder(currentInstance->GetRootDir());
}

bool MainWindow::DeleteSelectedInstance()
{
	auto currentInstance = instItems.GetSelectedInstance();
	if(currentInstance == nullptr)
		return false;

	wxMessageDialog dlg(this, "Are you sure you want to delete this instance?\n"
	                          "Deleted instances are lost FOREVER! (a really long time)",
	                          "Confirm deletion.",
	                          wxYES_NO | wxNO_DEFAULT | wxICON_QUESTION | wxCENTRE | wxSTAY_ON_TOP);
	dlg.CenterOnParent();
	if (dlg.ShowModal() == wxID_YES)
	{
		instItems.DeleteCurrent();
		
		if(GetGUIMode() == GUI_Fancy)
		{
			UpdateInstPanel();
		}
		return true;
	}
	return false;
}

void MainWindow::OnInstDeleteKey ( InstanceCtrlEvent& event )
{
	DeleteSelectedInstance();
}

void MainWindow::OnDeleteClicked(wxCommandEvent& event)
{
	DeleteSelectedInstance();
}

void MainWindow::OnInstMenuOpened(InstanceCtrlEvent& event)
{
	if (event.GetItemIndex().isItem())
	{
		if (instActionsEnabled)
			PopupMenu(instMenu, event.GetPosition());
	}
	else if (event.GetItemIndex().isGroup())
	{
		lastClickedGroup = event.GetGroup();
		if (lastClickedGroup)
		{
			PopupMenu(groupMenu, event.GetPosition());
		}
	}
	else
	{
		//TODO: A menu for the instance control itself could be spawned there (with stuff like 'create instance', etc.)
	}
}

void MainWindow::OnRenameGroupClicked(wxCommandEvent& event)
{
	if (!lastClickedGroup)
		return;

	wxTextEntryDialog textDlg(this, _("Enter a new name for this group."), 
		_("Rename Group"), lastClickedGroup->GetName());
	textDlg.CenterOnParent();
	if (textDlg.ShowModal() == wxID_OK)
	{
		lastClickedGroup->SetName(textDlg.GetValue());
		instListCtrl->ReloadAll();
		instItems.SaveGroupInfo();
	}
}

void MainWindow::OnDeleteGroupClicked(wxCommandEvent& event)
{
	if (!lastClickedGroup)
		return;

	instItems.DeleteGroup(lastClickedGroup);
}

// this catches background tasks and destroys them
void MainWindow::OnTaskEnd(TaskEvent& event)
{
	Task * t = event.m_task;
	t->Wait();
	delete t;
}

void MainWindow::OnTaskError(TaskErrorEvent& event)
{
	wxLogError(event.m_errorMsg);
}

int MainWindow::StartTask ( Task* task )
{
	TaskProgressDialog dlg(this);
	return dlg.ShowModal(task);
}

void MainWindow::OnWindowClosed(wxCloseEvent& event)
{
	wxPersistenceManager::Get().SaveAndUnregister(this);
	if(instNotesEditor)
	{
		// Save instance notes on exit.
		SaveNotesBox(true);
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

void MainWindow::OnExitApp(wxCommandEvent &event)
{
	Close();
}
/*
void MainWindow::OnNotesLostFocus(wxFocusEvent& event)
{
	SaveNotesBox(true);
	event.Skip();
}
*/

MainWindow::GUIState MainWindow::GetGUIState() const
{
	return m_guiState;
}

void MainWindow::SetGUIState(MainWindow::GUIState state)
{
	m_guiState = state;

	if (m_guiState == STATE_IDLE)
	{
		// If idle, call idle functions.
		CallIdleFunctions();
	}
}

void MainWindow::CallWhenIdle(DeferredEventFunc func)
{
	// If idle right now, call the function.
	// Otherwise, add it to the queue.
	if (GetGUIState() == STATE_IDLE)
		func();
	else
		m_idleQueue.push(func);
}

void MainWindow::CallIdleFunctions()
{
	while (!m_idleQueue.empty() && GetGUIState() == STATE_IDLE)
	{
		ProcessNextIdleFunction();
	}
}

void MainWindow::ProcessNextIdleFunction()
{
	// Pop before calling the function to avoid possible infinite recursion.
	DeferredEventFunc func = m_idleQueue.front();
	m_idleQueue.pop();
	func();
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
	EVT_MENU(ID_ImportFTB, MainWindow::OnImportFTBClicked)

	EVT_MENU(ID_Play, MainWindow::OnPlayClicked)
	
	EVT_MENU(ID_Rename, MainWindow::OnRenameClicked)
	EVT_MENU(ID_SetGroup, MainWindow::OnChangeGroupClicked)
	EVT_MENU(ID_ChangeIcon, MainWindow::OnChangeIconClicked)
	EVT_MENU(ID_EditNotes, MainWindow::OnNotesClicked)
	EVT_MENU(ID_MakeDesktopLink, MainWindow::OnMakeDesktopLinkClicked)
	EVT_MENU(ID_Configure, MainWindow::OnInstanceSettingsClicked)
	
	EVT_MENU(ID_ManageSaves, MainWindow::OnManageSavesClicked)
	EVT_MENU(ID_EditMods, MainWindow::OnEditModsClicked)
	EVT_MENU(ID_DowngradeInst, MainWindow::OnDowngradeInstClicked)
	EVT_MENU(ID_UseSnapshot, MainWindow::OnSnapshotClicked)
	EVT_MENU(ID_ChangeLWJGL, MainWindow::OnChangeLWJGLClicked)
	EVT_MENU(ID_RebuildJar, MainWindow::OnRebuildJarClicked)
	EVT_MENU(ID_ViewInstFolder, MainWindow::OnViewInstFolderClicked)

	EVT_MENU(ID_DeleteInst, MainWindow::OnDeleteClicked)


	EVT_MENU(ID_RenameGroup, MainWindow::OnRenameGroupClicked)
	EVT_MENU(ID_DeleteGroup, MainWindow::OnDeleteGroupClicked)
	
	
	EVT_BUTTON(ID_Play, MainWindow::OnPlayClicked)
	
	EVT_BUTTON(ID_Rename, MainWindow::OnRenameClicked)
	EVT_BUTTON(ID_ChangeIcon, MainWindow::OnChangeIconClicked)
	EVT_BUTTON(ID_CopyInst, MainWindow::OnCopyInstClicked)
	EVT_BUTTON(ID_EditNotes, MainWindow::OnNotesClicked)
	EVT_BUTTON(ID_Configure, MainWindow::OnInstanceSettingsClicked)
	
	EVT_BUTTON(ID_ManageSaves, MainWindow::OnManageSavesClicked)
	EVT_BUTTON(ID_EditMods, MainWindow::OnEditModsClicked)
	EVT_BUTTON(ID_DowngradeInst, MainWindow::OnDowngradeInstClicked)
	EVT_BUTTON(ID_UseSnapshot, MainWindow::OnSnapshotClicked)
	EVT_BUTTON(ID_RebuildJar, MainWindow::OnRebuildJarClicked)
	EVT_BUTTON(ID_ViewInstFolder, MainWindow::OnViewInstFolderClicked)
	
	EVT_BUTTON(ID_DeleteInst, MainWindow::OnDeleteClicked)
	
	
	EVT_INST_ACTIVATE(ID_InstListCtrl, MainWindow::OnInstActivated)
	EVT_INST_DELETE(ID_InstListCtrl, MainWindow::OnInstDeleteKey)
	EVT_INST_RENAME(ID_InstListCtrl, MainWindow::OnInstRenameKey)
	EVT_INST_MENU(ID_InstListCtrl, MainWindow::OnInstMenuOpened)
	EVT_INST_ITEM_SELECTED(ID_InstListCtrl, MainWindow::OnInstSelected)
	
	EVT_TASK_END(MainWindow::OnTaskEnd)
	EVT_TASK_ERRORMSG(MainWindow::OnTaskError)
	
	EVT_CHECK_UPDATE(MainWindow::OnCheckUpdateComplete)
	
	EVT_TEXT_ENTER(ID_InstNameEditor, MainWindow::OnRenameEnterPressed)

	EVT_CLOSE(MainWindow::OnWindowClosed)
	EVT_MENU(wxID_EXIT, MainWindow::OnExitApp)
END_EVENT_TABLE()
