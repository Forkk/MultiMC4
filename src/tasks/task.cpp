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
	: wxThread(wxTHREAD_JOINABLE)
{
	m_status = _("");
	m_progress = 0;
	started = false;
	endCalled = false;
	startCalled = false;
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
	SetProgress(100);
	wxThread::Delete();
}

void *Task::Entry()
{
	OnTaskStart();
	TaskStart();
	OnTaskEnd();
	return NULL;
}

void Task::Dispose()
{
	wxThread::Delete();
}

void Task::SetProgress(int progress, bool fireEvent /* = true */)
{
	// For GTK to stop bitching
	if (progress >= 100)
		progress = 100;
	if (progress <= 0)
		progress = 0;
	
	m_progress = progress;
	if (fireEvent && !endCalled)
		OnProgressChanged(progress);
}

int Task::GetProgress() const
{
	return m_progress;
}

void Task::SetStatus(wxString status, bool fireEvent /* = true */)
{
	m_status = status;
	if (fireEvent && !endCalled)
		OnStatusChanged(status);
}

wxString Task::GetStatus() const
{
	return m_status;
}

bool Task::IsRunning() const
{
	return wxThread::IsRunning();
}

void Task::SetEvtHandler(wxEvtHandler* handler)
{
	this->m_evtHandler = handler;
}

void Task::OnTaskStart()
{
	if (startCalled || endCalled)
		return;
	
	TaskEvent event(wxEVT_TASK_START, this);
	m_evtHandler->AddPendingEvent(event);
}

void Task::OnTaskEnd()
{
	if (endCalled)
		return;
	endCalled = true;
	SetProgress(100);
	
	TaskEvent event(wxEVT_TASK_END, this);
	m_evtHandler->AddPendingEvent(event);
}

void Task::OnStatusChanged(const wxString& status)
{
	if (endCalled)
	{
		wxLogWarning(_("OnStatusChanged should not be called after the task ends!"));
		return;
	}
	
	TaskStatusEvent event(this, status);
	m_evtHandler->AddPendingEvent(event);
}

void Task::OnProgressChanged(const int& progress)
{
	if (endCalled)
	{
		wxLogWarning(_("OnProgressChanged should not be called after the task ends!"));
		return;
	}
	
	TaskProgressEvent event(this, progress);
	m_evtHandler->AddPendingEvent(event);
}

void Task::OnErrorMessage(const wxString& msg)
{
	TaskErrorEvent event(this, msg);
	m_evtHandler->AddPendingEvent(event);
}

void Task::SetProgressDialog(wxProgressDialog* dialog)
{
	m_progDlg = dialog;
	m_modal = true;
}

wxProgressDialog* Task::GetProgressDialog() const
{
	return m_progDlg;
}

bool Task::IsModal() const
{
	return m_modal;
}

bool Task::HasEnded() const
{
	return endCalled;
}

bool Task::HasStarted() const
{
	return started;
}

void Task::SetStarted(bool value)
{
	started = value;
}

bool Task::CanUserCancel() const
{
	return false;
}

bool Task::UserCancelled() const
{
	return userCancelled;
}

bool Task::CheckUserCancelled() const
{
	if (userCancelled)
		return true;
	
	if (IsModal())
	{
// 		wxMutexGuiEnter();
// 		bool userCancelled = !m_progDlg->Update(m_progress);
// 		wxMutexGuiLeave();
// 		return userCancelled;
	}
	return false;
}
