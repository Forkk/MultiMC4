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
#include "checkupdatetask.h"

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
	void OnInstActivated(wxListEvent& event);
	
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
	
	void OnCheckUpdateComplete(CheckUpdateEvent &event);
	
	// Other events
	void OnInstMenuOpened(wxListEvent& event);
	
	
	// Other methods
	void StartTask(Task &task);
	bool StartModalTask(Task &task, bool forceModal = true);
	
	void ShowLoginDlg(wxString errorMsg);
	
	
	DECLARE_EVENT_TABLE()

protected:
	wxListCtrl *instListCtrl;
	
	wxMenu *instMenu;
	
	bool modalTaskRunning;
	
	bool canLaunch;
	
	bool closeOnTaskEnd;

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

