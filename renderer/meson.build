cec_dep = dependency('libcec', required : false)

inc = include_directories('..', meson.current_build_dir() + '/../modules/audio')

renderer_files = [
    'devicedescriptions.h',
    'renderer.h', 'renderer.cpp',
    'renderersettings.h', 'renderersettings.cpp',
    'mediarendererdevice.h', 'mediarendererdevice.cpp',
    'playqueue.h', 'playqueue.cpp',
    'typeconversions.h'
]

doozyrenderer_lib = static_library('doozyrenderer',
                                   files(renderer_files),
                                   include_directories : inc,
                                   dependencies : [utils_sub.get_variable('utils_dep'),
                                                   upnp_sub.get_variable('upnp_dep'),
                                                   upnp_sub.get_variable('libupnp_dep'),
                                                   image_sub.get_variable('image_dep'),
                                                   audio_sub.get_variable('audio_dep')])
