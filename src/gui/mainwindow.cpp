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
#include "modeditdialog.h"
#include "changeicondialog.h"

#include "toolbaricons.h"
#include "windowicon.h"

#include "gameupdatetask.h"
#include "logintask.h"
#include "moddertask.h"
#include <checkupdatetask.h>
#include <filedownloadtask.h>
#include "version.h"

#include <wx/filesys.h>
#include <wx/dir.h>
#include <wx/fs_arc.h>
#include <wx/fs_mem.h>
#include <wx/aboutdlg.h>
#include <wx/cmdline.h>
#include <wx/stdpaths.h>

IMPLEMENT_APP(MultiMC)

const int instNameLengthLimit = 20;

// Main window
MainWindow::MainWindow(void)
	: wxFrame(NULL, -1, 
		wxString::Format(_("MultiMC"), 
			AppVersion.GetMajor(), AppVersion.GetMinor(), AppVersion.GetRevision()),
		wxPoint(0, 0), wxSize(620, 400)),
		instIcons(32, 32)
{
	closeOnTaskEnd = false;
	
	SetIcon(wxGetApp().GetAppIcon());
	
	wxToolBar *mainToolBar = CreateToolBar();
	
	// Load toolbar icons
	wxBitmap newInstIcon = wxMEMORY_IMAGE(newinsticon);
	wxBitmap reloadIcon = wxMEMORY_IMAGE(refreshinsticon);
	wxBitmap viewFolderIcon = wxMEMORY_IMAGE(viewfoldericon);
	wxBitmap settingsIcon = wxMEMORY_IMAGE(settingsicon);
	wxBitmap checkUpdateIcon = wxMEMORY_IMAGE(checkupdateicon);
	wxBitmap helpIcon = wxMEMORY_IMAGE(helpicon);
	wxBitmap aboutIcon = wxMEMORY_IMAGE(abouticon);

	// Build the toolbar
	mainToolBar->AddTool(ID_AddInst, _("Add instance"), newInstIcon, _("Add a new instance."));
	mainToolBar->AddTool(ID_Refresh, _("Refresh"), reloadIcon, _("Reload ALL the instances!"));
	mainToolBar->AddTool(ID_ViewFolder, _("View folder"), viewFolderIcon, _("Open the instance folder."));
	mainToolBar->AddSeparator();
	mainToolBar->AddTool(ID_Settings, _("Settings"), settingsIcon, _("Settings"));
	mainToolBar->AddTool(ID_CheckUpdate, _("Check for updates"), checkUpdateIcon, _("Check for MultiMC updates."));
	mainToolBar->AddSeparator();
	mainToolBar->AddTool(ID_Help, _("Help"), helpIcon, _("Help"));
	mainToolBar->AddTool(ID_About, _("About"), aboutIcon, _("About MultiMC"));

	mainToolBar->Realize();

	// Build the instance context menu
	instMenu = new wxMenu();
	instMenu->Append(ID_Play, _T("&Play"), _T("Launch the instance."));
	instMenu->AppendSeparator();
	instMenu->Append(ID_Rename, _T("&Rename"), _T("Change the instance's name."));
	instMenu->Append(ID_ChangeIcon, _T("&Change Icon"), _T("Change this instance's icon."));
	instMenu->Append(ID_Notes, _T("&Notes"), _T("View / edit this instance's notes."));
	instMenu->AppendSeparator();
	instMenu->Append(ID_ManageSaves, _T("&Manage Saves"), _T("Backup / restore your saves."));
	instMenu->Append(ID_EditMods, _T("&Edit Mods"), _T("Install or remove mods."));
	instMenu->Append(ID_RebuildJar, _T("Re&build Jar"), _T("Reinstall all the instance's jar mods."));
	instMenu->Append(ID_ViewInstFolder, _T("&View Folder"), _T("Open the instance's folder."));
	instMenu->AppendSeparator();
	instMenu->Append(ID_DeleteInst, _T("Delete"), _T("Delete this instance."));
	
	// Create the status bar
	CreateStatusBar(1);
	SetStatusBarPane(0);
	
	// Set up the main panel and sizers
	wxPanel *panel = new wxPanel(this, -1);
	wxBoxSizer *box = new wxBoxSizer(wxVERTICAL);
	panel->SetSizer(box);
	
	// Create the instance list
	instListCtrl = new wxListCtrl(panel, ID_InstListCtrl, wxDefaultPosition, wxDefaultSize,
		wxLC_ICON | wxLC_ALIGN_LEFT | wxLC_EDIT_LABELS | wxLC_SINGLE_SEL);
	box->Add(instListCtrl, 1, wxEXPAND);
	instListCtrl->SetColumnWidth(0, 48);
	
	// Load instance icons
	LoadInstIconList();
	
	// Load instance list
	LoadInstanceList();
	
	CenterOnScreen();
}

MainWindow::~MainWindow(void)
{
	
}

void MainWindow::LoadInstIconList(wxString customIconDirName)
{
	instListCtrl->AssignImageList(instIcons.CreateImageList(), wxIMAGE_LIST_NORMAL);
}

void MainWindow::LoadInstanceList(wxFileName instDir)
{
	instListCtrl->ClearAll();
	instItems.clear();
	
	wxDir dir(instDir.GetFullPath());
	if (!dir.IsOpened())
	{
		return;
	}
	
	wxString subFolder;
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
		}
		cont = dir.GetNext(&subFolder);
	}
}

void MainWindow::AddInstance(Instance *inst)
{
	wxString instName = inst->GetName();
	if (instName.Len() > instNameLengthLimit)
	{
		instName.Truncate(instNameLengthLimit - 3);
		instName.Append(_("..."));
	}
	
	long item = instListCtrl->InsertItem(instListCtrl->GetItemCount(), 
		instName, instIcons[inst->GetIconKey()]);
	instItems[item] = inst;
}

Instance* MainWindow::GetLinkedInst(long item)
{
	return instItems[item];
}

Instance* MainWindow::GetSelectedInst()
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
	return NULL;
}


// Toolbar
void MainWindow::OnAddInstClicked(wxCommandEvent& event)
{
	wxString newInstName = wxEmptyString;
Retry:
	newInstName = wxGetTextFromUser(_T("Instance name:"), 
		_T("Create new instance"), newInstName, this);
	
	if (newInstName.empty())
	{
		return;
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
	wxFileName instDir = wxFileName::DirName(Path::Combine(settings.GetInstDir(), dirName));
	
	Instance *inst = new Instance(instDir);
	inst->SetName(newInstName);
	AddInstance(inst);
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
	if (event.m_latestBuildNumber > AppVersion.GetBuild())
	{
		if (wxMessageBox(wxString::Format(_("Build #%i is available. Would you like to download and install it?"), 
				event.m_latestBuildNumber), _("Update Available"), wxYES_NO) == wxYES)
		{
			FileDownloadTask dlTask(event.m_downloadURL, 
				wxFileName(_("MultiMCUpdate")), _("Downloading updates..."));
			wxGetApp().updateOnExit = true;
			StartModalTask(dlTask);
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
limitations under the License."));
	info.SetIcon(wxGetApp().GetAppIcon());
#endif
	
	info.AddDeveloper(_("Andrew Okin <forkk@forkk.net>"));
	info.AddDeveloper(_("Petr Mrázek"));
	
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
	UserInfo lastLogin;
	if (wxFileExists(_("lastlogin")))
	{
		wxFFileInputStream inStream(_("lastlogin"));
		lastLogin.LoadFromStream(inStream);
	}
	
	LoginDialog loginDialog(this, errorMsg, lastLogin);
	int response = loginDialog.ShowModal();
	
	if (response == wxID_OK)
	{
		UserInfo info(loginDialog);
		
		wxFFileOutputStream outStream(_("lastlogin"));
		info.SaveToStream(outStream);
		
		LoginTask *task = new LoginTask(info, GetSelectedInst(), loginDialog.ShouldForceUpdate());
		StartModalTask(*task, true);
	}
}

void MainWindow::OnLoginComplete(LoginCompleteEvent& event)
{
	LoginTask *loginTask = (LoginTask*)event.m_task;
	LoginResult result = event.m_loginResult;
	if (!result.loginFailed)
	{
		// Login success
		// If the session ID is empty, the game updater will not be run.
		if (!result.sessionID.empty())
		{
			Instance *inst = loginTask->m_inst;
			GameUpdateTask task(inst, result.latestVersion, _("minecraft.jar"), loginTask->m_forceUpdate);
			if (!StartModalTask(task))
			{
				return;
			}
			
			if (inst->ShouldRebuild())
			{
				ModderTask modTask(inst);
				StartModalTask(modTask);
			}
			
			Show(false);
			inst->Launch(result.username, result.sessionID, true);
			InstConsoleWindow *cwin = new InstConsoleWindow(inst, this);
			cwin->Show();
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
	Instance *inst = GetSelectedInst();
	wxTextEntryDialog textDlg(this, _("Enter a new name for this instance."), 
		_("Rename Instance"), inst->GetName());
	if (textDlg.ShowModal() == wxID_OK)
	{
		inst->SetName(textDlg.GetValue());
		LoadInstanceList();
	}
}

void MainWindow::OnChangeIconClicked(wxCommandEvent& event)
{
	ChangeIconDialog iconDlg(this, &instIcons);
	if (iconDlg.ShowModal() == wxID_OK)
	{
		Instance *inst = GetSelectedInst();
		inst->SetIconKey(iconDlg.GetSelectedIconKey());
		LoadInstanceList();
	}
}

void MainWindow::OnNotesClicked(wxCommandEvent& event)
{
	Instance *inst = GetSelectedInst();
	wxTextEntryDialog textDlg(this, _("Instance notes"), _("Notes"), inst->GetNotes(), 
		wxOK | wxCANCEL | wxTE_MULTILINE);
	textDlg.SetSize(600, 400);
	if (textDlg.ShowModal() == wxID_OK)
	{
		inst->SetNotes(textDlg.GetValue());
		LoadInstanceList();
	}
}

void MainWindow::OnManageSavesClicked(wxCommandEvent& event)
{
	NotImplemented();
}

void MainWindow::OnEditModsClicked(wxCommandEvent& event)
{
	ModEditDialog *editDlg = new ModEditDialog(this, GetSelectedInst());
	editDlg->ShowModal();
}

void MainWindow::OnRebuildJarClicked(wxCommandEvent& event)
{
	ModderTask *modTask = new ModderTask(GetSelectedInst());
	StartModalTask(*modTask);
}

void MainWindow::OnViewInstFolderClicked(wxCommandEvent& event)
{
	Instance *inst = GetSelectedInst();
	if (inst)
	{
		Utils::OpenFile(inst->GetRootDir());
	}
}

void MainWindow::OnDeleteClicked(wxCommandEvent& event)
{
	NotImplemented();
}

void MainWindow::OnInstMenuOpened(wxListEvent& event)
{
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
		if (task.GetProgress() == 0 || task.GetProgress() == 100)
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
	progDialog->Close(false);
	progDialog->Destroy();
	return !cancelled;
}


// App
bool MultiMC::OnInit()
{
	startNormally = true;
	
	wxApp::OnInit();
	
	if (!startNormally)
		return false;
	
	SetAppName(_("MultiMC"));
	
	wxInitAllImageHandlers();
	
	wxMemoryInputStream iconInput(MultiMC32_png, MultiMC32_png_len);
	AppIcon.CopyFromBitmap(wxBitmap(wxImage(iconInput)));
	
	wxFileSystem::AddHandler(new wxArchiveFSHandler);
// 	wxFileSystem::AddHandler(new wxMemoryFSHandler);
	
	if (!InitAppSettings())
	{
		wxLogError(_("Failed to initialize settings."));
		return false;
	}
	
	if (!settings.GetInstDir().DirExists())
	{
		settings.GetInstDir().Mkdir();
	}
	
	MainWindow *mainWin = new MainWindow();
	mainWin->Show();

	return true;
}

void MultiMC::OnInitCmdLine(wxCmdLineParser &parser)
{
	wxCmdLineEntryDesc updateOption;
	updateOption.kind = wxCMD_LINE_OPTION;
	updateOption.type = wxCMD_LINE_VAL_STRING;
	updateOption.shortName = _("u");
	updateOption.longName = _("update");
	updateOption.description = _("Used by the update system. Causes the running executable to replace the given file with itself, run it, and exit.");
	
	wxCmdLineEntryDesc descriptions[] = 
	{
		updateOption
	};
	parser.SetDesc(descriptions);
	parser.Parse();
	
	if (parser.Found(_("u")))
	{
		wxString fileToUpdate;
		parser.Found(_("u"), &fileToUpdate);
		startNormally = false;
		InstallUpdate(wxFileName(wxStandardPaths::Get().GetExecutablePath()), 
			wxFileName(fileToUpdate));
	}
}

void MultiMC::InstallUpdate(wxFileName thisFile, wxFileName targetFile)
{
	// Let the other process exit.
	wxSleep(3);
	
	printf("Installing updates...");
	wxCopyFile(thisFile.GetFullPath(), targetFile.GetFullPath());
	
	targetFile.MakeAbsolute();
	
	wxProcess proc;
	wxExecute(targetFile.GetFullPath(), wxEXEC_ASYNC, &proc);
	proc.Detach();
}

int MultiMC::OnExit()
{
	if (updateOnExit && wxFileExists(_("MultiMCUpdate")))
	{
		wxFileName updateFile(Path::Combine(wxGetCwd(), _("MultiMCUpdate")));
		if (IS_LINUX())
		{
			wxExecute(_("chmod +x ") + updateFile.GetFullPath());
		}
		
		wxProcess proc;
		wxString launchCmd = updateFile.GetFullPath() + _(" -u:") + wxStandardPaths::Get().GetExecutablePath();
		wxExecute(launchCmd, wxEXEC_ASYNC, &proc);
		proc.Detach();
	}
	
	return wxApp::OnExit();
}

void MultiMC::OnFatalException()
{
	wxMessageBox(_("A fatal error has occurred and MultiMC has to exit. Sorry for the inconvenience."), 
		_("Fatal Error"));
}

const wxIcon &MultiMC::GetAppIcon() const
{
	return AppIcon;
}
