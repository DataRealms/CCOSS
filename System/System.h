#ifndef _RTE_SYSTEM_H_
#define _RTE_SYSTEM_H_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            System.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Header file for the cross platform system functionality.
// Project:         Retro Terrain Engine
// Author(s):       Chris Kruger
//                  c/- Daniel Tabar (data@datarealms.com)
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files, forward declarations, namespace stuff


namespace RTE
{

class System
{

//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable, method and friend function declarations

public:

//////////////////////////////////////////////////////////////////////////////////////////
// Method:  ChangeWorkingDirectory
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Some platforms have by design their data seperated from the executable. 
//					As many serializable classes have fixed data paths we need to change the 
//					working directory so that the data can be found. Currently a call to this
//					method will do the correct thing for the platform in question. It needs to be 
//					called on application startup before any file io takes place.
// Arguments:       None.
// Return value:    None.

	void ChangeWorkingDirectory();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  GetWorkingDirectory
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns current working directory.
// Arguments:       None.
// Return value:    Absolute path to current working directory.
	std::string GetWorkingDirectory();
	
//////////////////////////////////////////////////////////////////////////////////////////
// Method:  MakeDirectory
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Create a directory. Returns 0 if successful, POSIX compliant errorcode if error.
// Arguments:       path to create.
// Return value:    Returns 0 if successful, POSIX compliant errorcode if error.	
	int MakeDirectory(const std::string& path);

//////////////////////////////////////////////////////////////////////////////////////////
// Private member variable and method declarations

	
	
private:

};

extern System g_System;

}

#endif // _RTE_SYSTEM_H_
