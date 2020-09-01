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
subdir('Activities')
subdir('Entities')
subdir('GUI')
subdir('Managers')
subdir('Menus')
subdir('System')
subdir('external')


executable('CCOSS', 'Main.cpp',sources, luabind_src, raknet_src,link_args: '-lstdc++fs',dependencies: [deps,link_libs], include_directories:inc_dirs,
            override_options:['cpp_std=c++14'], cpp_args:['-pedantic','-fpermissive', '-Wno-return-type', '-Wno-write-strings', '-Wno-unknown-pragmas', '-Wno-unused-variable', '-g', '-Og'], cpp_pch:pch)
