lua_sources = files(
'LuabindObjectWrapper.cpp',
'LuaAdapters.cpp',
'LuaBindingsActivities.cpp',
'LuaBindingsEntities.cpp',
'LuaBindingsGUI.cpp',
'LuaBindingsInput.cpp',
'LuaBindingsManagers.cpp',
'LuaBindingsMisc.cpp',
'LuaBindingsSystem.cpp',
'LuaBindingsPrimitives.cpp'
)

luabindings_cpp_args = []
defaults_override = []
if compiler.get_argument_syntax() == 'gcc'
elif compiler.get_argument_syntax() == 'msvc'
  luabindings_cpp_args += ['-D_HAS_ITERATOR_DEBUGGING=0', '-D_HAS_AUTO_PTR_ETC=1', '-permissive', '-bigobj']
  defaults_override = ['cpp_std=v'+get_option('cpp_std')] # override standard to use vc++, removing permissive- flag
  message(defaults_override)
else
error('Unsupported compiler')
endif

lua_bindings = static_library('lua_bindings', lua_sources, dependencies:deps + luabind_dep, include_directories: [source_inc_dirs, external_inc_dirs], cpp_args: luabindings_cpp_args, cpp_pch:pch, override_options: defaults_override)
source_libs += lua_bindings