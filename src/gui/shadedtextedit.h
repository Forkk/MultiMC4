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

#pragma once
#include "wx/textctrl.h"

class ShadedTextEdit: public wxTextCtrl
{
public:
	DECLARE_DYNAMIC_CLASS(ShadedTextEdit);
	ShadedTextEdit ( wxWindow* parent, wxString emptyContent, wxWindowID id = wxID_ANY, long int style = 0 );
	ShadedTextEdit ():wxTextCtrl(){}
	
	bool IsEmptyUnfocused()
	{
		return m_isempty && !m_focused || GetValue().Strip(wxString::both).empty() && m_focused;
	}
	
	void OnKillFocus(wxFocusEvent& evt);
	void OnSetFocus(wxFocusEvent& evt);
protected:
	wxString m_emptyContent;
	bool m_isempty;
	bool m_focused;
	DECLARE_EVENT_TABLE()
};