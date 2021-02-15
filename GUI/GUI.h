#ifndef _GUI_
#define _GUI_

#pragma region Rectangle Structure
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
inline void SetRect(GUIRect *rect, int left, int top, int right, int bottom) { rect->left = left; rect->top = top; rect->right = right; rect->bottom = bottom; }
#pragma endregion

#include "RTETools.h"

#include "GUI/Interface.h"
#include "GUI/GUIProperties.h"
#include "GUI/GUIInput.h"
#include "GUI/GUIFont.h"
#include "GUI/GUISkin.h"
#include "GUI/GUIPanel.h"
#include "GUI/GUIManager.h"
#include "GUI/GUIUtil.h"
#include "GUI/GUIControl.h"
#include "GUI/GUIEvent.h"
#include "GUI/GUIControlFactory.h"
#include "GUI/GUIControlManager.h"
#include "GUI/GUISound.h"

#endif