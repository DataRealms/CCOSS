#include "System.h"

    #include <direct.h>
    #define getcwd _getcwd

namespace RTE
{

System g_System;

void System::ChangeWorkingDirectory()
{
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
	return _mkdir(path.c_str());
}
	
}
