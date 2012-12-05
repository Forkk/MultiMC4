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

#include "wx/wx.h"
#include <wx/zipstrm.h>
#include <wx/wfstream.h>
#include "mcprocess.h"
#include "consolewindow.h"
#include <insticonlist.h>
#include <memory>
#include "launcher/launcherdata.h"
#if !defined(WIN32)
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#endif

// macro for adding "" around strings
#define DQuote(X) "\"" << X << "\""

void ExtractLauncher(Instance* source)
{
	wxMemoryInputStream launcherInputStream(multimclauncher, sizeof(multimclauncher));
	wxZipInputStream dezipper(launcherInputStream);
	wxFFileOutputStream launcherOutStream( Path::Combine(source->GetMCDir(),"MultiMCLauncher.jar") );
	wxZipOutputStream zipper(launcherOutStream);
	std::auto_ptr<wxZipEntry> entry;
	// copy all files from the old zip file
	while (entry.reset(dezipper.GetNextEntry()), entry.get() != NULL)
		if (!zipper.CopyEntry(entry.release(), dezipper))
			break;
	// add the icon file
	zipper.PutNextEntry("icon.png");
	InstIconList * iconList = InstIconList::Instance();
	//FIXME: what if there is no such image?
	wxImage &img =  iconList->getImage128ForKey(source->GetIconKey());
	img.SaveFile(zipper,wxBITMAP_TYPE_PNG);
}

wxProcess* MinecraftProcess::Launch ( Instance* source, InstConsoleWindow* parent, wxString username, wxString sessionID )
{
	// Set lastLaunch
	source->SetLastLaunchNow();

	if (username.IsEmpty())
		username = "Offline";
	
	if (sessionID.IsEmpty())
		sessionID = "Offline";
	
	ExtractLauncher(source);
	
	// window size parameter (depends on some flags also)
	wxString winSizeArg;
	if (!source->GetUseAppletWrapper())
		winSizeArg = "compatmode";
	else if (source->GetMCWindowMaximize())
		winSizeArg = "max";
	else
		winSizeArg << source->GetMCWindowWidth() << "x" << source->GetMCWindowHeight();
	
	// putting together the window title
	wxString windowTitle;
	windowTitle << "MultiMC: " << source->GetName();
	
	// now put together the launch command in the form:
	// "%java%" %extra_args% -Xms%min_memory%m -Xmx%max_memory%m -jar MultiMCLauncher.jar "%user_name%" "%session_id%" "%window_title%" "%window_size%"
	wxString launchCmd;
	launchCmd << DQuote(source->GetJavaPath()) << " " << source->GetJvmArgs()
	          << " -Xms" << source->GetMinMemAlloc() << "m" << " -Xmx" << source->GetMaxMemAlloc() << "m"
	          << " -jar MultiMCLauncher.jar "
	          << " " << DQuote(username) << " " << DQuote(sessionID) << " " << DQuote(windowTitle) << " " << DQuote(winSizeArg);
	
	// create a (custom) process object!
	MinecraftProcess *instProc = new MinecraftProcess(source, parent);
	instProc->Redirect();
	
	// set up environment path
	wxExecuteEnv env;
	wxFileName mcDir = source->GetMCDir();
	mcDir.MakeAbsolute();
	env.cwd = mcDir.GetFullPath();
	
	parent->AppendMessage(wxString::Format(_("Instance folder is:\n%s\n"), env.cwd.c_str()));
	
	// run minecraft using the stuff above :)
	int pid = wxExecute(launchCmd,wxEXEC_ASYNC|wxEXEC_MAKE_GROUP_LEADER,instProc,&env);
	if(pid > 0)
	{
		instProc->m_pid = pid;
		parent->LinkProcess(instProc);
		parent->AppendMessage(wxString::Format(_("Instance started with command:\n%s\n"), launchCmd.c_str()));
	}
	else
	{
		parent->AppendMessage(wxString::Format(_("Failed to start instance with command:\n%s\n"), launchCmd.c_str()),
		                      InstConsoleWindow::MSGT_STDERR);
		parent->AppendMessage(_("This can mean that you either don't have Java installed,\n or that you need to set up Java path in MultiMC settings."),InstConsoleWindow::MSGT_STDERR);
		delete instProc;
		instProc = nullptr;
	}
	return instProc;
}

MinecraftProcess::MinecraftProcess(Instance * source, InstConsoleWindow* parent)
	: wxProcess(wxPROCESS_REDIRECT), m_wasKilled(false), m_parent(parent)
{
	m_pid = 0;
}

bool MinecraftProcess::ProcessInput()    // The following methods are adapted from the exec sample.  This one manages the stream redirection
{
	int c;
	bool hasInput = false;
	// The original used wxTextInputStream to read a line at a time.  Fine, except when there was no \n, whereupon the thing would hang
	// Instead, read the stream (which may occasionally contain non-ascii bytes e.g. from g++) into a memorybuffer, then to a wxString
	while (IsInputAvailable())
	{
		wxMemoryBuffer buf;
		do
		{
			// Get a char from the input
			c = GetInputStream()->GetC();
			if (c == wxEOF)
				break;
			if (c == '\r')
				continue;
			if (c== '\n')
				break;
			buf.AppendByte(c);
		}
		while (IsInputAvailable());// Unless \n, loop to get another char
		
		wxString line((const char*)buf.GetData(), wxConvLibc, buf.GetDataLen());
		m_parent->AppendMessage(line,InstConsoleWindow::MSGT_STDOUT);
		hasInput = true;
	}

	while (IsErrorAvailable())
	{
		wxMemoryBuffer buf;
		do
		{
			c = GetErrorStream()->GetC();
			if (c == wxEOF)
				break;
			if (c == '\r')
				continue;
			if (c== '\n')
				break;
			buf.AppendByte(c);
		}
		while (IsErrorAvailable());
		wxString line((const char*)buf.GetData(), wxConvLibc, buf.GetDataLen());
		m_parent->AppendMessage(line,InstConsoleWindow::MSGT_STDERR);
		hasInput = true;
	}
	return hasInput;
}

// empty the minecraft output buffers, notify the console window about process exiting
void MinecraftProcess::OnTerminate(int pid, int status)
{
	while (ProcessInput());
#if !defined(WIN32)
	if (status == -1)
	{
		// Workaround for wxProcess bug:
		// wxProcess may call waitpid with WNOHANG when child has not exited and then
		// incorrectly report the exit code as -1.
		// For details, see:
		// Ticket #10258: race condition in wxEndProcessFDIOHandler::OnExceptionWaiting
		// http://trac.wxwidgets.org/ticket/10258
		int result;
		do
		{
			result = waitpid(pid, &status, 0);
		} while (result == -1 && errno == EINTR);

		if (result == -1)
			status = -1;
		else if (WIFEXITED(status))
			status = WEXITSTATUS(status);
	}
#endif
	m_parent->OnProcessExit(m_wasKilled, status);
}

// KILL IT WITH FIRE
void MinecraftProcess::KillMinecraft()
{
	if (!m_pid) return;
	// Take the pid we stored earlier & kill it.  SIGTERM seems to work now we use wxEXEC_MAKE_GROUP_LEADER and wxKILL_CHILDREN but first send a SIGHUP because of 
	// the stream redirection (else if the child is saying "Are you sure?" and waiting for input, SIGTERM fails in practice -> zombie processes and a non-deleted wxProcess)
	Kill(m_pid, wxSIGHUP, wxKILL_CHILDREN);

	// first try to be nice.
	wxKillError result = Kill(m_pid, wxSIGTERM, wxKILL_CHILDREN);
	if (result == wxKILL_OK)
	{
		m_parent->AppendMessage(_("Minecraft successfully aborted\n"));
	}
	else
	{
		// and then be mean :3
		result = Kill(m_pid, wxSIGKILL, wxKILL_CHILDREN);
		if (result == wxKILL_OK)
		{
			m_parent->AppendMessage(_("Minecraft successfully killed\n"));
		}
		else
		{
			m_parent->AppendMessage(_("It was impossible to kill Minecraft due to an error.\n"),InstConsoleWindow::MSGT_STDERR);
		}
	}
	m_wasKilled = true;
}
