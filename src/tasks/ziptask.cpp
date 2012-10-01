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

bool ZipTask::CompressRecursively(const wxString &path, wxZipOutputStream &zipStream, const wxString &topDir)
{
	wxFileName destPath(path);
	destPath.MakeRelativeTo(topDir);

	if (wxFileExists(path))
	{
		zipStream.PutNextEntry(destPath.GetFullPath());

		SetStatus(wxT("Zipping ") + path);
		wxFFileInputStream inStream(path);
		zipStream.Write(inStream);
	}
	else if (wxDirExists(path))
	{
		zipStream.PutNextDirEntry(destPath.GetFullPath());
		wxDir dir(path);

		wxString subpath;
		if (dir.GetFirst(&subpath))
		{
			do
			{
				if (!CompressRecursively(Path::Combine(path, subpath), zipStream, topDir))
					return false;
			} while (dir.GetNext(&subpath));
		}
	}
	return true;
}

wxThread::ExitCode ZipTask::TaskStart()
{
	SetStatus("Zipping...");
	wxZipOutputStream zipStream(*m_out);
	if (CompressRecursively(m_path, zipStream, m_path))
		return (ExitCode)1;
	return (ExitCode)0;
}
