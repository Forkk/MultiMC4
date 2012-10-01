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

#include <wx/string.h>

#include "world.h"

class WorldList : public std::vector<World>
{
public:
	WorldList(const wxString& dir = wxEmptyString);

	// Reloads the world list.
	virtual void UpdateWorldList();

	// Returns a pointer to the world in the list with the given filename.
	// Returns nullptr if no world with the given filename is in the list.
	World *FindByFilename(const wxString& filename);

	// Returns the index of the world in the list with the given filename.
	// Returns -1 if no world with the given filename is in the list.
	int FindIndexByFilename(const wxString& filename);

	// Returns this world list's directory.
	wxString GetDir() const;

	// Sets this world list's directory
	void SetDir(const wxString& dir);

protected:
	// Loads the save list from the given directory.
	// Returns true if the list changed.
	virtual bool LoadWorldListFromDir(const wxString& loadFrom = wxEmptyString);

	wxString m_worldsDir;
};
