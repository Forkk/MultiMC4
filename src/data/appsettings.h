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

#include "utils/osutils.h"

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

enum ProxyType
{
	Proxy_None,
	Proxy_HTTP,
	Proxy_SOCKS4,
	Proxy_SOCKS5,
};

#define STR_VALUE(val) #val

#define DEFINE_SETTING_ADVANCED(funcName, cfgEntryName, typeName, defVal) \
	virtual typeName Get ## funcName() const { return GetSetting<typeName>(cfgEntryName, defVal); } \
	virtual void Set ## funcName(typeName value) { SetSetting<typeName>(cfgEntryName, value); } \
	virtual void Reset ## funcName() { config->DeleteEntry(cfgEntryName); }

#define DEFINE_SETTING(settingName, typeName, defVal) \
	DEFINE_SETTING_ADVANCED(settingName, STR_VALUE(settingName), typeName, defVal)

#define DEFINE_FN_SETTING_ADVANCED(funcName, cfgEntryName, defVal) \
	virtual wxFileName Get ## funcName() const { return GetSetting(cfgEntryName, defVal); } \
	virtual void Set ## funcName(wxFileName value) { SetSetting(cfgEntryName, value); } \
	virtual void Reset ## funcName() { config->DeleteEntry(cfgEntryName); }

#define DEFINE_FN_SETTING(settingName, defVal) \
	DEFINE_FN_SETTING_ADVANCED(settingName, STR_VALUE(settingName), defVal)

#define DEFINE_ENUM_SETTING_ADVANCED(funcName, cfgEntryName, typeName, defVal) \
	virtual typeName Get ## funcName() const { return (typeName)GetSetting<int>(cfgEntryName, defVal); } \
	virtual void Set ## funcName(typeName value) { SetSetting<int>(cfgEntryName, value); } \
	virtual void Reset ## funcName() { config->DeleteEntry(cfgEntryName); };

#define DEFINE_ENUM_SETTING(settingName, typeName, defVal) \
	DEFINE_ENUM_SETTING_ADVANCED(settingName, STR_VALUE(settingName), typeName, defVal)

#define DEFINE_OVERRIDE_SETTING_BLANK(overrideName) \
	virtual bool Get ## overrideName ## Override() const { return false; }; \
	virtual void Set ## overrideName ## Override(bool value) {  };


class SettingsBase
{
public:
	virtual bool GetMemoryOverride() const { return false; };
	virtual void SetMemoryOverride( bool ) {};

	virtual long GetLanguageID() const;
	virtual wxString GetLanguage() const;
	virtual void SetLanguage(wxString value) { SetSetting<wxString>("Language", value); }
	virtual void ResetLanguage() { config->DeleteEntry("Language"); }

	DEFINE_SETTING(UseSystemLang, bool, true);

	DEFINE_SETTING_ADVANCED(MinMemAlloc, "MinMemoryAlloc", int, 512);
	DEFINE_SETTING_ADVANCED(MaxMemAlloc, "MaxMemoryAlloc", int, 1024);

	DEFINE_OVERRIDE_SETTING_BLANK(Window);
	DEFINE_SETTING(MCWindowWidth, int, 854);
	DEFINE_SETTING(MCWindowHeight, int, 480);
	DEFINE_SETTING(MCWindowMaximize, bool, false);
	DEFINE_SETTING(UseAppletWrapper, bool, true);

	DEFINE_OVERRIDE_SETTING_BLANK(Updates);
	DEFINE_ENUM_SETTING(UpdateMode, UpdateMode, Update_Never);

	DEFINE_OVERRIDE_SETTING_BLANK(Login);
	DEFINE_SETTING(AutoLogin, bool, false);

	DEFINE_OVERRIDE_SETTING_BLANK(Java);
	DEFINE_SETTING_ADVANCED(JavaPath, JPATH_FIELD_NAME, wxString, "java");
	DEFINE_SETTING(JvmArgs, wxString, wxEmptyString);

	DEFINE_ENUM_SETTING(ProxyType, ProxyType, Proxy_None);
	DEFINE_SETTING(ProxyHostName, wxString, wxEmptyString);
	DEFINE_SETTING(ProxyPort, long, 8080);
	DEFINE_SETTING(ProxyUsername, wxString, wxEmptyString);
	DEFINE_SETTING(ProxyPassword, wxString, wxEmptyString);
	DEFINE_SETTING(ProxyDNS, bool, true);

	DEFINE_FN_SETTING_ADVANCED(InstDir, "InstanceDir", wxFileName::DirName("instances"));
	DEFINE_FN_SETTING(ModsDir, wxFileName::DirName("mods"));
	DEFINE_FN_SETTING(IconsDir, wxFileName::DirName("icons"));
	DEFINE_FN_SETTING(LwjglDir, wxFileName::DirName("lwjgl"));

	DEFINE_SETTING(AutoCloseConsole, bool, true);
	DEFINE_SETTING(ShowConsole, bool, true);

	DEFINE_SETTING(AutoUpdate, bool, true);
	DEFINE_SETTING(UseDevBuilds, bool, false);

	DEFINE_ENUM_SETTING(GUIMode, GUIMode, GUI_Simple);

	DEFINE_ENUM_SETTING(InstSortMode, InstSortMode, Sort_LastLaunch);

	// These are special... No macros for them.
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
