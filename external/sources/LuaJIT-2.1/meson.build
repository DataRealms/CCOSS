# Meson files from github.com/franko/luajit
project('luajit', 'c', version : '2.0.5', default_options : ['c_winlibs='])

pkg = import('pkgconfig')

cc = meson.get_compiler('c')
libm = cc.find_library('m', required : false)
libdl = cc.find_library('dl', required : false)
luajit_dependencies = [libm, libdl]

luajit_source_dir = include_directories('src')
luajit_abiver = '51'

dynasm = files('dynasm/dynasm.lua')

subdir('src')

if not get_option('use_prebuilt_libraries') or host_machine.system() in ['darwin', 'linux']
    libluajit = library(lj_libname, ljlib_sources + ljcore_sources + buildvm_headers,
        include_directories: luajit_source_dir,
        c_args: lj_defines,
        name_prefix: lj_libprefix,
        dependencies: luajit_dependencies,
        install: true
    )
else
    libluajit = []
    if get_option('debug')
        luajit_dependencies += cc.find_library('luajit-debug', dirs: meson.current_source_dir()/'_Bin')
    else
        luajit_dependencies += cc.find_library('luajit-release', dirs: meson.current_source_dir()/'_Bin')
    endif
endif

luajit_dep = declare_dependency(
    dependencies : luajit_dependencies,
    include_directories : luajit_source_dir,
    link_with : libluajit,
    link_args: lj_linkargs,
)

meson.override_dependency('luajit', luajit_dep)

pkg.generate(libluajit,
    filebase : 'luajit',
    name : 'LuaJIT',
    description : 'Just-in-time compiler for Lua',
    url : 'http://luajit.org',
)