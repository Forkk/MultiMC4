#pragma once

#include <wx/string.h>
#include <wx/stattext.h>
#include <wx/gdicmn.h>
#include <wx/font.h>
#include <wx/colour.h>
#include <wx/settings.h>
#include <wx/textctrl.h>
#include <wx/statline.h>
#include <wx/choice.h>
#include <wx/checkbox.h>
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/dialog.h>
#include <vector>

class MCVersion;
class ShadedTextEdit;
///////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
/// Class NewInstanceDialog
///////////////////////////////////////////////////////////////////////////////
class NewInstanceDialog : public wxDialog 
{
	protected:
		wxButton* m_btnIcon;
		ShadedTextEdit* m_textName;
		
		wxTextCtrl * m_versionDisplay;
		wxChoice* m_choiceLwjgl;
		wxButton* m_btnOK;
		wxButton* m_changeVersionButton;
		
		enum NI_IDs
		{
			ID_text_name= 5000,
			ID_text_version,
			ID_btn_version,
			ID_select_MC,
			ID_select_LWJGL,
			ID_icon_select
		};
		
		bool m_loadingDone;
		wxString m_iconKey;
		wxString m_visibleIconKey;
		wxString m_name;
		wxString m_username;
		MCVersion * m_selectedVersion;
		
	protected:
		void OnIcon(wxCommandEvent& event);
		void OnVersion(wxCommandEvent& event);
		void OnName(wxCommandEvent& event);
		void SetIconKey(wxString iconkey);
		void UpdateIcon();
		void UpdateVersionDisplay();
		void UpdateOKButton();
		
	public:
		NewInstanceDialog( wxWindow* parent, wxString username = wxEmptyString)
		: wxDialog( parent, wxID_ANY, wxT("Create New Instance"), wxDefaultPosition, wxSize( 394,380 ), wxDEFAULT_DIALOG_STYLE )
		{
			m_username = username;
			Create();
		};
		bool Create();
		
		~NewInstanceDialog();
		wxString GetInstanceName();
		MCVersion * GetInstanceMCVersion();
		wxString GetInstanceMCVersionDescr();
		wxString GetInstanceLWJGL();
		wxString GetInstanceIconKey();
		int ShowModal();
	
	
	protected:
		DECLARE_EVENT_TABLE()
};

