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
#include <vector>
#include <wx/wx.h>
#include <wx/filesys.h>
#include <wx/config.h>
#include <wx/fileconf.h>
#include <wx/process.h>

#include "appsettings.h"
#include "mod.h"
#include "modlist.h"

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
	
	wxString ReadVersionFile();
	void WriteVersionFile(const wxString& contents);
	
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

	virtual bool GetJavaOverride() const { return GetSetting<bool>(_("OverrideJava"), false); };
	virtual void SetJavaOverride( bool value ) {              SetSetting<bool>(_("OverrideJava"), value); };

	virtual bool GetMemoryOverride() const { return GetSetting<bool>(_("OverrideMemory"), false); };
	virtual void SetMemoryOverride( bool value ) {              SetSetting<bool>(_("OverrideMemory"), value); };

	virtual bool GetWindowOverride() const { return GetSetting<bool>(_("OverrideWindow"), false); };
	virtual void SetWindowOverride( bool value ) {              SetSetting<bool>(_("OverrideWindow"), value); };

	virtual bool GetUpdatesOverride() const { return GetSetting<bool>(_("OverrideUpdates"), false); };
	virtual void SetUpdatesOverride( bool value ) {              SetSetting<bool>(_("OverrideUpdates"), value); };
	
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
	
	wxFileName rootDir;
	
	wxProcess *instProc;
	bool m_running;
	bool modloader_list_inited;
	bool coremod_list_inited;
	bool jar_list_inited;
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

