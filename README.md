# Cortex Command Community Project Source
*The Cortex Command Community Project is Free/Libre and Open Source under GNU AGPL v3*

[![Meson Build (Linux, macOS)](https://github.com/cortex-command-community/Cortex-Command-Community-Project-Source/actions/workflows/meson.yml/badge.svg)](https://github.com/cortex-command-community/Cortex-Command-Community-Project-Source/actions/workflows/meson.yml) [![Windows Build](https://github.com/cortex-command-community/Cortex-Command-Community-Project-Source/actions/workflows/msbuild.yml/badge.svg)](https://github.com/cortex-command-community/Cortex-Command-Community-Project-Source/actions/workflows/msbuild.yml)

This is a community-driven effort to continue the development of Cortex Command.  
Stay up to date in our [Discord channel](https://discord.gg/TSU6StNQUG).

***

# Installing the Game
If you just want to play the latest version of the game you can get it from our [website](https://cortex-command-community.github.io/downloads).

# Getting Mods
You can get mods from our [mod portal](https://cccp.mod.io).

***

# Windows Build Instructions
First you need to download the necessary files:

1. Install the necessary tools.  
You'll probably want [Visual Studio Community Edition](https://visualstudio.microsoft.com/downloads/) (build supports both 2017 and 2019 versions).  
You also need to have both x86 and x64 versions of the [Visual C++ Redistributable for Visual Studio 2017](https://support.microsoft.com/en-us/help/2977003/the-latest-supported-visual-c-downloads) installed in order to run the compiled builds.  
You may also want to check out the list of recommended Visual Studio plugins [here](https://github.com/cortex-command-community/Cortex-Command-Community-Project-Source/wiki/Information,-Recommended-Plugins-and-Useful-Links).

2. Clone this Source Repository and the [Data Repository](https://github.com/cortex-command-community/Cortex-Command-Community-Project-Data) in neighboring folders.  
**Do Not** change the folder names unless you want to make trouble for yourself.

3. Copy the following libraries from `Cortex-Command-Community-Project-Source\external\lib\win` into the **Data Repository**:
* `fmod.dll`
* `SDL2-64.dll`
For 32-bit builds, copy the following libraries from the `x86` folder inside `...\lib\win`:
* `fmodL.dll`
* `SDL2.dll`

Now you're ready to build and launch the game.  
Simply open `RTEA.sln` with Visual Studio, choose your target platform (x86 or x64) and configuration, and run the project.

* Use `Debug Full` for debugging with all visual elements enabled (builds fast, runs very slow).
* Use `Debug Minimal` for debugging with all visual elements disabled (builds fast, runs slightly faster).
* Use `Debug Release` for a debugger-enabled release build (builds slow, runs almost as fast as Final. **Debugging may be unreliable due to compiler optimizations**).
* Use `Final` to build release executable.

The first build will take a while, but future ones should be quicker.

If you want to use an IDE other than Visual Studio, you will have to build using meson. Check the [Linux](#building) and [Installing Dependencies](#installing-dependencies) section for pointers.

## Windows Subsystem for Linux (WSL)

The Linux build can be built and run on Windows 10 using WSL by following the Linux [building](#building) and [running](#running) instructions.  
Information on installing and using WSL can be found [here](https://learn.microsoft.com/en-us/windows/wsl/install).

Building can be done directly from the Windows filesystem side, without having to clone the repositories on the Linux filesystem side.  
By default WSL will mount your `C:` drive to `/mnt/c/`, or just `/c/`. From there you can navigate to the Source and Data directories to follow the meson build steps.

This has been tested with WSL2 Ubuntu 22.04 but should work with other distributions and WSL1 as well.

***

# Linux and macOS Build Instructions
The Linux build uses the meson build system, and builds against system libraries.

## Dependencies

* `gcc`, `g++` (>=9, clang unsupported) 
* `sdl2`
* `flac`
* `luajit`
* `lua` (maybe optional)
* `minizip`
* `tbb`
* `lz4>=1.9.0`
* `libpng`
* [`meson`](https://www.mesonbuild.com)`>= 1.0.0` (`pip install meson` if your distro doesn't include a recent version)

For unspecified versions assume compatibility with the latest ubuntu LTS release.

## Building

1. Install Dependencies (see [below](#installing-dependencies) for instructions).

2. Clone this Source Repository and the [Data Respository](https://github.com/cortex-command-community/Cortex-Command-Community-Project-Data).

3. Open a terminal in the Source Repository.

4. `meson setup build` or `meson setup --buildtype=debug build` for debug build (default is release build)  
	For macOS you need to specify gcc, with `env CC=gcc-12 CXX=g++-12 meson setup build`

5. `ninja -C build`

6. (optional) `sudo ninja install -C build` (To uninstall later, keep the build directory intact. The game can then be uninstalled by `sudo ninja uninstall -C build`)

If you want to change the buildtype afterwards, you can use `meson configure --buildtype {release or debug}` in the build directory or create a secondary build directory as in Step 4. There are also additional build options documented in the [wiki](https://github.com/cortex-command-community/Cortex-Command-Community-Project-Source/wiki/Meson-build-options) as well as through running `meson configure` in the build directory.

## Running
(If you installed the game in step 6 above, it should appear with your regular applications and will just run)

1. Copy (or link, might be preferable for testing builds) `build/CortexCommand` or `build/CortexCommand_debug` (depending on if you made a debug build) into the **Data Repository**.

   `cd $DATA_REPOSITORY; ln -s ../Cortex-Command-Community-Project-Source/build/CortexCommand . `

2. Copy all `libfmod` files from `external/lib/linux/x86_64` into the **Data Repository**.

   `cd $DATA_REPOSITORY; ln -s ../Cortex-Command-Community-Project-Source/external/lib/linux/x86_64/libfmod.so* .`

4. Run `./CortexCommand` or `./CortexCommand_debug` in the **Data Repository**.

## Installing Dependencies

**macOS additional dependencies:**  
- `brew` [brew.sh](https://brew.sh) (or any other package manager)  
- `Xcode` or `Command Line Tools for Xcode` (if you need to, you can also generate an xcode project from meson using the `--backend=xcode` option on setup)

**Homebrew (macOS):**  
`brew install pkg-config sdl2 minizip lz4 flac luajit lua libpng tbb gcc@12 ninja meson`

**Arch Linux:**  
`sudo pacman -S sdl2 tbb flac luajit lua minizip lz4 libpng meson ninja base-devel`  

**Ubuntu >=22.04:**  
`sudo apt-get install build-essential libsdl2-dev libloadpng4-dev libflac++-dev luajit-5.1-dev liblua5.1-dev libminizip-dev liblz4-dev libpng++-dev libtbb-dev ninja-build python3-pip`  
`sudo python3 -m pip install meson`

**Fedora:**  
`# dnf install allegro-loadpng-devel allegro-devel libsdl2-devel lua-devel boost-devel meson ninja-build flac-devel luajit-devel minizip-compat-devel tbb-devel lz4-devel libpng-devel lua-devel gcc gcc-c++`  

## Troubleshooting

* older versions of `pipewire(-alsa)` and fmod don't work well together, so the game might [not close, have no sound or crash](https://gitlab.freedesktop.org/pipewire/pipewire/-/issues/1514). Workaround by `ln -s /bin/true /usr/bin/pulseaudio`

***

**Windows 10 (64-bit) without Visual Studio**  
- [Windows SDK](https://developer.microsoft.com/de-de/windows/downloads/windows-10-sdk/)
- [Clang Toolset](https://github.com/llvm/llvm-project/releases) (Grab the latest LLVM-...-win64.exe)
- [git](https://www.git-scm.org)
- [meson](https://github.com/mesonbuild/meson/releases) (documentation [here](https://www.mesonbuild.com))
- (optional) Visual Studio for the Developer Consoles since setup otherwise may be unnecessarily hard

***

# More Information

See the [Information and Recommendations](https://github.com/cortex-command-community/Cortex-Command-Community-Project-Source/wiki/Information,-Recommended-Plugins-and-Useful-Links) page for more details and useful development tools.
