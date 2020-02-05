#ifndef _GUI_
#define _GUI_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            GUI.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Main header file for the GUI library
// Project:         GUI Library
// Author(s):       Jason Boettcher
//                  jackal@shplorb.com
//                  www.shplorb.com/~jackal


//////////////////////////////////////////////////////////////////////////////////////////
// GUI Types
typedef unsigned long Uint32;

struct RECT
{
    long    left;
    long    top;
    long    right;
    long    bottom;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "allegro.h"
#include "Constants.h"
#include "GUIProperties.h"
#include "GUIInput.h"
#include "GUIBitmap.h"
#include "GUIScreen.h"
#include "GUIFont.h"
#include "GUISkin.h"
#include "GUIPanel.h"
#include "GUIManager.h"
#include "GUIUtil.h"
#include "GUIControl.h"
#include "GUIEvent.h"
#include "GUIControlFactory.h"
#include "GUIControlManager.h"

//////////////////////////////////////////////////////////////////////////////////////////
// GUI Library macros

void SetRect(RECT *pRect,
             int left,
             int top,
             int right,
             int bottom);


int DDTstricmp(const char *s1, const char *s2);

#endif