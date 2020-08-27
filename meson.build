project('CCOSS-community', 'cpp')

src_dirs = include_directories('.')

deps = [dependency('boost'),
  dependency('allegro-5'),
  dependency('flac')
  ]

subdir('Activities')
subdir('Entities')
subdir('GUI')
subdir('Managers')
subdir('Menus')
subdir('System')

executable('CCOSS', 'Main.cpp', act_src, ent_src, gui_src, man_src,
            menu_src, sys_src, dependencies: deps) #, include_directories:src_dirs)
