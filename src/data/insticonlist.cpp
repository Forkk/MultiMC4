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

wxString InstIconList::getRealIconKeyForEasterEgg ( wxString key, wxString name )
{
	if (key == "default")
	{
		if (name.Lower().Contains("btw") ||
			name.Lower().Contains("better then wolves") || // Because some people are stupid :D
			name.Lower().Contains("better than wolves"))
		{
			return "herobrine";
		}
		else if (name.Lower().Contains("direwolf"))
		{
			return "enderman";
		}
	}
	return key;
}



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
	InstIconDef(wxString key, wxString name, wxImage image, wxImage image128)
	{
		this->name = name;
		this->key = key;
		this->image = image;
		this->image128 = image128;
	}
	InstIconDef(wxString key, wxString name, wxImage image)
	{
		this->name = name;
		this->key = key;
		this->image = image;
		this->image128 = image;
	}
	
	wxString name;
	wxString key;
	wxImage image;
	wxImage image128;
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
		InstIconDef("ftb-logo",_("FTB logo"), wxMEMORY_IMAGE(ftb_logo), wxMEMORY_IMAGE(ftb_logo128)),
		InstIconDef("ftb-glow",_("FTB glow"), wxMEMORY_IMAGE(ftb_glow), wxMEMORY_IMAGE(ftb_glow128)),
		InstIconDef("infinity",_("Infinity"), wxMEMORY_IMAGE(infinity), wxMEMORY_IMAGE(infinity128)),
		InstIconDef("netherstar",_("Nether Star"), wxMEMORY_IMAGE(netherstar), wxMEMORY_IMAGE(netherstar128)),
		InstIconDef("creeper",_("Creeper"), wxMEMORY_IMAGE(creeper), wxMEMORY_IMAGE(creeper128)),
		InstIconDef("square creeper",_("Square creeper"), wxMEMORY_IMAGE(squarecreeper), wxMEMORY_IMAGE(squarecreeper128)),
		InstIconDef("skeleton",_("Skeleton"), wxMEMORY_IMAGE(skeleton), wxMEMORY_IMAGE(skeleton128)),
		InstIconDef("gear",_("Golden gear"), wxMEMORY_IMAGE(gear), wxMEMORY_IMAGE(gear128)),
		InstIconDef("magitech",_("Magitech"), wxMEMORY_IMAGE(magitech), wxMEMORY_IMAGE(magitech128)),
		InstIconDef("enderpearl",_("Enderpearl"), wxMEMORY_IMAGE(enderpearl), wxMEMORY_IMAGE(enderpearl128)),
		InstIconDef("herobrine",_("Herobrine"), wxMEMORY_IMAGE(herobrine), wxMEMORY_IMAGE(herobrine128)),
		InstIconDef("meat",_("The Meat"), wxMEMORY_IMAGE(meat), wxMEMORY_IMAGE(meat128)),
		InstIconDef("chicken",_("Chicken"), wxMEMORY_IMAGE(chicken), wxMEMORY_IMAGE(chicken128)),
		InstIconDef("steve",_("Steve"), wxMEMORY_IMAGE(steve), wxMEMORY_IMAGE(steve128)),
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
		Add(builtInIcons[i].image, highlightIcon, builtInIcons[i].image128, builtInIcons[i].key, builtInIcons[i].name);
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

bool InstIconList::Add(const wxImage image, const wxImage hlimage, const wxImage image128, const wxString key, const wxString name,
	const wxString filename)
{
	InstIcon *defIcon = nullptr;
	if (iconMap.count(key) > 0)
	{
		defIcon = new InstIcon(iconMap[key]);
	}
	wxImage newbig = image128;
	if(image128.GetWidth() != 128 || image128.GetHeight() != 128)
	{
		newbig.Rescale(128,128);
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

		InstIcon newInstIcon(key, name, newImg, newHLImg, newbig, filename, defIcon);
		newInstIcon.deleteDefIconOnDestroy = false;
		iconMap[key] = newInstIcon;
	}
	else
	{
		InstIcon newInstIcon(key, name, image, hlimage, newbig, filename, defIcon);
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
	return Add(image,highlightIcon, image, iconKey, iconKey, iconFileName.GetFullPath());
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

wxImage& InstIconList::getImage128ForKey ( wxString key )
{
	if(!iconMap.count(key))
	{
		return iconMap["default"].m_image128;
	}
	return iconMap[key].m_image128;
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

InstIcon::InstIcon(wxString key, wxString name, wxImage image, wxImage hlImage, wxImage image128,
	wxString fileName, InstIcon *defIcon)
	: m_key(key), m_name(name), m_image(image), m_hlImage(hlImage), m_image128(image128), m_fileName(fileName)
{
	m_defIcon = defIcon;
	deleteDefIconOnDestroy = true;
}

InstIcon::InstIcon(const InstIcon &iIcon)
	: m_key(iIcon.m_key), m_image(iIcon.m_image), m_hlImage(iIcon.m_hlImage),
	  m_image128(iIcon.m_image128), m_fileName(iIcon.m_fileName),
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
