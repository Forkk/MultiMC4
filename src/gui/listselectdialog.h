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
#include <wx/listctrl.h>

class wxGridBagSizer;

class ListSelectDialog : public wxDialog
{
public:
	ListSelectDialog(wxWindow *parent, const wxString& title);

	wxString GetSelection();
	int GetSelectedIndex();

	virtual int ShowModal();

protected:
	virtual void LoadList();
	virtual bool DoLoadList() = 0;
	virtual wxString OnGetItemText(long item, long column);

	// By default, the first column's size is set to the width of the list
	// control minus the sum of the widths of the other columns.
	// Override this function to change the behavior.
	virtual void SetupColumnSizes();

	void OnRefreshListClicked(wxCommandEvent& event);
	void OnListBoxSelChange(wxListEvent& event);

	void UpdateOKBtn();

	// Shows / hides the list control header.
	void ShowHeader(bool show);

	// String list that the list control reads from.
	wxArrayString sList;

	class ListSelectCtrl : public wxListCtrl
	{
	public:
		ListSelectCtrl(wxWindow* parent, ListSelectDialog* owner);

		virtual wxString OnGetItemText(long item, long column) const;

		void OnResize(wxSizeEvent& event);

	protected:
		ListSelectDialog* m_owner;

		DECLARE_EVENT_TABLE()
	} *listCtrl;

	// Sizers and other GUI controls. These are used by overriding classes to customize the GUI.
	wxBoxSizer* dlgSizer;

	wxPanel* mainPanel;
	wxGridBagSizer* mainSz;

	wxSizer* btnSz;
	wxButton* refreshButton;

	DECLARE_EVENT_TABLE()
};
