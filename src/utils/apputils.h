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

#pragma once
#include <wx/wx.h>
#include <wx/filesys.h>
#include <wx/protocol/http.h>

#if wxCHECK_VERSION(2, 9, 0)
#define wx29
#endif

#ifdef __GNUC__
#define DEPRECATED(func) func __attribute__ ((deprecated))
#elif defined(_MSC_VER)
#define DEPRECATED(func) __declspec(deprecated) func
#else
#pragma message("WARNING: You need to implement DEPRECATED for this compiler")
#define DEPRECATED(func) func
#endif

#define ENUM_CONTAINS(val, check) (val & check) == check

#if defined __WXMSW__ && !defined wx29
#define TOASCII(str) str.ToAscii()
#define MBSTR(str) str.mb_str(wxConvUTF8)
#else
#define TOASCII(str) str.ToAscii().data()
#define MBSTR(str) str.mb_str(wxConvUTF8).data()
#endif

#if defined __WXMSW__
#define FNSTR(str) str.fn_str()
#else
#define FNSTR(str) str.fn_str().data()
#endif

#define STR_VALUE(val) #val

inline const char* cStr(const std::string& str)
{
	return str.c_str();
}

inline const char* cStr(const wxString& str)
{
	return str.mb_str();
}

inline wxString wxStr(const std::string &str)
{
	return wxString(str.c_str(), wxConvUTF8);
}

inline std::string stdStr(const wxString &str)
{
	return std::string(TOASCII(str));
}

namespace Utils
{
	// Opens the given url with whatever browser the OS thinks is best.
	bool OpenURL(wxString what);
	
	// Opens the given filename with whatever the OS thinks is best.
	void OpenFolder(wxFileName what);
	
	// Gets the max value of the users memory allocation settings.
	// (this is based on the amount of free memory on the users computer)
	int GetMaxAllowedMemAlloc();
	
	wxString RemoveInvalidPathChars(wxString path, wxChar replaceWith = '-', bool allowExclamationMark = true);
	bool ContainsInvalidPathChars(wxString path, bool allowExclamationMark = true);
	wxString RemoveInvalidFilenameChars(wxString path, wxChar replaceWith);
	
	wxString ExecutePost(const wxString &address, const wxString &requestString, 
		wxProtocolError *error);
	
	wxString BytesToString(unsigned char *bytes);
}

namespace Path
{
	wxString Combine(const wxFileName& path, const wxString& str);
	wxString Combine(const wxString& path, const wxString& str);
	wxString Combine(const wxString& path, const wxString& str, const wxString& str2);

	wxString GetParent(const wxString &path);

	wxString GetDesktopDir();
}

wxString FindJavaPath(const wxString& def = "java");

bool CreateShortcut(wxString path, wxString dest, wxString args);
