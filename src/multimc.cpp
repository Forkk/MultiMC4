#include "multimc.h"

#include <wx/utils.h>
#include <wx/mstream.h>
#include <wx/cmdline.h>
#include <wx/process.h>
#include <wx/stdpaths.h>
#include <wx/fs_arc.h>

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
	
	return true;
}

void MultiMC::OnInitCmdLine(wxCmdLineParser &parser)
{
	wxCmdLineEntryDesc updateOption;
	updateOption.kind = wxCMD_LINE_OPTION;
	updateOption.type = wxCMD_LINE_VAL_STRING;
	updateOption.shortName = _("u");
	updateOption.longName = _("update");
	updateOption.description = _("Used by the update system. Causes the running executable to replace the given file with itself, run it, and exit.");
	
	wxCmdLineEntryDesc descriptions[] = 
	{
		updateOption
	};
	parser.SetDesc(descriptions);
	parser.Parse();
	
	if (parser.Found(_("u")))
	{
		wxString fileToUpdate;
		parser.Found(_("u"), &fileToUpdate);
		startNormally = false;
		InstallUpdate(wxFileName(wxStandardPaths::Get().GetExecutablePath()), 
					  wxFileName(fileToUpdate));
	}
}

void MultiMC::InstallUpdate(wxFileName thisFile, wxFileName targetFile)
{
	// Let the other process exit.
	wxSleep(3);
	
	printf("Installing updates...");
	wxCopyFile(thisFile.GetFullPath(), targetFile.GetFullPath());
	
	targetFile.MakeAbsolute();
	
	wxProcess proc;
	wxExecute(targetFile.GetFullPath(), wxEXEC_ASYNC, &proc);
	proc.Detach();
}

int MultiMC::OnExit()
{
	if (updateOnExit && wxFileExists(_("MultiMCUpdate")))
	{
		wxFileName updateFile(Path::Combine(wxGetCwd(), _("MultiMCUpdate")));
		if (IS_LINUX())
		{
			wxExecute(_("chmod +x ") + updateFile.GetFullPath());
		}
		
		wxProcess proc;
		wxString launchCmd = updateFile.GetFullPath() + _(" -u:") + wxStandardPaths::Get().GetExecutablePath();
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