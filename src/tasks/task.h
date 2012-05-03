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

#pragma once
#include "includes.h"

DECLARE_EVENT_TYPE(wxEVT_TASK_START, -1)
DECLARE_EVENT_TYPE(wxEVT_TASK_END, -1)

DECLARE_EVENT_TYPE(wxEVT_TASK_PROGRESS, -1)
DECLARE_EVENT_TYPE(wxEVT_TASK_STATUS, -1)
DECLARE_EVENT_TYPE(wxEVT_TASK_ERRORMSG, -1)

class Task : protected wxThread
{
public:
	Task();
	~Task();
	
	void Start();
	void Cancel();
	void Dispose();
	
	virtual void *Entry();
	
	
	// Accessors
	virtual wxString GetStatus();
	virtual int GetProgress();
	virtual bool IsRunning();
	
	virtual void SetEvtHandler(wxEvtHandler *handler);
	
	int GetID();
	
protected:
	virtual void TaskStart() = 0;
	
	virtual void SetStatus(wxString status, bool fireEvent = true);
	virtual void SetProgress(int progress, bool fireEvent = true);
	
	virtual void OnTaskStart();
	virtual void OnTaskEnd();
	virtual void OnProgressChanged(int &progress);
	virtual void OnStatusChanged(wxString &status);
	virtual void OnErrorMessage(wxString &msg);
	
	wxString m_status;
	int m_progress;
	
	wxEvtHandler *m_evtHandler;
private:
	int m_taskID;
};

struct TaskEvent : wxNotifyEvent
{
	TaskEvent(wxEventType type, const Task *task) : wxNotifyEvent(type, -1) { m_task = task; }
	
	const Task *m_task;
	virtual wxEvent *Clone() const
	{
		return new TaskEvent(GetEventType(), m_task);
	}
};

struct TaskStatusEvent : TaskEvent
{
	TaskStatusEvent(const Task *task, const wxString &status)
		: TaskEvent(wxEVT_TASK_STATUS, task) { m_status = status; }
	
	wxString m_status;
	virtual wxEvent *Clone() const
	{
		return new TaskStatusEvent(m_task, m_status);
	}
};

struct TaskProgressEvent : TaskEvent
{
	TaskProgressEvent(const Task *task, const int &progress) 
		: TaskEvent(wxEVT_TASK_PROGRESS, task) { m_progress = progress; }
	
	int m_progress;
	virtual wxEvent *Clone() const
	{
		return new TaskProgressEvent(m_task, m_progress);
	}
};

struct TaskErrorEvent : TaskEvent
{
	TaskErrorEvent(Task *task, wxString &errorMsg) 
		: TaskEvent(wxEVT_TASK_ERRORMSG, task) { m_errorMsg = errorMsg; }
	
	wxString m_errorMsg;
	virtual wxEvent *Clone() const
	{
		return new TaskStatusEvent(m_task, m_errorMsg);
	}
};

typedef void (wxEvtHandler::*TaskEventFunction)(TaskEvent&);

typedef void (wxEvtHandler::*TaskStatusEventFunction)(TaskStatusEvent&);
typedef void (wxEvtHandler::*TaskProgressEventFunction)(TaskProgressEvent&);
typedef void (wxEvtHandler::*TaskErrorEventFunction)(TaskErrorEvent&);

#define EVT_TASK_START(fn)\
	DECLARE_EVENT_TABLE_ENTRY(wxEVT_TASK_START, wxID_ANY, -1,\
		(wxObjectEventFunction)(wxEventFunction)\
		(TaskEventFunction) &fn, (wxObject*) NULL),

#define EVT_TASK_END(fn)\
	DECLARE_EVENT_TABLE_ENTRY(wxEVT_TASK_END, wxID_ANY, -1,\
		(wxObjectEventFunction)(wxEventFunction)\
		(TaskEventFunction) &fn, (wxObject*) NULL ),

#define EVT_TASK_PROGRESS(fn)\
	DECLARE_EVENT_TABLE_ENTRY(wxEVT_TASK_PROGRESS, wxID_ANY, -1,\
		(wxObjectEventFunction)(wxEventFunction)\
		(TaskProgressEventFunction) &fn, (wxObject*) NULL),

#define EVT_TASK_STATUS(fn)\
	DECLARE_EVENT_TABLE_ENTRY(wxEVT_TASK_STATUS, wxID_ANY, -1,\
		(wxObjectEventFunction)(wxEventFunction)\
		(TaskStatusEventFunction) &fn, (wxObject*) NULL),

#define EVT_TASK_ERRORMSG(fn)\
	DECLARE_EVENT_TABLE_ENTRY(wxEVT_TASK_ERRORMSG, wxID_ANY, -1,\
		(wxObjectEventFunction)(wxEventFunction)\
		(TaskProgressEventFunction) &fn, (wxObject*) NULL),

#define EVT_TASK_CUSTOM(evt_id, fn, type)\
	DECLARE_EVENT_TABLE_ENTRY(evt_id, wxID_ANY, -1,\
		(wxObjectEventFunction)(wxEventFunction)\
		(type) &fn, (wxObject*) NULL),
