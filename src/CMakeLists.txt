cmake_minimum_required (VERSION 2.8)
project (MultiMC)

IF (DEFINED CMAKE_TOOLCHAIN_FILE)
	include("${CMAKE_TOOLCHAIN_FILE}")
ENDIF ()

IF (MSVC)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc /D MSVC")

	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /D CURL_STATICLIB")
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D CURL_STATICLIB")
ELSE ()
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DCURL_STATICLIB")
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DCURL_STATICLIB")
ENDIF ()

IF (WIN32)
	SET(MULTIMC_ADDITIONAL_LINKS "ws2_32")
ENDIF ()

# Set the path where CMake will look for modules.
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}")

set(MultiMC_VERSION_MAJOR		4)
set(MultiMC_VERSION_MINOR		0)
set(MultiMC_VERSION_REV			0)

IF (DEFINED MultiMC_VERSION_BUILD)
	message(STATUS "MultiMC build #${MultiMC_VERSION_BUILD}")
ELSE ()
	set(MultiMC_VERSION_BUILD	0)
	message(STATUS "Build number not specified. Defaulted to 0.")
ENDIF ()

IF (DEFINED MultiMC_ARCH)
	message (STATUS "Architecture is ${MultiMC_ARCH}")
ELSE()
	set (MultiMC_ARCH "x86")
	message (STATUS "Architecture not specified. Defaulted to x86.")
ENDIF ()

configure_file("${PROJECT_SOURCE_DIR}/config.h.in"
			   "${PROJECT_BINARY_DIR}/config.h")

IF (CMAKE_CROSSCOMPILING)
	set(CMAKE_EXE_LINK_DYNAMIC_C_FLAGS "-Wl,-Bstatic -static-libgcc -static-libstdc++")
	set(CMAKE_EXE_LINK_DYNAMIC_CXX_FLAGS "-Wl,-Bstatic -static-libgcc -static-libstdc++")
ENDIF ()

# Set the sources and headers variables.
set(SRCS 
multimc.cpp

gui/mainwindow.cpp
gui/settingsdialog.cpp
gui/logindialog.cpp
gui/consolewindow.cpp
gui/modeditwindow.cpp
gui/changeicondialog.cpp

data/appsettings.cpp
data/instance.cpp
data/userinfo.cpp
data/mod.cpp

tasks/task.cpp
tasks/logintask.cpp
tasks/moddertask.cpp
tasks/gameupdatetask.cpp
tasks/checkupdatetask.cpp
tasks/filedownloadtask.cpp

apputils.cpp
curlutils.cpp
datautils.cpp
fsutils.cpp
httputils.cpp

version.cpp

insticonlist.cpp
)

set (INCS
multimc.h

gui/mainwindow.h
gui/settingsdialog.h
gui/logindialog.h
gui/consolewindow.h
gui/modeditwindow.h
gui/changeicondialog.h

data/appsettings.h
data/instance.h
data/userinfo.h
data/mod.h

tasks/task.h
tasks/logintask.h
tasks/moddertask.h
tasks/gameupdatetask.h
tasks/checkupdatetask.h
tasks/filedownloadtask.h

apputils.h
curlutils.h
osutils.h
datautils.h
fsutils.h
httputils.h

version.h

insticonlist.h
insticons.h
toolbaricons.h
)

# Find wxWidgets.
MESSAGE(STATUS "** Finding wxWidgets")
find_package(wxWidgets REQUIRED core base net adv)
MESSAGE(STATUS "** wxWidgets Root: ${wxWidgets_ROOT_DIR}")
MESSAGE(STATUS "** wxWidgets Configuration: ${wxWidgets_CONFIGURATION}")

# Find boost.
set(Boost_USE_STATIC_LIBS ON)
MESSAGE(STATUS "** Finding Boost...")
find_package(Boost 1.46.0 REQUIRED)
MESSAGE(STATUS "** Boost Include: ${Boost_INCLUDE_DIR}")
MESSAGE(STATUS "** Boost Libraries: ${Boost_LIBRARY_DIRS}")

#find_package(winsock REQUIRED)

IF(UNIX)
  # assume GCC, add C++0x/C++11 stuff
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
ELSEIF(MINGW)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++0x")
ENDIF()

# Find libCURL
find_package(CURL REQUIRED)

# Include wxWidgets.
include("${wxWidgets_USE_FILE}")

if(WIN32)
  set(SRCS ${SRCS} multimc.rc)
endif(WIN32)

# Include CURL
include_directories("${CURL_INCLUDE_DIR}")

# Include boost.
include_directories("${Boost_INCLUDE_DIRS}")

# Make the static md5 lib build
add_subdirectory(md5)

include_directories("${PROJECT_BINARY_DIR}")

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
	${MULTIMC_ADDITIONAL_LINKS}
)
