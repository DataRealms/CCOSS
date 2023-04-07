project('luabind', 'cpp')
subdir('src')

luabind_include = include_directories('.', 'luabind')

luabind_args = ['-D_HAS_AUTO_PTR_ETC=1', '-D_LIBCPP_ENABLE_CXX17_REMOVED_FEATURES']

luabind_dependencies = [dependency('luajit'), dependency('boost-175')]

luabind = static_library('luabind071', luabind_src, dependencies: luabind_dependencies, include_directories:luabind_include, cpp_args:['-w', '-fpermissive'] + luabind_args)

luabind_dep = declare_dependency(link_with: luabind, include_directories: luabind_include, compile_args: luabind_args)

meson.override_dependency('luabind', luabind_dep)