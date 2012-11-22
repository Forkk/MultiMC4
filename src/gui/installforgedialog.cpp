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

#include "installforgedialog.h"

#include <wx/gbsizer.h>
#include <wx/regex.h>

#include "utils/apputils.h"
#include "utils/httputils.h"
#include "forgeversions.h"

InstallForgeDialog::InstallForgeDialog(wxWindow *parent)
	: ListSelectDialog(parent, _("Install Minecraft Forge"))
{
	// Custom GUI stuff.

	// Clear columns and add our own.
	listCtrl->DeleteAllColumns();
	listCtrl->AppendColumn(_("Filename"), wxLIST_FORMAT_LEFT);
	listCtrl->AppendColumn(_("Minecraft Version"), wxLIST_FORMAT_RIGHT, 120);

	// Show column headers
	ShowHeader(true);
}

bool InstallForgeDialog::DoLoadList()
{
	wxString dlURL = "http://files.minecraftforge.net";

	wxString buildListText;
	if (DownloadString(dlURL, &buildListText))
	{
		wxRegEx forgeRegex;
		if (!forgeRegex.Compile("minecraftforge-(universal|client)-([0-9]+.[0-9]+.[0-9]+.[0-9]+).zip"))
		{
			wxLogError("Regex failed to compile.");
			return false;
		}

		while (forgeRegex.Matches(buildListText))
		{
			size_t start, len;
			forgeRegex.GetMatch(&start, &len);

			wxString fileName = buildListText.Mid(start, len);
			if (sList.IsEmpty() || sList.Last() != fileName)
			{
				sList.push_back(fileName);
			}

			forgeRegex.ReplaceFirst(&buildListText, wxEmptyString);
		}
	}
	else
	{
		wxLogError(_("Failed to load build list. Check your internet connection."));
		return false;
	}

	return true;
}

wxString InstallForgeDialog::OnGetItemText(long item, long column)
{
	switch (column)
	{
	case 1:
		return forgeutils::MCVersionFromForgeFilename(sList[item]);

	default:
		return ListSelectDialog::OnGetItemText(item, column);
	}
}
