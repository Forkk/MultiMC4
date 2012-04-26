#include "MainWindow.h"
#include "ToolbarIcons.h"

#include <boost/foreach.hpp>

IMPLEMENT_APP(MultiMC)

// Main window
MainWindow::MainWindow(void)
	: wxFrame(NULL, -1, _T("MultiMC"), wxPoint(0, 0), wxSize(620, 400))
{
	wxToolBar *mainToolBar = CreateToolBar();

	// Load toolbar icons
	wxInitAllImageHandlers();
	wxBitmap newInstIcon = wxMEMORY_IMAGE(newinsticon);
	wxBitmap reloadIcon = wxMEMORY_IMAGE(refreshinsticon);
	wxBitmap viewFolderIcon = wxMEMORY_IMAGE(viewfoldericon);
	wxBitmap settingsIcon = wxMEMORY_IMAGE(settingsicon);
	wxBitmap checkUpdateIcon = wxMEMORY_IMAGE(checkupdateicon);
	wxBitmap helpIcon = wxMEMORY_IMAGE(helpicon);
	wxBitmap aboutIcon = wxMEMORY_IMAGE(abouticon);

	// Build the toolbar
	mainToolBar->AddTool(ID_AddInst, _("Add instance"), newInstIcon);
	mainToolBar->AddTool(ID_Refresh, _("Refresh"), reloadIcon);
	mainToolBar->AddTool(ID_ViewFolder, _("View folder"), viewFolderIcon);
	mainToolBar->AddSeparator();
	mainToolBar->AddTool(ID_Settings, _("Settings"), settingsIcon);
	mainToolBar->AddTool(ID_CheckUpdate, _("Check for updates"), checkUpdateIcon);
	mainToolBar->AddSeparator();
	mainToolBar->AddTool(ID_Help, _("Help"), helpIcon);
	mainToolBar->AddTool(ID_About, _("About"), aboutIcon);

	mainToolBar->Realize();

	// Create the status bar
	CreateStatusBar(1);

	// Set up the main panel and sizers
	wxPanel *panel = new wxPanel(this, -1);
	wxBoxSizer *box = new wxBoxSizer(wxVERTICAL);
	panel->SetSizer(box);

	// Create the instance list
	instListCtrl = new wxListCtrl(panel, -1);
	box->Add(instListCtrl, 1, wxEXPAND);

	// Load instance icons
	LoadInstIconList();

	// Load instance list
	LoadInstanceList(boost::filesystem::path(_T("instances")));

	CenterOnScreen();
}

MainWindow::~MainWindow(void)
{
	delete instIcons;
}

void MainWindow::LoadInstIconList(wxString customIconDirName)
{
	instIcons = new InstIconList(32, 32);

	instListCtrl->SetImageList(instIcons->GetImageList(), 0);
}

void MainWindow::LoadInstanceList(boost::filesystem::path instDir)
{
	namespace fs = boost::filesystem;

	instListCtrl->ClearAll();

	fs::directory_iterator endIter;

	if (fs::exists(instDir) && fs::is_directory(instDir))
	{
		for (fs::directory_iterator iter; iter != endIter; iter++)
		{
			fs::path file = iter->path();

			if (IsValidInstance(file))
			{
				Instance *inst = new Instance(file);
			}
		}
	}
}


// App
bool MultiMC::OnInit()
{
	MainWindow *mainWin = new MainWindow();
	mainWin->Show();

	return true;
}