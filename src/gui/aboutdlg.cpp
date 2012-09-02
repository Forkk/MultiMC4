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

#include "aboutdlg.h"

#include <wx/gbsizer.h>
#include <wx/stattext.h>
#include <wx/statbmp.h>
#include <wx/hyperlink.h>
#include <wx/button.h>
#include <wx/textdlg.h>
#include <wx/msgdlg.h>

AboutDlg::AboutDlg(wxWindow *parent, const wxAboutDialogInfo &info)
	: wxDialog(parent, -1, _("About MultiMC"), wxDefaultPosition, wxSize(600, 180)),
		m_licenseText(info.GetLicence())
{
	wxWindow *mainPanel = this;
	wxGridBagSizer *mainSz = new wxGridBagSizer();
	mainSz->AddGrowableCol(1);
	mainPanel->SetSizer(mainSz);

	#define GB_CenterFlags wxLEFT | wxRIGHT | wxTOP | wxALIGN_CENTER, 8
	#define GB_IconFlags wxALL | wxALIGN_LEFT | wxALIGN_TOP, 8

	wxFont titleFont(12, wxSWISS, wxNORMAL, wxBOLD);

	const int cols = 3;
	int rows = 0;

	wxStaticText *titleLabel = new wxStaticText(mainPanel, -1, 
		wxString::Format(_("%s %s"), info.GetName().c_str(), info.GetVersion().c_str()));
	titleLabel->SetFont(titleFont);
	mainSz->Add(titleLabel, wxGBPosition(rows++, 1), wxGBSpan(1, cols - 1), GB_CenterFlags);

	wxStaticText *copyrightLabel = new wxStaticText(mainPanel, -1,
		info.GetCopyrightToDisplay());
	mainSz->Add(copyrightLabel, wxGBPosition(rows++, 1), wxGBSpan(1, cols - 1), GB_CenterFlags);

	wxStaticText *descLabel = new wxStaticText(mainPanel, -1, 
		info.GetDescription(), wxDefaultPosition, wxDefaultSize, 
		wxALIGN_CENTRE | wxST_NO_AUTORESIZE);
	mainSz->Add(descLabel, wxGBPosition(rows++, 1), wxGBSpan(1, cols - 1), GB_CenterFlags);
	descLabel->Wrap(GetClientSize().GetWidth() - 100);

	wxHyperlinkCtrl *homepageLink = new wxHyperlinkCtrl(mainPanel, -1, 
		info.GetWebSiteURL(), info.GetWebSiteURL());
	mainSz->Add(homepageLink, wxGBPosition(rows++, 1), wxGBSpan(1, cols - 1), GB_CenterFlags);

	mainSz->AddGrowableRow(rows++);

	wxBoxSizer *btnSz = new wxBoxSizer(wxHORIZONTAL);
	mainSz->Add(btnSz, wxGBPosition(rows, 2), wxGBSpan(1, 1), 
		wxALL | wxALIGN_RIGHT | wxALIGN_BOTTOM, 4);

	auto btnFlags = wxSizerFlags().Border(wxALL, 4);

	wxButton *licenseButton = new wxButton(mainPanel, ID_License, _("&Licenses"));
	btnSz->Add(licenseButton, btnFlags);

	wxButton *closeButton = new wxButton(mainPanel, wxID_CANCEL, _("&Close"));
	btnSz->Add(closeButton, btnFlags);

	wxStaticBitmap *iconCtrl = new wxStaticBitmap(mainPanel, -1, info.GetIcon());
	mainSz->Add(iconCtrl, wxGBPosition(0, 0), wxGBSpan(rows, 1), GB_IconFlags);
}

void AboutDlg::OnViewLicense(wxCommandEvent& event)
{
	wxMessageBox(m_licenseText);
}

BEGIN_EVENT_TABLE(AboutDlg, wxDialog)
	EVT_BUTTON(ID_License, AboutDlg::OnViewLicense)
END_EVENT_TABLE()
