#pragma once

#include <wx/app.h>
#include <wx/icon.h>
#include <wx/filename.h>

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
	
	const wxIcon &GetAppIcon() const;
	
	bool updateOnExit;
protected:
	wxIcon AppIcon;
	
	void InstallUpdate(wxFileName thisFile, wxFileName targetFile);
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
