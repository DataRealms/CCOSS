cp = find_program('cp')
fmod_so = files('libfmod.so.12.7', 'libfmod.so.12', 'libfmod.so')
fmod = custom_target('libfmod', output:['libfmod.so'], input:fmod_so, command:[cp, '-P', '@INPUT@', '@OUTDIR@'])
install_data(fmod_so, install_dir:get_option('fmod_dir'), install_mode:'rwxr-xr-x')
external_objects += [fmod_so]
