find_package(Java 1.6 REQUIRED COMPONENTS Development)

add_custom_command(
OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/MultiMCLauncher.class
COMMAND ${Java_JAVAC_EXECUTABLE} ARGS -target 1.6 -source 1.6 "${CMAKE_CURRENT_SOURCE_DIR}/MultiMCLauncher.java" -d "${CMAKE_CURRENT_BINARY_DIR}"
DEPENDS MultiMCLauncher.java
WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
COMMENT "Generating the launcher header."
)

SET_SOURCE_FILES_PROPERTIES(${CMAKE_CURRENT_BINARY_DIR}/MultiMCLauncher.class GENERATED)

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/launcherdata.h
                   COMMAND wxinclude ARGS launcherdata.h MultiMCLauncher.class
                   DEPENDS wxinclude ${CMAKE_CURRENT_BINARY_DIR}/MultiMCLauncher.class
                   WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                   COMMENT "Generating the launcher header."
                   VERBATIM)

ADD_CUSTOM_TARGET(generate_launcher DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/launcherdata.h)