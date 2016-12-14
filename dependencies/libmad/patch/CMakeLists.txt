cmake_minimum_required(VERSION 2.8)
cmake_policy(VERSION 2.8)

project(libmad)
include(GNUInstallDirs)
include(CheckTypeSize)
include(CheckFunctionExists)

if (CMAKE_COMPILER_IS_GNUCC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
endif ()

set(PACKAGE "libmad")
set(PACKAGE_NAME "MPEG Audio Decoder")
set(PACKAGE_VERSION "0.15.1b")
set(PACKAGE_STRING "MPEG Audio Decoder 0.15.1b")
set(PACKAGE_TARNAME "libmad")
set(PACKAGE_BUGREPORT "support@underbit.com")

option(OPT_ACCURACY "Define to optimize for accuracy over speed." OFF)
option(OPT_SPEED "Define to optimize for speed over accuracy." OFF)
option(OPT_SSO "Define to enable a fast subband synthesis approximation optimization." OFF)
option(OPT_STRICT "Define to influence a strict interpretation of the ISO/IEC standards, even if this is in opposition with best accepted practices." OFF)

set(FPM "DEFAULT" CACHE STRING "fixed point math setting (DEFAULT|INTEL|64BIT|FLOAT|ARM|MIPS|SPARC|PPC)")

if (FPM STREQUAL "DEFAULT")
    message(WARNING "default fixed-point math will yield limited accuracy")
endif ()

add_definitions("-DFPM_${FPM}")

check_include_file(assert.h HAVE_ASSERT_H)
check_include_file(dlcfn.h HAVE_DLFCN_H)
check_include_file(fcntl.h HAVE_FCNTL_H)
check_include_file(inttypes.h HAVE_INTTYPES_H)
check_include_file(limits.h HAVE_LIMITS_H)
check_include_file(memory.h HAVE_MEMORY_H)
check_include_file(stdint.h HAVE_STDINT_H)
check_include_file(stdlib.h HAVE_STDLIB_H)
check_include_file(strings.h HAVE_STRINGS_H)
check_include_file(string.h HAVE_STRING_H)
check_include_file(sys/stat.h HAVE_SYS_STAT_H)
check_include_file(sys/types.h HAVE_SYS_TYPES_H)
check_include_file(sys/wait.h HAVE_SYS_WAIT_H)
check_include_file(unistd.h HAVE_UNISTD_H)

set(CMAKE_EXTRA_INCLUDE_FILES fcntl.h)
check_function_exists(fcntl HAVE_FCNTL)
set(CMAKE_EXTRA_INCLUDE_FILES unistd.h)
check_function_exists(fork HAVE_FORK)
check_function_exists(pipe HAVE_PIPE)
set(CMAKE_EXTRA_INCLUDE_FILES sys/wait.h)
check_function_exists(waitpid HAVE_WAITPID)
set(CMAKE_EXTRA_INCLUDE_FILES)

check_type_size(int SIZEOF_INT)
check_type_size(long SIZEOF_LONG)
check_type_size("long long" SIZEOF_LONG_LONG)
set(STDC_HEADERS ON)

configure_file(config.h.in ${CMAKE_BINARY_DIR}/config.h)
configure_file(mad.pc.in ${CMAKE_BINARY_DIR}/mad.pc @ONLY)

set(MAD_INLCUDES mad.h)

set(DATA_INCLUDES
    D.dat
    imdct_s.dat
    qc_table.dat
    rq_table.dat
    sf_table.dat
)

add_library(mad
    bit.c
    decoder.c
    frame.c
    huffman.c
    layer12.c
    layer3.c
    fixed.c
    stream.c
    synth.c
    timer.c
    version.c
    ${DATA_INCLUDES}
)

if (UNIX)
    add_executable(minimad
        minimad.c
    )

    target_link_libraries(minimad mad)
endif ()

if (BUILD_SHARED_LIBS)
    set(MAD_SOVERSION_CURRENT 2)
    set(MAD_SOVERSION_REVISION 1)
    set(MAD_SOVERSION_AGE 2)

    set_target_properties(mad PROPERTIES
        PREFIX ""
        VERSION ${MAD_SOVERSION_CURRENT}.${MAD_SOVERSION_REVISION}.${MAD_SOVERSION_AGE}
        SOVERSION ${MAD_SOVERSION_CURRENT}
        PUBLIC_HEADER "${MAD_INLCUDES}"
        MACOSX_RPATH TRUE
    )
endif ()

install(TARGETS mad
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(FILES ${MAD_INLCUDES} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(FILES ${CMAKE_BINARY_DIR}/mad.pc DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")