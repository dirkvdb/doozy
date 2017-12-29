set(MAJOR 1)
set(MINOR 3)
set(REVISION 0)
set(NAME benchmark)

externalproject_add(${NAME}
    PREFIX ${NAME}
    URL https://github.com/google/${NAME}/archive/v${MAJOR}.${MINOR}.${REVISION}.tar.gz TIMEOUT 30
    INSTALL_DIR ${CMAKE_INSTALL_PREFIX}
    CMAKE_ARGS -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
               -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
               -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
               -DBENCHMARK_ENABLE_TESTING=NO
               ${POSTFIX_CONFIG}
)