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

#include "taskprogressdialog.h"
#include <wx/gbsizer.h>
const wxString initial_text = _("This text represents the size of the dialog and the area\nreserved for any possible status text.");

TaskProgressDialog::TaskProgressDialog ( wxWindow* parent)
	: wxDialog ( parent, -1, _("Please wait..."), wxDefaultPosition, wxDefaultSize , wxCAPTION)
{
	wxClientDC dc(this);
	dc.SetFont(wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT));
	long widthText = 0;
	long heightText = 0;
	long lineHeight = 0;
	dc.GetTextExtent(initial_text, &widthText, &heightText, NULL, NULL, NULL);
	dc.GetTextExtent("ABEND", NULL, &lineHeight, NULL, NULL, NULL);
	
	auto wrapsizer = new wxBoxSizer(wxVERTICAL);
	
	auto centerizer = new wxGridBagSizer( 0, 0 );
	centerizer->AddGrowableCol( 0 );
	centerizer->AddGrowableRow( 0 );
	centerizer->SetFlexibleDirection( wxBOTH );
	centerizer->SetNonFlexibleGrowMode( wxFLEX_GROWMODE_ALL );
	wrapsizer->Add( centerizer, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL);
	
	wxSize textSize(widthText, heightText);
	message = new wxGenericStaticText(this, -1, "" ,wxDefaultPosition, textSize,wxALIGN_CENTRE_HORIZONTAL);
	message->SetMinSize(textSize);
	centerizer->Add(message,wxGBPosition( 0, 0 ), wxGBSpan( 1, 1 ), wxALIGN_CENTER_VERTICAL|wxALL, lineHeight/2);
	
	gauge = new wxGauge(this,-1,100,wxDefaultPosition,wxDefaultSize,wxGA_HORIZONTAL | wxGA_SMOOTH);
	wrapsizer->Add(gauge,wxSizerFlags().Expand().Border(wxBOTTOM|wxLEFT|wxRIGHT,lineHeight/2).Proportion(0));
	
	EnableCloseButton(false);
	SetSizerAndFit(wrapsizer);
	CenterOnParent();
	
#ifdef __WXGTK__
	pulse_timer = new wxTimer(this,ID_PulseTimer);
#endif
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
#ifdef __WXGTK__
	pulse_timer->Stop();
#endif
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
#ifdef __WXGTK__
		pulse_timer->Start(30);
#endif
	}
}

#ifdef __WXGTK__
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
#endif

BEGIN_EVENT_TABLE(TaskProgressDialog, wxDialog)
	EVT_TASK_START(TaskProgressDialog::OnTaskStart)
	EVT_TASK_END(TaskProgressDialog::OnTaskEnd)
	EVT_TASK_PROGRESS(TaskProgressDialog::OnTaskProgress)
	EVT_TASK_ERRORMSG(TaskProgressDialog::OnTaskError)
#ifdef __WXGTK__
	EVT_TIMER(ID_PulseTimer, TaskProgressDialog::OnTimer)
#endif
END_EVENT_TABLE()