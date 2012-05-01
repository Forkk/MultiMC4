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

#include "task.h"

DEFINE_EVENT_TYPE(wxEVT_TASK_START)
DEFINE_EVENT_TYPE(wxEVT_TASK_END)

DEFINE_EVENT_TYPE(wxEVT_TASK_PROGRESS)
DEFINE_EVENT_TYPE(wxEVT_TASK_STATUS)
DEFINE_EVENT_TYPE(wxEVT_TASK_ERRORMSG)

Task::Task()
	: wxThread(wxTHREAD_DETACHED)
{

}

Task::~Task()
{
	
}

void Task::Start()
{
	wxThread::Create();
	wxThread::Run();
}

void Task::Cancel()
{
	wxThread::Delete();
}

void *Task::Entry()
{
	OnTaskStart();
	TaskStart();
	OnTaskEnd();
	return NULL;
}

void Task::SetProgress(int progress, bool fireEvent /* = true */)
{
	m_progress = progress;
	if (fireEvent)
		OnProgressChanged(progress);
}

int Task::GetProgress()
{
	return m_progress;
}

void Task::SetStatus(wxString status, bool fireEvent /* = true */)
{
	m_status = status;
	if (fireEvent)
		OnStatusChanged(status);
}

wxString Task::GetStatus()
{
	return m_status;
}

bool Task::IsRunning()
{
	return wxThread::IsRunning();
}

void Task::SetEvtHandler(wxEvtHandler* handler)
{
	this->m_evtHandler = handler;
}

void Task::OnTaskStart()
{
	TaskEvent event(wxEVT_TASK_START, this);
	m_evtHandler->AddPendingEvent(event);
}

void Task::OnTaskEnd()
{
	TaskEvent event(wxEVT_TASK_END, this);
	m_evtHandler->AddPendingEvent(event);
}

void Task::OnStatusChanged(wxString& status)
{
	
}

void Task::OnProgressChanged(int& progress)
{
	
}

void Task::OnErrorMessage(wxString& msg)
{
	
}
