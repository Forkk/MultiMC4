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
#include <vector>
#include <sstream>
#include <stdint.h>
#include <wx/wx.h>
#include <wx/filesys.h>
#include <wx/config.h>
#include <wx/fileconf.h>
#include <wx/process.h>

#include "appsettings.h"
#include "mod.h"
#include "modlist.h"
#include "world.h"
#include "worldlist.h"

bool IsValidInstance(wxFileName rootDir);

class Instance : public wxEvtHandler, public SettingsBase
{
public:
	static Instance *LoadInstance(wxFileName rootDir);
	Instance(const wxFileName &rootDir);
	~Instance(void);
	
	bool Save() const;
	bool Load(bool loadDefaults = false);
	
	// Directories
	wxFileName GetRootDir() const;
	wxFileName GetInstModsDir() const;
	
	// Minecraft dir subfolders
	wxFileName GetMCDir() const;
	wxFileName GetBinDir() const;
	wxFileName GetSavesDir() const;
	wxFileName GetMLModsDir() const;
	wxFileName GetCoreModsDir() const;
	wxFileName GetResourceDir() const;
	wxFileName GetScreenshotsDir() const;
	wxFileName GetTexturePacksDir() const;
	
	// Files
	wxFileName GetConfigPath() const;
	wxFileName GetVersionFile() const;
	wxFileName GetMCJar() const;
	wxFileName GetMCBackup() const;
	wxFileName GetModListFile() const;
	
	bool IsRunning() const;
	
	int64_t ReadVersionFile();
	void WriteVersionFile(const int64_t contents);
	
	wxString GetName() const;
	void SetName(wxString name);
	
	wxString GetIconKey() const;
	void SetIconKey(wxString iconKey);
	
	wxString GetNotes() const;
	void SetNotes(wxString notes);
	
	bool ShouldRebuild() const;
	void SetNeedsRebuild(bool value = true);
	
	wxProcess *GetInstProcess() const;
	
	wxString GetLastLaunchCommand() const;
	
	void SetEvtHandler(wxEvtHandler *handler);
	
	bool HasBinaries();
	wxProcess* Launch(wxString username, wxString sessionID, bool redirectOutput = false);
	
	ModList *GetModList();
	ModList *GetMLModList();
	ModList *GetCoreModList();

	WorldList *GetWorldList();
	
	void GetPossibleConfigFiles(wxArrayString *array, wxString dir = wxEmptyString);
	
	// these just pass on through
	virtual bool GetAutoCloseConsole() const { return settings->GetAutoCloseConsole(); };
	virtual bool GetAutoUpdate() const { return settings->GetAutoUpdate(); };
	virtual wxColour GetConsoleStderrColor() const { return settings->GetConsoleStderrColor(); };
	virtual wxColour GetConsoleStdoutColor() const { return settings->GetConsoleStdoutColor(); };
	virtual wxColour GetConsoleSysMsgColor() const { return settings->GetConsoleSysMsgColor(); };
	virtual GUIMode GetGUIMode() const { return settings->GetGUIMode(); };
	virtual wxFileName GetIconsDir() const { return settings->GetIconsDir(); };
	virtual wxFileName GetInstDir() const { return settings->GetInstDir(); };
	virtual wxFileName GetModsDir() const { return settings->GetModsDir(); };
	virtual bool GetShowConsole() const { return settings->GetShowConsole(); };
	
	// and these are overrides
	wxString GetJavaPath() const { return GetSetting<wxString>("JPath",settings->GetJavaPath()); };
	wxString GetJvmArgs() const { return GetSetting<wxString>("JvmArgs",settings->GetJvmArgs()); };
	int GetMaxMemAlloc() const { return GetSetting<int>("MaxMemoryAlloc",settings->GetMaxMemAlloc()); };
	int GetMinMemAlloc() const { return GetSetting<int>("MinMemoryAlloc",settings->GetMinMemAlloc()); };
	int GetMCWindowHeight() const { return GetSetting<int>("MCWindowHeight",settings->GetMCWindowHeight()); };
	int GetMCWindowWidth() const { return GetSetting<int>("MCWindowWidth",settings->GetMCWindowWidth()); };
	bool GetMCWindowMaximize() const { return GetSetting<bool>("MCWindowMaximize",settings->GetMCWindowMaximize()); };
	bool GetUseAppletWrapper() const { return GetSetting<bool>("UseAppletWrapper",settings->GetUseAppletWrapper()); };
	UpdateMode GetUpdateMode() const { return (UpdateMode) GetSetting<int>("UpdateMode",settings->GetUpdateMode()); };
	virtual bool GetAutoLogin() const { return GetSetting<bool>("AutoLogin", settings->GetAutoLogin()); }

	virtual bool GetJavaOverride() const { return GetSetting<bool>(_("OverrideJava"), false); };
	virtual void SetJavaOverride( bool value ) {  SetSetting<bool>(_("OverrideJava"), value); };

	virtual bool GetMemoryOverride() const { return GetSetting<bool>(_("OverrideMemory"), false); };
	virtual void SetMemoryOverride( bool value ) {  SetSetting<bool>(_("OverrideMemory"), value); };

	virtual bool GetWindowOverride() const { return GetSetting<bool>(_("OverrideWindow"), false); };
	virtual void SetWindowOverride( bool value ) {  SetSetting<bool>(_("OverrideWindow"), value); };

	virtual bool GetUpdatesOverride() const { return GetSetting<bool>(_("OverrideUpdates"), false); };
	virtual void SetUpdatesOverride( bool value ) {  SetSetting<bool>(_("OverrideUpdates"), value); };

	virtual bool GetLoginOverride() const { return GetSetting<bool>("OverrideLogin", false); }
	virtual void SetLoginOverride(bool value) {    SetSetting<bool>("OverrideLogin", value); }
	
	// and these are specific to instances only
	wxString GetJarVersion() const { return GetSetting<wxString>("JarVersion","Unknown"); };
	void SetJarVersion( wxString value ) {  SetSetting<wxString>("JarVersion", value); };
	
	uint64_t GetJarTimestamp() const
	{
		// no 64bit type support in wxConfig. This code is very 'meh', but works
		wxString str = GetSetting<wxString>("JarTimestamp","0");
		auto buf = str.ToAscii();
		const char * asciidata = buf.data();
		std::istringstream reader(asciidata);
		uint64_t data;
		reader >> data;
		return data;
	};
	void SetJarTimestamp( uint64_t value )
	{
		// same as above.
		std::ostringstream writer;
		writer << value;
		std::string str = writer.str();
		wxString finalstr = wxString::FromAscii(str.c_str());
		SetSetting<wxString>("JarTimestamp", finalstr);
	};
	
	/**
	 * Update the jar version and timestamp
	 * if keep_current is true, only updates the stored timestamp
	 */
	void UpdateVersion(bool keep_current = false);
	
	virtual bool IsConfigGlobal()
	{
		return false;
	}
	
protected:
	class JarModList : public ModList
	{
	public:
		JarModList(Instance *inst, const wxString& dir = wxEmptyString);

		virtual bool UpdateModList(bool quickLoad = false);

		virtual bool InsertMod(size_t index, const wxString &filename, const wxString& saveToFile = wxEmptyString);
		virtual bool DeleteMod(size_t index, const wxString& saveToFile = wxEmptyString);

	protected:
		Instance *m_inst;
	} modList;

	class FolderModList : public ModList
	{
	public:
		FolderModList(const wxString& dir = wxEmptyString)
			: ModList(dir) {}

		virtual bool UpdateModList(bool quickLoad = true)
		{
			return ModList::UpdateModList(quickLoad);
		}
		
	protected:
		virtual bool LoadModListFromDir(const wxString& loadFrom, bool quickLoad);
	};
	FolderModList mlModList;
	FolderModList coreModList;

	WorldList worldList;
	
	wxFileName rootDir;
	wxString version;
	
	wxProcess *instProc;
	bool m_running;
	bool modloader_list_inited;
	bool coremod_list_inited;
	bool jar_list_inited;
	bool world_list_initialized;
	wxString m_lastLaunchCommand;
	
	wxEvtHandler *evtHandler;
	
	void MkDirs();
	void ExtractLauncher();
	
	void OnInstProcExited(wxProcessEvent &event);
	DECLARE_EVENT_TABLE()
};


DECLARE_EVENT_TYPE(wxEVT_INST_OUTPUT, -1)

struct InstOutputEvent : wxThreadEvent
{
	InstOutputEvent(Instance *inst, wxString output, bool stdErr = false) 
		: wxThreadEvent(wxEVT_INST_OUTPUT) 
		{ m_inst = inst; m_output = output; m_stdErr = stdErr; }
	
	Instance *m_inst;
	wxString m_output;
	bool m_stdErr;
	virtual wxEvent *Clone() const
	{
		return new InstOutputEvent(m_inst, m_output, m_stdErr);
	}
};

typedef void (wxEvtHandler::*InstOutputEventFunction)(InstOutputEvent&);

#define EVT_INST_OUTPUT(fn)\
	DECLARE_EVENT_TABLE_ENTRY(wxEVT_INST_OUTPUT, wxID_ANY, -1,\
		(wxObjectEventFunction)(InstOutputEventFunction)\
		(InstOutputEventFunction) &fn, (wxObject*) NULL),

