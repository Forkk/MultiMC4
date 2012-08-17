#include "multimc.h"

#include <wx/utils.h>
#include <wx/mstream.h>
#include <wx/cmdline.h>
#include <wx/process.h>
#include <wx/stdpaths.h>
#include <wx/fs_arc.h>
#include <wx/socket.h>
#include <wx/app.h>

#include "mainwindow.h"

#include "apputils.h"
#include "osutils.h"

#include "resources/windowicon.h"

IMPLEMENT_APP(MultiMC)

// App
bool MultiMC::OnInit()
{
#if __WXGTK__ || defined MSVC
	// Only works with Linux GCC or MSVC
	wxHandleFatalExceptions();
#endif

	updateOnExit = false;
	
	// This is necessary for the update system since it calls OnInitCmdLine
	// to set up the command line arguments that the update system uses.
	if (!wxApp::OnInit())
		return false;
	
	SetAppName(_("MultiMC"));
	
	wxInitAllImageHandlers();
	wxSocketBase::Initialize();
	
	wxMemoryInputStream iconInput16(multimc16, sizeof(multimc16));
	wxMemoryInputStream iconInput32(multimc32, sizeof(multimc32));
	wxMemoryInputStream iconInput64(multimc64, sizeof(multimc64));
	wxMemoryInputStream iconInput128(multimc128, sizeof(multimc128));
	wxIcon icon16,icon32,icon64,icon128;
	icon16.CopyFromBitmap(wxBitmap(wxImage(iconInput16)));
	icon32.CopyFromBitmap(wxBitmap(wxImage(iconInput32)));
	icon64.CopyFromBitmap(wxBitmap(wxImage(iconInput64)));
	icon128.CopyFromBitmap(wxBitmap(wxImage(iconInput128)));
	AppIcons.AddIcon(icon16);
	AppIcons.AddIcon(icon32);
	AppIcons.AddIcon(icon64);
	AppIcons.AddIcon(icon128);
	
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
	mainWin->OnStartup();
	
	return true;
}

void MultiMC::OnInitCmdLine(wxCmdLineParser &parser)
{
	parser.SetDesc(cmdLineDesc);
	parser.SetSwitchChars(_("-"));
}

bool MultiMC::OnCmdLineParsed(wxCmdLineParser& parser)
{
	wxString fileToUpdate;
	if (parser.Found(_("u"), &fileToUpdate))
	{
		InstallUpdate(wxFileName(wxStandardPaths::Get().GetExecutablePath()), 
			wxFileName(fileToUpdate));
		return false;
	}
	return true;
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

const wxIconBundle &MultiMC::GetAppIcons() const
{
	return AppIcons;
}