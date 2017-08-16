set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)

set(CMAKE_EXE_LINKER_FLAGS "-L/usr/local/opt/llvm/lib" CACHE STRING "")
set(CMAKE_SHARED_LINKER_FLAGS "-L/usr/local/opt/llvm/lib" CACHE STRING "")
set(CMAKE_MODULE_LINKER_FLAGS "-L/usr/local/opt/llvm/lib" CACHE STRING "")

# for libraries and headers in the target directories
set(CMAKE_FIND_ROOT_PATH ${CMAKE_PREFIX_PATH})
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# make sure lto is enabled for all release targets
# set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)