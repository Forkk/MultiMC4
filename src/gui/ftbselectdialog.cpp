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

#include "ftbselectdialog.h"

#include <wx/dir.h>

#include "utils/apputils.h"
#include "utils/httputils.h"

SelectFTBDialog::SelectFTBDialog(wxWindow *parent, const wxString& ftbDir)
	: ListSelectDialog(parent, _("Select FTB pack to import"))
{
	m_ftbDir = ftbDir;
}

bool SelectFTBDialog::DoLoadList()
{
	wxDir dir(m_ftbDir);

	if (!dir.IsOpened())
	{
		wxLogError(_("Failed to get FTB pack list. Could not open directory."));
		return false;
	}

	wxString filename;
	for (bool cont = dir.GetFirst(&filename); cont; cont = dir.GetNext(&filename))
	{
		wxString path = Path::Combine(dir.GetName(), filename);

		// Verify that it's an FTB pack.
		if (wxDirExists(path) && 
			(wxDirExists(Path::Combine(path, "minecraft")) || wxDirExists(Path::Combine(path, ".minecraft")) &&
			wxDirExists(Path::Combine(path, "instMods"))))
		{
			sList.Add(filename);
		}
	}
	return true;
}

wxString SelectFTBDialog::GetSelectedFolder() const
{
	return Path::Combine(m_ftbDir, GetSelection());
}
