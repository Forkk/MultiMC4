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

#include "downgradedialog.h"

#include <wx/gbsizer.h>
#include <wx/hyperlink.h>
#include <wx/wfstream.h>

#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>

#include <string>
#include <sstream>

#include "apputils.h"
#include "httputils.h"

#include "bspatch.h"

#include "md5/md5.h"

#include "filedownloadtask.h"

const wxString mcnwebURL = _("http://sonicrules.org/mcnweb.py");

DowngradeWizard::DowngradeWizard(wxWindow *parent, Instance *inst)
	: wxWizard(parent, wxID_ANY, _("Downgrade Instance"))
{
	m_inst = inst;

	this->SetPageSize(wxSize(400, 300));

	wxFont titleFont(12, wxSWISS, wxNORMAL, wxNORMAL);

	chooseVersionPage = new wxWizardPageSimple(this);
	wxGridBagSizer *versionPageSz = new wxGridBagSizer();
	versionPageSz->AddGrowableCol(0, 0);
	versionPageSz->AddGrowableRow(1, 0);
	chooseVersionPage->SetSizer(versionPageSz);

	wxStaticText *versionPageTitle = new wxStaticText(chooseVersionPage, -1, _("Choose Version"));
	versionPageTitle->SetFont(titleFont);
	versionPageSz->Add(versionPageTitle, wxGBPosition(0, 0), wxGBSpan(1, 2), wxALL | wxALIGN_CENTER, 4);

	versionList = new wxListBox(chooseVersionPage, -1, wxDefaultPosition, wxDefaultSize,
		wxArrayString(), wxLB_SINGLE);
	versionPageSz->Add(versionList, wxGBPosition(1, 0), wxGBSpan(1, 2), wxEXPAND | wxALL, 4);

	wxHyperlinkCtrl *mcnLink = new wxHyperlinkCtrl(chooseVersionPage, -1, _("Powered by MCNostalgia"),
		_("http://www.minecraftforum.net/topic/800346-"));
	versionPageSz->Add(mcnLink, wxGBPosition(2, 0), wxGBSpan(1, 1), wxALL | wxALIGN_CENTER_VERTICAL, 4);

	wxButton *versionRefreshBtn = new wxButton(chooseVersionPage, ID_RefreshVersionList, _("&Refresh"));
	versionPageSz->Add(versionRefreshBtn, wxGBPosition(2, 1), wxGBSpan(1, 1), wxALL, 4);


	
	downloadPatchPage = new wxWizardPageSimple(this, chooseVersionPage);
	chooseVersionPage->SetNext(downloadPatchPage);
	wxGridBagSizer *patchPageSz = new wxGridBagSizer();
	patchPageSz->AddGrowableCol(0, 0);
	patchPageSz->AddGrowableRow(1, 0);
	downloadPatchPage->SetSizer(patchPageSz);

	wxStaticText *titleLabel = new wxStaticText(downloadPatchPage, -1, _("Patching Minecraft"));
	titleLabel->SetFont(titleFont);
	patchPageSz->Add(titleLabel, wxGBPosition(0, 0), wxGBSpan(1, 1), wxEXPAND | wxALL, 4);

	wxBoxSizer *pbarSz = new wxBoxSizer(wxVERTICAL);
	patchPageSz->Add(pbarSz, wxGBPosition(1, 0), wxGBSpan(1, 1), wxALIGN_CENTER | wxEXPAND | wxALL, 4);

	installStatusLbl = new wxStaticText(downloadPatchPage, -1, _("Status: "));
	pbarSz->Add(installStatusLbl, wxSizerFlags(0).Align(wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL).Border(wxALL, 4));

	installPBar = new wxGauge(downloadPatchPage, -1, 100);
	pbarSz->Add(installPBar, wxSizerFlags(0).Expand().Align(wxALIGN_CENTER).Border(wxALL, 4));
}

inline void SetControlEnable(int id, bool state)
{
	wxWindow *win = wxWindow::FindWindowById(id);
	if (win) win->Enable(state);
}

bool DowngradeWizard::Start()
{
	SetControlEnable(wxID_FORWARD, versionList->GetSelection() != wxNOT_FOUND);
	LoadVersionList();
	return RunWizard(chooseVersionPage);
}

void DowngradeWizard::LoadVersionList()
{
	wxString vlistJSON;
	if (DownloadString(mcnwebURL + _("?pversion=1&list=True"), &vlistJSON))
	{
		using namespace boost::property_tree;

		try
		{
			wxArrayString vList;

			// Parse the JSON
			ptree pt;
			std::stringstream jsonStream(stdStr(vlistJSON), std::ios::in);
			read_json(jsonStream, pt);

			BOOST_FOREACH(const ptree::value_type& v, pt.get_child("order"))
				vList.Insert(wxStr(v.second.data()), 0);

			versionList->Set(vList);
		}
		catch (json_parser_error e)
		{
			wxLogError(_("Failed to read version list.\nJSON parser error at line %i: %s"), 
				e.line(), wxStr(e.message()).c_str());
			return;
		}
	}
	else
	{
		wxLogError(_("Failed to get version list. Check your internet connection and try again later."));
	}
}

void DowngradeWizard::OnRefreshVListClicked(wxCommandEvent& event)
{
	LoadVersionList();
}

void DowngradeWizard::OnListBoxSelChange(wxCommandEvent& event)
{
	SetControlEnable(wxID_FORWARD, versionList->GetSelection() != wxNOT_FOUND);
}

void DowngradeWizard::OnPageChanged(wxWizardEvent& event)
{
	if (event.GetPage() == downloadPatchPage)
	{
		wxString patchBaseURL;
		RetrievePatchURL(versionList->GetStringSelection(), &patchBaseURL);

		patchURLs.Add(patchBaseURL + _("/minecraft.ptch"));
		patchURLs.Add(patchBaseURL + _("/lwjgl.ptch"));
		patchURLs.Add(patchBaseURL + _("/lwjgl_util.ptch"));
		patchURLs.Add(patchBaseURL + _("/jinput.ptch"));
		patchURLs.Add(patchBaseURL + _("/checksum.json"));

		patchDir = _("patches");

		SetControlEnable(wxID_BACKWARD, false);
		SetControlEnable(wxID_FORWARD, false);
		currentStep = DownloadPatches;
		DownloadNextPatch();
	}
}

bool DowngradeWizard::RetrievePatchURL(const wxString& mcVersion, wxString *patchURL)
{
	wxString json;
	if (DownloadString(wxString::Format(_("%s?pversion=1&mcversion=%s"), 
		mcnwebURL.c_str(), mcVersion.c_str()), &json))
	{
		using namespace boost::property_tree;
		try
		{
			ptree pt;
			std::stringstream jsonStream(stdStr(json), std::ios::in);
			read_json(jsonStream, pt);

			*patchURL = wxStr(pt.get<std::string>("url"));
		}
		catch (json_parser_error e)
		{
			wxLogError(_("Failed to get patch URL.\nJSON parser error at line %i: %s"), 
				e.line(), wxStr(e.message()).c_str());
			return false;
		}
		return true;
	}
	else
	{
		return false;
	}
}

void DowngradeWizard::DownloadNextPatch()
{
	if (!wxDirExists(patchDir))
		wxMkdir(patchDir);

	wxString patchURL = patchURLs[0];
	patchURLs.RemoveAt(0);

	wxString destFileName = patchURL.Right(patchURL.Len() - patchURL.Last('/') - 1);
	FileDownloadTask *downloadTask = new FileDownloadTask(patchURL, Path::Combine(patchDir, destFileName), 
		wxString::Format(_("Downloading %s..."), destFileName.c_str()));
	downloadTask->SetEvtHandler(this);
	downloadTask->Start();
}

void DowngradeWizard::DoApplyPatches()
{
	wxArrayString patchFiles;
	patchFiles.Add(_("minecraft"));
	patchFiles.Add(_("lwjgl"));
	patchFiles.Add(_("lwjgl_util"));
	patchFiles.Add(_("jinput"));

	currentStep = CheckOriginalMD5s;
	installStatusLbl->SetLabel(_("Verifying files..."));

	using namespace boost::property_tree;
	try
	{
		ptree pt;
		read_json("patches/checksum.json", pt);

		for (int i = 0; i < patchFiles.Count(); i++)
		{
			wxString cFileName = patchFiles[i];
			if (cFileName == _("minecraft") && wxFileExists(Path::Combine(m_inst->GetBinDir(), _("mcbackup.jar"))))
				cFileName = _("mcbackup");
			wxString checkFile = Path::Combine(m_inst->GetBinDir(), cFileName + _(".jar"));

			// Verify the file's MD5 sum.
			MD5Context md5ctx;
			MD5Init(&md5ctx);

			char buf[1024];
			wxFFileInputStream fileIn(checkFile);

			while (!fileIn.Eof())
			{
				fileIn.Read(buf, 1024);
				MD5Update(&md5ctx, (unsigned char*)buf, fileIn.LastRead());
			}

			unsigned char md5digest[16];
			MD5Final(md5digest, &md5ctx);

			if (!Utils::BytesToString(md5digest).IsSameAs(
				wxStr(pt.get<std::string>("CurrentVersion." + stdStr(patchFiles[i]))), false))
			{
				installStatusLbl->SetLabel(wxString::Format(_("The file %s has been modified from its original state. \
Unable to patch. Try force updating."), checkFile.c_str()));
				return;
			}
		}
	}
	catch (json_parser_error e)
	{
		wxLogError(_("Failed to check file MD5.\nJSON parser error at line %i: %s"), 
			e.line(), wxStr(e.message()).c_str());
		return;
	}


	currentStep = ApplyPatches;

	installStatusLbl->SetLabel(_("Applying patches..."));

	for (int i = 0; i < patchFiles.Count(); i++)
	{
		wxYieldIfNeeded();

		wxString file = patchFiles[i];

		wxString binDir = m_inst->GetBinDir().GetFullPath();
		wxString patchFile = Path::Combine(patchDir, file + _(".ptch"));

		if (file == _("minecraft") && wxFileExists(m_inst->GetMCBackup().GetFullPath()))
			file = _("mcbackup");
		wxString patchSrc = Path::Combine(binDir, file + _(".jar"));
		wxString patchDest = Path::Combine(binDir, file + _("_new.jar"));

		if (wxFileExists(patchDest))
			wxRemoveFile(patchDest);

		int err = bspatch(patchSrc.fn_str(), patchDest.fn_str(), patchFile.fn_str());

		if (err == ERR_NONE)
		{
			wxRemoveFile(patchSrc);
			wxRename(patchDest, patchSrc);
		}
		else
		{
			switch (err)
			{
			case ERR_CORRUPT_PATCH:
				installStatusLbl->SetLabel(wxString::Format(_("Failed to patch %s.jar. Patch is corrupt."), file.c_str()));
				break;

			default:
				installStatusLbl->SetLabel(wxString::Format(_("Failed to patch %s.jar. Unknown error."), file.c_str()));
				break;
			}
			return;
		}
	}


	currentStep = CheckFinalMD5s;
	installStatusLbl->SetLabel(_("Verifying files..."));

	try
	{
		ptree pt;
		read_json("patches/checksum.json", pt);

		for (int i = 0; i < patchFiles.Count(); i++)
		{
			wxString cFileName = patchFiles[i];
			if (cFileName == _("minecraft") && wxFileExists(Path::Combine(m_inst->GetBinDir(), _("mcbackup.jar"))))
				cFileName = _("mcbackup");
			wxString checkFile = Path::Combine(m_inst->GetBinDir(), cFileName + _(".jar"));

			// Verify the file's MD5 sum.
			MD5Context md5ctx;
			MD5Init(&md5ctx);

			char buf[1024];
			wxFFileInputStream fileIn(checkFile);

			while (!fileIn.Eof())
			{
				fileIn.Read(buf, 1024);
				MD5Update(&md5ctx, (unsigned char*)buf, fileIn.LastRead());
			}

			unsigned char md5digest[16];
			MD5Final(md5digest, &md5ctx);

			if (!Utils::BytesToString(md5digest).IsSameAs(
				wxStr(pt.get<std::string>("OldVersion." + stdStr(patchFiles[i]))), false))
			{
				installStatusLbl->SetLabel(
					wxString::Format(_("The file %s's MD5 didn't match what it was supposed to be."), 
					wxFileName(checkFile).GetFullName().c_str()));
				return;
			}
		}
	}
	catch (json_parser_error e)
	{
		wxLogError(_("Failed to check file MD5.\nJSON parser error at line %i: %s"), 
			e.line(), wxStr(e.message()).c_str());
		return;
	}

	installStatusLbl->SetLabel(_("Downgrade complete!"));
}

void DowngradeWizard::OnTaskStart(TaskEvent& event)
{
	installPBar->SetValue(event.m_task->GetProgress());
	installStatusLbl->SetLabel(event.m_task->GetStatus());
}

void DowngradeWizard::OnTaskEnd(TaskEvent& event)
{
	wxDELETE(event.m_task);

	if (!patchURLs.IsEmpty())
	{
		DownloadNextPatch();
	}
	else
	{
		DoApplyPatches();
		SetControlEnable(wxID_BACKWARD, true);
		SetControlEnable(wxID_FORWARD, true);
	}
}

void DowngradeWizard::OnTaskProgress(TaskProgressEvent& event)
{
	installPBar->SetValue(event.m_progress);
}

void DowngradeWizard::OnTaskStatus(TaskStatusEvent& event)
{
	switch (currentStep)
	{
	case DownloadPatches:
		installStatusLbl->SetLabel(_("Downloading patches: ") + event.m_status);
		break;

	case CheckOriginalMD5s:
	case CheckFinalMD5s:
		installStatusLbl->SetLabel(_("Verifying files: ") + event.m_status);
		break;

	case ApplyPatches:
		installStatusLbl->SetLabel(_("Applying patches: ") + event.m_status);
		break;

	default:
		installStatusLbl->SetLabel(_("Status: ") + event.m_status);
		break;
	}
}

BEGIN_EVENT_TABLE(DowngradeWizard, wxWizard)
	EVT_BUTTON(ID_RefreshVersionList, DowngradeWizard::OnRefreshVListClicked)

	EVT_LISTBOX(-1, DowngradeWizard::OnListBoxSelChange)

	EVT_WIZARD_PAGE_CHANGED(-1, DowngradeWizard::OnPageChanged)

	EVT_TASK_START(DowngradeWizard::OnTaskStart)
	EVT_TASK_END(DowngradeWizard::OnTaskEnd)
	EVT_TASK_PROGRESS(DowngradeWizard::OnTaskProgress)
	EVT_TASK_STATUS(DowngradeWizard::OnTaskStatus)
END_EVENT_TABLE()
