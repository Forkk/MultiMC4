cmake_minimum_required (VERSION 2.8)
project (MultiMC CXX C)

# Stupid FindOpenSSL fix...
SET(CMAKE_CROSS_COMPILING ${CMAKE_CROSSCOMPILING})

OPTION(MultiMC_STATIC "Do a static build." OFF)

# Static link!
IF (MultiMC_STATIC)
	IF(WIN32)
		SET(CMAKE_FIND_LIBRARY_SUFFIXES .lib .a ${CMAKE_FIND_LIBRARY_SUFFIXES})
	ELSE()
		SET(CMAKE_FIND_LIBRARY_SUFFIXES .a ${CMAKE_FIND_LIBRARY_SUFFIXES})
	ENDIF()
ENDIF()

IF (DEFINED CMAKE_TOOLCHAIN_FILE)
	include("${CMAKE_TOOLCHAIN_FILE}")
ENDIF ()

#Check for machine endianness
INCLUDE(TestBigEndian)
TEST_BIG_ENDIAN(BIGENDIAN)
IF(${BIGENDIAN})
     ADD_DEFINITIONS(-DMULTIMC_BIG_ENDIAN)
ENDIF(${BIGENDIAN})

# Use or don't use SSL
OPTION(MultiMC_USE_HTTPS "Use SSL for minecraft login." ON)
IF (MultiMC_USE_HTTPS)
	MESSAGE(STATUS "** Using HTTPS for login.")
	SET(MultiMC_LINK_OPENSSL true)
ELSE ()
	MESSAGE(STATUS "** Not using HTTPS for login.")
	SET(MultiMC_USE_HTTPS false)
ENDIF ()

IF (MSVC)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc /D MSVC")
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /D CURL_STATICLIB /GR")
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D CURL_STATICLIB /GR")
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
set(MultiMC_VERSION_MINOR		2)
set(MultiMC_VERSION_REV			1)

IF (DEFINED MultiMC_VERSION_BUILD)
	message(STATUS "MultiMC build #${MultiMC_VERSION_BUILD}")
ELSE ()
	set(MultiMC_VERSION_BUILD	0)
	message(STATUS "Build number not specified. Defaulted to 0.")
ENDIF ()

IF (DEFINED MultiMC_BUILD_TAG)
	message(STATUS "Build tag: ${MultiMC_BUILD_TAG}")
ELSE ()
	message(STATUS "No build tag specified.")
ENDIF ()

if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
	set (MultiMC_ARCH "x64"
		CACHE STRING "Architecture we're building for.")
else()
	set (MultiMC_ARCH "x86"
		CACHE STRING "Architecture we're building for.")
endif()
message (STATUS "Architecture is ${MultiMC_ARCH}")

SET(MultiMC_Extra_Label "")

IF (WIN32)
	SET(MultiMC_JOB_NAME "MultiMC4Windows" CACHE STRING "Jenkins job name.")
ELSEIF(UNIX AND APPLE)
	SET(MultiMC_JOB_NAME "MultiMC4OSX" CACHE STRING "Jenkins job name.")
	# This is here because the scheme doesn't exactly apply to every kind of build...
	SET(MultiMC_Extra_Label ",label=osx")
ELSE()
	SET(MultiMC_JOB_NAME "MultiMC4Linux" CACHE STRING "Jenkins job name.")
ENDIF()

SET(MultiMC_JOB_URL "http://ci.forkk.net/job/${MultiMC_JOB_NAME}/arch=${MultiMC_ARCH}${MultiMC_Extra_Label}/"
    CACHE STRING "URL of the jenkins job to pull updates from.")
message(STATUS "Job URL: ${MultiMC_JOB_URL}")

configure_file("${PROJECT_SOURCE_DIR}/config.h.in"
			   "${PROJECT_BINARY_DIR}/config.h")

IF (CMAKE_CROSSCOMPILING)
	set(CMAKE_EXE_LINK_DYNAMIC_C_FLAGS "-Wl,-Bstatic -static-libgcc -static-libstdc++")
	set(CMAKE_EXE_LINK_DYNAMIC_CXX_FLAGS "-Wl,-Bstatic -static-libgcc -static-libstdc++")
ENDIF ()

IF (MultiMC_STATIC_LIBGCC)
	MESSAGE(STATUS "** Static linking libgcc and libstdc++")
	set(CMAKE_EXE_LINK_DYNAMIC_C_FLAGS "-Wl,-Bstatic -static-libgcc -static-libstdc++")
	set(CMAKE_EXE_LINK_DYNAMIC_CXX_FLAGS "-Wl,-Bstatic -static-libgcc -static-libstdc++")
ENDIF ()

# Set the sources and headers variables.
set(SRCS 
multimc.cpp
version.cpp
buildtag.cpp

gui/mainwindow.cpp
gui/settingsdialog.cpp
gui/logindialog.cpp
gui/consolewindow.cpp
gui/modeditwindow.cpp
gui/changeicondialog.cpp
gui/exportinstwizard.cpp
gui/importpackwizard.cpp
gui/downgradedialog.cpp
gui/instancectrl.cpp
gui/aboutdlg.cpp
gui/updatepromptdlg.cpp
gui/newinstancedlg.cpp
gui/taskprogressdialog.cpp
gui/minecraftversiondialog.cpp
gui/installforgedialog.cpp
gui/savemgrwindow.cpp
gui/textdisplaydialog.cpp
gui/advancedmsgdlg.cpp
gui/listselectdialog.cpp
gui/lwjgldialog.cpp
gui/ftbselectdialog.cpp
gui/shadedtextedit.cpp

data/appsettings.cpp
data/instance.cpp
data/mcprocess.cpp
data/userinfo.cpp
data/mod.cpp
data/modlist.cpp
data/configpack.cpp
data/mcversionlist.cpp
data/lwjglversionlist.cpp
data/worldlist.cpp
data/world.cpp
data/texturepack.cpp
data/texturepacklist.cpp
data/stdinstance.cpp
data/instancemodel.cpp
data/forgeversions.cpp
data/insticonlist.cpp

tasks/task.cpp
tasks/logintask.cpp
tasks/moddertask.cpp
tasks/gameupdatetask.cpp
tasks/checkupdatetask.cpp
tasks/filedownloadtask.cpp
tasks/filecopytask.cpp
tasks/exportpacktask.cpp
tasks/downgradetask.cpp
tasks/ziptask.cpp
tasks/pastebintask.cpp
tasks/lambdatask.cpp
tasks/lwjglinstalltask.cpp

java/javautils.cpp
java/annotations.cpp

utils/apputils.cpp
utils/curlutils.cpp
utils/datautils.cpp
utils/fsutils.cpp
utils/fsutils_secure.cpp
utils/httputils.cpp
utils/langutils.cpp
)

set (INCS
multimc.h
multimc_pragma.h
version.h
buildtag.h

gui/mainwindow.h
gui/settingsdialog.h
gui/logindialog.h
gui/consolewindow.h
gui/modeditwindow.h
gui/changeicondialog.h
gui/exportinstwizard.h
gui/importpackwizard.h
gui/downgradedialog.h
gui/instancectrl.h
gui/aboutdlg.h
gui/updatepromptdlg.h
gui/newinstancedlg.h
gui/taskprogressdialog.h
gui/minecraftversiondialog.h
gui/installforgedialog.h
gui/savemgrwindow.h
gui/textdisplaydialog.h
gui/advancedmsgdlg.h
gui/listselectdialog.h
gui/lwjgldialog.h
gui/ftbselectdialog.h
gui/shadedtextedit.h

data/appsettings.h
data/instance.h
data/userinfo.h
data/mod.h
data/modlist.h
data/configpack.h
data/mcversionlist.h
data/lwjglversionlist.h
data/worldlist.h
data/world.h
data/texturepack.h
data/texturepacklist.h
data/stdinstance.h
data/instancemodel.h
data/forgeversions.h
data/insticonlist.h

tasks/task.h
tasks/logintask.h
tasks/moddertask.h
tasks/gameupdatetask.h
tasks/checkupdatetask.h
tasks/filedownloadtask.h
tasks/filecopytask.h
tasks/exportpacktask.h
tasks/downgradetask.h
tasks/ziptask.h
tasks/pastebintask.h
tasks/lambdatask.h
tasks/lwjglinstalltask.h

java/annotations.h
java/classfile.h
java/constants.h
java/endian.h
java/errors.h
java/javautils.h
java/membuffer.h

utils/apputils.h
utils/curlutils.h
utils/osutils.h
utils/datautils.h
utils/fsutils.h
utils/httputils.h
utils/langutils.h

${CMAKE_BINARY_DIR}/resources/insticons.h
${CMAKE_BINARY_DIR}/resources/toolbaricons.h
${CMAKE_BINARY_DIR}/launcher/launcherdata.h
)

SET_SOURCE_FILES_PROPERTIES(${CMAKE_BINARY_DIR}/resources/insticons.h ${CMAKE_BINARY_DIR}/resources/toolbaricons.h GENERATED)
SET_SOURCE_FILES_PROPERTIES(${CMAKE_BINARY_DIR}/launcher/launcherdata.h GENERATED)
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

# Find OpenSSL
if (MultiMC_LINK_OPENSSL)
	find_package(OpenSSL REQUIRED)
endif ()

# Include wxWidgets.
include("${wxWidgets_USE_FILE}")

if(WIN32)
  set(SRCS ${SRCS} multimc.rc)
endif(WIN32)

# First, include header overrides
include_directories("${PROJECT_SOURCE_DIR}/hacks")

# Include CURL
include_directories("${CURL_INCLUDE_DIR}")

# Include boost.
include_directories("${Boost_INCLUDE_DIRS}")

# Make the static md5 lib build
add_subdirectory(md5)

# same for the custom bsdiff/bzip2 lib
add_subdirectory(patchlib)
include_directories(patchlib)

if (NOT CMAKE_CROSSCOMPILING)
	# Build the wxinclude generator
	add_subdirectory(wxinclude)
else ()
	SET(IMPORT_EXECUTABLES "IMPORTFILE-NOTFOUND" CACHE FILEPATH "Point it to the export file from a native build of wxInclude")
	INCLUDE(${IMPORT_EXECUTABLES})
endif ()

# Create header files out of resources
add_subdirectory(resources)
add_subdirectory(launcher)

# Compile localizations.
add_subdirectory(lang)

include_directories("${PROJECT_BINARY_DIR}")

include_directories(
"${PROJECT_SOURCE_DIR}"
"${PROJECT_SOURCE_DIR}/data"
"${PROJECT_SOURCE_DIR}/gui"
"${PROJECT_SOURCE_DIR}/tasks"
)

# Do not generate manifests for the binary. we have our own...
if (MSVC)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /MANIFEST:NO")
endif()

# Add the executable.
add_executable(MultiMC WIN32 ${SRCS} ${INCS})
add_dependencies(MultiMC generate_headers generate_launcher generate_langheader)

# Link the libraries.
target_link_libraries(MultiMC
	${wxWidgets_LIBRARIES}
	${Boost_LIBRARIES}
	${CURL_LIBRARIES}
	${OPENSSL_LIBRARIES}
	tinymd5
	patchlib
	${MULTIMC_ADDITIONAL_LINKS}
)

OPTION(MultiMC_Build_Annotation_Tool "Build the simple java class annotation dump tool." OFF)
IF(MultiMC_Build_Annotation_Tool)
# Set the sources and headers variables.
set(CLASSTEST_SRCS 
java/test.cpp
java/annotations.cpp
)

set (CLASSTEST_INCS
java/annotations.h
java/classfile.h
java/constants.h
java/endian.h
java/errors.h
java/membuffer.h
)
add_executable(classtest ${CLASSTEST_SRCS} ${CLASSTEST_INCS})
ENDIF()

if (NOT CMAKE_CROSSCOMPILING)
	EXPORT(TARGETS wxinclude FILE ${CMAKE_BINARY_DIR}/ImportExecutables.cmake)
endif ()
