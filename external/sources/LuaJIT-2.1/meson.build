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

libluajit = library(lj_libname, ljlib_sources + ljcore_sources + buildvm_headers,
    include_directories: luajit_source_dir,
    c_args: lj_defines,
    name_prefix: lj_libprefix,
    dependencies: luajit_dependencies,
    install: true
)

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