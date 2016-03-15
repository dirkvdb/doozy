project('doozy', 'cpp', default_options : ['cpp_std=c++14'], subproject_dir : 'modules')

utils_sub = subproject('utils')
upnp_sub = subproject('upnp')
image_sub = subproject('image')
audio_sub = subproject('audio')

inc = include_directories(meson.current_build_dir() + '/modules/audio')

doozy_files = files(
    'common/doozydeviceinterface.h',
    'common/doozydevicefactory.h', 'common/doozydevicefactory.cpp',
    'common/settings.h', 'common/settings.cpp',
    'common/devicedescriptions.h',
    'main.cpp'
)

config = configuration_data()
config.set('PACKAGE', 'doozy')
config.set('PACKAGE_NAME', 'Doozy')
config.set('PACKAGE_VERSION_MAJOR', 0)
config.set('PACKAGE_VERSION_MINOR', 0)
config.set('PACKAGE_VERSION_PATCH', 1)
config.set10('DOOZY_RENDERER', get_option('renderer'))

if get_option('renderer')
    subdir('renderer')
endif

configure_file(input : 'doozyconfigmeson.h.in', output : 'doozyconfig.h', configuration : config)

doozy_exec = executable('doozy',
                        doozy_files,
                        include_directories : inc,
                        link_with : doozyrenderer_lib,
                        dependencies : [utils_sub.get_variable('utils_dep'),
                                        upnp_sub.get_variable('upnp_dep'),
                                        image_sub.get_variable('image_dep'),
                                        audio_sub.get_variable('audio_dep')])
