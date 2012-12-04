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

#include "insticonlist.h"

#include "resources/insticons.h"

#include "utils/apputils.h"

#include <wx/filename.h>
#include <wx/dir.h>
#include <boost/concept_check.hpp>
#include <iostream>

#include "appsettings.h"
// #define DEBUG_ICONS

InstIconList* InstIconList::pInstance = 0;

const int allowedImgExtensionsCount = 5;
const wxString allowedImgExtensions[] = 
{
	".bmp",
	".gif",
	".jpg", ".jpeg",
	".png",
};

struct InstIconDef
{
	InstIconDef(wxString key, wxString name, wxImage image)
	{
		this->name = name;
		this->key = key;
		this->image = image;
	}
	
	wxString name;
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
	if (customIconDirName == wxEmptyString)
		customIconDirName = settings->GetIconsDir().GetFullPath();

	this->width = width;
	this->height = height;
	
	const InstIconDef builtInIcons[] =
	{
		InstIconDef("default", _("Default icon"), wxMEMORY_IMAGE(grass)),
		InstIconDef("grass", _("Grass block"), wxMEMORY_IMAGE(grass)),
		InstIconDef("dirt", _("Dirt block"), wxMEMORY_IMAGE(dirt)),
		InstIconDef("stone", _("Stone block"), wxMEMORY_IMAGE(stone)),
		InstIconDef("planks", _("Planks"), wxMEMORY_IMAGE(planks)),
		InstIconDef("iron", _("Solid iron"), wxMEMORY_IMAGE(iron)),
		InstIconDef("gold", _("Shiny gold"), wxMEMORY_IMAGE(gold)),
		InstIconDef("diamond", _("Diamond!"), wxMEMORY_IMAGE(diamond)),
		InstIconDef("tnt", _("TNT"), wxMEMORY_IMAGE(tnt)),
		InstIconDef("enderman",_("Enderman"), wxMEMORY_IMAGE(enderman)),
		InstIconDef("ftb-logo",_("FTB logo"), wxMEMORY_IMAGE(ftb_logo)),
		InstIconDef("ftb-glow",_("FTB glow"), wxMEMORY_IMAGE(ftb_glow)),
		InstIconDef("infinity",_("Infinity"), wxMEMORY_IMAGE(infinity)),
		InstIconDef("creeper",_("Creeper"), wxMEMORY_IMAGE(creeper)),
		InstIconDef("square creeper",_("Square creeper"), wxMEMORY_IMAGE(squarecreeper)),
		InstIconDef("skeleton",_("Skeleton"), wxMEMORY_IMAGE(skeleton)),
		InstIconDef("gear",_("Golden gear"), wxMEMORY_IMAGE(gear)),
		InstIconDef("magitech",_("Magitech"), wxMEMORY_IMAGE(magitech)),
		InstIconDef("enderpearl",_("Enderpearl"), wxMEMORY_IMAGE(enderpearl)),
		InstIconDef("herobrine",_("Herobrine"), wxMEMORY_IMAGE(herobrine)),
		InstIconDef("meat",_("The Meat"), wxMEMORY_IMAGE(meat)),
		InstIconDef("chicken",_("Chicken"), wxMEMORY_IMAGE(chicken)),
		InstIconDef("steve",_("Steve"), wxMEMORY_IMAGE(steve)),
		InstIconDef("derp",_("Derp"), wxMEMORY_IMAGE(derp)),
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
		Add(builtInIcons[i].image, highlightIcon, builtInIcons[i].key, builtInIcons[i].name);
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
				// Check file extensions
				bool isAllowedImage = false;
				for (int i = 0; i < allowedImgExtensionsCount; i++)
				{
					if (iconFile.Lower().EndsWith(allowedImgExtensions[i]))
					{
						isAllowedImage = true;
						break;
					}
				}
				if (!isAllowedImage)
					continue;

				if (!AddFile(Path::Combine(customIconDirName, iconFile)))
				{
					if(customIconDir.GetNext(&iconFile))
						continue;
					break;
				}
			} while (customIconDir.GetNext(&iconFile));
		}
	}
}

bool InstIconList::Add(const wxImage image, const wxImage hlimage, const wxString key, const wxString name,
	const wxString filename)
{
	InstIcon *defIcon = nullptr;
	if (iconMap.count(key) > 0)
	{
		defIcon = new InstIcon(iconMap[key]);
	}

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

		InstIcon newInstIcon(key, name, newImg, newHLImg, filename, defIcon);
		newInstIcon.deleteDefIconOnDestroy = false;
		iconMap[key] = newInstIcon;
	}
	else
	{
		InstIcon newInstIcon(key, name, image, hlimage, filename, defIcon);
		newInstIcon.deleteDefIconOnDestroy = false;
		iconMap[key] = newInstIcon;
	}
	return true;
}

bool InstIconList::AddFile(const wxString fileName)
{
	wxFileName iconFileName(fileName);
	wxImage image(iconFileName.GetFullPath());
	if(!image.IsOk())
	{
		return false;
	}
	wxString iconKey = iconFileName.GetName();
	wxImage highlightIcon = tintImage(image,wxSystemSettings::GetColour(wxSYS_COLOUR_HIGHLIGHT));
	return Add(image,highlightIcon, iconKey, iconKey, iconFileName.GetFullPath());
}

bool InstIconList::RemoveIcon(const wxString key)
{
	if (iconMap.count(key) <= 0)
		return false;

	InstIcon *defIcon = iconMap[key].m_defIcon;

	iconMap[key].deleteDefIconOnDestroy = false;
	iconMap.erase(iconMap.find(key));

	if (defIcon != nullptr)
	{
		iconMap[key] = InstIcon(*defIcon);
	}
	return true;
}

wxImageList *InstIconList::CreateImageList() const
{
	wxImageList *newImgList = new wxImageList(width, height);
	
	for (InstIconMap::const_iterator iter = iconMap.begin(); iter != iconMap.end(); iter++)
	{
		newImgList->Add(iter->second.m_image);
	}
	return newImgList;
}

int InstIconList::GetCount() const
{
	return iconMap.size();
}

wxImage& InstIconList::getImageForKey(wxString key)
{
	if(!iconMap.count(key))
	{
		return iconMap["default"].m_image;
	}
	return iconMap[key].m_image;
}

wxImage& InstIconList::getHLImageForKey(wxString key)
{
	if(!iconMap.count(key))
	{
		return iconMap["default"].m_hlImage;
	}
	return iconMap[key].m_hlImage;
}

wxString& InstIconList::getFileNameForKey(wxString key)
{
	if(!iconMap.count(key))
	{
		return iconMap["default"].m_fileName;
	}
	return iconMap[key].m_fileName;
}

const InstIconMap &InstIconList::GetIconMap() const
{
	return iconMap;
}

InstIcon::InstIcon(wxString key, wxString name, wxImage image, wxImage hlImage, 
	wxString fileName, InstIcon *defIcon)
	: m_key(key), m_name(name), m_image(image), m_hlImage(hlImage), m_fileName(fileName)
{
	m_defIcon = defIcon;
	deleteDefIconOnDestroy = true;
}

InstIcon::InstIcon(const InstIcon &iIcon)
	: m_key(iIcon.m_key), m_image(iIcon.m_image), m_hlImage(iIcon.m_hlImage), m_fileName(iIcon.m_fileName),
	  m_name(iIcon.m_name)
{
	m_defIcon = iIcon.m_defIcon;
	deleteDefIconOnDestroy = true;
}

InstIcon::InstIcon()
{
	m_defIcon = nullptr;
	deleteDefIconOnDestroy = true;
}

InstIcon::~InstIcon()
{
	if (m_defIcon != nullptr && deleteDefIconOnDestroy)
		delete m_defIcon;
}
