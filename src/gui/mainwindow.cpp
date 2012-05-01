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

#include "toolbaricons.h"

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
	while (Path::Combine(settings.instanceDir, dirName).DirExists())
	{
		num++;
		dirName = Utils::RemoveInvalidPathChars(newInstName) + wxString::Format(_("_%i"), num);
		
		// If it's over 9000
		if (num > 9000)
		{
			wxLogError(_T("Couldn't create instance folder: %s"),
					   stdStr(Path::Combine(settings.instanceDir, dirName).GetFullPath()).c_str());
			goto Retry;
		}
	}
	wxFileName instDir;
	
	Instance *inst = new Instance(instDir, newInstName);
	inst->Save();
	AddInstance(inst);
}

void MainWindow::OnViewFolderClicked(wxCommandEvent& event)
{
	Utils::OpenFile(settings.instanceDir);
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
	wxString errorMsg = _("");
Retry:
	LoginDialog loginDialog(this, errorMsg);
	loginDialog.ShowModal();
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
	NotImplemented();
}

void MainWindow::OnRebuildJarClicked(wxCommandEvent& event)
{
	NotImplemented();
}

void MainWindow::OnViewInstFolderClicked(wxCommandEvent& event)
{
	NotImplemented();
}

void MainWindow::OnDeleteClicked(wxCommandEvent& event)
{
	NotImplemented();
}

void MainWindow::OnInstMenuOpened(wxListEvent& event)
{
	PopupMenu(instMenu, event.GetPoint());
}


// App
bool MultiMC::OnInit()
{
	wxInitAllImageHandlers();
	
	if (!InitAppSettings())
	{
		wxLogError(_("Failed to initialize settings."));
		return false;
	}
	
	if (!settings.instanceDir.DirExists())
	{
		settings.instanceDir.Mkdir();
	}
	
	MainWindow *mainWin = new MainWindow();
	mainWin->Show();

	return true;
}
