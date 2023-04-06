project('RakNet', 'cpp')
subdir('src')

raknet_include = include_directories('.', 'include')

raknet = static_library('raknet', raknet_src, include_directories: raknet_include, cpp_args:['-w'], override_options:['cpp_std=c++03'])

raknet_dep = declare_dependency(link_with: raknet, include_directories: raknet_include)

meson.override_dependency('RakNet', raknet_dep)
