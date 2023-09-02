#!/bin/sh -eu

dylibbundler -od -b -x "${MESON_INSTALL_DESTDIR_PREFIX}/Contents/MacOS/${1}" \
 -d "${MESON_INSTALL_DESTDIR_PREFIX}/Contents/Frameworks" \
 -p "@executable_path/../Frameworks" \
 -s "${MESON_SOURCE_ROOT}/external/lib/macos"