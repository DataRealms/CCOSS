project('allegro', 'c')

c = meson.get_compiler('c')
allegro_defines = []
alconfig = configuration_data()
c_args = []

if host_machine.system() in ['linux', 'darwin']
  alconfig.set('ALLEGRO_COLOR8', true)
  alconfig.set('ALLEGRO_COLOR16', true)
  alconfig.set('ALLEGRO_COLOR24', true)
  alconfig.set('ALLEGRO_COLOR32', true)
  alconfig.set('ALLEGRO_BIG_ENDIAN', host_machine.endian() == 'big')
  alconfig.set('ALLEGRO_LITTLE_ENDIAN', host_machine.endian() == 'little')


  alconfig.set('ALLEGRO_UNIX', true)
  alconfig.set('ALLEGRO_HAVE_LIBPTHREAD', c.find_library('pthread', required:false).found())
  alconfig.set('ALLEGRO_HAVE_STDINT_H', c.has_header('stdint.h'))
  alconfig.set('ALLEGRO_HAVE_INTTYPES_H', c.has_header('inttypes.h'))
  alconfig.set('ALLEGRO_HAVE_SYS_TIME_H', c.has_header('sys/time.h'))
  alconfig.set('ALLEGRO_HAVE_SYS_IO_H', c.has_header('sys/io.h'))
  alconfig.set('ALLEGRO_HAVE_DIRENT_H', c.has_header('dirent.h'))
  alconfig.set('ALLEGRO_HAVE_SYS_STAT_H', c.has_header('sys/stat.h'))
  alconfig.set('ALLEGRO_HAVE_SYS_UTSNAME_H', c.has_header('sys/utsname.h'))

  alconfig.set('ALLEGRO_HAVE_MKSTEMP', c.has_function('mkstemp'))
  alconfig.set('ALLEGRO_HAVE_MEMCMP', c.has_function('memcmp'))
  alconfig.set('ALLEGRO_HAVE_MPROTECT', c.has_function('mprotect'))
  alconfig.set('ALLEGRO_HAVE_MMAP', c.has_function('mmap'))
  alconfig.set('ALLEGRO_HAVE_SCHED_YIELD', c.has_function('sched_yield'))
  alconfig.set('ALLEGRO_HAVE_STRICMP', c.has_function('stricmp'))
  alconfig.set('ALLEGRO_HAVE_STRLWR', c.has_function('strlwr'))
  alconfig.set('ALLEGRO_HAVE_STRUPR', c.has_function('strupr'))

  alconfig.set('MAP_FAILED_DEFINED', c.compiles('''
    #include <unistd.h>
    #include <sys/mman.h>
    int main(void) {
      void *x = MAP_FAILED;
    }"
    MAP_FAILED_DEFINED
  '''))
  alconfig.set('ALLEGRO_USE_CONSTRUCTOR', true) # TODO: dont understand if constructors work or not, or if that even makes any difference
  alconfig.set('ALLEGRO_HAVE_POSIX_MONOTONIC_CLOCK', c.compiles('''
    #include <time.h>
    int main(void) {
        struct timespec new_time_ns;
        clock_gettime(CLOCK_MONOTONIC, &new_time_ns);
        return 0;
    }
  '''))


elif host_machine.system() == 'windows'
  if c.get_argument_syntax() == 'msvc'
    c_args += ['-DALLEGRO_NO_MAGIC_MAIN', '-DALLEGRO_STATICLINK', '-D_CRT_NONSTDC_NO_DEPRECATE', '-D_CRT_SECURE_NO_DEPRECATE']
    alconfig.set('ALLEGRO_MSVC', true)
  else
    alconfig.set('ALLEGRO_MINGW32', true)
  endif


endif
subdir('include/allegro/platform')

allegro_defines += '-DALLEGRO_SRC'

allegro_sources = []
subdir('src')

allegro_include = include_directories('include')
loadpng_include = include_directories('addons/loadpng')

loadpng_dependencies = [dependency('libpng')]

loadpng_sources = []
subdir('addons/loadpng')

compiler = meson.get_compiler('c')

if compiler.get_argument_syntax() == 'gcc' 
  c_args = ['-Wno-deprecated-declarations']
else
endif
allegro_args = ['-DALLEGRO_NO_COMPATIBILITY']
allegro_lib = []
loadpng_lib = []

if not get_option('use_prebuilt_libraries') or host_machine.system() in ['darwin', 'linux']
  allegro = static_library('allegro', sources: allegro_sources, include_directories: allegro_include, c_args: [c_args, allegro_defines])
  loadpng = static_library('loadpng', sources: loadpng_sources, dependencies: loadpng_dependencies, include_directories: allegro_include)
else 
  allegro = []
  loadpng = []
  if get_option('debug')
    allegro_lib = compiler.find_library('allegro-debug', dirs: meson.current_source_dir()/'_Bin')
    loadpng_lib = compiler.find_library('loadpng-debug', dirs:meson.current_source_dir()/'_Bin')
  else
    allegro_lib = compiler.find_library('allegro-release', dirs: meson.current_source_dir()/'_Bin')
    loadpng_lib = compiler.find_library('loadpng-release', dirs: meson.current_source_dir()/'_Bin')
  endif
endif


allegro_dep = declare_dependency(link_with: allegro, dependencies: allegro_lib, include_directories: allegro_include, compile_args: allegro_args + c_args)
loadpng_dep = declare_dependency(link_with: loadpng, dependencies: loadpng_lib, include_directories: loadpng_include)

meson.override_dependency('allegro', allegro_dep)
meson.override_dependency('loadpng', loadpng_dep)