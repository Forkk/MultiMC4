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

#include "appsettings.h"
#include <apputils.h>
#include <boost/property_tree/ini_parser.hpp>

#include "version.h"

AppSettings *settings;

bool InitAppSettings(void)
{
	settings = new AppSettings();
	return true;
}

AppSettings::AppSettings()
	: config(_("MultiMC"), wxEmptyString, _("multimc.cfg"), 
		wxEmptyString, wxCONFIG_USE_LOCAL_FILE | wxCONFIG_USE_RELATIVE_PATH)
{
	
}


int AppSettings::GetMinMemAlloc() const
{
	return GetSetting<int>(_("MinMemoryAlloc"), 512);
}

void AppSettings::SetMinMemAlloc(int value)
{
	SetSetting<int>(_("MinMemoryAlloc"), value);
}


int AppSettings::GetMaxMemAlloc() const
{
	return GetSetting<int>(_("MaxMemoryAlloc"), 1024);
}

void AppSettings::SetMaxMemAlloc(int value)
{
	SetSetting<int>(_("MaxMemoryAlloc"), value);
}


int AppSettings::GetMCWindowWidth() const
{
	return GetSetting<int>(_("MCWindowWidth"), 854);
}

void AppSettings::SetMCWindowWidth(int value)
{
	SetSetting<int>(_("MCWindowWidth"), value);
}


int AppSettings::GetMCWindowHeight() const
{
	return GetSetting<int>(_("MCWindowHeight"), 480);
}

void AppSettings::SetMCWindowHeight(int value)
{
	SetSetting<int>(_("MCWindowHeight"), value);
}


bool AppSettings::GetMCWindowMaximize() const
{
	return GetSetting<bool>(_("MCWindowMaximize"), false);
}

void AppSettings::SetMCWindowMaximize(bool value)
{
	SetSetting<bool>(_("MCWindowMaximize"), value);
}


bool AppSettings::GetUseAppletWrapper() const
{
	return GetSetting<bool>(_("UseAppletWrapper"), true);
}

void AppSettings::SetUseAppletWrapper(bool value)
{
	SetSetting<bool>(_("UseAppletWrapper"), value);
}


wxFileName AppSettings::GetInstDir() const
{
	return GetSetting(_("InstanceDir"), wxFileName::DirName(_("instances")));
}

void AppSettings::SetInstDir(wxFileName value)
{
	SetSetting(_("InstanceDir"), value);
}


wxString AppSettings::GetJavaPath() const
{
	return GetSetting<wxString>(_("JavaPath"), _("java"));
}

void AppSettings::SetJavaPath(wxString value)
{
	SetSetting<wxString>(_("JavaPath"), value);
}


void AppSettings::SetJvmArgs(wxString value)
{
	SetSetting<wxString>(_("JvmArgs"), value);
}

wxString AppSettings::GetJvmArgs() const
{
	return GetSetting<wxString>(_("JvmArgs"), wxEmptyString);
}


wxFileName AppSettings::GetModsDir() const
{
	return GetSetting(_("ModsDir"), wxFileName::DirName(_("mods")));
}

void AppSettings::SetModsDir(wxFileName value)
{
	SetSetting(_("ModsDir"), value);
}


bool AppSettings::GetAutoCloseConsole() const
{
	return GetSetting<bool>(_("AutoCloseConsole"), true);
}

void AppSettings::SetAutoCloseConsole(bool value)
{
	SetSetting<bool>(_("AutoCloseConsole"), value);
}


bool AppSettings::GetAutoUpdate() const
{
	return GetSetting<bool>(_("AutoUpdate"), true);
}

void AppSettings::SetAutoUpdate(bool value)
{
	SetSetting<bool>(_("AutoUpdate"), value);
}


bool AppSettings::GetQuitIfProblem() const
{
	return GetSetting<bool>(_("QuitIfProblem"), false);
}

void AppSettings::SetQuitIfProblem(bool value)
{
	SetSetting<bool>(_("QuitIfProblem"), true);
}


bool AppSettings::GetShowConsole() const
{
	return GetSetting<bool>(_("ShowConsole"), true);
}

void AppSettings::SetShowConsole(bool value)
{
	SetSetting<bool>(_("ShowConsole"), value);
}


bool AppSettings::GetUseDevBuilds() const
{
	return GetSetting<bool>(_("UseDevBuilds"), AppVersion.IsDevBuild());
}

void AppSettings::SetUseDevBuilds(bool value)
{
	SetSetting<bool>(_("UseDevBuilds"), value);
}


GUIMode AppSettings::GetGUIMode() const
{
	return (GUIMode)GetSetting<int>(_("GUIMode"), GUI_Default);
}

void AppSettings::SetGUIMode(GUIMode value)
{
	SetSetting<int>(_("GUIMode"), value);
}


wxColor AppSettings::GetConsoleSysMsgColor() const
{
	return wxColor(GetSetting<wxString>(_("ConsoleSysMsgColor"), _("#0000FF")));
}

void AppSettings::SetConsoleSysMsgColor(wxColor color)
{
	SetSetting<wxString>(_("ConsoleSysMsgColor"), color.GetAsString());
}


wxColor AppSettings::GetConsoleStdoutColor() const
{
	return wxColor(GetSetting<wxString>(_("ConsoleStdoutColor"), _("#000000")));
}

void AppSettings::SetConsoleStdoutColor(wxColor color)
{
	SetSetting<wxString>(_("ConsoleStdoutColor"), color.GetAsString());
}


wxColor AppSettings::GetConsoleStderrColor() const
{
	return wxColor(GetSetting<wxString>(_("ConsoleStderrColor"), _("#FF0000")));
}

void AppSettings::SetConsoleStderrColor(wxColor color)
{
	SetSetting<wxString>(_("ConsoleStderrColor"), color.GetAsString());
}


template <typename T>
T AppSettings::GetSetting(const wxString &key, T defValue) const
{
	T val;
	if (config.Read(key, &val))
		return val;
	else
		return defValue;
}

template <typename T>
void AppSettings::SetSetting(const wxString &key, T value, bool suppressErrors)
{
	if (!config.Write(key, value) && !suppressErrors)
		wxLogError(_("Failed to write config setting %s"), key.c_str());
	config.Flush();
}

wxFileName AppSettings::GetSetting(const wxString &key, wxFileName defValue) const
{
	wxString val;
	if (config.Read(key, &val))
	{
		if (defValue.IsDir())
			return wxFileName::DirName(val);
		else
			return wxFileName::FileName(val);
	}
	else
		return defValue;
}

void AppSettings::SetSetting(const wxString &key, wxFileName value, bool suppressErrors)
{
	if (!config.Write(key, value.GetFullPath()) && !suppressErrors)
		wxLogError(_("Failed to write config setting %s"), key.c_str());
	config.Flush();
}
