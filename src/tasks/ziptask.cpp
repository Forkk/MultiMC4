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

#include "ziptask.h"
#include <fsutils.h>
#include <wx/wfstream.h>
#include <wx/dir.h>

#include <apputils.h>

#include <set>
#include <memory>

ZipTask::ZipTask(wxOutputStream *out, const wxString &path)
{
	m_out = out;
	m_path = path;
}

bool ZipTask::DiscoverFiles(const wxString &path, wxArrayString &fileList)
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

wxThread::ExitCode ZipTask::TaskStart()
{
	SetStatus("Searching for files...");

	wxArrayString fileList;
	if (!DiscoverFiles(m_path, fileList))
		return (ExitCode)0;
	
	int ctr = 0;
	wxZipOutputStream zipStream(*m_out);
	for (wxArrayString::iterator iter = fileList.begin(); iter != fileList.end(); ++iter, ctr++)
	{
		wxFileName file(*iter);

		wxFileName destFile(file);
		destFile.MakeRelativeTo(m_path);

		zipStream.PutNextEntry(destFile.GetFullPath());

		SetStatus(wxT("Zipping ") + destFile.GetFullPath());
		wxFFileInputStream inStream(file.GetFullPath());
		zipStream.Write(inStream);
		SetProgress(((float)ctr / (float)fileList.size()) * 100);
	}

	return (ExitCode)1;
}
