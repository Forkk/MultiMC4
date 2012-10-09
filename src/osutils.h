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
#include <wx/platform.h>

#include "apputils.h"

#define IS_WINDOWS() ENUM_CONTAINS(wxPlatformInfo::Get().GetOperatingSystemId(), wxOS_WINDOWS_NT)

#define IS_LINUX() ENUM_CONTAINS(wxPlatformInfo::Get().GetOperatingSystemId(), wxOS_UNIX_LINUX)

#ifdef __APPLE__
#define IS_MAC() true
#else
#define IS_MAC() ENUM_CONTAINS(wxPlatformInfo::Get().GetOperatingSystemId(), wxOS_MAC)
#endif

#if defined __WXMSW__
#define NEWLINE "\r\n"
#else
#define NEWLINE "\n"
#endif
