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
#include <wx/taskbar.h>

#include "instance.h"

class InstConsoleWindow : public wxFrame
{
public:
	InstConsoleWindow(Instance* inst, wxWindow* mainWin);
	virtual ~InstConsoleWindow();
	
	virtual bool Show(bool show = true);
	bool Start();
	
	Instance *GetInstance();
	void StopListening();
	
protected:
	enum MessageType
	{
		MSGT_SYSTEM,
		MSGT_STDOUT,
		MSGT_STDERR,
	};

	class ConsoleIcon : public wxTaskBarIcon
	{
	public:
		ConsoleIcon(InstConsoleWindow *console);
		
		virtual wxMenu *CreatePopupMenu();
		
		void OnShowConsole(wxCommandEvent &event);
		void OnKillMC(wxCommandEvent &event);
	protected:
		
		InstConsoleWindow *m_console;
		
		DECLARE_EVENT_TABLE()
	} *trayIcon;
	
	wxScrolledWindow *scrollWindow;
	wxTextCtrl *consoleTextCtrl;
	
	wxButton *closeButton;
	wxCheckBox *showConsoleCheckbox;
	
	wxWindow *m_mainWin;
	Instance *m_inst;
	
	class InstConsoleListener : public wxThread
	{
	public:
		enum Type
		{
			LISTENER_STDOUT,
			LISTENER_STDERR,
		};

		InstConsoleListener(Instance* inst, InstConsoleWindow* console, 
			Type lType = LISTENER_STDOUT);
		
		virtual void* Entry();
		
	protected:
		Instance *m_inst;
		wxProcess *instProc;
		Type m_lType;
		
		InstConsoleWindow *console;
		InstConsoleWindow* m_console;
	} stdoutListener, stderrListener;
	
	void AppendMessage(const wxString &msg, MessageType msgT = MSGT_SYSTEM);
	
	void OnInstExit(wxProcessEvent &event);
	void OnCloseClicked(wxCommandEvent &event);
	void OnWindowClosed(wxCloseEvent &event);
	void OnInstOutput(InstOutputEvent &event);
	
	void AllowClose(bool allow = true);
	
	void Close();

	void KillMinecraft(int tries = 0);
	
	bool m_closeAllowed;
	bool instListenerStarted;
	bool killedInst;
	
	DECLARE_EVENT_TABLE()
};

enum
{
	ID_SHOW_CONSOLE = 1,
	ID_KILL_MC
};
