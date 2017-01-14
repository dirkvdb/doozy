SET(CMAKE_SYSTEM_NAME Linux)
SET(CMAKE_SYSTEM_VERSION 1)
SET(CMAKE_SYSTEM_PROCESSOR armv7)

SET(LIBTYPE STATIC)

SET(CMAKE_C_COMPILER armv7-rpi2-linux-gnueabihf-gcc)
SET(CMAKE_CXX_COMPILER armv7-rpi2-linux-gnueabihf-g++)

SET(CMAKE_C_FLAGS "-mcpu=cortex-a7 -mtune=cortex-a7 -mfpu=neon-vfpv4 -mvectorize-with-neon-quad -mfloat-abi=hard -O3" CACHE STRING "" FORCE)
SET(CMAKE_CXX_FLAGS "-mcpu=cortex-a7 -mtune=cortex-a7 -mfpu=neon-vfpv4 -mvectorize-with-neon-quad -mfloat-abi=hard -O3 -std=c++14" CACHE STRING "" FORCE)

SET(CMAKE_SYSROOT /opt/armv7-rpi2-linux-gnueabihf/armv7-rpi2-linux-gnueabihf/sysroot)

# search for programs in the build host directories
SET(CMAKE_FIND_ROOT_PATH ${CMAKE_PREFIX_PATH})
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# for libraries and headers in the target directories
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
