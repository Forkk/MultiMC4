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

#pragma once
#include <wx/wx.h>
#include <wx/event.h>
#include <wx/progdlg.h>

class Task;
struct TaskEvent;
struct TaskProgressEvent;
struct TaskErrorEvent;

class Task : protected wxThread
{
public:
	
	Task();
	~Task();
	
	// Start the task by calling this
	void Start(wxEvtHandler *handler, bool modal);
	
	// Start the task from inside an another task, chaining it.
	ExitCode Chain(Task * parent);
	
	// Every task has to be waited on to properly free up resources.
	// ExitCode of 1 means everything went OK. 0 means failure
	ExitCode Wait()
	{
		return wxThread::Wait();
	};

	// Accessors
	virtual wxString GetStatus();
	virtual int GetProgress();
	bool isModal();
	bool hasEnded();
	
protected:
	// entry point for the task's thread - fires start/end events and calls TaskStart
	// The returned value is the thread exit code which is the value returned by Wait.
	virtual ExitCode Entry();
	// This is the actual entry point. override this in the derived classes
	virtual ExitCode TaskStart() = 0;
	
	virtual void SetStatus(wxString status);
	virtual void SetProgress(int progress);
	
	virtual void EmitTaskStart();
	virtual void EmitTaskEnd();
	virtual void EmitProgressChanged(const int &progress);
	virtual void EmitStatusChanged(const wxString &status);
	virtual void EmitErrorMessage(const wxString &msg);
	
	wxString m_status;
	int m_progress;
	wxEvtHandler *m_evtHandler;
	bool m_modal;
	bool ended;
	wxMutex access;
};

DECLARE_EVENT_TYPE(wxEVT_TASK_START, -1)
DECLARE_EVENT_TYPE(wxEVT_TASK_END, -1)

DECLARE_EVENT_TYPE(wxEVT_TASK_PROGRESS, -1)
DECLARE_EVENT_TYPE(wxEVT_TASK_STATUS, -1)
DECLARE_EVENT_TYPE(wxEVT_TASK_ERRORMSG, -1)

struct TaskEvent : wxThreadEvent
{
	TaskEvent(wxEventType type, Task *task) : wxThreadEvent(type, -1) { m_task = task; }
	
	Task *m_task;
	virtual wxEvent *Clone() const
	{
		return new TaskEvent(GetEventType(), m_task);
	}
};

struct TaskProgressEvent : TaskEvent
{
	TaskProgressEvent(Task *task, const int &progress, const wxString &status) 
		: TaskEvent(wxEVT_TASK_PROGRESS, task)
	{
		m_progress = progress;
		m_status = status;
	}
	
	int m_progress;
	wxString m_status;
	virtual wxEvent *Clone() const
	{
		return new TaskProgressEvent(m_task, m_progress, m_status);
	}
};

struct TaskErrorEvent : TaskEvent
{
	TaskErrorEvent(Task *task, const wxString &errorMsg) 
		: TaskEvent(wxEVT_TASK_ERRORMSG, task) { m_errorMsg = errorMsg; }
	
	wxString m_errorMsg;
	virtual wxEvent *Clone() const
	{
		return new TaskErrorEvent(m_task, m_errorMsg);
	}
};

typedef void (wxEvtHandler::*TaskEventFunction)(TaskEvent&);

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

#define EVT_TASK_ERRORMSG(fn)\
	DECLARE_EVENT_TABLE_ENTRY(wxEVT_TASK_ERRORMSG, wxID_ANY, -1,\
		(wxObjectEventFunction)(wxEventFunction)\
		(TaskErrorEventFunction) &fn, (wxObject*) NULL),

#define EVT_TASK_CUSTOM(evt_id, fn, type)\
	DECLARE_EVENT_TABLE_ENTRY(evt_id, wxID_ANY, -1,\
		(wxObjectEventFunction)(wxEventFunction)\
		(type) &fn, (wxObject*) NULL),

#define TaskEventHandler(func) \
	(wxObjectEventFunction)(wxEventFunction)wxStaticCastEvent(TaskEventFunction, &func)

#define TaskProgressHandler(func) \
	(wxObjectEventFunction)(wxEventFunction)wxStaticCastEvent(TaskProgressEventFunction, &func)

#define TaskErrorEventHandler(func) \
	(wxObjectEventFunction)(wxEventFunction)wxStaticCastEvent(TaskErrorEventFunction, &func)

