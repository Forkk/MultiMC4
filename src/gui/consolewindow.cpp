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


#include "consolewindow.h"

#include <wx/gbsizer.h>
#include <wx/sstream.h>
#include <wx/mstream.h>
#include <gui/mainwindow.h>

#ifdef WIN32
#include <windows.h>
#endif

#include "multimc.h"
#include "resources/consoleicon.h"

InstConsoleWindow::InstConsoleWindow(Instance *inst, wxWindow* mainWin, bool quitAppOnClose)
	: wxFrame(NULL, -1, _("MultiMC Console"), wxDefaultPosition, wxSize(620, 250)),
	  stdoutListener(inst, this, InstConsoleListener::LISTENER_STDOUT), 
	  stderrListener(inst, this, InstConsoleListener::LISTENER_STDERR)
{
	m_quitAppOnClose = quitAppOnClose;
	instListenerStarted = false;
	killedInst = false;
	m_mainWin = mainWin;
	m_inst = inst;
	inst->SetEvtHandler(this);
	
	wxPanel *mainPanel = new wxPanel(this, -1);
	wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);
	mainPanel->SetSizer(mainSizer);
	
	consoleTextCtrl = new wxTextCtrl(mainPanel, -1, wxEmptyString, 
									 wxDefaultPosition, wxSize(200, 100), 
									 wxTE_MULTILINE | wxTE_READONLY | wxTE_RICH);
	mainSizer->Add(consoleTextCtrl, wxSizerFlags(1).Expand().Border(wxALL, 8));
	consoleTextCtrl->SetBackgroundColour(*wxWHITE);
	
	wxBoxSizer *btnBox = new wxBoxSizer(wxHORIZONTAL);
	mainSizer->Add(btnBox, wxSizerFlags(0).Align(wxALIGN_BOTTOM | wxALIGN_RIGHT).
				   Border(wxBOTTOM | wxRIGHT, 8));
	
	closeButton = new wxButton(mainPanel, wxID_CLOSE, _("&Close"));
	
	// disable close button and the X button provided by the window manager
	AllowClose(false);
	btnBox->Add(closeButton, wxSizerFlags(0).Align(wxALIGN_RIGHT));
	
	consoleIcons = new wxIconArray();
	wxMemoryInputStream iconInput1(console, sizeof(console));
	wxMemoryInputStream iconInput2(console_error, sizeof(console_error));
	wxMemoryInputStream iconInput3(console24, sizeof(console24));
	wxMemoryInputStream iconInput4(console_error24, sizeof(console_error24));
	wxIcon icon_OK,icon_BAD,icon_OK24,icon_BAD24;
	icon_OK.CopyFromBitmap(wxBitmap(wxImage(iconInput1)));
	icon_BAD.CopyFromBitmap(wxBitmap(wxImage(iconInput2)));
	icon_OK24.CopyFromBitmap(wxBitmap(wxImage(iconInput3)));
	icon_BAD24.CopyFromBitmap(wxBitmap(wxImage(iconInput4)));
	consoleIcons->Add(icon_OK);
	consoleIcons->Add(icon_BAD);
	consoleIcons->Add(icon_OK24);
	consoleIcons->Add(icon_BAD24);
	
	// Create the task bar icon.
	trayIcon = new ConsoleIcon(this);
	SetState(STATE_OK);
	
	inst->GetInstProcess()->SetNextHandler(this);

	AppendMessage(wxString::Format(_("Instance started with command: %s\n"), 
		inst->GetLastLaunchCommand().c_str()));
	
	stdoutListener.Create();
	stderrListener.Create();
	
	CenterOnScreen();
}

InstConsoleWindow::~InstConsoleWindow()
{
	stdoutListener.Delete();
	stderrListener.Delete();
}

void InstConsoleWindow::AppendMessage(const wxString& msg, MessageType msgT)
{
	switch (msgT)
	{
	case MSGT_SYSTEM:
		consoleTextCtrl->SetDefaultStyle(
			wxTextAttr(settings->GetConsoleSysMsgColor()));
		break;

	case MSGT_STDOUT:
		consoleTextCtrl->SetDefaultStyle(
			wxTextAttr(settings->GetConsoleStdoutColor()));
		break;

	case MSGT_STDERR:
		consoleTextCtrl->SetDefaultStyle(
			wxTextAttr(settings->GetConsoleStderrColor()));
		break;
	}

	(*consoleTextCtrl) << msg << _("\n");

	consoleTextCtrl->SetDefaultStyle(wxTextAttr(
		wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOXTEXT)));
}

void InstConsoleWindow::OnInstExit(wxProcessEvent& event)
{
	AppendMessage(wxString::Format(_("Instance exited with code %i."), 
		event.GetExitCode()));
	
	AllowClose();
	if (event.GetExitCode() != 0)
	{
		AppendMessage(_("Minecraft has crashed!"));
		SetState(STATE_BAD);
		Show();
	}
	else if (killedInst)
	{
		AppendMessage(_("Instance was killed."));
		SetState(STATE_BAD);
		Show();
	}
	else if (settings->GetAutoCloseConsole())
	{
		Close();
	}
	else
	{
		Show();
	}
}

void InstConsoleWindow::AllowClose(bool allow)
{
	if(allow)
	{
		EnableCloseButton(true);
		m_closeAllowed = true;
		closeButton->Enable();
	}
	else
	{
		EnableCloseButton(false);
		m_closeAllowed = false;
		closeButton->Enable(false);
	}
}

void InstConsoleWindow::Close()
{
	wxFrame::Close();
	if (trayIcon->IsIconInstalled())
		trayIcon->RemoveIcon();
	m_mainWin->Show();
}

bool InstConsoleWindow::Show(bool show)
{
	bool retval = wxFrame::Show(show);
	if (!instListenerStarted)
	{
		instListenerStarted = true;
		stdoutListener.Run();
		stderrListener.Run();
	}
	return retval;
}

bool InstConsoleWindow::Start()
{
	return Show(settings->GetShowConsole());
}


void InstConsoleWindow::OnCloseClicked(wxCommandEvent& event)
{
	Close();
}

void InstConsoleWindow::OnWindowClosed(wxCloseEvent& event)
{
	if (event.CanVeto() && !m_closeAllowed)
	{
		event.Veto();
	}
	else
	{
		if (trayIcon->IsIconInstalled())
			trayIcon->RemoveIcon();
		m_mainWin->Show();
		Destroy();
		if (m_quitAppOnClose)
		{
			m_mainWin->Destroy();
			wxGetApp().ExitMainLoop();
		}
	}
}

InstConsoleWindow::InstConsoleListener::InstConsoleListener(Instance* inst, InstConsoleWindow *console, Type lType)
	: wxThread(wxTHREAD_JOINABLE)
{
	m_inst = inst;
	m_console = console;
	m_lType = lType;
	instProc = inst->GetInstProcess();
}

void* InstConsoleWindow::InstConsoleListener::Entry()
{
	if (!instProc->IsRedirected())
	{
		printf("Output not redirected!\n");
		m_console->AppendMessage(_("Output not redirected!\n"));
		return NULL;
	}
	
	int instPid = instProc->GetPid();
	
	wxInputStream *consoleStream;
	if (m_lType == LISTENER_STDERR)
		consoleStream = instProc->GetErrorStream();
	else
		consoleStream = instProc->GetInputStream();

	wxString outputBuffer;
	
	const size_t bufSize = 1024;
	char *buffer = new char[bufSize];
	
	size_t readSize = 0;
	while (m_inst->IsRunning() && !TestDestroy() && wxProcess::Exists(instPid))
	{
		if (TestDestroy())
			break;
		
		// Read from input
		wxString temp;
		wxStringOutputStream tempStream(&temp);
		
		consoleStream->Read(buffer, bufSize);
		readSize = consoleStream->LastRead();
		
		/*
		 * readSize of 0 signifies that consoleStream reached EOF
		 * This means it has been closed by minecraft.
		 * Do not continue trying to read.
		 */
		if(readSize == 0)
		{
			InstOutputEvent event(m_inst, wxString::Format(_("Instance closed its output %d."), m_lType), true);
			m_console->AddPendingEvent(event);
			break;
		}
		
		tempStream.Write(buffer, readSize);
		outputBuffer.Append(temp);
		
		// Pass lines to the console
		size_t newlinePos;
		wxString lines;
		while ((newlinePos = outputBuffer.First('\n')) != wxString::npos)
		{
			wxString line = outputBuffer.Left(newlinePos);
			if (line.EndsWith(_("\n")) || line.EndsWith(_("\r")))
				line = line.Left(line.size() - 1);
			if (line.EndsWith(_("\r\n")))
				line = line.Left(line.size() - 2);
			outputBuffer = outputBuffer.Mid(newlinePos + 1);

			if (lines != wxEmptyString)
				lines = lines.Append(wxT("\n"));
			lines = lines.Append(line);
		}

		InstOutputEvent event(m_inst, lines, m_lType == LISTENER_STDERR);
		m_console->AddPendingEvent(event);
	}

	return NULL;
}

void InstConsoleWindow::OnInstOutput(InstOutputEvent& event)
{
	MessageType msgT = (event.m_stdErr ? MSGT_STDERR : MSGT_STDOUT);

	if (msgT == MSGT_STDERR && (event.m_output.Contains(_("[STDOUT]")) || 
		event.m_output.Contains(_("[ForgeModLoader]"))))
		msgT = MSGT_STDOUT;

	AppendMessage(event.m_output, msgT);
}

InstConsoleWindow::ConsoleIcon::ConsoleIcon(InstConsoleWindow *console)
{
	m_console = console;
}

Instance *InstConsoleWindow::GetInstance()
{
	return m_inst;
}

void InstConsoleWindow::StopListening()
{
	stdoutListener.Pause();
	stderrListener.Pause();
}

void InstConsoleWindow::SetState ( InstConsoleWindow::State newstate )
{
	switch(newstate)
	{
		case STATE_OK:
			trayIcon->SetIcon(consoleIcons->operator[](2));
			SetIcon(consoleIcons->operator[](0));
			break;
		case STATE_BAD:
			trayIcon->SetIcon(consoleIcons->operator[](3));
			SetIcon(consoleIcons->operator[](1));
			break;
	}
}


wxMenu *InstConsoleWindow::ConsoleIcon::CreatePopupMenu()
{
	wxMenu *menu = new wxMenu();
	menu->AppendCheckItem(ID_SHOW_CONSOLE, _("Show Console"), _("Shows or hides the console."))->
		Check(m_console->IsShown());
	menu->Append(ID_KILL_MC, _("Kill Minecraft"), _("Kills Minecraft's process."));
	
	return menu;
}

void InstConsoleWindow::ConsoleIcon::OnShowConsole(wxCommandEvent &event)
{
	m_console->Show(event.IsChecked());
}

void InstConsoleWindow::ConsoleIcon::OnKillMC(wxCommandEvent &event)
{
	if (wxMessageBox(_("Killing Minecraft may damage saves. You should only do this if the game is frozen."),
		_("Are you sure?"), wxOK | wxCANCEL | wxCENTER) == wxOK)
	{
		m_console->KillMinecraft();
	}
}

void InstConsoleWindow::KillMinecraft(int tries)
{
	wxProcess *instProc = GetInstance()->GetInstProcess();
	
	// if there is no instance process anymore, it's DEAD. ABORT!
	if(!instProc)
		return;
	
	int pid = instProc->GetPid();
	if (pid == 0)
		return;

	// Stop listening for output from the process
	StopListening();

#ifdef WIN32
	// On Windows, use Win32's TerminateProcess()

	// Get a handle to the process.
	HANDLE pHandle = OpenProcess(PROCESS_TERMINATE | SYNCHRONIZE, true, pid);

	if (pHandle == NULL)
	{
		// An error occurred.
		DWORD error = GetLastError();

		switch (error)
		{
		case ERROR_ACCESS_DENIED:
			AppendMessage(wxString::Format(_("Failed to get process handle. Error 0x%08X: access denied."), error));
			break;

		default:
			AppendMessage(wxString::Format(_("Unknown error 0x%08X occurred when getting process handle."), error));
			break;
		}

		return;
	}

	// Terminate the process.
	if (!TerminateProcess(pHandle, 0))
	{
		// An error occurred.
		DWORD error = GetLastError();

		switch (error)
		{
		case ERROR_ACCESS_DENIED:
			AppendMessage(wxString::Format(_("Failed terminate process. Error 0x%08X: access denied."), error));
			break;

		default:
			AppendMessage(wxString::Format(_("Unknown error 0x%08X occurred when terminating the process."), error));
			break;
		}

		return;
	}

	// Wait for the process to actually exit. If it hasn't exited after 3 seconds, display an error message.
	for (int i = 0; i < 60; i++)
	{
		wxSafeYield(nullptr, true);

		DWORD waitResult = WaitForSingleObject(pHandle, 50);

		// The process exited successfully.
		if (waitResult == WAIT_OBJECT_0)
		{
			break;
		}
		else if (waitResult == WAIT_FAILED)
		{
			DWORD error = GetLastError();
			AppendMessage(wxString::Format(_("Unknown error 0x%08X occurred when waiting for process to exit."), error));
			CloseHandle(pHandle);
			return;
		}
	}

	// Close the handle.
	CloseHandle(pHandle);

#else
	// On other OSes, use wxProcess::Kill()
	wxKillError error = wxProcess::Kill(pid, wxSIGTERM);
	if (error != wxKILL_OK)
	{
		wxString errorName;
		switch (error)
		{
		case wxKILL_ACCESS_DENIED:
			errorName = _("wxKILL_ACCESS_DENIED");
			break;

		case wxKILL_BAD_SIGNAL:
			errorName = _("wxKILL_BAD_SIGNAL");
			break;

		case wxKILL_ERROR:
			errorName = _("wxKILL_ERROR");

			// Wait to see if the process kills.
			for (int i = 0; i < 20; i++)
			{
				wxSafeYield(nullptr, true);
				wxMilliSleep(100);

				if (!wxProcess::Exists(pid))
				{
					goto KillSuccess;
					break;
				}
			}

			if (tries == 0)
			{
				// Try again
				AppendMessage(_("Failed to kill the process. Trying again..."));
				KillMinecraft(1);
				return;
			}
			break;

		case wxKILL_NO_PROCESS:
			errorName = _("wxKILL_NO_PROCESS");
			break;

		default:
			errorName = _("Unknown error.");
		}

		AppendMessage(wxString::Format(
			_("Error %i (%s) when killing process %i!"), error, errorName.c_str(), 
			GetInstance()->GetInstProcess()->GetPid()));
		return;
	}

KillSuccess:
#endif
	AppendMessage(wxString::Format(_("Killed Minecraft (pid: %i)"), pid));
	killedInst = true;
	//wxProcessEvent fakeEvent(0, pid, -1);
	//OnInstExit(fakeEvent);
}


BEGIN_EVENT_TABLE(InstConsoleWindow, wxFrame)
	EVT_END_PROCESS(-1, InstConsoleWindow::OnInstExit)
	EVT_BUTTON(wxID_CLOSE, InstConsoleWindow::OnCloseClicked)
	EVT_CLOSE( InstConsoleWindow::OnWindowClosed )
	EVT_INST_OUTPUT(InstConsoleWindow::OnInstOutput)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(InstConsoleWindow::ConsoleIcon, wxTaskBarIcon)
	EVT_MENU(ID_SHOW_CONSOLE, InstConsoleWindow::ConsoleIcon::OnShowConsole)
	EVT_MENU(ID_KILL_MC, InstConsoleWindow::ConsoleIcon::OnKillMC)
END_EVENT_TABLE()