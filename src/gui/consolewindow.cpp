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

InstConsoleWindow::InstConsoleWindow(Instance *inst, wxWindow* mainWin)
	: wxFrame(NULL, -1, _("MultiMC Console"), wxDefaultPosition, wxSize(620, 250)),
	  instListener(inst, this)
{
	m_closeAllowed = false;
	m_mainWin = mainWin;
	m_inst = inst;
	inst->SetEvtHandler(this);
	
	wxPanel *mainPanel = new wxPanel(this, -1);
	wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);
	mainPanel->SetSizer(mainSizer);
	
	wxString launchCmdMessage = wxString::Format(_("Instance started with command: %s\n"), 
												 inst->GetLastLaunchCommand().c_str());
	
	consoleTextCtrl = new wxTextCtrl(mainPanel, -1, launchCmdMessage, 
									 wxDefaultPosition, wxSize(200, 100), 
									 wxTE_MULTILINE | wxTE_READONLY);
	mainSizer->Add(consoleTextCtrl, wxSizerFlags(1).Expand().Border(wxALL, 8));
	
	wxBoxSizer *btnBox = new wxBoxSizer(wxHORIZONTAL);
	mainSizer->Add(btnBox, wxSizerFlags(0).Align(wxALIGN_BOTTOM | wxALIGN_RIGHT).
				   Border(wxBOTTOM | wxRIGHT, 8));
	
	closeButton = new wxButton(mainPanel, wxID_CLOSE, _("&Close"));
	closeButton->Enable(m_closeAllowed);
	btnBox->Add(closeButton, wxSizerFlags(0).Align(wxALIGN_RIGHT));
	
	instListener.Create();
	instListener.Run();
	
	CenterOnScreen();
}

InstConsoleWindow::~InstConsoleWindow()
{
	instListener.Delete();
}

void InstConsoleWindow::AppendMessage(const wxString& msg)
{
	(*consoleTextCtrl) << msg << _("\n");
}

void InstConsoleWindow::OnInstExit(wxProcessEvent& event)
{
	AppendMessage(wxString::Format(_("Instance exited with code %i."), 
								   event.GetExitCode()));
	if (event.GetExitCode() != 0)
	{
		AppendMessage(_("Minecraft has crashed!"));
		Show();
		AllowClose();
	}
	else if (settings.autoCloseConsole)
	{
		Close();
	}
	else
	{
		Show();
		AllowClose();
	}
}

void InstConsoleWindow::AllowClose()
{
	m_closeAllowed = true;
	closeButton->Enable();
}

void InstConsoleWindow::Close()
{
	wxFrame::Close();
	m_mainWin->Show();
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
		m_mainWin->Show();
	}
}

InstConsoleWindow::InstConsoleListener::InstConsoleListener(Instance* inst, InstConsoleWindow *console)
	: wxThread(wxTHREAD_JOINABLE)
{
	m_inst = inst;
	m_console = console;
	instProc = inst->GetInstProcess();
}

void* InstConsoleWindow::InstConsoleListener::Entry()
{
	if (!instProc->IsRedirected())
	{
		printf("Output not redirected!\n");
		return NULL;
	}
	
	wxInputStream *consoleStream = instProc->GetInputStream();
	wxInputStream *errorStream = instProc->GetErrorStream();
	wxString outputBuffer;
	
	bool readConsole = false;
	bool readError = false;
	
	const size_t bufSize = 1024;
	char *buffer = new char[bufSize];
	
	size_t readSize = 0;
	while (m_inst->IsRunning() && !TestDestroy())
	{
		readConsole = consoleStream->CanRead();
		readError = errorStream->CanRead();
		
		if (!readConsole && !readError)
			wxMicroSleep(100);
		
		// Read from input / error
		wxString temp;
		wxStringOutputStream tempStream(&temp);
		
		if (readConsole)
		{
			consoleStream->Read(buffer, bufSize);
			readSize = consoleStream->LastRead();
		}
		else if (readError)
		{
			errorStream->Read(buffer, bufSize);
			readSize = errorStream->LastRead();
		}
		else
			continue;
		
		tempStream.Write(buffer, readSize);
		outputBuffer.Append(temp);
		
		// Pass lines to the console
		size_t newlinePos;
		while ((newlinePos = outputBuffer.First('\n')) != wxString::npos)
		{
			wxString line = outputBuffer.SubString(0, newlinePos - 1);
			outputBuffer = outputBuffer.SubString(newlinePos, wxString::npos);
			
			InstOutputEvent event(m_inst, line);
			m_console->AddPendingEvent(event);
		}
	}
}

void InstConsoleWindow::OnInstOutput(InstOutputEvent& event)
{
	AppendMessage(event.m_output);
}


BEGIN_EVENT_TABLE(InstConsoleWindow, wxFrame)
	EVT_END_PROCESS(-1, InstConsoleWindow::OnInstExit)
	EVT_BUTTON(wxID_CLOSE, InstConsoleWindow::OnCloseClicked)
	
	EVT_INST_OUTPUT(InstConsoleWindow::OnInstOutput)
END_EVENT_TABLE()