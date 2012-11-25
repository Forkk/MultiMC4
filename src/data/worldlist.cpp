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
#include "multimc_pragma.h"
#include "worldlist.h"

#include <wx/filename.h>
#include <wx/dir.h>

#include <wx/log.h>

#include "utils/apputils.h"

WorldList::WorldList(const wxString& dir)
	: m_worldsDir(dir)
{

}

wxString WorldList::GetDir() const
{
	return m_worldsDir;
}

void WorldList::SetDir(const wxString& dir)
{
	m_worldsDir = dir;
}

void WorldList::UpdateWorldList()
{
	for (size_t i = 0; i < size(); i++)
	{
		erase(begin() + i);
		i--;
	}
	LoadWorldListFromDir();
}

bool WorldList::LoadWorldListFromDir(const wxString& loadFrom)
{
	wxString dir = loadFrom;
	if (dir.IsEmpty())
		dir = GetDir();

	bool listChanged = false;
	wxDir savesDir(dir);

	if (!savesDir.IsOpened())
	{
		wxLogError(_("Failed to open directory: ") + dir);
		return false;
	}

	wxString currentFile;
	if (savesDir.GetFirst(&currentFile))
	{
		do
		{
			wxFileName worldPath(Path::Combine(dir, currentFile));

			if (wxDirExists(worldPath.GetFullPath()) && 
				wxFileExists(Path::Combine(worldPath.GetFullPath(), wxT("level.dat"))))
			{
				World world(worldPath.GetFullPath());
				push_back(world);
				listChanged = true;
			}
		} while (savesDir.GetNext(&currentFile));
	}

	return listChanged;
}
