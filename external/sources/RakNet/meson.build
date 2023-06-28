project('RakNet', 'cpp')
subdir('src')

raknet_include = include_directories('.', 'include')

cxx = meson.get_compiler('cpp')
cpp_options = []

if cxx.get_argument_syntax() == 'gcc'
  cpp_args = ['-w', '-fpermissive']
elif cxx.get_argument_syntax() == 'msvc'
  cpp_args = ['-permissive', '-DBOOST_BIND_GLOBAL_PLACEHOLDERS=1', '-DLUABIND_BUILDING', '-D_ITERATOR_DEBUG_LEVEL=0']
  cpp_options = ['cpp_std=vc++20']
endif

raknet = static_library('raknet', raknet_src, include_directories: raknet_include, cpp_args: cpp_args, override_options: ['warning_level=0']+cpp_options)

raknet_dep = declare_dependency(link_with: raknet, include_directories: raknet_include)

meson.override_dependency('RakNet', raknet_dep)