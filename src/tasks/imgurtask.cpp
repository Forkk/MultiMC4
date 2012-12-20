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

#include "imgurtask.h"

#include <string>
#include <sstream>

#include <boost/property_tree/json_parser.hpp>

#if WINDOWS
#include <wx/msw/winundef.h>
#endif

#include <wx/wfstream.h>
#include <wx/sstream.h>
#include <wx/mstream.h>
#include <wx/base64.h>
#include <wx/dir.h>

#include "utils/curlutils.h"
#include "utils/apputils.h"
#include "utils/fsutils.h"

const wxString imgurApiKey = "aeb90f104110450afe442760e5da12ab";
const wxString imgurApiUploadURL = "http://api.imgur.com/2/upload.json";

ImgurTask::ImgurTask(const wxString& filename)
	: Task(), m_imgFileName(filename)
{

}

wxThread::ExitCode ImgurTask::TaskStart()
{
	SetStatus(_("Sending image to imgur - Reading file..."));
	wxFFileInputStream fIn(m_imgFileName);

	int dataLen = fIn.GetLength();
	//wxMemoryBuffer dataBuf(dataLen);
	unsigned char* buffer = new unsigned char[dataLen];
	wxMemoryOutputStream mOut(buffer, dataLen);//(dataBuf.GetWriteBuf(dataLen));
	fIn.Read(mOut);
	//dataBuf.UngetWriteBuf(mOut.GetLength());

	wxString base64Data = wxBase64Encode(buffer, dataLen);//.AfterFirst(',');
	delete buffer;


	SetStatus(_("Sending image to imgur - Uploading image..."));
	CURL *curl = InitCurlHandle();
	
	wxString postData = "image=";
	postData = postData.Append(wxStr(curl_easy_escape(curl, TOASCII(base64Data), base64Data.Len())));
	wxStringInputStream imgIn(postData);

	wxString responseData;
	wxStringOutputStream out(&responseData);

	wxString url = imgurApiUploadURL + wxString::Format("?key=%s&type=base64", imgurApiKey.c_str());

	curl_easy_setopt(curl, CURLOPT_POST, true);
	curl_easy_setopt(curl, CURLOPT_URL, TOASCII(url));
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlOutStreamCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &out);
	curl_easy_setopt(curl, CURLOPT_COPYPOSTFIELDS, TOASCII(postData));
	curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, CurlLambdaProgressCallback);
	curl_easy_setopt(curl, CURLOPT_NOPROGRESS, false);

	CurlLambdaProgressCallbackFunction progressFunc = 
		[this] (double dltotal, double dlnow, double ultotal, double ulnow) -> int
	{
		double ulprogress = 100;
		double dlprogress = 100;

		if (ultotal > 0)
			ulprogress = (ulnow / ultotal) * 100;
		if (dltotal > 0)
			dlprogress = (dlnow / dltotal) * 100;

		this->SetProgress((ulprogress + dlprogress) / 2);
		return 0;
	};
	curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, &progressFunc);

	int err = curl_easy_perform(curl);
	if (err != 0)
	{
		curl_easy_cleanup(curl);

		// Bad things happened
		m_errorMsg = wxString::Format(_("libCURL error %i occurred."), err);
		return (ExitCode) 0;
	}

	curl_easy_cleanup(curl);

	SetStatus(_("Sending image to imgur - Done!"));

	using namespace boost::property_tree;
	try
	{
		// Parse the JSON returned by imgur.
		ptree pt;
		std::stringstream responseDataStream(stdStr(responseData), std::ios::in);
		read_json(responseDataStream, pt);
		
		if (pt.get_child_optional("upload.links.imgur_page") != nullptr)
		{
			m_imgURL = wxStr(pt.get<std::string>("upload.links.imgur_page"));
		}
		else
		{
			if (pt.get_child_optional("error.message") != nullptr)
				m_errorMsg = _("Imgur returned error: ") + 
					wxStr(pt.get<std::string>("error.message"));
			else
				m_errorMsg = _("Recieved an invalid response from server.");
			return (ExitCode) 0;
		}
	}
	catch (json_parser_error e)
	{
		m_errorMsg = wxString::Format(
			_("Failed to parse response data.\nJSON parser error at line %i: %s"), 
			e.line(), wxStr(e.message()).c_str());
		return (ExitCode) 0;
	}

	return (ExitCode) 1;
}

wxString ImgurTask::GetImageURL()
{
	return m_imgURL;
}

wxString ImgurTask::GetErrorMsg()
{
	return m_errorMsg;
}
