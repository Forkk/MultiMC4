#pragma once

#include <wx/app.h>
#include <wx/icon.h>
#include <wx/filename.h>
#include <wx/iconbndl.h>

class MultiMC : public wxApp
{
public:
	virtual bool OnInit();
	virtual void OnInitCmdLine(wxCmdLineParser &parser);
	virtual int OnExit();
	virtual void OnFatalException();
	virtual void OnUnhandledException();
	
	const wxIconBundle &GetAppIcons() const;
	
	bool updateOnExit;
protected:
	wxIconBundle AppIcons;
	
	bool startNormally;
	
	void InstallUpdate(wxFileName thisFile, wxFileName targetFile);
};

DECLARE_APP(MultiMC)
