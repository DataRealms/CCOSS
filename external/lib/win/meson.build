allegro_args = ['-D ALLEGRO_STATICLINK', '-D ALLEGRO_NO_STD_HEADERS', '-D ALLEGRO_NO_MAGIC_MAIN']
zlib_args = ['-D ZLIB_WINAPI']
if host_machine.cpu_family() == 'x86'
  if get_option('debug')
  deps+=[
      declare_dependency(link_with:'alleg-debug-static.lib', compile_args:allegro_args),
      declare_dependency(link_with:'RakNet_Debug_Win32.lib'),
      declare_dependency(link_with:'fmodL_vc.lib'),
      declare_dependency(link_with:'liblz4_debug_static.lib'),
      declare_dependency(link_with:'libpng16_static.lib'),
      declare_dependency(link_with:'loadpng-debug.lib'),
      declare_dependency(link_with:'lua51.lib'),
      # declare_dependency(link_with:'luabind-debug.lib'),
      declare_dependency(link_with:'zlibstat.lib', compile_args:zlib_args)
    ]
  else
    deps+=[
      declare_dependency(link_with:'alleg.lib', compile_args:allegro_args),
      declare_dependency(link_with:'RakNet_Release_Win32.lib'),
      declare_dependency(link_with:'fmodL_vc.lib'),
      declare_dependency(link_with:'liblz4_release_static.lib'),
      declare_dependency(link_with:'libpng16_static.lib'),
      declare_dependency(link_with:'loadpng.lib'),
      declare_dependency(link_with:'lua51.lib'),
      # declare_dependency(link_with:'luabind-release.lib'),
      declare_dependency(link_with:'zlibstat.lib', compile_args:zlib_args)
    ]
  endif
else
  if get_option('debug')
  deps+=[
      declare_dependency(link_with:'x64/alleg-debug-static.lib', compile_args:allegro_args),
      declare_dependency(link_with:'x64/RakNet_Debug_x64.lib'),
      declare_dependency(link_with:'x64/fmodL_vc.lib'),
      declare_dependency(link_with:'x64/liblz4_debug_static.lib'),
      declare_dependency(link_with:'x64/libpng16_static.lib'),
      declare_dependency(link_with:'x64/loadpng-debug.lib'),
      declare_dependency(link_with:'x64/lua51.lib'),
      # declare_dependency(link_with:'x64/luabind-debug.lib'),
      declare_dependency(link_with:'x64/zlibstat.lib', compile_args:zlib_args)
    ]
  else
    deps+=[
      declare_dependency(link_with:'x64/alleg.lib', compile_args:allegro_args),
      declare_dependency(link_with:'x64/RakNet_Release_x64.lib'),
      declare_dependency(link_with:'x64/fmod_vc.lib'),
      declare_dependency(link_with:'x64/liblz4_release_static.lib'),
      declare_dependency(link_with:'x64/libpng16_static.lib'),
      declare_dependency(link_with:'x64/loadpng.lib'),
      declare_dependency(link_with:'x64/lua51-64.lib'),
      # declare_dependency(link_with:'x64/luabind-release.lib'),
      declare_dependency(link_with:'x64/zlibstat.lib', compile_args:zlib_args)
    ]
  endif
endif
