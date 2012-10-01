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

#pragma once
#include <wx/string.h>
#include <wx/frame.h>
#include <wx/listctrl.h>
#include <wx/dnd.h>
#include <instance.h>

class MainWindow;

class SaveMgrWindow : public wxFrame
{
public:
	SaveMgrWindow(MainWindow *parent, Instance *inst);
	
	class SaveListCtrl : public wxListCtrl
	{
	public:
		SaveListCtrl(wxWindow *parent, Instance *inst);

		virtual wxString OnGetItemText(long item, long column) const;

		void UpdateListItems();

		World *GetSelectedSave();

	protected:
		Instance *m_inst;

	} *saveList;
	
protected:	
	MainWindow *m_mainWin;
	Instance *m_inst;

	void OnCloseClicked(wxCommandEvent& event);
	void OnViewFolderClicked(wxCommandEvent& event);

	// Exports selected save as a zip file.
	wxButton *exportZip;

	void OnExportZipClicked(wxCommandEvent& event);

	void OnSelChanged(wxListEvent &event);

	void EnableSideButtons(bool enable = true);
	
	DECLARE_EVENT_TABLE()
};
