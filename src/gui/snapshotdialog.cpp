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

#include "snapshotdialog.h"
#include "taskprogressdialog.h"
#include "mcversionlist.h"
#include <lambdatask.h>
#include <wx/dcclient.h>

const wxString typeNames[4] = 
{
	_("Old snapshot"),
	_("Old version"),
	_("Current version"),
	_("Snapshot")
};

SnapshotDialog::SnapshotDialog(wxWindow *parent)
	: ListSelectDialog(parent, _("Select Minecraft version"))
{
	// data setup
	showOldSnapshots = false;
	
	// Custom GUI stuff.
	wxCheckBox *snapshotShow = new wxCheckBox(this, 5555, _("Show old snapshots"));
	snapshotShow->SetValue(showOldSnapshots);
	auto cnt = dlgSizer->GetItemCount();
	dlgSizer->Insert(cnt-1,snapshotShow,0, wxALL | wxEXPAND | wxALIGN_CENTER_VERTICAL, 4);
	
	
	wxClientDC dc(this);
	dc.SetFont(listCtrl->GetFont());
	int w,h;
	int maxWidth = 0;
	typeColumnWidth = 120;

	for(int i = 0; i < 4; i++)
	{
		dc.GetTextExtent(typeNames[i], & w, & h);
		if(w > maxWidth)
			maxWidth = w;
	}
	typeColumnWidth = maxWidth + 10;
	
	// Clear columns and add our own.
	listCtrl->DeleteAllColumns();
	listCtrl->AppendColumn(_("Minecraft Version"), wxLIST_FORMAT_LEFT);
	listCtrl->AppendColumn(_("Type"), wxLIST_FORMAT_RIGHT, typeColumnWidth);

	// Show column headers
	ShowHeader(true);
}

void SnapshotDialog::LoadList()
{
	MCVersionList & verList = MCVersionList::Instance();

	bool success = false;
	
	if(verList.NeedsLoad())
	{
		LambdaTask::TaskFunc func = [&] (LambdaTask *task) -> wxThread::ExitCode
		{
			task->DoSetStatus(_("Loading Minecraft version list..."));
			return (wxThread::ExitCode)DoLoadList();
		};

		LambdaTask *lTask = new LambdaTask(func);
		TaskProgressDialog taskDlg(this);
		success = taskDlg.ShowModal(lTask);
		delete lTask;
	}
	else
	{
		success = true;
	}
	
	Refilter();
}

bool SnapshotDialog::DoLoadList()
{
	MCVersionList & verList = MCVersionList::Instance();
	return verList.LoadIfNeeded();
}

void SnapshotDialog::Refilter()
{
	visibleIndexes.clear();
	MCVersionList & verList = MCVersionList::Instance();
	for(unsigned i = 0; i < verList.versions.size(); i++ )
	{
		MCVersion & ver = verList.versions[i];
		if(showOldSnapshots || ver.type != OldSnapshot)
			visibleIndexes.push_back(i);
	}
	listCtrl->SetItemCount(visibleIndexes.size());
	// we can use that value because the indexes do correspond up to the current stable version
	listCtrl->SetItemState(verList.stableVersionIndex,wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
	listCtrl->SetColumnWidth(1,typeColumnWidth);
	SetupColumnSizes();
	listCtrl->Refresh();
	listCtrl->Update();
	UpdateOKBtn();
}

wxString SnapshotDialog::OnGetItemText(long item, long column)
{
	MCVersionList & verList = MCVersionList::Instance();
	MCVersion & ver = verList.versions[visibleIndexes[item]];

	switch (column)
	{
	case 0:
		return ver.name;
	case 1:
		return typeNames[ver.type];

	default:
		return "...";
	}
}

bool SnapshotDialog::GetSelectedVersion ( MCVersion& out )
{
	MCVersionList & verList = MCVersionList::Instance();
	int idx = GetSelectedIndex();
	if(idx == -1)
		return false;
	out = verList.versions[visibleIndexes[idx]];
	return true;
}


void SnapshotDialog::OnCheckbox(wxCommandEvent& event)
{
	showOldSnapshots = event.IsChecked();
	Refilter();
}

BEGIN_EVENT_TABLE(SnapshotDialog, ListSelectDialog)
	EVT_CHECKBOX(5555, SnapshotDialog::OnCheckbox)
END_EVENT_TABLE()