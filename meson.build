project('CCOSS-community', ['cpp','c'], default_options:['cpp_std=c++17'], version:'0.1.0-Pre-3.1', meson_version:'>=0.49')

source_inc_dirs = include_directories(
  'Activities',
  'Entities',
  'GUI',
  'Managers',
  'Menus',
  'System')

deps = [
  dependency('allegro'),
  dependency('loadpng'),
  dependency('flac'),
  dependency('luajit'),
  dependency('minizip'),
  dependency('threads'),
  dependency('liblz4'),
  dependency('libpng'),
  dependency('boost'), #needed for luabind
  dependency('x11')
  ]

compiler = meson.get_compiler('cpp')
build_rpath = []
link_args=[]
extra_args=[]
preprocessor_flags = []
preprocessor_flags_debug = []
suffix=''

if compiler.get_argument_syntax()== 'gcc' # this will allow for clang to be used
  build_rpath += '$ORIGIN'
  extra_args = ['-fpermissive', '-Wno-unknown-pragmas'] # Warnings are there for a reason
  release_args = ['-w']
  suffix = 'x86_64'
  preprocessor_flags = ['-DRELEASE_BUILD']
  preprocessor_flags_debug = ['-DDEBUG_RELEASE_BUILD']
  if compiler.get_id() =='gcc' and compiler.version().version_compare('<9')
    link_args += ['-lstdc++fs']
  endif
elif compiler.get_argument_syntax()== 'msvc'
#TODO: add MSVC related arguments and stuff in here
else
  error('Using unknown compiler, please use gcc or msvc compatible compilers')
endif

pch = 'System/StandardIncludes.h'
sources = []
subdir('Activities')
subdir('Entities')
subdir('GUI')
subdir('Managers')
subdir('Menus')
subdir('System')
subdir('Resources')
subdir('external')

debug = executable('CCCP_debug', 'Main.cpp',sources, link_with: [luabind_debug, raknet_debug, fmod],
            dependencies: [deps], include_directories:[source_inc_dirs, external_inc_dirs],
            cpp_args:[extra_args, preprocessor_flags_debug], link_args:link_args, cpp_pch:pch,
            name_suffix:suffix, build_by_default:false, override_options:['buildtype=debugoptimized', 'optimization=g'])

release = executable('CCCP', 'Main.cpp', sources, link_with: [luabind, raknet, fmod],
            dependencies: [deps], include_directories:[source_inc_dirs, external_inc_dirs],
            cpp_args:[extra_args, preprocessor_flags], link_args:link_args+release_args, cpp_pch:pch,
            name_suffix:suffix, build_by_default:true, override_options:['buildtype=release', 'optimization=3', 'debug=false'])

if target_machine.system() in ['linux']
  #AppImage
  wget = find_program('wget')

  linuxdeploy_url = 'https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage'

  linuxdeploy = custom_target('linuxdeploy', build_by_default:false, output:'linuxdeploy-x86_64.AppImage',
		command:[find_program('env'), wget, '-O', '@OUTPUT@', linuxdeploy_url,
		find_program('chmod'), '+x', '@OUTPUT@'])
  outname = 'cccp-x86_64.AppImage'

  custom_target('appimage', depends:[release, linuxdeploy], output:outname,
		command:[find_program('env'), 'OUTPUT='+outname, linuxdeploy.full_path(),
		'--appdir=@PRIVATE_DIR@', '-e', release, '-l', fmod_so[0], '-d', desktop_file, '-i', icon_file, '-oappimage'])
endif
