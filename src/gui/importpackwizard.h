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
#include <wx/wizard.h>

#include "instance.h"
#include "configpack.h"

class MainWindow;

class ImportPackWizard : public wxWizard
{
public:
	ImportPackWizard(MainWindow *parent, ConfigPack *pack);

	bool Start();

protected:
	MainWindow *m_mainWin;

	void UpdateMissingModList();
	void UpdateMissingModList(wxCommandEvent& event);

	void ViewFolderClicked(wxCommandEvent& event);

	ConfigPack *m_pack;

	wxWizardPageSimple *packInfoPage;
	wxWizardPageSimple *findModFilesPage;

	wxTextCtrl *packNotesTextbox;

	wxListBox *missingModsList;

	DECLARE_EVENT_TABLE()
};

enum
{
	ID_RefreshList,
	ID_ViewCentralModsFolder
};
