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

#include "lwjgldialog.h"
#include "taskprogressdialog.h"
#include "lwjglversionlist.h"
#include <lambdatask.h>

ChooseLWJGLDialog::ChooseLWJGLDialog(wxWindow *parent)
	: ListSelectDialog(parent, _("Choose LWJGL Version"))
{

}

void ChooseLWJGLDialog::LoadList()
{
	LWJGLVersionList & verList = LWJGLVersionList::Instance();

	bool success = false;
	
	if(verList.NeedsLoad())
	{
		LambdaTask::TaskFunc func = [&] (LambdaTask *task) -> wxThread::ExitCode
		{
			task->DoSetStatus(_("Loading LWJGL version list..."));
			return (wxThread::ExitCode)DoLoadList();
		};

		LambdaTask *lTask = new LambdaTask(func);
		TaskProgressDialog taskDlg(this);
		success = taskDlg.ShowModal(lTask) > 0;
		delete lTask;
	}
	else
	{
		success = true;
	}
	sList.Clear();
	linkList.Clear();
	for(auto iter = verList.versions.begin(); iter != verList.versions.end(); iter++)
	{
		LWJGLVersion & ver = *iter;
		sList.push_back(ver.GetName());
		linkList.push_back(ver.GetDLUrl());
	}
	listCtrl->SetItemCount(sList.size());
	listCtrl->Refresh();
	listCtrl->Update();
	UpdateOKBtn();
}

bool ChooseLWJGLDialog::DoLoadList()
{
	LWJGLVersionList & verList = LWJGLVersionList::Instance();
	return verList.LoadIfNeeded();
}

wxString ChooseLWJGLDialog::GetSelectedURL()
{
	return linkList[GetSelectedIndex()];
}

wxString ChooseLWJGLDialog::GetSelectedName()
{
	return sList[GetSelectedIndex()];
}