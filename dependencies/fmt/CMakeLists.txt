set(MAJOR 4)
set(MINOR 0)
set(REVISION 0)
set(NAME fmt)
set(VERSION ${MAJOR}.${MINOR}.${REVISION})
set(PACKAGE ${NAME}-${VERSION})

externalproject_add(${NAME}
    URL https://github.com/fmtlib/${NAME}/releases/download/${VERSION}/${PACKAGE}.zip TIMEOUT 180
    INSTALL_DIR ${CMAKE_INSTALL_PREFIX}
    CMAKE_ARGS -G ${CMAKE_GENERATOR}
               -DFMT_TEST=OFF
               -DFMT_DOC=OFF
               -DFMT_INSTALL=ON
               -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
               -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
               -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
               -DCMAKE_POSITION_INDEPENDENT_CODE=TRUE
               -DCMAKE_POLICY_DEFAULT_CMP0063=NEW
               -DCMAKE_CXX_VISIBILITY_PRESET=hidden
               -DCMAKE_POLICY_DEFAULT_CMP0069=NEW
               ${POSTFIX_CONFIG}
)
