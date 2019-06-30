/////////////////////////////////////////////////////////////////////
// File    : Debug.h
// Desc    : For debugging tools
// Created : Wednesday, February 27, 2002
// Author  : Jamie Cheng
// 
// (c) 2002 The Beautiful Why
//

#pragma once

#ifndef TBW__15092002_JCHENG_DEBUG_H
#define TBW__15092002_JCHENG_DEBUG_H

#if defined(SLICK_PROFILER) || defined(_DEBUG)
#define TBW_DEBUG 
#endif

// FlipCode - Tip Of The Day
//	by Alberto García-Baquero Vega, posted on 11 January 2001 
#ifndef NULL
#define NULL ((void *) 0)
#endif

#define _QUOTE(x) # x
#define QUOTE(x) _QUOTE(x)
#define __FILE__LINE__ __FILE__ "(" QUOTE(__LINE__) ") :"
#define __FUNCTION__LINE__ __FUNCTION__ "(" QUOTE(__LINE__) ") :"

#define FIXME( x )	message( __FILE__LINE__" FIXME : " #x "\n" ) 
#define TODO( x )	message( __FILE__LINE__" TODO : " #x "\n" ) 

//force the use of out SlickAssert
#ifdef assert
#undef assert
#endif

//	MFC generated code uses ASSERT
#ifndef __AFX_H__
	#ifdef ASSERT
	#undef ASSERT
	#endif
#endif

//inherit from unary_function to make function object adaptable
struct dbOutputHandler
{
	virtual ~dbOutputHandler() {};
    virtual bool HandleOutput(const char* message) = 0;
};

//use these to get your handler to handle the output
void dbOutputHandlerAdd(dbOutputHandler* handler);
void dbOutputHandlerRemove(dbOutputHandler* handler);

void Printf(const char*, ...);
void Warningf( const char*, ... );
void Fatalf( const char* description, ...);

#if defined(TBW_DEBUG) || defined(SLICK_PROFILER)

void AssertFunc(int, const char*);

#define Break() { Printf("Break at %s", __FILE__LINE__); DebugBreak(); }

#define SlickAssert(exp)                                 \
if( !(exp) ) {                                        \
    AssertFunc(__LINE__, __FILE__);                 \
    { /*DebugBreak();*/ }                              \
}

#else

#define Break()
#define SlickAssert(exp)

#endif //TBW_DEBUG

#endif //TBW__15092002_JCHENG_DEBUG_H