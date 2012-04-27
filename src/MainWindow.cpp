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

#include "MainWindow.h"

#include "ToolbarIcons.h"

IMPLEMENT_APP(MultiMC)

// Main window
MainWindow::MainWindow(void)
	: wxFrame(NULL, -1, _T("MultiMC"), wxPoint(0, 0), wxSize(620, 400))
{
	wxToolBar *mainToolBar = CreateToolBar();

	// Load toolbar icons
	wxInitAllImageHandlers();
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
	instMenu = new wxMenu("Instance Menu");
	instMenu->Append(ID_Play, "&Play", "Launch the instance.");
	instMenu->AppendSeparator();
	instMenu->Append(ID_Rename, "&Rename", "Change the instance's name.");
	instMenu->Append(ID_ChangeIcon, "&Change Icon", "Change this instance's icon.");
	instMenu->Append(ID_Notes, "&Notes", "View / edit this instance's notes.");
	instMenu->AppendSeparator();
	instMenu->Append(ID_ManageSaves, "&Manage Saves", "Backup / restore your saves.");
	instMenu->Append(ID_EditMods, "&Edit Mods", "Install or remove mods.");
	instMenu->Append(ID_RebuildJar, "Re&build Jar", "Reinstall all the instance's jar mods.");
	instMenu->Append(ID_ViewInstFolder, "&View Folder", "Open the instance's folder.");
	instMenu->AppendSeparator();
	instMenu->Append(ID_DeleteInst, "Delete", "Delete this instance.");

	// Create the status bar
	CreateStatusBar(1);

	// Set up the main panel and sizers
	wxPanel *panel = new wxPanel(this, -1);
	wxBoxSizer *box = new wxBoxSizer(wxVERTICAL);
	panel->SetSizer(box);

	// Create the instance list
	instListCtrl = new wxListCtrl(panel, -1);
	box->Add(instListCtrl, 1, wxEXPAND);

	// Load instance icons
	LoadInstIconList();

	// Load instance list
	LoadInstanceList(boost::filesystem::path(_T("instances")));

	CenterOnScreen();
}

MainWindow::~MainWindow(void)
{
	delete instIcons;
}

void MainWindow::LoadInstIconList(wxString customIconDirName)
{
	instIcons = new InstIconList(32, 32);

	instListCtrl->SetImageList(instIcons->GetImageList(), 0);
}

void MainWindow::LoadInstanceList(boost::filesystem::path instDir)
{
	namespace fs = boost::filesystem;

	instListCtrl->ClearAll();
	instIndices.clear();

	fs::directory_iterator endIter;

	if (fs::exists(instDir) && fs::is_directory(instDir))
	{
		for (fs::directory_iterator iter(instDir); iter != endIter; iter++)
		{
			fs::path file = iter->path();

			if (IsValidInstance(file))
			{
				Instance *inst = new Instance(file);
				AddInstance(inst);
			}
		}
	}
}

void MainWindow::AddInstance(Instance *inst)
{
	int instIndex = instListCtrl->GetItemCount();
	instListCtrl->InsertItem(instIndex, inst->GetName(), 
		(*instIcons)[inst->GetIconKey()]);
}

Instance* MainWindow::GetLinkedInst(int index)
{
	return instIndices[index];
}

Instance* MainWindow::GetSelectedInst()
{
	long item = -1;
	while (true)
	{
		item = instListCtrl
	}
}


// Toolbar
void MainWindow::OnAddInstClicked(wxCommandEvent& event)
{
	wxTextEntryDialog newInstDialog(this, "Instance name:", "Add new instance");
	newInstDialog.ShowModal();

	wxString newInstName = newInstDialog.GetValue();
	fs::path instDir = settings.instanceDir / newInstName.c_str();

	Instance *inst = new Instance(instDir, newInstName);
	inst->Save();
}

void MainWindow::OnViewFolderClicked(wxCommandEvent& event)
{
#if defined WIN32
	std::string cmd("explorer ");
	cmd.append(settings.instanceDir.string());
	system(cmd.c_str());
#elif defined LINUX
	std::string cmd("xdg-open ");
	cmd.append(settings.instanceDir.string());
	system(cmd.c_str());
#else
	wxMessageBox("This feature is not supported by your operating system.",
		"Error", wxOK | wxCENTER, this);
#endif
}

void MainWindow::OnRefreshClicked(wxCommandEvent& event)
{
	LoadInstanceList();
}

void MainWindow::OnSettingsClicked(wxCommandEvent& event)
{
	Instance *inst = GetLinkedInst(instListCtrl->getsel	)
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
	wxMessageBox("This feature has not yet been implemented.", "Not implemented");
}


// Instance menu
void MainWindow::OnPlayClicked(wxCommandEvent& event)
{
	NotImplemented();
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


// App
bool MultiMC::OnInit()
{
	if (!InitAppSettings())
		return false;

	if (!fs::exists(settings.instanceDir))
	{
		fs::create_directories(settings.instanceDir);
	}

	MainWindow *mainWin = new MainWindow();
	mainWin->Show();

	return true;
}