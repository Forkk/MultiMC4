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
#include "texturepacklist.h"

#include <wx/filename.h>
#include <wx/dir.h>

#include <wx/log.h>

#include "utils/apputils.h"

TexturePackList::TexturePackList(const wxString& dir)
	: m_tpackDir(dir)
{

}

wxString TexturePackList::GetDir() const
{
	return m_tpackDir;
}

void TexturePackList::SetDir(const wxString& dir)
{
	m_tpackDir = dir;
}

void TexturePackList::UpdateTexturePackList()
{
	for (size_t i = 0; i < size(); i++)
	{
		erase(begin() + i);
		i--;
	}
	LoadTexturePackListFromDir();
}

bool TexturePackList::LoadTexturePackListFromDir(const wxString& loadFrom)
{
	wxString dir = loadFrom;
	if (dir.IsEmpty())
		dir = GetDir();

	bool listChanged = false;
	wxDir tpackDir(dir);

	if (!tpackDir.IsOpened())
	{
		wxLogError(_("Failed to open directory: ") + dir);
		return false;
	}

	wxString currentFile;
	if (tpackDir.GetFirst(&currentFile))
	{
		do
		{
			wxFileName tpPath(Path::Combine(dir, currentFile));

			if (wxFileExists(tpPath.GetFullPath()))
			{
				TexturePack tpack(tpPath.GetFullPath());
				push_back(tpack);
				listChanged = true;
			}
		} while (tpackDir.GetNext(&currentFile));
	}

	return listChanged;
}

bool TexturePackList::DeletePack(size_t index)
{
	TexturePack *pack = &at(index);
	if (wxRemoveFile(pack->GetFileName()))
	{
		erase(begin() + index);
		return true;
	}
	else
	{
		wxLogError(_("Failed to delete mod."));
	}
	return false;
}
