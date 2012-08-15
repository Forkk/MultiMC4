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

//typedef std::vector<Mod> ModList;
wxDEPRECATED(typedef ModList::iterator ModIterator); // Use ModList::iterator instead
wxDEPRECATED(typedef ModList::const_iterator ConstModIterator); // Use ModList::const_iterator instead

class Instance : public wxEvtHandler
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
	
	wxProcess* Launch(wxString username, wxString sessionID, bool redirectOutput = false);
	
	void LoadModList();
	void SaveModList();
	void UpdateModList();
	ModList *GetModList();
	
	void LoadMLModList();
	ModList *GetMLModList();
	
	wxDEPRECATED(void InsertMod(size_t index, const wxFileName &source)); // Use GetModList()->InsertMod instead
	wxDEPRECATED(void DeleteMod(size_t index)); // Use GetModList()->DeleteMod instead
	
	wxDEPRECATED(void DeleteMLMod(size_t index)); // Use GetMLModList()->InsertMod instead
	
	template <typename T>
	T GetSetting(const wxString &key, T defValue) const;
	template <typename T>
	void SetSetting(const wxString &key, T value, bool suppressErrors = false);
	
	wxFileName GetSetting(const wxString &key, wxFileName defValue) const;
	void SetSetting(const wxString &key, wxFileName value, bool suppressErrors = false);

	void GetPossibleConfigFiles(wxArrayString *array, wxString dir = wxEmptyString);
	
protected:
	wxFileConfig *config;
	

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

	class MLModList : public ModList
	{
	public:
		MLModList(const wxString& dir = wxEmptyString)
			: ModList(dir) {}

		virtual bool UpdateModList(bool quickLoad = true)
		{
			return ModList::UpdateModList(quickLoad);
		}
		
	protected:
		virtual bool LoadModListFromDir(const wxString& loadFrom, bool quickLoad);
	} mlModList;
	
	wxFileName rootDir;
	
	wxProcess *instProc;
	bool m_running;
	wxString m_lastLaunchCommand;
	
	wxEvtHandler *evtHandler;
	
	void MkDirs();
	
	void ExtractLauncher();
	
	void OnInstProcExited(wxProcessEvent &event);
	
	wxDEPRECATED(void LoadModListFromDir(const wxFileName &dir, bool mlMod = false));
	
	DECLARE_EVENT_TABLE()
};


DECLARE_EVENT_TYPE(wxEVT_INST_OUTPUT, -1)

struct InstOutputEvent : wxNotifyEvent
{
	InstOutputEvent(Instance *inst, wxString output, bool stdErr = false) 
		: wxNotifyEvent(wxEVT_INST_OUTPUT) 
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

