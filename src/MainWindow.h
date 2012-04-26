#pragma once
#include <wx/wx.h>
#include <wx/listctrl.h>

#include <boost/filesystem.hpp>

#include "InstIconList.h"
#include "Instance.h"

enum
{
	ID_AddInst = 1,
	ID_ViewFolder,
	ID_ModsFolder,
	ID_Refresh,

	ID_Settings,
	ID_CheckUpdate,

	ID_Help,
	ID_About,
};

//const wxString tbarIconPrefix = _T("resources/toolbar/");

class MainWindow : public wxFrame
{
public:
	MainWindow(void);
	~MainWindow(void);

protected:
	wxListCtrl *instListCtrl;

	// An unordered map to map icon keys to their icon list indices.
	InstIconList *instIcons;
	void LoadInstIconList(wxString customIconDirName = _T("icons"));

	void LoadInstanceList(boost::filesystem::path instDir);

private:

};

class MultiMC : public wxApp
{
public:
	virtual bool OnInit();
};