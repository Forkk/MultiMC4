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

#include "instance.h"
#include <wx/filesys.h>
#include <wx/sstream.h>
#include <wx/wfstream.h>
#include <wx/mstream.h>
#include <wx/dir.h>
#include <wx/zipstrm.h>
#include <memory>

#include "launcher/launcherdata.h"
#include "osutils.h"
#include <datautils.h>
#include <insticonlist.h>

DEFINE_EVENT_TYPE(wxEVT_INST_OUTPUT)

const wxString cfgFileName = _("instance.cfg");

bool IsValidInstance(wxFileName rootDir)
{
	return rootDir.DirExists() && wxFileExists(Path::Combine(rootDir, cfgFileName));
}

Instance *Instance::LoadInstance(wxFileName rootDir)
{
	if (IsValidInstance(rootDir))
		return new Instance(rootDir);
	else
		return NULL;
}

Instance::Instance(const wxFileName &rootDir)
	: modList(this), m_running(false)
{
	this->rootDir = rootDir;
	config = new wxFileConfig(wxEmptyString, wxEmptyString, GetConfigPath().GetFullPath(), wxEmptyString,
		wxCONFIG_USE_LOCAL_FILE | wxCONFIG_USE_RELATIVE_PATH);
	evtHandler = NULL;
	MkDirs();

	// initialize empty mod lists - they are filled later and only if requested (see apropriate Get* methods)
	modList.SetDir(GetInstModsDir().GetFullPath());
	mlModList.SetDir(GetMLModsDir().GetFullPath());
	modloader_list_inited = false;
	jar_list_inited = false;
}

Instance::~Instance(void)
{
	delete config;
	if (IsRunning())
	{
		instProc->Detach();
	}
	Save();
}

// Makes ALL the directories! \o/
void Instance::MkDirs()
{
	if (!GetRootDir().DirExists())
		GetRootDir().Mkdir();
	
	if (!GetMCDir().DirExists())
		GetMCDir().Mkdir();
	if (!GetBinDir().DirExists())
		GetBinDir().Mkdir();
	if (!GetSavesDir().DirExists())
		GetSavesDir().Mkdir();
	if (!GetMLModsDir().DirExists())
		GetMLModsDir().Mkdir();
	if (!GetResourceDir().DirExists())
		GetResourceDir().Mkdir();
	if (!GetScreenshotsDir().DirExists())
		GetScreenshotsDir().Mkdir();
	if (!GetTexturePacksDir().DirExists())
		GetTexturePacksDir().Mkdir();
	
	if (!GetInstModsDir().DirExists())
		GetInstModsDir().Mkdir();
}

bool Instance::Save() const
{
	return false;
}

bool Instance::Load(bool loadDefaults)
{
	return false;
}

wxFileName Instance::GetRootDir() const
{
	return rootDir;
}

wxFileName Instance::GetConfigPath() const
{
	return wxFileName(rootDir.GetFullPath(), cfgFileName);
}

wxFileName Instance::GetMCDir() const
{
	wxFileName mcDir;
	
	if (IS_MAC() || wxFileExists(Path::Combine(GetRootDir(), _("minecraft"))))
	{
		mcDir = wxFileName::DirName(Path::Combine(GetRootDir(), _("minecraft")));
	}
	else
	{
		mcDir = wxFileName::DirName(Path::Combine(GetRootDir(), _(".minecraft")));
	}
	
	return mcDir;
}

wxFileName Instance::GetBinDir() const
{
	return wxFileName::DirName(GetMCDir().GetFullPath() + _("/bin"));
}

wxFileName Instance::GetMLModsDir() const
{
	return wxFileName::DirName(Path::Combine(GetMCDir().GetFullPath(), _("mods")));
}

wxFileName Instance::GetResourceDir() const
{
	return wxFileName::DirName(Path::Combine(GetMCDir().GetFullPath(), _("resources")));
}

wxFileName Instance::GetSavesDir() const
{
	return wxFileName::DirName(Path::Combine(GetMCDir().GetFullPath(), _("saves")));
}

wxFileName Instance::GetScreenshotsDir() const
{
	return wxFileName::DirName(Path::Combine(GetMCDir().GetFullPath(), _("screenshots")));
}

wxFileName Instance::GetTexturePacksDir() const
{
	return wxFileName::DirName(Path::Combine(GetMCDir().GetFullPath(), _("texturepacks")));
}


wxFileName Instance::GetInstModsDir() const
{
	return wxFileName::DirName(Path::Combine(GetRootDir().GetFullPath(), _("instMods")));
}

wxFileName Instance::GetVersionFile() const
{
	return wxFileName::FileName(GetBinDir().GetFullPath() + _("/version"));
}

wxFileName Instance::GetMCBackup() const
{
	return wxFileName::FileName(GetBinDir().GetFullPath() + _("/mcbackup.jar"));
}

wxFileName Instance::GetMCJar() const
{
	return wxFileName::FileName(GetBinDir().GetFullPath() + _("/minecraft.jar"));
}

wxFileName Instance::GetModListFile() const
{
	return wxFileName::FileName(Path::Combine(GetRootDir(), _("modlist")));
}


wxString Instance::ReadVersionFile()
{
	if (!GetVersionFile().FileExists())
		return _("");
	
	// Open the file for reading
	wxFSFile *vFile = wxFileSystem().OpenFile(GetVersionFile().GetFullPath(), wxFS_READ);
	wxString retVal;
	wxStringOutputStream outStream(&retVal);
	if(!vFile)
		return _("");
	wxInputStream * versionFileStream = vFile->GetStream();
	if(!versionFileStream)
		return _("");
	outStream.Write(*versionFileStream);
	wxDELETE(vFile);
	return retVal;
}

void Instance::WriteVersionFile(const wxString &contents)
{
	if (!GetBinDir().DirExists())
		GetBinDir().Mkdir();
	
	wxFile vFile;
	if (!vFile.Create(GetVersionFile().GetFullPath(), true))
		return;
	wxFileOutputStream outStream(vFile);
	wxStringInputStream inStream(contents);
	outStream.Write(inStream);
}


wxString Instance::GetName() const
{
	return GetSetting<wxString>(_("name"), _("Unnamed Instance"));
}

void Instance::SetName(wxString name)
{
	SetSetting<wxString>(_("name"), name);
}

wxString Instance::GetIconKey() const
{
	return GetSetting<wxString>(_("iconKey"), _("default"));
}

void Instance::SetIconKey(wxString iconKey)
{
	SetSetting<wxString>(_("iconKey"), iconKey);
}

wxString Instance::GetNotes() const
{
	return GetSetting<wxString>(_("notes"), _(""));
}

void Instance::SetNotes(wxString notes)
{
	SetSetting<wxString>(_("notes"), notes);
}

bool Instance::ShouldRebuild() const
{
	return GetSetting<bool>(_("NeedsRebuild"), false);
}

void Instance::SetNeedsRebuild(bool value)
{
	SetSetting<bool>(_("NeedsRebuild"), value);
}


wxProcess *Instance::Launch(wxString username, wxString sessionID, bool redirectOutput)
{
	if (username.IsEmpty())
		username = _("Offline");
	
	if (sessionID.IsEmpty())
		sessionID = _("Offline");
	
	ExtractLauncher();
	
	wxString javaPath = settings->GetJavaPath();
	wxString additionalArgs = settings->GetJvmArgs();
	int xms = settings->GetMinMemAlloc();
	int xmx = settings->GetMaxMemAlloc();
	wxFileName mcDirFN = GetMCDir().GetFullPath();
	mcDirFN.MakeAbsolute();
	wxString mcDir = mcDirFN.GetFullPath();
	wxString wdArg = wxGetCwd();
	wxString winSizeArg = wxString::Format(_("%ix%i"), 
		settings->GetMCWindowWidth(), settings->GetMCWindowHeight());

	if (!settings->GetUseAppletWrapper())
		winSizeArg = _("compatmode");
	else if (settings->GetMCWindowMaximize())
		winSizeArg = _("max");
	
// 	if (IS_WINDOWS())
// 	{
	mcDir.Replace(_("\\"), _("\\\\"));
	wdArg.Replace(_("\\"), _("\\\\"));
// 	}
	
	wxString title = _("MultiMC: ") + GetName();
	// FIXME: implement instance icons here (pass a filename of raw bitmap)
	wxString launchCmd = wxString::Format(_("\"%s\" %s -Xmx%im -Xms%im -cp \"%s\" -jar MultiMCLauncher.jar \"%s\" \"%s\" \"%s\" \"%s\" \"%s\""),
		javaPath.c_str(), additionalArgs.c_str(), xmx, xms, wdArg.c_str(), 
		mcDir.c_str(), username.c_str(), sessionID.c_str(), title.c_str(), 
		winSizeArg.c_str());
	m_lastLaunchCommand = launchCmd;
	
	instProc = new wxProcess(this);
	
	if (redirectOutput)
		instProc->Redirect();
	
	instProc = wxProcess::Open(launchCmd, wxEXEC_ASYNC);
	if(instProc)
		m_running = true;
	return instProc;
}

void Instance::ExtractLauncher()
{
	
	wxMemoryInputStream launcherInputStream(multimclauncher, sizeof(multimclauncher));
	wxZipInputStream dezipper(launcherInputStream);
	wxFFileOutputStream launcherOutStream(_("MultiMCLauncher.jar"));
	wxZipOutputStream zipper(launcherOutStream);
	std::auto_ptr<wxZipEntry> entry;
	// copy all files from the old zip file
	while (entry.reset(dezipper.GetNextEntry()), entry.get() != NULL)
		if (!zipper.CopyEntry(entry.release(), dezipper))
			break;
	// add the icon file
	zipper.PutNextEntry(_("icon.png"));
	InstIconList * iconList = InstIconList::Instance();
	//FIXME: what if there is no such image?
	wxImage &img =  iconList->getImageForKey(GetIconKey());
	img.SaveFile(zipper,wxBITMAP_TYPE_PNG);
}

void Instance::OnInstProcExited(wxProcessEvent& event)
{
	m_running = false;
	printf("Instance exited with code %i.\n", event.GetExitCode());
	if (evtHandler != NULL)
	{
		evtHandler->AddPendingEvent(event);
	}
}

void Instance::SetEvtHandler(wxEvtHandler* handler)
{
	evtHandler = handler;
}

bool Instance::IsRunning() const
{
	return m_running;
}

wxProcess* Instance::GetInstProcess() const
{
	return instProc;
}

wxString Instance::GetLastLaunchCommand() const
{
	return m_lastLaunchCommand;
}

ModList *Instance::GetModList()
{
	// if nothing requested the jar list yet, load it.
	if(!jar_list_inited)
	{
		// load jar mod list from file, update it and save the list to file again
		modList.LoadFromFile(GetModListFile().GetFullPath());
		modList.UpdateModList();
		modList.SaveToFile(GetModListFile().GetFullPath());
		jar_list_inited = true;
	}
	return &modList;
}

ModList *Instance::GetMLModList()
{
	// if nothing requested the modloader list yet, load it.
	if(!modloader_list_inited)
	{
		// check the modloader mods...
		mlModList.UpdateModList();
		modloader_list_inited = true;
	}
	return &mlModList;
}

template <typename T>
T Instance::GetSetting(const wxString &key, T defValue) const
{
	T val;
	if (config->Read(key, &val))
		return val;
	else
		return defValue;
}

template <typename T>
void Instance::SetSetting(const wxString &key, T value, bool suppressErrors)
{
	if (!config->Write(key, value) && !suppressErrors)
		wxLogError(_("Failed to write config setting %s"), key.c_str());
	config->Flush();
}

wxFileName Instance::GetSetting(const wxString &key, wxFileName defValue) const
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
}

void Instance::SetSetting(const wxString &key, wxFileName value, bool suppressErrors)
{
	if (!config->Write(key, value.GetFullPath()) && !suppressErrors)
		wxLogError(_("Failed to write config setting %s"), key.c_str());
	config->Flush();
}

void Instance::GetPossibleConfigFiles(wxArrayString *array, wxString dir)
{
	if (dir.IsEmpty())
		dir = GetMCDir().GetFullPath();
	
	wxDir mcDir(dir);

	wxString currentFile;
	if (mcDir.GetFirst(&currentFile))
	{
		do
		{
			wxFileName fileName(Path::Combine(dir, currentFile));
			
			if (wxDirExists(fileName.GetFullPath()))
			{
				if (currentFile != _("saves"))
					GetPossibleConfigFiles(array, fileName.GetFullPath());
			}
			else if (fileName.GetExt() == _("cfg") || 
				fileName.GetExt() == _("conf") || 
				fileName.GetExt() == _("config") || 
				fileName.GetExt() == _("props") || 
				fileName.GetExt() == _("properties") ||
				fileName.GetExt() == _("xml") ||
				fileName.GetExt() == _("yml") ||
				fileName.GetFullPath().Contains(_("options")))
			{
				fileName.MakeRelativeTo(GetRootDir().GetFullPath());
				array->Add(fileName.GetFullPath());
			}
		} while (mcDir.GetNext(&currentFile));
	}
}

Instance::JarModList::JarModList(Instance *inst, const wxString& dir)
	: ModList(dir)
{
	m_inst = inst;
}

bool Instance::JarModList::InsertMod(size_t index, const wxString &filename, const wxString& saveToFile)
{
	wxString saveFile = saveToFile;
	if (saveToFile.IsEmpty())
		saveFile = m_inst->GetModListFile().GetFullPath();

	if (ModList::InsertMod(index, filename, saveFile))
	{
		m_inst->SetNeedsRebuild();
		return true;
	}
	return false;
}

bool Instance::JarModList::DeleteMod(size_t index, const wxString& saveToFile)
{
	wxString saveFile = saveToFile;
	if (saveToFile.IsEmpty())
		saveFile = m_inst->GetModListFile().GetFullPath();

	if (ModList::DeleteMod(index, saveFile))
	{
		m_inst->SetNeedsRebuild();
		return true;
	}
	return false;
}

bool Instance::JarModList::UpdateModList(bool quickLoad)
{
	if (ModList::UpdateModList(quickLoad))
	{
		m_inst->SetNeedsRebuild();
		return true;
	}
	return false;
}

bool Instance::MLModList::LoadModListFromDir(const wxString& loadFrom, bool quickLoad)
{
	wxString dir(loadFrom.IsEmpty() ? modsFolder : loadFrom);

	if (!wxDirExists(dir))
		return false;

	bool listChanged = false;
	wxDir modDir(dir);

	if (!modDir.IsOpened())
	{
		wxLogError(_("Failed to open directory: ") + dir);
		return false;
	}

	wxString currentFile;
	if (modDir.GetFirst(&currentFile))
	{
		do
		{
			wxFileName modFile(Path::Combine(dir, currentFile));

			if (wxFileExists(modFile.GetFullPath()))
			{
				if (quickLoad || FindByFilename(modFile.GetFullPath()) == nullptr)
				{
					Mod mod(modFile.GetFullPath());
					push_back(mod);
					listChanged = true;
				}
			}
		} while (modDir.GetNext(&currentFile));
	}

	return listChanged;
}


BEGIN_EVENT_TABLE(Instance, wxEvtHandler)
	EVT_END_PROCESS(wxID_ANY, Instance::OnInstProcExited)
END_EVENT_TABLE()