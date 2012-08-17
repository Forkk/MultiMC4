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

#include "insticonlist.h"

#include "resources/insticons.h"

#include "apputils.h"

#include <wx/filename.h>
#include <wx/dir.h>

InstIconList* InstIconList::pInstance = 0;

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
	: imageList(width, height)
{
	this->width = width;
	this->height = height;
	
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
		InstIconDef(_T("creeper"), wxMEMORY_IMAGE(creeper)),
		InstIconDef(_T("skeleton"), wxMEMORY_IMAGE(skeleton)),
		InstIconDef(_T("enderpearl"), wxMEMORY_IMAGE(enderpearl)),
	};
	const int builtInIconCount = sizeof(builtInIcons)/sizeof(InstIconDef);

	for (int i = 0; i < builtInIconCount; i++)
	{
		Add(builtInIcons[i].image, builtInIcons[i].key);
	}

	if (wxDirExists(customIconDirName))
	{
		wxDir customIconDir(customIconDirName);

		if (!customIconDir.IsOpened())
			return;

		wxString iconFile;
		
		if (customIconDir.GetFirst(&iconFile))
		{
			do 
			{
				wxFileName iconFileName(Path::Combine(customIconDirName, iconFile));
				wxImage image(iconFileName.GetFullPath());

				wxString iconKey = iconFileName.GetName();
				Add(image, iconKey);
			} while (customIconDir.GetNext(&iconFile));
		}
	}
}

int InstIconList::Add(const wxImage image, const wxString key)
{
	if (image.GetWidth() != 32 || image.GetHeight() != 32)
	{
		wxImage newImg(image);
		newImg.Rescale(32, 32);
		return indexMap[key] = imageList.Add(newImg);
	}
	else
	{
		return indexMap[key] = imageList.Add(image);
	}
}

int InstIconList::operator [](wxString key)
{
	return indexMap[key];
}

const wxImageList &InstIconList::GetImageList()
{
	return imageList;
}

const IconListIndexMap &InstIconList::GetIndexMap() const
{
	return indexMap;
}

wxImageList *InstIconList::CreateImageList() const
{
	wxImageList *newImgList = new wxImageList(width, height);
	
	for (int i = 0; i < imageList.GetImageCount(); i++)
	{
		newImgList->Add(imageList.GetBitmap(i));
	}
	return newImgList;
}

int InstIconList::GetCount() const
{
	return indexMap.size();
}
