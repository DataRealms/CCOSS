project('CCOSS-community', ['cpp','c'], default_options:['cpp_std=c++17', 'buildtype=release'], version:'0.1.0-Pre-3.1', meson_version:'>=0.49')

#### Build environment Setup ####
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
elfname = 'CCCP'
compiler = meson.get_compiler('cpp')
build_rpath = ''
link_args=[]
extra_args=[]
preprocessor_flags = []
preprocessor_flags_debug = []
suffix=''

if compiler.get_argument_syntax()== 'gcc' # used for gcc compatible compilers
  # Build against system libraries on linux

  build_rpath = '$ORIGIN:$ORIGIN/../external/lib/linux/x86_64' # Set RUNPATH so that CCCP can find libfmod.so without needing to set LD_LIBRARY_PATH
  suffix = 'x86_64'
  link_args += ['-Wl,--enable-new-dtags'] # Set RUNPATH instead of RPATH

  # Set preprocessor flags and cpp arguments according to build options
  buildtype_debug = get_option('debug')
  if buildtype_debug

    elfname += '_debug' # add _debug suffix for debug builds
    extra_args = ['-fpermissive', '-Wno-unknown-pragmas'] # Disable #pragma region complaints

    if not get_option('full_debug')
      preprocessor_flags += ['-DDEBUG_RELEASE_BUILD'] # enable some debug features
    else
      preprocessor_flags += ['-DDEBUG_BUILD'] # enable all debug features; may slow down game
    endif
  else
    extra_args = ['-fpermissive', '-w'] # Disable all warnings for release builds
    preprocessor_flags += ['-DRELEASE_BUILD'] # disable all debug features
  endif
  if compiler.get_id() =='gcc' and compiler.version().version_compare('<9')
    link_args += ['-lstdc++fs']
  endif
elif compiler.get_argument_syntax()== 'msvc'
#TODO: add MSVC related arguments and stuff in here
else
  error('Using unknown compiler, please use gcc or msvc compatible compilers')
endif

#### Sources Setup ####
pch = 'System/StandardIncludes.h'
sources = []
source_inc_dirs = include_directories(
  'Activities',
  'Entities',
  'GUI',
  'Managers',
  'Menus',
  'System')

subdir('Activities')
subdir('Entities')
subdir('GUI')
subdir('Managers')
subdir('Menus')
subdir('System')
subdir('Resources')

#### External Libraries ####
subdir('external')

#### Target Definitions ####
executable(
  elfname, ['Main.cpp', sources], include_directories:[source_inc_dirs, external_inc_dirs], cpp_pch:pch, # Sources options
  link_with:[luabind, raknet], objects:[fmod_so], dependencies:deps,                                     # Link options
  cpp_args:[extra_args, preprocessor_flags], link_args:link_args, build_rpath:build_rpath,               # Compiler setup
  name_suffix:suffix,   # Executable name options
  build_by_default:true # Meson options
)

# disable appimage target unless option appimage_enabbled is set
appimage_disable = get_option('enable_appimage') or disabler()

if appimage_disable
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
