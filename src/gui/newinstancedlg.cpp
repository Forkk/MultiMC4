///////////////////////////////////////////////////////////////////////////
// C++ code generated with wxFormBuilder (version Aug 30 2012)
// http://www.wxformbuilder.org/
//
// PLEASE DO "NOT" EDIT THIS FILE!
///////////////////////////////////////////////////////////////////////////

#include "newinstancedlg.h"
#include "taskprogressdialog.h"
#include "mcversionlist.h"
#include <utils/fsutils.h>
#include <lambdatask.h>

///////////////////////////////////////////////////////////////////////////

NewInstanceDialog::NewInstanceDialog( wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& pos, const wxSize& size, long style ) : wxDialog( parent, id, title, pos, size, style )
{
	this->SetSizeHints( wxDefaultSize, wxDefaultSize );
	
	wxBoxSizer* bSizer1;
	bSizer1 = new wxBoxSizer( wxVERTICAL );
	
	// TODO: add instance icon selector here.
	
	auto staticText1 = new wxStaticText( this, wxID_ANY, wxT("Instance name:"), wxDefaultPosition, wxDefaultSize, 0 );
	//staticText1->Wrap( -1 );
	bSizer1->Add( staticText1, 0, wxALL|wxEXPAND, 5 );
	
	m_textName = new wxTextCtrl( this, ID_text_name, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_textName->SetMaxLength( 25 ); 
	bSizer1->Add( m_textName, 0, wxALL|wxEXPAND, 5 );
	
	auto staticText1b = new wxStaticText( this, wxID_ANY, wxT("Folder name:"), wxDefaultPosition, wxDefaultSize, 0 );
	bSizer1->Add( staticText1b, 0, wxALL|wxEXPAND, 5 );
	m_textFolder = new wxTextCtrl( this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0 );
	m_textFolder->Enable(false);
	bSizer1->Add( m_textFolder, 0, wxALL|wxEXPAND, 5 );
	
	auto staticline1 = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	bSizer1->Add( staticline1, 0, wxEXPAND | wxALL, 5 );
	
	auto staticText2 = new wxStaticText( this, wxID_ANY, wxT("Minecraft version:"), wxDefaultPosition, wxDefaultSize, 0 );
	//staticText2->Wrap( -1 );
	bSizer1->Add( staticText2, 0, wxALL, 5 );
	
	m_choiceMCVersion = new MCVersionChoice( this, ID_select_MC );
	bSizer1->Add( m_choiceMCVersion, 0, wxALL|wxEXPAND, 5 );
	
	auto bSizer2 = new wxBoxSizer( wxHORIZONTAL );
	
	m_showOldSnapshots = false;
	m_checkOldSnapshots = new wxCheckBox( this, ID_show_old, wxT("Show old snapshots"), wxDefaultPosition, wxDefaultSize, 0 );
	m_checkOldSnapshots->SetValue(m_showOldSnapshots); 
	bSizer2->Add( m_checkOldSnapshots, 0, wxALL, 5 );
	
	
	m_showNewSnapshots = false;
	m_checkNewSnapshots = new wxCheckBox( this, ID_show_new, wxT("Show new snapshots"), wxDefaultPosition, wxDefaultSize, 0 );
	m_checkNewSnapshots->SetValue(m_showNewSnapshots); 
	bSizer2->Add( m_checkNewSnapshots, 0, wxALL, 5 );
	
	bSizer1->Add( bSizer2, 0, wxALIGN_CENTER_HORIZONTAL, 5 );
	
	auto staticline2 = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	bSizer1->Add( staticline2, 0, wxEXPAND | wxALL, 5 );
	/*
	auto staticText3 = new wxStaticText( this, wxID_ANY, wxT("LWJGL version:"), wxDefaultPosition, wxDefaultSize, 0 );
	//staticText3->Wrap( -1 );
	bSizer1->Add( staticText3, 0, wxALL, 5 );
	
	wxArrayString m_choiceLwjglChoices;
	m_choiceLwjglChoices.Add(_("Mojang"));
	m_choiceLwjgl = new wxChoice( this, ID_select_LWJGL, wxDefaultPosition, wxDefaultSize, m_choiceLwjglChoices, 0 );
	m_choiceLwjgl->SetSelection( 0 );
	bSizer1->Add( m_choiceLwjgl, 0, wxALL|wxEXPAND, 5 );
	
	auto staticline3 = new wxStaticLine( this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLI_HORIZONTAL );
	bSizer1->Add( staticline3, 0, wxEXPAND | wxALL, 5 );
	*/
	bSizer1->AddStretchSpacer();
	
	auto btnSizer = CreateButtonSizer(wxOK | wxCANCEL);
	m_btnOK = (wxButton *)FindWindowById(wxID_OK, this);
	m_btnOK->Enable(false);
	
	bSizer1->Add(btnSizer, wxSizerFlags(0).Border(wxBOTTOM | wxRIGHT, 8).Align(wxALIGN_RIGHT | wxALIGN_BOTTOM));
	
	this->Layout();
	this->SetSizerAndFit(bSizer1);
	
	this->Center( wxBOTH );
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
	wxString name = m_textName->GetValue();
	name.Strip(wxString::both);
	if(name.empty())
	{
		m_textFolder->SetValue(_("Invalid Path"));
		m_btnOK->Enable(false);
		return;
	}
	wxString folderName;
	if(fsutils::GetValidInstanceFolderName(name,folderName))
	{
		m_textFolder->SetValue(folderName);
		m_btnOK->Enable(true);
	}
	else
	{
		m_textFolder->SetValue(_("Invalid Path"));
		m_btnOK->Enable(false);
	}
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

NewInstanceDialog::~NewInstanceDialog()
{
}

wxString NewInstanceDialog::GetInstanceName()
{
	return m_textName->GetValue().Strip(wxString::both);
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
	/*
	int selection = m_choiceLwjgl->GetSelection();
	if(selection != -1)
		return m_choiceLwjgl->GetString(selection);
	*/
	return _("Mojang");
}

BEGIN_EVENT_TABLE(NewInstanceDialog, wxDialog)
	EVT_TEXT(ID_text_name, NewInstanceDialog::OnName)
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
