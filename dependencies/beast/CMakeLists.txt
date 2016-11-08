SET(NAME beast)

EXTERNALPROJECT_ADD(${NAME}
    DEPENDS boost
    GIT_REPOSITORY https://github.com/vinniefalco/Beast.git
    GIT_TAG origin/headers
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_DIR ${CMAKE_BINARY_DIR}/local
    INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_BINARY_DIR}/beast-prefix/src/beast/include/beast <INSTALL_DIR>/include/beast
)
