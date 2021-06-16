//////////////////////////////////////////////////////////////////////////////////////////
// File:            AllegroBitmap.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     AllegroBitmap class
// Project:         GUI Library
// Author(s):       Jason Boettcher
//                  jackal@shplorb.com
//                  www.shplorb.com/~jackal


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "GUI.h"
#include "AllegroBitmap.h"
#include "RTEError.h"

using namespace RTE;


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     AllegroBitmap
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a AllegroBitmap object in 
//                  system memory.

AllegroBitmap::AllegroBitmap()
{
    m_BitmapFile.Reset();
    m_pBitmap = nullptr;
    m_SelfCreated = false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     AllegroBitmap
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a AllegroBitmap object in 
//                  system memory.

AllegroBitmap::AllegroBitmap(BITMAP *pBitmap)
{
    m_BitmapFile.Reset();
    m_pBitmap = pBitmap;
    m_SelfCreated = false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a blank bitmap

bool AllegroBitmap::Create(int Width, int Height, int Depth)
{
    m_SelfCreated = true;

    m_BitmapFile.Reset();
    m_pBitmap = create_bitmap_ex(Depth, Width, Height);

    if (!m_pBitmap)
    {
        return false;
    }
    
    return true;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates the bitmap from a filename

bool AllegroBitmap::Create(const std::string Filename)
{
    m_SelfCreated = false;

    m_BitmapFile.Create(Filename.c_str());

    m_pBitmap = m_BitmapFile.GetAsBitmap();//COLORCONV_8_TO_32 | COLORCONV_24_TO_32);
    RTEAssert(m_pBitmap, "Could not load bitmap from file into AllegroBitmap!");

    return true;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and frees the bitmap

void AllegroBitmap::Destroy(void)
{
    if (m_SelfCreated && m_pBitmap)
        destroy_bitmap(m_pBitmap);

    m_pBitmap = nullptr;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draw a section of this bitmap onto another bitmap

void AllegroBitmap::Draw(GUIBitmap *pDestBitmap, int X, int Y, GUIRect *pRect)
{
    if (!m_pBitmap)
        return;

    RTEAssert(pDestBitmap && ((AllegroBitmap *)pDestBitmap)->GetBitmap(), "Null destination bitmap passed when trying to draw AllegroBitmap");

    if (pRect)
        blit(m_pBitmap, ((AllegroBitmap *)pDestBitmap)->GetBitmap(), pRect->left, pRect->top, X, Y, pRect->right - pRect->left, pRect->bottom - pRect->top);
    else
        blit(m_pBitmap, ((AllegroBitmap *)pDestBitmap)->GetBitmap(), 0, 0, X, Y, pDestBitmap->GetWidth(), pDestBitmap->GetHeight());
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          DrawTrans
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draw a section of this bitmap onto another bitmap ignoring 
//                  color-keyed pixels

void AllegroBitmap::DrawTrans(GUIBitmap *pDestBitmap, int X, int Y, GUIRect *pRect)
{
    if (!m_pBitmap)
        return;

    RTEAssert(pDestBitmap && ((AllegroBitmap *)pDestBitmap)->GetBitmap(), "Null destination bitmap passed when trying to draw AllegroBitmap");

    if (pRect)
        masked_blit(m_pBitmap, ((AllegroBitmap *)pDestBitmap)->GetBitmap(), pRect->left, pRect->top, X, Y, pRect->right - pRect->left, pRect->bottom - pRect->top);
    else
        masked_blit(m_pBitmap, ((AllegroBitmap *)pDestBitmap)->GetBitmap(), 0, 0, X, Y, pDestBitmap->GetWidth(), pDestBitmap->GetHeight());
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  DrawTransScaled
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draw this bitmap scaled onto another bitmap ignoring color-keyed pixels.

void AllegroBitmap::DrawTransScaled(GUIBitmap *pDestBitmap, int X, int Y, int width, int height)
{
    if (!m_pBitmap)
        return;

    RTEAssert(pDestBitmap && ((AllegroBitmap *)pDestBitmap)->GetBitmap(), "Null destination bitmap passed when trying to draw AllegroBitmap");

    stretch_sprite(((AllegroBitmap *)pDestBitmap)->GetBitmap(), m_pBitmap, X, Y, width, height);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          DrawLine
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws a line.

void AllegroBitmap::DrawLine(int x1, int y1, int x2, int y2, unsigned long Color)
{
    if (!m_pBitmap)
        return;

    line(m_pBitmap, x1, y1, x2, y2, Color);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          DrawRectangle
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws a rectangle.

void AllegroBitmap::DrawRectangle(int X, int Y, int Width, int Height, unsigned long Color, bool Filled)
{
    if (!m_pBitmap)
        return;

    if (Filled)
        rectfill(m_pBitmap, X, Y, X + Width - 1, Y + Height - 1, Color);
    else
        rect(m_pBitmap, X, Y, X + Width - 1, Y + Height - 1, Color);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetPixel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the colour of a pixel at a specific point.
// Arguments:       Point.

unsigned long AllegroBitmap::GetPixel(int X, int Y)
{
    if (!m_pBitmap)
        return 0;

    RTEAssert(m_pBitmap, "GUI Bitmap is null; can't get pixel");

//    m_pBitmap->Lock();

    unsigned long col = getpixel(m_pBitmap, X, Y);

//    m_pBitmap->UnLock();

    return col;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetPixel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the color of a pixel at a specific point.

void AllegroBitmap::SetPixel(int X, int Y, unsigned long Color)
{
    if (!m_pBitmap)
        return;

    RTEAssert(m_pBitmap, "Trying to set a pixel on a null bitmap!");

//    m_pBitmap->Lock();
    putpixel(m_pBitmap, X, Y, Color);
//    m_pBitmap->UnLock();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetWidth
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the Width of the bitmap.

int AllegroBitmap::GetWidth(void)
{
    if (!m_pBitmap)
        return 0;

    return m_pBitmap->w;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetHeight
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the Height of the bitmap.

int AllegroBitmap::GetHeight(void)
{
    if (!m_pBitmap)
        return 0;

    return m_pBitmap->h;
}

/* NA
//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetColorKey
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the color key of the bitmap.

void AllegroBitmap::SetColorKey(unsigned long Key)
{
    if (m_pBitmap)
        m_pBitmap->SetColorKey(Key);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetColorKey
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the color key of the bitmap.

void AllegroBitmap::GetColorKey()
{
    if (m_pBitmap)
        m_pBitmap->SetColorKey();
}
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  GetColorDepth
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the number of bits per pixel color depth of this bitmap.

int AllegroBitmap::GetColorDepth()
{
    if (m_pBitmap)
        return bitmap_color_depth(m_pBitmap);
    return 8;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  GetClipRect
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the clipping rectangle of the bitmap.

void AllegroBitmap::GetClipRect(GUIRect *pRect)
{
    if (m_pBitmap && pRect)
    {
        int x1, y1, x2, y2;
        get_clip_rect(m_pBitmap, &x1, &y1, &x2, &y2);
        pRect->left = x1;
        pRect->top = y1;
        pRect->right = x2;
        pRect->bottom = y2;
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetClipRect
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the clipping rectangle of the bitmap.

void AllegroBitmap::SetClipRect(GUIRect *pRect)
{
    if (m_pBitmap)
    {
        set_clip_state(m_pBitmap, 1);

        if (!pRect)
        {
            set_clip_rect(m_pBitmap, 0, 0, m_pBitmap->w - 1, m_pBitmap->h - 1);
        }
        else
        {
            set_clip_rect(m_pBitmap, pRect->left, pRect->top, pRect->right, pRect->bottom);
        }
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  AddClipRect
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the clipping rectangle of the specified bitmap as the
//                  intersection of its current clipping rectangle and the rectangle
//                  described by the passed-in rect. 

void AllegroBitmap::AddClipRect(GUIRect *pRect)
{
    if (m_pBitmap)
    {
        set_clip_state(m_pBitmap, 1);

        if (!pRect)
        {
            set_clip_rect(m_pBitmap, 0, 0, m_pBitmap->w - 1, m_pBitmap->h - 1);
        }
        else
        {
            add_clip_rect(m_pBitmap, pRect->left, pRect->top, pRect->right, pRect->bottom);
        }
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetDataPath
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns the path to the datafile object this GUIBitmap uses.
// Arguments:       None.

string AllegroBitmap::GetDataPath()
{
    return m_BitmapFile.GetDataPath();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AllegroBitmap::SetBitmap(BITMAP *newBitmap) {
    m_BitmapFile.Reset();
    m_pBitmap = newBitmap;
    m_SelfCreated = false;
}