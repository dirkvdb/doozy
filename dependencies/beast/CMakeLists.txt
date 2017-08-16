SET(NAME beast)

EXTERNALPROJECT_ADD(${NAME}
    DEPENDS boost
    GIT_REPOSITORY https://github.com/boostorg/beast.git
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_DIR ${CMAKE_INSTALL_PREFIX}
    INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR>/include/boost <INSTALL_DIR>/include/boost
)
