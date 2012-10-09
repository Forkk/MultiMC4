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

#include <wx/string.h>

// Sends text to pastebin
class PastebinTask : public Task
{
public:
	PastebinTask(const wxString& content, const wxString& author = "MultiMC");

	virtual ExitCode TaskStart();

	wxString GetPasteURL();

protected:
	wxString m_content;
	wxString m_author;

	wxString m_pasteURL;
};
