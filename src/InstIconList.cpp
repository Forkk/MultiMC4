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

#include "InstIconList.h"

#include "InstIcons.h"

struct InstIconDef
{
	InstIconDef(wxString key, wxImage image)
	{
		this->key = key;
		this->image = image;
	}

	wxString key;
	wxImage image;
};

InstIconList::InstIconList(int width, int height, wxString customIconDirName)
{
	const InstIconDef builtInIcons[] =
	{
		InstIconDef(_T("grass"), wxMEMORY_IMAGE(grass)),
		InstIconDef(_T("dirt"), wxMEMORY_IMAGE(dirt)),
		InstIconDef(_T("stone"), wxMEMORY_IMAGE(stone)),
		InstIconDef(_T("planks"), wxMEMORY_IMAGE(planks)),
		InstIconDef(_T("iron"), wxMEMORY_IMAGE(iron)),
		InstIconDef(_T("gold"), wxMEMORY_IMAGE(gold)),
		InstIconDef(_T("diamond"), wxMEMORY_IMAGE(diamond)),
		InstIconDef(_T("tnt"), wxMEMORY_IMAGE(tnt)),
		InstIconDef(_T("enderman"), wxMEMORY_IMAGE(enderman)),
		InstIconDef(_T("infinity"), wxMEMORY_IMAGE(infinity)),
	};

	imageList = new wxImageList(width, height);
	indexMap = new boost::unordered_map<wxString, int>();

	BOOST_FOREACH(InstIconDef builtInIcon, builtInIcons)
	{
		Add(builtInIcon.image, builtInIcon.key);
	}

	namespace fs = boost::filesystem;

	fs::path iconDir(customIconDirName.c_str());
	fs::directory_iterator endIter;
	
	if (fs::exists(iconDir) && fs::is_directory(iconDir))
	{
		for (fs::directory_iterator iter(iconDir); iter != endIter; iter++)
		{
			if (fs::is_regular_file(iter->status()))
			{
				wxImage *image = new wxImage(iter->path().c_str());

				wxString iconKey = iter->path().filename().replace_extension("").c_str();
				(*indexMap)[iconKey] = imageList->Add(*image);

				delete image;
			}
		}
	}
}

InstIconList::~InstIconList(void)
{
	indexMap->clear();
	delete indexMap;

	imageList->RemoveAll();
	delete imageList;
}

int InstIconList::Add(wxImage image, wxString &key)
{
	return (*indexMap)[key] = imageList->Add(image);
}

int InstIconList::operator [](wxString key)
{
	return (*indexMap)[key];
}

wxImageList *InstIconList::GetImageList()
{
	return imageList;
}