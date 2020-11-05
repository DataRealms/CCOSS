project('CCOSS-community', ['cpp','c'], default_options:['cpp_std=c++17'])

source_inc_dirs = include_directories('Activities',
          'Entities',
          'GUI',
          'Managers',
          'Menus',
          'System')

deps = [
  dependency('allegro'),
  dependency('loadpng'),
  dependency('flac'),
  dependency('luajit'),
  dependency('minizip'),
  dependency('threads'),
  dependency('liblz4'),
  dependency('libpng'),
  dependency('boost'), #needed for luabind
  dependency('x11')
  ]

compiler = meson.get_compiler('cpp')


if meson.get_compiler('cpp').get_id() == 'gcc'
  extra_args = ['-pedantic','-fpermissive', '-Wno-write-strings', '-Wno-unknown-pragmas']
  suffix = 'x86_64'
elif meson.get_compiler('cpp').get_id() == 'msvc'
  extra_args = []
endif

pch = 'System/StandardIncludes.h'
sources = []
subdir('Activities')
subdir('Entities')
subdir('GUI')
subdir('Managers')
subdir('Menus')
subdir('System')
subdir('Resources')
subdir('external')

debug = executable('CCCP_debug', 'Main.cpp',sources, link_with: [luabind_debug, raknet_debug, fmod],
            dependencies: [deps], include_directories:[source_inc_dirs, external_inc_dirs],
            cpp_args:extra_args, cpp_pch:pch, name_suffix:suffix, override_options:['buildtype=debugoptimized', 'optimization=g'])

release = executable('CCCP', 'Main.cpp', sources, link_with: [luabind, raknet, fmod],
            dependencies: [deps], include_directories:[source_inc_dirs, external_inc_dirs],
            cpp_args:extra_args, cpp_pch:pch, name_suffix:suffix, build_by_default:false, override_options:['buildtype=release', 'optimization=3', 'debug=false'])

#AppImage Targeting
wget = find_program('wget')

linuxdeploy_url = 'https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage'

linuxdeploy = custom_target('linuxdeploy', build_by_default:false, output:'linuxdeploy-x86_64.AppImage', command:[find_program('env'), wget, '-O', '@OUTPUT@', linuxdeploy_url, '&&', find_program('chmod'), '+x', '@OUTPUT@'])
chmod = find_program('chmod')
outname = 'cccp-x86_64.AppImage'

custom_target('appimage', depends:[release, linuxdeploy], output:outname , command:[find_program('env'), 'OUTPUT='+outname, linuxdeploy.full_path(), '--appdir=@PRIVATE_DIR@', '-e', release, '-l', fmod, '-l', fmod.full_path()+'.11', '-d', desktop_file, '-i', icon_file, '-oappimage'])
