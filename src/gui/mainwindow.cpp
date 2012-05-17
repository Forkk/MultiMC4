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

#include "toolbaricons.h"
#include "gameupdatetask.h"
#include "logintask.h"
#include "moddertask.h"

#include <wx/filesys.h>
#include <wx/dir.h>
#include <wx/fs_arc.h>
#include <wx/fs_mem.h>

IMPLEMENT_APP(MultiMC)

const int instNameLengthLimit = 20;

// Main window
MainWindow::MainWindow(void)
	: wxFrame(NULL, -1, _T("MultiMC"), wxPoint(0, 0), wxSize(620, 400))
{
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
	mainToolBar->AddTool(ID_AddInst, _("Add instance"), newInstIcon);
	mainToolBar->AddTool(ID_Refresh, _("Refresh"), reloadIcon);
	mainToolBar->AddTool(ID_ViewFolder, _("View folder"), viewFolderIcon);
	mainToolBar->AddSeparator();
	mainToolBar->AddTool(ID_Settings, _("Settings"), settingsIcon);
	mainToolBar->AddTool(ID_CheckUpdate, _("Check for updates"), checkUpdateIcon);
	mainToolBar->AddSeparator();
	mainToolBar->AddTool(ID_Help, _("Help"), helpIcon);
	mainToolBar->AddTool(ID_About, _("About"), aboutIcon);

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
	//instIcons.

	instListCtrl->SetImageList(instIcons.GetImageList(), 0);
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
	NotImplemented();
}

void MainWindow::OnHelpClicked(wxCommandEvent& event)
{
	NotImplemented();
}

void MainWindow::OnAboutClicked(wxCommandEvent& event)
{
	NotImplemented();
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

void MainWindow::ShowLoginDlg(wxString errorMsg)
{
	LoginDialog loginDialog(this, errorMsg);
	int response = loginDialog.ShowModal();
	
	if (response == wxID_OK)
	{
		UserInfo info(loginDialog);
		LoginTask *task = new LoginTask(info, GetSelectedInst(), loginDialog.ShouldForceUpdate());
		StartModalTask(*task);
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
			StartModalTask(task);
			
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
	NotImplemented();
}

void MainWindow::OnChangeIconClicked(wxCommandEvent& event)
{
	NotImplemented();
}

void MainWindow::OnNotesClicked(wxCommandEvent& event)
{
	NotImplemented();
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
	if (event.m_task->IsModal())
	{
		event.m_task->GetProgressDialog()->Close();
	}
	event.m_task->Dispose();
}

void MainWindow::OnTaskProgress(TaskProgressEvent& event)
{
	if (event.m_task->IsModal())
	{
		event.m_task->GetProgressDialog()->Update(event.m_progress, event.m_task->GetStatus());
		event.m_task->GetProgressDialog()->Fit();
	}
}

void MainWindow::OnTaskStatus(TaskStatusEvent& event)
{
	if (event.m_task->IsModal())
	{
		event.m_task->GetProgressDialog()->Update(event.m_task->GetProgress(), event.m_status);
		event.m_task->GetProgressDialog()->Fit();
	}
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

void MainWindow::StartModalTask(Task& task)
{
	wxProgressDialog *progDialog = new wxProgressDialog(_("Please wait..."), 
														task.GetStatus(), 100, this);
	progDialog->SetMinSize(wxSize(400, 80));
	progDialog->Fit();
	progDialog->CenterOnParent();
	task.SetProgressDialog(progDialog);
	task.SetEvtHandler(this);
	task.Start();
	progDialog->ShowModal();
}


// App
bool MultiMC::OnInit()
{
	SetAppName(_("MultiMC"));
	
	wxInitAllImageHandlers();
	
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

void MultiMC::OnFatalException()
{
	wxMessageBox(_("A fatal error has occurred and MultiMC has to exit. Sorry for the inconvenience."), 
				 _("Fatal Error"));
}
