set(MAJOR 1)
set(MINOR 2)
set(REVISION 0)
set(NAME libxdg-basedir)
set(PACKAGE ${NAME}-${MAJOR}.${MINOR}.${REVISION})

if (CMAKE_CROSSCOMPILING)
    set (EXTRA_ARGS --host=${HOST})
    list (APPEND CROSS_ARGS ac_cv_func_malloc_0_nonnull=yes ac_cv_func_realloc_0_nonnull=yes)
endif ()

EXTERNALPROJECT_ADD(${NAME}
    URL http://github.com/devnev/${NAME}/archive/${PACKAGE}.tar.gz TIMEOUT 30
    INSTALL_DIR ${CMAKE_INSTALL_PREFIX}
    PATCH_COMMAND autoreconf --force --install
    CONFIGURE_COMMAND ${CROSS_ARGS}  <SOURCE_DIR>/configure ${EXTRA_ARGS} --disable-dependency-tracking --prefix=<INSTALL_DIR> --disable-shared
)
