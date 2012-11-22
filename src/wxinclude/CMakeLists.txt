cmake_minimum_required (VERSION 2.8)
project (wxinclude CXX)

IF(UNIX)
  # assume GCC, add C++0x/C++11 stuff
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
ELSEIF(MINGW)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++0x")
ENDIF()

add_executable(wxinclude wxInclude.cpp)
