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

#include "Task.h"

Task::Task()
	: wxThread(wxTHREAD_DETACHED)
{

}

Task::~Task()
{
	OnStart.disconnect_all_slots();
	OnEnd.disconnect_all_slots();
	OnProgressChanged.disconnect_all_slots();
	OnStatusChanged.disconnect_all_slots();
	OnErrorMessage.disconnect_all_slots();
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
	OnStart(this);
	TaskStart();
	OnEnd(this);
	return NULL;
}

void Task::SetProgress(int progress, bool fireEvent /* = true */)
{
	m_progress = progress;
	if (fireEvent)
		OnProgressChanged(this, progress);
}

int Task::GetProgress()
{
	return m_progress;
}

void Task::SetStatus(wxString status, bool fireEvent /* = true */)
{
	m_status = status;
	if (fireEvent)
		OnStatusChanged(this, status);
}

wxString Task::GetStatus()
{
	return m_status;
}

bool Task::IsRunning()
{
	return wxThread::IsRunning();
}
