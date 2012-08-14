cmake_minimum_required(VERSION 2.8.6)
find_package(Java 1.6 REQUIRED COMPONENTS Development)
include(UseJava)
project(launcher Java)

set(CMAKE_JAVA_COMPILE_FLAGS -target 1.6 -source 1.6)

set(SRC
    MultiMCLauncher.java
)
set(CMAKE_JAVA_JAR_ENTRY_POINT MultiMCLauncher)
add_jar(MultiMCLauncher ${SRC})


#add_custom_command(
#OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/MultiMCLauncher.class
#COMMAND ${Java_JAVAC_EXECUTABLE} ARGS -target 1.6 -source 1.6 "${CMAKE_CURRENT_SOURCE_DIR}/MultiMCLauncher.java" -d "${CMAKE_CURRENT_BINARY_DIR}"
#DEPENDS MultiMCLauncher.java
#WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
#COMMENT "Generating the launcher header."
#)

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/launcherdata.h
                   COMMAND wxinclude ARGS launcherdata.h MultiMCLauncher.jar
                   DEPENDS wxinclude MultiMCLauncher
                   WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                   COMMENT "Generating the launcher header."
                   VERBATIM)

ADD_CUSTOM_TARGET(generate_launcher DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/launcherdata.h)