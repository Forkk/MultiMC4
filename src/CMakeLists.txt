cmake_minimum_required (VERSION 2.8)
project (MultiMC)

# Set the path where CMake will look for modules.
#set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}")


# Set the sources and headers variables.
file(GLOB SRCS "*.cpp")
set(SRCS 
gui/MainWindow.cpp
gui/SettingsDialog.cpp

data/AppSettings.cpp
data/Instance.cpp

tasks/Task.cpp

AppUtils.cpp
InstIconList.cpp
)

set (INCS
includes.h

gui/MainWindow.h
gui/SettingsDialog.h

data/AppSettings.h
data/Instance.h

tasks/Task.h

AppUtils.h
InstIconList.h

InstIcons.h
ToolbarIcons.h
)

# Find wxWidgets.
MESSAGE(STATUS "** Finding wxWidgets")
find_package(wxWidgets REQUIRED core base)
MESSAGE(STATUS "** wxWidgets Root: ${wxWidgets_ROOT_DIR}")
MESSAGE(STATUS "** wxWidgets Configuration: ${wxWidgets_CONFIGURATION}")

# Find boost.
set(Boost_USE_STATIC_LIBS ON)
MESSAGE(STATUS "** Finding Boost...")
find_package(Boost 1.46.0 COMPONENTS system filesystem signals REQUIRED)
MESSAGE(STATUS "** Boost Include: ${Boost_INCLUDE_DIR}")
MESSAGE(STATUS "** Boost Libraries: ${Boost_LIBRARY_DIRS}")

# Include wxWidgets.
include("${wxWidgets_USE_FILE}")

# Include boost.
include_directories("${Boost_INCLUDE_DIRS}")

include_directories(
"${PROJECT_SOURCE_DIR}"
"${PROJECT_SOURCE_DIR}/data"
"${PROJECT_SOURCE_DIR}/gui"
"${PROJECT_SOURCE_DIR}/tasks"
)

# Add the executable.
add_executable(MultiMC WIN32 ${SRCS} ${INCS})

# Link the libraries.
target_link_libraries(MultiMC
	${wxWidgets_LIBRARIES}
	${Boost_LIBRARIES}
)
