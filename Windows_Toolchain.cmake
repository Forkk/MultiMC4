# the name of the target operating system
SET(CMAKE_SYSTEM_NAME Windows)
SET(CMAKE_CROSSCOMPILING TRUE)

SET(MINGW_TYPE i686-w64-mingw32)

# which compilers to use for C and C++
SET(CMAKE_C_COMPILER ${MINGW_TYPE}-gcc)
SET(CMAKE_CXX_COMPILER ${MINGW_TYPE}-g++)
SET(CMAKE_RC_COMPILER ${MINGW_TYPE}-windres)

# here is the target environment located
SET(CMAKE_FIND_ROOT_PATH /usr/${MINGW_TYPE})
SET(wxWidgets_ROOT_DIR /usr/${MINGW_TYPE})

# adjust the default behaviour of the FIND_XXX() commands:
# search headers and libraries in the target environment, search 
# programs in the host environment
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
