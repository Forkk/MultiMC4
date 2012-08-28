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
#include <boost/concept_check.hpp>
#include <iostream>
// #define DEBUG_ICONS

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

wxImage tintImage( wxImage to_colorize, wxColour col)
{
	wxImage highlightIcon(to_colorize.GetWidth(),to_colorize.GetHeight());
	bool do_alpha = false;
	if(to_colorize.HasAlpha())
	{
		highlightIcon.InitAlpha();
		do_alpha = true;
	}
	else if(to_colorize.HasMask())
	{
		highlightIcon.SetMaskFromImage(to_colorize,to_colorize.GetMaskRed(),to_colorize.GetMaskGreen(),to_colorize.GetMaskBlue());
	}
	for(int x = 0; x < highlightIcon.GetWidth(); x++)
	{
		for(int y = 0; y < highlightIcon.GetHeight(); y++)
		{
			to_colorize.GetData();
			unsigned char srcR = to_colorize.GetRed(x,y);
			unsigned char srcG = to_colorize.GetGreen(x,y);
			unsigned char srcB = to_colorize.GetBlue(x,y);
			highlightIcon.SetRGB(x,y,(srcR + col.Red())/2,(srcG + col.Green())/2, (srcB + col.Blue())/2);
			if(do_alpha)
				highlightIcon.SetAlpha(x,y,to_colorize.GetAlpha(x,y));
		}
	}
	return highlightIcon;
}

InstIconList::InstIconList(int width, int height, wxString customIconDirName)
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
#ifdef DEBUG_ICONS
		if(!builtInIcons[i].image.HasAlpha())
		{
			wxLogMessage(_("Image %d %s has no alpha."), i, builtInIcons[i].key.c_str()  );
		}
#endif
		wxImage highlightIcon = tintImage(builtInIcons[i].image,wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT));
		Add(builtInIcons[i].image, highlightIcon, builtInIcons[i].key);
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
				if(!image.IsOk())
				{
					if(customIconDir.GetNext(&iconFile))
						continue;
					break;
				}
				wxString iconKey = iconFileName.GetName();
				wxImage highlightIcon = tintImage(image,wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT));
				Add(image,highlightIcon, iconKey);
			} while (customIconDir.GetNext(&iconFile));
		}
	}
}

int InstIconList::Add(const wxImage image, const wxImage hlimage, const wxString key)
{
	if (image.GetWidth() != 32 || image.GetHeight() != 32)
	{
		wxImage newImg(image);
		wxImage newHLImg(hlimage);
		newImg.Rescale(32, 32);
		newHLImg.Rescale(32, 32);
#ifdef DEBUG_ICONS
		if(!newImg.HasAlpha())
		{
			wxLogMessage(_("Image %s has no alpha after rescaling."), key.c_str()  );
		}
#endif
		
		imageList.push_back(newImg);
		hlimageList.push_back(newHLImg);
	}
	else
	{
		imageList.push_back(image);
		hlimageList.push_back(hlimage);
	}
	return indexMap[key] = imageList.size() - 1;
}

const IconListIndexMap &InstIconList::GetIndexMap() const
{
	return indexMap;
}

wxImageList *InstIconList::CreateImageList() const
{
	wxImageList *newImgList = new wxImageList(width, height);
	
	for (int i = 0; i < imageList.size(); i++)
	{
		newImgList->Add(imageList[i]);
	}
	return newImgList;
}

int InstIconList::GetCount() const
{
	return indexMap.size();
}
