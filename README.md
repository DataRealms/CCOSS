# Cortex Command Community Project Source #
*The Cortex Command Community Project is Free/Libre and Open Source under GNU AGPL v3*

This is a community-driven effort to continue the development of Cortex Command. Stay up to date in our [discord channel](https://discord.gg/SdNnKJN).

***

# Setup Process #

First you need to download the necessary files:
1. Install the necessary tools. You'll probably want [Visual Studio Community Edition](https://visualstudio.microsoft.com/downloads/) (we mostly use 2017 but 2019 should be fine) and, if you're using that, you'll need [Visual C++ 2008 Express Edition With SP1 - ENU](https://www.dropbox.com/s/vhd0sd4bwczg1un/Visual_C_2008_Express_With_SP1.exe?dl=0) to use the debugger. You may also want to check out the [list of recommended Visual Studio plugins](https://github.com/cortex-command-community/Cortex-Command-Community-Project-Source/wiki/Information,-Recommended-Plugins-and-Useful-Links).

2. Clone this Source Repository and the [Data Repository](https://github.com/cortex-command-community/Cortex-Command-Community-Project-Data) in neighbouring folders. **Do Not** change the folder names unless you want to make trouble for yourself.

3. Copy *Cortex-Command-Community-Project-Source\external\sources\zlib\DLLs\dll32\zlibwapi.dll* and *Cortex-Command-Community-Project-Source\external\lib\win\lua51.dll* into the Data Repository

4. Copy *Scenes.rte* and *Metagames.rte* from your purchased copy of Cortex Command into the Data Repository.

Now you're ready to build and run the game. Simply open RTEA.sln with Visual Studio, choose your configuration, and run the project.
* Use "Debug Open Source" configuration to debug (be prepared, it's very slow).
* Use "Final Open Source" configuration to build release .exe.

***

# More Information #

For more information and recommendations, see [here](https://github.com/cortex-command-community/Cortex-Command-Community-Project-Source/wiki/Information,-Recommended-Plugins-and-Useful-Links).
