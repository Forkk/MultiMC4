// 
//  Copyright 2012 Andrew Okin
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

#include <boost/property_tree/json_parser.hpp>

#include <sstream>

DEFINE_EVENT_TYPE(wxEVT_CHECK_UPDATE)

const wxString ciURL = _("http://forkk.net:8080/job/MultiMC4/");

CheckUpdateTask::CheckUpdateTask()
	: Task()
{
	
}

void CheckUpdateTask::TaskStart()
{
	SetStatus(_("Getting version info..."));
	
	// Get the main page for the project
	wxString mainPageJSON;
	if (!DownloadString(ciURL + _("api/json"), &mainPageJSON))
	{
		OnErrorMessage(_("Failed to check for updates. Please check your internet connection."));
		return;
	}
	
	// Determine the latest stable build.
	int buildNumber = GetBuildNumber(mainPageJSON);
	
	// Figure out where to download the latest update.
	wxString dlFileName;
	if (IS_WINDOWS())
		dlFileName = _("MultiMC.exe");
	else if (IS_LINUX() || IS_MAC())
		dlFileName = _("MultiMC");
	
	wxString osName;
	if (IS_WINDOWS())
		osName = _("Windows");
	else if (IS_LINUX())
		osName = _("Linux");
	else if (IS_MAC())
		osName = _("OSX");
	
	wxString dlURL = wxString::Format(_("%s%i/os=%s/artifact/%s"), ciURL.c_str(), 
		buildNumber, osName.c_str(), dlFileName.c_str());
	
	SetProgress(75);
	OnCheckComplete(buildNumber, dlURL);
}

int CheckUpdateTask::GetBuildNumber(const wxString &mainPageJSON, bool stableOnly)
{
	using namespace boost::property_tree;
	
	ptree pt;
	std::stringstream inStream(stdStr(mainPageJSON), std::ios::in);
	read_json(inStream, pt);
	
	if (stableOnly)
		return pt.get<int>("lastStableBuild.number");
	else
		return pt.get<int>("lastSuccessfulBuild.number");
}

void CheckUpdateTask::OnCheckComplete(int buildNumber, wxString downloadURL)
{
	SetProgress(100);
	OnTaskEnd();
	CheckUpdateEvent event(this, buildNumber, downloadURL);
	m_evtHandler->AddPendingEvent(event);
}
