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

#include "mod.h"

Mod::Mod(const wxFileName& file)
{
	modFile = file;
	
	modName = modFile.GetName();
}

Mod::Mod(const Mod& mod)
{
	modFile = mod.GetFileName();
	modName = mod.GetName();
	modVersion = mod.GetModVersion();
	mcVersion = mod.GetMCVersion();
}

wxFileName Mod::GetFileName() const
{
	return modFile;
}

wxString Mod::GetName() const
{
	return modName;
}

wxString Mod::GetModVersion() const
{
	return modVersion;
}

wxString Mod::GetMCVersion() const
{
	return mcVersion;
}
