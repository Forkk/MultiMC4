/*
    Copyright 2012 Andrew Okin

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

#pragma once
#include "task.h"
#include <functional>
#include <instance.h>
#include <boost/array.hpp>
#include <wx/url.h>
#include <wx/wfstream.h>

#include "curlutils.h"
#include "osutils.h"

enum UpdateState
{
	STATE_INIT,
	STATE_DETERMINING_PACKAGES,
	STATE_CHECKING_CACHE,
	STATE_DOWNLOADING,
	STATE_EXTRACTING_PACKAGES,
	STATE_DONE,
};

class GameUpdateTask : public Task
{
public:
	GameUpdateTask(Instance *inst, 
				   wxString latestVersion, 
				   wxString mainGameURL, 
				   bool forceUpdate);
	
protected:
	Instance *m_inst;
	wxString m_latestVersion;
	wxString m_mainGameURL;
	bool m_forceUpdate;
	
	bool m_shouldUpdate;
	
	boost::array<wxURL, 5> jarURLs;
	
	virtual void TaskStart();
	virtual void LoadJarURLs();
	virtual void AskToUpdate();
	virtual void DownloadJars();
	virtual void ExtractNatives();
	
	virtual void SetState(UpdateState state);
	
	virtual void OnGameUpdateComplete();
	
// 	struct WriteJarCBkInfo
// 	{
// 		WriteJarCBkInfo(GameUpdateTask *task, 
// 						wxFileOutputStream *fileOutStream, 
// 						int initialProgress, 
// 						int currentFileSize, 
// 						int totalDownloadSize)
// 		{
// 			m_task = task;
// 			m_fileOutStream = fileOutStream;
// 			m_initialProgress = initialProgress;
// 			m_currentFileSize = currentFileSize;
// 			m_totalDLSize = totalDownloadSize;
// 		}
// 		
// 		GameUpdateTask *m_task;
// 		wxFileOutputStream *m_fileOutStream;
// 		int m_initialProgress;
// 		int m_currentFileSize;
// 		int m_totalDLSize;
// 	};
};

DECLARE_EVENT_TYPE(wxEVT_GAME_UPDATE_COMPLETE, -1)

#define EVT_GAME_UPDATE_COMPLETE(fn) EVT_TASK_CUSTOM(wxEVT_GAME_UPDATE_COMPLETE, fn, TaskEventFunction)
