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

#include "moddertask.h"
#include <fsutils.h>
#include <wx/wfstream.h>
#include <wx/fs_mem.h>
#include <apputils.h>

#include <set>
#include <memory>

ModderTask::ModderTask(Instance* inst)
	: Task()
{
	m_inst = inst;
	step = 0;
}

wxThread::ExitCode ModderTask::TaskStart()
{
	// Get the mod list
	ModList *modList = m_inst->GetModList();
	
	wxFileName mcBin = m_inst->GetBinDir();
	wxFileName mcJar = m_inst->GetMCJar();
	wxFileName mcBackup = m_inst->GetMCBackup();
	
	SetStatus(_("Installing mods - backing up minecraft.jar..."));
	if (!mcBackup.FileExists() && !wxCopyFile(mcJar.GetFullPath(), mcBackup.GetFullPath()))
	{
		OnFail(_("Failed to back up minecraft.jar"));
		return (ExitCode)0;
	}
	
	if (mcJar.FileExists() && !wxRemoveFile(mcJar.GetFullPath()))
	{
		OnFail(_("Failed to delete old minecraft.jar"));
		return (ExitCode)0;
	}
	
	
	// TODO: good spot for user cancel check? or not...
	
	TaskStep(); // STEP 1
	SetStatus(_("Installing mods - Opening minecraft.jar"));

	wxFFileOutputStream jarStream(mcJar.GetFullPath());
	wxZipOutputStream zipOut(jarStream);

	// Files already added to the jar.
	// These files will be skipped.
	std::set<wxString> addedFiles;

	// Modify the jar
	TaskStep(); // STEP 2
	SetStatus(_("Installing mods - Adding mod files..."));
	for (ModList::const_reverse_iterator iter = modList->rbegin(); iter != modList->rend(); iter++)
	{
		wxFileName modFileName = iter->GetFileName();
		SetStatus(_("Installing mods - Adding ") + modFileName.GetFullName());
		if (iter->IsZipMod())
		{
			wxFFileInputStream modStream(modFileName.GetFullPath());
			wxZipInputStream zipStream(modStream);
			std::auto_ptr<wxZipEntry> entry;
			while (entry.reset(zipStream.GetNextEntry()), entry.get() != NULL)
			{
				if (entry->IsDir())
					continue;

				wxString name = entry->GetName();
				if (addedFiles.count(name) == 0)
				{
					if (!zipOut.CopyEntry(entry.release(), zipStream))
						break;
					addedFiles.insert(name);
				}
			}
		}
		else
		{
			wxFileName destFileName = modFileName;
			destFileName.MakeRelativeTo(m_inst->GetInstModsDir().GetFullPath());
			wxString destFile = destFileName.GetFullPath();

			if (addedFiles.count(destFile) == 0)
			{
				wxFFileInputStream input(modFileName.GetFullPath());
				zipOut.PutNextEntry(destFile);
				zipOut.Write(input);

				addedFiles.insert(destFile);
			}
		}
	}

	{
		wxFFileInputStream inStream(mcBackup.GetFullPath());
		wxZipInputStream zipIn(inStream);

		std::auto_ptr<wxZipEntry> entry;
		while (entry.reset(zipIn.GetNextEntry()), entry.get() != NULL)
		{
			wxString name = entry->GetName();

			if (!name.Matches(_("META-INF*")) &&
				addedFiles.count(name) == 0)
			{
				if (!zipOut.CopyEntry(entry.release(), zipIn))
					break;
				addedFiles.insert(name);
			}
		}
	}
	
	// Recompress the jar
	TaskStep(); // STEP 3
	SetStatus(_("Installing mods - Recompressing jar..."));

	m_inst->SetNeedsRebuild(false);
	return (ExitCode)1;
}


void ModderTask::TaskStep()
{
	step++;
	int p = (int)(((float)step / 4.0f) * 100);
	SetProgress(p);
}

void ModderTask::OnFail(const wxString &errorMsg)
{
	SetStatus(errorMsg);
	wxSleep(3);
	EmitErrorMessage(errorMsg);
}
