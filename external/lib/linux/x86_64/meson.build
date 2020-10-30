cp = find_program('cp')
fmod_so = files('libfmod.so.11')
fmod = custom_target('libfmod', output:'libfmod.so', input:fmod_so, command:[cp, '@INPUT@', '@OUTPUT@'])
