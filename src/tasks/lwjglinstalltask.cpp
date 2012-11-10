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

#include "lwjglinstalltask.h"
#include <fsutils.h>
#include <wx/wfstream.h>
#include <wx/dir.h>

#include <apputils.h>

#include <set>
#include <memory>

#include "instance.h"
#include "apputils.h"

LWJGLInstallTask::LWJGLInstallTask(Instance *inst, const wxString &path)
{
	m_inst = inst;
	m_path = path;
}

wxThread::ExitCode LWJGLInstallTask::TaskStart()
{
	SetStatus(_("Installing new LWJGL..."));

	wxFFileInputStream in(m_path);
	wxZipInputStream zipIn(in);

	// make sure the instance natives dir is there (and all the dirs leading to it from root)
	wxString nativesPath = Path::Combine(m_inst->GetBinDir(), "natives");
	bool success = wxDir::Make(nativesPath,wxS_DIR_DEFAULT,wxPATH_MKDIR_FULL);
	if(!success)
		return (wxThread::ExitCode) 0;
	
	std::auto_ptr<wxZipEntry> entry;
	while (entry.reset(zipIn.GetNextEntry()), entry.get() != NULL)
	{
		if (entry->IsDir())
			continue;

		const wxString jarNames[] = { "jinput.jar", "lwjgl_util.jar", "lwjgl.jar" };

		wxString name = entry->GetName();
		wxString destFileName;

		// Put things in their places.
		for (int i = 0; i < 3; i++)
			if (name.EndsWith(jarNames[i]))
				destFileName = Path::Combine(m_inst->GetBinDir(), jarNames[i]);

		// If we haven't found a jar file, look for natives.
		if (destFileName.IsEmpty())
		{
#if WINDOWS
			wxString nativesDir = "windows";
#elif OSX
			wxString nativesDir = "macosx";
#elif LINUX
			wxString nativesDir = "linux";
#endif
			if (name.Contains(nativesDir))
			{
				destFileName = Path::Combine(nativesPath, name.AfterLast('/').AfterLast('\\'));
			}
		}

		// Now if destFileName is still empty, go to the next file.
		if (!destFileName.IsEmpty())
		{
			SetStatus(_("Installing new LWJGL - Extracting " + name));
			wxFFileOutputStream out(destFileName);
			zipIn.Read(out);
		}
	}

	return (ExitCode)1;
}
