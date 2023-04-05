cp = find_program('cp')
fmod_so = files('libfmod.so.13.13', 'libfmod.so.13', 'libfmod.so')
if get_option('install_fmod')
  install_data(fmod_so, install_dir:get_option('fmod_dir'), install_mode:'rwxr-xr-x')
endif

fmod = compiler.find_library('libfmod', dirs: meson.current_source_dir())

deps += fmod