SET(CMAKE_CROSSCOMPILING false)

# which compilers to use for C and C++
SET(CMAKE_C_COMPILER clang)
SET(CMAKE_CXX_COMPILER clang++)

# here is the target environment located
SET(CMAKE_FIND_ROOT_PATH /usr)
SET(wxWidgets_ROOT_DIR /usr)

# adjust the default behaviour of the FIND_XXX() commands:
# search headers and libraries in the target environment, search 
# programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
