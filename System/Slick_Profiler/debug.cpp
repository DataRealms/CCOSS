/////////////////////////////////////////////////////////////////////
// File    : debug.cpp
// Desc    : 
// Created : Monday, May 20, 2002
// Author  : Jamie Cheng
// 
// (c) 2002 The Beautiful Why?
//

#include "pch.h"

#if defined(_XBOX)
#  include <xtl.h>
#elif defined(__APPLE__) || defined(__unix__)
#include <cstdarg>
#else
#  include <windows.h>
#endif

#ifdef SLICK_PROFILER
#define ENABLE_PRINTF
#endif

#include "debug.h"

//---- GLOBALS ---------------------------------------------

typedef std::vector< dbOutputHandler* > DBHandlers;
DBHandlers dbHandlers;

const unsigned int MAX_MSG = 1024;

void Printf(const char* description, ...)
{	
#ifdef ENABLE_PRINTF

	static CRITICAL_SECTION critical_section;
	static bool bInitOnce = true;
	if (bInitOnce)
	{
		bInitOnce = false;
		InitializeCriticalSection(&critical_section);
	}

	try
	{
		EnterCriticalSection(&critical_section);

		char formatted[ MAX_MSG ];
		char slash_n[MAX_MSG];

		va_list vl;

		va_start( vl, description );

		_vsnprintf( formatted, MAX_MSG, description, vl );

		va_end(vl);		

		formatted[MAX_MSG-1] = '\0';
		
		sprintf_s(slash_n, MAX_MSG, "%s\n", formatted);
		slash_n[MAX_MSG-1] = '\0';

		// Disabled: Daniel, if you're using boost, you can hook this up again
//		if(!dbHandlers.empty())
//			std::for_each(dbHandlers.begin(), dbHandlers.end(), boost::bind( &dbOutputHandler::HandleOutput, _1, formatted ) );

		//Ultra hack. Sleep for 5ms to try and stop a OutputDebugString hang.
		//Sleep(5);
//		OutputDebugString( slash_n );

		LeaveCriticalSection(&critical_section);
	}
	catch (...)
	{
		LeaveCriticalSection(&critical_section);
		Printf("Exception printing \"%s\"\n", description);
	}
#endif
}

void dbOutputHandlerAdd(dbOutputHandler* handler)
{
    dbHandlers.push_back(handler);
}

void dbOutputHandlerRemove(dbOutputHandler* handler)
{
    dbHandlers.erase(std::remove(dbHandlers.begin(), dbHandlers.end(), handler));
}

void FatalFunc(const char* description)
{
	char formatted[ MAX_MSG ];
#if defined(__APPLE__) || defined(__unix__)
	sprintf(formatted, "Fatal: %s", description);
#else
	sprintf_s(formatted, MAX_MSG, "Fatal: %s", description);
#endif // defined(__APPLE__)
#ifdef ENABLE_PRINTF
//	OutputDebugString(formatted);
#endif
#if defined(_MSC_VER_)
#ifndef KLEI_SUBMISSION
    DebugBreak();
#endif
#endif // defined(_MSC_VER_)
#ifdef ENABLE_PRINTF
	//Second output is so that if we connect the debugger after we can still see what the problem was
//	OutputDebugString(formatted);
#endif
}

void Fatalf( const char* description, ...)
{
	char formatted[ MAX_MSG ];

	va_list vl;
    
	va_start( vl, description );
   
#if defined(__APPLE__)	|| defined(__unix__)
	vsnprintf(formatted, MAX_MSG, description, vl);
#else
	_vsnprintf( formatted, MAX_MSG, description, vl );
#endif // defined(__APPLE__) || defined(__unix__)
	
	va_end(vl);
    
	formatted[MAX_MSG-1] = '\0';

	FatalFunc(formatted);
}                                                   

void Warningf( const char* description, ... )
{
#ifdef ENABLE_PRINTF
	try
	{
		char formatted[ MAX_MSG ];

		va_list vl;

		va_start( vl, description );

		_vsnprintf( formatted, MAX_MSG, description, vl );

		va_end(vl);
	    
		formatted[MAX_MSG-1] = '\0';
	    
		Printf("Warning: %s", formatted);
	}
	catch (...)
	{
		Printf("Exception printing warning: \"%s\"\n", description);
	}
#endif
}

#if defined(TBW_DEBUG) || defined(_DEBUG) || defined(SLICK_PROFILER)

//----------------------------------------------------------

void AssertFunc(int line, const char* file)
{
	// format debug message
	char message[ 1024 ];
    sprintf(message,"Assert failure: %d, %s\n", line, file);
    Printf( message );

    return;
}

#endif //TBW_DEBUG
