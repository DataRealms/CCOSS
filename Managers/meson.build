sources += files(
#'AchievementMan.cpp',
'ActivityMan.cpp',
'AudioMan.cpp',
'CameraMan.cpp',
'ConsoleMan.cpp',
'FrameMan.cpp',
#'LuaMan.cpp',
'MenuMan.cpp',
'MetaMan.cpp',
'MovableMan.cpp',
'NetworkClient.cpp',
'NetworkServer.cpp',
'PerformanceMan.cpp',
'PostProcessMan.cpp',
'PresetMan.cpp',
'PrimitiveMan.cpp',
'SceneMan.cpp',
'SettingsMan.cpp',
'ThreadMan.cpp',
'TimerMan.cpp',
'UInputMan.cpp',
'WindowMan.cpp'
)
luaman_cpp_args = preprocessor_flags
defaults_override = []
if compiler.get_argument_syntax() == 'gcc'
  luaman_cpp_args += ['-fpermissive']
elif compiler.get_argument_syntax() == 'msvc'
  luaman_cpp_args += ['-D_HAS_ITERATOR_DEBUGGING=0', '-D_HAS_AUTO_PTR_ETC=1', '-permissive', '-bigobj']
  defaults_override = ['cpp_std=v'+get_option('cpp_std')] # override standard to use vc++, removing permissive- flag
  message(defaults_override)
else
error('Unsupported compiler')
endif

luaman = static_library('LuaMan', 'LuaMan.cpp', dependencies:deps + luabind_dep, include_directories: [source_inc_dirs, external_inc_dirs], cpp_args: luaman_cpp_args, cpp_pch:pch, override_options: defaults_override)
source_libs += luaman
