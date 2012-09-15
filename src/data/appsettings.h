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
#include <wx/listctrl.h>
#include <wx/filesys.h>
#include <wx/config.h>
#include <wx/fileconf.h>
#include <wx/confbase.h>

const wxFileName iniConfigFile(_("multimc.cfg"));

bool InitAppSettings(void);

// Default values
const wxString defJavaPath = _("java");
const wxString defInstDir = _("instances");
const wxString defModsDir = _("mods");

enum GUIMode
{
	GUI_Default,
	GUI_Simple,
};

enum UpdateMode
{
	Update_Auto,
	Update_Never
};

class SettingsBase
{
public:
	virtual bool GetMemoryOverride() const { return false; };
	virtual void SetMemoryOverride( bool ) {};
	
	virtual int GetMinMemAlloc() const { return GetSetting<int>(_("MinMemoryAlloc"), 512); };
	virtual void SetMinMemAlloc(int value) {    SetSetting<int>(_("MinMemoryAlloc"), value); };
	virtual void ResetMinMemAlloc() {   config->DeleteEntry(_("MinMemoryAlloc")); };

	virtual int GetMaxMemAlloc() const { return GetSetting<int>(_("MaxMemoryAlloc"), 1024); };
	virtual void SetMaxMemAlloc(int value) {    SetSetting<int>(_("MaxMemoryAlloc"), value); };
	virtual void ResetMaxMemAlloc() {   config->DeleteEntry(_("MaxMemoryAlloc")); };
	
	virtual bool GetWindowOverride() const { return false; };
	virtual void SetWindowOverride( bool ) {};
	
	virtual int GetMCWindowWidth() const { return GetSetting<int>(_("MCWindowWidth"), 854); };
	virtual void SetMCWindowWidth(int value) {    SetSetting<int>(_("MCWindowWidth"), value); };
	virtual void ResetMCWindowWidth() {   config->DeleteEntry(_("MCWindowWidth")); };

	virtual int GetMCWindowHeight() const { return GetSetting<int>(_("MCWindowHeight"), 480); };
	virtual void SetMCWindowHeight(int value) {    SetSetting<int>(_("MCWindowHeight"), value); };
	virtual void ResetMCWindowHeight() {   config->DeleteEntry(_("MCWindowHeight")); };
	
	virtual bool GetMCWindowMaximize() const { return GetSetting<bool>(_("MCWindowMaximize"), false); };
	virtual void SetMCWindowMaximize(bool value) {    SetSetting<bool>(_("MCWindowMaximize"), value); };
	virtual void ResetMCWindowMaximize() {    config->DeleteEntry(_("MCWindowMaximize")); };
	
	virtual bool GetUseAppletWrapper() const { return GetSetting<bool>(_("UseAppletWrapper"), true); };
	virtual void SetUseAppletWrapper(bool value) {    SetSetting<bool>(_("UseAppletWrapper"), value); };
	virtual void ResetUseAppletWrapper() {    config->DeleteEntry(_("UseAppletWrapper")); };

	virtual bool GetUpdatesOverride() const { return false; };
	virtual void SetUpdatesOverride( bool ) {};
	
	virtual UpdateMode GetUpdateMode() const { return (UpdateMode)GetSetting<int>(_("UpdateMode"), Update_Never); }
	void SetUpdateMode(UpdateMode value) { SetSetting<int>(_("UpdateMode"), value); }
	virtual void ResetUpdateMode() {           config->DeleteEntry(_("UpdateMode")); };
	
	virtual bool GetJavaOverride() const { return false; };
	virtual void SetJavaOverride( bool ) {};
	
	virtual wxString GetJavaPath() const { return GetSetting<wxString>(_("JPath"), _("java")); }
	virtual void SetJavaPath(wxString value) {    SetSetting<wxString>(_("JPath"), value); }
	virtual void ResetJavaPath() {           config->DeleteEntry(_("JPath")); };
	
	virtual void SetJvmArgs(wxString value) { SetSetting<wxString>(_("JvmArgs"), value); }
	wxString GetJvmArgs() const { return GetSetting<wxString>(_("JvmArgs"), wxEmptyString ); }
	virtual void ResetJvmArgs() {           config->DeleteEntry(_("JvmArgs")); };

	virtual wxFileName GetInstDir() const { return GetSetting(_("InstanceDir"), wxFileName::DirName(_("instances"))); }
	virtual void SetInstDir(wxFileName value) {    SetSetting(_("InstanceDir"), value); }
	
	virtual wxFileName GetModsDir() const { return GetSetting(_("ModsDir"), wxFileName::DirName(_("mods"))); }
	void SetModsDir(wxFileName value) { SetSetting(_("ModsDir"), value); }

	virtual wxFileName GetIconsDir() const { return GetSetting(_("IconsDir"), wxFileName::DirName(_("icons"))); }
	void SetIconsDir(wxFileName value) { SetSetting(_("IconsDir"), value); }

	virtual bool GetAutoCloseConsole() const { return GetSetting<bool>(_("AutoCloseConsole"), true); }
	void SetAutoCloseConsole(bool value) { SetSetting<bool>(_("AutoCloseConsole"), value); }

	virtual bool GetAutoUpdate() const { return GetSetting<bool>(_("AutoUpdate"), true); }
	void SetAutoUpdate(bool value) { SetSetting<bool>(_("AutoUpdate"), value); }

	virtual bool GetShowConsole() const { return GetSetting<bool>(_("ShowConsole"), true); }
	void SetShowConsole(bool value) { SetSetting<bool>(_("ShowConsole"), value); }

	virtual GUIMode GetGUIMode() const { return (GUIMode)GetSetting<int>(_("GUIMode"), GUI_Default); }
	void SetGUIMode(GUIMode value) { SetSetting<int>(_("GUIMode"), value); }

	virtual wxColor GetConsoleSysMsgColor() const { return wxColor(GetSetting<wxString>(_("ConsoleSysMsgColor"), _("#0000FF"))); }
	void SetConsoleSysMsgColor(wxColor color) { SetSetting<wxString>(_("ConsoleSysMsgColor"), color.GetAsString()); }

	virtual wxColor GetConsoleStdoutColor() const { return wxColor(GetSetting<wxString>(_("ConsoleStdoutColor"), _("#000000"))); }
	void SetConsoleStdoutColor(wxColor color) { SetSetting<wxString>(_("ConsoleStdoutColor"), color.GetAsString()); }

	virtual wxColor GetConsoleStderrColor() const { return wxColor(GetSetting<wxString>(_("ConsoleStderrColor"), _("#FF0000"))); }
	virtual void SetConsoleStderrColor(wxColor color) { SetSetting<wxString>(_("ConsoleStderrColor"), color.GetAsString()); }

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
