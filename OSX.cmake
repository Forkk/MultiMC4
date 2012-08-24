SET(CMAKE_CROSSCOMPILING false)

# which compilers to use for C and C++
SET(CMAKE_C_COMPILER /opt/local/bin/gcc-mp-4.6)
SET(CMAKE_CXX_COMPILER /opt/local/bin/g++-mp-4.6)

# here is the target environment located
SET(CMAKE_FIND_ROOT_PATH /usr)
SET(wxWidgets_ROOT_DIR /usr/local)

# adjust the default behaviour of the FIND_XXX() commands:
# search headers and libraries in the target environment, search 
# programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
