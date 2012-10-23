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

#include "instance.h"
#include "stdinstance.h"

#include <wx/filesys.h>
#include <wx/sstream.h>
#include <wx/wfstream.h>
#include <wx/mstream.h>
#include <wx/dir.h>
#include <wx/zipstrm.h>
#include <wx/txtstrm.h>
#include <memory>
#include <sstream>

#include "launcher/launcherdata.h"
#include "osutils.h"
#include "datautils.h"
#include "insticonlist.h"
#include "java/javautils.h"
#include "instancemodel.h"

DEFINE_EVENT_TYPE(wxEVT_INST_OUTPUT)

// macro for adding "" around strings
#define DQuote(X) "\"" << X << "\""

const wxString cfgFileName = _("instance.cfg");

/* HACK HACK HACK HACK HACK HACK HACK HACK*
 * This is a workaround for a wxWidgets bug
 * HACK HACK HACK HACK HACK HACK HACK HACK*/
class MinecraftProcess : public wxProcess
{
public:
	MinecraftProcess ( wxEvtHandler* parent = 0 )
	:wxProcess(nullptr,wxID_ANY)
	{
		myParent = parent;
	}
protected:
	virtual void OnTerminate ( int pid, int status )
	{
		wxProcessEvent ev(wxID_ANY,pid, status);
		myParent->AddPendingEvent(ev);
	};
	wxEvtHandler * myParent;
};

bool IsValidInstance(wxFileName rootDir)
{
	return rootDir.DirExists() && wxFileExists(Path::Combine(rootDir, cfgFileName));
}

Instance *Instance::LoadInstance(wxFileName rootDir)
{
	if (IsValidInstance(rootDir))
	{
		wxFileConfig fcfg(wxEmptyString, wxEmptyString, 
			Path::Combine(rootDir, "instance.cfg"), wxEmptyString,
			wxCONFIG_USE_LOCAL_FILE | wxCONFIG_USE_RELATIVE_PATH);

		int type = 0; 
		if (!fcfg.Read("type", &type))
		{
			type = INST_TYPE_STANDARD;
		}

		switch (type)
		{
		case INST_TYPE_STANDARD:
		default:
			return new StdInstance(rootDir);
			break;
		}
	}
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
	coreModList.SetDir(GetCoreModsDir().GetFullPath());
	worldList.SetDir(GetSavesDir().GetFullPath());
	tpList.SetDir(GetTexturePacksDir().GetFullPath());
	modloader_list_inited = false;
	coremod_list_inited = false;
	jar_list_inited = false;
	world_list_initialized = false;
	tp_list_initialized = false;
	parentModel = nullptr;
	UpdateVersion();
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

void Instance::SetParentModel ( InstanceModel* parent )
{
	parentModel = parent;
}


void Instance::UpdateVersion ( bool keep_current )
{
	wxFileName jar = GetMCJar();
	if(!jar.FileExists())
	{
		SetJarTimestamp(0);
		SetJarVersion("Unknown");
		return;
	}
	
	auto dt = jar.GetModificationTime();
	dt.MakeUTC(true);
	auto time =dt.GetTicks();
	if(keep_current)
	{
		SetJarTimestamp(time);
		return;
	}
	auto saved_time = GetJarTimestamp();
	if(time != saved_time)
	{
		wxString newversion = javautils::GetMinecraftJarVersion(jar);
		SetJarTimestamp(time);
		SetJarVersion(newversion);
	}
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
	if (!GetCoreModsDir().DirExists())
		GetCoreModsDir().Mkdir();
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

	wxString dotMCDir = Path::Combine(GetRootDir(), ".minecraft");
	wxString nodotMCDir = Path::Combine(GetRootDir(), "minecraft");
	
	if (wxDirExists(dotMCDir) && !wxDirExists(nodotMCDir))
	{
		mcDir = wxFileName::DirName(dotMCDir);
	}
	else
	{
		mcDir = wxFileName::DirName(nodotMCDir);
	}
	
	return mcDir;
}

wxFileName Instance::GetBinDir() const
{
	return wxFileName::DirName(GetMCDir().GetFullPath() + _("/bin"));
}

wxFileName Instance::GetCoreModsDir() const
{
	return wxFileName::DirName(Path::Combine(GetMCDir().GetFullPath(), _("coremods")));
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


int64_t Instance::ReadVersionFile()
{
	int64_t number = -1;
	if (!GetVersionFile().FileExists()) return number;
	
	// Open the file for reading
	wxFFileInputStream input( GetVersionFile().GetFullPath() );
	if(input.IsOk())
	{
		auto len = input.GetLength();
		char * buf = new char[len];
		input.Read(buf, len);
		std::istringstream in(buf);
		in >> number;
		delete[] buf;
	}
	return number;
}

void Instance::WriteVersionFile(int64_t number)
{
	if (!GetBinDir().DirExists())
		GetBinDir().Mkdir();
	
	wxFile vFile;
	if (!vFile.Create(GetVersionFile().GetFullPath(), true))
		return;
	wxFileOutputStream outStream(vFile);
	wxString contents;
	contents << number;
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
	if(parentModel)
		parentModel->InstanceRenamed(this);
}

wxString Instance::GetIconKey() const
{
	wxString iconKey = GetSetting<wxString>(_("iconKey"), _("default"));

	if (iconKey == "default")
	{
		if (GetName().Lower().Contains("btw") ||
			GetName().Lower().Contains("better then wolves") || // Because some people are stupid :D
			GetName().Lower().Contains("better than wolves"))
		{
			iconKey = "herobrine";
		}
		else if (GetName().Lower().Contains("direwolf"))
		{
			iconKey = "enderman";
		}
	}

	return iconKey;
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

bool Instance::HasBinaries()
{
	bool isOK = true;
	isOK &= GetMCJar().FileExists();
	// FIXME: add more here, as needed
	return isOK;
}

void Instance::ExtractLauncher()
{
	wxMemoryInputStream launcherInputStream(multimclauncher, sizeof(multimclauncher));
	wxZipInputStream dezipper(launcherInputStream);
	wxFFileOutputStream launcherOutStream( Path::Combine(GetMCDir(),"MultiMCLauncher.jar") );
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

wxProcess *Instance::Launch(wxString username, wxString sessionID, bool redirectOutput)
{
	if (username.IsEmpty())
		username = _("Offline");
	
	if (sessionID.IsEmpty())
		sessionID = _("Offline");
	
	ExtractLauncher();
	
	// window size parameter (depends on some flags also)
	wxString winSizeArg;
	if (!GetUseAppletWrapper())
		winSizeArg = "compatmode";
	else if (GetMCWindowMaximize())
		winSizeArg = "max";
	else
		winSizeArg << GetMCWindowWidth() << "x" << GetMCWindowHeight();
	
	// putting together the window title
	wxString windowTitle;
	windowTitle << "MultiMC: " << GetName();
	
	// now put together the launch command in the form:
	// "%java%" %extra_args% -Xms%min_memory%m -Xmx%max_memory%m -jar MultiMCLauncher.jar "%user_name%" "%session_id%" "%window_title%" "%window_size%"
	wxString launchCmd;
	launchCmd << DQuote(GetJavaPath()) << " " << GetJvmArgs() << " -Xms" << GetMinMemAlloc() << "m" << " -Xmx" << GetMaxMemAlloc() << "m"
	          << " -jar MultiMCLauncher.jar "
	          << " " << DQuote(username) << " " << DQuote(sessionID) << " " << DQuote(windowTitle) << " " << DQuote(winSizeArg);
	m_lastLaunchCommand = launchCmd;
	
	// create a (custom) process object!
	instProc = new MinecraftProcess(this);
	if (redirectOutput)
		instProc->Redirect();
	
	// set up environment path
	wxExecuteEnv env;
	wxFileName mcDir = GetMCDir();
	mcDir.MakeAbsolute();
	env.cwd = GetMCDir().GetFullPath();
	
	// run minecraft using the stuff above :)
	int pid = wxExecute(launchCmd,wxEXEC_ASYNC|wxEXEC_HIDE_CONSOLE,instProc,&env);
	if(pid > 0)
	{
		m_running = true;
	}
	else
	{
		m_running = false;
		delete instProc;
		instProc = nullptr;
	}
	return instProc;
}

void Instance::OnInstProcExited(wxProcessEvent& event)
{
	m_running = false;
	printf("Instance exited with code %i.\n", event.GetExitCode());
	delete instProc;
	instProc = nullptr;
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

ModList *Instance::GetCoreModList()
{
	// if nothing requested the modloader list yet, load it.
	if(!coremod_list_inited)
	{
		// check the modloader mods...
		coreModList.UpdateModList();
		coremod_list_inited = true;
	}
	return &coreModList;
}

WorldList *Instance::GetWorldList()
{
	if (!world_list_initialized)
	{
		worldList.UpdateWorldList();
		world_list_initialized = true;
	}
	return &worldList;
}

TexturePackList *Instance::GetTexturePackList()
{
	if (!tp_list_initialized)
	{
		tpList.UpdateTexturePackList();
		tp_list_initialized = true;
	}
	return &tpList;
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
				fileName.GetFullPath().Contains(_("options")) ||
				fileName.GetPath().Contains(_("config")) ||
				fileName.GetName().Contains(_("config")) ||
				fileName.GetName().Contains(_("options")))
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

bool Instance::FolderModList::LoadModListFromDir(const wxString& loadFrom, bool quickLoad)
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

			if (wxFileExists(modFile.GetFullPath()) || wxDirExists(modFile.GetFullPath()))
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

wxString Instance::GetInstID() const
{
	wxString id = GetRootDir().GetFullName();
	return id;
}


BEGIN_EVENT_TABLE(Instance, wxEvtHandler)
	EVT_END_PROCESS(wxID_ANY, Instance::OnInstProcExited)
END_EVENT_TABLE()