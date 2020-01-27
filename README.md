# Cortex Command Community Project Source #
*The Cortex Command Community Project is Free/Libre and Open Source under GNU AGPL v3*

This is a community-driven effort to continue the development of Cortex Command. Stay up to date in our [discord channel](https://discord.gg/SdNnKJN).

***

## Setup Process ##

First you need to download the necessary files:

1. Install the necessary tools.  
You'll probably want [Visual Studio Community Edition](https://visualstudio.microsoft.com/downloads/) (we mostly use 2017 but 2019 should be fine).  
You also need to have [Visual C++ Redistributable for Visual Studio 2015 (x86)](https://www.microsoft.com/en-us/download/confirmation.aspx?id=48145&6B49FDFB-8E5B-4B07-BC31-15695C5A2143=1) installed in order to run the compiled builds.  
You may also want to check out the list of recommended Visual Studio plugins [here](https://github.com/cortex-command-community/Cortex-Command-Community-Project-Source/wiki/Information,-Recommended-Plugins-and-Useful-Links).

2. Clone this Source Repository and the [Data Repository](https://github.com/cortex-command-community/Cortex-Command-Community-Project-Data) in neighboring folders.  
**Do Not** change the folder names unless you want to make trouble for yourself.

3. Copy `Cortex-Command-Community-Project-Source\external\sources\zlib\DLLs\dll32\zlibwapi.dll` and `Cortex-Command-Community-Project-Source\external\lib\win\lua51.dll` into the Data Repository

4. Copy `Scenes.rte` and `Metagames.rte` from your purchased copy of Cortex Command into the Data Repository.

Now you're ready to build and launch the game.  
Simply open `RTEA.sln` with Visual Studio, choose your configuration, and run the project.

* Use "`Debug Open Source`" configuration to debug (this runs very slowly)
* Use "`Minimal Debug Open Source`" configuration to debug with all visual debug elements disabled (this runs slightly faster)
* Use "`Final Open Source`" configuration to build release .exe

The first build will take a while, but future ones should be quicker.

***

## More Information ##

For more information and recommendations, see [here](https://github.com/cortex-command-community/Cortex-Command-Community-Project-Source/wiki/Information,-Recommended-Plugins-and-Useful-Links).
