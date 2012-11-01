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

#include "pastebintask.h"

#include <wx/wfstream.h>
#include <wx/sstream.h>

#include "curlutils.h"
#include "apputils.h"
#include "fsutils.h"

PastebinTask::PastebinTask(const wxString& content, const wxString& author)
	: Task(), m_content(content), m_author(author)
{

}

wxThread::ExitCode PastebinTask::TaskStart()
{
	SetStatus(_("Sending to pastebin..."));

	// Create handle
	CURL *curl = curl_easy_init();
	char errBuffer[CURL_ERROR_SIZE];

	// URL encode
	wxCharBuffer contentBuf = m_content.ToUTF8();
	char *content = curl_easy_escape(curl, contentBuf.data(), strlen(contentBuf));

	wxCharBuffer posterBuf = m_author.ToUTF8();
	char *poster = curl_easy_escape(curl, posterBuf.data(), strlen(posterBuf));

	wxString postFields;
	postFields << "poster=" << poster << "&syntax=text&content=" << content;

	curl_easy_setopt(curl, CURLOPT_URL, "http://paste.ubuntu.com/");
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlLambdaCallback);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, cStr(postFields));
	curl_easy_setopt(curl, CURLOPT_HEADER, true);
	curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, &errBuffer);

	wxString outString;
	wxStringOutputStream outStream(&outString);
	CurlLambdaCallbackFunction curlWrite = [&] (void *buffer, size_t size) -> size_t
	{
		outStream.Write(buffer, size);
		return outStream.LastWrite();
	};
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &curlWrite);

	int status = curl_easy_perform(curl);

	if (status == 0)
	{
		// Parse the response header for the redirect location.
		m_pasteURL = outString.Mid(outString.Find("Location: ") + 10);
		m_pasteURL = m_pasteURL.Mid(0, m_pasteURL.Find('\n'));
		return (ExitCode)1;
	}
	else
	{
		EmitErrorMessage(wxString::Format("Pastebin failed: %s", errBuffer));
		return (ExitCode)0;
	}
}

wxString PastebinTask::GetPasteURL()
{
	return m_pasteURL;
}


