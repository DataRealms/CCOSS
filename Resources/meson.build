desktop_file = files('cccp.desktop')
icon_file = files('cccp.xpm')
configure_file(input: 'cccp.desktop', output: 'cccp.desktop',
               install: true, install_dir:get_option('datadir')/'applications', install_mode: 'rw-r--r--',
               configuration:{'EXENAME' : elfname})

install_data('cccp.xpm', install_dir: get_option('datadir')/'icons/hicolor/32x32/apps', install_mode:'rw-r--r--')
