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

#include "langutils.h"

#include <wx/dir.h>

#include "multimc.h"
#include "appsettings.h"

#include "apputils.h"

LocaleHelper::LocaleHelper()
{
	m_locale = nullptr;
}

LocaleHelper::~LocaleHelper()
{
	if (m_locale)
	{
		delete m_locale;
	}
}

bool LocaleHelper::GetSupportedLanguages(wxArrayString& langs, wxArrayLong& ids)
{
	langs.Clear();
	ids.Clear();

	langs.Add("English");
	ids.Add(wxLANGUAGE_ENGLISH);

	wxDir dir(Path::Combine(wxGetCwd(), "lang"));

	if (!dir.IsOpened())
	{
		wxLogError(_("Failed to load the langauge list. Couldn't open language directory."));
		return false;
	}

	const wxLanguageInfo *langInfo = nullptr;
	wxString filename;
	if (dir.GetFirst(&filename))
	{
		do
		{
			langInfo = wxLocale::FindLanguageInfo(filename);

			if (langInfo)
			{
				if (wxFileExists(Path::Combine(Path::Combine(dir.GetName(), filename), 
					wxGetApp().GetAppName() + ".mo")))
				{
					langs.Add(langInfo->Description);
					ids.Add(langInfo->Language);
				}
			}
		} while (dir.GetNext(&filename));
	}

	return true;
}

bool LocaleHelper::SetLanguage(long id)
{
	if (m_locale)
		delete m_locale;

	m_locale = new wxLocale(id);
	m_locale->AddCatalogLookupPathPrefix(
		Path::Combine(wxPathOnly(wxGetApp().argv[0]), "lang"));
	return m_locale->AddCatalog(wxGetApp().GetAppName());
}

wxLocale* LocaleHelper::GetLocale()
{
	return m_locale;
}
