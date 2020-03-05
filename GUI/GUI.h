#ifndef _GUI_
#define _GUI_

#include "RTETools.h"

/// <summary>
/// The GUIRect structure defines a rectangle by the coordinates of its upper-left and lower-right corners.
/// </summary>
struct GUIRect { long left; long top; long right; long bottom; };

/// <summary>
/// Sets the bounds of a GUIRect.
/// </summary>
/// <param name="pRect">Pointer to the GUIRect.</param>
/// <param name="left">Position of top left corner on X axis.</param>
/// <param name="top">Position of top left corner on Y axis.</param>
/// <param name="right">Position of bottom right corner on X axis.</param>
/// <param name="bottom">Position of bottom right corner on Y axis.</param>
void SetRect(GUIRect *pRect, int left, int top, int right, int bottom);

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

#endif