cmake_minimum_required(VERSION 3.9)
enable_testing()

find_package(PkgConfig)

project(doozy LANGUAGES CXX)

set(PACKAGE "doozy")
set(PACKAGE_NAME "Doozy")
set(PACKAGE_VERSION_MAJOR 0)
set(PACKAGE_VERSION_MINOR 0)
set(PACKAGE_VERSION_PATCH 1)
set(PACKAGE_LOCALE_DIR ${CMAKE_INSTALL_PREFIX}/share/locale)

set(CPACK_PACKAGE_NAME ${PACKAGE})
set(CPACK_PACKAGE_VERSION_MAJOR ${PACKAGE_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${PACKAGE_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${PACKAGE_VERSION_PATCH})
set(CPACK_SOURCE_GENERATOR "TBZ2")
set(CPACK_PACKAGE_FILE_NAME $CPACK_PACKAGE_NAME)
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}-bin")
set(CPACK_SOURCE_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
set(CPACK_SOURCE_IGNORE_FILES "/build/;/debug/;/release/;/dist/;/gfx/;/include/;/lib/;/web/;/.git/;/server/;/renderer/;/controlpoint/;/tags;~$;${CPACK_SOURCE_IGNORE_FILES}")
include(CPack)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

message(STATUS "Host: ${CMAKE_HOST_SYSTEM}")
message(STATUS "Target: ${CMAKE_SYSTEM_NAME}")
message(STATUS "Cross compiling: ${CMAKE_CROSSCOMPILING}")
message(STATUS "Pkg-config root: $ENV{PKG_CONFIG_SYSROOT_DIR}")

option(DOOZY_RENDERER "Renderer device support" ON)
option(DOOZY_SERVER "Server device support" OFF)
option(DOOZY_CONTROLPOINT "Controlpoint device support" ON)
option(STATIC_BINARY "Build static binary" OFF)
option(SHARED_LIBRARY "Create single shared library containing all dependencies" OFF)
option(ENABLE_TESTS "build unit tests" ON)
option(LOCAL_DEPS "force usage of locally build dependencies" OFF)
option(SANITIZE_ADDRESS "Build with address sanitizer (detect invalid memory access and memory leaks)" OFF)
option(SANITIZE_UNDEFINED "Build with undefined behavior sanitizer (detect integer overflow, bad shifts, ...)" OFF)
option(USE_GOLD_LINKER "Use Gnu gold linker instead of GNU ld")

option(WITHOUT_CEC "Disable libcec support" OFF)

if (SANITIZE_ADDRESS)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=address")
endif ()

if (SANITIZE_UNDEFINED)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined -fsanitize=shift -fsanitize=return -fsanitize=null -fsanitize=signed-integer-overflow -fsanitize=integer-divide-by-zero -ftrapv")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=undefined -fsanitize=shift -fsanitize=return -fsanitize=null -fsanitize=signed-integer-overflow -fsanitize=integer-divide-by-zero -ftrapv")
endif ()


if (STATIC_BINARY)
    add_definitions("-DAL_LIBTYPE_STATIC")
endif ()

add_definitions("-DCOMPILE_DEBUG_LOG")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" AND APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
endif ()

if (USE_GOLD_LINKER)
    execute_process(COMMAND ${CMAKE_C_COMPILER} -fuse-ld=gold -Wl,--version ERROR_QUIET OUTPUT_VARIABLE LD_VERSION)
    if (NOT "${LD_VERSION}" MATCHES "GNU gold")
        message(FATAL_ERROR "GNU gold linker isn't available.")
    endif ()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fuse-ld=gold")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fuse-ld=gold")
    message(STATUS "Using GNU gold linker")
endif ()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wnon-virtual-dtor -Wfatal-errors")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")
#set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -flto")
#set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS} -flto")

set(CMAKE_XCODE_ATTRIBUTE_GCC_VERSION "com.apple.compilers.llvm.clang.1_0")
set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++14")
set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")

add_custom_target(uninstall COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/CMakeUninstall.cmake)

include_directories(${CMAKE_BINARY_DIR})

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})

add_subdirectory(modules/utils)
add_subdirectory(modules/audio)
add_subdirectory(modules/image)
add_subdirectory(modules/upnp)

if (DOOZY_RENDERER)
    add_subdirectory(renderer)
    list(APPEND LINK_LIBS doozyrenderer)
endif ()

if (DOOZY_CONTROLPOINT)
    add_subdirectory(controlpoint)
    list(APPEND LINK_LIBS doozycontrolpoint)
endif ()

if (DOOZY_SERVER)
    add_subdirectory(modules/sqlpp11)
    add_subdirectory(modules/sqlpp11-connector-sqlite3)
    add_subdirectory(server)
    list(APPEND LINK_LIBS doozyserver)
endif ()

if (MINGW)
    list (APPEND LINK_LIBS ws2_32)
endif ()

add_subdirectory(common)

add_executable(doozy
    main.cpp
    doozydevicefactory.h doozydevicefactory.cpp
    .travis.yml
)

target_link_libraries(doozy doozycommon ${LINK_LIBS})
install(TARGETS doozy RUNTIME DESTINATION bin)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/doozyconfig.h.in ${CMAKE_BINARY_DIR}/doozyconfig.h)
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/CMakeUninstall.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/CMakeUninstall.cmake" IMMEDIATE @ONLY)

if (SHARED_LIBRARY)
    add_library(doozylib SHARED ${DOOZY_SOURCES})
    target_link_libraries(doozylib "-Wl,--whole-archive" ${LINK_LIBS} "-Wl,--no-whole-archive")
endif ()

if (ENABLE_TESTS)
    add_subdirectory(test)
endif ()
