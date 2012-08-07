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
#include <wx/wx.h>
#include <wx/filename.h>

class Mod
{
public:
	Mod(const wxFileName &file);
	Mod(const Mod &mod);
	
	wxFileName GetFileName() const;
	
	wxString GetName() const;
	wxString GetModID() const;
	wxString GetModVersion() const;
	wxString GetMCVersion() const;
	
	// True if this mod is a zip file.
	bool IsZipMod() const;

	bool operator ==(const Mod &other) const;
	
protected:
	wxFileName modFile;
	
	wxString modName;
	wxString modVersion;
	wxString mcVersion;
};
