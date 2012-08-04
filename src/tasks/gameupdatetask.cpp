/*
    Copyright 2012 Andrew Okin

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
#include <wx/sstream.h>
#include <wx/zipstrm.h>

#include <boost/property_tree/ini_parser.hpp>
// #include <boost/regex.hpp>

#include <md5/md5.h>

#include "curlutils.h"

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

GameUpdateTask::~GameUpdateTask()
{
	
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
				if (CheckUserCancelled()) return;
				DownloadJars();
				if (CheckUserCancelled()) return;
				ExtractNatives();
				if (CheckUserCancelled()) return;
				wxRemoveFile(Path::Combine(m_inst->GetBinDir(), 
					wxFileName(jarURLs[jarURLs.size() - 1]).GetFullName()));
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
	
	for (size_t i = 0; i < jarURLs.size() - 1; i++)
	{
		wxString url = (mojangURL + jarList[i]);
		this->jarURLs[i] = url;
	}
	
	wxString nativeJar = wxEmptyString;
	wxOperatingSystemId osID = wxPlatformInfo::Get().GetOperatingSystemId();
	if (IS_WINDOWS())
	{
		nativeJar = _("windows_natives.jar");
	}
	else if (IS_MAC())
	{
		nativeJar = _("macosx_natives.jar");
	}
	else if (IS_LINUX())
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
	for (size_t i = 0; i < jarURLs.size(); i++)
	{
		wxString etagOnDisk = wxStr(md5s.get<std::string>(
			stdStr(wxURL(jarURLs[i]).GetPath()), ""));
		
		struct curl_slist *headers = NULL;
		headers = curl_slist_append(headers, stdStr(_("If-None-Match: ") + etagOnDisk).c_str());
		
		CURL *curl = curl_easy_init();
		curl_easy_setopt(curl, CURLOPT_HEADER, true);
		curl_easy_setopt(curl, CURLOPT_URL, jarURLs[i].ToAscii().data());
		curl_easy_setopt(curl, CURLOPT_NOBODY, true);
		
#ifdef HTTPDEBUG
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL);
#else
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlBlankCallback);
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
		else
			skip[i] = false;
		
		fileSizes[i] = contentLen;
		totalDownloadSize += contentLen;
		
		curl_easy_cleanup(curl);
	}
	
	int initialProgress = 10;
	SetProgress(initialProgress);
	
	// Download jars
	int totalDownloadedSize = 0;
	for (size_t i = 0; i < jarURLs.size(); i++)
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
			
			const int maxDownloadTries = 5;
			int downloadTries = 0;
		DownloadFile:
			if (downloadTries >= maxDownloadTries)
			{
				OnErrorMessage(_("Failed to download ") + currentFile.GetURL());
				Cancel();
				TestDestroy();
			}
			
			downloadTries++;
			
			unsigned char md5digest[16];
			int currentDownloadedSize = 0;
			
			CURL *curl = curl_easy_init();
			curl_easy_setopt(curl, CURLOPT_URL, currentFile.GetURL().ToAscii().data());
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlLambdaCallback);
			curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, CurlLambdaCallback);
			
			wxFileOutputStream outStream(dlDest.GetFullPath());
			MD5Context md5ctx;
			MD5Init(&md5ctx);
			
			bool cancel = false;
			CurlLambdaCallbackFunction curlWrite = [&] (void *buffer, size_t size) -> size_t
			{
				currentDownloadedSize += size;
				totalDownloadedSize += size;
				
				outStream.Write(buffer, size);
				MD5Update(&md5ctx, (unsigned char*)buffer, size);
				
				if (CheckUserCancelled())
				{
					cancel = true;
					return 0;
				}
				
				SetProgress(initialProgress + 
					((double)totalDownloadedSize / (double)totalDownloadSize) * 
					(100 - initialProgress - 10));
				
				return outStream.LastWrite();
			};
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &curlWrite);
			
			wxString headerContent;
			wxStringOutputStream headerStream(&headerContent);
			CurlLambdaCallbackFunction curlWriteHeaders = [&] (void *buffer, size_t size) -> size_t
			{
				headerStream.Write(buffer, size);
				return headerStream.LastWrite();
			};
			curl_easy_setopt(curl, CURLOPT_HEADERDATA, &curlWriteHeaders);
			
			int errorCode = curl_easy_perform(curl);
			curl_easy_cleanup(curl);
			
			if (CheckUserCancelled())
			{
				return;
			}
			
			MD5Final(md5digest, &md5ctx);
			
// 			printf("MD5 for file %s: %s\n", 
// 				   stdStr(currentFile.GetURL()).c_str(),
// 				   stdStr(Utils::BytesToString(md5digest)).c_str());
			
			// Find the "ETag" in the headers
			const wxString etagHeader = _("ETag: \"");
			
			size_t etagStart = headerContent.find(etagHeader) + etagHeader.Len();
			size_t etagEnd = headerContent.find(_("\""), etagStart) - 1;
			wxString etag = headerContent.SubString(etagStart, etagEnd);
			
			wxString md5sum = Utils::BytesToString(md5digest);
			
// 			printf("ETag:\t%s\n", cStr(etag));
// 			printf("MD5:\t%s\n", cStr(Utils::BytesToString(md5digest)));
			
			bool md5matches = md5sum.IsSameAs(etag, false);
			
			if (md5matches)
			{
				wxString keystr = wxFileName(currentFile.GetPath()).GetName();
				std::string key(keystr.fn_str().data());
				// ASCII is fine. it's lower case letters and numbers
				std::string value (md5sum.ToAscii().data());
				md5s.put<std::string>(key, value);
				std::string stlstring = std::string(md5File.GetFullPath().mb_str());
				write_ini(stlstring, md5s);
			}
			else
			{
				totalDownloadedSize -= currentDownloadedSize;
				goto DownloadFile;
			}
		}
	}
}

void GameUpdateTask::ExtractNatives()
{
	SetState(STATE_EXTRACTING_PACKAGES);
	SetProgress(90);
	
	wxFileName nativesJar(Path::Combine(m_inst->GetBinDir(), 
		wxFileName(jarURLs[jarURLs.size() - 1]).GetFullName()));
	wxFileName nativesDir = wxFileName::DirName(Path::Combine(m_inst->GetBinDir(), 
															  _("natives")));
	
	if (!nativesDir.DirExists())
		nativesDir.Mkdir();
	
	wxFileInputStream jarFileStream(nativesJar.GetFullPath());
	wxZipInputStream zipStream(jarFileStream);
	
	std::auto_ptr<wxZipEntry> entry;
	while (entry.reset(zipStream.GetNextEntry()), entry.get() != NULL)
	{
		if (entry->IsDir() || entry->GetInternalName().Contains(_("META-INF")))
			continue;
		wxFileName destFile(Path::Combine(nativesDir, entry->GetName()));
		wxFileOutputStream outStream(destFile.GetFullPath());
		outStream.Write(zipStream);
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
