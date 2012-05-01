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
#include "includes.h"

const wxString instIconPrefix = _T("icons/inst/");

class InstIconList
{
public:
	InstIconList(int width = 32, int height = 32, 
		wxString customIconDir = _T("icons"));
	~InstIconList(void);

	int Add(wxImage image, wxString &key);

	int operator[](wxString key);

	wxImageList *GetImageList();

protected:
	wxImageList *imageList;
	boost::unordered_map<wxString, int> *indexMap;
};

