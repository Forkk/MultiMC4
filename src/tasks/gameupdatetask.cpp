/*
    Copyright 2012 <copyright holder> <email>

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/


#include "gameupdatetask.h"
#include <apputils.h>
#include <wx/wfstream.h>
#include <curl/curl.h>
#include <curl/easy.h>
#include <boost/property_tree/ini_parser.hpp>

DEFINE_EVENT_TYPE(wxEVT_GAME_UPDATE_COMPLETE)

GameUpdateTask::GameUpdateTask(Instance *inst, 
							   wxString latestVersion, 
							   wxString mainGameURL, 
							   bool forceUpdate)
	: Task()
{
	m_inst = inst;
	m_latestVersion = latestVersion;
	m_mainGameURL = mainGameURL;
	m_forceUpdate = forceUpdate;
}

void GameUpdateTask::TaskStart()
{
	LoadJarURLs();
	SetProgress(5);
	
	if (!m_inst->GetBinDir().DirExists())
		m_inst->GetBinDir().Mkdir();
	
	wxFileName binDir = m_inst->GetBinDir();
	
	if (!m_latestVersion.empty())
	{
		wxFileName versionFile = m_inst->GetVersionFile();
		bool cacheAvailable = false;
		
		if (!m_forceUpdate && versionFile.FileExists() && 
			(m_latestVersion == _("-1") || m_latestVersion == m_inst->ReadVersionFile()))
		{
			cacheAvailable = true;
			SetProgress(90);
		}
		
		if (m_forceUpdate || !cacheAvailable)
		{
			m_shouldUpdate = true;
			if (!m_forceUpdate && versionFile.FileExists())
			{
				AskToUpdate();
			}
			
			// This check is not actually stupid. 
			// The AskToUpdate method will set m_shouldUpdate to true or false depending 
			// on whether or not the user wants to update.
			if (m_shouldUpdate)
			{
				m_inst->WriteVersionFile(m_latestVersion);
				DownloadJars();
			}
		}
	}
}

void GameUpdateTask::LoadJarURLs()
{
	SetState(STATE_DETERMINING_PACKAGES);
	wxString jarList[] =
	{ 
		m_mainGameURL, _("lwjgl_util.jar"), _("jinput.jar"), _("lwjgl.jar")
	};
	
	wxString mojangURL = _("http://s3.amazonaws.com/MinecraftDownload/");
	
	for (int i = 0; i < jarURLs.size() - 1; i++)
	{
		wxString url = (mojangURL + jarList[i]);
		this->jarURLs[i] = url;
	}
	
	wxString nativeJar = wxEmptyString;
	wxOperatingSystemId osID = wxPlatformInfo::Get().GetOperatingSystemId();
	if (ENUM_CONTAINS(osID, wxOS_WINDOWS))
	{
		nativeJar = _("windows_natives.jar");
	}
	else if (ENUM_CONTAINS(osID, wxOS_MAC))
	{
		nativeJar = _("macosx_natives.jar");
	}
	else if (ENUM_CONTAINS(osID, wxOS_UNIX_LINUX))
	{
		nativeJar = _("linux_natives.jar");
	}
	else
	{
		OnErrorMessage(_("Your operating system does not support minecraft."));
		Cancel();
	}
	
	jarURLs[jarURLs.size() - 1] = mojangURL + nativeJar;
}

void GameUpdateTask::AskToUpdate()
{
	// TODO Ask to update.
	
	// For now, we'll just assume the user doesn't want to update.
	m_shouldUpdate = false;
}

void GameUpdateTask::DownloadJars()
{
	using boost::property_tree::ptree;
	ptree md5s;
	
	wxFileName md5File(m_inst->GetBinDir().GetFullPath(), _("md5s"));
	if (md5File.FileExists())
		read_ini(stdStr(md5File.GetFullPath()), md5s);
	
	SetState(STATE_DOWNLOADING);
	
	int totalDownloadSize = 0;
	int *fileSizes = new int[jarURLs.size()];
	bool *skip = new bool[jarURLs.size()];
	
	// Compare MD5s and skip ones that match.
	for (int i = 0; i < jarURLs.size(); i++)
	{
		wxString etagOnDisk = wxStr(md5s.get<std::string>(
			stdStr(jarURLs[i].GetPath()), ""));
		
		struct curl_slist *headers = NULL;
		headers = curl_slist_append(headers, stdStr(_("If-None-Match: ") + etagOnDisk).c_str());
		
		std::string urlStr = stdStr(jarURLs[i].GetURL()).c_str();
		
		CURL *curl = curl_easy_init();
		curl_easy_setopt(curl, CURLOPT_HEADER, true);
		curl_easy_setopt(curl, CURLOPT_URL, urlStr.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlCallback);
		curl_easy_setopt(curl, CURLOPT_NOBODY, true);
		
#ifdef HTTPDEBUG
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
#endif
		
		if (curl_easy_perform(curl) != 0)
		{
			
		}
		
		long response = 0;
		double contentLen = 0;
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response);
		curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &contentLen);
		
		if (response == 300 && !m_forceUpdate)
			skip[i] = true;
		
		fileSizes[i] = contentLen;
		totalDownloadSize += contentLen;
		
		curl_easy_cleanup(curl);
	}
	
	int initialProgress = 10;
	SetProgress(initialProgress);
	
	// Download jars
	for (int i = 0; i < jarURLs.size(); i++)
	{
		// Skip this file because we already have it.
		if (skip[i])
		{
			SetProgress(initialProgress + fileSizes[i] * 
				initialProgress / totalDownloadSize);
		}
		else
		{
			wxURL currentFile = jarURLs[i];
			wxFileName dlDest(m_inst->GetBinDir().GetFullPath(), 
							  wxFileName(currentFile.GetPath()).GetFullName());
			
			if (currentFile.GetURL().Contains(_("minecraft.jar")) && 
				m_inst->GetMCBackup().FileExists())
			{
				wxRemoveFile(m_inst->GetMCBackup().GetFullPath());
			}
			
			int downloadTries = 0;
		DownloadFile:
			if (downloadTries >= 5)
			{
				OnErrorMessage(_("Failed to download ") + currentFile.GetURL());
				Cancel();
				TestDestroy();
			}
			
			downloadTries++;
			
			char *buffer = new char[1024];
			int bytesRead = 0;
			int dlSize = 0;
			
			wxInputStream *downStream = currentFile.GetInputStream();
			wxFileOutputStream outStream(dlDest.GetFullPath());
			
			while (!downStream->Eof())
			{
				downStream->Read(buffer, sizeof(buffer));
				outStream.Write(buffer, downStream->LastRead());
				dlSize += downStream->LastRead();
				
				SetProgress(((initialProgress / dlSize) * 
							(100 - initialProgress)) / totalDownloadSize);
			}
			
			wxDELETE(downStream);
			
			// TODO Verify the MD5 of the downloaded file.
		}
	}
}

void GameUpdateTask::SetState(UpdateState state)
{
	switch (state)
	{
	case STATE_INIT:
		SetStatus(_T("Initializing..."));
		break;
		
	case STATE_DETERMINING_PACKAGES:
		SetStatus(_T("Determining packages to load..."));
		break;
		
	case STATE_CHECKING_CACHE:
		SetStatus(_T("Checking cache for existing files..."));
		break;
		
	case STATE_DOWNLOADING:
		SetStatus(_T("Downloading packages..."));
		break;
		
	case STATE_EXTRACTING_PACKAGES:
		SetStatus(_T("Extracting downloaded packages..."));
		break;
		
	default:
		break;
	}
}

void GameUpdateTask::OnGameUpdateComplete()
{
	TaskEvent event(wxEVT_GAME_UPDATE_COMPLETE, this);
	m_evtHandler->AddPendingEvent(event);
}

size_t CurlCallback(void* buffer, size_t size, size_t nmemb, void* userp)
{

}
