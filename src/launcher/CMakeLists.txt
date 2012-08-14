cmake_minimum_required(VERSION 2.8.6)
find_package(Java 1.6 REQUIRED COMPONENTS Development)
project(launcher Java)
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}")
include(UseJava)
set(CMAKE_JAVA_JAR_ENTRY_POINT MultiMCLauncher)
set(CMAKE_JAVA_COMPILE_FLAGS -target 1.6 -source 1.6)

if (WIN32 AND CMAKE_CROSSCOMPILING)
	string(REPLACE ";" ":" CMAKE_JAVA_COMPILE_FLAGS CMAKE_JAVA_COMPILE_FLAGS)
endif ()


set(SRC
    MultiMCLauncher.java
    AppletWrap.java
    MCFrame.java
)

add_jar(MultiMCLauncher ${SRC})

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/launcherdata.h
                   COMMAND wxinclude ARGS launcherdata.h MultiMCLauncher.jar
                   DEPENDS wxinclude ${CMAKE_CURRENT_BINARY_DIR}/MultiMCLauncher.jar
                   WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                   COMMENT "Generating the launcher header."
                   VERBATIM)

ADD_CUSTOM_TARGET(generate_launcher DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/launcherdata.h)