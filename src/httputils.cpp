// 
//  Copyright 2012 Andrew Okin
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

#include "httputils.h"
#include "curlutils.h"
#include "apputils.h"

#include <wx/sstream.h>

bool DownloadString(const wxString &url, wxString *output)
{
	CURL *curl = curl_easy_init();
	
	curl_easy_setopt(curl, CURLOPT_URL, url.ToAscii().data());
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CurlLambdaCallback);
	
	wxStringOutputStream outStream(output);
	CurlLambdaCallbackFunction curlWrite = [&outStream] (void *buffer, size_t size) -> size_t
	{
		outStream.Write(buffer, size);
		return outStream.LastWrite();
	};
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &curlWrite);
	
	int curlErr = curl_easy_perform(curl);
	curl_easy_cleanup(curl);
	
	if (curlErr != 0)
		return false;
	
	return true;
}