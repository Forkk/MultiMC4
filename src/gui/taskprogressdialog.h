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

#pragma once
#include <wx/wx.h>
#include <task.h>

enum
{
	ID_PulseTimer = 1,
};

class TaskProgressDialog : public wxDialog
{
public:
	TaskProgressDialog(wxWindow * parent);
	int ShowModal( Task* run_task );
	
protected:
	// UI elements
	wxGauge * gauge;
	wxStaticText * message;
	
	// Stuff used to auto-pulse the progress bar
#ifdef __WXGTK__
	wxTimer * pulse_timer;
	void OnTimer(wxTimerEvent& event);
#endif
	bool is_pulsing;
	void StartPulsing();
	
	// The managed task and its events
	Task * task;
	void OnTaskStart(TaskEvent &event);
	void OnTaskEnd(TaskEvent &event);
	void OnTaskProgress(TaskProgressEvent &event);
	void OnTaskError(TaskErrorEvent &event);
	
	DECLARE_EVENT_TABLE()
	
};
