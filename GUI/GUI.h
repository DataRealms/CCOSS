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
#include "GUI/Interface.h"
#include "GUIProperties.h"
#include "GUIInput.h"
#include "GUISound.h"
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

#endif