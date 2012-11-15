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

#include "fsutils.h"
#include "apputils.h"
#include <wx/dir.h>
#include <wx/filefn.h>
#include <wx/filesys.h>
#include <wx/zipstrm.h>
#include <wx/wfstream.h>

#include <memory>

namespace fsutils {

// tests if 'a' is subset of 'b'
bool isSubsetOf ( wxFileName a, wxFileName b )
{
	a.MakeAbsolute();
	b.MakeAbsolute();
	
	// different volume?
	if(a.GetVolume() != b.GetVolume())
		return false;
	if(a.HasName())
	{
		a.AppendDir(a.GetName());
		a.SetFullName(wxEmptyString);
	}
	if(b.HasName())
	{
		b.AppendDir(b.GetName());
		b.SetFullName(wxEmptyString);
	}
	
	auto adirs = a.GetDirs();
	auto bdirs = b.GetDirs();
	// 'a' can't be a subset of 'b' if it's a shorter path
	if(adirs.size() < bdirs.size())
		return false;
	
	// now we compare from the start... 'b' is the same size or shorter than 'a', use 'b's length as a limit
	for(int i = 0; i < bdirs.size(); i++)
	{
		// any difference = failure
		if(adirs[i] != bdirs[i])
			return false;
	}
	// 'a' is a subset of 'b'
	return true;
}

	
void CopyFileList ( const wxArrayString& filenames, wxFileName targetDir )
{
	for (wxArrayString::const_iterator iter = filenames.begin(); iter != filenames.end(); ++iter)
	{
		wxFileName source (*iter);
		wxString fileName = source.GetFullName();
		wxFileName dest(Path::Combine(targetDir.GetFullPath(), fileName));
		if(wxFileName::DirExists(*iter))
		{
			fsutils::CopyDir(*iter,dest.GetFullPath());
		}
		else
		{
			wxCopyFile(*iter, dest.GetFullPath());
		}
	}
}

bool CopyDir(wxString sFrom, wxString sTo)
{
	if (sFrom.Last() != wxFILE_SEP_PATH) sFrom += wxFILE_SEP_PATH;
	if (sTo.Last() != wxFILE_SEP_PATH) sTo += wxFILE_SEP_PATH;

	if (!wxDirExists(sFrom))
	{
		//wxLogError(wxT("%s does not exist!\r\nCan not copy directory"), sFrom.c_str());
		return false;
	}
	if (!wxDirExists(sTo))
	{
		if (!wxFileName::Mkdir(sTo, 0750, wxPATH_MKDIR_FULL)) {
			//::wxLogError(wxT("%s could not be created!"), sTo.c_str());
			return false;
		}
	}

	wxDir fDir(sFrom);
	wxString sNext = wxEmptyString;
	bool bIsFile = fDir.GetFirst(&sNext);
	while (bIsFile)
	{
		const wxString sFileFrom = sFrom + sNext;
		const wxString sFileTo = sTo + sNext;
		if (wxDirExists(sFileFrom))
		{
			CopyDir(sFileFrom, sFileTo);
		}
		else
		{
			if (!wxFileExists(sFileTo))
			{
				if (!wxCopyFile(sFileFrom, sFileTo))
				{
					//wxLogError(wxT("Could not copy %s to %s !"), sFileFrom.c_str(), sFileTo.c_str());
					return false;
				}
			}
		}
		bIsFile = fDir.GetNext(&sNext);
	}
	return true;
}

bool RecursiveDelete(const wxString &path)
{
	if (wxFileExists(path))
	{
		if (!wxRemoveFile(path))
			return false;
	}
	else if (wxDirExists(path))
	{
		wxDir dir(path);
		wxString subpath;
		
		if (dir.GetFirst(&subpath))
		{
			do
			{
				if (!RecursiveDelete(Path::Combine(path, subpath)))
					return false;
			} while (dir.GetNext(&subpath));
		}
		
		if (!wxRmdir(path))
			return false;
	}
	return true;
}

void ExtractZipArchive(wxInputStream &stream, const wxString &dest)
{
	wxZipInputStream zipStream(stream);
	std::auto_ptr<wxZipEntry> entry;
	while (entry.reset(zipStream.GetNextEntry()), entry.get() != NULL)
	{
		if (entry->IsDir())
			continue;
		
		wxString name = entry->GetName();
		wxFileName destFile(dest + (name.StartsWith("/") ? "" : "/") + name);
		
		destFile.Mkdir(0777, wxPATH_MKDIR_FULL);
		
		if (destFile.FileExists())
			wxRemoveFile(destFile.GetFullPath());
		
		wxFFileOutputStream outStream(destFile.GetFullPath());
		outStream.Write(zipStream);
		
// 		wxFFile file(destFile.GetFullPath(), "w");
// 		
// 		const size_t bufSize = 1024;
// 		void *buffer = new char[bufSize];
// 		while (!zipStream.Eof())
// 		{
// 			zipStream.Read(buffer, bufSize);
// 			file.Write(buffer, bufSize);
// 		}
// 		file.Flush();
	}
}

void TransferZipArchive(wxInputStream &stream, wxZipOutputStream &out)
{
	wxZipInputStream zipStream(stream);
	std::auto_ptr<wxZipEntry> entry;
	while (entry.reset(zipStream.GetNextEntry()), entry.get() != NULL)
	{
		if (entry->IsDir())
			continue;

		wxString name = entry->GetName();

		out.PutNextEntry(name);
		out.Write(zipStream);
	}
}

bool CompressRecursively(const wxString &path, wxZipOutputStream &zipStream, const wxString &topDir)
{
	wxFileName destPath(path);
	destPath.MakeRelativeTo(topDir);
	
	if (wxFileExists(path))
	{
		zipStream.PutNextEntry(destPath.GetFullPath());
		
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

bool CompressZipArchive(wxOutputStream &stream, const wxString &srcDir)
{
	wxZipOutputStream zipStream(stream);
	return CompressRecursively(srcDir, zipStream, srcDir);
}

bool CreateAllDirs(const wxFileName &dir)
{
	if (!wxDirExists(Path::GetParent(dir.GetFullPath())))
	{
		if (!CreateAllDirs(Path::GetParent(dir.GetFullPath())))
			return false;
	}
	return wxMkdir(dir.GetFullPath());
}

#ifdef WIN32
#define _WIN32_WINNT 0x0500
#include <windows.h>
#include <sddl.h>

// This constructs an ACL to give the current user file permissions.
BOOL CreateMyDACL(SECURITY_ATTRIBUTES * pSA)
{
	// Get process token
	HANDLE hToken = NULL;
	if (!OpenProcessToken( GetCurrentProcess(), TOKEN_QUERY, &hToken ))
	{
		_tprintf( _T("OpenProcessToken failed. GetLastError returned: %d\n"), GetLastError());
		return false;
	}

	// Get the size of the memory buffer needed for the SID
	DWORD dwBufferSize = 0;
	if (!GetTokenInformation( hToken, TokenUser, NULL, 0, &dwBufferSize) && (GetLastError() != ERROR_INSUFFICIENT_BUFFER))
	{
		_tprintf( _T("GetTokenInformation failed. GetLastError returned: %d\n"), GetLastError());
		// Cleanup
		CloseHandle( hToken );
		hToken = NULL;
		return false;
	}
	
	// Allocate buffer for user token data
	PTOKEN_USER pTokenUser = (PTOKEN_USER) malloc(dwBufferSize);
	
	// Retrieve the token information in a TOKEN_USER structure
	if (!GetTokenInformation( hToken, TokenUser, pTokenUser, dwBufferSize, &dwBufferSize))
	{
		_tprintf( _T("2 GetTokenInformation failed. GetLastError returned: %d\n"), GetLastError());
		// Cleanup
		CloseHandle( hToken );
		hToken = NULL;
		free(pTokenUser);
		return false;
	}
	
	// Check if SID is valid
	if (!IsValidSid(pTokenUser->User.Sid))
	{
		_tprintf( _T("The owner SID is invalid.\n") );
		// Cleanup
		CloseHandle(hToken);
		hToken = NULL;
		free(pTokenUser);
		return false;
	}
	
	// Convert user SID to the equivalent string
	LPTSTR pszSID = NULL;
	if(!ConvertSidToStringSid( pTokenUser->User.Sid, &pszSID ))
	{
		CloseHandle(hToken);
		hToken = NULL;
		free(pTokenUser);
		return false;
	}

	// 1024 bytes is enough for everyone. This is final.
	wchar_t long_str[1024];
	// give full access to the file to the user running this process and nobody else
	swprintf(long_str,TEXT("D:P(A;OI;GA;;;%s)"),pszSID);
	if(pSA == NULL)
	{
		free(pTokenUser);
		return FALSE;
	}
	
	PULONG nSize = 0;
	// Do some verification
	/*
	wprintf(TEXT("The ACE strings: %s \n"), long_str);
	wprintf(TEXT("The converted string is at: %p \n"), &(pSA->lpSecurityDescriptor));
	*/

	// Convert the string to the security descriptor binary and return
	bool result = ConvertStringSecurityDescriptorToSecurityDescriptor(
		long_str,                         // The ACE strings
		SDDL_REVISION_1,              // Standard revision level
		&(pSA->lpSecurityDescriptor), // Pointer to the converted security descriptor
		nSize);                // The size in byte the converted security descriptor
	free(pTokenUser);
	return result;
}

// only works on NTFS volumes.
bool SecureFile (const char * filename)
{
	SECURITY_ATTRIBUTES  sa;
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.bInheritHandle = FALSE;
	if(!CreateMyDACL(&sa))
		return false;
	int result = SetFileSecurityA(filename,DACL_SECURITY_INFORMATION,sa.lpSecurityDescriptor);
	LocalFree(sa.lpSecurityDescriptor);
	return result != 0;
}
#else
// Windows developers, gaze upon the perfection of this solution and weep. Weep blood.
bool SecureFile (const char * filename)
{
	int result = chmod(filename,S_IRUSR | S_IWUSR);
	return result != -1;
}
#endif

}