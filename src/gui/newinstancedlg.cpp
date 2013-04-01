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

#include "newinstancedlg.h"
#include "taskprogressdialog.h"
#include "mcversionlist.h"
#include <insticonlist.h>
#include "changeicondialog.h"
#include <utils/fsutils.h>
#include <lambdatask.h>
#include "shadedtextedit.h"
#include "minecraftversiondialog.h"
#include <wx/hyperlink.h>

#include "utils/apputils.h"

bool NewInstanceDialog::Create()
{
	SetAprilFonts(this);

	m_loadingDone = false;
	m_iconKey = wxEmptyString;
	m_visibleIconKey = wxEmptyString;
	m_selectedVersion = nullptr;
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer( wxVERTICAL );
	
	m_btnIcon = new wxButton(this, ID_icon_select, wxEmptyString, wxDefaultPosition, wxDefaultSize);
	bSizer1->Add( m_btnIcon, 0,wxALIGN_CENTER_HORIZONTAL | wxALL, 5 );
	
	// Because ponies.
	if (m_username.Lower().Contains("rootbear75"))
	{
		SetIconKey("derp");
	}
	else
	{
		SetIconKey("default");
	}

	
	m_textName = new ShadedTextEdit( this, _("Instance name here"), ID_text_name, wxTE_CENTRE);
	m_textName->SetMaxLength( 25 ); 
	bSizer1->Add( m_textName, 0, wxALL|wxEXPAND, 5 );
	
	auto staticline0 = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	bSizer1->Add( staticline0, 0, wxEXPAND | wxALL, 5 );

	
	
	auto bSizer2 = new wxBoxSizer( wxHORIZONTAL );
	{
		auto staticText2 = new wxStaticText( this, wxID_ANY, wxT("Minecraft version:"), wxDefaultPosition, wxDefaultSize, 0 );
		//staticText2->Wrap( -1 );
		bSizer2->Add( staticText2, 0, wxALL| wxALIGN_CENTER_VERTICAL, 5 );
		
		m_versionDisplay = new wxTextCtrl( this, ID_text_version, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_CENTRE | wxTE_READONLY );
		bSizer2->Add( m_versionDisplay, 1, wxALL| wxEXPAND, 5 );
	}
	bSizer1->Add( bSizer2, 0, wxEXPAND, 5 );
	
	m_changeVersionButton = new wxButton(this, ID_btn_version, _("Change version"));
	bSizer1->Add( m_changeVersionButton, 0, wxEXPAND | wxALL, 5 );
	
	auto staticline2 = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	bSizer1->Add( staticline2, 0, wxEXPAND | wxALL, 5 );
	
	bSizer1->AddStretchSpacer();
	
	auto btnSizer = CreateButtonSizer(wxOK | wxCANCEL);
	m_btnOK = (wxButton *)FindWindowById(wxID_OK, this);
	m_btnOK->Enable(false);
	
	bSizer1->Add(btnSizer, wxSizerFlags(0).Border(wxBOTTOM | wxRIGHT, 8).Align(wxALIGN_RIGHT | wxALIGN_BOTTOM));
	
	this->Layout();
	this->SetSizerAndFit(bSizer1);
	
	this->Center( wxBOTH );
	m_loadingDone = true;
	return true;
}

int NewInstanceDialog::ShowModal()
{
	MCVersionList & verList = MCVersionList::Instance();
	if(verList.NeedsLoad())
	{
		LambdaTask::TaskFunc func = [&] (LambdaTask *task) -> wxThread::ExitCode
		{
			task->DoSetStatus(_("Loading version list..."));
			return (wxThread::ExitCode) verList.LoadMojang();
		};

		LambdaTask *lTask = new LambdaTask(func);
		TaskProgressDialog taskDlg(this);
		taskDlg.ShowModal(lTask);
		delete lTask;
	}
	m_selectedVersion = verList.GetCurrentStable();
	/*
	if(!m_selectedVersion && verList.size())
	{
		m_selectedVersion = &verList[0];
	}
	*/
	UpdateVersionDisplay();
	return wxDialog::ShowModal();
}

void NewInstanceDialog::UpdateVersionDisplay()
{
	if(m_selectedVersion)
	{
		m_versionDisplay->SetValue(m_selectedVersion->GetName());
	}
	else
	{
		m_versionDisplay->SetValue(_("Unknown"));
	}
}

void NewInstanceDialog::UpdateOKButton()
{
	wxString folderName;
	if(fsutils::GetValidInstanceFolderName(m_name,folderName) && m_selectedVersion)
	{
		m_btnOK->Enable(true);
	}
	else
	{
		m_btnOK->Enable(false);
	}
}


void NewInstanceDialog::OnName ( wxCommandEvent& event )
{
	if(!m_loadingDone)
		return;
	if(m_textName->IsEmptyUnfocused())
	{
		m_btnOK->Enable(false);
		m_name = wxEmptyString;
		UpdateIcon();
		return;
	}
	m_name = m_textName->GetValue();
	m_name.Strip(wxString::both);
	UpdateOKButton();
	UpdateIcon();
}

void NewInstanceDialog::OnVersion ( wxCommandEvent& event )
{
	bool btn_enabled = m_btnOK->IsEnabled();
	MinecraftVersionDialog versionDlg(this);
	versionDlg.CenterOnParent();
	MCVersion * ver;
	if(versionDlg.ShowModal() != wxID_OK)
	{
		m_btnOK->Enable(btn_enabled);
		return;
	}
	ver = versionDlg.GetSelectedVersion();
	if(!ver)
	{
		m_btnOK->Enable(false);
		return;
	}
	m_selectedVersion = ver;
	UpdateVersionDisplay();
	m_btnOK->Enable(btn_enabled);
}

void NewInstanceDialog::OnIcon ( wxCommandEvent& event )
{
	ChangeIconDialog iconDlg(this);
	iconDlg.CenterOnParent();
	if (iconDlg.ShowModal() == wxID_OK)
	{
		SetIconKey(iconDlg.GetSelectedIconKey());
	}
}

void NewInstanceDialog::SetIconKey ( wxString iconkey )
{
	wxString filteredIconKey = InstIconList::getRealIconKeyForEasterEgg(iconkey, m_name);
	if(m_visibleIconKey != filteredIconKey)
	{
		auto icons = InstIconList::Instance();
		auto image = icons->getImage128ForKey(filteredIconKey);
		m_btnIcon->SetBitmap(wxBitmap(image));
		m_visibleIconKey = filteredIconKey;
	}
	m_iconKey = iconkey;
}

void NewInstanceDialog::UpdateIcon()
{
	SetIconKey(m_iconKey);
}

NewInstanceDialog::~NewInstanceDialog()
{
}

wxString NewInstanceDialog::GetInstanceName()
{
	return m_textName->GetValue().Strip(wxString::both);
}

wxString NewInstanceDialog::GetInstanceIconKey()
{
	return m_iconKey;
}

MCVersion * NewInstanceDialog::GetInstanceMCVersion()
{
	return m_selectedVersion;
}

wxString NewInstanceDialog::GetInstanceMCVersionDescr()
{
	if(m_selectedVersion)
	{
		return m_selectedVersion->GetDescriptor();
	}
	return MCVer_Unknown;
}


wxString NewInstanceDialog::GetInstanceLWJGL()
{
	return "Mojang";
}

BEGIN_EVENT_TABLE(NewInstanceDialog, wxDialog)
	EVT_TEXT(ID_text_name, NewInstanceDialog::OnName)
	EVT_BUTTON(ID_icon_select, NewInstanceDialog::OnIcon)
	EVT_BUTTON(ID_btn_version, NewInstanceDialog::OnVersion)
END_EVENT_TABLE()
