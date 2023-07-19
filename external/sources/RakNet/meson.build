project('RakNet', 'cpp')
subdir('src')

raknet_include = include_directories('.', 'include')

cxx = meson.get_compiler('cpp')
cpp_options = []
deps = []

if cxx.get_argument_syntax() == 'gcc'
  cpp_args = ['-w', '-fpermissive']
  if host_machine.system() == 'windows' 
    cpp_args += ['-D_ITERATOR_DEBUG_LEVEL=0','-DWIN32','-D_RAKNET_LIB','-D_CRT_NONSTDC_NO_DEPRECATE','-D_CRT_SECURE_NO_DEPRECATE']
    if get_option('debug')
      cpp_args +=['-D_DEBUG']
    endif
  endif
elif cxx.get_argument_syntax() == 'msvc'
  cpp_args = ['-permissive', '-DBOOST_BIND_GLOBAL_PLACEHOLDERS=1', '-DLUABIND_BUILDING', '-D_ITERATOR_DEBUG_LEVEL=0']
  cpp_options = ['cpp_std=vc++20']
endif

if not get_option('use_prebuilt_libraries') or host_machine.system() in ['darwin', 'linux']
  raknet = static_library('raknet', raknet_src, include_directories: raknet_include, cpp_args: cpp_args, override_options: ['warning_level=0']+cpp_options)
else
  raknet = []
  if get_option('debug')
    deps = cxx.find_library('raknet-debug', dirs: meson.current_source_dir()/'_Bin')
  else
    deps = cxx.find_library('raknet-release', dirs: meson.current_source_dir()/'_Bin')
  endif
endif
raknet_dep = declare_dependency(link_with: raknet, dependencies: deps, include_directories: raknet_include)

meson.override_dependency('RakNet', raknet_dep)