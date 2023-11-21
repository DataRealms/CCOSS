external_inc_dirs = [include_directories('common',
                                         'common/boost_1_75',
                                         'common/fmod',
                                         'common/thread-pool-3.5.0/include',
                                         'common/hopscotch-map-2.3.1/include',)]
if host_machine.system() in ['linux', 'darwin']
endif
if host_machine.system() == 'windows'
  external_inc_dirs += include_directories('win',
                                           'win/LZ4',)
endif
