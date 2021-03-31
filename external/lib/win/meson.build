allegro_args = ['-D ALLEGRO_STATICLINK', '-D ALLEGRO_NO_STD_HEADERS', '-D ALLEGRO_NO_MAGIC_MAIN']
zlib_args = ['-D ZLIB_WINAPI']
preprocessor_flags += [allegro_args, zlib_args]
if host_machine.cpu_family() == 'x86'
  if get_option('debug')
  external_objects += files(
      'alleg-debug-static.lib',
      'RakNet_Debug_Win32.lib',
      'fmodL_vc.lib',
      'liblz4_debug_static.lib',
      'libpng16_static.lib',
      'loadpng-debug.lib',
      'lua51.lib',
      'luabind-debug.lib',
      'zlibstat.lib'
    )
  else
    external_objects += files(
      'alleg.lib',
      'RakNet_Release_Win32.lib',
      'fmodL_vc.lib',
      'liblz4_release_static.lib',
      'libpng16_static.lib',
      'loadpng.lib',
      'lua51.lib',
      'luabind-release.lib',
      'zlibstat.lib',
    )
  endif
else
message('using x86_64 libs')
  if get_option('debug')
  message('using debug libs')
  external_objects += files(
    'x64/alleg-debug-static.lib',
    'x64/RakNet_Debug_x64.lib',
    'x64/fmod_vc.lib',
    'x64/liblz4_debug_static.lib',
    'x64/libpng16_static.lib',
    'x64/loadpng-debug.lib',
    'x64/lua51-64.lib',
    'x64/luabind-debug.lib',
    'x64/zlibstat.lib',
    )
  else
    external_objects += files(
     'x64/alleg.lib',
     'x64/RakNet_Release_x64.lib',
     'x64/fmod_vc.lib',
     'x64/liblz4_release_static.lib',
     'x64/libpng16_static.lib',
     'x64/loadpng.lib',
     'x64/lua51-64.lib',
     'x64/luabind-release.lib',
     'x64/zlibstat.lib',
    )
  endif
endif
