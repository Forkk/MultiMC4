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

#include "datautils.h"
#include <wx/sstream.h>
#include <wx/tokenzr.h>

template <class T>
bool VectorContains(const std::vector<T>& vec, const T& value)
{
	typename std::vector<T>::iterator iter;
	for (iter = vec.begin(); iter != vec.end(); ++iter)
	{
		if (value == *iter)
			return true;
	}
}

wxString ReadAllText(wxInputStream& input)
{
	wxString outString;
	wxStringOutputStream outStream(&outString);
	outStream.Write(input);
	return outString;
}

wxStringList ReadAllLines(wxInputStream& input)
{
	wxString text = ReadAllText(input);
	wxStringTokenizer tokenizer(text, _("\r\n"));
	wxStringList lineList;
	while (tokenizer.HasMoreTokens())
	{
		lineList.Add(tokenizer.NextToken());
	}
	return lineList;
}

void WriteAllText(wxOutputStream& output, wxString text)
{
	wxStringInputStream input(text);
	output.Write(input);
}
