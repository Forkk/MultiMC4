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

#include <wx/dialog.h>
#include <vector>

class AdvancedMsgDialog : public wxDialog
{
public:
	struct ButtonDef
	{
	public:
		ButtonDef(wxString text, int id);

		wxString m_text;
		int m_id;
	};

	typedef std::vector<ButtonDef> ButtonDefList;

	AdvancedMsgDialog(wxWindow *parent, const wxString &msg, 
		ButtonDefList btns, const wxString &title = wxEmptyString);

	void OnButtonClicked(wxCommandEvent &event);

protected:

	DECLARE_EVENT_TABLE()
};
