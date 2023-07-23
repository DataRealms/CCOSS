allegro_args = ['-DALLEGRO_STATICLINK', '-DALLEGRO_NO_STD_HEADERS', '-DALLEGRO_NO_MAGIC_MAIN']
zlib_args = ['-DZLIB_WINAPI']
preprocessor_flags += [allegro_args, zlib_args]
if host_machine.cpu_family() == 'x86_64'
  if get_option('debug')
    fmod = declare_dependency(dependencies: compiler.find_library('fmod_vc', dirs: meson.current_source_dir()))
    lz4 = declare_dependency(dependencies: compiler.find_library('lz4_debug_static', dirs: meson.current_source_dir()))
  else
    fmod = declare_dependency(dependencies: compiler.find_library('fmod_vc', dirs: meson.current_source_dir()))
    lz4 = declare_dependency(dependencies: compiler.find_library('lz4_release_static', dirs: meson.current_source_dir()))
  endif
else
message('using x86 libs')
  if get_option('debug')
  message('using debug libs')
    fmod = declare_dependency(dependencies: compiler.find_library('fmodL_vc', dirs: meson.current_source_dir()/'x86'))
    lz4 = declare_dependency(dependencies: compiler.find_library('lz4_debug_static', dirs: meson.current_source_dir()/'x86'))
  else
    fmod = declare_dependency(dependencies: compiler.find_library('fmodL_vc', dirs: meson.current_source_dir()/'x86'))
    lz4 = declare_dependency(dependencies: compiler.find_library('lz4_debug_static', dirs: meson.current_source_dir()/'x86'))
  endif
endif
deps += [fmod, lz4]