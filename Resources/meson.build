# User frontend
desktop_file = files('cccp.desktop')
icon_file = files('cccp.xpm')
install_data('cccp.desktop', install_dir:get_option('datadir')/'applications', install_mode: 'rw-r--r--')
install_data('cccp.xpm', install_dir: get_option('datadir')/'icons/hicolor/32x32/apps', install_mode:'rw-r--r--')
configure_file(input: 'cccp.sh', output:'CortexCommand', install:true, install_dir: get_option('bindir'), install_mode:'rwxr-xr-x', configuration: conf_data)
