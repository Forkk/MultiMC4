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
#include <wx/regex.h>
#include <wx/dir.h>

#include <gui/mainwindow.h>

#ifdef WIN32
#include <windows.h>
#endif

#include "tasks/pastebintask.h"
#include "tasks/imgurtask.h"

#include "gui/taskprogressdialog.h"
#include "textdisplaydialog.h"
#include "advancedmsgdlg.h"

#include "multimc.h"
#include "resources/consoleicon.h"
#include "utils/apputils.h"
#include "utils/osutils.h"
#include "version.h"
#include "buildtag.h"
#include "mcprocess.h"
#include "mainwindow.h"

InstConsoleWindow::InstConsoleWindow(Instance *inst, MainWindow* mainWin, bool quitAppOnClose)
	: wxFrame(NULL, -1, _("MultiMC Console"), wxDefaultPosition, wxSize(620, 250))
{
	SetAprilFonts(this);

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
	
	killButton = new wxButton(mainPanel, wxID_DELETE, _("&Kill Minecraft"));
	btnBox->Add(killButton, wxSizerFlags(0).Align(wxALIGN_RIGHT));
	closeButton = new wxButton(mainPanel, wxID_CLOSE, _("&Close"));
	btnBox->Add(closeButton, wxSizerFlags(0).Align(wxALIGN_RIGHT));

	// close is close at this point. there is no linked process yet.
	SetCloseIsHide(false);
	
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
	SetCloseIsHide(false);
	
	AppendMessage(wxString::Format(_("Minecraft exited with code %i."), status));

	bool keepOpen = CheckCommonProblems(consoleTextCtrl->GetValue());

	if (killed)
	{
		AppendMessage(_("Minecraft was killed."));
		SetState(STATE_BAD);
		Show();
		Raise();
	}
	else if (status != 0)
	{
		AppendMessage(_("Minecraft has crashed!"));
		SetState(STATE_BAD);
		Show();
		Raise();
	}
	else if ((settings->GetAutoCloseConsole() || !IsShown() ) && !crashReportIsOpen && !keepOpen)
	{
		Close();
	}
	else
	{
		Show();
		Raise();
	}
}

void InstConsoleWindow::SetCloseIsHide ( bool isHide )
{
	closeIsHide = isHide;
	killButton->Enable(isHide);
	if(closeIsHide)
	{
		closeButton->SetLabel("&Hide");
	}
	else
	{
		closeButton->SetLabel("&Close");
	}
}


void InstConsoleWindow::Close()
{
	if(crashReportIsOpen)
		return;
	
	//FIXME: is this actually intended? This calls OnWindowClosed() via wxFrame
	wxFrame::Close();
}


//NOTE: indirectly called by Close()
void InstConsoleWindow::OnWindowClosed(wxCloseEvent& event)
{
	if(closeIsHide && event.CanVeto())
	{
		event.Veto(true);
		Show(false);
		return;
	}
	if (trayIcon->IsIconInstalled())
		trayIcon->RemoveIcon();
	delete trayIcon;
	Destroy();
	
	if (m_quitAppOnClose)
	{
		m_mainWin->Close();
	}
	else
	{
		m_mainWin->ReturnToMainWindow();
	}
}

void InstConsoleWindow::OnCloseButton ( wxCommandEvent& event )
{
	Close();
}


InstConsoleWindow::ConsoleIcon::ConsoleIcon(InstConsoleWindow *console)
{
	m_console = console;
}

void InstConsoleWindow::ConsoleIcon::TaskBarLeft ( wxTaskBarIconEvent& e )
{
	if(!m_console->IsShown() || m_console->IsIconized())
	{
		m_console->Show();
		m_console->Raise();
	}
	else
	{
		m_console->Iconize();
	}
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
	menu->AppendSeparator();
	menu->Append(ID_IMGUR, _("Send last screenshot to imgur"), 
		_("Uploads the last screenshot taken ingame to imgur.com."));
	menu->Append(ID_KILL_MC, _("Kill Minecraft"), _("Kills Minecraft's process."));
	
	return menu;
}

void InstConsoleWindow::ConsoleIcon::OnShowConsole(wxCommandEvent &event)
{
	m_console->Show(event.IsChecked());
	if(event.IsChecked())
		m_console->Raise();
}

void InstConsoleWindow::ConsoleIcon::OnKillMC(wxCommandEvent &event)
{
	m_console->OnKillMC(event);
}

void InstConsoleWindow::OnKillMC ( wxCommandEvent& event )
{
	if (wxMessageBox(_("Killing Minecraft may damage saves. You should only do this if the game is frozen."),
		_("Are you sure?"), wxOK | wxCANCEL | wxCENTER, this) == wxOK)
	{
		if(m_running)
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
		SetCloseIsHide(true);
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
	msgDlg.CenterOnParent();
	int response = msgDlg.ShowModal();
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
	else if (response == id_file) // Save to file
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
	crashReportIsOpen = false;
}

bool InstConsoleWindow::CheckCommonProblems(const wxString& output)
{
	wxRegEx idConflictRegex("([0-9]+) is already occupied by ([A-Za-z0-9.]+)@[A-Za-z0-9]+ when adding ([A-Za-z0-9.]+)@[A-Za-z0-9]+");

	if (!idConflictRegex.IsValid())
	{
		wxLogError(_("ID conflict regex is invalid!"));
		return false;
	}

	if (idConflictRegex.Matches(output))
	{
		// We have an ID conflict.
		wxArrayString values;

		for (unsigned i = 0; i < idConflictRegex.GetMatchCount(); i++)
		{
			values.Add(idConflictRegex.GetMatch(output, i));
		}

		if (values.Count() < 4)
		{
			// Something's wrong here...
			wxLogError(_("Not enough values matched ID conflict regex!"));
			return false;
		}

		// Alert the user.
		AppendMessage(wxString::Format(
			_("MultiMC found a block or item ID conflict. %s and %s are both using the same block ID (%s)."), 
			values[2].c_str(), values[3].c_str(), values[1].c_str()), MSGT_SYSTEM);
		return true;
	}

	// No common problems found.
	return false;
}

void InstConsoleWindow::OnImgurClicked(wxCommandEvent& event)
{
	// Find the newest screenshot.
	wxDir screnshotDir(m_inst->GetScreenshotsDir().GetFullPath());
	if (!screnshotDir.IsOpened())
	{
		wxLogError(_("Failed to open screenshot folder."));
		return;
	}

	wxDateTime newestModTime(1, wxDateTime::Jan, 0); // Set this to 'zero'
	wxString newestFile;
	wxString currentFile;
	if (screnshotDir.GetFirst(&currentFile, "*.png"))
	{
		do
		{
			wxFileName cFileName(screnshotDir.GetName(), currentFile);
			wxDateTime currentModTime = cFileName.GetModificationTime();

			if (currentModTime > newestModTime)
			{
				newestModTime = currentModTime;
				newestFile = Path::Combine(screnshotDir.GetName(), currentFile);
			}
		} while (screnshotDir.GetNext(&currentFile));
	}

	if (newestFile == wxEmptyString || !wxFileExists(newestFile))
	{
		wxMessageBox(_("No screenshots found. You must press F2 in-game to take a screenshot first."),
			_("No screenshots to upload"), wxOK | wxCENTER, this);
		return;
	}


	ImgurTask* task = new ImgurTask(newestFile);
	TaskProgressDialog tDlg(this);

	if (tDlg.ShowModal(task))
	{
		wxTextEntryDialog urlDialog(this, _("Your screenshot has been"
			" sent to the URL shown below."), _("Success"), task->GetImageURL(),
			wxOK | wxCENTER);
		urlDialog.ShowModal();
	}
	else
	{
		wxMessageBox(wxString::Format(_("Failed to upload the image.\n%s"),
			task->GetErrorMsg().c_str()), _("Error"), wxOK | wxCENTER, this);
	}
}


BEGIN_EVENT_TABLE(InstConsoleWindow, wxFrame)
	EVT_BUTTON(wxID_CLOSE, InstConsoleWindow::OnCloseButton)
	EVT_BUTTON(wxID_DELETE, InstConsoleWindow::OnKillMC)
	EVT_CLOSE( InstConsoleWindow::OnWindowClosed )
	EVT_IDLE(InstConsoleWindow::OnIdle)
	EVT_TIMER(wakeupidle, InstConsoleWindow::OnProcessTimer)
	
	EVT_BUTTON(ID_GENREPORT, InstConsoleWindow::OnGenReportClicked)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(InstConsoleWindow::ConsoleIcon, wxTaskBarIcon)
	EVT_MENU(ID_SHOW_CONSOLE, InstConsoleWindow::ConsoleIcon::OnShowConsole)
	EVT_MENU(ID_KILL_MC, InstConsoleWindow::ConsoleIcon::OnKillMC)
	EVT_MENU(ID_IMGUR, InstConsoleWindow::ConsoleIcon::OnImgurClicked)
	EVT_TASKBAR_LEFT_DOWN(InstConsoleWindow::ConsoleIcon::TaskBarLeft)
	
END_EVENT_TABLE()
