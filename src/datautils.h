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
#include <vector>

#include <wx/wx.h>

template<typename T>
bool VectorContains(const std::vector<T>& vec, const T& value);

template<class InputIterator, class Function>
bool Any(InputIterator first, InputIterator last, Function f)
{
	for (; first != last; ++first)
	{
		if (f(*first))
			return true;
	}
	return false;
}

template<class InputIterator, class Predicate>
int Count(InputIterator first, InputIterator last, Predicate pred)
{
	int count = 0;
	for (; first != last; ++first)
	{
		if (pred(*first))
			count++;
	}
	return count;
}

template<class InputIterator, class Predicate>
size_t Find(InputIterator first, InputIterator last, Predicate pred)
{
	for (int i = 0; first != last; ++first, ++i)
	{
		if (pred(*first))
			return i;
	}
	return -1;
}

wxString ReadAllText(wxInputStream &input);
wxStringList ReadAllLines(wxInputStream &input);

void WriteAllText(wxOutputStream &output, wxString text);
