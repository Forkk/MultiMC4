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

#include "texturepack.h"

TexturePack::TexturePack(const wxString& file)
	: m_file(file), m_name(wxFileName(file).GetFullName())
{

}

wxString TexturePack::GetFileName() const
{
	return m_file;
}

void TexturePack::SetFileName(const wxString& file)
{
	m_file = file;
}


wxString TexturePack::GetName() const
{
	return m_name;
}

void TexturePack::SetName(const wxString& name)
{
	m_name = name;
}
