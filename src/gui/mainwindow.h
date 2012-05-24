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

#pragma once
#include <wx/wx.h>

#include <map>

#include "task.h"
#include "logintask.h"

#include "insticonlist.h"

#include "settingsdialog.h"

//const wxString tbarIconPrefix = _T("resources/toolbar/");

class MainWindow : public wxFrame
{
public:
	MainWindow(void);
	~MainWindow(void);

	// Toolbar
	void OnAddInstClicked(wxCommandEvent& event);
	void OnViewFolderClicked(wxCommandEvent& event);
	void OnRefreshClicked(wxCommandEvent& event);

	void OnSettingsClicked(wxCommandEvent& event);
	void OnCheckUpdateClicked(wxCommandEvent& event);

	void OnHelpClicked(wxCommandEvent& event);
	void OnAboutClicked(wxCommandEvent& event);


	// Instance menu
	void OnPlayClicked(wxCommandEvent& event);
	
	void OnRenameClicked(wxCommandEvent& event);
	void OnChangeIconClicked(wxCommandEvent& event);
	void OnNotesClicked(wxCommandEvent& event);

	void OnManageSavesClicked(wxCommandEvent& event);
	void OnEditModsClicked(wxCommandEvent& event);
	void OnRebuildJarClicked(wxCommandEvent& event);
	void OnViewInstFolderClicked(wxCommandEvent& event);
	
	void OnDeleteClicked(wxCommandEvent& event);
	
	
	// Task Events
	void OnTaskStart(TaskEvent &event);
	void OnTaskEnd(TaskEvent &event);
	void OnTaskProgress(TaskProgressEvent &event);
	void OnTaskStatus(TaskStatusEvent &event);
	void OnTaskError(TaskErrorEvent &event);
	
	void OnLoginComplete(LoginCompleteEvent &event);
	
	
	// Other events
	void OnInstMenuOpened(wxListEvent& event);
	
	
	// Other methods
	void StartTask(Task &task);
	void StartModalTask(Task &task, bool forceModal = true);
	
	void ShowLoginDlg(wxString errorMsg);
	
	
	DECLARE_EVENT_TABLE()

protected:
	wxListCtrl *instListCtrl;

	wxMenu *instMenu;
	
	bool modalTaskRunning;

	InstIconList instIcons;
	void LoadInstIconList(wxString customIconDirName = _T("icons"));

	void LoadInstanceList(wxFileName instDir = settings.GetInstDir());

	void AddInstance(Instance *inst);

	Instance* GetLinkedInst(long item);

	Instance* GetSelectedInst();

	std::map<long, Instance*> instItems;

private:
	void NotImplemented();
};

enum
{
	// Toolbar
	ID_AddInst = 1,
	ID_ViewFolder,
	ID_ModsFolder,
	ID_Refresh,

	ID_Settings,
	ID_CheckUpdate,

	ID_Help,
	ID_About,

	// Instance menu
	ID_Play,

	ID_Rename,
	ID_ChangeIcon,
	ID_Notes,

	ID_ManageSaves,
	ID_EditMods,
	ID_RebuildJar,
	ID_ViewInstFolder,

	ID_DeleteInst,

	// Other
	ID_InstListCtrl,
};

BEGIN_EVENT_TABLE(MainWindow, wxFrame)
	EVT_TOOL(ID_AddInst, MainWindow::OnAddInstClicked)
	EVT_TOOL(ID_ViewFolder, MainWindow::OnViewFolderClicked)
	EVT_TOOL(ID_Refresh, MainWindow::OnRefreshClicked)

	EVT_TOOL(ID_Settings, MainWindow::OnSettingsClicked)
	EVT_TOOL(ID_CheckUpdate, MainWindow::OnCheckUpdateClicked)

	EVT_TOOL(ID_Help, MainWindow::OnHelpClicked)
	EVT_TOOL(ID_About, MainWindow::OnAboutClicked)


	EVT_MENU(ID_Play, MainWindow::OnPlayClicked)

	EVT_MENU(ID_Rename, MainWindow::OnRenameClicked)
	EVT_MENU(ID_ChangeIcon, MainWindow::OnChangeIconClicked)
	EVT_MENU(ID_Notes, MainWindow::OnNotesClicked)

	EVT_MENU(ID_ManageSaves, MainWindow::OnManageSavesClicked)
	EVT_MENU(ID_EditMods, MainWindow::OnEditModsClicked)
	EVT_MENU(ID_RebuildJar, MainWindow::OnRebuildJarClicked)
	EVT_MENU(ID_ViewInstFolder, MainWindow::OnViewInstFolderClicked)

	EVT_MENU(ID_DeleteInst, MainWindow::OnDeleteClicked)

	EVT_LIST_ITEM_RIGHT_CLICK(ID_InstListCtrl, MainWindow::OnInstMenuOpened)
	
	EVT_TASK_START(MainWindow::OnTaskStart)
	EVT_TASK_END(MainWindow::OnTaskEnd)
	EVT_TASK_STATUS(MainWindow::OnTaskStatus)
	EVT_TASK_PROGRESS(MainWindow::OnTaskProgress)
	EVT_TASK_ERRORMSG(MainWindow::OnTaskError)
	
	EVT_LOGIN_COMPLETE(MainWindow::OnLoginComplete)
END_EVENT_TABLE()

class MultiMC : public wxApp
{
public:
	virtual bool OnInit();
	virtual void OnFatalException();
};
