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

#include <wx/string.h>

#include "mod.h"

class ModList : public std::vector<Mod>
{
public:
	ModList(const wxString& dir = wxEmptyString);

	// Reloads the mod list and returns true if the list changed.
	virtual bool UpdateModList(bool quickLoad = false);

	// Returns a pointer to the mod in the list with the given filename.
	// Returns nullptr if no mod with the given filename is in the list.
	Mod *FindByFilename(const wxString& filename);

	// Returns the index of the mod in the list with the given filename.
	// Returns -1 if no mod with the given filename is in the list.
	int FindIndexByFilename(const wxString& filename);

	// Returns the mod with the given ID and version.
	// Returns nullptr if no mod is found.
	Mod *FindByID(const wxString& modID, const wxString& modVersion);

	// Returns this mod list's directory.
	wxString GetDir() const;

	// Sets this mod list's directory
	void SetDir(const wxString& dir);

	// Saves the mod list to a file.
	virtual void SaveToFile(const wxString& file);

	// Loads the mod list from a file.
	virtual void LoadFromFile(const wxString& file);

	// Adds the given mod to the list at the given index.
	// If saveToFile is not blank, saves the list to the given file.
	virtual bool InsertMod(size_t index, const wxString &filename, const wxString& saveToFile = wxEmptyString);

	// Deletes the mod at the given index.
	// If saveToFile is not blank, saves the list to the given file.
	// Returns true if successful or false if it fails.
	virtual bool DeleteMod(size_t index, const wxString& saveToFile = wxEmptyString);

protected:
	// Loads the mod list from the given directory.
	// Returns true if the list changed.
	virtual bool LoadModListFromDir(const wxString& loadFrom = wxEmptyString, bool quickLoad = false);

	wxString modsFolder;
};
