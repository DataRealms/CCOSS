//////////////////////////////////////////////////////////////////////////////////////////
// File:            SceneLayer.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Source file for the SceneLayer class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "SceneLayer.h"
#include "ContentFile.h"

namespace RTE {

ConcreteClassInfo(SceneLayer, Entity, 0);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this SceneLayer, effectively
//                  resetting the members of this abstraction level only.

void SceneLayer::Clear()
{
    m_BitmapFile.Reset();
    m_pMainBitmap = 0;
    m_MainBitmapOwned = false;
    m_DrawTrans = true;
    m_Offset.Reset();
    m_ScrollInfo.SetXY(1.0, 1.0);
    m_ScrollRatio.SetXY(1.0, 1.0);
    m_ScaleFactor.SetXY(1.0, 1.0);
    m_ScaleInverse.SetXY(1.0, 1.0);
    m_ScaledDimensions.SetXY(1.0, 1.0);
    m_WrapX = true;
    m_WrapY = true;
    m_FillLeftColor = g_MaskColor;
    m_FillRightColor = g_MaskColor;
    m_FillUpColor = g_MaskColor;
    m_FillDownColor = g_MaskColor;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the SceneLayer object ready for use.

int SceneLayer::Create()
{
    // Read all the properties
    if (Entity::Create() < 0)
        return -1;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the SceneLayer object ready for use.

int SceneLayer::Create(ContentFile bitmapFile,
                       bool drawTrans,
                       Vector offset,
                       bool wrapX,
                       bool wrapY,
                       Vector scrollInfo)
{
    m_BitmapFile = bitmapFile;

    m_pMainBitmap = m_BitmapFile.GetAsBitmap();
    RTEAssert(m_pMainBitmap, "Failed to load BITMAP in SceneLayer::Create");

    Create(m_pMainBitmap, drawTrans, offset, wrapX, wrapY, scrollInfo);

    m_MainBitmapOwned = false;

    // Establisht he scaled dimensions of this
    m_ScaledDimensions.SetXY(m_pMainBitmap->w * m_ScaleFactor.m_X, m_pMainBitmap->h * m_ScaleFactor.m_Y);

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the SceneLayer object ready for use.

int SceneLayer::Create(BITMAP *pBitmap,
                       bool drawTrans,
                       Vector offset,
                       bool wrapX,
                       bool wrapY,
                       Vector scrollInfo)
{
    m_pMainBitmap = pBitmap;
    RTEAssert(m_pMainBitmap, "Null bitmap passed in when creating SceneLayer");
    m_MainBitmapOwned = true;

    m_DrawTrans = drawTrans;
    m_Offset = offset;
    m_WrapX = wrapX;
    m_WrapY = wrapY;
    m_ScrollInfo = scrollInfo;

    if (m_WrapX)
        m_ScrollRatio.m_X = m_ScrollInfo.m_X;
    else
        if (m_ScrollInfo.m_X == -1.0 || m_ScrollInfo.m_X == 1.0)
            m_ScrollRatio.m_X = 1.0;
        else if (m_ScrollInfo.m_X == g_FrameMan.GetPlayerScreenWidth())
            m_ScrollRatio.m_X = m_pMainBitmap->w - g_FrameMan.GetPlayerScreenWidth();
        else if (m_pMainBitmap->w == g_FrameMan.GetPlayerScreenWidth())
            m_ScrollRatio.m_X = 1.0f / (float)(m_ScrollInfo.m_X - g_FrameMan.GetPlayerScreenWidth());
        else
            m_ScrollRatio.m_X = (float)(m_pMainBitmap->w - g_FrameMan.GetPlayerScreenWidth()) /
                                (float)(m_ScrollInfo.m_X - g_FrameMan.GetPlayerScreenWidth());

    if (m_WrapY)
        m_ScrollRatio.m_Y = m_ScrollInfo.m_Y;
    else
        if (m_ScrollInfo.m_Y == -1.0 || m_ScrollInfo.m_Y == 1.0)
            m_ScrollRatio.m_Y = 1.0;
        else if (m_ScrollInfo.m_Y == g_FrameMan.GetPlayerScreenHeight())
            m_ScrollRatio.m_Y = m_pMainBitmap->h - g_FrameMan.GetPlayerScreenHeight();
        else if (m_pMainBitmap->h == g_FrameMan.GetPlayerScreenHeight())
            m_ScrollRatio.m_Y = 1.0f / (float)(m_ScrollInfo.m_Y - g_FrameMan.GetPlayerScreenHeight());
        else
            m_ScrollRatio.m_Y = (float)(m_pMainBitmap->h - g_FrameMan.GetPlayerScreenHeight()) /
                                (float)(m_ScrollInfo.m_Y - g_FrameMan.GetPlayerScreenHeight());

    // Establisht he scaled dimensions of this
    m_ScaledDimensions.SetXY(m_pMainBitmap->w * m_ScaleFactor.m_X, m_pMainBitmap->h * m_ScaleFactor.m_Y);

    // Sampled color at the edges of the layer that can be used to fill gap if the layer isn't large enough to cover a target bitmap
    m_FillLeftColor = m_WrapX ? g_MaskColor : _getpixel(m_pMainBitmap, 0, m_pMainBitmap->h / 2);
    m_FillRightColor = m_WrapX ? g_MaskColor : _getpixel(m_pMainBitmap, m_pMainBitmap->w - 1, m_pMainBitmap->h / 2);
    m_FillUpColor = m_WrapY ? g_MaskColor : _getpixel(m_pMainBitmap, m_pMainBitmap->w / 2, 0);
    m_FillDownColor = m_WrapY ? g_MaskColor : _getpixel(m_pMainBitmap, m_pMainBitmap->w / 2, m_pMainBitmap->h - 1);

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a SceneLayer to be identical to another, by deep copy.
// Arguments:       A reference to the SceneLayer to deep copy.

int SceneLayer::Create(const SceneLayer &reference)
{
    Entity::Create(reference);

    m_BitmapFile = reference.m_BitmapFile;

    // Deep copy the bitmap
    if (reference.m_pMainBitmap)
    {
        // Copy the bitmap from the ContentFile, because we're going to be changing it!
        BITMAP *pCopyFrom = reference.m_pMainBitmap;
        RTEAssert(pCopyFrom, "Couldn't load the bitmap file specified for SceneLayer!");

        // Destination
        m_pMainBitmap = create_bitmap_ex(8, pCopyFrom->w, pCopyFrom->h);
        RTEAssert(m_pMainBitmap, "Failed to allocate BITMAP in SceneLayer::Create");

        // Copy!
        blit(pCopyFrom, m_pMainBitmap, 0, 0, 0, 0, pCopyFrom->w, pCopyFrom->h);

        InitScrollRatios();

        m_MainBitmapOwned = true;
    }
    // If no bitmap to copy, has to load the data (LoadData) to create this in the copied to SL
    else
        m_MainBitmapOwned = false;

    m_DrawTrans = reference.m_DrawTrans;
    m_Offset = reference.m_Offset;
    m_WrapX = reference.m_WrapX;
    m_WrapY = reference.m_WrapY;
    m_ScrollInfo = reference.m_ScrollInfo;
    // The ratios need to be calculated from the ScrollInfo with InitScrollRatios later in LoadData
    m_ScrollRatio = reference.m_ScrollRatio;
    m_ScaleFactor = reference.m_ScaleFactor;
    m_ScaleInverse = reference.m_ScaleInverse;
    m_ScaledDimensions = reference.m_ScaledDimensions;
    m_FillLeftColor = reference.m_FillLeftColor;
    m_FillRightColor = reference.m_FillRightColor;
    m_FillUpColor = reference.m_FillUpColor;
    m_FillDownColor = reference.m_FillDownColor;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  LoadData
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Actually loads previously specified/created data into memory. Has
//                  to be done before using this SceneLayer.

int SceneLayer::LoadData()
{
/* No need to do this copying, we are re-loading fresh from disk each time
    // Copy the bitmap from the ContentFile, because we're going to be changing it!
    BITMAP *pCopyFrom = m_BitmapFile.GetAsBitmap();
    RTEAssert(pCopyFrom, "Couldn't load the bitmap file specified for SceneLayer!");

    // Destination
    m_pMainBitmap = create_bitmap_ex(8, pCopyFrom->w, pCopyFrom->h);
    RTEAssert(m_pMainBitmap, "Failed to allocate BITMAP in SceneLayer::Create");

    // Copy!
    blit(pCopyFrom, m_pMainBitmap, 0, 0, 0, 0, pCopyFrom->w, pCopyFrom->h);
*/
    // Re-load directly from disk each time; don't do any caching of these bitmaps
    m_pMainBitmap = m_BitmapFile.GetAsBitmap(COLORCONV_NONE, false);

    m_MainBitmapOwned = true;

    InitScrollRatios();

    // Sampled color at the edges of the layer that can be used to fill gap if the layer isn't large enough to cover a target bitmap
    m_FillLeftColor = m_WrapX ? g_MaskColor : _getpixel(m_pMainBitmap, 0, m_pMainBitmap->h / 2);
    m_FillRightColor = m_WrapX ? g_MaskColor : _getpixel(m_pMainBitmap, m_pMainBitmap->w - 1, m_pMainBitmap->h / 2);
    m_FillUpColor = m_WrapY ? g_MaskColor : _getpixel(m_pMainBitmap, m_pMainBitmap->w / 2, 0);
    m_FillDownColor = m_WrapY ? g_MaskColor : _getpixel(m_pMainBitmap, m_pMainBitmap->w / 2, m_pMainBitmap->h - 1);

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SaveData
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves current data in memory to disk.

int SceneLayer::SaveData(string bitmapPath)
{
    if (bitmapPath.empty())
        return -1;

    // Save out the bitmap
    if (m_pMainBitmap)
    {
        PALETTE palette;
        get_palette(palette);
        if (save_bmp(bitmapPath.c_str(), m_pMainBitmap, palette) != 0)
            return -1;

        // Set the new path to point to the new file location - only if there was a successful save of the bitmap
        m_BitmapFile.SetDataPath(bitmapPath);
    }

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ClearData
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears out any previously loaded bitmap data from memory.

int SceneLayer::ClearData()
{
    if (m_pMainBitmap && m_MainBitmapOwned)
        destroy_bitmap(m_pMainBitmap);
    m_pMainBitmap = 0;

    m_MainBitmapOwned = false;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReadProperty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads a property value from a reader stream. If the name isn't
//                  recognized by this class, then ReadProperty of the parent class
//                  is called. If the property isn't recognized by any of the base classes,
//                  false is returned, and the reader's position is untouched.

int SceneLayer::ReadProperty(const std::string_view &propName, Reader &reader)
{
    if (propName == "BitmapFile")
        reader >> m_BitmapFile;
    else if (propName == "DrawTransparent")
        reader >> m_DrawTrans;
    else if (propName == "Offset")
        reader >> m_Offset;
    else if (propName == "WrapX")
        reader >> m_WrapX;
    else if (propName == "WrapY")
        reader >> m_WrapY;
    // Actually read the scrollinfo, not the ratio. The ratios will be initalized later
    else if (propName == "ScrollRatio")
        reader >> m_ScrollInfo;
    else if (propName == "ScaleFactor")
    {
        reader >> m_ScaleFactor;
        // Gotto init more
        SetScaleFactor(m_ScaleFactor);
    }
    else
        return Entity::ReadProperty(propName, reader);

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this SceneLayer with a Writer for
//                  later recreation with Create(Reader &reader);

int SceneLayer::Save(Writer &writer) const
{
    Entity::Save(writer);

    writer.NewProperty("BitmapFile");
    writer << m_BitmapFile;
    writer.NewProperty("DrawTransparent");
    writer << m_DrawTrans;
    writer.NewProperty("Offset");
    writer << m_Offset;
    writer.NewProperty("WrapX");
    writer << m_WrapX;
    writer.NewProperty("WrapY");
    writer << m_WrapY;
    writer.NewProperty("ScrollRatio");
    writer << m_ScrollInfo;
    writer.NewProperty("ScaleFactor");
    writer << m_ScaleFactor;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the SceneLayer object.

void SceneLayer::Destroy(bool notInherited)
{
    if (m_MainBitmapOwned)
        destroy_bitmap(m_pMainBitmap);

    if (!notInherited)
        Entity::Destroy();
    Clear();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetScaleFactor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the scale that this should be drawn at when using DrawScaled.

void SceneLayer::SetScaleFactor(const Vector newScale)
{
    m_ScaleFactor = newScale;
    m_ScaleInverse.SetXY(1.0f / newScale.m_X, 1.0f / newScale.m_Y);
    if (m_pMainBitmap)
        m_ScaledDimensions.SetXY(m_pMainBitmap->w * newScale.m_X, m_pMainBitmap->h * newScale.m_Y);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetPixel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets a specific pixel from the main bitmap of this SceneLayer.
//                  LockColorBitmap() must be called before using this method.

unsigned char SceneLayer::GetPixel(const int pixelX, const int pixelY)
{
    // Make sure it's within the boundaries of the bitmap.
    if (pixelX < 0 || pixelX >= m_pMainBitmap->w || pixelY < 0 || pixelY >= m_pMainBitmap->h)
        return 0;
//    RTEAssert(m_pTerrain->GetBitmap()->m_LockCount > 0, "Trying to access unlocked terrain bitmap");
//    RTEAssert(is_inside_bitmap(m_pMainBitmap, pixelX, pixelY, 0), "Trying to access pixel outside of SceneLayer's bitmap's boundaries!");
    return  _getpixel(m_pMainBitmap, pixelX, pixelY);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetPixel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets a specific pixel on the main bitmap of this SceneLayer to a
//                  specific value. LockColorBitmap() must be called before using this
//                  method.

void SceneLayer::SetPixel(const int pixelX, const int pixelY, const unsigned char value)
{
    RTEAssert(m_MainBitmapOwned, "Trying to set a pixel of a SceneLayer's bitmap which isn't owned!");

    // Make sure it's within the boundaries of the bitmap.
    if (pixelX < 0 ||
       pixelX >= m_pMainBitmap->w ||
       pixelY < 0 ||
       pixelY >= m_pMainBitmap->h)
       return;
//    RTEAssert(m_pTerrain->GetBitmap()->m_LockCount > 0, "Trying to access unlocked terrain bitmap");
//    RTEAssert(is_inside_bitmap(m_pMainBitmap, pixelX, pixelY, 0), "Trying to access pixel outside of SceneLayer's bitmap's boundaries!");
//    _putpixel(m_pMainBitmap, pixelX, pixelY, value);
    putpixel(m_pMainBitmap, pixelX, pixelY, value);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ForceBounds
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Wraps or bounds a position coordinate if it is off bounds of the
//                  SceneLayer, depending on the wrap settings of this SceneLayer.

bool SceneLayer::ForceBounds(int &posX, int &posY, bool scaled) const
{
    bool wrapped = false;
    int width = scaled ? m_ScaledDimensions.GetFloorIntX() : m_pMainBitmap->w;
    int height = scaled ? m_ScaledDimensions.GetFloorIntY() : m_pMainBitmap->h;

    if (posX < 0) {
        if (m_WrapX)
        {
            while (posX < 0)
                posX += width;
            wrapped = true;
        }
        else
            posX = 0;
    }

    if (posY < 0) {
        if (m_WrapY)
        {
            while (posY < 0)
                posY += height;
            wrapped = true;
        }
        else
            posY = 0;
    }

    if (posX >= width) {
        if (m_WrapX)
        {
            posX %= width;
            wrapped = true;
        }
        else
            posX = width - 1;
    }

    if (posY >= height) {
        if (m_WrapY)
        {
            posY %= height;
            wrapped = true;
        }
        else
            posY = height - 1;
    }

    return wrapped;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ForceBounds
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Wraps or bounds a position coordinate if it is off bounds of the
//                  SceneLayer, depending on the wrap settings of this SceneLayer.

bool SceneLayer::ForceBounds(Vector &pos, bool scaled) const
{
    int posX = std::floor(pos.m_X);
    int posY = std::floor(pos.m_Y);

    bool wrapped = ForceBounds(posX, posY, scaled);

    pos.m_X = posX + (pos.m_X - std::floor(pos.m_X));
    pos.m_Y = posY + (pos.m_Y - std::floor(pos.m_Y));

    return wrapped;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          WrapPosition
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Only wraps a position coordinate if it is off bounds of the SceneLayer
//                  and wrapping in the corresponding axes are turned on.

bool SceneLayer::WrapPosition(int &posX, int &posY, bool scaled) const
{
    bool wrapped = false;
    int width = scaled ? m_ScaledDimensions.GetFloorIntX() : m_pMainBitmap->w;
    int height = scaled ? m_ScaledDimensions.GetFloorIntY() : m_pMainBitmap->h;

    if (m_WrapX) {
        if (posX < 0) {
            while (posX < 0)
                posX += width;
            wrapped = true;
        }
        else if (posX >= width) {
            posX %= width;
            wrapped = true;
        }
    }

    if (m_WrapY) {
        if (posY < 0) {
            while (posY < 0)
                posY += height;
            wrapped = true;
        }
        else if (posY >= height) {
            posY %= height;
            wrapped = true;
        }
    }

    return wrapped;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          WrapPosition
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Only wraps a position coordinate if it is off bounds of the SceneLayer
//                  and wrapping in the corresponding axes are turned on.

bool SceneLayer::WrapPosition(Vector &pos, bool scaled) const
{
    int posX = std::floor(pos.m_X);
    int posY = std::floor(pos.m_Y);

    bool wrapped = WrapPosition(posX, posY, scaled);

    pos.m_X = posX + (pos.m_X - std::floor(pos.m_X));
    pos.m_Y = posY + (pos.m_Y - std::floor(pos.m_Y));

    return wrapped;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the state of this SceneLayer. Supposed to be done every frame.

void SceneLayer::Update()
{
    ;
}


// TODO: Declare this in the header and remove the dupe declaration in NetworkClient.cpp
// Data structure for constructing the draw boxes we'll need to use for drawing
struct SLDrawBox
{
    int sourceX;
    int sourceY;
    int sourceW;
    int sourceH;
    int destX;
    int destY;
};


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this SceneLayer's current scrolled position to a bitmap.

void SceneLayer::Draw(BITMAP *pTargetBitmap, Box& targetBox, const Vector &scrollOverride) const
{
    RTEAssert(m_pMainBitmap, "Data of this SceneLayer has not been loaded before trying to draw!");

    int sourceX = 0;
    int sourceY = 0;
    int sourceW = 0;
    int sourceH = 0;
    int destX = 0;
    int destY = 0;
    list<SLDrawBox> drawList;

    int offsetX;
    int offsetY;
    bool scrollOverridden = !(scrollOverride.m_X == -1 && scrollOverride.m_Y == -1);

    // Overridden scroll position
    if (scrollOverridden)
    {
        offsetX = scrollOverride.GetFloorIntX();
        offsetY = scrollOverride.GetFloorIntY();
    }
    // Regular scroll
    else
    {
        offsetX = std::floor(m_Offset.m_X * m_ScrollRatio.m_X);
        offsetY = std::floor(m_Offset.m_Y * m_ScrollRatio.m_Y);
        // Only force bounds when doing regular scroll offset because the override is used to do terrain object application tricks and sometimes needs the offsets to be < 0
//        ForceBounds(offsetX, offsetY);
        WrapPosition(offsetX, offsetY);
    }

    // Make target box valid size if it's empty
    if (targetBox.IsEmpty())
    {
        targetBox.SetCorner(Vector(0, 0));
        targetBox.SetWidth(pTargetBitmap->w);
        targetBox.SetHeight(pTargetBitmap->h);
    }

    // Set the clipping rectangle of the target bitmap to match the specified target box
    set_clip_rect(pTargetBitmap, targetBox.GetCorner().m_X, targetBox.GetCorner().m_Y, targetBox.GetCorner().m_X + targetBox.GetWidth() - 1, targetBox.GetCorner().m_Y + targetBox.GetHeight() - 1);

    // Choose the correct blitting function based on transparency setting
    void (*pfBlit)(BITMAP *source, BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height) = m_DrawTrans ? &masked_blit : &blit;

    // See if this SceneLayer is wider AND higher than the target bitmap; then use simple wrapping logic - oterhwise need to tile
    if (m_pMainBitmap->w >= pTargetBitmap->w && m_pMainBitmap->h >= pTargetBitmap->h)
    {
        sourceX     = offsetX;
        sourceY     = offsetY;
        sourceW     = m_pMainBitmap->w - offsetX;
        sourceH     = m_pMainBitmap->h - offsetY;
        destX       = targetBox.GetCorner().m_X;
        destY       = targetBox.GetCorner().m_Y;
        pfBlit(m_pMainBitmap, pTargetBitmap, sourceX, sourceY, destX, destY, sourceW, sourceH);

        sourceX     = 0;
        sourceY     = offsetY;
        sourceW     = offsetX;
        sourceH     = m_pMainBitmap->h - offsetY;
        destX       = targetBox.GetCorner().m_X + m_pMainBitmap->w - offsetX;
        destY       = targetBox.GetCorner().m_Y;
        pfBlit(m_pMainBitmap, pTargetBitmap, sourceX, sourceY, destX, destY, sourceW, sourceH);

        sourceX     = offsetX;
        sourceY     = 0;
        sourceW     = m_pMainBitmap->w - offsetX;
        sourceH     = offsetY;
        destX       = targetBox.GetCorner().m_X;
        destY       = targetBox.GetCorner().m_Y + m_pMainBitmap->h - offsetY;
        pfBlit(m_pMainBitmap, pTargetBitmap, sourceX, sourceY, destX, destY, sourceW, sourceH);

        sourceX     = 0;
        sourceY     = 0;
        sourceW     = offsetX;
        sourceH     = offsetY;
        destX       = targetBox.GetCorner().m_X + m_pMainBitmap->w - offsetX;
        destY       = targetBox.GetCorner().m_Y + m_pMainBitmap->h - offsetY;
        pfBlit(m_pMainBitmap, pTargetBitmap, sourceX, sourceY, destX, destY, sourceW, sourceH);
    }
    // Target bitmap is larger in some dimension, so need to draw this tiled as many times as necessary to cover the whole target
    else
    {
        int tiledOffsetX = 0;
        int tiledOffsetY = 0;
        // Use the dimensions of the target box, if it has any area at all
        int targetWidth = MIN(pTargetBitmap->w, targetBox.GetWidth());
        int targetHeight = MIN(pTargetBitmap->h, targetBox.GetHeight());
        int toCoverX = offsetX + targetBox.GetCorner().m_X + targetWidth;
        int toCoverY = offsetY + targetBox.GetCorner().m_Y + targetHeight;

        // Check for special case adjustment when the screen is larger than the scene
        bool screenLargerThanSceneX = false;
        bool screenLargerThanSceneY = false;
        if (!scrollOverridden && g_SceneMan.GetSceneWidth() > 0)
        {
            screenLargerThanSceneX = pTargetBitmap->w > g_SceneMan.GetSceneWidth();
            screenLargerThanSceneY = pTargetBitmap->h > g_SceneMan.GetSceneHeight();
        }

        // Y tiling
        do
        {
            // X tiling
            do
            {
                sourceX     = 0;
                sourceY     = 0;
                sourceW     = m_pMainBitmap->w;
                sourceH     = m_pMainBitmap->h;
                // If the unwrapped and untiled direction can't cover the target area, place it in the middle of the target bitmap, and leave the excess perimeter on each side untouched
                destX       = (!m_WrapX && screenLargerThanSceneX) ? ((pTargetBitmap->w / 2) - (m_pMainBitmap->w / 2)) : (targetBox.GetCorner().m_X + tiledOffsetX - offsetX);
                destY       = (!m_WrapY && screenLargerThanSceneY) ? ((pTargetBitmap->h / 2) - (m_pMainBitmap->h / 2)) : (targetBox.GetCorner().m_Y + tiledOffsetY - offsetY);
                pfBlit(m_pMainBitmap, pTargetBitmap, sourceX, sourceY, destX, destY, sourceW, sourceH);

                tiledOffsetX += m_pMainBitmap->w;
            }
            // Only tile if we're supposed to wrap widthwise
            while (m_WrapX && toCoverX > tiledOffsetX);

            tiledOffsetY += m_pMainBitmap->h;
        }
        // Only tile if we're supposed to wrap heightwise
        while (m_WrapY && toCoverY > tiledOffsetY);

// TODO: Do this above instead, testing down here only
        // Detect if nonwrapping layer dimensions can't cover the whole target area with its main bitmap. If so, fill in the gap with appropriate solid color sampled from the hanging edge
        if (!m_WrapX && !screenLargerThanSceneX && m_ScrollRatio.m_X < 0)
        {
            if (m_FillLeftColor != g_MaskColor && offsetX != 0)
                rectfill(pTargetBitmap, targetBox.GetCorner().m_X, targetBox.GetCorner().m_Y, targetBox.GetCorner().m_X - offsetX, targetBox.GetCorner().m_Y + targetBox.GetHeight(), m_FillLeftColor);
            if (m_FillRightColor != g_MaskColor)
                rectfill(pTargetBitmap, (targetBox.GetCorner().m_X - offsetX) + m_pMainBitmap->w, targetBox.GetCorner().m_Y, targetBox.GetCorner().m_X + targetBox.GetWidth(), targetBox.GetCorner().m_Y + targetBox.GetHeight(), m_FillRightColor);
        }

        if (!m_WrapY && !screenLargerThanSceneY && m_ScrollRatio.m_Y < 0)
        {
            if (m_FillUpColor != g_MaskColor && offsetY != 0)
                rectfill(pTargetBitmap, targetBox.GetCorner().m_X, targetBox.GetCorner().m_Y, targetBox.GetCorner().m_X + targetBox.GetWidth(), targetBox.GetCorner().m_Y - offsetY, m_FillUpColor);
            if (m_FillDownColor != g_MaskColor)
                rectfill(pTargetBitmap, targetBox.GetCorner().m_X, (targetBox.GetCorner().m_Y - offsetY) + m_pMainBitmap->h, targetBox.GetCorner().m_X + targetBox.GetWidth(), targetBox.GetCorner().m_Y + targetBox.GetHeight(), m_FillDownColor);
        }
    }

    // Reset the clip rect back to the entire target bitmap
    set_clip_rect(pTargetBitmap, 0, 0, pTargetBitmap->w - 1, pTargetBitmap->h - 1);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  DrawScaled
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this SceneLayer's current scrolled position to a bitmap, but also
//                  scaled according to what has been set with SetScaleFactor.

void SceneLayer::DrawScaled(BITMAP *pTargetBitmap, Box &targetBox, const Vector &scrollOverride) const
{
    // If no scaling, use the regular scaling routine
    if (m_ScaleFactor.m_X == 1.0 && m_ScaleFactor.m_Y == 1.0)
        return Draw(pTargetBitmap, targetBox, scrollOverride);

    RTEAssert(m_pMainBitmap, "Data of this SceneLayer has not been loaded before trying to draw!");


/*

    // Calculate how many times smaller the unseen map is compared to the entire terrain's dimensions
    Vector resDenom((float)pTerrain->GetBitmap()->w / (float)pUnseenLayer->GetBitmap()->w, (float)pTerrain->GetBitmap()->h / (float)pUnseenLayer->GetBitmap()->h);
    resDenom.Floor();
    // Now convert it back to the multiplier so that the flooring that happened above is reflected in the multiplier too, to avoid jittering
    Vector resMult(1.0f / resDenom.m_X, 1.0f / resDenom.m_Y);
    // This is the sub-source-pixel offset on the target to make scrolling of the pixelated bitmap smooth
    Vector subOffset((int)targetPos.m_X % (int)resDenom.m_X, (int)targetPos.m_Y % (int)resDenom.m_Y);
    masked_stretch_blit(pUnseenLayer->GetBitmap(), pTargetBitmap, (int)(targetPos.m_X * resMult.m_X + 0.0001f), (int)(targetPos.m_Y * resMult.m_Y + 0.0001f), pTargetBitmap->w * resMult.m_X + 1.0001f, pTargetBitmap->h * resMult.m_Y + 1.0001f, -subOffset.m_X, -subOffset.m_Y, pTargetBitmap->w + resDenom.m_X, pTargetBitmap->h + resDenom.m_Y);



// TODO: Remove
//                char balle[245];
//                std::snprintf(balle, sizeof(balle), "y: %f becomes %f through %f and %f, needs to match: %f, which is: %i", targetPos.m_Y, subOffset.m_Y, resMult.m_Y, resDenom.m_Y, targetPos.m_Y * resMult.m_Y + 0.0001f, (int)(targetPos.m_Y * resMult.m_Y + 0.0001f));
//                g_FrameMan.SetScreenText(string(balle));


*/



    int sourceX = 0;
    int sourceY = 0;
    int sourceW = 0;
    int sourceH = 0;
    int destX = 0;
    int destY = 0;
    int destW = 0;
    int destH = 0;
    list<SLDrawBox> drawList;

    int offsetX;
    int offsetY;
    bool scrollOverridden = !(scrollOverride.m_X == -1 && scrollOverride.m_Y == -1);

    // Overridden scroll position
    if (scrollOverridden)
    {
        offsetX = scrollOverride.GetFloorIntX();
        offsetY = scrollOverride.GetFloorIntY();
    }
    // Regular scroll
    else
    {
        offsetX = std::floor(m_Offset.m_X * m_ScrollRatio.m_X);
        offsetY = std::floor(m_Offset.m_Y * m_ScrollRatio.m_Y);
        // Only force bounds when doing regular scroll offset because the override is used to do terrain object application tricks and sometimes needs the offsets to be < 0
//        ForceBounds(offsetX, offsetY);
        WrapPosition(offsetX, offsetY);
    }

    // Make target box valid size if it's empty
    if (targetBox.IsEmpty())
    {
        targetBox.SetCorner(Vector(0, 0));
        targetBox.SetWidth(pTargetBitmap->w);
        targetBox.SetHeight(pTargetBitmap->h);
    }

    // Set the clipping rectangle of the target bitmap to match the specified target box
    set_clip_rect(pTargetBitmap, targetBox.GetCorner().m_X, targetBox.GetCorner().m_Y, targetBox.GetCorner().m_X + targetBox.GetWidth() - 1, targetBox.GetCorner().m_Y + targetBox.GetHeight() - 1);

    // Choose the correct blitting function based on transparency setting
    void (*pfBlit)(BITMAP *source, BITMAP *dest, int source_x, int source_y, int source_w, int source_h, int dest_x, int dest_y, int dest_w, int dest_h) = m_DrawTrans ? &masked_stretch_blit : &stretch_blit;
//    void (*pfBlit)(BITMAP *source, BITMAP *dest, int source_x, int source_y, int dest_x, int dest_y, int width, int height) = m_DrawTrans ? &masked_blit : &blit;

    // Get a scaled offset for the source layer
    Vector sourceOffset(offsetX * m_ScaleInverse.m_X, offsetY * m_ScaleInverse.m_Y);

    // See if this SceneLayer is wider AND higher than the target bitmap when scaled; then use simple wrapping logic - oterhwise need to tile
    if (m_ScaledDimensions.m_X >= pTargetBitmap->w && m_ScaledDimensions.m_Y >= pTargetBitmap->h)
    {
        // Upper left
        sourceX     = 0;
        sourceY     = 0;
        sourceW     = m_pMainBitmap->w;
        sourceH     = m_pMainBitmap->h;
        destX       = targetBox.GetCorner().m_X - offsetX;
        destY       = targetBox.GetCorner().m_Y - offsetY;
        destW       = sourceW * m_ScaleFactor.m_X + 1;
        destH       = sourceH * m_ScaleFactor.m_Y + 1;
        pfBlit(m_pMainBitmap, pTargetBitmap, sourceX, sourceY, sourceW, sourceH, destX, destY, destW, destH);

        // Upper right
        sourceX     = 0;
        sourceY     = 0;
        sourceW     = sourceOffset.m_X;
        sourceH     = m_pMainBitmap->h;
        destX       = targetBox.GetCorner().m_X + m_ScaledDimensions.m_X - offsetX;
        destY       = targetBox.GetCorner().m_Y - offsetY;
        destW       = sourceW * m_ScaleFactor.m_X + 1;
        destH       = sourceH * m_ScaleFactor.m_Y + 1;
        pfBlit(m_pMainBitmap, pTargetBitmap, sourceX, sourceY, sourceW, sourceH, destX, destY, destW, destH);

        // Lower left
        sourceX     = 0;
        sourceY     = 0;
        sourceW     = m_pMainBitmap->w;
        sourceH     = sourceOffset.m_Y;
        destX       = targetBox.GetCorner().m_X - offsetX;
        destY       = targetBox.GetCorner().m_Y + m_ScaledDimensions.m_Y - offsetY;
        destW       = sourceW * m_ScaleFactor.m_X + 1;
        destH       = sourceH * m_ScaleFactor.m_Y + 1;
        pfBlit(m_pMainBitmap, pTargetBitmap, sourceX, sourceY, sourceW, sourceH, destX, destY, destW, destH);

        // Lower right
        sourceX     = 0;
        sourceY     = 0;
        sourceW     = sourceOffset.m_X;
        sourceH     = sourceOffset.m_Y;
        destX       = targetBox.GetCorner().m_X + m_ScaledDimensions.m_X - offsetX;
        destY       = targetBox.GetCorner().m_Y + m_ScaledDimensions.m_Y - offsetY;
        destW       = sourceW * m_ScaleFactor.m_X + 1;
        destH       = sourceH * m_ScaleFactor.m_Y + 1;
        pfBlit(m_pMainBitmap, pTargetBitmap, sourceX, sourceY, sourceW, sourceH, destX, destY, destW, destH);
    }
    // Target bitmap is larger in some dimension, so need to draw this tiled as many times as necessary to cover the whole target
    else
    {
        int tiledOffsetX = 0;
        int tiledOffsetY = 0;
        // Use the dimensions of the target box, if it has any area at all
        int targetWidth = MIN(pTargetBitmap->w, targetBox.GetWidth());
        int targetHeight = MIN(pTargetBitmap->h, targetBox.GetHeight());
        int toCoverX = offsetX + targetBox.GetCorner().m_X + targetWidth;
        int toCoverY = offsetY + targetBox.GetCorner().m_Y + targetHeight;

        // Check for special case adjustment when the screen is larger than the scene
        bool screenLargerThanSceneX = false;
        bool screenLargerThanSceneY = false;
        if (!scrollOverridden && g_SceneMan.GetSceneWidth() > 0)
        {
            screenLargerThanSceneX = pTargetBitmap->w > g_SceneMan.GetSceneWidth();
            screenLargerThanSceneY = pTargetBitmap->h > g_SceneMan.GetSceneHeight();
        }

        // Y tiling
        do
        {
            // X tiling
            do
            {
                sourceX     = 0;
                sourceY     = 0;
                sourceW     = m_pMainBitmap->w;
                sourceH     = m_pMainBitmap->h;
                // If the unwrapped and untiled direction can't cover the target area, place it in the middle of the target bitmap, and leave the excess perimeter on each side untouched
                destX       = (!m_WrapX && screenLargerThanSceneX) ? ((pTargetBitmap->w / 2) - (m_ScaledDimensions.m_X / 2)) : (targetBox.GetCorner().m_X + tiledOffsetX - offsetX);
                destY       = (!m_WrapY && screenLargerThanSceneY) ? ((pTargetBitmap->h / 2) - (m_ScaledDimensions.m_Y / 2)) : (targetBox.GetCorner().m_Y + tiledOffsetY - offsetY);
                destW       = m_ScaledDimensions.m_X;
                destH       = m_ScaledDimensions.m_Y;
                pfBlit(m_pMainBitmap, pTargetBitmap, sourceX, sourceY, sourceW, sourceH, destX, destY, destW, destH);

                tiledOffsetX += m_ScaledDimensions.m_X;
            }
            // Only tile if we're supposed to wrap widthwise
            while (m_WrapX && toCoverX > tiledOffsetX);

            tiledOffsetY += m_ScaledDimensions.m_Y;
        }
        // Only tile if we're supposed to wrap heightwise
        while (m_WrapY && toCoverY > tiledOffsetY);

// TODO: Do this above instead, testing down here only
/*
        // Detect if nonwrapping layer dimensions can't cover the whole target area with its main bitmap. If so, fill in the gap with appropriate solid color sampled from the hanging edge
        if (!m_WrapX && !screenLargerThanSceneX && m_ScrollRatio.m_X < 0)
        {
            if (m_FillLeftColor != g_MaskColor && offsetX != 0)
                rectfill(pTargetBitmap, targetBox.GetCorner().m_X, targetBox.GetCorner().m_Y, targetBox.GetCorner().m_X - offsetX, targetBox.GetCorner().m_Y + targetBox.GetHeight(), m_FillLeftColor);
            if (m_FillRightColor != g_MaskColor)
                rectfill(pTargetBitmap, (targetBox.GetCorner().m_X - offsetX) + m_pMainBitmap->w, targetBox.GetCorner().m_Y, targetBox.GetCorner().m_X + targetBox.GetWidth(), targetBox.GetCorner().m_Y + targetBox.GetHeight(), m_FillRightColor);
        }

        if (!m_WrapY && !screenLargerThanSceneY && m_ScrollRatio.m_Y < 0)
        {
            if (m_FillUpColor != g_MaskColor && offsetY != 0)
                rectfill(pTargetBitmap, targetBox.GetCorner().m_X, targetBox.GetCorner().m_Y, targetBox.GetCorner().m_X + targetBox.GetWidth(), targetBox.GetCorner().m_Y - offsetY, m_FillUpColor);
            if (m_FillDownColor != g_MaskColor)
                rectfill(pTargetBitmap, targetBox.GetCorner().m_X, (targetBox.GetCorner().m_Y - offsetY) + m_pMainBitmap->h, targetBox.GetCorner().m_X + targetBox.GetWidth(), targetBox.GetCorner().m_Y + targetBox.GetHeight(), m_FillDownColor);
        }
*/
    }

    // Reset the clip rect back to the entire target bitmap
    set_clip_rect(pTargetBitmap, 0, 0, pTargetBitmap->w - 1, pTargetBitmap->h - 1);
}

/* not neccessary
//////////////////////////////////////////////////////////////////////////////////////////
// Method:          LoadContour
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Loads a terrain contour file and applies it to this SceneLayer.

virtual int SceneLayer::LoadContour(char material, bool dirtBelowContour, ContentFile &contourFile)
{
    char *contour
}
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  InitScrollRatios
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Initialize the scroll ratios from the encoded scroll info. Must be
//                  done after the bitmap has been created in the derived concrete classes.

void SceneLayer::InitScrollRatios()
{
    if (m_WrapX)
        m_ScrollRatio.m_X = m_ScrollInfo.m_X;
    else
    {
        if (m_ScrollInfo.m_X == -1.0 || m_ScrollInfo.m_X == 1.0)
            m_ScrollRatio.m_X = 1.0;
        else if (m_ScrollInfo.m_X == g_FrameMan.GetPlayerScreenWidth())
            m_ScrollRatio.m_X = m_pMainBitmap->w - g_FrameMan.GetPlayerScreenWidth();
        else if (m_pMainBitmap->w == g_FrameMan.GetPlayerScreenWidth())
            m_ScrollRatio.m_X = 1.0f / (float)(m_ScrollInfo.m_X - g_FrameMan.GetPlayerScreenWidth());
        else
            m_ScrollRatio.m_X = (float)(m_pMainBitmap->w - g_FrameMan.GetPlayerScreenWidth()) /
                                (float)(m_ScrollInfo.m_X - g_FrameMan.GetPlayerScreenWidth());
    }

    if (m_WrapY)
        m_ScrollRatio.m_Y = m_ScrollInfo.m_Y;
    else
    {
        if (m_ScrollInfo.m_Y == -1.0 || m_ScrollInfo.m_Y == 1.0)
            m_ScrollRatio.m_Y = 1.0;
        else if (m_ScrollInfo.m_Y == g_FrameMan.GetPlayerScreenHeight())
            m_ScrollRatio.m_Y = m_pMainBitmap->h - g_FrameMan.GetPlayerScreenHeight();
        else if (m_pMainBitmap->h == g_FrameMan.GetPlayerScreenHeight())
            m_ScrollRatio.m_Y = 1.0f / (float)(m_ScrollInfo.m_Y - g_FrameMan.GetPlayerScreenHeight());
        else
            m_ScrollRatio.m_Y = (float)(m_pMainBitmap->h - g_FrameMan.GetPlayerScreenHeight()) /
                                (float)(m_ScrollInfo.m_Y - g_FrameMan.GetPlayerScreenHeight());
    }

    // Establish the scaled dimensions of this
    m_ScaledDimensions.SetXY(m_pMainBitmap->w * m_ScaleFactor.m_X, m_pMainBitmap->h * m_ScaleFactor.m_Y);
}


void SceneLayer::UpdateScrollRatiosForNetworkPlayer(int player)
{
	if (m_WrapX)
		m_ScrollRatio.m_X = m_ScrollInfo.m_X;
	else
	{
		if (m_ScrollInfo.m_X == -1.0 || m_ScrollInfo.m_X == 1.0)
			m_ScrollRatio.m_X = 1.0;
		else if (m_ScrollInfo.m_X == g_FrameMan.GetPlayerFrameBufferWidth(player))
			m_ScrollRatio.m_X = m_pMainBitmap->w - g_FrameMan.GetPlayerFrameBufferWidth(player);
		else if (m_pMainBitmap->w == g_FrameMan.GetPlayerFrameBufferWidth(player))
			m_ScrollRatio.m_X = 1.0f / (float)(m_ScrollInfo.m_X - g_FrameMan.GetPlayerFrameBufferWidth(player));
		else
			m_ScrollRatio.m_X = (float)(m_pMainBitmap->w - g_FrameMan.GetPlayerFrameBufferWidth(player)) /
			(float)(m_ScrollInfo.m_X - g_FrameMan.GetPlayerFrameBufferWidth(player));
	}

	if (m_WrapY)
		m_ScrollRatio.m_Y = m_ScrollInfo.m_Y;
	else
	{
		if (m_ScrollInfo.m_Y == -1.0 || m_ScrollInfo.m_Y == 1.0)
			m_ScrollRatio.m_Y = 1.0;
		else if (m_ScrollInfo.m_Y == g_FrameMan.GetPlayerFrameBufferHeight(player))
			m_ScrollRatio.m_Y = m_pMainBitmap->h - g_FrameMan.GetPlayerFrameBufferHeight(player);
		else if (m_pMainBitmap->h == g_FrameMan.GetPlayerFrameBufferHeight(player))
			m_ScrollRatio.m_Y = 1.0f / (float)(m_ScrollInfo.m_Y - g_FrameMan.GetPlayerFrameBufferHeight(player));
		else
			m_ScrollRatio.m_Y = (float)(m_pMainBitmap->h - g_FrameMan.GetPlayerFrameBufferHeight(player)) /
			(float)(m_ScrollInfo.m_Y - g_FrameMan.GetPlayerFrameBufferHeight(player));
	}

	// Establish the scaled dimensions of this
	m_ScaledDimensions.SetXY(m_pMainBitmap->w * m_ScaleFactor.m_X, m_pMainBitmap->h * m_ScaleFactor.m_Y);
}


} // namespace RTE
