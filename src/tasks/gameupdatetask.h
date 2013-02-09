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
#include "task.h"
#include <functional>
#include <array>
#include <instance.h>
#include <wx/url.h>
#include <wx/wfstream.h>

#include "utils/curlutils.h"
#include "utils/osutils.h"

enum UpdateState
{
	STATE_INIT,
	STATE_DETERMINING_PACKAGES,
	STATE_DOWNLOADING,
	STATE_EXTRACTING_PACKAGES,
	STATE_APPLYING_PATCHES,
	STATE_DONE,
};

class GameUpdateTask : public Task
{
public:
	GameUpdateTask(Instance *inst, int64_t latestVersion, bool forceUpdate);
	virtual ~GameUpdateTask();
	
protected:
	Instance *m_inst;
	int64_t m_latestVersion;
	bool m_forceUpdate;
	
	bool m_shouldUpdate;
	
	std::vector<wxString> jarURLs;
	
	virtual ExitCode TaskStart();
	virtual void DownloadJars();
	virtual void ExtractNatives();
	
	bool RetrievePatchBaseURL(const wxString& mcVersion, wxString *patchURL);
	bool DownloadPatches(const wxString& mcVersion);
	bool ApplyPatches();
	bool VerifyPatchedFiles();
	
	virtual void SetState(UpdateState state, const wxString& msg = wxEmptyString);
};
