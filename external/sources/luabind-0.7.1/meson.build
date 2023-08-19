project('luabind', 'cpp')
subdir('src')

luabind_include = include_directories('.', 'luabind')

luabind_args = ['-D_HAS_AUTO_PTR_ETC=1', '-D_LIBCPP_ENABLE_CXX17_REMOVED_FEATURES']

luabind_dependencies = [dependency('luajit', 'LuaJIT-2.1'), dependency('boost-175')]

cpp_args = []
cpp_options = []
deps = []
cxx = meson.get_compiler('cpp')
if cxx.get_argument_syntax() == 'gcc'
  cpp_args = ['-w', '-fpermissive']
  luabind_args = cpp_args
elif cxx.get_argument_syntax() == 'msvc'
  cpp_args = ['-permissive', '-DBOOST_BIND_GLOBAL_PLACEHOLDERS=1', '-DLUABIND_BUILDING', '-D_ITERATOR_DEBUG_LEVEL=0']
  luabind_args = ['-D_HAS_AUTO_PTR_ETC=1', '-DBOOST_BIND_GLOBAL_PLACEHOLDERS=1', '-permissive']
  cpp_options = ['cpp_std=vc++20']
endif

if not get_option('use_prebuilt_libraries') or host_machine.system() in ['darwin', 'linux']
  luabind = static_library('luabind071', luabind_src, dependencies: luabind_dependencies, include_directories:luabind_include, cpp_args:cpp_args+ luabind_args, override_options: ['warning_level=0']+ cpp_options)
else
  luabind = []
  if get_option('debug')
    deps = cxx.find_library('luabind-debug' , dirs: meson.current_source_dir()/'_Bin')
  else
    deps = cxx.find_library('luabind-release', dirs: meson.current_source_dir()/'_Bin')
  endif
endif

luabind_dep = declare_dependency(link_with: luabind, dependencies: deps, include_directories: luabind_include, compile_args: luabind_args)

meson.override_dependency('luabind', luabind_dep)