SET(CMAKE_SYSTEM_NAME Linux)
SET(CMAKE_SYSTEM_VERSION 1)

SET(LIBTYPE STATIC)
SET(HOST arm-unknown-linux-musleabi)
SET(CROSS arm-unknown-linux-musleabi-)

SET(CMAKE_C_COMPILER ${CROSS}gcc)
SET(CMAKE_CXX_COMPILER ${CROSS}g++)

SET(CMAKE_C_FLAGS "-mcpu=cortex-a7 -mtune=cortex-a7 -mfpu=neon-vfpv4 -mvectorize-with-neon-quad -mfloat-abi=hard -O3" CACHE STRING "" FORCE)
SET(CMAKE_CXX_FLAGS "-mcpu=cortex-a7 -mtune=cortex-a7 -mfpu=neon-vfpv4 -mvectorize-with-neon-quad -mfloat-abi=hard -O3" CACHE STRING "" FORCE)
SET(CMAKE_EXE_LINKER_FLAGS "-static" CACHE STRING "" FORCE)

# search for programs in the build host directories
SET(CMAKE_FIND_ROOT_PATH ${CMAKE_PREFIX_PATH})
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# for libraries and headers in the target directories
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)