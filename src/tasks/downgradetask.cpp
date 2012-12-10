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

#include "downgradetask.h"

#include <wx/wfstream.h>

#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>

#include "md5/md5.h"
#include "bspatch.h"

#include "utils/curlutils.h"
#include "utils/httputils.h"
#include "utils/apputils.h"
#include <java/javautils.h>

const wxString mcnwebURL = "http://sonicrules.org/mcnweb.py";

DowngradeTask::DowngradeTask(Instance *inst, const wxString& targetVersion)
{
	m_inst = inst;
	m_targetVersion = targetVersion;
}

wxThread::ExitCode DowngradeTask::TaskStart()
{
	if (!wxDirExists("patches"))
		wxMkdir("patches");

	if (!DownloadPatches())
		return (ExitCode)0;

	if (!VerifyOriginalFiles())
		return (ExitCode)0;

	if (!ApplyPatches())
		return (ExitCode)0;

	if (!VerifyPatchedFiles())
		return (ExitCode)0;

	wxString backup = Path::Combine(m_inst->GetBinDir(), "mcbackup.jar");
	wxString jar = Path::Combine(m_inst->GetBinDir(), "minecraft.jar");
	
	// FIXME: this is still dirty
	if(wxFileExists(backup))
	{
		// if mcbackup.jar exists, we updated that. The minecraft.jar needs rebuilding.
		m_inst->SetNeedsRebuild();
		// determine new intended version based on the backup
		wxString newversion = javautils::GetMinecraftJarVersion(backup);
		m_inst->SetIntendedJarVersion(newversion);
		m_inst->SetJarVersion(newversion);
	}
	else
	{
		// backup doesn't exist. This means we patched vanilla minecraft.jar
		// update the version now based on minecraft.jar.
		m_inst->UpdateVersion();
		// intended version is what we updated to.
		m_inst->SetIntendedJarVersion(m_inst->GetJarVersion());
	}
	SetStep(STEP_DONE);
	return (ExitCode)1;
}

bool DowngradeTask::DownloadPatches()
{
	SetStep(STEP_DOWNLOAD_PATCHES);

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
	SetStatusDetail(_("Getting patch URLs."));
	if (!RetrievePatchBaseURL(m_targetVersion, &baseURL))
		return false;

	// Download the patches.
	for (int i = 0; i < patchURLCount; i++)
	{
		SetStatusDetail(_("Downloading ") + patchURLs[i]);
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

bool DowngradeTask::RetrievePatchBaseURL(const wxString& mcVersion, wxString *patchURL)
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
		return true;
	}
	else
	{
		return false;
	}
}

bool DowngradeTask::VerifyOriginalFiles()
{
	SetStep(STEP_VERIFY_FILES);

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
			SetStatusDetail(_("Verifying ") + patchFiles[i]);

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
				wxStr(pt.get<std::string>("CurrentVersion." + stdStr(patchFiles[i]))), false))
			{
				wxLogError(_("Can't patch because %s has been modified. Try force updating."), 
					wxFileName(checkFile).GetFullName().c_str());
				return false;
			}
		}
	}
	catch (json_parser_error e)
	{
		wxLogError(_("Failed to check file MD5.\nJSON parser error at line %i: %s"), 
			e.line(), wxStr(e.message()).c_str());
		return false;
	}

	return true;
}

bool DowngradeTask::ApplyPatches()
{
	const int patchFileCount = 1;
	const wxString patchFiles[] =
	{ 
		"minecraft", 
		/*"lwjgl",
		"lwjgl_util",
		"jinput",*/
	};

	SetStep(STEP_APPLY_PATCHES);

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

bool DowngradeTask::VerifyPatchedFiles()
{

	SetStep(STEP_VERIFY_FILES2);

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
			SetStatusDetail(_("Verifying ") + patchFiles[i]);

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

	return true;
}

void DowngradeTask::SetStep(TaskStep step, bool clearStatusDetail)
{
	currentStep = step;

	if (clearStatusDetail)
		SetStatusDetail(wxEmptyString);
	else
		UpdateStatus();

	SetProgress((int)(((float)step) / ((float)STEP_DONE) * 100));
}

void DowngradeTask::SetStatusDetail(wxString detail)
{
	statusDetail = detail;
	UpdateStatus();
}

void DowngradeTask::UpdateStatus()
{
	if (statusDetail.IsEmpty())
	{
		switch (currentStep)
		{
		case STEP_DOWNLOAD_PATCHES:
			SetStatus(_("Downloading patches..."));
			break;

		case STEP_VERIFY_FILES:
		case STEP_VERIFY_FILES2:
			SetStatus(_("Verifying files..."));
			break;

		case STEP_APPLY_PATCHES:
			SetStatus(_("Patching files..."));
			break;

		case STEP_DONE:
			SetStatus(_("Done!"));
			break;
		}
	}
	else
	{
		switch (currentStep)
		{
		case STEP_DOWNLOAD_PATCHES:
			SetStatus(_("Downloading patches: ") + statusDetail);
			break;

		case STEP_VERIFY_FILES:
		case STEP_VERIFY_FILES2:
			SetStatus(_("Verifying files: ") + statusDetail);
			break;

		case STEP_APPLY_PATCHES:
			SetStatus(_("Patching files: ") + statusDetail);
			break;

		case STEP_DONE:
			SetStatus(_("Done: ") + statusDetail);
		}
	}
}
