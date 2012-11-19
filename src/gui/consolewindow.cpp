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

#include "consolewindow.h"

#include <wx/gbsizer.h>
#include <wx/sstream.h>
#include <wx/mstream.h>
#include <wx/wfstream.h>
#include <wx/msgdlg.h>
#include <wx/clipbrd.h>
#include <wx/persist.h>

#include <gui/mainwindow.h>

#ifdef WIN32
#include <windows.h>
#endif

#include "tasks/pastebintask.h"

#include "gui/taskprogressdialog.h"
#include "advancedmsgdlg.h"

#include "multimc.h"
#include "resources/consoleicon.h"
#include "apputils.h"
#include "osutils.h"
#include "version.h"
#include "buildtag.h"
#include "mcprocess.h"

InstConsoleWindow::InstConsoleWindow(Instance *inst, wxWindow* mainWin, bool quitAppOnClose)
	: wxFrame(NULL, -1, _("MultiMC Console"), wxDefaultPosition, wxSize(620, 250))
{
	m_quitAppOnClose = quitAppOnClose;
	m_mainWin = mainWin;
	m_running = nullptr;
	m_inst = inst;
	crashReportIsOpen = false;
	
	wxPanel *mainPanel = new wxPanel(this, -1);
	wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);
	mainPanel->SetSizer(mainSizer);
	
	consoleTextCtrl = new wxTextCtrl(mainPanel, -1, wxEmptyString, 
									 wxDefaultPosition, wxSize(200, 100), 
									 wxTE_MULTILINE | wxTE_READONLY | wxTE_RICH);
	mainSizer->Add(consoleTextCtrl, wxSizerFlags(1).Expand().Border(wxALL, 8));
	consoleTextCtrl->SetBackgroundColour(*wxWHITE);
	

	wxBoxSizer *btnBox = new wxBoxSizer(wxHORIZONTAL);
	mainSizer->Add(btnBox, 0, wxEXPAND | wxBOTTOM | wxRIGHT | wxLEFT, 8);

	wxButton *crashReportBtn = new wxButton(mainPanel, ID_GENREPORT, _("Generate Crash &Report"));
	btnBox->Add(crashReportBtn, wxSizerFlags(0).Align(wxALIGN_LEFT));

	btnBox->AddStretchSpacer();
	
	killButton = new wxButton(mainPanel, wxID_CANCEL, _("&Kill Minecraft"));
	btnBox->Add(killButton, wxSizerFlags(0).Align(wxALIGN_RIGHT));
	closeButton = new wxButton(mainPanel, wxID_CLOSE, _("&Close"));
	btnBox->Add(closeButton, wxSizerFlags(0).Align(wxALIGN_RIGHT));
	
	
	// disable close button and the X button provided by the window manager
	AllowClose(false);
	
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
	CenterOnScreen();
}

InstConsoleWindow::~InstConsoleWindow() {}

void InstConsoleWindow::AppendMessage(const wxString& msg, MessageType msgT)
{
	// Prevent some red spam
	if (msg.Contains("[STDOUT]") || msg.Contains("[ForgeModLoader]"))
		msgT = MSGT_STDOUT;

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

	(*consoleTextCtrl) << msg << "\n";

	consoleTextCtrl->SetDefaultStyle(wxTextAttr(
		wxSystemSettings::GetColour(wxSYS_COLOUR_LISTBOXTEXT)));
}

void InstConsoleWindow::OnProcessExit( bool killed, int status )
{
	m_timerIdleWakeUp.Stop();
	//FIXME: what are the exact semantics of this?
	if(killed)
		delete m_running;
	else
		m_running->Detach();
	m_running = nullptr;
	
	AppendMessage(wxString::Format(_("Minecraft exited with code %i."), status));
	
	AllowClose();
	if (killed)
	{
		AppendMessage(_("Minecraft was killed."));
		SetState(STATE_BAD);
		Show();
	}
	else if (status != 0)
	{
		AppendMessage(_("Minecraft has crashed!"));
		SetState(STATE_BAD);
		Show();
	}
	else if (settings->GetAutoCloseConsole() && !crashReportIsOpen)
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
	if(crashReportIsOpen)
		return;
	
	//FIXME: is this actually intended? This calls OnWindowClosed() via wxFrame
	wxFrame::Close();
	if (trayIcon->IsIconInstalled())
		trayIcon->RemoveIcon();
	m_mainWin->Show();
}

void InstConsoleWindow::OnCloseClicked(wxCommandEvent& event)
{
	Close();
}

//NOTE: indirectly called by Close()
void InstConsoleWindow::OnWindowClosed(wxCloseEvent& event)
{
	if (event.CanVeto() && !m_closeAllowed)
	{
		event.Veto();
	}
	else
	{
		wxPersistenceManager::Get().SaveAndUnregister(this);
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

InstConsoleWindow::ConsoleIcon::ConsoleIcon(InstConsoleWindow *console)
{
	m_console = console;
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
	m_console->OnKillMC(event);
}

void InstConsoleWindow::OnKillMC ( wxCommandEvent& event )
{
	if(m_running == nullptr)
		return;
	if (wxMessageBox(_("Killing Minecraft may damage saves. You should only do this if the game is frozen."),
		_("Are you sure?"), wxOK | wxCANCEL | wxCENTER) == wxOK)
	{
		m_running->KillMinecraft();
	}
}

void InstConsoleWindow::OnProcessTimer(wxTimerEvent& WXUNUSED(event))
{
	wxWakeUpIdle();
}

bool InstConsoleWindow::LinkProcess(MinecraftProcess *process)
{
	if (m_running==NULL)
	{
		m_running = process;
		m_timerIdleWakeUp.Start(100);
		return true;
	}
	return false;
}

void InstConsoleWindow::OnIdle(wxIdleEvent& event)
{
	if (m_running==NULL)
		return;
	m_running->ProcessInput();
}

void InstConsoleWindow::SetUserInfo(wxString username, wxString sessID)
{
	m_username = username;
	m_sessID = sessID;
}

wxString InstConsoleWindow::GetCrashReport()
{
	wxString mlLogPath = Path::Combine(m_inst->GetMCDir(), "ModLoader.txt");
	wxString fmlLogPath = Path::Combine(m_inst->GetMCDir(), "ForgeModLoader-client-0.log");
	if (!wxFileExists(fmlLogPath))
		fmlLogPath = Path::Combine(m_inst->GetMCDir(), "ForgeModLoader-0.log");

	wxString consoleLog, modListStr, mlLog, fmlLog;

	consoleLog = consoleTextCtrl->GetValue();

	// Mask the username and session ID if possible.
	if (!m_username.IsEmpty())
	{
		consoleLog.Replace(m_username, "<username>");
	}

	if (!m_sessID.IsEmpty())
	{
		consoleLog.Replace(m_sessID, "<session ID>");
	}

	if (wxFileExists(mlLogPath))
	{
		wxFFileInputStream in(mlLogPath);
		wxStringOutputStream out(&mlLog);
		in.Read(out);
	}

	if (wxFileExists(fmlLogPath))
	{
		wxFFileInputStream in(fmlLogPath);
		wxStringOutputStream out(&fmlLog);
		in.Read(out);
	}

	{
		wxString jModList = m_inst->GetModList()->ToString(1);
		wxString mlModList = m_inst->GetMLModList()->ToString(1);
		wxString cModList = m_inst->GetCoreModList()->ToString(1);
		if (!jModList.IsEmpty())
			modListStr << "Jar Mods: " << NEWLINE << jModList << NEWLINE;
		if (!mlModList.IsEmpty())
			modListStr << "ModLoader Mods: " << NEWLINE << mlModList << NEWLINE;
		if (!cModList.IsEmpty())
			modListStr << "Core Mods: " << NEWLINE << cModList << NEWLINE;
	}

	// Fix newline chars in the logs.
	// First, convert all CRLF into LF so we don't screw things up.
	consoleLog.Replace("\r\n", "\n");
	mlLog.Replace("\r\n", "\n");
	fmlLog.Replace("\r\n", "\n");

	// Next, convert everything from LF to the correct newline.
	consoleLog.Replace("\n", NEWLINE);
	mlLog.Replace("\n", NEWLINE);
	fmlLog.Replace("\n", NEWLINE);


	wxString versionInfo = wxString::Format("%d.%d.%d %s", 
		AppVersion.GetMajor(), AppVersion.GetMinor(), AppVersion.GetRevision(), 
		AppBuildTag.ToString().c_str());

	wxString crashReportString;
	crashReportString << "------------- MultiMC Crash Report -------------" << NEWLINE
		<< "Information:" << NEWLINE
		<< "\tDate: " << wxDateTime::Now().Format("%m-%d-%Y %H:%M:%S") << NEWLINE
		<< "\tOperating System: " << wxGetOsDescription() << NEWLINE
		<< "\tMultiMC Version: " << versionInfo << NEWLINE
		<< "\tMinecraft Version: " << m_inst->GetJarVersion() << NEWLINE;

	crashReportString << NEWLINE << "------------------ Mod Lists -------------------" << NEWLINE
		<< modListStr;
	
	crashReportString << NEWLINE << "----------------- Console Log ------------------" << NEWLINE
		<< consoleLog << NEWLINE;

	if (mlLog != wxEmptyString)
	{
		crashReportString << NEWLINE << "---------------- ModLoader Log -----------------" << NEWLINE
			<< mlLog << NEWLINE;
	}

	if (fmlLog != wxEmptyString)
	{
		crashReportString << NEWLINE << "------------------- FML Log --------------------" << NEWLINE
			<< fmlLog << NEWLINE;
	}

	return crashReportString;
}

void InstConsoleWindow::OnGenReportClicked(wxCommandEvent& event)
{
	wxString crashReportString = GetCrashReport();

	const int id_pastebin = 1;
	const int id_file = 2;
	const int id_clipboard = 3;

	AdvancedMsgDialog::ButtonDefList btns;
	btns.push_back(AdvancedMsgDialog::ButtonDef(_("Send to Pastebin"), id_pastebin));
	btns.push_back(AdvancedMsgDialog::ButtonDef(_("Save to File"), id_file));
	btns.push_back(AdvancedMsgDialog::ButtonDef(_("Copy to Clipboard"), id_clipboard));
	btns.push_back(AdvancedMsgDialog::ButtonDef(_("&Cancel"), wxID_CANCEL));

	AdvancedMsgDialog msgDlg(this, _("A crash report has been generated. "
		"What would you like to do with it?"), btns, _("Crash Report"));


	crashReportIsOpen = true;
	int response = msgDlg.ShowModal();
	crashReportIsOpen = false;
	if (response == id_pastebin) // Pastebin
	{
		PastebinTask *task = new PastebinTask(crashReportString);
		TaskProgressDialog tDlg(this);
		if (tDlg.ShowModal(task))
		{
			wxTextEntryDialog urlDialog(this, _("Your error report has been"
				" sent to the URL listed below."), _("Success"), task->GetPasteURL(),
				wxOK | wxCENTER);
			urlDialog.ShowModal();
		}
		else
		{
			wxMessageBox(_("Failed to send the crash report to pastebin. "
				"Please check your internet connection."), _("Error"));
		}
		delete task;
	}
	else if (response == id_pastebin) // Save to file
	{
		wxFileDialog saveReportDlg(this, _("Save Crash Report"), wxGetCwd(), 
			wxDateTime::Now().Format("MultiMC_Report_%m-%d-%Y_%H-%M-%S.txt"), 
			"*.txt", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
		if (saveReportDlg.ShowModal() != wxID_CANCEL)
		{
			wxFFileOutputStream outStream(saveReportDlg.GetFilename());
			wxStringInputStream inStream(crashReportString);
			outStream.Write(inStream);
		}
	}
	else if (response == id_clipboard)
	{
		if (wxTheClipboard->Open())
		{
			wxTheClipboard->SetData(new wxTextDataObject(crashReportString));
			wxTheClipboard->Close();
		}
	}
}


BEGIN_EVENT_TABLE(InstConsoleWindow, wxFrame)
	EVT_BUTTON(wxID_CLOSE, InstConsoleWindow::OnCloseClicked)
	EVT_BUTTON(wxID_CANCEL, InstConsoleWindow::OnKillMC)
	EVT_CLOSE( InstConsoleWindow::OnWindowClosed )
	EVT_IDLE(InstConsoleWindow::OnIdle)
	EVT_TIMER(wakeupidle, InstConsoleWindow::OnProcessTimer)
	
	EVT_BUTTON(ID_GENREPORT, InstConsoleWindow::OnGenReportClicked)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(InstConsoleWindow::ConsoleIcon, wxTaskBarIcon)
	EVT_MENU(ID_SHOW_CONSOLE, InstConsoleWindow::ConsoleIcon::OnShowConsole)
	EVT_MENU(ID_KILL_MC, InstConsoleWindow::ConsoleIcon::OnKillMC)
END_EVENT_TABLE()
