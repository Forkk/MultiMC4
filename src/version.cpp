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

#include "version.h"
#include "config.h"

const Version AppVersion(VERSION_MAJOR, VERSION_MINOR, VERSION_REVISION, VERSION_BUILD);

Version::Version(int major, int minor, int revision, int build)
{
	m_major = major;
	m_minor = minor;
	m_revision = revision;
	m_build = build;
}

Version::Version(const Version &version)
{
	m_major = version.GetMajor();
	m_minor = version.GetMinor();
	m_revision = version.GetRevision();
	m_build = version.GetBuild();
}

int Version::GetMajor() const
{
	return m_major;
}

void Version::SetMajor(int value)
{
	m_major = value;
}


int Version::GetMinor() const
{
	return m_minor;
}

void Version::SetMinor(int value)
{
	m_minor = value;
}


int Version::GetRevision() const
{
	return m_revision;
}

void Version::SetRevision(int value)
{
	m_revision = value;
}

int Version::GetBuild() const
{
	return m_build;
}

void Version::SetBuild(int value)
{
	m_build = value;
}


bool Version::Equals(const Version &other) const
{
	return GetMajor() == other.GetMajor() && 
		   GetMinor() == other.GetMinor() &&
		   GetRevision() == other.GetRevision() &&
		   GetBuild() == other.GetBuild();
}

bool Version::operator<(const Version &other) const
{
	return CompareTo(other) < 0;
}

bool Version::operator>(const Version &other) const
{
	return CompareTo(other) > 0;
}

int Version::CompareTo(const Version &other) const
{
	if (GetMajor() > other.GetMajor())
		return 1;
	else if (GetMajor() < other.GetMajor())
		return -1;
	
	if (GetMinor() > other.GetMinor())
		return 1;
	else if (GetMinor() < other.GetMinor())
		return -1;
	
	if (GetRevision() > other.GetRevision())
		return 1;
	else if (GetRevision() < other.GetRevision())
		return -1;
	
	if (GetBuild() > other.GetBuild())
		return 1;
	else if (GetBuild() < other.GetBuild())
		return -1;
	
	return 0;
}

wxString Version::ToString() const
{
	return wxString::Format(_T("%i.%i.%i.%i"), GetMajor(), GetMinor(), GetRevision(), GetBuild());
}
