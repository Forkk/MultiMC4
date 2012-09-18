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

#include "task.h"

DEFINE_EVENT_TYPE(wxEVT_TASK_START)
DEFINE_EVENT_TYPE(wxEVT_TASK_END)

DEFINE_EVENT_TYPE(wxEVT_TASK_PROGRESS)
DEFINE_EVENT_TYPE(wxEVT_TASK_STATUS)
DEFINE_EVENT_TYPE(wxEVT_TASK_ERRORMSG)

Task::Task()
	: wxThread(wxTHREAD_JOINABLE)
{
	m_status = _("");
	m_progress = 0;
	m_modal = false;
	ended = false;
}

Task::~Task()
{
	
}

void Task::Start(wxEvtHandler * handler, bool modal)
{
	m_evtHandler = handler;
	m_modal = modal;
	wxThread::Create();
	wxThread::Run();
}

wxThread::ExitCode Task::Entry()
{
	EmitTaskStart();
	ExitCode ec = TaskStart();
	EmitTaskEnd();
	ended = true;
	return ec;
}

void Task::SetProgress(int progress)
{
	wxMutexLocker lock(access);
	// For GTK to stop bitching
	if (progress >= 100)
		progress = 100;
	if (progress <= 0)
		progress = 0;
	
	if(m_progress != progress)
	{
		m_progress = progress;
		EmitProgressChanged(progress);
	}
}

int Task::GetProgress()
{
	wxMutexLocker lock(access);
	return m_progress;
}

void Task::SetStatus(wxString status)
{
	wxMutexLocker lock(access);
	if(m_status != status)
	{
		m_status = status;
		EmitStatusChanged(status);
	}
}

wxString Task::GetStatus()
{
	wxMutexLocker lock(access);
	return m_status;
}

bool Task::hasEnded()
{
	wxMutexLocker lock(access);
	return ended;
}

bool Task::isModal()
{
	wxMutexLocker lock(access);
	return m_modal;
}


void Task::EmitTaskStart()
{
	TaskEvent event(wxEVT_TASK_START, this);
	m_evtHandler->AddPendingEvent(event);
}

void Task::EmitTaskEnd()
{
	SetProgress(100);
	TaskEvent event(wxEVT_TASK_END, this);
	m_evtHandler->AddPendingEvent(event);
}

void Task::EmitStatusChanged(const wxString& status)
{
	TaskProgressEvent event(this, m_progress, status);
	m_evtHandler->AddPendingEvent(event);
}

void Task::EmitProgressChanged(const int& progress)
{
	TaskProgressEvent event(this, progress, m_status);
	m_evtHandler->AddPendingEvent(event);
}

void Task::EmitErrorMessage(const wxString& msg)
{
	TaskErrorEvent event(this, msg);
	m_evtHandler->AddPendingEvent(event);
}
