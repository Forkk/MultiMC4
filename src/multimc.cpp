#include "multimc.h"

#include <wx/utils.h>
#include <wx/mstream.h>
#include <wx/cmdline.h>
#include <wx/process.h>
#include <wx/stdpaths.h>
#include <wx/fs_arc.h>
#include <wx/socket.h>

#include "mainwindow.h"

#include "apputils.h"
#include "osutils.h"

#include "windowicon.h"

IMPLEMENT_APP(MultiMC)

// App
bool MultiMC::OnInit()
{
#if __WXGTK__
	// Only works with Linux GCC or MSVC
	wxHandleFatalExceptions();
#endif
	
	startNormally = true;
	
	wxApp::OnInit();
	
	if (!startNormally)
		return false;
	
	SetAppName(_("MultiMC"));
	
	wxInitAllImageHandlers();
	wxSocketBase::Initialize();
	
	wxMemoryInputStream iconInput(MultiMC32_png, MultiMC32_png_len);
	AppIcon.CopyFromBitmap(wxBitmap(wxImage(iconInput)));
	
	wxFileSystem::AddHandler(new wxArchiveFSHandler);
	// 	wxFileSystem::AddHandler(new wxMemoryFSHandler);
	
	if (!InitAppSettings())
	{
		wxLogError(_("Failed to initialize settings."));
		return false;
	}
	
	if (!settings.GetInstDir().DirExists())
	{
		settings.GetInstDir().Mkdir();
	}
	
	MainWindow *mainWin = new MainWindow();
	mainWin->Show();
	mainWin->LoadInstanceList();
	
	return true;
}

void MultiMC::OnInitCmdLine(wxCmdLineParser &parser)
{
	parser.AddOption(_("u"), _("update"), 
		_("Used by the update system. Causes the running executable to replace the given file with itself, run it, and exit."),
		wxCMD_LINE_VAL_STRING);

	int pRetVal = parser.Parse();
	if (pRetVal == -1)
	{
		startNormally = false;
		return;
	}
	else if (pRetVal > 0)
	{
		startNormally = false;
		return;
	}
	
	wxString fileToUpdate;
	if (parser.Found(_("u"), &fileToUpdate))
	{
		startNormally = false;
		InstallUpdate(wxFileName(wxStandardPaths::Get().GetExecutablePath()), 
					  wxFileName(fileToUpdate));
	}
}

void MultiMC::InstallUpdate(wxFileName thisFile, wxFileName targetFile)
{
	// Let the other process exit.
	printf("Installing updates, please wait...");
	wxSleep(3);
	
	wxCopyFile(thisFile.GetFullPath(), targetFile.GetFullPath());
	
	targetFile.MakeAbsolute();
	
	wxProcess proc;
	wxExecute(targetFile.GetFullPath(), wxEXEC_ASYNC, &proc);
	proc.Detach();
}

int MultiMC::OnExit()
{
#ifdef __WXMSW__
	wxString updaterFileName = _("MultiMCUpdate.exe");
#else
	wxString updaterFileName = _("MultiMCUpdate");
#endif

	if (updateOnExit && wxFileExists(updaterFileName))
	{
		wxFileName updateFile(Path::Combine(wxGetCwd(), updaterFileName));
		if (IS_LINUX())
		{
			wxExecute(_("chmod +x ") + updateFile.GetFullPath());
		}
		
		wxProcess proc;
		wxString launchCmd = updateFile.GetFullPath() + _(" -u:") + wxStandardPaths::Get().GetExecutablePath();

		if (IS_WINDOWS())
		{
			launchCmd = wxString::Format(_("cmd /C \"%s\""), launchCmd.c_str());
		}

		wxExecute(launchCmd, wxEXEC_ASYNC, &proc);
		proc.Detach();
	}
	
	return wxApp::OnExit();
}

void MultiMC::OnFatalException()
{
	wxMessageBox(_("A fatal error has occurred and MultiMC has to exit. Sorry for the inconvenience."), 
		_("Oh no!"), wxICON_ERROR | wxCENTER);
}

void MultiMC::OnUnhandledException()
{
	OnFatalException();
}

const wxIcon &MultiMC::GetAppIcon() const
{
	return AppIcon;
}