if host_machine.system() == 'linux'
  external_inc_dirs = include_directories('common',
                                         'common/RakNet',
                                         'common/boost_1_75',
                                         'common/fmod',
                                         'common/luabind')
endif
if host_machine.system() == 'windows'
  external_inc_dirs = include_directories('common',
                                          'win/GL',
                                          'win/LZ4',
                                          'common/RakNet',
                                          'win/allegro',
                                          'win/allegrogl',
                                          'common/boost_1_75/',
                                          'common/fmod',
                                          'common/luabind')
endif
