/*
 *    Copyright 2012 Andrew Okin
 * 
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 * 
 *        http://www.apache.org/licenses/LICENSE-2.0
 * 
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include "buildtag.h"
#include "config.h"

#include <wx/string.h>
#include <wx/tokenzr.h>

#include <string>
#include <cstdlib>

#include "apputils.h"

const BuildTag AppBuildTag(_T(JENKINS_BUILD_TAG));

BuildTag::BuildTag(const wxString &buildType, const wxString &jobName, int buildNumber)
	: m_buildType(buildType), m_jobName(jobName)
{
	m_buildNumber = buildNumber;
}

BuildTag::BuildTag(const BuildTag &tag)
	: m_jobName(tag.GetJobName())
{
	m_buildNumber = tag.GetBuildNum();
}

BuildTag::BuildTag(const wxString &str)
{
	wxArrayString strings = wxStringTokenize(str, _T("-"));
	if (strings.GetCount() >= 3)
	{
		m_buildType = strings[0];
		m_jobName = strings[1];
		m_buildNumber = atoi(TOASCII(strings[strings.GetCount() - 1]));
	}
	else
	{
		m_buildType = _T("CustomBuild");
		m_jobName = _T("MultiMC");
		m_buildNumber = 0;
	}
}


wxString BuildTag::ToString() const
{
	wxString str = wxString::Format(_T("%s-%s-%i"), 
		GetBuildType().c_str(), GetJobName().c_str(), GetBuildNum());

	return str;
}

wxString BuildTag::GetBuildType() const
{
	return m_buildType;
}

void BuildTag::SetBuildType(const wxString &value)
{
	m_buildType = value;
}


wxString BuildTag::GetJobName() const
{
	return m_jobName;
}

void BuildTag::SetJobName(const wxString &value)
{
	m_jobName = value;
}


int BuildTag::GetBuildNum() const
{
	return m_buildNumber;
}

void BuildTag::SetBuildNum(int value)
{
	m_buildNumber = value;
}
