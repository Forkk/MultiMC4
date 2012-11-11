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
#include <wx/listctrl.h>
#include <wx/filesys.h>
#include <wx/config.h>
#include <wx/fileconf.h>
#include <wx/confbase.h>

#include "osutils.h"

#if WINDOWS
#define JPATH_FIELD_NAME "JPathWindows"
#elif OSX
#define JPATH_FIELD_NAME "JPathOSX"
#elif LINUX
#define JPATH_FIELD_NAME "JPathLinux"
#endif

const wxFileName iniConfigFile("multimc.cfg");

bool InitAppSettings(void);

// Default values
const wxString defJavaPath = "java";
const wxString defInstDir = "instances";
const wxString defModsDir = "mods";

enum GUIMode
{
	GUI_Fancy,
	GUI_Simple,
};

enum UpdateMode
{
	Update_Auto,
	Update_Never
};

enum InstSortMode
{
	// Sort alphabetically by name.
	Sort_Name,

	// Sort by which instance was launched most recently.
	Sort_LastLaunch,
};

class SettingsBase
{
public:
	virtual bool GetMemoryOverride() const { return false; };
	virtual void SetMemoryOverride( bool ) {};

	virtual long GetLanguage() const;
	virtual void SetLanguage(long value) { SetSetting<long>("Language", value); }
	virtual void ResetLanguage() { config->DeleteEntry("Language"); }
	
	virtual int GetMinMemAlloc() const { return GetSetting<int>("MinMemoryAlloc", 512); };
	virtual void SetMinMemAlloc(int value) {    SetSetting<int>("MinMemoryAlloc", value); };
	virtual void ResetMinMemAlloc() {   config->DeleteEntry("MinMemoryAlloc"); };

	virtual int GetMaxMemAlloc() const { return GetSetting<int>("MaxMemoryAlloc", 1024); };
	virtual void SetMaxMemAlloc(int value) {    SetSetting<int>("MaxMemoryAlloc", value); };
	virtual void ResetMaxMemAlloc() {   config->DeleteEntry("MaxMemoryAlloc"); };
	
	virtual bool GetWindowOverride() const { return false; };
	virtual void SetWindowOverride( bool ) {};
	
	virtual int GetMCWindowWidth() const { return GetSetting<int>("MCWindowWidth", 854); };
	virtual void SetMCWindowWidth(int value) {    SetSetting<int>("MCWindowWidth", value); };
	virtual void ResetMCWindowWidth() {   config->DeleteEntry("MCWindowWidth"); };

	virtual int GetMCWindowHeight() const { return GetSetting<int>("MCWindowHeight", 480); };
	virtual void SetMCWindowHeight(int value) {    SetSetting<int>("MCWindowHeight", value); };
	virtual void ResetMCWindowHeight() {   config->DeleteEntry("MCWindowHeight"); };
	
	virtual bool GetMCWindowMaximize() const { return GetSetting<bool>("MCWindowMaximize", false); };
	virtual void SetMCWindowMaximize(bool value) {    SetSetting<bool>("MCWindowMaximize", value); };
	virtual void ResetMCWindowMaximize() {    config->DeleteEntry("MCWindowMaximize"); };
	
	virtual bool GetUseAppletWrapper() const { return GetSetting<bool>("UseAppletWrapper", true); };
	virtual void SetUseAppletWrapper(bool value) {    SetSetting<bool>("UseAppletWrapper", value); };
	virtual void ResetUseAppletWrapper() {    config->DeleteEntry("UseAppletWrapper"); };

	virtual bool GetUpdatesOverride() const { return false; };
	virtual void SetUpdatesOverride( bool ) {};
	
	virtual UpdateMode GetUpdateMode() const { return (UpdateMode)GetSetting<int>("UpdateMode", Update_Never); }
	void SetUpdateMode(UpdateMode value) { SetSetting<int>("UpdateMode", value); }
	virtual void ResetUpdateMode() {           config->DeleteEntry("UpdateMode"); };

	virtual bool GetLoginOverride() const { return false; }
	virtual void SetLoginOverride( bool ) {};

	virtual bool GetAutoLogin() const { return GetSetting<bool>("AutoLogin", false); }
	virtual void SetAutoLogin(bool value) { SetSetting<bool>("AutoLogin", value); }
	virtual void ResetAutoLogin() { config->DeleteEntry("AutoLogin"); }
	
	virtual bool GetJavaOverride() const { return false; };
	virtual void SetJavaOverride( bool ) {};
	
	virtual wxString GetJavaPath() const { return GetSetting<wxString>(JPATH_FIELD_NAME, "java"); }
	virtual void SetJavaPath(wxString value) {    SetSetting<wxString>(JPATH_FIELD_NAME, value); }
	virtual void ResetJavaPath() {           config->DeleteEntry(JPATH_FIELD_NAME); };
	
	virtual void SetJvmArgs(wxString value) { SetSetting<wxString>("JvmArgs", value); }
	wxString GetJvmArgs() const { return GetSetting<wxString>("JvmArgs", wxEmptyString ); }
	virtual void ResetJvmArgs() {           config->DeleteEntry("JvmArgs"); };

	virtual wxFileName GetInstDir() const { return GetSetting("InstanceDir", wxFileName::DirName("instances")); }
	virtual void SetInstDir(wxFileName value) {    SetSetting("InstanceDir", value); }
	
	virtual wxFileName GetModsDir() const { return GetSetting("ModsDir", wxFileName::DirName("mods")); }
	void SetModsDir(wxFileName value) { SetSetting("ModsDir", value); }

	virtual wxFileName GetIconsDir() const { return GetSetting("IconsDir", wxFileName::DirName("icons")); }
	void SetIconsDir(wxFileName value) { SetSetting("IconsDir", value); }

	virtual bool GetAutoCloseConsole() const { return GetSetting<bool>("AutoCloseConsole", true); }
	void SetAutoCloseConsole(bool value) { SetSetting<bool>("AutoCloseConsole", value); }

	virtual bool GetAutoUpdate() const { return GetSetting<bool>("AutoUpdate", true); }
	void SetAutoUpdate(bool value) { SetSetting<bool>("AutoUpdate", value); }

	virtual bool GetShowConsole() const { return GetSetting<bool>("ShowConsole", true); }
	void SetShowConsole(bool value) { SetSetting<bool>("ShowConsole", value); }

	virtual bool GetUseDevBuilds() const { return GetSetting<bool>("UseDevBuilds", false); }
	void SetUseDevBuilds(bool value) { SetSetting<bool>("UseDevBuilds", value); }

	virtual GUIMode GetGUIMode() const { return (GUIMode)GetSetting<int>("GUIMode", GUI_Simple); }
	void SetGUIMode(GUIMode value) { SetSetting<int>("GUIMode", value); }

	virtual InstSortMode GetInstSortMode() const { return (InstSortMode)GetSetting<int>("InstSortMode", Sort_LastLaunch); }
	void SetInstSortMode(InstSortMode value) { SetSetting<int>("InstSortMode", value); }

	virtual wxColor GetConsoleSysMsgColor() const { return wxColor(GetSetting<wxString>("ConsoleSysMsgColor", "#0000FF")); }
	void SetConsoleSysMsgColor(wxColor color) { SetSetting<wxString>("ConsoleSysMsgColor", color.GetAsString()); }

	virtual wxColor GetConsoleStdoutColor() const { return wxColor(GetSetting<wxString>("ConsoleStdoutColor", "#000000")); }
	void SetConsoleStdoutColor(wxColor color) { SetSetting<wxString>("ConsoleStdoutColor", color.GetAsString()); }

	virtual wxColor GetConsoleStderrColor() const { return wxColor(GetSetting<wxString>("ConsoleStderrColor", "#FF0000")); }
	virtual void SetConsoleStderrColor(wxColor color) { SetSetting<wxString>("ConsoleStderrColor", color.GetAsString()); }

	virtual bool IsConfigGlobal()
	{
		return true;
	}
protected:
	wxFileConfig *config;
	template <typename T>
	T GetSetting(const wxString &key, T defValue) const
	{
		T val;
		if (config->Read(key, &val))
			return val;
		else
			return defValue;
	};
	
	template <typename T>
	void SetSetting(const wxString &key, T value, bool suppressErrors = false)
	{
		if (!config->Write(key, value) && !suppressErrors)
			wxLogError(_("Failed to write config setting %s"), key.c_str());
		config->Flush();
	};
	
	wxFileName GetSetting(const wxString &key, wxFileName defValue) const
	{
		wxString val;
		if (config->Read(key, &val))
		{
			if (defValue.IsDir())
				return wxFileName::DirName(val);
			else
				return wxFileName::FileName(val);
		}
		else
			return defValue;
	};
	void SetSetting(const wxString &key, wxFileName value, bool suppressErrors = false)
	{
		if (!config->Write(key, value.GetFullPath()) && !suppressErrors)
			wxLogError(_("Failed to write config setting %s"), key.c_str());
		config->Flush();
	};
};

class AppSettings : public SettingsBase
{
public:
	AppSettings();
	~AppSettings();
};

extern AppSettings *settings;
