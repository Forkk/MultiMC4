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

// Better OS Detection
#if defined _WIN32 | defined _WIN64
#define WINDOWS	1
#elif __APPLE__ & __MACH__
#define OSX 1
#elif __linux__
#define LINUX 1
#endif

#if _WIN32 || _WIN64
#if _WIN64
#define ENV64 1
#else
#define ENV32 1
#endif

#elif __GNUC__
#if __x86_64__ || __ppc64__
#define ENV64 1
#else
#define ENV32 1
#endif
#endif

#if WINDOWS
#define NEWLINE "\r\n"
#else
#define NEWLINE "\n"
#endif

#ifdef USE_DEPRECATED_MACROS
#warning Using deprecated version detection macros.

#define IS_WINDOWS() WINDOWS
#define IS_LINUX() LINUX
#define IS_MAC() OSX
#endif
