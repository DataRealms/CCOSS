project('allegro', 'c')

c = meson.get_compiler('c')
allegro_defines = []
if host_machine.system() in ['linux', 'darwin']
  allegro_defines += 'ALLEGRO_UNIX'
  aluconfig = configuration_data()
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
loadpng_include = include_directories('addons/loadpng')

loadpng_dependencies = [dependency('libpng')]

loadpng_sources = []
subdir('addons/loadpng')
allegro = static_library('allegro', sources: allegro_sources, include_directories: allegro_include, c_args:['-Wno-deprecated-declarations'])
allegro_args = ['-DALLEGRO_NO_COMPATIBILITY', '-Wno-deprecated-declarations']

loadpng = static_library('loadpng', sources: loadpng_sources, dependencies: loadpng_dependencies, include_directories: allegro_include)

allegro_dep = declare_dependency(link_with: allegro, include_directories: allegro_include, compile_args: allegro_args)
loadpng_dep = declare_dependency(link_with: loadpng, include_directories: loadpng_include)

meson.override_dependency('allegro', allegro_dep)
meson.override_dependency('loadpng', loadpng_dep)