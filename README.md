# Cortex Command Community Project Source
*The Cortex Command Community Project is Free/Libre and Open Source under GNU AGPL v3*

This is a community-driven effort to continue the development of Cortex Command.  
Stay up to date in our [Discord channel](https://discord.gg/SdNnKJN).

***

# Windows Build Instructions
First you need to download the necessary files:

1. Install the necessary tools.  
You'll probably want [Visual Studio Community Edition](https://visualstudio.microsoft.com/downloads/) (build supports both 2017 and 2019 versions).  
You also need to have [Visual C++ Redistributable for Visual Studio 2017 (x86)](https://support.microsoft.com/en-us/help/2977003/the-latest-supported-visual-c-downloads) installed in order to run the compiled builds.  
You may also want to check out the list of recommended Visual Studio plugins [here](https://github.com/cortex-command-community/Cortex-Command-Community-Project-Source/wiki/Information,-Recommended-Plugins-and-Useful-Links).

2. Clone this Source Repository and the [Data Repository](https://github.com/cortex-command-community/Cortex-Command-Community-Project-Data) in neighboring folders.  
**Do Not** change the folder names unless you want to make trouble for yourself.

3. Copy the following libraries from `Cortex-Command-Community-Project-Source\external\lib\` into the **Data Repository**:
* `lua51.dll`
* `fmod.dll`
* `liblz4.dll`
* `zlibwapi.dll`

4. Copy `Scenes.rte` and `Metagames.rte` from your purchased copy of Cortex Command into the **Data Repository**.

Now you're ready to build and launch the game.  
Simply open `RTEA.sln` with Visual Studio, choose your configuration, and run the project.

* Use `Debug Full` for debugging with all visual elements enabled (builds fast, runs very slow).
* Use `Debug Minimal` for debugging with all visual elements disabled (builds fast, runs slightly faster).
* Use `Debug Release` for a debugger-enabled release build (builds slow, runs almost as fast as Final).
* Use `Final` to build release executable.

The first build will take a while, but future ones should be quicker.

# Linux Build Instructions
The Linux build uses the meson build system, and builds against system libraries

### Dependencies:

* `g++>=8.1` (needs to support c++17 filesystem)
* `allegro4`
* `loadpng`
* `flac`
* `luajit`
* `minizip`
* `lz4`
* `libpng`
* `libX11`
* `meson>=0.43`
* `boost>=1.55`
* `xorg-misc-fonts`

### Building:

1. Install Dependencies (see below for some distro-specific instructions).

2. Clone this Source Repository and the [Data Respository](https://github.com/cortex-command-community/Cortex-Command-Community-Project-Data)

3. Open a terminal in the Source Repository.

4. `meson builddir`

5. `cd builddir`

For `meson` versions `>=0.54` (check `meson --version`) :

6. `meson compile` for debug build, or `meson compile CCCP` for a release build.  
If the build fails because of memory shortage you may need to reduce the number of build threads (meson will use all available threads by default) using the `-j<number of threads>` option, if this doesn't help increase your swap size to at least 6Gb.

For `meson` versions `<0.54`

6. `ninja` for debug builds, or `ninja CCCP.x86_64` for release builds.  
Using the `-j<number of threads>` option will also work here.

### Running:

1. Copy (or link, might be preferable for testing builds) `builddir/CCCP_debug.x86_64` or `builddir/CCCP.x86_64` (depending on if you made a release build) into the **Data Repository**.

2. Copy `Scenes.rte` and `Metagames.rte` from your purchased copy of Cortex Command into **Data Repository**.

3. Run `./CCCP.x86_64` or `./CCCP_debug.x86_64` in the **Data Repository**.

### Troubleshooting:
Until borderless windows are implemented, you might seem get stuck in fullscreen mode.  
Try Alt-Return, or if that doesn't work kill CC with ctrl-alt-end.

On X11 media keys and such don't work in fullscreen, this is a known issue (this does not happen on Wayland).

If you get errors similar to `ALSA lib ... could not open slave`, try adding:
```
option snd_mia index=0
option snd_intel_hda index=1
```
to `/etc/modprobe.d/alsa-base.conf`

***

**Arch Linux:**
`# pacman -S allegro4 flac luajit minizip lz4 libpng libx11 meson xorg-fonts-misc`

**Ubuntu:**
`# apt-get install liballegro4-dev libloadpng4-dev libflac++-dev luajit-5.1-dev libminizip-dev liblz4-dev libpng++-dev libx11-dev meson`

## More Information

For more information and recommendations, see [here](https://github.com/cortex-command-community/Cortex-Command-Community-Project-Source/wiki/Information,-Recommended-Plugins-and-Useful-Links).