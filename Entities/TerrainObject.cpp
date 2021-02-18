//////////////////////////////////////////////////////////////////////////////////////////
// File:            TerrainObject.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Source file for the TerrainObject class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "TerrainObject.h"
#include "PresetMan.h"
#include "ContentFile.h"

namespace RTE {

ConcreteClassInfo(TerrainObject, SceneObject, 0)


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this TerrainObject, effectively
//                  resetting the members of this abstraction level only.

void TerrainObject::Clear()
{
    m_FGColorFile.Reset();
    m_MaterialFile.Reset();
    m_BGColorFile.Reset();
    m_pFGColor = 0;
    m_pMaterial = 0;
    m_pBGColor = 0;
    m_BitmapOffset.Reset();
    m_OffsetDefined = false;
    m_ChildObjects.clear();
	m_DisplayAsTerrain = false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the TerrainObject object ready for use.

int TerrainObject::Create()
{
    if (SceneObject::Create() < 0)
        return -1;

    // If no bitmapoffset is set, make it point to dead center by default
    if (!m_OffsetDefined && m_pFGColor)
    {
/*
        if (m_pFGColor->w > 24)
            m_BitmapOffset.m_X = -std::ceil(((float)m_pFGColor->w / (float)2) + 0.5f);
        if (m_pFGColor->h > 24)
            m_BitmapOffset.m_Y = -std::ceil(((float)m_pFGColor->h / (float)2) + 0.5f);
*/
        if (m_pFGColor->w > 24)
            m_BitmapOffset.m_X = -(m_pFGColor->w / 2);
        if (m_pFGColor->h > 24)
            m_BitmapOffset.m_Y = -(m_pFGColor->h / 2);
    }

    return 0;
}

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the TerrainObject object ready for use.

int TerrainObject::Create(ContentFile *pBitmapFile,
                       bool drawTrans,
                       Vector offset,
                       bool wrapX,
                       bool wrapY,
                       Vector scrollInfo)
{
    m_pBitmapFile = pBitmapFile;

    m_pBitmap = new BITMAP();
    RTEAssert(m_pBitmap, "Failed to allocate BITMAP in TerrainObject::Create");

    if (FAILED(m_pBitmap->Create(g_FrameMan.GetScreen(), m_pBitmapFile->GetDataSize(), m_pBitmapFile->GetContent()))) {
        CDXError(g_FrameMan.GetScreen(), "Could not create TerrainObject bitmap");
        return -1;
    }

    if (m_DrawTrans = drawTrans)
        m_pBitmap->SetColorKey(g_MaskColor);

    m_Offset = offset;

    m_WrapX = wrapX;
    m_WrapY = wrapY;

    if (m_WrapX)
        m_ScrollRatio.m_X = scrollInfo.m_X;
    else
        if (scrollInfo.m_X == -1.0)
            m_ScrollRatio.m_X = 1.0;
        else
            m_ScrollRatio.m_X = (float)(m_pBitmap->GetWidth() - g_FrameMan.GetResX()) /
                                (float)(scrollInfo.m_X - g_FrameMan.GetResX());

    if (m_WrapY)
        m_ScrollRatio.m_Y = scrollInfo.m_Y;
    else
        if (scrollInfo.m_Y == -1.0)
            m_ScrollRatio.m_Y = 1.0;
        else
            m_ScrollRatio.m_Y = (float)(m_pBitmap->GetHeight() - g_FrameMan.GetResY()) /
                                (float)(scrollInfo.m_Y - g_FrameMan.GetResY());

    return 0;
}
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a MOPixel to be identical to another, by deep copy.

int TerrainObject::Create(const TerrainObject &reference)
{
    SceneObject::Create(reference);

    m_FGColorFile = reference.m_FGColorFile;
    m_MaterialFile = reference.m_MaterialFile;
    m_BGColorFile = reference.m_BGColorFile;

    m_pFGColor = reference.m_pFGColor;
    m_pMaterial = reference.m_pMaterial;
    m_pBGColor = reference.m_pBGColor;

    m_BitmapOffset = reference.m_BitmapOffset;
    m_OffsetDefined = reference.m_OffsetDefined;

    for (list<SOPlacer>::const_iterator itr = reference.m_ChildObjects.begin(); itr != reference.m_ChildObjects.end(); ++itr)
        m_ChildObjects.push_back(*itr);

	m_DisplayAsTerrain = reference.m_DisplayAsTerrain;
    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReadProperty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads a property value from a reader stream. If the name isn't
//                  recognized by this class, then ReadProperty of the parent class
//                  is called. If the property isn't recognized by any of the base classes,
//                  false is returned, and the reader's position is untouched.

int TerrainObject::ReadProperty(const std::string_view &propName, Reader &reader)
{
    if (propName == "FGColorFile")
    {
        reader >> m_FGColorFile;
        m_pFGColor = m_FGColorFile.GetAsBitmap();
    }
    else if (propName == "MaterialFile")
    {
        reader >> m_MaterialFile;
        m_pMaterial = m_MaterialFile.GetAsBitmap();
    }
    else if (propName == "BGColorFile")
    {
        reader >> m_BGColorFile;
        m_pBGColor = m_BGColorFile.GetAsBitmap();
    }
    else if (propName == "BitmapOffset")
    {
        reader >> m_BitmapOffset;
        m_OffsetDefined = true;
    }
    // This is to handle legacy placement of terrain objects, without the bitmap offset
    else if (propName == "Location")
    {
        reader >> m_Pos;
        m_Pos -= m_BitmapOffset;
    }
    else if (propName == "AddChildObject")
    {
        SOPlacer newChild;
        reader >> newChild;
        newChild.SetTeam(m_Team);
        m_ChildObjects.push_back(newChild);
    }
    else if (propName == "DisplayAsTerrain")
    {
		reader >> m_DisplayAsTerrain;
	}
    else
        return SceneObject::ReadProperty(propName, reader);

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this TerrainObject with a Writer for
//                  later recreation with Create(Reader &reader);

int TerrainObject::Save(Writer &writer) const
{
    SceneObject::Save(writer);

    writer.NewProperty("FGColorFile");
    writer << m_FGColorFile;
    writer.NewProperty("MaterialFile");
    writer << m_MaterialFile;
    writer.NewProperty("BGColorFile");
    writer << m_BGColorFile;
    if (m_OffsetDefined)
    {
        writer.NewProperty("BitmapOffset");
        writer << m_BitmapOffset;
    }
    for (list<SOPlacer>::const_iterator itr = m_ChildObjects.begin(); itr != m_ChildObjects.end(); ++itr)
    {
        writer.NewProperty("AddChildObject");
        writer << (*itr);
    }

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the TerrainObject object.

void TerrainObject::Destroy(bool notInherited)
{
/* None of these owned by this
    delete m_pFGColor;
    delete m_pMaterial;
    if (m_pBGColor)
        delete m_pBGColor;
*/

    if (!notInherited)
        SceneObject::Destroy();
    Clear();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetGraphicalIcon
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets a bitmap showing a good identifyable icon of this, for use in
//                  GUI lists etc.

BITMAP * TerrainObject::GetGraphicalIcon()
{
    if (!m_pFGColor)
        return m_pBGColor;

    // Checking if the FG has anything to show, if not, show the bg layer isntead
    int piece = m_pFGColor->w / 10;
    if (getpixel(m_pFGColor, m_pFGColor->w / 2, m_pFGColor->h / 2) != g_MaskColor ||
        getpixel(m_pFGColor, piece, piece) != g_MaskColor ||
        getpixel(m_pFGColor, m_pFGColor->w - piece, piece) != g_MaskColor ||
        getpixel(m_pFGColor, piece, m_pFGColor->h - piece) != g_MaskColor)
        return m_pFGColor;
    else
        return m_pBGColor;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  IsOnScenePoint
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether this' current graphical representation overlaps
//                  a point in absolute scene coordinates.

bool TerrainObject::IsOnScenePoint(Vector &scenePoint) const
{
    if (!m_pFGColor)
        return false;
// TODO: TAKE CARE OF WRAPPING
/*
    // Take care of wrapping situations
    bitmapPos = m_Pos + m_BitmapOffset;
    Vector aScenePoint[4];
    aScenePoint[0] = scenePoint;
    int passes = 1;

    // See if need to double draw this across the scene seam if we're being drawn onto a scenewide bitmap
    if (targetPos.IsZero())
    {
        if (g_SceneMan.SceneWrapsX())
        {
            if (bitmapPos.m_X < m_pFGColor->w)
            {
                aScenePoint[passes] = aScenePoint[0];
                aScenePoint[passes].m_X += g_SceneMan.GetSceneWidth();
                passes++;
            }
            else if (aScenePoint[0].m_X > pTargetBitmap->w - m_pFGColor->w)
            {
                aScenePoint[passes] = aScenePoint[0];
                aScenePoint[passes].m_X -= g_SceneMan.GetSceneWidth();
                passes++;
            }
        }
        if (g_SceneMan.SceneWrapsY())
        {
            
        }
    }

    // Check all the passes needed
    for (int i = 0; i < passes; ++i)
    {

        if (IsWithinBox(aScenePoint[i], m_Pos + m_BitmapOffset, m_pFGColor->w, m_pFGColor->h))
        {
            if (getpixel(m_pFGColor, aScenePoint[i].m_X, aScenePoint[i].m_Y) != g_MaskColor ||
               (m_pBGColor && getpixel(m_pBGColor, aScenePoint[i].m_X, aScenePoint[i].m_Y) != g_MaskColor) ||
               (m_pMaterial && getpixel(m_pMaterial, aScenePoint[i].m_X, aScenePoint[i].m_Y) != g_MaterialAir))
               return true;
        }
    }
*/
    Vector bitmapPos = m_Pos + m_BitmapOffset;
    if (WithinBox(scenePoint, bitmapPos, m_pFGColor->w, m_pFGColor->h))
    {
        // Scene point on the bitmap
        Vector bitmapPoint = scenePoint - bitmapPos;
        if (getpixel(m_pFGColor, bitmapPoint.m_X, bitmapPoint.m_Y) != g_MaskColor ||
           (m_pBGColor && getpixel(m_pBGColor, bitmapPoint.m_X, bitmapPoint.m_Y) != g_MaskColor) ||
           (m_pMaterial && getpixel(m_pMaterial, bitmapPoint.m_X, bitmapPoint.m_Y) != g_MaterialAir))
           return true;
    }

    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetTeam
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets which team this Actor belongs to.

void TerrainObject::SetTeam(int team)
{
    SceneObject::SetTeam(team);

    // Make sure all the objects to be placed will be of the same team
    for (list<SOPlacer>::iterator itr = m_ChildObjects.begin(); itr != m_ChildObjects.end(); ++itr)
        (*itr).SetTeam(team);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this TerrainObject's current graphical representation to a
//                  BITMAP of choice.

void TerrainObject::Draw(BITMAP *pTargetBitmap, const Vector &targetPos, DrawMode mode, bool onlyPhysical) const
{
    if (!m_pFGColor)
        RTEAbort("TerrainObject's bitmaps are null when drawing!");

    // Take care of wrapping situations
    Vector aDrawPos[4];
    aDrawPos[0] = m_Pos + m_BitmapOffset - targetPos;
    int passes = 1;

    // See if need to double draw this across the scene seam if we're being drawn onto a scenewide bitmap
	if (targetPos.IsZero() && g_SceneMan.GetSceneWidth() <= pTargetBitmap->w)
    {
        if (aDrawPos[0].m_X < m_pFGColor->w)
        {
            aDrawPos[passes] = aDrawPos[0];
            aDrawPos[passes].m_X += pTargetBitmap->w;
            passes++;
        }
        else if (aDrawPos[0].m_X > pTargetBitmap->w - m_pFGColor->w)
        {
            aDrawPos[passes] = aDrawPos[0];
            aDrawPos[passes].m_X -= pTargetBitmap->w;
            passes++;
        }
    }
    // Only screenwide target bitmap, so double draw within the screen if the screen is straddling a scene seam
    else
    {
        if (g_SceneMan.SceneWrapsX())
        {
            int sceneWidth = g_SceneMan.GetSceneWidth();
            if (targetPos.m_X < 0)
            {
                aDrawPos[passes] = aDrawPos[0];
                aDrawPos[passes].m_X -= sceneWidth;
                passes++;
            }
            if (targetPos.m_X + pTargetBitmap->w > sceneWidth)
            {
                aDrawPos[passes] = aDrawPos[0];
                aDrawPos[passes].m_X += sceneWidth;
                passes++;
            }
        }
    }

    // Draw all the passes needed
    for (int i = 0; i < passes; ++i)
    {
        if (mode == g_DrawColor)
        {
            masked_blit(m_pBGColor, pTargetBitmap, 0, 0, aDrawPos[i].GetFloorIntX(), aDrawPos[i].GetFloorIntY(), m_pBGColor->w, m_pBGColor->h);
            masked_blit(m_pFGColor, pTargetBitmap, 0, 0, aDrawPos[i].GetFloorIntX(), aDrawPos[i].GetFloorIntY(), m_pFGColor->w, m_pFGColor->h);
        }
        else if (mode == g_DrawMaterial)
        {
            masked_blit(m_pMaterial, pTargetBitmap, 0, 0, aDrawPos[i].GetFloorIntX(), aDrawPos[i].GetFloorIntY(), m_pMaterial->w, m_pMaterial->h);
        }
        else if (mode == g_DrawLess)
        {
            masked_blit(m_pFGColor, pTargetBitmap, 0, 0, aDrawPos[i].GetFloorIntX(), aDrawPos[i].GetFloorIntY(), m_pFGColor->w, m_pFGColor->h);
        }
        else if (mode == g_DrawTrans)
        {
            draw_trans_sprite(pTargetBitmap, m_pFGColor, aDrawPos[i].GetFloorIntX(), aDrawPos[i].GetFloorIntY());
            draw_trans_sprite(pTargetBitmap, m_pBGColor, aDrawPos[i].GetFloorIntX(), aDrawPos[i].GetFloorIntY());
        }
    }
}



} // namespace RTE
