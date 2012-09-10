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

#include "apputils.h"
#include <wx/wx.h>
#include <wx/arrstr.h>
#include "osutils.h"

void Utils::OpenFolder(wxFileName path)
{
	wxString cmd;
	if (IS_WINDOWS())
	{
		cmd = _("explorer ");
	}
	else if (IS_MAC())
	{
		cmd = _("open ");
	}
	else if (IS_LINUX())
	{
		cmd = _("xdg-open ");
	}
	else
	{
		wxMessageBox(_T("This feature is not supported by your OS."), _T("Error"));
		return;
	}
	cmd.Append(wxT("\""));
	cmd.Append(path.GetFullPath());
	cmd.Append(wxT("\""));
    wxExecute(cmd);
}

#ifdef __WXMSW__
bool Utils::OpenURL(wxString url)
{
	auto r = (int) ShellExecute(NULL, L"open", url.fn_str(), NULL, NULL, SW_SHOWNORMAL);
	return r > 32;
}
#else
bool Utils::OpenURL(wxString url)
{
	return wxLaunchDefaultBrowser(url);
}
#endif

int Utils::GetMaxAllowedMemAlloc()
{
	//wxMemorySize mem = wxGetFreeMemory();
	//if (mem < 0)
	//{
	//	// If we can't determine the amount of available memory, set the max to 65535
	//	return 65536;
	//}
	//else
	//{
	//	return mem
	//}
	return 65536;
}

wxString Utils::RemoveInvalidPathChars(wxString path, wxChar replaceWith)
{
	for (size_t i = 0; i < path.Len(); i++)
	{
		if (wxFileName::GetForbiddenChars().Contains(path[i]) || path[i] == wxT('!'))
		{
			path[i] = replaceWith;
		}
	}
	return path;
}

wxString Path::Combine(const wxFileName& path, const wxString& str)
{
	return wxFileName(path.GetFullPath(), str).GetFullPath();
}

wxString Path::Combine(const wxString& path, const wxString& str)
{
	return wxFileName(path, str).GetFullPath();
}

wxString Path::GetParent(const wxString &path)
{
	wxFileName pathName = wxFileName::DirName(path);
	pathName.AppendDir(_(".."));
	pathName.Normalize();
	return pathName.GetFullPath();
}

wxString Utils::BytesToString(unsigned char *bytes)
{
	char asciihash[33];
	
	int p = 0;
	for(int i=0; i<16; i++)
	{
		::sprintf(&asciihash[p],"%02x",bytes[i]);
		p += 2;
	}
	asciihash[32] = '\0';
	return wxStr(std::string(asciihash));
}

#if defined __WXMSW__

// We can use the registry to find Java on Windows.
#include <winreg.h>

wxString FindJavaPath(const wxString& def)
{
	// Open the JRE registry key.
	HKEY jreKey;
	std::string jreKeyName = "SOFTWARE\\JavaSoft\\Java Runtime Environment";
	if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, jreKeyName.c_str(), 0, KEY_READ | KEY_WOW64_64KEY, &jreKey) == ERROR_SUCCESS)
	{
		// Read the current JRE version from the registry.
		// This will be used to find the key that contains the JavaHome value.
		char *value = new char[0];
		DWORD valueSz = 0;
		if (RegQueryValueExA(jreKey, "CurrentVersion", NULL, NULL, (BYTE*)value, &valueSz) == ERROR_MORE_DATA)
		{
			value = new char[valueSz];
			RegQueryValueExA(jreKey, "CurrentVersion", NULL, NULL, (BYTE*)value, &valueSz);
		}

		RegCloseKey(jreKey);

		// Now open the registry key for the JRE version that we just got.
		jreKeyName.append("\\").append(value);
		if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, jreKeyName.c_str(), 0, KEY_READ | KEY_WOW64_64KEY, &jreKey) == ERROR_SUCCESS)
		{
			// Read the JavaHome value to find where Java is installed.
			value = new char[0];
			valueSz = 0;
			if (RegQueryValueExA(jreKey, "JavaHome", NULL, NULL, (BYTE*)value, &valueSz) == ERROR_MORE_DATA)
			{
				value = new char[valueSz];
				RegQueryValueExA(jreKey, "JavaHome", NULL, NULL, (BYTE*)value, &valueSz);
			}

			RegCloseKey(jreKey);

			wxString javaHome = wxStr(value);
			javaHome = Path::Combine(Path::Combine(javaHome, _("bin")), _("java.exe"));
			return javaHome;
		}
	}
	return def;
}

#elif defined __WXGTK__

wxString FindJavaPath(const wxString& def)
{
    wxArrayString whichOutput;

    // This should work on all UNIX systems, including MacOS X
    if (wxExecute(_("which java"), whichOutput, wxEXEC_SYNC | wxEXEC_NODISABLE) == 0)
    {
        // Valid java found, return the string
        return whichOutput.Last();
    }
    else
    {
        //Either no `java` is in the ${PATH}, or `which` command in not available
        return def;
    }
}

#else

wxString FindJavaPath(const wxString& def)
{
	return def;
}

#endif
