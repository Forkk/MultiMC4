#include "shadedtextedit.h"
#include <wx/settings.h>

IMPLEMENT_DYNAMIC_CLASS(ShadedTextEdit,wxTextCtrl);

BEGIN_EVENT_TABLE(ShadedTextEdit, wxTextCtrl)
	EVT_SET_FOCUS(ShadedTextEdit::OnSetFocus)
	EVT_KILL_FOCUS(ShadedTextEdit::OnKillFocus)
END_EVENT_TABLE()

ShadedTextEdit::ShadedTextEdit ( wxWindow* parent, wxString emptyContent, wxWindowID id, long int style )
: wxTextCtrl(parent,id,emptyContent,wxDefaultPosition,wxDefaultSize,style)
{
	m_emptyContent = emptyContent;
	m_isempty = true;
	SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
};

void ShadedTextEdit::OnSetFocus(wxFocusEvent& event)
{
	m_focused = true;
	if(m_isempty)
	{
		SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_WINDOWTEXT));
		SetValue("");
	}
	event.Skip();
}

void ShadedTextEdit::OnKillFocus(wxFocusEvent& event)
{
	wxString val = GetValue();
	m_focused = false;
	if(val.Strip(wxString::both).empty() || val == m_emptyContent)
	{
		m_isempty = true;
		SetForegroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
		SetValue(m_emptyContent);
	}
	else
	{
		m_isempty = false;
	}
	
	event.Skip();
}