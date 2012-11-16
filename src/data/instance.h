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
#include "texturepack.h"
#include "texturepacklist.h"


class InstanceModel;

bool IsValidInstance(wxFileName rootDir);

// Defines an "ignored" setting. These are settings where the instance just 
// returns their value in the global application settings.
#define DEFINE_IGNORED_SETTING(settingName, typeName) \
	virtual typeName Get ## settingName() const { return settings->Get ## settingName(); };

#define DEFINE_OVERRIDDEN_SETTING_ADVANCED(funcName, cfgEntryName, typeName) \
	typeName Get ## funcName() const { return GetSetting<typeName>(cfgEntryName, settings->Get ## funcName()); }

#define DEFINE_OVERRIDDEN_SETTING(settingName, typeName) \
	DEFINE_OVERRIDDEN_SETTING_ADVANCED(settingName, STR_VALUE(settingName), typeName)

#define DEFINE_OVERRIDE_SETTING(overrideName) \
	virtual bool Get ## overrideName ## Override() const { return GetSetting<bool>(STR_VALUE(Override ## overrideName), false); } \
	virtual void Set ## overrideName ## Override( bool value ) {  SetSetting<bool>(STR_VALUE(Override ## overrideName), value); }

class Instance : public wxEvtHandler, public SettingsBase
{
public:
	enum Type
	{
		INST_TYPE_STANDARD,
	};

	static Instance *LoadInstance(wxString rootDir);
	Instance(const wxString &rootDir);
	~Instance(void);
	
	bool Save() const;
	bool Load(bool loadDefaults = false);

	// Returns the instance's ID. (the root directory name)
	wxString GetInstID() const;
	
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

	virtual Type GetType() const = 0;
	
	wxProcess *GetInstProcess() const;
	
	wxString GetLastLaunchCommand() const;
	
	void SetEvtHandler(wxEvtHandler *handler);
	
	bool HasBinaries();
	wxProcess* Launch(wxString username, wxString sessionID, bool redirectOutput = false);
	
	ModList *GetModList();
	ModList *GetMLModList();
	ModList *GetCoreModList();

	WorldList *GetWorldList();

	TexturePackList *GetTexturePackList();
	
	void GetPossibleConfigFiles(wxArrayString *array, wxString dir = wxEmptyString);
	
	// these just pass on through
	DEFINE_IGNORED_SETTING(AutoCloseConsole, bool);
	DEFINE_IGNORED_SETTING(AutoUpdate, bool);
	DEFINE_IGNORED_SETTING(ConsoleStderrColor, wxColour);
	DEFINE_IGNORED_SETTING(ConsoleStdoutColor, wxColour);
	DEFINE_IGNORED_SETTING(ConsoleSysMsgColor, wxColour);
	DEFINE_IGNORED_SETTING(GUIMode, GUIMode);
	DEFINE_IGNORED_SETTING(IconsDir, wxFileName);
	DEFINE_IGNORED_SETTING(InstDir, wxFileName);
	DEFINE_IGNORED_SETTING(ModsDir, wxFileName);
	DEFINE_IGNORED_SETTING(ShowConsole, bool);
	DEFINE_IGNORED_SETTING(InstSortMode, InstSortMode);

	// and these are overrides
	DEFINE_OVERRIDDEN_SETTING_ADVANCED(JavaPath, JPATH_FIELD_NAME, wxString);
	DEFINE_OVERRIDDEN_SETTING(JvmArgs, wxString);

	DEFINE_OVERRIDDEN_SETTING_ADVANCED(MaxMemAlloc, "MaxMemoryAlloc", int);
	DEFINE_OVERRIDDEN_SETTING_ADVANCED(MinMemAlloc, "MinMemoryAlloc", int);

	DEFINE_OVERRIDDEN_SETTING(MCWindowHeight, int);
	DEFINE_OVERRIDDEN_SETTING(MCWindowWidth, int);
	DEFINE_OVERRIDDEN_SETTING(MCWindowMaximize, bool);
	DEFINE_OVERRIDDEN_SETTING(UseAppletWrapper, bool);

	UpdateMode GetUpdateMode() const { return (UpdateMode)GetSetting<int>("UpdateMode", settings->GetUpdateMode()); };

	DEFINE_OVERRIDDEN_SETTING(AutoLogin, bool);

	DEFINE_OVERRIDE_SETTING(Java);
	DEFINE_OVERRIDE_SETTING(Memory);
	DEFINE_OVERRIDE_SETTING(Window);
	DEFINE_OVERRIDE_SETTING(Updates);
	DEFINE_OVERRIDE_SETTING(Login);
	
	// and these are specific to instances only
	wxString GetJarVersion() const { return GetSetting<wxString>("JarVersion","Unknown"); };
	void SetJarVersion( wxString value ) {  SetSetting<wxString>("JarVersion", value); };

	uint64_t GetLastLaunch() const
	{
		// no 64bit type support in wxConfig. This code is very 'meh', but works
		wxString str = GetSetting<wxString>("lastLaunch", "0").Trim(true).Trim(false);
		auto buf = str.ToAscii();

		const wxString numbers = "1234567890";
		for (int i = 0; i < str.Length(); i++)
		{
			if (!numbers.Contains(str[i]))
			{
				return 0;
			}
		}

		const char * asciidata = buf.data();
		std::istringstream reader(asciidata);
		uint64_t data;
		reader >> data;
		return data;
	}

	void SetLastLaunch(uint64_t value)
	{
		// same as above.
		std::ostringstream writer;
		writer << value;
		std::string str = writer.str();
		wxString finalstr = wxString::FromAscii(str.c_str());
		SetSetting<wxString>("lastLaunch", finalstr);
	}

	void SetLastLaunchNow()
	{
		SetLastLaunch(wxDateTime::Now().GetTicks());
	}
	
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
	
	/// Get the instance's group.
	wxString GetGroup();

	/// Set the instance's group.
	void SetGroup(const wxString& group);
	
	/**
	 * Update the jar version and timestamp
	 * if keep_current is true, only updates the stored timestamp
	 */
	void UpdateVersion(bool keep_current = false);
	
	virtual bool IsConfigGlobal()
	{
		return false;
	}
	
	/// Make this instance report relevant changes to the model
	void SetParentModel ( InstanceModel* parent );
	
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
	InstanceModel * parentModel;
	FolderModList mlModList;
	FolderModList coreModList;

	WorldList worldList;

	TexturePackList tpList;
	
	wxFileName rootDir;
	wxString version;
	wxString group;
	
	wxProcess *instProc;
	bool m_running;
	bool modloader_list_inited;
	bool coremod_list_inited;
	bool jar_list_inited;
	bool world_list_initialized;
	bool tp_list_initialized;
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

