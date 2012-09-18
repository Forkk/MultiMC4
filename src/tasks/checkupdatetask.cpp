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

#include "checkupdatetask.h"
#include "httputils.h"
#include "apputils.h"
#include "osutils.h"
#include "config.h"
#include "appsettings.h"

#include <boost/property_tree/json_parser.hpp>

#include <sstream>

DEFINE_EVENT_TYPE(wxEVT_CHECK_UPDATE)

const wxString ciURL = _(JENKINS_JOB_URL);
//const wxString ciURL = _("http://ci.forkk.net/job/MultiMC4/arch=x64,os=Linux/");


CheckUpdateTask::CheckUpdateTask()
	: Task()
{
	
}

wxThread::ExitCode CheckUpdateTask::TaskStart()
{
	SetStatus(_("Getting version info..."));
	
	// Get the main page for the project
	wxString jobURL = ciURL;

	if (jobURL.EndsWith("/"))
		jobURL.RemoveLast();

	wxString buildURLSeg;
	if (settings->GetUseDevBuilds())
	{
		buildURLSeg = "lastSuccessfulBuild";
	}
	else
	{
		buildURLSeg = "Stable";
	}

	jobURL = jobURL.BeforeLast('/') + "/" + buildURLSeg + "/" + jobURL.AfterLast('/');

	if (!jobURL.EndsWith("/"))
		jobURL.Append("/");

	wxString mainPageJSON;
	if (!DownloadString(jobURL + _("api/json"), &mainPageJSON))
	{
		wxLogError(_("Failed to check for updates. Please check your internet connection."));
		return (ExitCode)0;
	}
	
	// Determine the latest stable build.
	int buildNumber = GetBuildNumber(mainPageJSON);

	// Figure out where to download the latest update.
	wxString dlFileName;
	if (IS_WINDOWS())
		dlFileName = _("MultiMC.exe");
	else if (IS_LINUX() || IS_MAC())
		dlFileName = _("MultiMC");

	wxString newCIURL = ciURL;
	if (newCIURL.EndsWith("/"))
		newCIURL.RemoveLast();

	wxString dlURL = wxString::Format(_("%s/%i/artifact/%s"), newCIURL.c_str(), 
		buildNumber, dlFileName.c_str());
	
	SetProgress(75);
	OnCheckComplete(buildNumber, dlURL);
	return (ExitCode)1;
}

int CheckUpdateTask::GetBuildNumber(const wxString &mainPageJSON)
{
	using namespace boost::property_tree;
	
	try
	{
		ptree pt;
		std::stringstream inStream(stdStr(mainPageJSON), std::ios::in);
		read_json(inStream, pt);
		
		return pt.get<int>("number");
	}
	catch (json_parser_error e)
	{
		wxLogError(_("Failed to check for updates.\nJSON parser error at line %i: %s"), 
			e.line(), wxStr(e.message()).c_str());
		return 0;
	}
	
}

void CheckUpdateTask::OnCheckComplete(int buildNumber, wxString downloadURL)
{
	CheckUpdateEvent event(this, buildNumber, downloadURL);
	m_buildNumber = buildNumber;
	m_downloadURL = downloadURL;
	m_evtHandler->AddPendingEvent(event);
}
