project('minizip', ['c', 'cpp'])


libname = 'minizip' #filename without extension or lib prefix
lib_include_dir = include_directories('include', 'src')

cc = meson.get_compiler('c')
cxx = meson.get_compiler('cpp')
if host_machine.cpu_family() == 'x86_64'
	lib = cc.find_library(libname, dirs:  meson.current_source_dir()/'_Bin', required: false)
	if not lib.found()
		lib = cxx.find_library(libname, dirs:  meson.current_source_dir()/'_Bin')
	endif
else
	lib = cc.find_library(libname+'-32', dirs:  meson.current_source_dir()/'_Bin', required: false)
	if not lib.found()
		lib = cxx.find_library(libname+'-32', dirs: meson.current_source_dir()/'_Bin')
	endif
endif

lib_dep = declare_dependency(dependencies: lib, include_directories: lib_include_dir)

meson.override_dependency(meson.project_name(), lib_dep)