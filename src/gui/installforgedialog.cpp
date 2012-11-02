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

#include "installforgedialog.h"

#include <wx/gbsizer.h>
#include <wx/regex.h>

#include "apputils.h"
#include "httputils.h"
#include "forgeversions.h"

IMPLEMENT_DYNAMIC_CLASS(ForgeListCtrl, wxListCtrl)

void ForgeListCtrl::OnSize ( wxSizeEvent& event )
{
	int w, h;

	event.Skip();
	
	int x,y;
	wxWindow::GetClientSize(&x, &y);
	
	int mcvwidth = GetColumnWidth(1);
	int forgewidth = x - mcvwidth;
	if(forgewidth < 0)
		forgewidth = 0;
	SetColumnWidth(0,forgewidth);
	
}
void ForgeListCtrl::SetupColumns()
{
	int x,y;
	wxWindow::GetClientSize(&x, &y);
	SetColumnWidth(1,wxLIST_AUTOSIZE_USEHEADER);
	int mcvwidth = GetColumnWidth(1);
	SetColumnWidth(0,x-mcvwidth);
}

BEGIN_EVENT_TABLE( ForgeListCtrl, wxListCtrl )
	EVT_SIZE( ForgeListCtrl::OnSize )
END_EVENT_TABLE()

enum
{
	ID_RefreshList,
};

inline void SetControlEnable(wxWindow *parentWin, int id, bool state)
{
	wxWindow *win = parentWin->FindWindowById(id);
	if (win) win->Enable(state);
}

InstallForgeDialog::InstallForgeDialog(wxWindow *parent)
	: wxDialog(parent, wxID_ANY, _("Install Minecraft Forge"), wxDefaultPosition, wxSize(400, 420))
{
	wxFont titleFont(12, wxSWISS, wxNORMAL, wxNORMAL);

	wxBoxSizer *dlgSizer = new wxBoxSizer(wxVERTICAL);
	SetSizer(dlgSizer);
	wxStaticText *pageTitle = new wxStaticText(this, -1, _("Choose a Version"));
	pageTitle->SetFont(titleFont);
	dlgSizer->Add(pageTitle, 0, wxALL | wxALIGN_CENTER, 4);
	
	buildList = new ForgeListCtrl(this);
	buildList->AppendColumn("Forge file");
	buildList->AppendColumn("MC version",wxLIST_FORMAT_RIGHT);
	dlgSizer->Add(buildList, 1, wxEXPAND | wxALL, 4);
	
	wxSizer *btnSz = CreateButtonSizer(wxOK | wxCANCEL);
	wxButton *refreshBtn = new wxButton(this, ID_RefreshList, _("&Refresh"));
	btnSz->Insert(0,refreshBtn);
	btnSz->InsertStretchSpacer(1);
	dlgSizer->Add(btnSz, wxSizerFlags(0).Border(wxBOTTOM | wxRIGHT | wxLEFT, 8).Expand());
/*
	
	mainSz->Add(refreshBtn, wxGBPosition(2, 1), wxGBSpan(1, 1), wxALL, 3);

*/
	SetControlEnable(this, wxID_OK, false);
	LoadBuildList();
	buildList->SetupColumns();
}

void InstallForgeDialog::LoadBuildList()
{
	wxString dlURL = "http://files.minecraftforge.net";

	wxString buildListText;
	if (DownloadString(dlURL, &buildListText))
	{
		buildList->DeleteAllItems();
		wxArrayString buildArrayStr;
		wxRegEx forgeRegex;
		if (!forgeRegex.Compile("minecraftforge-(universal|client)-([0-9]+.[0-9]+.[0-9]+.[0-9]+).zip"))
		{
			wxLogError("Regex failed to compile.");
			return;
		}

		while (forgeRegex.Matches(buildListText))
		{
			size_t start, len;
			forgeRegex.GetMatch(&start, &len);

			wxString fileName = buildListText.Mid(start, len);
			if(buildArrayStr.empty() || buildArrayStr.Last() != fileName)
			{
				buildArrayStr.push_back(fileName);
			}
			forgeRegex.ReplaceFirst(&buildListText, wxEmptyString);
		}
		int idx = 0;
		for(auto iter = buildArrayStr.begin(); iter != buildArrayStr.end(); iter++)
		{
			wxString & current = (*iter);
			
			wxString MCVer = forgeutils::MCVersionFromForgeFilename(current);
			buildList->InsertItem(idx,current);
			buildList->SetItem(idx,1,MCVer);
			idx++;
		}
	}
	else
	{
		wxLogError(_("Failed to load build list. Check your internet connection."));
	}

	UpdateOKBtn();
}

void InstallForgeDialog::OnRefreshListClicked(wxCommandEvent& event)
{
	LoadBuildList();
}

void InstallForgeDialog::OnListBoxSelChange(wxListEvent& event)
{
	UpdateOKBtn();
}

void InstallForgeDialog::UpdateOKBtn()
{
	SetControlEnable(this, wxID_OK, buildList->GetSelectedItemCount() != 0);
}

wxString InstallForgeDialog::GetSelectedBuild()
{
	if (buildList->GetSelectedItemCount() != 0)
	{
		int item = buildList->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
        if ( item == -1 )
            return wxEmptyString;

		return buildList->GetItemText(item,0);
	}
	else
		return wxEmptyString;
}

BEGIN_EVENT_TABLE(InstallForgeDialog, wxDialog)
	EVT_BUTTON(ID_RefreshList, InstallForgeDialog::OnRefreshListClicked)
	EVT_LIST_ITEM_SELECTED(-1, InstallForgeDialog::OnListBoxSelChange)
	EVT_LIST_ITEM_DESELECTED(-1, InstallForgeDialog::OnListBoxSelChange)
END_EVENT_TABLE()
