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

#pragma once
#include <string>

#include <wx/wx.h>
#include <wx/utils.h>
#include <wx/imaglist.h>
#include <wx/listctrl.h>
#include <wx/notebook.h>
#include <wx/statbox.h>
#include <wx/tglbtn.h>
#include <wx/spinctrl.h>

#include <boost/pending/queue.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/foreach.hpp>
#include <boost/unordered_map.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/signal.hpp>
#include <boost/function.hpp>

#include "InstIconList.h"
#include "Instance.h"
#include "AppSettings.h"
#include "Task.h"

//#include "MainWindow.h"
#include "SettingsDialog.h"
#include "AppUtils.h"

namespace fs = boost::filesystem;