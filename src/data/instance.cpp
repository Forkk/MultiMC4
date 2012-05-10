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

#include "launcherdata.h"

DEFINE_EVENT_TYPE(wxEVT_INST_OUTPUT)

const wxString cfgFileName = _("instance.cfg");

bool IsValidInstance(wxFileName rootDir)
{
	return rootDir.DirExists() && wxFileExists(Path::Combine(rootDir, cfgFileName));
}

Instance *Instance::LoadInstance(wxFileName rootDir)
{
	Instance *inst = new Instance(rootDir, _(""));
	if (!inst->Load())
		return (Instance*)(NULL);
	return inst;
}

Instance::Instance(wxFileName rootDir, wxString name)
{
	this->rootDir = rootDir;
	Load(true);
	this->name = name;
	evtHandler = NULL;
	MkDirs();
}

Instance::~Instance(void)
{
	if (IsRunning())
	{
		instProc->Detach();
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
	if (!GetRootDir().DirExists())
	{
		GetRootDir().Mkdir();
	}

	wxFileName filename = GetConfigPath();
	using boost::property_tree::ptree;
	ptree pt;

	pt.put<std::string>("name", stdStr(name));
	pt.put<std::string>("iconKey", stdStr(iconKey));
	pt.put<std::string>("notes", stdStr(notes));
	pt.put<bool>("NeedsRebuild", needsRebuild);
	pt.put<bool>("AskUpdate", askUpdate);

	write_ini(stdStr(filename.GetFullPath()).c_str(), pt);
	return true;
}

bool Instance::Load(bool loadDefaults)
{
	using boost::property_tree::ptree;
	ptree pt;

	wxFileName filename = GetConfigPath();
	try
	{
		if (!loadDefaults)
			read_ini(stdStr(filename.GetFullPath()).c_str(), pt);
	}
	catch (boost::property_tree::ini_parser_error e)
	{
		wxLogError(_("Failed to parse instance config file '%s'. %s"), 
			stdStr(filename.GetFullPath()).c_str(),
			e.message().c_str());
		return false;
	}

	name = wxStr(pt.get<std::string>("name", "Unnamed Instance"));
	iconKey = wxStr(pt.get<std::string>("iconKey", "default"));
	notes = wxStr(pt.get<std::string>("notes", ""));
	
	needsRebuild = pt.get<bool>("NeedsRebuild", false);
	askUpdate = pt.get<bool>("AskUpdate", true);
	return true;
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
	
	if (ENUM_CONTAINS(wxPlatformInfo::Get().GetOperatingSystemId(), wxOS_MAC) || 
		wxFileExists(Path::Combine(GetRootDir(), _("minecraft"))))
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


wxString Instance::ReadVersionFile()
{
	if (!GetVersionFile().FileExists())
		return _("");
	
	// Open the file for reading
	wxFSFile *vFile = wxFileSystem().OpenFile(GetVersionFile().GetFullPath(), wxFS_READ);
	wxString retVal;
	wxStringOutputStream outStream(&retVal);
	outStream.Write(*vFile->GetStream());
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
	return name;
}

void Instance::SetName(wxString name)
{
	this->name = name;
}

wxString Instance::GetIconKey() const
{
	return iconKey;
}

void Instance::SetIconKey(wxString iconKey)
{
	this->iconKey = iconKey;
}

wxProcess *Instance::Launch(wxString username, wxString sessionID, bool redirectOutput)
{
	if (username.IsEmpty())
		username = _("Offline");
	
	if (sessionID.IsEmpty())
		sessionID = _("Offline");
	
	ExtractLauncher();
	
	wxString javaPath = settings.javaPath.GetFullPath();
	int xms = settings.minMemAlloc;
	int xmx = settings.maxMemAlloc;
	wxFileName mcDirFN = GetMCDir().GetFullPath();
	mcDirFN.MakeAbsolute();
	wxString mcDir = mcDirFN.GetFullPath();
	
	wxString launchCmd = wxString::Format(_("\"%s\" -Xmx%im -Xms%im -cp \"%s\" MultiMCLauncher \"%s\" \"%s\" \"%s\""),
		javaPath.c_str(), xmx, xms, wxGetCwd().c_str(), mcDir.c_str(), username.c_str(), sessionID.c_str());
	m_lastLaunchCommand = launchCmd;
	
	instProc = new wxProcess(this);
	
	if (redirectOutput)
		instProc->Redirect();
	
	wxExecute(launchCmd, wxEXEC_ASYNC, instProc);
	m_running = true;
	
	return instProc;
}

void Instance::ExtractLauncher()
{
	wxMemoryInputStream launcherInputStream(MultiMCLauncher_class, MultiMCLauncher_class_len);
	wxFileOutputStream launcherOutStream(_("MultiMCLauncher.class"));
	launcherOutStream.Write(launcherInputStream);
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


BEGIN_EVENT_TABLE(Instance, wxEvtHandler)
	EVT_END_PROCESS(wxID_ANY, Instance::OnInstProcExited)
END_EVENT_TABLE()