project('CCOSS-community', 'cpp', default_options:['cpp_std=c++14'])

inc_dirs = include_directories('Activities',
          'Entities',
          'GUI',
          'Managers',
          'Menus',
          'System',
          'external/linux/include')

deps = [dependency('boost'),
  dependency('allegro-5'),
  dependency('flac')
  ]

pch = 'System/StandardIncludes.h'
subdir('Activities')
subdir('Entities')
subdir('GUI')
subdir('Managers')
subdir('Menus')
subdir('System')


executable('CCOSS', 'Main.cpp', act_src, ent_src, gui_src, man_src,
            menu_src, sys_src, dependencies: deps, include_directories:inc_dirs,
            override_options:['cpp_std=c++14'], cpp_args:['-fpermissive','-w', '-Wno-return-type', '-Wno-write-strings', '-Wno-unknown-pragmas', '-Wno-unused-variable'], cpp_pch:pch)
