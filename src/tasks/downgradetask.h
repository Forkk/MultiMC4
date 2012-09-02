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
#include "instance.h"

class DowngradeTask : public Task
{
public:
	DowngradeTask(Instance *inst, const wxString& targetVersion);

	ExitCode TaskStart();

protected:
	Instance *m_inst;
	wxString m_targetVersion;

	wxString statusDetail;

	enum TaskStep
	{
		STEP_DOWNLOAD_PATCHES = 1,
		STEP_VERIFY_FILES = 2,
		STEP_APPLY_PATCHES = 3,
		STEP_VERIFY_FILES2 = 4,
		STEP_DONE = 5,
	} currentStep;

	void SetStep(TaskStep step, bool clearStatusDetail = true);
	void SetStatusDetail(wxString detail);
	void UpdateStatus();

	bool RetrievePatchBaseURL(const wxString& mcVersion, wxString *patchURL);

	bool DownloadPatches();
	bool VerifyOriginalFiles();
	bool ApplyPatches();
	bool VerifyPatchedFiles();
};
