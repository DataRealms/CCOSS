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
  dependency('allegro-5'),
  dependency('flac'),
  dependency('luajit')
  ]

pch = 'System/StandardIncludes.h'
sources = []
subdir('Activities')
subdir('Entities')
subdir('GUI')
subdir('Managers')
subdir('Menus')
subdir('System')
subdir('external/sources')


executable('CCOSS', 'Main.cpp',sources, luabind_src, raknet_src, dependencies: deps, include_directories:inc_dirs,
            override_options:['cpp_std=c++14'], cpp_args:['-fpermissive','-w', '-Wno-return-type', '-Wno-write-strings', '-Wno-unknown-pragmas', '-Wno-unused-variable'], cpp_pch:pch)
