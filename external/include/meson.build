external_inc_dirs = [include_directories('common',
                                         'common/RakNet',
                                         'common/boost_1_75',
                                         'common/fmod',
                                         'common/luabind')]
if host_machine.system() in ['linux', 'darwin']
endif
if host_machine.system() == 'windows'
  external_inc_dirs += include_directories('win',
                                           'win/SDL2',
                                           'win/LZ4',
                                           'win/allegro')
endif
