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

#include "filecopytask.h"

#include "apputils.h"
#include "fsutils.h"

#include <wx/string.h>
#include <wx/dir.h>

FileCopyTask::FileCopyTask(const wxFileName &src, const wxFileName &dest)
	: Task()
{
	m_src = src;
	m_dest = dest;
}

wxThread::ExitCode FileCopyTask::TaskStart()
{
	SetStatus(_("Discovering files..."));

	wxArrayString copyFiles;
	if (!DiscoverFiles(m_src.GetFullPath(), copyFiles))
	{
		EmitErrorMessage(_("Failed to read source directory."));
		return (ExitCode)0;
	}

	SetStatus(wxString::Format(_("Copying %i files..."), copyFiles.size()));
	int ctr = 0;
	for (wxArrayString::iterator iter = copyFiles.begin(); iter != copyFiles.end(); ++iter, ctr++)
	{
		wxFileName file(*iter);

		wxFileName destFile(file);
		destFile.MakeRelativeTo(m_src.GetFullPath());
		destFile.Normalize(wxPATH_NORM_ALL, m_dest.GetFullPath());

		if (!wxDirExists(destFile.GetPath(true)))
		{
			if (!CreateAllDirs(wxFileName::DirName(destFile.GetPath(true))))
				return (ExitCode)0;
		}

		wxCopyFile(file.GetFullPath(), destFile.GetFullPath());
		SetProgress(((float)ctr / (float)copyFiles.size()) * 100);
	}
	return (ExitCode)1;
}

bool FileCopyTask::DiscoverFiles(const wxString &path, wxArrayString &fileList)
{
	if (wxDirExists(path))
	{
		wxDir subDir(path);

		if (!subDir.IsOpened())
		{
			return false;
		}

		wxString currentFile;
		if (subDir.GetFirst(&currentFile))
		{
			do 
			{
				currentFile = Path::Combine(subDir.GetName(), currentFile);

				if (wxFileExists(currentFile) || wxDirExists(currentFile))
				{
					if (!DiscoverFiles(currentFile, fileList))
						return false;
				}
			} while (subDir.GetNext(&currentFile));
		}
	}
	else if (wxFileExists(path))
	{
		fileList.Add(path);
	}
	return true;
}
