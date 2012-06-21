#pragma once

#include <wx/app.h>
#include <wx/icon.h>
#include <wx/filename.h>

class MultiMC : public wxApp
{
public:
	virtual bool OnInit();
	virtual void OnInitCmdLine(wxCmdLineParser &parser);
	virtual int OnExit();
	virtual void OnFatalException();
	
	const wxIcon &GetAppIcon() const;
	
	bool updateOnExit;
protected:
	wxIcon AppIcon;
	
	bool startNormally;
	
	void InstallUpdate(wxFileName thisFile, wxFileName targetFile);
};

DECLARE_APP(MultiMC)
