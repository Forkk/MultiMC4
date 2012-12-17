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

bool NewInstanceDialog::Create()
{
	m_loadingDone = false;
	m_iconKey = wxEmptyString;
	m_visibleIconKey = wxEmptyString;
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

	
	m_textName = new ShadedTextEdit( this, _("Instance name"), ID_text_name, wxTE_CENTRE);
	m_textName->SetMaxLength( 25 ); 
	bSizer1->Add( m_textName, 0, wxALL|wxEXPAND, 5 );
	
	auto staticline0 = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	bSizer1->Add( staticline0, 0, wxEXPAND | wxALL, 5 );

	auto staticText2 = new wxStaticText( this, wxID_ANY, wxT("Minecraft version:"), wxDefaultPosition, wxDefaultSize, 0 );
	//staticText2->Wrap( -1 );
	bSizer1->Add( staticText2, 0, wxALL, 5 );
	
	m_choiceMCVersion = new MCVersionChoice( this, ID_select_MC );
	bSizer1->Add( m_choiceMCVersion, 0, wxALL|wxEXPAND, 5 );
	
	auto bSizer2 = new wxBoxSizer( wxHORIZONTAL );
	{
		m_showOldSnapshots = false;
		m_checkOldSnapshots = new wxCheckBox( this, ID_show_old, wxT("Show old snapshots"), wxDefaultPosition, wxDefaultSize, 0 );
		m_checkOldSnapshots->SetValue(m_showOldSnapshots); 
		bSizer2->Add( m_checkOldSnapshots, 0, wxALL, 5 );
		
		
		m_showNewSnapshots = false;
		m_checkNewSnapshots = new wxCheckBox( this, ID_show_new, wxT("Show new snapshots"), wxDefaultPosition, wxDefaultSize, 0 );
		m_checkNewSnapshots->SetValue(m_showNewSnapshots); 
		bSizer2->Add( m_checkNewSnapshots, 0, wxALL, 5 );
	}
	bSizer1->Add( bSizer2, 0, wxALIGN_CENTER_HORIZONTAL, 5 );
	
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
			task->DoSetStatus("Loading version list...");
			return (wxThread::ExitCode) verList.Reload();
		};

		LambdaTask *lTask = new LambdaTask(func);
		TaskProgressDialog taskDlg(this);
		taskDlg.ShowModal(lTask);
		delete lTask;
	}
	Refilter();
	
	return wxDialog::ShowModal();
}

void NewInstanceDialog::Refilter()
{
	m_choiceMCVersion->Refilter();
	Refresh();
}

void NewInstanceDialog::OnName ( wxCommandEvent& event )
{
	if(!m_loadingDone)
		return;
	if(m_textName->IsEmptyUnfocused())
	{
		//m_textFolder->SetValue("");
		m_btnOK->Enable(false);
		m_name = wxEmptyString;
		UpdateIcon();
		return;
	}
	m_name = m_textName->GetValue();
	m_name.Strip(wxString::both);
	wxString folderName;
	if(fsutils::GetValidInstanceFolderName(m_name,folderName))
	{
		//m_textFolder->SetValue(folderName);
		m_btnOK->Enable(true);
	}
	else
	{
		//m_textFolder->SetValue(_("Invalid Path"));
		m_btnOK->Enable(false);
	}
	UpdateIcon();
}

void NewInstanceDialog::OnCheckbox ( wxCommandEvent& event )
{
	bool changed = false;
	changed |= m_checkNewSnapshots->GetValue() != m_showNewSnapshots;
	m_showNewSnapshots = m_checkNewSnapshots->GetValue();
	changed |= m_checkOldSnapshots->GetValue() != m_showOldSnapshots;
	m_showOldSnapshots = m_checkOldSnapshots->GetValue();
	if(changed)
		Refilter();
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

wxString NewInstanceDialog::GetInstanceMCVersion()
{
	int selection = m_choiceMCVersion->GetSelection();
	if(selection != -1)
		return m_choiceMCVersion->GetString(selection);
	return _("Current stable");
}

wxString NewInstanceDialog::GetInstanceLWJGL()
{
	return "Mojang";
}

BEGIN_EVENT_TABLE(NewInstanceDialog, wxDialog)
	EVT_TEXT(ID_text_name, NewInstanceDialog::OnName)
	EVT_BUTTON(ID_icon_select, NewInstanceDialog::OnIcon)
	EVT_CHECKBOX(wxID_ANY, NewInstanceDialog::OnCheckbox)
END_EVENT_TABLE()

void NewInstanceDialog::MCVersionChoice::Refilter()
{
	visibleIndexes.clear();
	Clear();
	MCVersionList & verList = MCVersionList::Instance();
	for(unsigned i = 0; i < verList.versions.size(); i++ )
	{
		MCVersion & ver = verList.versions[i];
		VersionType vt = ver.GetVersionType();
		if(m_owner->m_showOldSnapshots &&  vt == OldSnapshot ||
			m_owner->m_showNewSnapshots && vt == Snapshot ||
			vt == CurrentStable || vt == Stable
		)
		{
			visibleIndexes.push_back(i);
			Append(verList.versions[i].GetName());
		}
	}
	if(visibleIndexes.size())
		SetSelection(0);
}
