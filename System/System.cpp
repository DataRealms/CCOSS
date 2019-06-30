#include "System.h"

#ifdef _WIN32
    #include <direct.h>
    #define getcwd _getcwd
#else
    #include <unistd.h>
	 #include <sys/stat.h>
#endif
#if defined(__APPLE__)
#include <limits.h>
#include <CoreFoundation/CoreFoundation.h>
#endif // defined(__APPLE__)

namespace RTE
{

System g_System;

void System::ChangeWorkingDirectory()
{

#if defined(__APPLE__)

	char path[PATH_MAX];
	
	CFBundleRef mainBundle;
	CFURLRef resourceDirectoryURL;

	mainBundle = CFBundleGetMainBundle();
	resourceDirectoryURL = CFBundleCopyResourcesDirectoryURL(mainBundle);
	CFURLGetFileSystemRepresentation(resourceDirectoryURL, true, (UInt8 *) path, PATH_MAX);
	CFRelease(resourceDirectoryURL);
	
	chdir(path);
	
#endif // defined(__APPLE__) 
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:  GetWorkingDirectory
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns current working directory.
std::string System::GetWorkingDirectory()
{
	char cwd[1024];
	if (getcwd(cwd, sizeof(cwd)))
		return std::string(cwd);
	return std::string(".");
}
	
//////////////////////////////////////////////////////////////////////////////////////////
// Method:  MakeDirectory
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Create a directory on disk, return error code if any
int System::MakeDirectory(const std::string& path)
{
#ifdef _WIN32
	return _mkdir(path.c_str());
#else
	return  mkdir(path.c_str(), S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
#endif 
}
	
}
