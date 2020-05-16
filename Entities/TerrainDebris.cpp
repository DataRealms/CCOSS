//////////////////////////////////////////////////////////////////////////////////////////
// File:            TerrainDebris.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Source file for the TerrainDebris class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "TerrainDebris.h"
#include "RTETools.h"
#include "SLTerrain.h"

namespace RTE {

ConcreteClassInfo(TerrainDebris, Entity, 0)


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this TerrainDebris, effectively
//                  resetting the members of this abstraction level only.

void TerrainDebris::Clear()
{
    m_DebrisFile.Reset();
    m_aBitmaps = 0;
    m_BitmapCount = 0;
    m_Material.Reset();
    m_TargetMaterial.Reset();
    m_OnlyOnSurface = false;
    m_OnlyBuried = false;
    m_MinDepth = 0;
    m_MaxDepth = 10;
    m_Density = 0.01;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the TerrainDebris object ready for use.

int TerrainDebris::Create()
{
    if (Entity::Create() < 0)
        return -1;

    // Get the bitmaps
    m_aBitmaps = m_DebrisFile.GetAsAnimation(m_BitmapCount);
    RTEAssert(m_aBitmaps && m_aBitmaps[0], "Failed to load debris bitmaps!")

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a TerrainDebris to be identical to another, by deep copy.

int TerrainDebris::Create(const TerrainDebris &reference)
{
    Entity::Create(reference);

    m_DebrisFile = reference.m_DebrisFile;
    m_aBitmaps = reference.m_aBitmaps;
    m_BitmapCount = reference.m_BitmapCount;
    m_Material = reference.m_Material;
    m_TargetMaterial = reference.m_TargetMaterial;
    m_OnlyOnSurface = reference.m_OnlyOnSurface;
    m_OnlyBuried = reference.m_OnlyBuried;
    m_MinDepth = reference.m_MinDepth;
    m_MaxDepth = reference.m_MaxDepth;
    m_Density = reference.m_Density;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReadProperty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads a property value from a reader stream. If the name isn't
//                  recognized by this class, then ReadProperty of the parent class
//                  is called. If the property isn't recognized by any of the base classes,
//                  false is returned, and the reader's position is untouched.

int TerrainDebris::ReadProperty(std::string propName, Reader &reader)
{
    if (propName == "DebrisFile")
        reader >> m_DebrisFile;
    else if (propName == "DebrisPieceCount")
        reader >> m_BitmapCount;
    else if (propName == "DebrisMaterial")
        reader >> m_Material;
    else if (propName == "TargetMaterial")
        reader >> m_TargetMaterial;
    else if (propName == "OnlyOnSurface")
        reader >> m_OnlyOnSurface;
    else if (propName == "OnlyBuried")
        reader >> m_OnlyBuried;
    else if (propName == "MinDepth")
        reader >> m_MinDepth;
    else if (propName == "MaxDepth")
        reader >> m_MaxDepth;
    else if (propName == "DensityPerMeter")
        reader >> m_Density;
    else
        // See if the base class(es) can find a match instead
        return Entity::ReadProperty(propName, reader);

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this TerrainDebris with a Writer for
//                  later recreation with Create(Reader &reader);

int TerrainDebris::Save(Writer &writer) const
{
    Entity::Save(writer);

    writer.NewProperty("DebrisFile");
    writer << m_DebrisFile;
    writer.NewProperty("DebrisPieceCount");
    writer << m_BitmapCount;
    writer.NewProperty("DebrisMaterial");
    writer << m_Material;
    writer.NewProperty("TargetMaterial");
    writer << m_TargetMaterial;
    writer.NewProperty("OnlyOnSurface");
    writer << m_OnlyOnSurface;
    writer.NewProperty("OnlyBuried");
    writer << m_OnlyBuried;
    writer.NewProperty("MinDepth");
    writer << m_MinDepth;
    writer.NewProperty("MaxDepth");
    writer << m_MaxDepth;
    writer.NewProperty("DensityPerMeter");
    writer << m_Density;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the TerrainDebris object.

void TerrainDebris::Destroy(bool notInherited)
{
    // Don't delete bitmaps since they are owned in the CoententFile static maps

    if (!notInherited)
        Entity::Destroy();
    Clear();
}

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: TerrainDebris addition
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Addition operator overload for TerrainDebris:s which adds together the
//                  Atom:s of two Groups and merges them into one.

TerrainDebris operator+(const TerrainDebris &lhs, const TerrainDebris &rhs)
{
    TerrainDebris returnAG(lhs);
    returnAG.SetOwner(lhs.GetOwner());
    returnAG.AddAtoms(rhs.GetAtomList());
    return returnAG;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Friend operator: TerrainDebris pointer addition
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Addition operator overload for TerrainDebris:s pointer which adds together
//                  the Atom:s of two Groups pointed to and merges them into one.
//                  Ownership of the returned dallocated AG IS TRANSFERRED!

TerrainDebris * operator+(const TerrainDebris *lhs, const TerrainDebris *rhs)
{
    TerrainDebris *pReturnAG = new TerrainDebris(*lhs);
    pReturnAG->SetOwner(lhs->GetOwner());
    pReturnAG->AddAtoms(rhs->GetAtomList());
    return pReturnAG;
}
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ApplyDebris
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Applies the debris to an SLTerrain as its read parameters specify.
// Arguments:       Pointer to the terrain to place the debris on. Ownership is NOT xferred!
// Return value:    None.

void TerrainDebris::ApplyDebris(SLTerrain *pTerrain)
{
    RTEAssert(m_aBitmaps && m_BitmapCount > 0, "No bitmaps loaded for terrain debris!");

    BITMAP *pTerrBitmap = pTerrain->GetFGColorBitmap();
    BITMAP *pMatBitmap = pTerrain->GetMaterialBitmap();

    acquire_bitmap(pTerrBitmap);
    acquire_bitmap(pMatBitmap);

    int terrainWidth = pTerrBitmap->w;
    // How many pieces of debris we're spreading out.
    int  pieceCount = (terrainWidth * g_FrameMan.GetMPP()) * m_Density;
    int currentBitmap, x, y, depth, checkPixel;
    bool place = false;
    // First is index into the bitmap array, second is blit location
    list<pair<int, Vector> > piecesToPlace;
    Vector location;
    Box pieceBox;

    for (int piece = 0; piece < pieceCount; ++piece)
    {
        place = false;
        currentBitmap = floorf((float)m_BitmapCount * PosRand());
        RTEAssert(currentBitmap >= 0 && currentBitmap < m_BitmapCount, "Bitmap index is out of bounds!");

        pieceBox.SetWidth(m_aBitmaps[currentBitmap]->w);
        pieceBox.SetHeight(m_aBitmaps[currentBitmap]->h);

        x = floorf((float)terrainWidth * PosRand());
        depth = floorf(RangeRand(m_MinDepth, m_MaxDepth));

        y = 0;
        while (y < pTerrBitmap->h)
        {
            // Find the air-terrain boundary
            for (; y < pTerrBitmap->h; ++y)
            {
                checkPixel = _getpixel(pMatBitmap, x, y);
				// Check for terrain hit
				if (checkPixel != g_MaterialAir)
				{
					// Found target material
					if (checkPixel == m_TargetMaterial.GetIndex())
					{
						place = true;
						break;
					}
					// If we didn't hit target material, but are specified to, then don't place
					else if (m_OnlyOnSurface)
					{
						place = false;
						break;
					}
				}
            }

            // If we're not placing, then skip to next
            if (!place)
                break;

            // The target locations are on the center of the objects; if supposed to be buried, move down so it is
			y += depth + (m_OnlyBuried ? pieceBox.GetHeight() * 0.6 : 0);
			pieceBox.SetCenter(Vector(x, y));

            // Make sure we're not trying to place something into a cave or other air pocket
			if (!g_SceneMan.GetTerrain()->IsAirPixel(x, y) && (!m_OnlyBuried || g_SceneMan.GetTerrain()->IsBoxBuried(pieceBox)))
            {
                // Do delayed drawing so that we don't end up placing things on top of each other
				piecesToPlace.push_back(pair<int, Vector>(currentBitmap, pieceBox.GetCorner()));
                break;
            }
			// Try deeper if we didn't get properly buried
        }
    }

    // Now draw all the debris onto the terrain
    for (list<pair<int, Vector> >::iterator itr = piecesToPlace.begin(); itr != piecesToPlace.end(); ++itr)
    {
        // Draw the color sprite onto the terrain color layer.
        draw_sprite(pTerrBitmap, m_aBitmaps[itr->first], itr->second.m_X, itr->second.m_Y);
        // Draw the material representation onto the terrain's material layer
        draw_character_ex(pMatBitmap, m_aBitmaps[itr->first], itr->second.m_X, itr->second.m_Y, m_Material.GetIndex(), -1);
    }

    release_bitmap(pTerrBitmap);
    release_bitmap(pMatBitmap);
    pTerrBitmap = 0;
    pMatBitmap = 0;
}

} // namespace RTE
