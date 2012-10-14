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

	if(buildNumber == -1)
	{
		wxFileName fname("JsonDUMP.txt");
		fname.MakeAbsolute();
		wxFile dump("JsonDUMP.txt",wxFile::write);
		
		dump.Write(mainPageJSON,wxMBConvStrictUTF8());
		wxString err = "Failed to check for updates. The update server is likely down. Please try later.\n\n";
		err << "If you have good reasons to believe that the problem is not on the server end, please ";
		err << "report a bug and attach JsonDUMP.txt to it. (it should be in MultiMC's folder)";
		wxLogError(err);
		dump.Flush();
		dump.Close();
		return (ExitCode)0;
	}
	
	// Figure out where to download the latest update.
#if WINDOWS
	wxString dlFileName = _("MultiMC.exe");
#else
	wxString dlFileName = _("MultiMC");
#endif

	wxString newCIURL = ciURL;
	if (newCIURL.EndsWith("/"))
		newCIURL.RemoveLast();
	
	wxString dlURL = wxString::Format(_("%s/%i/artifact/%s"), newCIURL.c_str(), buildNumber, dlFileName.c_str());
	
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
		return -1;
	}
	
}

void CheckUpdateTask::OnCheckComplete(int buildNumber, wxString downloadURL)
{
	CheckUpdateEvent event(this, buildNumber, downloadURL);
	m_buildNumber = buildNumber;
	m_downloadURL = downloadURL;
	m_evtHandler->AddPendingEvent(event);
}
