#include "shadedtextedit.h"

IMPLEMENT_DYNAMIC_CLASS(ShadedTextEdit,wxTextCtrl);

BEGIN_EVENT_TABLE(ShadedTextEdit, wxTextCtrl)
	EVT_SET_FOCUS(ShadedTextEdit::OnSetFocus)
	EVT_KILL_FOCUS(ShadedTextEdit::OnKillFocus)
END_EVENT_TABLE()

void ShadedTextEdit::OnSetFocus(wxFocusEvent& event)
{
	m_focused = true;
	if(m_isempty)
	{
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
		SetValue(m_emptyContent);
	}
	else
	{
		m_isempty = false;
	}
	
	event.Skip();
}