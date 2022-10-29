luabind_src = files('class.cpp',
'class_info.cpp',
'class_registry.cpp',
'class_rep.cpp',
'create_class.cpp',
'error.cpp',
'find_best_match.cpp',
'function.cpp',
'implicit_cast.cpp',
'link_compatibility.cpp',
'object_rep.cpp',
'open.cpp',
'overload_rep.cpp',
'pcall.cpp',
'ref.cpp',
'scope.cpp',
'stack_content_by_name.cpp',
'weak_ref.cpp',
'wrapper_base.cpp')

luabind = static_library('luabind071', luabind_src, include_directories:external_inc_dirs, dependencies: [deps], cpp_args:extra_args + ['-w', '-fpermissive', '-D_HAS_AUTO_PTR_ETC=1', '-D_LIBCPP_ENABLE_CXX17_REMOVED_FEATURES'], override_options:['debug=false', 'optimization=3'])
luabind_debug = static_library('luabind_debug', luabind_src, include_directories:external_inc_dirs, dependencies: [deps], cpp_args:extra_args + ['-w', '-fpermissive', '-D_HAS_AUTO_PTR_ETC=1', '-D_LIBCPP_ENABLE_CXX17_REMOVED_AUTO_PTR'], override_options:['debug=true', 'optimization=g'])
if get_option('debug')
  external_libs += [luabind_debug]
else
  external_libs += [luabind]
endif
