project('CCOSS-community', 'cpp', default_options:['cpp_std=c++14'])

inc_dirs = include_directories('Activities',
          'Entities',
          'GUI',
          'Managers',
          'Menus',
          'System',
          'external/linux/include',
          'external/linux/include/RakNet',
          'external/linux/include/fmod')

deps = [dependency('boost'),
  dependency('allegro'),
  dependency('loadpng'),
  dependency('flac'),
  dependency('luajit'),
  dependency('minizip'),
  dependency('threads'),
  dependency('liblz4'),
  dependency('libpng')
  ]

compiler = meson.get_compiler('cpp')

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

if meson.get_compiler('cpp').get_id() == 'gcc'
  link_args = '-lstdc++fs'
  extra_args = ['-pedantic','-fpermissive', '-Wno-write-strings', '-Wno-unknown-pragmas', '-g', '-Og']
  postfix = '.x86_64'
elif meson.get_compiler('cpp').get_id() == 'msvc'
  link_args = []
  extra_args = []
endif


executable('CCOSS_debug'+postfix, 'Main.cpp',sources, luabind_src, raknet_src,link_args: link_args,
            dependencies: [deps], include_directories:inc_dirs, objects:ext_libs,
            cpp_args:extra_args, cpp_pch:pch)

executable('CCOSS'+postfix, 'Main.cpp', sources, luabind_src, raknet_src, link_args: '-lstdc++fs',
            dependencies: deps, include_directories:inc_dirs, objects: ext_libs,
            cpp_args:['-pedantic', '-fpermissive', '-Wno-write-strings', '-Wno-unknown-pragmas', '-O2'], cpp_pch:pch,  build_by_default:false)
