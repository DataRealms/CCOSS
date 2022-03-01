# User frontend
desktop_file = files('cccp.desktop')
icon_file = files('cccp.xpm')
configure_file(input: 'cccp.desktop', output: 'CortexCommand.desktop', install: true, install_dir:get_option('datadir')/'applications', install_mode: 'rw-r--r--', configuration: conf_data)
install_data('cccp.xpm', install_dir: get_option('datadir')/'icons/hicolor/32x32/apps', install_mode:'rw-r--r--')
configure_file(input: 'cccp.sh', output:'CortexCommand', install: get_option('install_runner'), install_dir: get_option('bindir'), install_mode:'rwxr-xr-x', configuration: conf_data)
