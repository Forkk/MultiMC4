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
#include <wx/wx.h>
#include <wx/taskbar.h>

#include "instance.h"

class MinecraftProcess;
class MainWindow;

class InstConsoleWindow : public wxFrame
{
	enum timertype
	{
		wakeupidle=5000
	};

public:
	InstConsoleWindow(Instance* inst, MainWindow* mainWin, bool quitAppOnClose = false);
	virtual ~InstConsoleWindow();
	
	// Returns a "crash report" string that contains console logs, FML logs, 
	// and ML logs as well as other useful info.
	wxString GetCrashReport();

	// Tells the console the user's username and session ID. This allows 
	// them to be "masked" in the user's crash report.
	void SetUserInfo(wxString username, wxString sessID);
	
	enum MessageType
	{
		MSGT_SYSTEM,
		MSGT_STDOUT,
		MSGT_STDERR,
	};
	void AppendMessage(const wxString &msg, MessageType msgT = MSGT_SYSTEM);
	
	bool LinkProcess(MinecraftProcess *process);
	void OnProcessExit(bool killed, int status);
	
protected:
	wxString m_username, m_sessID;

	bool m_quitAppOnClose;

	class ConsoleIcon : public wxTaskBarIcon
	{
	public:
		ConsoleIcon(InstConsoleWindow *console);
		
		virtual wxMenu *CreatePopupMenu();
		
		void OnShowConsole(wxCommandEvent &event);
		void OnKillMC(wxCommandEvent &event);
		void TaskBarLeft(wxTaskBarIconEvent &e);

	protected:
		
		InstConsoleWindow *m_console;
		
		DECLARE_EVENT_TABLE()
	} *trayIcon;
	
	wxIconArray *consoleIcons;
	
	wxScrolledWindow *scrollWindow;
	wxTextCtrl *consoleTextCtrl;
	
	wxButton *closeButton;
	wxButton *killButton;
	
	MainWindow *m_mainWin;
	Instance *m_inst;

	// Scans the output for common problems and alerts the user.
	bool CheckCommonProblems(const wxString& output);
	
	// Called by timer to generate wakeupidle events
	void OnProcessTimer(wxTimerEvent& event);
	void OnIdle(wxIdleEvent& event);

	void OnWindowClosed(wxCloseEvent &event);

	void OnGenReportClicked(wxCommandEvent& event);
	void OnPastebinClicked(wxCommandEvent& event);
	void OnKillMC(wxCommandEvent &event);
	void OnCloseButton(wxCommandEvent &event);
	
	void Close();
	void SetCloseIsHide(bool isHide);

	enum State
	{
		STATE_OK,
		STATE_BAD,
	};
	void SetState(State newstate);
	
	bool closeIsHide;
	bool crashReportIsOpen;
	wxTimer m_timerIdleWakeUp;
	MinecraftProcess* m_running;
	
	DECLARE_EVENT_TABLE()
};

enum
{
	ID_SHOW_CONSOLE = 1,
	ID_KILL_MC,

	ID_GENREPORT,
};
