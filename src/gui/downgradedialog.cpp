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

#include "downgradedialog.h"

#include <wx/gbsizer.h>
#include <wx/hyperlink.h>

#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>

#include <string>
#include <sstream>

#include "apputils.h"
#include "httputils.h"

const wxString mcnwebURL = _("http://sonicrules.org/mcnweb.py");

inline void SetControlEnable(wxWindow *parentWin, int id, bool state)
{
	wxWindow *win = parentWin->FindWindowById(id);
	if (win) win->Enable(state);
}

DowngradeDialog::DowngradeDialog(wxWindow *parent)
	: wxDialog(parent, wxID_ANY, _("Downgrade Instance"), wxDefaultPosition, wxSize(400, 420))
{
	wxFont titleFont(12, wxSWISS, wxNORMAL, wxNORMAL);

	wxBoxSizer *dlgSizer = new wxBoxSizer(wxVERTICAL);
	SetSizer(dlgSizer);

	wxPanel *mainPanel = new wxPanel(this);
	dlgSizer->Add(mainPanel, wxSizerFlags(1).Expand().Border(wxALL, 8));
	wxGridBagSizer *mainSz = new wxGridBagSizer();
	mainSz->AddGrowableCol(0, 0);
	mainSz->AddGrowableRow(1, 0);
	mainPanel->SetSizer(mainSz);

	wxStaticText *versionPageTitle = new wxStaticText(mainPanel, -1, _("Choose Version"));
	versionPageTitle->SetFont(titleFont);
	mainSz->Add(versionPageTitle, wxGBPosition(0, 0), wxGBSpan(1, 2), wxALL | wxALIGN_CENTER, 4);

	versionList = new wxListBox(mainPanel, -1, wxDefaultPosition, wxDefaultSize,
		wxArrayString(), wxLB_SINGLE);
	mainSz->Add(versionList, wxGBPosition(1, 0), wxGBSpan(1, 2), wxEXPAND | wxALL, 4);

	wxHyperlinkCtrl *mcnLink = new wxHyperlinkCtrl(mainPanel, -1, _("Powered by MCNostalgia"),
		_("http://www.minecraftforum.net/topic/800346-"));
	mainSz->Add(mcnLink, wxGBPosition(2, 0), wxGBSpan(1, 1), wxALL | wxALIGN_CENTER_VERTICAL, 4);

	wxButton *versionRefreshBtn = new wxButton(mainPanel, ID_RefreshVersionList, _("&Refresh"));
	mainSz->Add(versionRefreshBtn, wxGBPosition(2, 1), wxGBSpan(1, 1), wxALL, 3);

	wxSizer *btnSz = CreateButtonSizer(wxOK | wxCANCEL);
	dlgSizer->Add(btnSz, wxSizerFlags(0).Border(wxBOTTOM | wxRIGHT, 8).
		Align(wxALIGN_RIGHT | wxALIGN_BOTTOM));

	SetControlEnable(this, wxID_OK, false);

	LoadVersionList();
}

void DowngradeDialog::LoadVersionList()
{
	wxString vlistJSON;
	if (DownloadString(mcnwebURL + _("?pversion=1&list=True"), &vlistJSON))
	{
		using namespace boost::property_tree;

		try
		{
			wxArrayString vList;

			// Parse the JSON
			ptree pt;
			std::stringstream jsonStream(stdStr(vlistJSON), std::ios::in);
			read_json(jsonStream, pt);

			BOOST_FOREACH(const ptree::value_type& v, pt.get_child("order"))
				vList.Insert(wxStr(v.second.data()), 0);

			versionList->Set(vList);
		}
		catch (json_parser_error e)
		{
			wxLogError(_("Failed to read version list.\nJSON parser error at line %i: %s"), 
				e.line(), wxStr(e.message()).c_str());
			return;
		}
	}
	else
	{
		wxLogError(_("Failed to get version list. Check your internet connection and try again later."));
	}

	UpdateOKBtn();
}

void DowngradeDialog::OnRefreshVListClicked(wxCommandEvent& event)
{
	LoadVersionList();
}

void DowngradeDialog::OnListBoxSelChange(wxCommandEvent& event)
{
	UpdateOKBtn();
}

void DowngradeDialog::UpdateOKBtn()
{
	SetControlEnable(this, wxID_OK, versionList->GetSelection() != wxNOT_FOUND);
}

wxString DowngradeDialog::GetSelectedVersion()
{
	if (versionList->GetSelection() != wxNOT_FOUND)
		return versionList->GetStringSelection();
	else
		return wxEmptyString;
}

BEGIN_EVENT_TABLE(DowngradeDialog, wxDialog)
	EVT_BUTTON(ID_RefreshVersionList, DowngradeDialog::OnRefreshVListClicked)
	EVT_LISTBOX(-1, DowngradeDialog::OnListBoxSelChange)
END_EVENT_TABLE()
