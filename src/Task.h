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

class Task : protected wxThread
{
public:
	Task();
	~Task();

	void Start();
	void Cancel();
	void Dispose();

	virtual void *Entry();

	// Called when the task starts.
	boost::signal<void (Task*)> OnStart;

	// Called when the task ends.
	boost::signal<void (Task*)> OnEnd;

	// Called when the progress changes.
	boost::signal<void (Task*, int progress)> OnProgressChanged;

	// Called when the status changes.
	boost::signal<void (Task*, wxString status)> OnStatusChanged;

	// Called when an error occurs.
	boost::signal<void (Task*, wxString msg)> OnErrorMessage;

	// Accessors
	virtual wxString GetStatus();
	virtual int GetProgress();
	virtual bool IsRunning();

	int GetID();

protected:
	virtual void TaskStart() = 0;

	virtual void SetStatus(wxString status, bool fireEvent = true);
	virtual void SetProgress(int progress, bool fireEvent = true);

	wxString m_status;
	int m_progress;

private:
	int m_taskID;
};