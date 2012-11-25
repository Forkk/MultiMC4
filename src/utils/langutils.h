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
#include <wx/intl.h>
#include <wx/string.h>
#include <wx/dynarray.h>

// Represents a certain locale.
struct LanguageDef
{
	LanguageDef(wxString name, wxString canonicalName, long id);

	wxString m_name;
	wxString m_canonicalName;
	int m_id;
};

WX_DECLARE_OBJARRAY(LanguageDef, LanguageArray);

class LocaleHelper
{
public:
	LocaleHelper();
	~LocaleHelper();

	// Updates the list of installed languages.
	bool UpdateLangList();

	// Sets the current language.
	bool SetLanguage(long id);

	wxLocale* GetLocale();

	// Gets the language list.
	const LanguageArray* GetLanguages() const;
	LanguageArray* GetLanguages();

	// Checks if a language is supported.
	bool IsLanguageSupported(long langID) const;

	// Finds the language with the given ID. If the language isn't supported,
	// returns NULL.
	// If exactMatch is true, it will look only for languages that match
	// exactly. Otherwise it will substitute similar ones. (for example, 
	// if en_GB isn't supported, en_US will be used instead)
	long FindClosestMatch(long langID, bool exactMatch = false);

protected:
	wxLocale* m_locale;

	LanguageArray m_langDefs;
};

long GetDefaultLanguage();

// Extract .mo files to their directories.
bool InstallLangFiles();
