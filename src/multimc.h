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

#pragma once

#include <wx/app.h>
#include <wx/icon.h>
#include <wx/filename.h>
#include <wx/iconbndl.h>

#include <wx/cmdline.h>

#include "langutils.h"

extern const wxString licenseText;

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

	enum ExitAction
	{
		// Do nothing on exit.
		EXIT_NORMAL,

		// Installs updates when MultiMC exits without restarting MultiMC when the update is done.
		EXIT_UPDATE,

		// Installs updates when MultiMC exits and restarts MultiMC when the update is done.
		EXIT_UPDATE_RESTART,

		// Restarts MultiMC on exit.
		EXIT_RESTART,
	} exitAction;
	
	bool useProvidedDir;
	wxFileName providedDir;

	LocaleHelper localeHelper;
protected:
	wxLocale* m_locale;

	wxIconBundle AppIcons;

	wxString thisFileName;
	wxString updateTarget;
	wxString launchInstance;
	
	void InstallUpdate();
	void YieldSleep(int secs);

	enum StartupMode
	{
		// Starts MultiMC normally.
		START_NORMAL,

		// Installs updates and exits.
		START_INSTALL_UPDATE,

		// Launches an instance on start.
		START_LAUNCH_INSTANCE
	} startMode;

	bool updateQuiet;
};

const wxCmdLineEntryDesc cmdLineDesc[] = 
{
	{ wxCMD_LINE_SWITCH, "h", "help", _("displays help on command line parameters"), 
		wxCMD_LINE_VAL_NONE, wxCMD_LINE_OPTION_HELP },

	{ wxCMD_LINE_OPTION, "u", "update", _("replaces the given file with the running executable"),
		wxCMD_LINE_VAL_STRING },

	{ wxCMD_LINE_SWITCH, "U", "quietupdate", _("doesn't restart MultiMC after installing updates"),
		wxCMD_LINE_VAL_NONE },

	{ wxCMD_LINE_OPTION, "l", "launch", _("tries to launch the given instance"),
		wxCMD_LINE_VAL_STRING },
	
	{ wxCMD_LINE_OPTION, "d", "dir", _("use the supplied directory as MultiMC root instead of the binary location (use '.' for current)"),
		wxCMD_LINE_VAL_STRING },

	{ wxCMD_LINE_NONE }
};

DECLARE_APP(MultiMC)
