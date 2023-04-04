project('Cortex-Command-Community-Project', ['cpp','c'], subproject_dir: 'external/sources', default_options:['cpp_std=c++17', 'buildtype=release', 'b_lto=true'], version:'0.1.0-Pre-4.1', meson_version:'>=0.60')

#### Build environment Setup ####

deps=[]
if host_machine.system() in ['linux','darwin']
  deps = [
    dependency('sdl2'),
    dependency('flac'),
    dependency('minizip', version:'<1.3', not_found_message:'The correct version (1.2.x) of minizip might be packaged as minizip-compat.'),
    dependency('luajit'),
    dependency('threads'),
    dependency('liblz4'),
    dependency('libpng'),
    dependency(['lua', 'lua54', 'lua5.4','lua-5.4', 'lua53', 'lua-5.3', 'lua5.3','lua52', 'lua5.2', 'lua-5.2', 'lua51', 'lua5.1', 'lua-5.1']),
    dependency('tbb'),
  ]
endif

elfname = 'CortexCommand'
compiler = meson.get_compiler('cpp')
build_rpath = ''
link_args=[]
extra_args=[]
preprocessor_flags = []
suffix=''

if compiler.get_argument_syntax()== 'gcc' # used for gcc compatible compilers
  # Build against system libraries on linux
  message('gcc detected')

  build_rpath = '$ORIGIN:$ORIGIN/../external/lib/linux/x86_64' # Set RUNPATH so that CCCP can find libfmod.so without needing to set LD_LIBRARY_PATH
  #suffix = 'x86_64'
	if host_machine.system()=='linux'
  link_args += ['-Wl,--enable-new-dtags'] # Set RUNPATH instead of RPATH
	endif

  # Set preprocessor flags and cpp arguments according to build options
  buildtype_debug = get_option('debug')
  if buildtype_debug

    elfname += '_debug' # add _debug suffix for debug builds
    extra_args += ['-Wno-unknown-pragmas', '-Wno-deprecated-declarations', '-Wno-misleading-indentation'] # Disable #pragma region complaints and luabind auto_ptr warning

    if get_option('sane_warnings')
      extra_args += ['-Wno-sign-compare', '-Wno-non-virtual-dtor', '-Wno-parentheses', '-Wno-overloaded-virtual', '-Wno-unused-variable', '-Wno-unused-function']
      if compiler.get_id() == 'clang'
        extra_args += ['-Wno-inconsistent-missing-override', '-D_LIBCPP_ENABLE_CXX17_REMOVED_AUTO_PTR']
      endif
    endif

    debug_type = get_option('debug_type')
    if debug_type == 'release'
      preprocessor_flags += ['-DDEBUG_RELEASE_BUILD'] # enable minimal debug features
    elif debug_type == 'minimal'
      preprocessor_flags += ['-DDEBUGMODE'] # enable some debug features
    elif debug_type == 'full'
      preprocessor_flags += ['-DDEBUG_BUILD', '-DDEBUGMODE'] # enable all debug features; may slow down game
    endif
  else
    extra_args = ['-w'] # Disable all warnings for release builds
    preprocessor_flags += ['-DRELEASE_BUILD'] # disable all debug features
  endif
  if compiler.get_id() =='gcc' and compiler.version().version_compare('<9')
    link_args += ['-lstdc++fs']
  endif
elif compiler.get_argument_syntax()== 'msvc'
#TODO: add MSVC related arguments and stuff in here
message('cl detected')
  elfname = 'Cortex Command'
  extra_args += ['-permissive-', '-D_HAS_ITERATOR_DEBUGGING=0', '-D_HAS_AUTO_PTR_ETC=1']
  add_project_link_arguments(['winmm.lib', 'ws2_32.lib', 'dinput8.lib', 'ddraw.lib', 'dxguid.lib', 'dsound.lib'], language:'cpp')
  if host_machine.cpu_family() == 'x64'
    elfname += ' x64'
  endif
  preprocessor_flags += ['-D_CRT_SECURE_NO_DEPRECATE', '-D_WINDOWS', '-DWIN32', '-DWIN32_LEAN_AND_MEAN']
  link_args+=['-ignore:4099', '-ignore:4217']
  buildtype_debug = get_option('debug')
  if buildtype_debug
    elfname+='.debug'
    debug_type = get_option('debug_type')
    if debug_type == 'release'
      preprocessor_flags += ['-DDEBUG_RELEASE_BUILD'] # enable minimal debug features
    elif debug_type == 'minimal'
      elfname += '.minimal'
      preprocessor_flags += ['-DDEBUGMODE'] # enable some debug features
    elif debug_type == 'full'
      elfname += '.full'
      preprocessor_flags += ['-DDEBUG_BUILD', '-DDEBUGMODE'] # enable all debug features; may slow down game
    endif
  else
    preprocessor_flags += ['-DRELEASE_BUILD', '-DNDEBUG']
  endif
else
  error('Using unknown compiler, please use gcc or msvc compatible compilers')
endif

#### Configuration ####
conf_data = configuration_data()
prefix = get_option('prefix')

conf_data.set_quoted('BASEDATAPATH', prefix/get_option('data_install_dir'))
if suffix==''
  suffix=[]
  conf_data.set('EXENAME', elfname)
else
  conf_data.set('EXENAME', elfname + '.' + suffix)
endif
if get_option('install_runner')
  conf_data.set('EXERUNNER', 'CortexCommand')
  conf_data.set('EXEPATH', prefix/get_option('libdir')/'CortexCommand')
else
  conf_data.set('EXERUNNER', conf_data.get('EXENAME'))
  conf_data.set('EXEPATH', prefix/get_option('bindir'))
endif

#### Sources Setup ####
pch = 'System/StandardIncludes.h'
sources = []
source_inc_dirs = include_directories(
  'Activities',
  'Entities',
  'GUI',
  'GUI/Wrappers',
  'Lua',
  'Managers',
  'Menus',
  'Resources',
  'System')

subdir('Activities')
subdir('Entities')
subdir('GUI')
subdir('Lua')
subdir('Managers')
subdir('Menus')
subdir('System')
subdir('Resources')

#### External Libraries ####
external_objects = []
external_libs = []
subdir('external')

boost_dep = declare_dependency(include_directories: include_directories('external/include/common/boost_1_75'))
meson.override_dependency('boost-175', boost_dep)
install_rpath = prefix/get_option('fmod_dir')
allegro_proj = subproject('allegro 4.4.3.1-custom')
allegro_dep = dependency('allegro')
loadpng_dep = dependency('loadpng')
luabind_proj = subproject('luabind-0.7.1')
luabind_dep = dependency('luabind')
raknet_proj = subproject('RakNet')
raknet_dep = dependency('RakNet')
deps += [allegro_dep, loadpng_dep, luabind_dep, raknet_dep]

#### Target Definitions ####
cccpelf = executable(
  elfname, ['Main.cpp', sources], include_directories:[source_inc_dirs, external_inc_dirs], cpp_pch:pch, # Sources options
  link_with:external_libs, objects:[external_objects], dependencies:deps,                                # Link options
  cpp_args:[extra_args, preprocessor_flags], link_args:link_args, build_rpath:build_rpath,               # Compiler setup
  name_suffix:suffix,   # Executable name options
  build_by_default:true, # Meson options
  install: true, install_rpath: install_rpath, install_dir: get_option('install_runner') ? get_option('libdir')/'CortexCommand' : get_option('bindir'),
  win_subsystem:'windows' # Windows mark as GUI app
)

if get_option('debug') and get_option('b_lto')
  warning('Link time optimization enabled in debug mode, debug symbols may not be fully available. Disable b_lto with "meson configure -Db_lto=false".')
endif

#### Installing #####
base_exclude_files = [
	'Base.rte/Settings.ini',
	'Launch Actor Editor.bat',
	'Launch Area Editor.bat',
	'Launch Assembly Editor.bat',
	'Launch Gib Editor.bat',
	'Launch Scene Editor.bat',
	'LogConsole.txt',
	'LogLoading.txt',
	'LogLoadingWarning.txt',
	'Start Dedicated Server.bat'
]

if get_option('install_data')
install_subdir(get_option('cccpdatadir'),
               exclude_directories:['.git', '.github', 'Metagames.rte', 'Scenes.rte'],
               exclude_files:base_exclude_files,
               install_dir:get_option('data_install_dir'),
               strip_directory:true)
endif
