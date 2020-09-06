project('CCOSS-community', ['cpp','c'], default_options:['cpp_std=c++14'])

source_inc_dirs = include_directories('Activities',
          'Entities',
          'GUI',
          'Managers',
          'Menus',
          'System')

deps = [dependency('boost'),
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
  link_args = ['-lstdc++fs', '-ldl']
  extra_args = ['-pedantic','-fpermissive', '-Wno-write-strings', '-Wno-unknown-pragmas']
  debug_args = ['-g', '-Og']
  release_args = ['-O2']
  suffix = 'x86_64'
elif meson.get_compiler('cpp').get_id() == 'msvc'
  link_args = []
  extra_args = []
endif

pch = 'System/StandardIncludes.h'
sources = []
luabind_src = []
raknet_src = []
subdir('Activities')
subdir('Entities')
subdir('GUI')
subdir('Managers')
subdir('Menus')
subdir('System')
subdir('external')

executable('CCCP_debug', 'Main.cpp',sources, link_args: link_args, link_with: [luabind, raknet],
            dependencies: [deps], include_directories:[source_inc_dirs, external_inc_dirs], objects:ext_libs,
            cpp_args:extra_args, cpp_pch:pch, name_suffix:suffix, override_options:['buildtype=debugoptimized', 'optimization=g'])

executable('CCCP', 'Main.cpp', sources, link_args: '-lstdc++fs', link_with: [luabind,raknet],
            dependencies: [deps], include_directories:[source_inc_dirs, external_inc_dirs], objects: ext_libs,
            cpp_args:extra_args, cpp_pch:pch, name_suffix:suffix, build_by_default:false, override_options:['buildtype=release', 'optimization=3'])
