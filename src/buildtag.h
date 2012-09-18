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
#include <wx/string.h>

class BuildTag
{
public:
	BuildTag(const wxString &buildType, const wxString &jobName, int buildNumber);
	BuildTag(const wxString &str);
	BuildTag(const BuildTag &tag);

	wxString GetBuildType() const;
	void SetBuildType(const wxString &value);

	wxString GetJobName() const;
	void SetJobName(const wxString &value);
	
	int GetBuildNum() const;
	void SetBuildNum(int value);
	
	wxString ToString() const;
	
protected:
	wxString m_buildType;
	wxString m_jobName;
	int m_buildNumber;
};

const extern BuildTag AppBuildTag;
