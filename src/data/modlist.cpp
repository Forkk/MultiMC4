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

#include "modlist.h"

#include <wx/dir.h>
#include <wx/wfstream.h>
#include <wx/file.h>

#include "apputils.h"
#include "datautils.h"

ModList::ModList(const wxString &dir)
	: modsFolder(dir)
{
	
}

bool ModList::UpdateModList()
{
	bool listChanged = false;

	// Check for mods in the list whose files do not exist and remove them from the list.
	for (size_t i = 0; i < size(); i++)
	{
		if (!at(i).GetFileName().FileExists())
		{
			erase(begin() + i);
			i--;
			listChanged = true;
		}
	}

	// Add any mods in the mods folder that aren't already in the list.
	if (LoadModListFromDir())
		listChanged = true;

	return listChanged;
}

bool ModList::LoadModListFromDir(const wxString& loadFrom)
{
	wxString dir(loadFrom.IsEmpty() ? modsFolder : loadFrom);

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
				if (FindByFilename(modFile.GetFullPath()) == nullptr)
				{
					Mod mod(modFile.GetFullPath());
					push_back(mod);
					listChanged = true;
				}
			}
			else if (wxDirExists(modFile.GetFullPath()))
			{
				if (LoadModListFromDir(modFile.GetFullPath()))
					listChanged = true;
			}
		} while (modDir.GetNext(&currentFile));
	}

	return listChanged;
}

Mod *ModList::FindByFilename(const wxString& filename)
{
	// Search the list for a mod with the given filename.
	for (iterator iter = begin(); iter != end(); ++iter)
	{
		if (iter->GetFileName().SameAs(wxFileName(filename)))
			return &(*iter);
	}

	// If nothing is found, return nullptr.
	return nullptr;
}

int ModList::FindIndexByFilename(const wxString& filename)
{
	// Search the list for a mod with the given filename.
	int i = 0;
	for (iterator iter = begin(); iter != end(); ++iter, i++)
	{
		if (iter->GetFileName().SameAs(wxFileName(filename)))
			return i;
	}

	// If nothing is found, return nullptr.
	return -1;
}

void ModList::LoadFromFile(const wxString& file)
{
	wxFFileInputStream inputStream(file);
	wxStringList modListFile = ReadAllLines(inputStream);

	for (wxStringList::iterator iter = modListFile.begin(); iter != modListFile.end(); iter++)
	{
		// Normalize the path to the instMods dir.
		wxFileName modFile(*iter);
		modFile.Normalize(wxPATH_NORM_ALL, modsFolder);
		modFile.MakeRelativeTo();

		if (FindByFilename(modFile.GetFullPath()) == nullptr)
		{
			push_back(Mod(modFile));
		}
	}
}

void ModList::SaveToFile(const wxString& file)
{
	wxString text;
	for (iterator iter = begin(); iter != end(); ++iter)
	{
		wxFileName modFile = iter->GetFileName();
		modFile.MakeRelativeTo(modsFolder);
		text.Append(modFile.GetFullPath());
		text.Append(_("\n"));
	}

	wxTempFileOutputStream out(file);
	WriteAllText(out, text);
	out.Commit();
}

bool ModList::InsertMod(size_t index, const wxString &filename, const wxString& saveToFile)
{
	wxFileName source(filename);
	wxFileName dest(Path::Combine(modsFolder, source.GetFullName()));

	if (!source.SameAs(dest))
	{
		wxCopyFile(source.GetFullPath(), dest.GetFullPath());
	}

	dest.MakeRelativeTo();

	int oldIndex = FindIndexByFilename(dest.GetFullPath());

	if (oldIndex != -1)
	{
		erase(begin() + oldIndex);
	}

	if (index >= size())
		push_back(Mod(dest));
	else
		insert(begin() + index, Mod(dest));

	if (!saveToFile.IsEmpty())
		SaveToFile(saveToFile);

	return true;
}

bool ModList::DeleteMod(size_t index, const wxString& saveToFile)
{
	Mod *mod = &at(index);
	if (wxRemoveFile(mod->GetFileName().GetFullPath()))
	{
		erase(begin() + index);
		
		if (!saveToFile.IsEmpty())
			SaveToFile(saveToFile);

		return true;
	}
	else
	{
		wxLogError(_("Failed to delete mod."));
	}
	return false;
}

wxString ModList::GetDir() const
{
	return modsFolder;
}

void ModList::SetDir(const wxString& dir)
{
	modsFolder = dir;
}
