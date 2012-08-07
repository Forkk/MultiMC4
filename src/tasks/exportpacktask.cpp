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

#include "exportpacktask.h"

#include <boost/property_tree/json_parser.hpp>

#include <wx/zipstrm.h>
#include <wx/wfstream.h>
#include <wx/sstream.h>

#include <string>
#include <sstream>

#include "apputils.h"

ExportPackTask::ExportPackTask(Instance *inst, const wxString &packName, 
	const wxString &packNotes, const wxString &filename, wxArrayString &includedConfigs)
	: m_packName(packName), m_packNotes(packNotes), m_filename(filename), m_includedConfigs(includedConfigs)
{
	m_inst = inst;
}

void ExportPackTask::TaskStart()
{
	wxFFileOutputStream fileOut(m_filename);
	wxZipOutputStream zipOut(fileOut);

	SetStatus(_("Adding config files..."));
	for (wxArrayString::iterator iter = m_includedConfigs.begin(); iter != m_includedConfigs.end(); ++iter)
	{
		wxFileName destFile(*iter);
		destFile.MakeRelativeTo(m_inst->GetRootDir().GetFullPath());
		zipOut.PutNextEntry(destFile.GetFullPath());

		wxFFileInputStream confIn(*iter);
		zipOut.Write(confIn);
	}

	SetStatus(_("Writing metadata..."));
	using namespace boost::property_tree;
	ptree pt;

	pt.put<std::string>("name", stdStr(m_packName));
	pt.put<std::string>("notes", stdStr(m_packNotes));

	std::stringstream jsonOut;
	write_json(jsonOut, pt);
	wxString json = wxStr(jsonOut.str());

	zipOut.PutNextEntry(_("modpack.json"));
	wxStringInputStream jsonIn(json);
	zipOut.Write(jsonIn);
}
