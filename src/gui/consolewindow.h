/*
    Copyright 2012 Andrew Okin

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#pragma once
#include <wx/wx.h>

#include "instance.h"

class InstConsoleWindow : public wxFrame
{
public:
	InstConsoleWindow(Instance* inst, wxWindow* mainWin);
	virtual ~InstConsoleWindow();
	
protected:
	wxScrolledWindow *scrollWindow;
	wxTextCtrl *consoleTextCtrl;
	
	wxButton *closeButton;
	wxCheckBox *showConsoleCheckbox;
	
	wxWindow *m_mainWin;
	Instance *m_inst;
	
	class InstConsoleListener : public wxThread
	{
	public:
		InstConsoleListener(Instance* inst, InstConsoleWindow* console);
		
		virtual void* Entry();
		
	protected:
		Instance *m_inst;
		wxProcess *instProc;
		
		InstConsoleWindow *console;
		InstConsoleWindow* m_console;
	} instListener;
	
	void AppendMessage(const wxString &msg);
	
	void OnInstExit(wxProcessEvent &event);
	void OnCloseClicked(wxCommandEvent &event);
	void OnWindowClosed(wxCloseEvent &event);
	void OnInstOutput(InstOutputEvent &event);
	
	void AllowClose();
	
	void Close();
	
	bool m_closeAllowed;
	
	DECLARE_EVENT_TABLE()
};