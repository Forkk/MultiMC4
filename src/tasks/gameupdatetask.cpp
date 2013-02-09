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

#include "gameupdatetask.h"


#include <wx/wfstream.h>
#include <wx/sstream.h>
#include <wx/zipstrm.h>

#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>

#include <md5/md5.h>
#include "bspatch.h"

#include "utils/curlutils.h"
#include "utils/apputils.h"
#include "utils/httputils.h"
#include <mcversionlist.h>

const wxString mcnwebURL = "http://sonicrules.org/mcnweb.py";

DEFINE_EVENT_TYPE(wxEVT_GAME_UPDATE_COMPLETE)

GameUpdateTask::GameUpdateTask(Instance *inst, int64_t latestVersion, bool forceUpdate)
	: Task(), m_inst(inst), m_latestVersion(latestVersion), m_forceUpdate(forceUpdate) {}

GameUpdateTask::~GameUpdateTask() {}

wxThread::ExitCode GameUpdateTask::TaskStart()
{
	wxString intendedVersion = m_inst->GetIntendedVersion();
	wxString currentVersion = m_inst->GetJarVersion();
	
	
	if(!m_inst->GetShouldUpdate() && !m_forceUpdate)
		return (ExitCode)1;
	
	SetState(STATE_DETERMINING_PACKAGES);
	
	MCVersionList & vlist = MCVersionList::Instance();
	vlist.LoadIfNeeded();
	
	MCVersion * ver = vlist.GetVersion(intendedVersion);
	
	if(!ver)
	{
		vlist.SetNeedsNostalgia();
		vlist.LoadIfNeeded();
		ver = vlist.GetVersion(intendedVersion);
		if(!ver)
			return (ExitCode)0;
	}
	
	bool do_patching = false;
	
	MCVersion * real_ver;
	wxString patch_version;
	wxString patchBaseURL;
	using namespace boost::property_tree;
	ptree mcn_checksum_data;
	
	// get the mcnostalgia stuff (optionally) and determine what version of MC do we actually want
	if(ver->GetVersionType() == MCNostalgia)
	{
		do_patching = true;
		patch_version = ver->GetDescriptor();
		if (!DownloadPatches(patch_version))
			return (ExitCode)false;
		wxString str;
		try
		{
			read_json("patches/checksum.json", mcn_checksum_data);
			str = wxStr(mcn_checksum_data.get<std::string>("CurrentVersion.version"));
		}
		catch (json_parser_error e)
		{
			wxLogError(_("Failed to check MC version.\nJSON parser error at line %i: %s"), 
				e.line(), wxStr(e.message()).c_str());
			return (ExitCode)false;
		}
		catch (ptree_error e)
		{
			wxLogError(_("Unspecified error: %s"), e.what());
			return (ExitCode)false;
		}
		real_ver = vlist.GetVersion(str);
	}
	else
	{
		real_ver = ver;
	}
	
	wxString mojangURL ("http://s3.amazonaws.com/MinecraftDownload/");
	jarURLs.clear();
	jarURLs.push_back(real_ver->GetDLUrl() + "minecraft.jar");
	jarURLs.push_back(mojangURL + "lwjgl_util.jar");
	jarURLs.push_back(mojangURL + "jinput.jar");
	jarURLs.push_back(mojangURL + "lwjgl.jar");

	wxString nativeJar;
#if WINDOWS
		nativeJar = "windows_natives.jar";
#elif OSX
		nativeJar = "macosx_natives.jar";
#elif LINUX
		nativeJar = "linux_natives.jar";
#else
#error Detected unsupported OS.
#endif
	jarURLs.push_back(mojangURL + nativeJar);
	
	SetProgress(5);
	
	wxFileName binDir = m_inst->GetBinDir();
	if (!binDir.DirExists())
		binDir.Mkdir();

	if(real_ver->GetVersionType() == CurrentStable)
		m_inst->WriteVersionFile(m_latestVersion);
	else
		m_inst->WriteVersionFile(real_ver->GetTimestamp() * 1000);
	DownloadJars();
	ExtractNatives();
	wxRemoveFile(Path::Combine(m_inst->GetBinDir(), wxFileName(jarURLs[jarURLs.size() - 1]).GetFullName()));
	m_inst->SetNeedsRebuild(true);
	
	// apply MCNostalgia patches
	bool success = true;
	if(do_patching)
	{
		if(!ApplyPatches())
		{
			wxLogError(_("Something went terribly wrong while patching the jar with MCNostalgia."));
			success = false;
		}
	}
	m_inst->UpdateVersion(false);
	m_inst->SetShouldUpdate(false);
	return (ExitCode)success;
}

void GameUpdateTask::DownloadJars()
{
	using namespace boost::property_tree;
	ptree md5s;
	
	wxFileName md5File(m_inst->GetBinDir().GetFullPath(), "md5sums");
	if (md5File.FileExists())
	{
		try
		{
			read_ini(stdStr(md5File.GetFullPath()), md5s);
		}
		catch (ini_parser_error e)
		{
			wxLogError(_("Failed to parse md5s file.\nINI parser error at line %i: %s"), 
				e.line(), wxStr(e.message()).c_str());
		}
	}
	
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
		headers = curl_slist_append(headers, stdStr("If-None-Match: " + etagOnDisk).c_str());
		
		CURL *curl = InitCurlHandle();
		curl_easy_setopt(curl, CURLOPT_HEADER, true);
		curl_easy_setopt(curl, CURLOPT_URL, TOASCII(jarURLs[i]));
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

			SetState(STATE_DOWNLOADING, wxFileName(currentFile.GetPath()).GetFullName());

			wxFileName dlDest(m_inst->GetBinDir().GetFullPath(), 
							  wxFileName(currentFile.GetPath()).GetFullName());
			
			if (currentFile.GetURL().Contains("minecraft.jar") && 
				m_inst->GetMCBackup().FileExists())
			{
				wxRemoveFile(m_inst->GetMCBackup().GetFullPath());
			}
			
			const int maxDownloadTries = 5;
			int downloadTries = 0;
		DownloadFile:
			if (downloadTries >= maxDownloadTries)
			{
				EmitErrorMessage(_("Failed to download ") + currentFile.GetURL());
				return;
			}
			
			downloadTries++;
			
			unsigned char md5digest[16];
			int currentDownloadedSize = 0;
			
			CURL *curl = InitCurlHandle();
			curl_easy_setopt(curl, CURLOPT_URL, TOASCII(currentFile.GetURL()));
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
			
			MD5Final(md5digest, &md5ctx);
			
// 			printf("MD5 for file %s: %s\n", 
// 				   stdStr(currentFile.GetURL()).c_str(),
// 				   stdStr(Utils::BytesToString(md5digest)).c_str());
			
			// Find the "ETag" in the headers
			const wxString etagHeader = "ETag: \"";
			
			size_t etagStart = headerContent.find(etagHeader) + etagHeader.Len();
			size_t etagEnd = headerContent.find("\"", etagStart) - 1;
			wxString etag = headerContent.SubString(etagStart, etagEnd);
			
			wxString md5sum = Utils::BytesToString(md5digest);
			
// 			printf("ETag:\t%s\n", cStr(etag));
// 			printf("MD5:\t%s\n", cStr(Utils::BytesToString(md5digest)));
			
			bool md5matches = md5sum.IsSameAs(etag, false);
			
			if (md5matches)
			{
				wxString keystr = wxFileName(currentFile.GetPath()).GetName();
				std::string key(TOASCII(keystr));
				// ASCII is fine. it's lower case letters and numbers
				std::string value (TOASCII(md5sum));
				md5s.put<std::string>(key, value);
				std::ofstream out;
				out.open(md5File.GetFullPath().mb_str());
				write_ini(out, md5s);
				out.flush();
				out.close();
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
															  "natives"));
	
	if (!nativesDir.DirExists())
		nativesDir.Mkdir();
	
	wxFileInputStream jarFileStream(nativesJar.GetFullPath());
	wxZipInputStream zipStream(jarFileStream);
	
	std::auto_ptr<wxZipEntry> entry;
	while (entry.reset(zipStream.GetNextEntry()), entry.get() != NULL)
	{
		if (entry->IsDir() || entry->GetInternalName().Contains("META-INF"))
			continue;
		SetState(STATE_EXTRACTING_PACKAGES, entry->GetName());
		wxFileName destFile(Path::Combine(nativesDir, entry->GetName()));
		wxFileOutputStream outStream(destFile.GetFullPath());
		outStream.Write(zipStream);
	}
}

bool GameUpdateTask::DownloadPatches(const wxString& mcVersion)
{
	//SetStep(STEP_DOWNLOAD_PATCHES);

	const int patchURLCount = 2;
	const wxString patchURLs[patchURLCount] =
	{ 
		"minecraft.ptch", 
		/*"lwjgl.ptch",
		"lwjgl_util.ptch",
		"jinput.ptch",*/
		"checksum.json",
	};

	// Get the base URL.
	wxString baseURL;
	//SetStatusDetail(_("Getting patch URLs."));
	if (!RetrievePatchBaseURL(mcVersion, &baseURL))
		return false;

	// Download the patches.
	for (int i = 0; i < patchURLCount; i++)
	{
		//SetStatusDetail(_("Downloading ") + patchURLs[i]);
		wxString downloadURL = baseURL + "/" + patchURLs[i];
		wxString dest = Path::Combine("patches", patchURLs[i]);

		CURL *curl = InitCurlHandle();

		curl_easy_setopt(curl, CURLOPT_URL, TOASCII(downloadURL));
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlLambdaCallback);

		wxFFileOutputStream outStream(dest);
		CurlLambdaCallbackFunction curlWrite = [&] (void *buffer, size_t size) -> size_t
		{
			outStream.Write(buffer, size);
			return outStream.LastWrite();
		};
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &curlWrite);

		int curlErr = curl_easy_perform(curl);
		curl_easy_cleanup(curl);

		if (curlErr != 0)
		{
			wxLogError(_("Failed to download patch %s."), patchURLs[i].c_str());
			return false;
		}
	}
	return true;
}

bool GameUpdateTask::ApplyPatches()
{
	const int patchFileCount = 1;
	const wxString patchFiles[] =
	{ 
		"minecraft", 
		/*"lwjgl",
		"lwjgl_util",
		"jinput",*/
	};

	//SetStep(STEP_APPLY_PATCHES);

	for (int i = 0; i < patchFileCount; i++)
	{
		wxString file = patchFiles[i];

		wxString binDir = m_inst->GetBinDir().GetFullPath();
		wxString patchFile = Path::Combine("patches", file + ".ptch");

		if (file == "minecraft" && wxFileExists(m_inst->GetMCBackup().GetFullPath()))
			file = "mcbackup";
		wxString patchSrc = Path::Combine(binDir, file + ".jar");
		wxString patchDest = Path::Combine(binDir, file + "_new.jar");

		if (wxFileExists(patchDest))
			wxRemoveFile(patchDest);

		int err = bspatch(patchSrc.char_str(), patchDest.char_str(), patchFile.char_str());

		if (err == ERR_NONE)
		{
			wxRemoveFile(patchSrc);
			wxRename(patchDest, patchSrc);
		}
		else
		{
			switch (err)
			{
			case ERR_CORRUPT_PATCH:
				wxLogError(_("Failed to patch %s.jar. Patch is corrupt."), file.c_str());
				break;

			default:
				wxLogError(_("Failed to patch %s.jar. Unknown error."), file.c_str());
				break;
			}
			return false;
		}
	}
	return true;
}

bool GameUpdateTask::VerifyPatchedFiles()
{

	//SetStep(STEP_VERIFY_FILES2);

	const int patchFileCount = 1;
	const wxString patchFiles[] =
	{ 
		"minecraft", 
		/*"lwjgl",
		"lwjgl_util",
		"jinput",*/
	};

	using namespace boost::property_tree;
	try
	{
		ptree pt;
		read_json("patches/checksum.json", pt);

		for (int i = 0; i < patchFileCount; i++)
		{
			//SetStatusDetail(_("Verifying ") + patchFiles[i]);

			wxString cFileName = patchFiles[i];
			if (cFileName == "minecraft" && wxFileExists(Path::Combine(m_inst->GetBinDir(), "mcbackup.jar")))
				cFileName = "mcbackup";
			wxString checkFile = Path::Combine(m_inst->GetBinDir(), cFileName + ".jar");

			// Verify the file's MD5 sum.
			MD5Context md5ctx;
			MD5Init(&md5ctx);

			char buf[1024];
			wxFFileInputStream fileIn(checkFile);

			while (!fileIn.Eof())
			{
				fileIn.Read(buf, 1024);
				MD5Update(&md5ctx, (unsigned char*)buf, fileIn.LastRead());
			}

			unsigned char md5digest[16];
			MD5Final(md5digest, &md5ctx);

			if (!Utils::BytesToString(md5digest).IsSameAs(
				wxStr(pt.get<std::string>("OldVersion." + stdStr(patchFiles[i]))), false))
			{
				wxLogWarning(_("The MD5 sum of %s didn't match what it was supposed to be after patching!"), 
					wxFileName(checkFile).GetFullName().c_str());
			}
		}
	}
	catch (json_parser_error e)
	{
		wxLogError(_("Failed to check file MD5.\nJSON parser error at line %i: %s"), 
			e.line(), wxStr(e.message()).c_str());
		return false;
	}
	catch (ptree_error e)
	{
		wxLogError(_("Unspecified error: %s"), e.what());
		return false;
	}

	return true;
}

bool GameUpdateTask::RetrievePatchBaseURL(const wxString& mcVersion, wxString *patchURL)
{
	wxString json;
	if (DownloadString(wxString::Format("%s?pversion=1&mcversion=%s", 
		mcnwebURL.c_str(), mcVersion.c_str()), &json))
	{
		using namespace boost::property_tree;
		try
		{
			ptree pt;
			std::stringstream jsonStream(stdStr(json), std::ios::in);
			read_json(jsonStream, pt);

			*patchURL = wxStr(pt.get<std::string>("url"));
		}
		catch (json_parser_error e)
		{
			wxLogError(_("Failed to get patch URL.\nJSON parser error at line %i: %s"), 
				e.line(), wxStr(e.message()).c_str());
			return false;
		}
		catch (ptree_error e)
		{
			wxLogError(_("Unspecified error: %s"), e.what());
			return false;
		}
		return true;
	}
	else
	{
		return false;
	}
}


void GameUpdateTask::SetState(UpdateState state, const wxString& msg)
{
	switch (state)
	{
	case STATE_INIT:
		if (msg == wxEmptyString)
			SetStatus(_("Initializing..."));
		else
			SetStatus(_("Initializing: ") + msg);
		break;
		
	case STATE_DETERMINING_PACKAGES:
		if (msg == wxEmptyString)
			SetStatus(_("Determining packages to load..."));
		else
			SetStatus(_("Determining packages to load: ") + msg);
		break;
		
	case STATE_CHECKING_CACHE:
		if (msg == wxEmptyString)
			SetStatus(_("Checking cache for existing files..."));
		else
			SetStatus(_("Checking cache for existing files: ") + msg);
		break;
		
	case STATE_DOWNLOADING:
		if (msg == wxEmptyString)
			SetStatus(_("Downloading packages..."));
		else
			SetStatus(_("Downloading packages: ") + msg);
		break;
		
	case STATE_EXTRACTING_PACKAGES:
		if (msg == wxEmptyString)
			SetStatus(_("Extracting downloaded packages..."));
		else
			SetStatus(_("Extracting downloaded packages: ") + msg);
		break;
		
	default:
		break;
	}
}
