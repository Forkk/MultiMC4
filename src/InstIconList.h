#pragma once
#include <wx/wx.h>

#include <boost/unordered_map.hpp>

const wxString instIconPrefix = _T("icons/inst/");

class InstIconList
{
public:
	InstIconList(int width, int height, wxString customIconDir = _T("icons"));
	~InstIconList(void);

	int Add(wxImage image, wxString &key);

	int operator[](wxString key);

	wxImageList *GetImageList();

protected:
	wxImageList *imageList;
	boost::unordered_map<wxString, int> *indexMap;
};

