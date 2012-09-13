// 
//  Copyright 2012 Andrew Okin, Petr Mrázek
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

#include "taskprogressdialog.h"
const wxString initial_text = _("This text represents the size of the dialog and the area\nreserved for any possible status text.");

TaskProgressDialog::TaskProgressDialog ( wxWindow* parent)
	: wxDialog ( parent, -1, _("Please wait..."), wxDefaultPosition, wxDefaultSize )
{
	auto wrapsizer = new wxBoxSizer(wxVERTICAL);
	auto panel = new wxPanel(this);
	wrapsizer->Add(panel,wxSizerFlags().Border(wxALL,10).Expand());
	
	auto sizer = new wxBoxSizer(wxVERTICAL);
	panel->SetSizer(sizer);
	
	pulse_timer = new wxTimer(this,ID_PulseTimer);
	
	message = new wxStaticText(panel, -1, initial_text ,wxDefaultPosition, wxDefaultSize,wxALIGN_CENTRE|wxST_NO_AUTORESIZE);
	sizer->Add(message,wxSizerFlags().Expand());
	gauge = new wxGauge(panel,-1,100);
	sizer->Add(gauge,wxSizerFlags().Expand());
	SetSizerAndFit(wrapsizer);
	CenterOnParent();
}

int TaskProgressDialog::ShowModal ( Task* run_task )
{
	task = run_task;
	task->Start(this,true);
	return wxDialog::ShowModal();
}

void TaskProgressDialog::OnTaskStart ( TaskEvent& event )
{
	StartPulsing();
}
void TaskProgressDialog::OnTaskProgress ( TaskProgressEvent& event )
{
	if (event.m_progress == 0)
	{
		StartPulsing();
	}
	else
	{
		is_pulsing = false;
		gauge->SetValue(event.m_progress);
	}
	message->SetLabel(event.m_status);
}
void TaskProgressDialog::OnTaskEnd ( TaskEvent& event )
{
	Task * t = event.m_task;
	long exitcode = (long) t->Wait();
	// running timer would cause a segfault.
	is_pulsing = false;
	pulse_timer->Stop();
	EndModal(exitcode);
}
void TaskProgressDialog::OnTaskError ( TaskErrorEvent& event )
{
	wxLogError(event.m_errorMsg);
}

void TaskProgressDialog::StartPulsing()
{
	if(!is_pulsing)
	{
		is_pulsing = true;
		gauge->Pulse();
		pulse_timer->Start(30);
	}
}


void TaskProgressDialog::OnTimer ( wxTimerEvent& event )
{
	if(is_pulsing)
	{
		gauge->Pulse();
	}
	else
	{
		pulse_timer->Stop();
	}
}


BEGIN_EVENT_TABLE(TaskProgressDialog, wxDialog)
	EVT_TASK_START(TaskProgressDialog::OnTaskStart)
	EVT_TASK_END(TaskProgressDialog::OnTaskEnd)
	EVT_TASK_PROGRESS(TaskProgressDialog::OnTaskProgress)
	EVT_TASK_ERRORMSG(TaskProgressDialog::OnTaskError)
	EVT_TIMER(ID_PulseTimer, TaskProgressDialog::OnTimer)
END_EVENT_TABLE()