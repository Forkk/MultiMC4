cmake_minimum_required (VERSION 2.8)
project (wxinclude)

# Find boost.
set(Boost_USE_STATIC_LIBS ON)
MESSAGE(STATUS "** Finding Boost...")
FIND_PACKAGE( Boost 1.46.0 COMPONENTS program_options filesystem system REQUIRED )
MESSAGE(STATUS "** Boost Include: ${Boost_INCLUDE_DIR}")
MESSAGE(STATUS "** Boost Libraries: ${Boost_LIBRARY_DIRS}")

IF(UNIX)
  # assume GCC, add C++0x/C++11 stuff
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
ELSEIF(MINGW)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++0x")
ENDIF()

# Include boost.
include_directories("${Boost_INCLUDE_DIRS}")

add_executable(wxinclude wxInclude.cpp)

# Link the libraries.
target_link_libraries(wxinclude ${Boost_LIBRARIES})
