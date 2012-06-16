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

#pragma once
#include <wx/string.h>

class Version
{
public:
	Version(int major, int minor, int revision, int build);
	Version(const Version &version);
	
	int GetMajor() const;
	void SetMajor(int value);
	
	int GetMinor() const;
	void SetMinor(int value);
	
	int GetRevision() const;
	void SetRevision(int value);
	
	int GetBuild() const;
	void SetBuild(int value);
	
	wxString ToString() const;
	
	bool Equals(const Version &other) const;
	
	int CompareTo(const Version &other) const;
	
	bool operator==(const Version &other) const { return Equals(other); }
	bool operator>(const Version &other) const;
	bool operator<(const Version &other) const;
	
protected:
	int m_major;
	int m_minor;
	int m_revision;
	int m_build;
};

const extern Version AppVersion;
