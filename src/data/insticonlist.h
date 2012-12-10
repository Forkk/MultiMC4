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
#include <map>
#include <vector>
#include <wx/imaglist.h>

struct InstIcon
{
	InstIcon(wxString key, wxString name, wxImage image, wxImage hlImage, wxImage image128,
		wxString fileName = wxEmptyString, InstIcon *defIcon = nullptr);
	InstIcon(const InstIcon &iIcon);
	InstIcon();
	~InstIcon();

	wxString m_key;
	wxString m_fileName;
	wxString m_name;
	wxImage m_image;
	wxImage m_image128;
	wxImage m_hlImage;

	InstIcon *m_defIcon;

	bool deleteDefIconOnDestroy;
};

typedef std::map<wxString, InstIcon> InstIconMap;

class InstIconList
{
public:
	static wxString getRealIconKeyForEasterEgg( wxString key, wxString name );
	
	bool Add(const wxImage image, const wxImage hlimage, wxImage image128, const wxString key, const wxString name,
		const wxString filename = wxEmptyString);
	bool AddFile(const wxString fileName);
	bool RemoveIcon(const wxString key);

	wxImage &getImageForKey(wxString key);
	wxImage &getImage128ForKey(wxString key);
	wxImage &getHLImageForKey(wxString key);
	wxString &getFileNameForKey(wxString key);
	static InstIconList* Instance()
	{
		if (pInstance == 0)
			pInstance = new InstIconList();
		return pInstance;
	};
	//FIXME: actually call this on app shutdown
	static void Dispose()
	{
		delete pInstance;
		pInstance = 0;
	};
	wxImageList *CreateImageList() const;
	
	const InstIconMap &GetIconMap() const;

	int GetCount() const;
	
protected:
	InstIconList(int width = 32, int height = 32, wxString customIconDir = wxEmptyString);
	
	InstIconMap iconMap;
	int width, height;
private:
	static InstIconList* pInstance;
};

