#pragma once

#include <wx/app.h>
#include <wx/icon.h>
#include <wx/filename.h>
#include <wx/iconbndl.h>

#include <wx/cmdline.h>

class MultiMC : public wxApp
{
public:
	virtual bool OnInit();
	virtual void OnInitCmdLine(wxCmdLineParser &parser);
	virtual bool OnCmdLineParsed(wxCmdLineParser& parser);
	virtual int OnExit();
	virtual void OnFatalException();
	virtual void OnUnhandledException();
	
	const wxIconBundle &GetAppIcons() const;
	
	bool updateOnExit;
protected:
	wxIconBundle AppIcons;

	wxString thisFileName;
	wxString updateTarget;
	
	void InstallUpdate();
	void YieldSleep(int secs);

	enum StartupMode
	{
		// Starts MultiMC normally.
		START_NORMAL,

		// Installs updates and exits.
		START_INSTALL_UPDATE,
	} startMode;
};

const wxCmdLineEntryDesc cmdLineDesc[] = 
{
	{ wxCMD_LINE_SWITCH, _("h"), _("help"), _("displays help on command line parameters"), 
		wxCMD_LINE_VAL_NONE, wxCMD_LINE_OPTION_HELP },

	{ wxCMD_LINE_OPTION, _("u"), _("update"), _("replaces the given file with the running executable"),
		wxCMD_LINE_VAL_STRING },

	{ wxCMD_LINE_NONE }
};

DECLARE_APP(MultiMC)
