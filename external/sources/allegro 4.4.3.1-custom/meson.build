#project('allegro', 'c')

c = meson.get_compiler('c')
allegro_defines = []
if (not allegro_dep.found() or not loadpng_dep.found()) and host_machine.system() in ['linux', 'darwin']
  allegro_defines += 'ALLEGRO_UNIX'
  aluconfig = configuration_data()
  # aluconfig.set10('ALLEGRO_NO_ASM', true)
  aluconfig.set('ALLEGRO_COLOR8', true)
  aluconfig.set('ALLEGRO_COLOR16', true)
  aluconfig.set('ALLEGRO_COLOR24', true)
  aluconfig.set('ALLEGRO_COLOR32', true)
  aluconfig.set('ALLEGRO_BIG_ENDIAN', host_machine.endian() == 'big')
  aluconfig.set('ALLEGRO_LITTLE_ENDIAN', host_machine.endian() == 'little')


  aluconfig.set('ALLEGRO_UNIX', true)
  aluconfig.set('ALLEGRO_HAVE_LIBPTHREAD', c.find_library('pthread', required:false).found())
  aluconfig.set('ALLEGRO_HAVE_STDINT_H', c.has_header('stdint.h'))
  aluconfig.set('ALLEGRO_HAVE_INTTYPES_H', c.has_header('inttypes.h'))
  aluconfig.set('ALLEGRO_HAVE_SYS_TIME_H', c.has_header('sys/time.h'))
  aluconfig.set('ALLEGRO_HAVE_SYS_IO_H', c.has_header('sys/io.h'))
  aluconfig.set('ALLEGRO_HAVE_DIRENT_H', c.has_header('dirent.h'))
  aluconfig.set('ALLEGRO_HAVE_SYS_STAT_H', c.has_header('sys/stat.h'))
  aluconfig.set('ALLEGRO_HAVE_SYS_UTSNAME_H', c.has_header('sys/utsname.h'))

  aluconfig.set('ALLEGRO_HAVE_MKSTEMP', c.has_function('mkstemp'))
  aluconfig.set('ALLEGRO_HAVE_MEMCMP', c.has_function('memcmp'))
  aluconfig.set('ALLEGRO_HAVE_MPROTECT', c.has_function('mprotect'))
  aluconfig.set('ALLEGRO_HAVE_MMAP', c.has_function('mmap'))
  aluconfig.set('ALLEGRO_HAVE_SCHED_YIELD', c.has_function('sched_yield'))
  aluconfig.set('ALLEGRO_HAVE_STRICMP', c.has_function('stricmp'))
  aluconfig.set('ALLEGRO_HAVE_STRLWR', c.has_function('strlwr'))
  aluconfig.set('ALLEGRO_HAVE_STRUPR', c.has_function('strupr'))

  aluconfig.set('MAP_FAILED_DEFINED', c.compiles('''
    #include <unistd.h>
    #include <sys/mman.h>
    int main(void) {
      void *x = MAP_FAILED;
    }"
    MAP_FAILED_DEFINED
  '''))
	aluconfig.set('ALLEGRO_USE_CONSTRUCTOR', true) # TODO: dont understand if constructors work or not, or if that even makes any difference
  aluconfig.set('ALLEGRO_HAVE_POSIX_MONOTONIC_CLOCK', c.compiles('''
    #include <time.h>
    int main(void) {
        struct timespec new_time_ns;
        clock_gettime(CLOCK_MONOTONIC, &new_time_ns);
        return 0;
    }
  '''))

  subdir('include/allegro/platform')
endif

allegro_defines += 'ALLEGRO_SRC'

allegro_sources = []
subdir('src')

allegro_include = include_directories('include')

loadpng_sources = []
subdir('addons/loadpng')
if not allegro_dep.found()
  allegro = static_library('allegro', sources: allegro_sources, include_directories: allegro_include, c_args:['-Wno-deprecated-declarations'], override_options:['buildtype=release', 'optimization=3'])
  external_libs += allegro
	external_inc_dirs += allegro_include
  extra_args += ['-DALLEGRO_NO_COMPATIBILITY', '-Wno-deprecated-declarations']
endif

if not loadpng_dep.found()
  loadpng = static_library('loadpng', sources: loadpng_sources, dependencies:deps, include_directories: allegro_include, override_options:['buildtype=release', 'optimization=3'])
	external_libs += loadpng
	external_inc_dirs += include_directories('addons/loadpng')
endif
