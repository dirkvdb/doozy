set(CMAKE_SYSTEM_NAME Windows)
set(HOST x86_64-w64-mingw32)

set(CROSS_COMPILE x86_64-w64-mingw32-)
set(CMAKE_C_COMPILER ${CROSS_COMPILE}gcc)
set(CMAKE_CXX_COMPILER ${CROSS_COMPILE}g++)
set(CMAKE_RC_COMPILER ${CROSS_COMPILE}windres)

set(CMAKE_EXE_LINKER_FLAGS "-static -static-libgcc -static-libstdc++" CACHE STRING "")
set(CMAKE_SHARED_LINKER_FLAGS "-static -static-libgcc -static-libstdc++" CACHE STRING "")
set(CMAKE_MODULE_LINKER_FLAGS "-static -static-libgcc -static-libstdc++" CACHE STRING "")

set(CMAKE_FIND_ROOT_PATH ${CMAKE_PREFIX_PATH})
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)