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

#include "minecraftversiondialog.h"
#include "taskprogressdialog.h"
#include "mcversionlist.h"
#include <lambdatask.h>
#include <wx/hyperlink.h>

const wxString typeNames[5] = 
{
	_("Old snapshot"),
	_("Old version"),
	_("Current version"),
	_("Snapshot"),
	_("MCNostalgia")
};

MinecraftVersionDialog::MinecraftVersionDialog(wxWindow *parent)
	: ListSelectDialog(parent, _("Select Minecraft version"))
{
	// data setup
	showOldSnapshots = false;
	showMCNostagia = false;
	
	// Custom GUI stuff.
	auto bSizer2 = new wxBoxSizer( wxHORIZONTAL );
	{
		wxCheckBox *snapshotShow = new wxCheckBox(this, 5555, _("Show old snapshots"));
		snapshotShow->SetValue(showOldSnapshots);
		bSizer2->Add(snapshotShow,0, wxALL | wxEXPAND | wxALIGN_CENTER_VERTICAL, 4);
		
		wxCheckBox *nostalgiaShow = new wxCheckBox(this, 5556, _("Show MCNostalgia versions"));
		nostalgiaShow->SetValue(showMCNostagia);
		bSizer2->Add(nostalgiaShow,0, wxALL | wxEXPAND | wxALIGN_CENTER_VERTICAL, 4);
	}
	auto cnt = dlgSizer->GetItemCount();
	dlgSizer->Insert(cnt-1, bSizer2, 0, wxEXPAND, 0 );
	
	wxHyperlinkCtrl *mcnLink = new wxHyperlinkCtrl(this, -1, _("Powered by MCNostalgia"),
	"http://www.minecraftforum.net/topic/800346-");
	dlgSizer->Insert(cnt, mcnLink,0, wxALL | wxEXPAND | wxALIGN_CENTER_VERTICAL, 4);
	
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

void MinecraftVersionDialog::LoadList()
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
		success = taskDlg.ShowModal(lTask) > 0;
		delete lTask;
	}
	else
	{
		success = true;
	}
	
	Refilter();
}

bool MinecraftVersionDialog::DoLoadList()
{
	MCVersionList & verList = MCVersionList::Instance();
	return verList.LoadIfNeeded();
}

void MinecraftVersionDialog::Refilter()
{
	visibleIndexes.clear();
	MCVersionList & verList = MCVersionList::Instance();
	for(unsigned i = 0; i < verList.size(); i++ )
	{
		const MCVersion & ver = verList[i];
		VersionType type = ver.GetVersionType();
		if(type == OldSnapshot && showOldSnapshots
			|| type == MCNostalgia && showMCNostagia
			|| type != OldSnapshot && type != MCNostalgia)
			visibleIndexes.push_back(i);
	}
	
	listCtrl->SetItemCount(visibleIndexes.size());
	// we can use that value because the indexes do correspond up to the current stable version
	listCtrl->SetItemState(verList.GetStableVersionIndex(),wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
	listCtrl->SetColumnWidth(1,typeColumnWidth);
	SetupColumnSizes();
	listCtrl->Refresh();
	listCtrl->Update();
	OnSelectionChange();
}

wxString MinecraftVersionDialog::OnGetItemText(long item, long column)
{
	MCVersionList & verList = MCVersionList::Instance();
	const MCVersion & ver = verList[visibleIndexes[item]];

	switch (column)
	{
	case 0:
		return ver.GetName();
	case 1:
		return typeNames[ver.GetVersionType()];

	default:
		return "...";
	}
}

MCVersion* MinecraftVersionDialog::GetSelectedVersion ( )
{
	MCVersionList & verList = MCVersionList::Instance();
	int idx = GetSelectedIndex();
	if(idx == -1)
		return nullptr;
	return & verList[visibleIndexes[idx]];
}


void MinecraftVersionDialog::OnSnapshots(wxCommandEvent& event)
{
	showOldSnapshots = event.IsChecked();
	Refilter();
}

void MinecraftVersionDialog::OnNostalgia ( wxCommandEvent& event )
{
	showMCNostagia = event.IsChecked();
	if(showMCNostagia)
	{
		MCVersionList & verList = MCVersionList::Instance();
		verList.SetNeedsNostalgia();
		if(verList.NeedsLoad())
		{
			LambdaTask::TaskFunc func = [&] (LambdaTask *task) -> wxThread::ExitCode
			{
				task->DoSetStatus(_("Loading MCNostalgia version list..."));
				return (wxThread::ExitCode) verList.LoadNostalgia();
			};

			LambdaTask *lTask = new LambdaTask(func);
			TaskProgressDialog taskDlg(this);
			taskDlg.ShowModal(lTask);
			delete lTask;
		}
	}
	Refilter();
}


BEGIN_EVENT_TABLE(MinecraftVersionDialog, ListSelectDialog)
	EVT_CHECKBOX(5555, MinecraftVersionDialog::OnSnapshots)
	EVT_CHECKBOX(5556, MinecraftVersionDialog::OnNostalgia)
END_EVENT_TABLE()