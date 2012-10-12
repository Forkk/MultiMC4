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
#include <wx/wx.h>
#include <wx/filename.h>

class Mod
{
public:
	enum ModType
	{
		// Indicates an unspecified mod type.
		MOD_UNKNOWN,

		// The mod is a zip file containing the mod's class files.
		MOD_ZIPFILE,

		// The mod is a single file (not a zip file).
		MOD_SINGLEFILE,

		// The mod is in a folder on the filesystem.
		MOD_FOLDER,
	};

	Mod(const wxFileName &file, ModType type = MOD_UNKNOWN);
	Mod(const Mod &mod);
	
	wxFileName GetFileName() const;
	
	wxString GetName() const;
	wxString GetModID() const;
	wxString GetModVersion() const;
	wxString GetMCVersion() const;

	ModType GetModType() const;

	
	// True if this mod is a zip file. Deprecated, use 
	wxDEPRECATED(bool IsZipMod() const);

	bool operator ==(const Mod &other) const;
	
protected:

	void ReadModInfoData(wxString info);

	wxFileName modFile;
	
	wxString modID;
	wxString modName;
	wxString modVersion;
	wxString mcVersion;

	ModType modType;
};
