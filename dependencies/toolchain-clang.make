set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)

# for libraries and headers in the target directories
set(CMAKE_FIND_ROOT_PATH ${CMAKE_PREFIX_PATH})
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

set(Libunwind_INCLUDE_DIR /usr/local/include CACHE PATH "")
set(Libunwind_LIBRARY /usr/local/lib/libunwind.a CACHE FILEPATH "")

# make sure lto is enabled for all release targets
# set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)