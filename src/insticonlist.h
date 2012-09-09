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
#include <map>
#include <vector>
#include <wx/imaglist.h>

typedef std::map<wxString, int> IconListIndexMap;

class InstIconList
{
public:
	int Add(const wxImage image, const wxImage hlimage, const wxString key,
		const wxString filename = wxEmptyString);
	int AddFile(const wxString fileName);
	bool RemoveIcon(wxString key);

	int getIndexForKey(wxString key);
	wxImage &getImageForKey(wxString key);
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
	
	const IconListIndexMap &GetIndexMap() const;
	
	int GetCount() const;
	
protected:
	InstIconList(int width = 32, int height = 32, wxString customIconDir = wxEmptyString);
	
	IconListIndexMap indexMap;
	std::vector <wxImage> imageList;
	std::vector <wxImage> hlimageList;
	std::vector<wxString> fileNameList;
	int width, height;
private:
	static InstIconList* pInstance;
};

