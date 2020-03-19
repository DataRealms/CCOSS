//////////////////////////////////////////////////////////////////////////////////////////
// File:            AllegroScreen.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     AllegroScreen class
// Project:         GUI Library
// Author(s):       Jason Boettcher
//                  jackal@shplorb.com
//                  www.shplorb.com/~jackal


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "GUI.h"
#include "AllegroScreen.h"
#include "AllegroBitmap.h"

using namespace RTE;


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     AllegroScreen
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a AllegroScreen object in system
//                  memory.

AllegroScreen::AllegroScreen(BITMAP *pBackBuffer)
{
    // Create the bitmap interface from the screen's back buffer
    m_pBackBitmap = new AllegroBitmap(pBackBuffer);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroy the screen

void AllegroScreen::Destroy(void)
{
    delete m_pBackBitmap;
    m_pBackBitmap = 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CreateBitmap
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a bitmap from a file

GUIBitmap * AllegroScreen::CreateBitmap(const std::string Filename)
{
    AllegroBitmap *pABitmap = new AllegroBitmap();
    if (!pABitmap)
        return 0;

    // Load from file
    if (!pABitmap->Create(Filename)) {
        delete pABitmap;
        return 0;
    }

    return pABitmap;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CreateBitmap
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates an empty bitmap

GUIBitmap * AllegroScreen::CreateBitmap(int Width, int Height)
{
    AllegroBitmap *pABitmap = new AllegroBitmap();
    if (!pABitmap)
        return 0;

    // Create it, using the same color depth as the screen
    if (!pABitmap->Create(Width, Height, bitmap_color_depth(m_pBackBitmap->GetBitmap()))) {
        delete pABitmap;
        return 0;
    }

    return pABitmap;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          DrawBitmap
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws a bitmap onto the back buffer

void AllegroScreen::DrawBitmap(GUIBitmap *pGUIBitmap, int destX, int destY, GUIRect *pRect)
{
    if (!pGUIBitmap)
        return;

    BITMAP *pSourceBitmap = ((AllegroBitmap *)pGUIBitmap)->GetBitmap();
       
    if (!pSourceBitmap)
        return;

    if (pRect)
        blit(pSourceBitmap, m_pBackBitmap->GetBitmap(), pRect->left, pRect->top, destX, destY, pRect->right - pRect->left, pRect->bottom - pRect->top);
    else
        blit(pSourceBitmap, m_pBackBitmap->GetBitmap(), 0, 0, destX, destY, pSourceBitmap->w, pSourceBitmap->h);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          DrawBitmapTrans
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws a bitmap onto the back buffer using the colorkey.

void AllegroScreen::DrawBitmapTrans(GUIBitmap *pGUIBitmap, int destX, int destY, GUIRect *pRect)
{
    if (!pGUIBitmap)
        return;

    BITMAP *pSourceBitmap = ((AllegroBitmap *)pGUIBitmap)->GetBitmap();
       
    if (!pSourceBitmap)
        return;

    if (pRect)
        masked_blit(pSourceBitmap, m_pBackBitmap->GetBitmap(), pRect->left, pRect->top, destX, destY, pRect->right - pRect->left, pRect->bottom - pRect->top);
    else
        masked_blit(pSourceBitmap, m_pBackBitmap->GetBitmap(), 0, 0, destX, destY, pSourceBitmap->w, pSourceBitmap->h);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetBitmap
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets a bitmap representing the screen.

GUIBitmap * AllegroScreen::GetBitmap(void)
{
    return m_pBackBitmap;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ConvertColor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Converts an 8bit palette index to a valid pixel format.
//                  Primarily used for development in windowed mode.
   
unsigned long AllegroScreen::ConvertColor(unsigned long color, int targetDepth)
{
    // If no target to shoot for, use the current video mode depth
    if (targetDepth == 0)
        targetDepth = get_color_depth();

    // We need to end up with palettized color
    if (targetDepth == 8)
    {
        // Isn't indexed, don't convert
        if (!(color >= 0 && color <= 255))
            color = makecol8(getr32(color), getg32(color), getb32(color));            
    }
    // We need a 32bpp color
    else
    {
        // Indexed? Convert
        if (color >= 0 && color <= 255)
        {
            RGB rgbEntry;
            get_color(color, &rgbEntry);
            // Times 4 because RGB struct's elements are in range 0-63, and makecol needs 0-255
            color = makecol(rgbEntry.r * 4, rgbEntry.g * 4, rgbEntry.b * 4);
        }
    }

    // Return converted color
    return color;
}