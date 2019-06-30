#ifndef _RTESNPRINTF_H_
#define _RTESNPRINTF_H_

/////////////////////////////////////////////////////////////////////////////////////////
// File:            snprintf.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Portable version of snprintf
// Project:         Retro Terrain Engine
// Author(s):       Chris Kruger
//                  chris.kruger@krugerheavyindustries.com

#if defined(__APPLE__) 
#include <cstdio>
#elif _MSC_VER

#define snprintf c99_snprintf

inline int c99_vsnprintf(char* str, size_t size, const char* format, va_list ap)
{
   int count = -1;
   
   if (size != 0)
      count = _vsnprintf_s(str, size, _TRUNCATE, format, ap);
   if (count == -1)
      count = _vscprintf(format, ap);
   
   return count;
}

inline int c99_snprintf(char* str, size_t size, const char* format, ...)
{
   int count;
   va_list ap;
   
   va_start(ap, format);
   count = c99_vsnprintf(str, size, format, ap);
   va_end(ap);
   
   return count;
}

#endif // _MSC_VER

#endif
