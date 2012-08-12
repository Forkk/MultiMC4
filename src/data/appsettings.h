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

class AppSettings
{
public:
	AppSettings();
	virtual ~AppSettings();
	
	int GetMinMemAlloc() const;
	void SetMinMemAlloc(int value);
	
	int GetMaxMemAlloc() const;
	void SetMaxMemAlloc(int value);
	
	wxString GetJavaPath() const;
	void SetJavaPath(wxString value);

	wxString GetJvmArgs() const;
	void SetJvmArgs(wxString value);
	
	wxFileName GetInstDir() const;
	void SetInstDir(wxFileName value);
	
	wxFileName GetModsDir() const;
	void SetModsDir(wxFileName value);
	
	bool GetShowConsole() const;
	void SetShowConsole(bool value);
	
	bool GetAutoCloseConsole() const;
	void SetAutoCloseConsole(bool value);
	
	bool GetAutoUpdate() const;
	void SetAutoUpdate(bool value);
	
	bool GetQuitIfProblem() const;
	void SetQuitIfProblem(bool value);

	bool GetUseDevBuilds() const;
	void SetUseDevBuilds(bool value);
	
	GUIMode GetGUIMode() const;
	void SetGUIMode(GUIMode value);
	
protected:
// 	boost::property_tree::ptree ptree;
	wxConfig *config;
	
	template <typename T>
	T GetSetting(const wxString &key, T defValue) const;
	template <typename T>
	void SetSetting(const wxString &key, T value, bool suppressErrors = false);
	
	wxFileName GetSetting(const wxString &key, wxFileName defValue) const;
	void SetSetting(const wxString &key, wxFileName value, bool suppressErrors = false);
};

extern AppSettings settings;
