// 
//  Copyright 2012 Andrew Okin
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

#ifdef __GNUC__
#define DEPRECATED(func) func __attribute__ ((deprecated))
#elif defined(_MSC_VER)
#define DEPRECATED(func) __declspec(deprecated) func
#else
#pragma message("WARNING: You need to implement DEPRECATED for this compiler")
#define DEPRECATED(func) func
#endif

#define ENUM_CONTAINS(val, check) (val & check) == check

#ifdef MSVC
#define TOASCII(str) str.ToAscii()->data()
#else
#define TOASCII(str) str.ToAscii().data()
#endif

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
	// Opens the given path with the default program.
	void OpenFile(wxFileName path);
	
	// Gets the max value of the users memory allocation settings.
	// (this is based on the amount of free memory on the users computer)
	int GetMaxAllowedMemAlloc();
	
	wxString RemoveInvalidPathChars(wxString path, wxChar replaceWith = '-');
	
	wxString ExecutePost(const wxString &address, const wxString &requestString, 
		wxProtocolError *error);
	
	wxString BytesToString(unsigned char *bytes);
}

namespace Path
{
	wxString Combine(const wxString& path, const wxString& str);
	wxString Combine(const wxFileName& path, const wxString& str);

	wxString GetParent(const wxString &path);
}

wxString FindJavaPath(const wxString& def = _("java"));
