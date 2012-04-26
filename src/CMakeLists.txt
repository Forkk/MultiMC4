cmake_minimum_required (VERSION 2.8)
project (MultiMC)

# Set the path where CMake will look for modules.
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}")


# Set the sources and headers variables.
file(GLOB_RECURSE SRCS "*.cpp")
MESSAGE(STATUS "** Found source files: ${SRCS}")

file(GLOB_RECURSE INCS "*.h")
MESSAGE(STATUS "** Found header files: ${INCS}")

# Find wxWidgets.
MESSAGE(STATUS "** Finding wxWidgets")
find_package(wxWidgets COMPONENTS core base REQUIRED)
MESSAGE(STATUS "** wxWidgets Root: ${wxWidgets_ROOT_DIR}")
MESSAGE(STATUS "** wxWidgets Configuration: ${wxWidgets_CONFIGURATION}")

# Find boost.
set(Boost_USE_STATIC_LIBS ON)
MESSAGE(STATUS "** Finding Boost...")
find_package(Boost 1.49.0 COMPONENTS system filesystem REQUIRED)
MESSAGE(STATUS "** Boost Include: ${Boost_INCLUDE_DIR}")
MESSAGE(STATUS "** Boost Libraries: ${Boost_LIBRARY_DIRS}")

# Include wxWidgets.
include("${wxWidgets_USE_FILE}")

# Include boost.
include_directories("${Boost_INCLUDE_DIRS}")

# Add the executable.
add_executable(MultiMC WIN32 ${SRCS} ${INCS})

# Link the libraries.
target_link_libraries(MultiMC
	${wxWidgets_LIBRARIES}
	${Boost_LIBRARIES}
)