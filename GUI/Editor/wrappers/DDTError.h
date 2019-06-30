#ifndef _DDTERROR_
#define _DDTERROR_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            DDTError.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Source file for general error handling functions.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include <cstdio>
#include <string>


namespace RTE
{

//////////////////////////////////////////////////////////////////////////////////////////
// Global function: DDTAbortFunc
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Abort on Error function. Will try to dump a screenshot, show an
//                  abortion message, and then quit the program immediately.
// Arguments:       The description of the abortion.
//                  The source file in which the abortion is made.
//                  The line where the abortion is made.
// Return value:    None.

extern bool DDTAbortFunc(const char *description,
                         const char *file,
                         int line);

extern bool DDTAbortFunc(const std::string description,
                         const char *file,
                         int line);

#ifdef WIN32

    #define DDTAbort(description)                              \
    {                                                       \
        if (DDTAbortFunc(description, __FILE__, __LINE__))      \
        {                                                   \
            _asm { int 3 }                                  \
        }                                                   \
    }

#else // WIN32

    #define DDTAbort(description)                              \
    {                                                       \
        if (DDTAbortFunc(description, __FILE__, __LINE__))      \
        {                                                   \
        }                                                   \
    }

#endif // WIN32


//////////////////////////////////////////////////////////////////////////////////////////
// Global function: DDTAssert
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     A souped-up, customized assert function that brings up a nice dialog
//                  box on assert failure, where the user can choose to: break, ignore
//                  the particular assertion failure once, or to always ignore.
// Arguments:       The expression that will be asserted to be true.
//                  The description of the assertion.
//                  The source file in which the assertion is made.
//                  The line where the assertion is made.
//                  A referece to a bool that is used in an "Always ignore" functionality.
// Return value:    Whether the assertion failed AND the user chose to break in the dialog
//                  box.

extern bool DDTAssert(bool expression,
                      const char *description,
                      const char *file,
                      int line,
                      bool &alwaysIgnore);

extern bool DDTAssert(bool expression,
                      const std::string description,
                      const char *file,
                      int line,
                      bool &alwaysIgnore);

// Always-exists version of Assert
#ifdef WIN32

    #define AAssert(expression, description)                                                     \
    {                                                                                           \
        static bool alwaysIgnore = false;                                                       \
        if (!alwaysIgnore)                                                                      \
        {                                                                                       \
            if (DDTAssert((int)(expression), description, __FILE__, __LINE__, alwaysIgnore))    \
            {                                                                                   \
                _asm { int 3 }                                                                  \
            }                                                                                   \
        }                                                                                       \
    }

#else // WIN32

    #define AAssert(expression, description)                                                     \
    {                                                                                           \
        static bool alwaysIgnore = false;                                                       \
        if (!alwaysIgnore)                                                                      \
        {                                                                                       \
            if (DDTAssert((intptr_t)(expression), description, __FILE__, __LINE__, alwaysIgnore))    \
            {                                                                                   \
            }                                                                                   \
        }                                                                                       \
    }

#endif // WIN32

// Debug-only version of Assert
#ifdef _DEBUG

    #ifdef WIN32

        #define DAssert(expression, description)                                                    \
        {                                                                                           \
            static bool alwaysIgnore = false;                                                       \
            if (!alwaysIgnore)                                                                      \
            {                                                                                       \
                if (DDTAssert((int)(expression), description, __FILE__, __LINE__, alwaysIgnore))    \
                {                                                                                   \
                    _asm { int 3 }                                                                  \
                }                                                                                   \
            }                                                                                       \
        }

    #else // WIN32

        #define DAssert(expression, description)                                                    \
        {                                                                                           \
            static bool alwaysIgnore = false;                                                       \
            if (!alwaysIgnore)                                                                      \
            {                                                                                       \
                if (DDTAssert((int)(expression), description, __FILE__, __LINE__, alwaysIgnore))    \
                {                                                                                   \
                }                                                                                   \
            }                                                                                       \
        }

    #endif // WIN32

#else // _DEBUG

    #define DAssert(expression, description)

#endif // _DEBUG

} // Namespace RTE

#endif // File
