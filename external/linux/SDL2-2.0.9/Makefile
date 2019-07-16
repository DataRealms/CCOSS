# Makefile to build and install the SDL library

top_builddir = .
srcdir  = /home/ian/source/CCOSS/external/linux/SDL2-2.0.9
objects = build
gen = gen
prefix = /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/install
exec_prefix = ${prefix}
bindir	= ${exec_prefix}/bin
libdir  = ${exec_prefix}/lib
includedir = ${prefix}/include
datarootdir = ${prefix}/share
datadir	= ${datarootdir}
auxdir	= build-scripts
distpath = $(srcdir)/..
distdir = SDL2-2.0.9
distfile = $(distdir).tar.gz


SHELL	= /bin/sh
CC      = gcc
INCLUDE = -Iinclude -I/home/ian/source/CCOSS/external/linux/SDL2-2.0.9/include -idirafter /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/video/khronos
CFLAGS  = -g -O3 -DUSING_GENERATED_CONFIG_H
EXTRA_CFLAGS = -Iinclude -I/home/ian/source/CCOSS/external/linux/SDL2-2.0.9/include -idirafter /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/video/khronos  -mmmx -m3dnow -msse -msse2 -msse3 -Wall -fvisibility=hidden -Wdeclaration-after-statement -Werror=declaration-after-statement  -D_REENTRANT      -I$(gen) -I/usr/include/dbus-1.0 -I/usr/lib/dbus-1.0/include  -D_REENTRANT -DHAVE_LINUX_VERSION_H
LDFLAGS = 
EXTRA_LDFLAGS =  -Wl,--no-undefined -lm -ldl -lsndio -lts -lpthread -lrt
LIBTOOL = $(SHELL) $(top_builddir)/libtool
INSTALL = /usr/bin/install -c
AR	= ar
RANLIB	= ranlib
WINDRES	= :

TARGET  = libSDL2.la
OBJECTS = $(objects)/SDL.lo $(objects)/SDL_assert.lo $(objects)/SDL_dataqueue.lo $(objects)/SDL_error.lo $(objects)/SDL_hints.lo $(objects)/SDL_log.lo $(objects)/SDL_atomic.lo $(objects)/SDL_spinlock.lo $(objects)/SDL_audio.lo $(objects)/SDL_audiocvt.lo $(objects)/SDL_audiodev.lo $(objects)/SDL_audiotypecvt.lo $(objects)/SDL_mixer.lo $(objects)/SDL_wave.lo $(objects)/SDL_cpuinfo.lo $(objects)/SDL_dynapi.lo $(objects)/SDL_clipboardevents.lo $(objects)/SDL_displayevents.lo $(objects)/SDL_dropevents.lo $(objects)/SDL_events.lo $(objects)/SDL_gesture.lo $(objects)/SDL_keyboard.lo $(objects)/SDL_mouse.lo $(objects)/SDL_quit.lo $(objects)/SDL_touch.lo $(objects)/SDL_windowevents.lo $(objects)/SDL_rwops.lo $(objects)/SDL_haptic.lo $(objects)/SDL_gamecontroller.lo $(objects)/SDL_joystick.lo $(objects)/e_atan2.lo $(objects)/e_exp.lo $(objects)/e_fmod.lo $(objects)/e_log.lo $(objects)/e_log10.lo $(objects)/e_pow.lo $(objects)/e_rem_pio2.lo $(objects)/e_sqrt.lo $(objects)/k_cos.lo $(objects)/k_rem_pio2.lo $(objects)/k_sin.lo $(objects)/k_tan.lo $(objects)/s_atan.lo $(objects)/s_copysign.lo $(objects)/s_cos.lo $(objects)/s_fabs.lo $(objects)/s_floor.lo $(objects)/s_scalbn.lo $(objects)/s_sin.lo $(objects)/s_tan.lo $(objects)/SDL_power.lo $(objects)/SDL_d3dmath.lo $(objects)/SDL_render.lo $(objects)/SDL_yuv_sw.lo $(objects)/SDL_render_d3d.lo $(objects)/SDL_shaders_d3d.lo $(objects)/SDL_render_d3d11.lo $(objects)/SDL_shaders_d3d11.lo $(objects)/SDL_render_gl.lo $(objects)/SDL_shaders_gl.lo $(objects)/SDL_render_gles.lo $(objects)/SDL_render_gles2.lo $(objects)/SDL_shaders_gles2.lo $(objects)/SDL_render_psp.lo $(objects)/SDL_blendfillrect.lo $(objects)/SDL_blendline.lo $(objects)/SDL_blendpoint.lo $(objects)/SDL_drawline.lo $(objects)/SDL_drawpoint.lo $(objects)/SDL_render_sw.lo $(objects)/SDL_rotate.lo $(objects)/SDL_sensor.lo $(objects)/SDL_getenv.lo $(objects)/SDL_iconv.lo $(objects)/SDL_malloc.lo $(objects)/SDL_qsort.lo $(objects)/SDL_stdlib.lo $(objects)/SDL_string.lo $(objects)/SDL_thread.lo $(objects)/SDL_timer.lo $(objects)/SDL_RLEaccel.lo $(objects)/SDL_blit.lo $(objects)/SDL_blit_0.lo $(objects)/SDL_blit_1.lo $(objects)/SDL_blit_A.lo $(objects)/SDL_blit_N.lo $(objects)/SDL_blit_auto.lo $(objects)/SDL_blit_copy.lo $(objects)/SDL_blit_slow.lo $(objects)/SDL_bmp.lo $(objects)/SDL_clipboard.lo $(objects)/SDL_egl.lo $(objects)/SDL_fillrect.lo $(objects)/SDL_pixels.lo $(objects)/SDL_rect.lo $(objects)/SDL_shape.lo $(objects)/SDL_stretch.lo $(objects)/SDL_surface.lo $(objects)/SDL_video.lo $(objects)/SDL_vulkan_utils.lo $(objects)/SDL_yuv.lo $(objects)/yuv_rgb.lo $(objects)/SDL_nullevents.lo $(objects)/SDL_nullframebuffer.lo $(objects)/SDL_nullvideo.lo $(objects)/SDL_diskaudio.lo $(objects)/SDL_dummyaudio.lo $(objects)/SDL_sysloadso.lo $(objects)/SDL_dspaudio.lo $(objects)/SDL_alsa_audio.lo $(objects)/SDL_pulseaudio.lo $(objects)/SDL_jackaudio.lo $(objects)/SDL_sndioaudio.lo $(objects)/SDL_x11clipboard.lo $(objects)/SDL_x11dyn.lo $(objects)/SDL_x11events.lo $(objects)/SDL_x11framebuffer.lo $(objects)/SDL_x11keyboard.lo $(objects)/SDL_x11messagebox.lo $(objects)/SDL_x11modes.lo $(objects)/SDL_x11mouse.lo $(objects)/SDL_x11opengl.lo $(objects)/SDL_x11opengles.lo $(objects)/SDL_x11shape.lo $(objects)/SDL_x11touch.lo $(objects)/SDL_x11video.lo $(objects)/SDL_x11vulkan.lo $(objects)/SDL_x11window.lo $(objects)/SDL_x11xinput2.lo $(objects)/edid-parse.lo $(objects)/imKStoUCS.lo $(objects)/SDL_waylandclipboard.lo $(objects)/SDL_waylanddatamanager.lo $(objects)/SDL_waylanddyn.lo $(objects)/SDL_waylandevents.lo $(objects)/SDL_waylandmouse.lo $(objects)/SDL_waylandopengles.lo $(objects)/SDL_waylandtouch.lo $(objects)/SDL_waylandvideo.lo $(objects)/SDL_waylandvulkan.lo $(objects)/SDL_waylandwindow.lo $(objects)/SDL_dbus.lo $(objects)/SDL_ime.lo $(objects)/SDL_systhread.lo $(objects)/SDL_syssem.lo $(objects)/SDL_sysmutex.lo $(objects)/SDL_syscond.lo $(objects)/SDL_systls.lo $(objects)/SDL_sysjoystick.lo $(objects)/SDL_steamcontroller.lo $(objects)/SDL_syshaptic.lo $(objects)/SDL_syspower.lo $(objects)/SDL_sysfilesystem.lo $(objects)/SDL_systimer.lo $(objects)/SDL_udev.lo $(objects)/SDL_evdev.lo $(objects)/SDL_evdev_kbd.lo $(objects)/SDL_poll.lo $(objects)/SDL_dummysensor.lo
GEN_HEADERS =  $(gen)/pointer-constraints-unstable-v1-client-protocol.h $(gen)/relative-pointer-unstable-v1-client-protocol.h $(gen)/wayland-client-protocol.h $(gen)/xdg-shell-unstable-v6-client-protocol.h $(gen)/xdg-shell-client-protocol.h 
GEN_OBJECTS =  $(objects)/pointer-constraints-unstable-v1-protocol.lo $(objects)/relative-pointer-unstable-v1-protocol.lo $(objects)/wayland-protocol.lo $(objects)/xdg-shell-unstable-v6-protocol.lo $(objects)/xdg-shell-protocol.lo 
VERSION_OBJECTS = 

SDLMAIN_TARGET = libSDL2main.la
SDLMAIN_OBJECTS = $(objects)/SDL_dummy_main.lo

SDLTEST_TARGET = libSDL2_test.la
SDLTEST_OBJECTS = $(objects)/SDL_test_assert.lo $(objects)/SDL_test_common.lo $(objects)/SDL_test_compare.lo $(objects)/SDL_test_crc32.lo $(objects)/SDL_test_font.lo $(objects)/SDL_test_fuzzer.lo $(objects)/SDL_test_harness.lo $(objects)/SDL_test_imageBlit.lo $(objects)/SDL_test_imageBlitBlend.lo $(objects)/SDL_test_imageFace.lo $(objects)/SDL_test_imagePrimitives.lo $(objects)/SDL_test_imagePrimitivesBlend.lo $(objects)/SDL_test_log.lo $(objects)/SDL_test_md5.lo $(objects)/SDL_test_memory.lo $(objects)/SDL_test_random.lo

WAYLAND_SCANNER = /usr/bin/wayland-scanner

SRC_DIST = *.txt acinclude Android.mk autogen.sh android-project build-scripts cmake cmake_uninstall.cmake.in configure configure.in debian docs include Makefile.* sdl2-config.cmake.in sdl2-config.in sdl2.m4 sdl2.pc.in SDL2.spec.in SDL2Config.cmake src test VisualC.html VisualC VisualC-WinRT Xcode Xcode-iOS wayland-protocols
GEN_DIST = SDL2.spec

ifneq ($V,1)
RUN_CMD_AR     = @echo "  AR    " $@;
RUN_CMD_CC     = @echo "  CC    " $@;
RUN_CMD_CXX    = @echo "  CXX   " $@;
RUN_CMD_LTLINK = @echo "  LTLINK" $@;
RUN_CMD_RANLIB = @echo "  RANLIB" $@;
RUN_CMD_GEN    = @echo "  GEN   " $@;
LIBTOOL += --quiet
endif

HDRS = \
	SDL.h \
	SDL_assert.h \
	SDL_atomic.h \
	SDL_audio.h \
	SDL_bits.h \
	SDL_blendmode.h \
	SDL_clipboard.h \
	SDL_cpuinfo.h \
	SDL_egl.h \
	SDL_endian.h \
	SDL_error.h \
	SDL_events.h \
	SDL_filesystem.h \
	SDL_gamecontroller.h \
	SDL_gesture.h \
	SDL_haptic.h \
	SDL_hints.h \
	SDL_joystick.h \
	SDL_keyboard.h \
	SDL_keycode.h \
	SDL_loadso.h \
	SDL_log.h \
	SDL_main.h \
	SDL_messagebox.h \
	SDL_mouse.h \
	SDL_mutex.h \
	SDL_name.h \
	SDL_opengl.h \
	SDL_opengl_glext.h \
	SDL_opengles.h \
	SDL_opengles2_gl2ext.h \
	SDL_opengles2_gl2.h \
	SDL_opengles2_gl2platform.h \
	SDL_opengles2.h \
	SDL_opengles2_khrplatform.h \
	SDL_pixels.h \
	SDL_platform.h \
	SDL_power.h \
	SDL_quit.h \
	SDL_rect.h \
	SDL_render.h \
	SDL_rwops.h \
	SDL_scancode.h \
	SDL_sensor.h \
	SDL_shape.h \
	SDL_stdinc.h \
	SDL_surface.h \
	SDL_system.h \
	SDL_syswm.h \
	SDL_thread.h \
	SDL_timer.h \
	SDL_touch.h \
	SDL_types.h \
	SDL_version.h \
	SDL_video.h \
	SDL_vulkan.h \
	begin_code.h \
	close_code.h

SDLTEST_HDRS = $(shell ls $(srcdir)/include | fgrep SDL_test)

LT_AGE      = 9
LT_CURRENT  = 9
LT_RELEASE  = 2.0
LT_REVISION = 0
LT_LDFLAGS  = -no-undefined -rpath $(libdir) -release $(LT_RELEASE) -version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE)

all: $(srcdir)/configure Makefile $(objects) $(objects)/$(TARGET) $(objects)/$(SDLMAIN_TARGET) $(objects)/$(SDLTEST_TARGET)

$(srcdir)/configure: $(srcdir)/configure.in
	@echo "Warning, configure is out of date, please re-run autogen.sh"

Makefile: $(srcdir)/Makefile.in
	$(SHELL) config.status $@

Makefile.in:;

$(objects):
	$(SHELL) $(auxdir)/mkinstalldirs $@

update-revision:
	$(SHELL) $(auxdir)/updaterev.sh

.PHONY: all update-revision install install-bin install-hdrs install-lib install-data uninstall uninstall-bin uninstall-hdrs uninstall-lib uninstall-data clean distclean dist $(OBJECTS:.lo=.d)

$(objects)/$(TARGET): $(GEN_HEADERS) $(GEN_OBJECTS) $(OBJECTS) $(VERSION_OBJECTS)
	$(RUN_CMD_LTLINK)$(LIBTOOL) --tag=CC --mode=link $(CC) -o $@ $(OBJECTS) $(GEN_OBJECTS) $(VERSION_OBJECTS) $(LDFLAGS) $(EXTRA_LDFLAGS) $(LT_LDFLAGS)

$(objects)/$(SDLMAIN_TARGET): $(SDLMAIN_OBJECTS)
	$(RUN_CMD_LTLINK)$(LIBTOOL) --tag=CC --mode=link $(CC) -static -o $@ $(SDLMAIN_OBJECTS) -rpath $(libdir)

$(objects)/$(SDLTEST_TARGET): $(SDLTEST_OBJECTS)
	$(RUN_CMD_LTLINK)$(LIBTOOL) --tag=CC --mode=link $(CC) -static -o $@ $(SDLTEST_OBJECTS) -rpath $(libdir)

install: all install-bin install-hdrs install-lib install-data
install-bin:
	$(SHELL) $(auxdir)/mkinstalldirs $(DESTDIR)$(bindir)
	$(INSTALL) -m 755 sdl2-config $(DESTDIR)$(bindir)/sdl2-config
install-hdrs: update-revision
	$(SHELL) $(auxdir)/mkinstalldirs $(DESTDIR)$(includedir)/SDL2
	for file in $(HDRS) $(SDLTEST_HDRS); do \
	    $(INSTALL) -m 644 $(srcdir)/include/$$file $(DESTDIR)$(includedir)/SDL2/$$file; \
	done
	$(INSTALL) -m 644 include/SDL_config.h $(DESTDIR)$(includedir)/SDL2/SDL_config.h
	if test -f include/SDL_revision.h; then \
	    $(INSTALL) -m 644 include/SDL_revision.h $(DESTDIR)$(includedir)/SDL2/SDL_revision.h; \
	else \
	    $(INSTALL) -m 644 $(srcdir)/include/SDL_revision.h $(DESTDIR)$(includedir)/SDL2/SDL_revision.h; \
	fi

install-lib: $(objects) $(objects)/$(TARGET) $(objects)/$(SDLMAIN_TARGET) $(objects)/$(SDLTEST_TARGET)
	$(SHELL) $(auxdir)/mkinstalldirs $(DESTDIR)$(libdir)
	$(LIBTOOL) --mode=install $(INSTALL) $(objects)/$(TARGET) $(DESTDIR)$(libdir)/$(TARGET)
	$(LIBTOOL) --mode=install $(INSTALL) $(objects)/$(SDLMAIN_TARGET) $(DESTDIR)$(libdir)/$(SDLMAIN_TARGET)
	$(LIBTOOL) --mode=install $(INSTALL) $(objects)/$(SDLTEST_TARGET) $(DESTDIR)$(libdir)/$(SDLTEST_TARGET)
install-data:
	$(SHELL) $(auxdir)/mkinstalldirs $(DESTDIR)$(datadir)/aclocal
	$(INSTALL) -m 644 $(srcdir)/sdl2.m4 $(DESTDIR)$(datadir)/aclocal/sdl2.m4
	$(SHELL) $(auxdir)/mkinstalldirs $(DESTDIR)$(libdir)/pkgconfig
	$(INSTALL) -m 644 sdl2.pc $(DESTDIR)$(libdir)/pkgconfig
	$(SHELL) $(auxdir)/mkinstalldirs $(DESTDIR)$(libdir)/cmake/SDL2
	$(INSTALL) -m 644 sdl2-config.cmake $(DESTDIR)$(libdir)/cmake/SDL2

uninstall: uninstall-bin uninstall-hdrs uninstall-lib uninstall-data
uninstall-bin:
	rm -f $(DESTDIR)$(bindir)/sdl2-config
uninstall-hdrs:
	for file in $(HDRS) $(SDLTEST_HDRS); do \
	    rm -f $(DESTDIR)$(includedir)/SDL2/$$file; \
	done
	rm -f $(DESTDIR)$(includedir)/SDL2/SDL_config.h
	rm -f $(DESTDIR)$(includedir)/SDL2/SDL_revision.h
	-rmdir $(DESTDIR)$(includedir)/SDL2
uninstall-lib:
	$(LIBTOOL) --mode=uninstall rm -f $(DESTDIR)$(libdir)/$(TARGET)
	rm -f $(DESTDIR)$(libdir)/$(SDLMAIN_TARGET)
	rm -f $(DESTDIR)$(libdir)/$(SDLTEST_TARGET)
uninstall-data:
	rm -f $(DESTDIR)$(datadir)/aclocal/sdl2.m4
	rm -f $(DESTDIR)$(libdir)/pkgconfig/sdl2.pc
	rm -f $(DESTDIR)$(libdir)/cmake/SDL2/sdl2-config.cmake

clean:
	rm -rf $(objects)
	rm -rf $(gen)
	if test -f test/Makefile; then (cd test; $(MAKE) $@); fi

distclean: clean
	rm -f Makefile Makefile.rules sdl2-config
	rm -f config.status config.cache config.log libtool
	rm -rf $(srcdir)/autom4te*
	find $(srcdir) \( \
	    -name '*~' -o \
	    -name '*.bak' -o \
	    -name '*.old' -o \
	    -name '*.rej' -o \
	    -name '*.orig' -o \
	    -name '.#*' \) \
	    -exec rm -f {} \;
	if test -f test/Makefile; then (cd test; $(MAKE) $@); fi

dist $(distfile):
	$(SHELL) $(auxdir)/mkinstalldirs $(distdir)
	(cd $(srcdir); tar cf - $(SRC_DIST)) | (cd $(distdir); tar xf -)
	tar cf - $(GEN_DIST) | (cd $(distdir); tar xf -)
	find $(distdir) \( \
	    -name '*~' -o \
	    -name '*.bak' -o \
	    -name '*.old' -o \
	    -name '*.rej' -o \
	    -name '*.orig' -o \
	    -name '.#*' \) \
	    -exec rm -f {} \;
	if test -f $(distdir)/test/Makefile; then (cd $(distdir)/test && make distclean); fi
	(cd $(distdir); build-scripts/updaterev.sh)
	tar cvf - $(distdir) | gzip --best >$(distfile)
	rm -rf $(distdir)

rpm: $(distfile)
	rpmbuild -ta $?

# Build rules for objects
-include $(OBJECTS:.lo=.d)

# Special dependency for SDL.c, since it depends on SDL_revision.h
/home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/SDL.c: update-revision

$(objects)/SDL.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/SDL.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_assert.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/SDL_assert.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_dataqueue.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/SDL_dataqueue.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_error.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/SDL_error.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_hints.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/SDL_hints.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_log.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/SDL_log.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_atomic.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/atomic/SDL_atomic.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_spinlock.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/atomic/SDL_spinlock.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_audio.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/audio/SDL_audio.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_audiocvt.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/audio/SDL_audiocvt.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_audiodev.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/audio/SDL_audiodev.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_audiotypecvt.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/audio/SDL_audiotypecvt.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_mixer.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/audio/SDL_mixer.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_wave.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/audio/SDL_wave.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_cpuinfo.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/cpuinfo/SDL_cpuinfo.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_dynapi.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/dynapi/SDL_dynapi.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_clipboardevents.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/events/SDL_clipboardevents.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_displayevents.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/events/SDL_displayevents.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_dropevents.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/events/SDL_dropevents.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_events.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/events/SDL_events.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_gesture.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/events/SDL_gesture.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_keyboard.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/events/SDL_keyboard.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_mouse.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/events/SDL_mouse.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_quit.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/events/SDL_quit.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_touch.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/events/SDL_touch.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_windowevents.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/events/SDL_windowevents.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_rwops.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/file/SDL_rwops.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_haptic.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/haptic/SDL_haptic.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_gamecontroller.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/joystick/SDL_gamecontroller.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_joystick.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/joystick/SDL_joystick.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/e_atan2.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/libm/e_atan2.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/e_exp.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/libm/e_exp.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/e_fmod.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/libm/e_fmod.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/e_log.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/libm/e_log.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/e_log10.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/libm/e_log10.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/e_pow.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/libm/e_pow.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/e_rem_pio2.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/libm/e_rem_pio2.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/e_sqrt.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/libm/e_sqrt.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/k_cos.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/libm/k_cos.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/k_rem_pio2.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/libm/k_rem_pio2.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/k_sin.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/libm/k_sin.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/k_tan.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/libm/k_tan.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/s_atan.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/libm/s_atan.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/s_copysign.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/libm/s_copysign.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/s_cos.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/libm/s_cos.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/s_fabs.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/libm/s_fabs.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/s_floor.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/libm/s_floor.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/s_scalbn.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/libm/s_scalbn.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/s_sin.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/libm/s_sin.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/s_tan.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/libm/s_tan.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_power.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/power/SDL_power.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_d3dmath.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/render/SDL_d3dmath.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_render.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/render/SDL_render.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_yuv_sw.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/render/SDL_yuv_sw.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_render_d3d.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/render/direct3d/SDL_render_d3d.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_shaders_d3d.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/render/direct3d/SDL_shaders_d3d.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_render_d3d11.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/render/direct3d11/SDL_render_d3d11.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_shaders_d3d11.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/render/direct3d11/SDL_shaders_d3d11.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_render_gl.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/render/opengl/SDL_render_gl.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_shaders_gl.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/render/opengl/SDL_shaders_gl.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_render_gles.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/render/opengles/SDL_render_gles.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_render_gles2.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/render/opengles2/SDL_render_gles2.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_shaders_gles2.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/render/opengles2/SDL_shaders_gles2.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_render_psp.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/render/psp/SDL_render_psp.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_blendfillrect.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/render/software/SDL_blendfillrect.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_blendline.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/render/software/SDL_blendline.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_blendpoint.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/render/software/SDL_blendpoint.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_drawline.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/render/software/SDL_drawline.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_drawpoint.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/render/software/SDL_drawpoint.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_render_sw.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/render/software/SDL_render_sw.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_rotate.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/render/software/SDL_rotate.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_sensor.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/sensor/SDL_sensor.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_getenv.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/stdlib/SDL_getenv.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_iconv.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/stdlib/SDL_iconv.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_malloc.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/stdlib/SDL_malloc.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_qsort.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/stdlib/SDL_qsort.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_stdlib.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/stdlib/SDL_stdlib.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_string.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/stdlib/SDL_string.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_thread.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/thread/SDL_thread.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_timer.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/timer/SDL_timer.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_RLEaccel.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/video/SDL_RLEaccel.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_blit.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/video/SDL_blit.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_blit_0.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/video/SDL_blit_0.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_blit_1.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/video/SDL_blit_1.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_blit_A.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/video/SDL_blit_A.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_blit_N.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/video/SDL_blit_N.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_blit_auto.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/video/SDL_blit_auto.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_blit_copy.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/video/SDL_blit_copy.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_blit_slow.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/video/SDL_blit_slow.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_bmp.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/video/SDL_bmp.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_clipboard.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/video/SDL_clipboard.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_egl.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/video/SDL_egl.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_fillrect.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/video/SDL_fillrect.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_pixels.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/video/SDL_pixels.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_rect.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/video/SDL_rect.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_shape.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/video/SDL_shape.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_stretch.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/video/SDL_stretch.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_surface.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/video/SDL_surface.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_video.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/video/SDL_video.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_vulkan_utils.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/video/SDL_vulkan_utils.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_yuv.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/video/SDL_yuv.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/yuv_rgb.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/video/yuv2rgb/yuv_rgb.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_nullevents.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/video/dummy/SDL_nullevents.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_nullframebuffer.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/video/dummy/SDL_nullframebuffer.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_nullvideo.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/video/dummy/SDL_nullvideo.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_diskaudio.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/audio/disk/SDL_diskaudio.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_dummyaudio.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/audio/dummy/SDL_dummyaudio.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_sysloadso.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/loadso/dlopen/SDL_sysloadso.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_dspaudio.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/audio/dsp/SDL_dspaudio.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_alsa_audio.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/audio/alsa/SDL_alsa_audio.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_pulseaudio.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/audio/pulseaudio/SDL_pulseaudio.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_jackaudio.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/audio/jack/SDL_jackaudio.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_sndioaudio.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/audio/sndio/SDL_sndioaudio.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_x11clipboard.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/video/x11/SDL_x11clipboard.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_x11dyn.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/video/x11/SDL_x11dyn.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_x11events.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/video/x11/SDL_x11events.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_x11framebuffer.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/video/x11/SDL_x11framebuffer.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_x11keyboard.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/video/x11/SDL_x11keyboard.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_x11messagebox.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/video/x11/SDL_x11messagebox.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_x11modes.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/video/x11/SDL_x11modes.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_x11mouse.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/video/x11/SDL_x11mouse.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_x11opengl.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/video/x11/SDL_x11opengl.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_x11opengles.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/video/x11/SDL_x11opengles.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_x11shape.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/video/x11/SDL_x11shape.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_x11touch.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/video/x11/SDL_x11touch.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_x11video.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/video/x11/SDL_x11video.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_x11vulkan.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/video/x11/SDL_x11vulkan.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_x11window.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/video/x11/SDL_x11window.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_x11xinput2.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/video/x11/SDL_x11xinput2.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/edid-parse.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/video/x11/edid-parse.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/imKStoUCS.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/video/x11/imKStoUCS.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_waylandclipboard.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/video/wayland/SDL_waylandclipboard.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_waylanddatamanager.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/video/wayland/SDL_waylanddatamanager.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_waylanddyn.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/video/wayland/SDL_waylanddyn.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_waylandevents.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/video/wayland/SDL_waylandevents.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_waylandmouse.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/video/wayland/SDL_waylandmouse.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_waylandopengles.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/video/wayland/SDL_waylandopengles.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_waylandtouch.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/video/wayland/SDL_waylandtouch.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_waylandvideo.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/video/wayland/SDL_waylandvideo.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_waylandvulkan.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/video/wayland/SDL_waylandvulkan.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_waylandwindow.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/video/wayland/SDL_waylandwindow.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_dbus.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/core/linux/SDL_dbus.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_ime.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/core/linux/SDL_ime.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_systhread.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/thread/pthread/SDL_systhread.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_syssem.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/thread/pthread/SDL_syssem.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_sysmutex.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/thread/pthread/SDL_sysmutex.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_syscond.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/thread/pthread/SDL_syscond.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_systls.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/thread/pthread/SDL_systls.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_sysjoystick.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/joystick/linux/SDL_sysjoystick.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_steamcontroller.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/joystick/steam/SDL_steamcontroller.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_syshaptic.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/haptic/linux/SDL_syshaptic.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_syspower.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/power/linux/SDL_syspower.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_sysfilesystem.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/filesystem/unix/SDL_sysfilesystem.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_systimer.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/timer/unix/SDL_systimer.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_udev.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/core/linux/SDL_udev.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_evdev.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/core/linux/SDL_evdev.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_evdev_kbd.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/core/linux/SDL_evdev_kbd.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_poll.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/core/unix/SDL_poll.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_dummysensor.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/sensor/dummy/SDL_dummysensor.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@


$(objects)/SDL_dummy_main.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/main/dummy/SDL_dummy_main.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(objects)/SDL_test_assert.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/test/SDL_test_assert.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@ 
$(objects)/SDL_test_common.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/test/SDL_test_common.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@ 
$(objects)/SDL_test_compare.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/test/SDL_test_compare.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@ 
$(objects)/SDL_test_crc32.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/test/SDL_test_crc32.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@ 
$(objects)/SDL_test_font.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/test/SDL_test_font.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@ 
$(objects)/SDL_test_fuzzer.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/test/SDL_test_fuzzer.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@ 
$(objects)/SDL_test_harness.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/test/SDL_test_harness.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@ 
$(objects)/SDL_test_imageBlit.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/test/SDL_test_imageBlit.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@ 
$(objects)/SDL_test_imageBlitBlend.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/test/SDL_test_imageBlitBlend.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@ 
$(objects)/SDL_test_imageFace.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/test/SDL_test_imageFace.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@ 
$(objects)/SDL_test_imagePrimitives.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/test/SDL_test_imagePrimitives.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@ 
$(objects)/SDL_test_imagePrimitivesBlend.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/test/SDL_test_imagePrimitivesBlend.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@ 
$(objects)/SDL_test_log.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/test/SDL_test_log.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@ 
$(objects)/SDL_test_md5.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/test/SDL_test_md5.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@ 
$(objects)/SDL_test_memory.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/test/SDL_test_memory.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@ 
$(objects)/SDL_test_random.lo: /home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/test/SDL_test_random.c
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(gen)/pointer-constraints-unstable-v1-client-protocol.h: $(srcdir)/wayland-protocols/pointer-constraints-unstable-v1.xml
	@$(SHELL) $(auxdir)/mkinstalldirs $(gen)
	$(RUN_CMD_GEN)$(WAYLAND_SCANNER) client-header $< $@

$(gen)/pointer-constraints-unstable-v1-protocol.c: $(srcdir)/wayland-protocols/pointer-constraints-unstable-v1.xml
	@$(SHELL) $(auxdir)/mkinstalldirs $(gen)
	$(RUN_CMD_GEN)$(WAYLAND_SCANNER) code $< $@

$(objects)/pointer-constraints-unstable-v1-protocol.lo: $(gen)/pointer-constraints-unstable-v1-protocol.c $(gen)/pointer-constraints-unstable-v1-client-protocol.h
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(gen)/relative-pointer-unstable-v1-client-protocol.h: $(srcdir)/wayland-protocols/relative-pointer-unstable-v1.xml
	@$(SHELL) $(auxdir)/mkinstalldirs $(gen)
	$(RUN_CMD_GEN)$(WAYLAND_SCANNER) client-header $< $@

$(gen)/relative-pointer-unstable-v1-protocol.c: $(srcdir)/wayland-protocols/relative-pointer-unstable-v1.xml
	@$(SHELL) $(auxdir)/mkinstalldirs $(gen)
	$(RUN_CMD_GEN)$(WAYLAND_SCANNER) code $< $@

$(objects)/relative-pointer-unstable-v1-protocol.lo: $(gen)/relative-pointer-unstable-v1-protocol.c $(gen)/relative-pointer-unstable-v1-client-protocol.h
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(gen)/wayland-client-protocol.h: $(srcdir)/wayland-protocols/wayland.xml
	@$(SHELL) $(auxdir)/mkinstalldirs $(gen)
	$(RUN_CMD_GEN)$(WAYLAND_SCANNER) client-header $< $@

$(gen)/wayland-protocol.c: $(srcdir)/wayland-protocols/wayland.xml
	@$(SHELL) $(auxdir)/mkinstalldirs $(gen)
	$(RUN_CMD_GEN)$(WAYLAND_SCANNER) code $< $@

$(objects)/wayland-protocol.lo: $(gen)/wayland-protocol.c $(gen)/wayland-client-protocol.h
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(gen)/xdg-shell-unstable-v6-client-protocol.h: $(srcdir)/wayland-protocols/xdg-shell-unstable-v6.xml
	@$(SHELL) $(auxdir)/mkinstalldirs $(gen)
	$(RUN_CMD_GEN)$(WAYLAND_SCANNER) client-header $< $@

$(gen)/xdg-shell-unstable-v6-protocol.c: $(srcdir)/wayland-protocols/xdg-shell-unstable-v6.xml
	@$(SHELL) $(auxdir)/mkinstalldirs $(gen)
	$(RUN_CMD_GEN)$(WAYLAND_SCANNER) code $< $@

$(objects)/xdg-shell-unstable-v6-protocol.lo: $(gen)/xdg-shell-unstable-v6-protocol.c $(gen)/xdg-shell-unstable-v6-client-protocol.h
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

$(gen)/xdg-shell-client-protocol.h: $(srcdir)/wayland-protocols/xdg-shell.xml
	@$(SHELL) $(auxdir)/mkinstalldirs $(gen)
	$(RUN_CMD_GEN)$(WAYLAND_SCANNER) client-header $< $@

$(gen)/xdg-shell-protocol.c: $(srcdir)/wayland-protocols/xdg-shell.xml
	@$(SHELL) $(auxdir)/mkinstalldirs $(gen)
	$(RUN_CMD_GEN)$(WAYLAND_SCANNER) code $< $@

$(objects)/xdg-shell-protocol.lo: $(gen)/xdg-shell-protocol.c $(gen)/xdg-shell-client-protocol.h
	$(RUN_CMD_CC)$(LIBTOOL) --tag=CC --mode=compile $(CC) $(CFLAGS) $(EXTRA_CFLAGS) -MMD -MT $@ -c $< -o $@

/home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/video/wayland/SDL_waylandclipboard.c: $(gen)/pointer-constraints-unstable-v1-client-protocol.h $(gen)/relative-pointer-unstable-v1-client-protocol.h $(gen)/wayland-client-protocol.h $(gen)/xdg-shell-unstable-v6-client-protocol.h $(gen)/xdg-shell-client-protocol.h
/home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/video/wayland/SDL_waylanddatamanager.c: $(gen)/pointer-constraints-unstable-v1-client-protocol.h $(gen)/relative-pointer-unstable-v1-client-protocol.h $(gen)/wayland-client-protocol.h $(gen)/xdg-shell-unstable-v6-client-protocol.h $(gen)/xdg-shell-client-protocol.h
/home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/video/wayland/SDL_waylanddyn.c: $(gen)/pointer-constraints-unstable-v1-client-protocol.h $(gen)/relative-pointer-unstable-v1-client-protocol.h $(gen)/wayland-client-protocol.h $(gen)/xdg-shell-unstable-v6-client-protocol.h $(gen)/xdg-shell-client-protocol.h
/home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/video/wayland/SDL_waylandevents.c: $(gen)/pointer-constraints-unstable-v1-client-protocol.h $(gen)/relative-pointer-unstable-v1-client-protocol.h $(gen)/wayland-client-protocol.h $(gen)/xdg-shell-unstable-v6-client-protocol.h $(gen)/xdg-shell-client-protocol.h
/home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/video/wayland/SDL_waylandmouse.c: $(gen)/pointer-constraints-unstable-v1-client-protocol.h $(gen)/relative-pointer-unstable-v1-client-protocol.h $(gen)/wayland-client-protocol.h $(gen)/xdg-shell-unstable-v6-client-protocol.h $(gen)/xdg-shell-client-protocol.h
/home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/video/wayland/SDL_waylandopengles.c: $(gen)/pointer-constraints-unstable-v1-client-protocol.h $(gen)/relative-pointer-unstable-v1-client-protocol.h $(gen)/wayland-client-protocol.h $(gen)/xdg-shell-unstable-v6-client-protocol.h $(gen)/xdg-shell-client-protocol.h
/home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/video/wayland/SDL_waylandtouch.c: $(gen)/pointer-constraints-unstable-v1-client-protocol.h $(gen)/relative-pointer-unstable-v1-client-protocol.h $(gen)/wayland-client-protocol.h $(gen)/xdg-shell-unstable-v6-client-protocol.h $(gen)/xdg-shell-client-protocol.h
/home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/video/wayland/SDL_waylandvideo.c: $(gen)/pointer-constraints-unstable-v1-client-protocol.h $(gen)/relative-pointer-unstable-v1-client-protocol.h $(gen)/wayland-client-protocol.h $(gen)/xdg-shell-unstable-v6-client-protocol.h $(gen)/xdg-shell-client-protocol.h
/home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/video/wayland/SDL_waylandvulkan.c: $(gen)/pointer-constraints-unstable-v1-client-protocol.h $(gen)/relative-pointer-unstable-v1-client-protocol.h $(gen)/wayland-client-protocol.h $(gen)/xdg-shell-unstable-v6-client-protocol.h $(gen)/xdg-shell-client-protocol.h
/home/ian/source/CCOSS/external/linux/SDL2-2.0.9/src/video/wayland/SDL_waylandwindow.c: $(gen)/pointer-constraints-unstable-v1-client-protocol.h $(gen)/relative-pointer-unstable-v1-client-protocol.h $(gen)/wayland-client-protocol.h $(gen)/xdg-shell-unstable-v6-client-protocol.h $(gen)/xdg-shell-client-protocol.h
