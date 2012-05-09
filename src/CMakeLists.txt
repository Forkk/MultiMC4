cmake_minimum_required (VERSION 2.8)
project (MultiMC)

# Set the path where CMake will look for modules.
#set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}")


# Set the sources and headers variables.
file(GLOB SRCS "*.cpp")
set(SRCS 
gui/mainwindow.cpp
gui/settingsdialog.cpp
gui/logindialog.cpp
gui/consolewindow.cpp

data/appsettings.cpp
data/instance.cpp

tasks/task.cpp
tasks/logintask.cpp
tasks/gameupdatetask.cpp

apputils.cpp
insticonlist.cpp
)

set (INCS
includes.h

gui/mainwindow.h
gui/settingsdialog.h
gui/logindialog.h
gui/consolewindow.h

data/appsettings.h
data/instance.h

tasks/task.h
tasks/logintask.h
tasks/gameupdatetask.h

apputils.h
insticonlist.h

insticons.h
toolbaricons.h
)

# Find wxWidgets.
MESSAGE(STATUS "** Finding wxWidgets")
find_package(wxWidgets REQUIRED core base net)
MESSAGE(STATUS "** wxWidgets Root: ${wxWidgets_ROOT_DIR}")
MESSAGE(STATUS "** wxWidgets Configuration: ${wxWidgets_CONFIGURATION}")

# Find boost.
set(Boost_USE_STATIC_LIBS ON)
MESSAGE(STATUS "** Finding Boost...")
find_package(Boost 1.46.0 COMPONENTS regex REQUIRED)
MESSAGE(STATUS "** Boost Include: ${Boost_INCLUDE_DIR}")
MESSAGE(STATUS "** Boost Libraries: ${Boost_LIBRARY_DIRS}")

IF(UNIX)
  # assume GCC, add C++0x/C++11 stuff
  SET(CMAKE_CXX_FLAGS "-std=c++0x")
ENDIF()

# Find libCURL
find_package(CURL REQUIRED)

# Include wxWidgets.
include("${wxWidgets_USE_FILE}")

# Include CURL
include_directories("${CURL_INCLUDE_DIR}")

# Include boost.
include_directories("${Boost_INCLUDE_DIRS}")

# Make the static md5 lib build
add_subdirectory(md5)

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
	${CURL_LIBRARIES}
        tinymd5
)
