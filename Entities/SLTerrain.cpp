//////////////////////////////////////////////////////////////////////////////////////////
// File:            SLTerrain.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Source file for the SLTerrain class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "SLTerrain.h"
#include "TerrainDebris.h"
#include "TerrainObject.h"
#include "PresetMan.h"
#include "DataModule.h"
#include "SceneObject.h"
#include "MOPixel.h"
#include "MOSprite.h"
#include "Atom.h"

namespace RTE {

ConcreteClassInfo(SLTerrain, SceneLayer, 0)

const string SLTerrain::TerrainFrosting::c_ClassName = "TerrainFrosting";
BITMAP * SLTerrain::m_spTempBitmap16 = 0;
BITMAP * SLTerrain::m_spTempBitmap32 = 0;
BITMAP * SLTerrain::m_spTempBitmap64 = 0;
BITMAP * SLTerrain::m_spTempBitmap128 = 0;
BITMAP * SLTerrain::m_spTempBitmap256 = 0;
BITMAP * SLTerrain::m_spTempBitmap512 = 0;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this TerrainFrosting, effectively
//                  resetting the members of this abstraction level only.

void SLTerrain::TerrainFrosting::Clear()
{
    m_TargetMaterial.Reset();
    m_FrostingMaterial.Reset();
    m_MinThickness = 5;
    m_MaxThickness = 5;
    m_InAirOnly = true;
}

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the TerrainFrosting object ready for use.

int SLTerrain::TerrainFrosting::Create()
{
    if (Serializable::Create() < 0)
        return -1;

    return 0;
}
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a TerrainFrosting to be identical to another, by deep copy.

int SLTerrain::TerrainFrosting::Create(const TerrainFrosting &reference)
{
    m_TargetMaterial = reference.m_TargetMaterial;
    m_FrostingMaterial = reference.m_FrostingMaterial;
    m_MinThickness = reference.m_MinThickness;
    m_MaxThickness = reference.m_MaxThickness;
    m_InAirOnly = reference.m_InAirOnly;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReadProperty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads a property value from a reader stream. If the name isn't
//                  recognized by this class, then ReadProperty of the parent class
//                  is called. If the property isn't recognized by any of the base classes,
//                  false is returned, and the reader's position is untouched.

int SLTerrain::TerrainFrosting::ReadProperty(const std::string_view &propName, Reader &reader)
{
    if (propName == "TargetMaterial")
        reader >> m_TargetMaterial;
    else if (propName == "FrostingMaterial")
        reader >> m_FrostingMaterial;
    else if (propName == "MinThickness")
        reader >> m_MinThickness;
    else if (propName == "MaxThickness")
        reader >> m_MaxThickness;
    else if (propName == "InAirOnly")
        reader >> m_InAirOnly;
    else
        return Serializable::ReadProperty(propName, reader);

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this TerrainFrosting with a Writer for
//                  later recreation with Create(Reader &reader);

int SLTerrain::TerrainFrosting::Save(Writer &writer) const
{
    Serializable::Save(writer);

    writer.NewProperty("TargetMaterial");
    writer << m_TargetMaterial;
    writer.NewProperty("FrostingMaterial");
    writer << m_FrostingMaterial;
    writer.NewProperty("MinThickness");
    writer << m_MinThickness;
    writer.NewProperty("MaxThickness");
    writer << m_MaxThickness;
    writer.NewProperty("InAirOnly");
    writer << m_InAirOnly;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this SLTerrain, effectively
//                  resetting the members of this abstraction level only.

void SLTerrain::Clear()
{
    m_pFGColor = 0;
    m_pBGColor = 0;
    m_pStructural = 0;
    m_BGTextureFile.Reset();
    m_TerrainFrostings.clear();
    m_TerrainDebris.clear();
    m_TerrainObjects.clear();
    m_UpdatedMateralAreas.clear();
    m_DrawMaterial = false;
	m_NeedToClearFrostings = false;
	m_NeedToClearDebris = false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the SLTerrain object ready for use.

int SLTerrain::Create()
{
    if (SceneLayer::Create() < 0)
        return -1;

    if (!m_pFGColor)
        m_pFGColor = new SceneLayer();
    if (!m_pBGColor)
        m_pBGColor = new SceneLayer();

    // Can't create these earlier in the static declaration because allegro_init needs to be called before create_bitmap
    if (!m_spTempBitmap16)
        m_spTempBitmap16 = create_bitmap_ex(8, 16, 16);
    if (!m_spTempBitmap32)
        m_spTempBitmap32 = create_bitmap_ex(8, 32, 32);
    if (!m_spTempBitmap64)
        m_spTempBitmap64 = create_bitmap_ex(8, 64, 64);
    if (!m_spTempBitmap128)
        m_spTempBitmap128 = create_bitmap_ex(8, 128, 128);
    if (!m_spTempBitmap256)
        m_spTempBitmap256 = create_bitmap_ex(8, 256, 256);
    if (!m_spTempBitmap512)
        m_spTempBitmap512 = create_bitmap_ex(8, 512, 512);

    return 0;
}

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the SLTerrain object ready for use.

int SLTerrain::Create(char *filename,
                    bool drawTrans,
                    Vector offset,
                    bool wrapX,
                    bool wrapY,
                    Vector scrollInfo)
{
    //SceneLayer::Create();

    return 0;
}
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a SLTerrain to be identical to another, by deep copy.

int SLTerrain::Create(const SLTerrain &reference)
{
    SceneLayer::Create(reference);

    // Leave these because they are loaded late by LoadData
    m_pFGColor = dynamic_cast<SceneLayer *>(reference.m_pFGColor->Clone());
    m_pBGColor = dynamic_cast<SceneLayer *>(reference.m_pBGColor->Clone());
//    m_pStructural;
    m_BGTextureFile = reference.m_BGTextureFile;

    ////////////////////////////
    // Frostings
    m_TerrainFrostings.clear();
    for (list<TerrainFrosting>::const_iterator tfItr = reference.m_TerrainFrostings.begin(); tfItr != reference.m_TerrainFrostings.end(); ++tfItr)
        m_TerrainFrostings.push_back(TerrainFrosting((*tfItr)));

    //////////////////////
    // Debris
    // First have to clear out what we've got
    for (list<TerrainDebris *>::iterator tdItr = m_TerrainDebris.begin(); tdItr != m_TerrainDebris.end(); ++tdItr)
        delete (*tdItr);
    m_TerrainDebris.clear();
    // Then add the copies
    for (list<TerrainDebris *>::const_iterator tdItr = reference.m_TerrainDebris.begin(); tdItr != reference.m_TerrainDebris.end(); ++tdItr)
        m_TerrainDebris.push_back(dynamic_cast<TerrainDebris *>((*tdItr)->Clone()));

    /////////////////
    // Objects
    // First clear out what we've got
    for (list<TerrainObject *>::iterator toItr = m_TerrainObjects.begin(); toItr != m_TerrainObjects.end(); ++toItr)
        delete (*toItr);
    m_TerrainObjects.clear();
    // Then add the copies
    for (list<TerrainObject *>::const_iterator toItr = reference.m_TerrainObjects.begin(); toItr != reference.m_TerrainObjects.end(); ++toItr)
        m_TerrainObjects.push_back(dynamic_cast<TerrainObject *>((*toItr)->Clone()));

    m_DrawMaterial = reference.m_DrawMaterial;

	m_NeedToClearFrostings = true;
	m_NeedToClearDebris = true;

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  LoadData
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Actually loads previously specified/created data into memory. Has
//                  to be done before using this SceneLayer.

int SLTerrain::LoadData()
{
    // Load the materials bitmap into the main bitmap
    if (SceneLayer::LoadData())
        return -1;

    RTEAssert(m_pFGColor, "Terrain's foreground layer not instantiated before trying to load its data!");
    RTEAssert(m_pBGColor, "Terrain's background layer not instantiated before trying to load its data!");

    // Check if our color layers' BITMAP data is also to be loaded from disk, and not be generated from the material bitmap!
    if (m_pFGColor->IsFileData() && m_pBGColor->IsFileData())
    {
        if (m_pFGColor->LoadData() < 0)
        {
            RTEAbort("Could not load the Foreground Color SceneLayer data from file, when a path was specified for it!");
            return -1;
        }
        if (m_pBGColor->LoadData() < 0)
        {
            RTEAbort("Could not load the Background Color SceneLayer data from file, when a path was specified for it!");
            return -1;
        }
        // Ok, we have now loaded the layers in from files, don't need to generate them from the material layer
        InitScrollRatios();
        return 0;
    }

    // Create blank foreground layer
    m_pFGColor->Destroy();
    BITMAP *pFGBitmap = create_bitmap_ex(8, m_pMainBitmap->w, m_pMainBitmap->h);
    if (!pFGBitmap || m_pFGColor->Create(pFGBitmap, true, m_Offset, m_WrapX, m_WrapY, m_ScrollRatio))
    {
        RTEAbort("Failed to create terrain's foreground layer's bitmap!");
        return -1;
    }

    // Create blank background layer
    m_pBGColor->Destroy();
    BITMAP *pBGBitmap = create_bitmap_ex(8, m_pMainBitmap->w, m_pMainBitmap->h);
    if (!pBGBitmap || m_pBGColor->Create(pBGBitmap, true, m_Offset, m_WrapX, m_WrapY, m_ScrollRatio))
    {
        RTEAbort("Failed to create terrain's background layer's bitmap!");
        return -1;
    }

    // Structural integrity calc buffer bitmap
    destroy_bitmap(m_pStructural);
    m_pStructural = create_bitmap_ex(8, m_pMainBitmap->w, m_pMainBitmap->h);
    RTEAssert(m_pStructural, "Failed to allocate BITMAP in Terrain::Create");
    clear_bitmap(m_pStructural);

    ///////////////////////////////////////////////
    // Load and texturize the FG color bitmap, based on the materials defined in the recently loaded (main) material layer!

    int xPos, yPos, matIndex, pixelColor;

    // Temporary references for all the materials' textures and colors, since we'll access them a lot
    BITMAP *apTexBitmaps[256];
    int aColors[256];
    // Null em out so we can tell which ones we've already got once so far
    for (matIndex = 0; matIndex < 256; ++matIndex)
    {
        apTexBitmaps[matIndex] = 0;
        aColors[matIndex] = 0;
    }

    // Get the background texture
    BITMAP *m_pBGTexture = m_BGTextureFile.GetAsBitmap();
    // Get the material palette for quicker access
    Material **apMaterials = g_SceneMan.GetMaterialPalette();
    // Get the Material palette ID mappings local to the DataModule this SLTerrain is loaded from
    const unsigned char *materialMappings = g_PresetMan.GetDataModule(m_BitmapFile.GetDataModuleID())->GetAllMaterialMappings();
    Material *pMaterial = 0;

    // Lock all involved bitmaps
    acquire_bitmap(m_pMainBitmap);
    acquire_bitmap(pFGBitmap);
    acquire_bitmap(pBGBitmap);
    acquire_bitmap(m_pBGTexture);

    // Go through each pixel on the main bitmap, which contains all the material pixels loaded from the bitmap
    // Place texture pixels on the FG layer corresponding to the materials on the main material bitmap
    for (xPos = 0; xPos < m_pMainBitmap->w; ++xPos)
    {
        for (yPos = 0; yPos < m_pMainBitmap->h; ++yPos)
        {
            // Read which material the current pixel represents
            matIndex = _getpixel(m_pMainBitmap, xPos, yPos);
            // Map any materials defined in this data module but initially collided with other material ID's and thus were displaced to other ID's
            if (materialMappings[matIndex] != 0)
            {
                // Assign the mapping and put it onto the material bitmap too
                matIndex = materialMappings[matIndex];
                _putpixel(m_pMainBitmap, xPos, yPos, matIndex);
            }

            // Validate the material, or default to default material
            if (matIndex >= 0 && matIndex < c_PaletteEntriesNumber && apMaterials[matIndex])
                pMaterial = apMaterials[matIndex];
            else
                pMaterial = apMaterials[g_MaterialDefault];

            // If haven't read a pixel of this material before, then get its texture so we can quickly access it
            if (!apTexBitmaps[matIndex])
            {
                // Get, and acquire the texture bitmap if material has any
                if (apTexBitmaps[matIndex] = pMaterial->GetTexture())
                    acquire_bitmap(apTexBitmaps[matIndex]);
            }

            // If actually no texture for the material, then use the material's solid color instead
            if (!apTexBitmaps[matIndex])
            {
                // If the color hasn't been retrieved yet, then do so
                if (!aColors[matIndex])
                    aColors[matIndex] = pMaterial->GetColor().GetIndex();
                // Use the color
                pixelColor = aColors[matIndex];
            }
            // Use the texture's color
            else
            {
//                acquire_bitmap(apTexBitmaps[matIndex]);
                pixelColor = _getpixel(apTexBitmaps[matIndex], xPos % apTexBitmaps[matIndex]->w, yPos % apTexBitmaps[matIndex]->h);
            }

            // Draw the correct color pixel on the foreground
            _putpixel(pFGBitmap, xPos, yPos, pixelColor);

            // Draw background texture on the background where this is stuff on the foreground
            if (m_pBGTexture && pixelColor != g_MaskColor)
            {
                pixelColor = _getpixel(m_pBGTexture, xPos % m_pBGTexture->w, yPos % m_pBGTexture->h);
                _putpixel(pBGBitmap, xPos, yPos, pixelColor);
            }
            // Put a keycolor pixel in the bg otherwise
            else
                _putpixel(pBGBitmap, xPos, yPos, g_MaskColor);
        }
    }

    ///////////////////////////////////////
    // Material frostings application!

    bool targetFound = false, applyingFrosting = false;
    int targetId, frostingId, thickness, thicknessGoal;
    BITMAP *pFrostingTex = 0;
    for (list<TerrainFrosting>::iterator tfItr = m_TerrainFrostings.begin(); tfItr != m_TerrainFrostings.end(); ++tfItr)
    {
        targetId = (*tfItr).GetTargetMaterial().GetIndex();
        frostingId = (*tfItr).GetFrostingMaterial().GetIndex();
        // Try to get the color texture of the frosting material. If fail, we'll use the color isntead
        pFrostingTex = (*tfItr).GetFrostingMaterial().GetTexture();
        if (pFrostingTex)
            acquire_bitmap(pFrostingTex);

        // Loop through all columns
        for (xPos = 0; xPos < m_pMainBitmap->w; ++xPos)
        {
            // Get the thickness for this column
            thicknessGoal = (*tfItr).GetThicknessSample();

            // Work upward from the bottom of each column
            for (yPos = m_pMainBitmap->h - 1; yPos >= 0; --yPos)
            {
                // Read which material the current pixel represents
                matIndex = _getpixel(m_pMainBitmap, xPos, yPos);

                // We've encountered the target material! Prepare to apply frosting as soon as it ends!
                if (!targetFound && matIndex == targetId)
                {
                    targetFound = true;
                    thickness = 0;
                }
                // Target material has ended! See if we shuold start putting on the frosting
                else if (targetFound && matIndex != targetId && thickness <= thicknessGoal)
                {
                    applyingFrosting = true;
                    targetFound = false;
                }

                // If time to put down frosting pixels, then do so IF there is air, OR we're set to ignore what we're overwriting
                if (applyingFrosting && (matIndex == g_MaterialAir || !(*tfItr).InAirOnly()) && thickness <= thicknessGoal)
                {
                    // Get the color either from the frosting material's texture or the solid color
                    if (pFrostingTex)
                        pixelColor = _getpixel(pFrostingTex, xPos % pFrostingTex->w, yPos % pFrostingTex->h);
                    else
                        pixelColor = (*tfItr).GetFrostingMaterial().GetColor().GetIndex();

                    // Put the frosting pixel color on the FG color layer
                    _putpixel(pFGBitmap, xPos, yPos, pixelColor);
                    // Put the material ID pixel on the material layer
                    _putpixel(m_pMainBitmap, xPos, yPos, frostingId);

                    // Keep track of the applied thickness
                    thickness++;
                }
                else
                    applyingFrosting = false;
            }
        }

        if (pFrostingTex)
            release_bitmap(pFrostingTex);
    }

    // Release all involved bitmaps
    release_bitmap(m_pMainBitmap);
    release_bitmap(pFGBitmap);
    release_bitmap(pBGBitmap);
    release_bitmap(m_pBGTexture);

    for (matIndex = 0; matIndex < 256; ++matIndex)
    {
        if (apTexBitmaps[matIndex])
            release_bitmap(apTexBitmaps[matIndex]);
    }

    ///////////////////////////////////////////////
    // TerrainDebris application

    for (list<TerrainDebris *>::iterator tdItr = m_TerrainDebris.begin(); tdItr != m_TerrainDebris.end(); ++tdItr)
    {
        (*tdItr)->ApplyDebris(this);
    }

    ///////////////////////////////////////////////
    // Now take care of the TerrainObjects

    for (list<TerrainObject *>::iterator toItr = m_TerrainObjects.begin(); toItr != m_TerrainObjects.end(); ++toItr)
    {
        ApplyTerrainObject(*toItr);
    }
    CleanAir();

    InitScrollRatios();

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SaveData
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves currently data in memory to disk.

int SLTerrain::SaveData(string pathBase)
{
    if (pathBase.empty())
        return -1;

    // Save the bitmap of the material bitmap
    if (SceneLayer::SaveData(pathBase + " Mat.bmp") < 0)
    {
        RTEAbort("Failed to write the material bitmap data saving an SLTerrain!");
        return -1;
    }
    // Then the foreground color layer
    if (m_pFGColor->SaveData(pathBase + " FG.bmp") < 0)
    {
        RTEAbort("Failed to write the FG color bitmap data saving an SLTerrain!");
        return -1;
    }
    // Then the background color layer
    if (m_pBGColor->SaveData(pathBase + " BG.bmp") < 0)
    {
        RTEAbort("Failed to write the BG color bitmap data saving an SLTerrain!");
        return -1;
    }

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ClearData
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears out any previously loaded bitmap data from memory.

int SLTerrain::ClearData()
{
    // Clear the material layer
    if (SceneLayer::ClearData() < 0)
    {
        RTEAbort("Failed to clear material bitmap data of an SLTerrain!");
        return -1;
    }
    // Clear the foreground color layer
    if (m_pFGColor && m_pFGColor->ClearData() < 0)
    {
        RTEAbort("Failed to clear the foreground color bitmap data of an SLTerrain!");
        return -1;
    }
    // Clear the background color layer
    if (m_pBGColor && m_pBGColor->ClearData() < 0)
    {
        RTEAbort("Failed to clear the background color bitmap data of an SLTerrain!");
        return -1;
    }

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReadProperty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads a property value from a reader stream. If the name isn't
//                  recognized by this class, then ReadProperty of the parent class
//                  is called. If the property isn't recognized by any of the base classes,
//                  false is returned, and the reader's position is untouched.

int SLTerrain::ReadProperty(const std::string_view &propName, Reader &reader)
{
    if (propName == "BackgroundTexture")
        reader >> m_BGTextureFile;
    else if (propName == "FGColorLayer")
    {
        delete m_pFGColor;
        m_pFGColor = new SceneLayer();
        reader >> m_pFGColor;
    }
    else if (propName == "BGColorLayer")
    {
        delete m_pBGColor;
        m_pBGColor = new SceneLayer();
        reader >> m_pBGColor;
    }
    else if (propName == "AddTerrainFrosting")
    {
		// Clear frostings if we derived them from some other SLTerrain object
		// and then read another set from explicit terrain definition
		if (m_NeedToClearFrostings)
		{
			m_NeedToClearFrostings = false;
			m_TerrainFrostings.clear();
		}

        TerrainFrosting frosting;
        reader >> frosting;
        m_TerrainFrostings.push_back(frosting);
    }
    else if (propName == "AddTerrainDebris")
    {
		// Clear debris if we derived them from some other SLTerrain object
		// and then read another set from explicit terrain definition
		if (m_NeedToClearDebris)
		{
			m_NeedToClearDebris = false;
			m_TerrainDebris.clear();
		}

        TerrainDebris *pTerrainDebris = new TerrainDebris;
        reader >> pTerrainDebris;
        m_TerrainDebris.push_back(pTerrainDebris);
    }
    else if (propName == "AddTerrainObject" || propName == "PlaceTerrainObject")
    {
        TerrainObject *pTerrainObject = new TerrainObject;
        reader >> pTerrainObject;
        m_TerrainObjects.push_back(pTerrainObject);
    }
    else
        return SceneLayer::ReadProperty(propName, reader);

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this SceneLayer with a Writer for
//                  later recreation with Create(Reader &reader);

int SLTerrain::Save(Writer &writer) const
{
    SceneLayer::Save(writer);

    // Only write the background texture info if the background itself is not saved out as a file already
    if (m_pBGColor->IsFileData())
    {
        writer.NewProperty("BGColorLayer");
        writer << m_pBGColor;
    }
    else
    {
        writer.NewProperty("BackgroundTexture");
        writer << m_BGTextureFile;
    }

    // Only if we haven't saved out the FG as a done and altered bitmap file should we save the procedural params here
    if (m_pFGColor->IsFileData())
    {
        writer.NewProperty("FGColorLayer");
        writer << m_pFGColor;
    }
    // Layer data is not saved into a bitmap file yet, so just write out the procedural params to build the terrain
    else
    {
        for (list<TerrainFrosting>::const_iterator tfItr = m_TerrainFrostings.begin(); tfItr != m_TerrainFrostings.end(); ++tfItr)
        {
            writer.NewProperty("AddTerrainFrosting");
            writer << *tfItr;
        }
        for (list<TerrainDebris *>::const_iterator tdItr = m_TerrainDebris.begin(); tdItr != m_TerrainDebris.end(); ++tdItr)
        {
            writer.NewProperty("AddTerrainDebris");
            writer << *tdItr;
        }
        for (list<TerrainObject *>::const_iterator toItr = m_TerrainObjects.begin(); toItr != m_TerrainObjects.end(); ++toItr)
        {
            // Write out only what is needed to place a copy of this in the Terrain
            writer.NewProperty("AddTerrainObject");
            writer.ObjectStart((*toItr)->GetClassName());
            writer.NewProperty("CopyOf");
            writer << (*toItr)->GetModuleAndPresetName();
            writer.NewProperty("Position");
            writer << (*toItr)->GetPos();
            writer.ObjectEnd();
        }
    }

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the SLTerrain object.

void SLTerrain::Destroy(bool notInherited)
{
    delete m_pFGColor;
    delete m_pBGColor;
    destroy_bitmap(m_pStructural);

    for (list<TerrainDebris *>::iterator tdItr = m_TerrainDebris.begin(); tdItr != m_TerrainDebris.end(); ++tdItr)
    {
        delete (*tdItr);
        *tdItr = 0;
    }
    for (list<TerrainObject *>::iterator toItr = m_TerrainObjects.begin(); toItr != m_TerrainObjects.end(); ++toItr)
    {
        delete (*toItr);
        *toItr = 0;
    }

    if (!notInherited)
        SceneLayer::Destroy();
    Clear();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetFGColorPixel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets a specific pixel from the foreground color bitmap of this.
//                  LockBitmaps() must be called before using this method.

unsigned char SLTerrain::GetFGColorPixel(const int pixelX, const int pixelY) const
{
    int posX = pixelX;
    int posY = pixelY;

    WrapPosition(posX, posY);

    // If it's still below or to the sides out of bounds after
    // wrapping what is supposed to be wrapped, shit is out of bounds.
    if (posX < 0 || posX >= m_pMainBitmap->w || posY >= m_pMainBitmap->h)
        return g_MaskColor;

    // If above terrain bitmap, return key color.
    if (posY < 0)
        return g_MaskColor;

//    RTEAssert(m_pMainBitmap->m_LockCount > 0, "Trying to access unlocked terrain bitmap");
    return _getpixel(m_pFGColor->GetBitmap(), posX, posY);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetBGColorPixel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets a specific pixel from the background color bitmap of this.
//                  LockBitmaps() must be called before using this method.

unsigned char SLTerrain::GetBGColorPixel(const int pixelX, const int pixelY) const
{
    int posX = pixelX;
    int posY = pixelY;

    WrapPosition(posX, posY);

    // If it's still below or to the sides out of bounds after
    // wrapping what is supposed to be wrapped, shit is out of bounds.
    if (posX < 0 || posX >= m_pMainBitmap->w || posY >= m_pMainBitmap->h)
        return g_MaskColor;

    // If above terrain bitmap, return key color.
    if (posY < 0)
        return g_MaskColor;

//    RTEAssert(m_pMainBitmap->m_LockCount > 0, "Trying to access unlocked terrain bitmap");
    return _getpixel(m_pBGColor->GetBitmap(), posX, posY);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetMaterialPixel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets a specific pixel from the material bitmap of this SceneLayer.
//                  LockMaterialBitmap() must be called before using this method.

unsigned char SLTerrain::GetMaterialPixel(const int pixelX, const int pixelY) const
{
    int posX = pixelX;
    int posY = pixelY;

    WrapPosition(posX, posY);

    // If it's still below or to the sides out of bounds after
    // wrapping what is supposed to be wrapped, shit is out of bounds.
    if (posX < 0 || posX >= m_pMainBitmap->w || posY >= m_pMainBitmap->h)
//        return g_MaterialOutOfBounds;
        return g_MaterialAir;

    // If above terrain bitmap, return air material.
    if (posY < 0)
        return g_MaterialAir;

//    RTEAssert(m_pMainBitmap->m_LockCount > 0, "Trying to access unlocked terrain bitmap");
    return _getpixel(m_pMainBitmap, posX, posY);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsAirPixel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether a terrain pixel is of air or cavity material.

bool SLTerrain::IsAirPixel(const int pixelX, const int pixelY) const
{
    int posX = pixelX;
    int posY = pixelY;

    WrapPosition(posX, posY);

    // If it's still below or to the sides out of bounds after
    // wrapping what is supposed to be wrapped, shit is out of bounds.
    if (posX < 0 || posX >= m_pMainBitmap->w || posY >= m_pMainBitmap->h)
//        return g_MaterialOutOfBounds;
        return true;

    // If above terrain bitmap, return air material.
    if (posY < 0)
        return true;

	int checkPixel = _getpixel(m_pMainBitmap, posX, posY);
//    RTEAssert(m_pMainBitmap->m_LockCount > 0, "Trying to access unlocked terrain bitmap");
    return checkPixel == g_MaterialAir || checkPixel == g_MaterialCavity;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetFGColorPixel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets a specific pixel on the FG Color bitmap of this SLTerrain to a
//                  specific color. LockBitmaps() must be called before using this
//                  method.

void SLTerrain::SetFGColorPixel(const int pixelX, const int pixelY, const int color)
{
    int posX = pixelX;
    int posY = pixelY;

    WrapPosition(posX, posY);

    // If it's still below or to the sides out of bounds after
    // wrapping what is supposed to be wrapped, shit is out of bounds.
    if (posX < 0 ||
       posX >= m_pMainBitmap->w ||
       posY < 0 ||
       posY >= m_pMainBitmap->h)
       return;

//    RTEAssert(m_pFGColor->GetBitmap()->m_LockCount > 0, "Trying to access unlocked terrain bitmap");
    _putpixel(m_pFGColor->GetBitmap(), posX, posY, color);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetBGColorPixel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets a specific pixel on the BG Color bitmap of this SLTerrain to a
//                  specific color. LockBitmaps() must be called before using this
//                  method.

void SLTerrain::SetBGColorPixel(const int pixelX, const int pixelY, const int color)
{
    int posX = pixelX;
    int posY = pixelY;

    WrapPosition(posX, posY);

    // If it's still below or to the sides out of bounds after
    // wrapping what is supposed to be wrapped, shit is out of bounds.
    if (posX < 0 ||
       posX >= m_pMainBitmap->w ||
       posY < 0 ||
       posY >= m_pMainBitmap->h)
       return;

//    RTEAssert(m_pBGColor->GetBitmap()->m_LockCount > 0, "Trying to access unlocked terrain bitmap");
    _putpixel(m_pBGColor->GetBitmap(), posX, posY, color);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetMaterialPixel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets a specific pixel on the material bitmap of this SLTerrain to a
//                  specific material. LockMaterialBitmap() must be called before using this
//                  method.

void SLTerrain::SetMaterialPixel(const int pixelX, const int pixelY, const unsigned char material)
{
    int posX = pixelX;
    int posY = pixelY;

    WrapPosition(posX, posY);

    // If it's still below or to the sides out of bounds after
    // wrapping what is supposed to be wrapped, shit is out of bounds.
    if (posX < 0 ||
       posX >= m_pMainBitmap->w ||
       posY < 0 ||
       posY >= m_pMainBitmap->h)
       return;
//    RTEAssert(m_pMainBitmap->m_LockCount > 0, "Trying to access unlocked terrain bitmap");
    _putpixel(m_pMainBitmap, posX, posY, material);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          EraseSilhouette
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Takes a BITMAP and scans through the pixels on this terrain for pixels
//                  which overlap with it. Erases them from the terrain and can optionally
//                  generate MOPixel:s based on the erased or 'dislodged' terrain pixels.

deque<MOPixel *> SLTerrain::EraseSilhouette(BITMAP *pSprite,
                                            Vector pos,
                                            Vector pivot,
                                            Matrix rotation,
                                            float scale,
                                            bool makeMOPs,
                                            int skipMOP,
                                            int maxMOPs)
{
// TODO: OPTIMIZE THIS, IT'S A TIME HOG. MAYBE JSUT STAMP THE OUTLINE AND SAMPLE SOME RANDOM PARTICLES?

    RTEAssert(pSprite, "Null BITMAP passed to SLTerrain::EraseSilhouette");

    deque<MOPixel *> MOPDeque;

    // This will point to the chosen intermediate test bitmap
    BITMAP *pTempBitmap = 0;

    // Find the maximum possible sized bitmap that the passed-in sprite will need

    int halfWidth = pSprite->w / 2;
    int halfHeight = pSprite->h / 2;
    double SinA = sin(rotation.GetRadAngle());
    double CosA = cos(rotation.GetRadAngle());
    int maxWidth = (pSprite->w + abs(pivot.m_X - halfWidth)) * scale;
    int maxHeight = (pSprite->h + abs(pivot.m_Y - halfHeight)) * scale;
    int maxDiameter = sqrt((float)(maxWidth * maxWidth + maxHeight * maxHeight)) * 2;
    int skipCount = skipMOP;

    // Choose an appropriate size
    if (maxDiameter  >= 256)
        pTempBitmap = m_spTempBitmap512;
    if (maxDiameter  >= 128)
        pTempBitmap = m_spTempBitmap256;
    else if (maxDiameter  >= 64)
        pTempBitmap = m_spTempBitmap128;
    else if (maxDiameter >= 32)
        pTempBitmap = m_spTempBitmap64;
    else if (maxDiameter >= 16)
        pTempBitmap = m_spTempBitmap32;
    else
        pTempBitmap = m_spTempBitmap16;

    // Clear and draw the source sprite onto the test bitmap
    clear_bitmap(pTempBitmap);
    pivot_scaled_sprite(pTempBitmap, pSprite, pTempBitmap->w / 2, pTempBitmap->h / 2, pivot.m_X, pivot.m_Y,  ftofix(rotation.GetAllegroAngle()), ftofix(scale));

    // Do the test of intersection between color pixels of the test bitmap and non-air pixels of the terrain
    // Generate and collect MOPixels that represent the terrain overlap and clear the same pixels out of the terrain
    int testX, testY, terrX, terrY;
    MOPixel *pPixel = 0;
	Material const * sceneMat = g_SceneMan.GetMaterialFromID(g_MaterialAir); 
	Material const * spawnMat = g_SceneMan.GetMaterialFromID(g_MaterialAir);
    halfWidth = pTempBitmap->w / 2;
    halfHeight = pTempBitmap->h / 2;
    unsigned char testPixel = 0, matPixel = 0, colorPixel = 0;

    for (testY = 0; testY < pTempBitmap->h; ++testY)
    {
        for (testX = 0; testX < pTempBitmap->w; ++testX)
        {
            terrX = pos.m_X - halfWidth + testX;
            terrY = pos.m_Y - halfHeight + testY;

            // Make sure we're checking within bounds
            if (terrX < 0) {
                if (m_WrapX)
                    while (terrX < 0)
                        terrX += m_pMainBitmap->w;
                else
                    continue;
            }
            if (terrY < 0) {
                if (m_WrapY)
                    while (terrY < 0)
                        terrY += m_pMainBitmap->h;
                else
                    continue;
            }
            if (terrX >= m_pMainBitmap->w) {
                if (m_WrapX)
                    terrX %= m_pMainBitmap->w;
                else
                    continue;
            }
            if (terrY >= m_pMainBitmap->h) {
                if (m_WrapY)
                    terrY %= m_pMainBitmap->h;
                else
                    continue;
            }

            testPixel = getpixel(pTempBitmap, testX, testY);
            matPixel = getpixel(m_pMainBitmap, terrX, terrY);
            colorPixel = getpixel(m_pFGColor->GetBitmap(), terrX, terrY);

            if (testPixel != g_MaskColor)
            {
                // Only add PixelMO if we're not due to skip any
                if (makeMOPs && matPixel != g_MaterialAir && colorPixel != g_MaskColor && ++skipCount > skipMOP && MOPDeque.size() < maxMOPs)
                {
                    skipCount = 0;
                    sceneMat = g_SceneMan.GetMaterialFromID(matPixel);
                    spawnMat = sceneMat->GetSpawnMaterial() ? g_SceneMan.GetMaterialFromID(sceneMat->GetSpawnMaterial()) : sceneMat;
                    // Create the MOPixel based off the Terrain data.
                    pPixel = new MOPixel(colorPixel,
                                         spawnMat->GetPixelDensity(),
                                         Vector(terrX, terrY),
                                         Vector(),
                                         new Atom(Vector(), spawnMat->GetIndex(), 0, colorPixel, 2),
                                         0);

                    pPixel->SetToHitMOs(false);
                    MOPDeque.push_back(pPixel);
                    pPixel = 0;
                }

                // Clear the terrain pixels
                if (matPixel != g_MaterialAir)
                    putpixel(m_pMainBitmap, terrX, terrY, g_MaterialAir);
				if (colorPixel != g_MaskColor)
				{
					putpixel(m_pFGColor->GetBitmap(), terrX, terrY, g_MaskColor);
					g_SceneMan.RegisterTerrainChange(terrX, terrY, 1, 1, g_MaskColor, false);
				}
            }
        }    
    }

    // Add a box to the updated areas list to show there's been change to the materials layer
// TODO: improve fit/tightness of box here
    m_UpdatedMateralAreas.push_back(Box(pos - pivot, maxWidth, maxHeight));

    return MOPDeque;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ApplyObject
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws a passed in Object's graphical and material representations to
//                  this Terrain's respective layers.

bool SLTerrain::ApplyObject(Entity *pEntity)
{
    if (!pEntity)
        return false;

    // Find out what kind it is
    if (MovableObject *pMO = dynamic_cast<MovableObject *>(pEntity))
    {
        ApplyMovableObject(pMO);
        return true;
    }
    else if (TerrainObject *pTO = dynamic_cast<TerrainObject *>(pEntity))
    {
        ApplyTerrainObject(pTO);
        return true;
    }

    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ApplyMovableObject
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws a passed in MovableObject's graphical and material
//                  representations to this Terrain's respective layers.

void SLTerrain::ApplyMovableObject(MovableObject *pMObject)
{
    if (!pMObject)
        return;

    // Determine whether a sprite or just a pixel-based MO
    MOSprite *pMOSprite = dynamic_cast<MOSprite *>(pMObject);

    // Sprite, so try to integrate it into the terrain, with terrain on top
    if (pMOSprite)
    {
        deque<MOPixel *> MOPDeque;

        // This will point to the chosen intermediate test bitmap
        BITMAP *pTempBitmap = 0;
        // Temporary bitmap holder, doesn't own
        BITMAP *pSprite = pMOSprite->GetSpriteFrame();

        float diameter = pMOSprite->GetDiameter();
        // Choose an appropriate size
        if (diameter >= 256)
            pTempBitmap = m_spTempBitmap512;
        if (diameter >= 128)
            pTempBitmap = m_spTempBitmap256;
        else if (diameter >= 64)
            pTempBitmap = m_spTempBitmap128;
        else if (diameter >= 32)
            pTempBitmap = m_spTempBitmap64;
        else if (diameter >= 16)
            pTempBitmap = m_spTempBitmap32;
        else
            pTempBitmap = m_spTempBitmap16;

        // The position of the upper left corner of the temporary bitmap in the scene
        Vector bitmapScroll = pMOSprite->GetPos().GetFloored() - Vector(pTempBitmap->w / 2, pTempBitmap->w / 2);

		Box notUsed;
		
        // COLOR
        // Clear and draw the source sprite onto the temp bitmap
        clear_to_color(pTempBitmap, g_MaskColor);
        // Draw the actor and then the scene foreground to temp bitmap
        pMOSprite->Draw(pTempBitmap, bitmapScroll, g_DrawColor, true);
        m_pFGColor->Draw(pTempBitmap, notUsed, bitmapScroll);
        // Finally draw temporary bitmap to the Scene
        masked_blit(pTempBitmap, GetFGColorBitmap(), 0, 0, bitmapScroll.m_X, bitmapScroll.m_Y, pTempBitmap->w, pTempBitmap->h);

		// Register terrain change
		g_SceneMan.RegisterTerrainChange(bitmapScroll.m_X, bitmapScroll.m_Y, pTempBitmap->w, pTempBitmap->h, g_MaskColor, false);


// TODO: centralize seam drawing!
        // Draw over seams
        if (g_SceneMan.SceneWrapsX())
        {
            if (bitmapScroll.m_X < 0)
                masked_blit(pTempBitmap, GetFGColorBitmap(), 0, 0, bitmapScroll.m_X + g_SceneMan.GetSceneWidth(), bitmapScroll.m_Y, pTempBitmap->w, pTempBitmap->h);
            else if (bitmapScroll.m_X + pTempBitmap->w > g_SceneMan.GetSceneWidth())
                masked_blit(pTempBitmap, GetFGColorBitmap(), 0, 0, bitmapScroll.m_X - g_SceneMan.GetSceneWidth(), bitmapScroll.m_Y, pTempBitmap->w, pTempBitmap->h);
        }
        if (g_SceneMan.SceneWrapsY())
        {
            if (bitmapScroll.m_Y < 0)
                masked_blit(pTempBitmap, GetFGColorBitmap(), 0, 0, bitmapScroll.m_X, bitmapScroll.m_Y + g_SceneMan.GetSceneHeight(), pTempBitmap->w, pTempBitmap->h);
            else if (bitmapScroll.m_Y + pTempBitmap->h > g_SceneMan.GetSceneHeight())
                masked_blit(pTempBitmap, GetFGColorBitmap(), 0, 0, bitmapScroll.m_X, bitmapScroll.m_Y - g_SceneMan.GetSceneHeight(), pTempBitmap->w, pTempBitmap->h);
        }

        // Material
        clear_to_color(pTempBitmap, g_MaterialAir);
        // Draw the actor and then the scene material layer to temp bitmap
        pMOSprite->Draw(pTempBitmap, bitmapScroll, g_DrawMaterial, true);
        SceneLayer::Draw(pTempBitmap, notUsed, bitmapScroll);
        // Finally draw temporary bitmap to the Scene
        masked_blit(pTempBitmap, GetMaterialBitmap(), 0, 0, bitmapScroll.m_X, bitmapScroll.m_Y, pTempBitmap->w, pTempBitmap->h);
        // Add a box to the updated areas list to show there's been change to the materials layer
        m_UpdatedMateralAreas.push_back(Box(bitmapScroll, pTempBitmap->w, pTempBitmap->h));
// TODO: centralize seam drawing!
        // Draw over seams
        if (g_SceneMan.SceneWrapsX())
        {
            if (bitmapScroll.m_X < 0)
                masked_blit(pTempBitmap, GetMaterialBitmap(), 0, 0, bitmapScroll.m_X + g_SceneMan.GetSceneWidth(), bitmapScroll.m_Y, pTempBitmap->w, pTempBitmap->h);
            else if (bitmapScroll.m_X + pTempBitmap->w > g_SceneMan.GetSceneWidth())
                masked_blit(pTempBitmap, GetMaterialBitmap(), 0, 0, bitmapScroll.m_X - g_SceneMan.GetSceneWidth(), bitmapScroll.m_Y, pTempBitmap->w, pTempBitmap->h);
        }
        if (g_SceneMan.SceneWrapsY())
        {
            if (bitmapScroll.m_Y < 0)
                masked_blit(pTempBitmap, GetMaterialBitmap(), 0, 0, bitmapScroll.m_X, bitmapScroll.m_Y + g_SceneMan.GetSceneHeight(), pTempBitmap->w, pTempBitmap->h);
            else if (bitmapScroll.m_Y + pTempBitmap->h > g_SceneMan.GetSceneHeight())
                masked_blit(pTempBitmap, GetMaterialBitmap(), 0, 0, bitmapScroll.m_X, bitmapScroll.m_Y - g_SceneMan.GetSceneHeight(), pTempBitmap->w, pTempBitmap->h);
        }
    }
    // Not a big sprite, so just draw the representations
    else
    {
        pMObject->Draw(GetFGColorBitmap(), Vector(), g_DrawColor, true);
		// Register terrain change
		g_SceneMan.RegisterTerrainChange(pMObject->GetPos().m_X, pMObject->GetPos().m_Y, 1, 1, g_DrawColor, false);

        pMObject->Draw(GetMaterialBitmap(), Vector(), g_DrawMaterial, true);
    }
}



void SLTerrain::RegisterTerrainChange(TerrainObject *pTObject)
{
	if (!pTObject)
		return;

	Vector loc = pTObject->GetPos() + pTObject->GetBitmapOffset();

	if (pTObject->HasBGColor())
	{
		g_SceneMan.RegisterTerrainChange(loc.m_X, loc.m_Y, pTObject->GetBitmapWidth(), pTObject->GetBitmapHeight(), g_MaskColor, true);
	}

	// Register terrain change
	g_SceneMan.RegisterTerrainChange(loc.m_X, loc.m_Y, pTObject->GetBitmapWidth(), pTObject->GetBitmapHeight(), g_MaskColor, false);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ApplyTerrainObject
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws a passed in TerrainObject's graphical and material
//                  representations to this Terrain's respective layers.

void SLTerrain::ApplyTerrainObject(TerrainObject *pTObject)
{
    if (!pTObject)
        return;

    Vector loc = pTObject->GetPos() + pTObject->GetBitmapOffset();

    // Do duplicate drawing if the terrain object straddles a wrapping border
    if (loc.m_X < 0)
    {
        draw_sprite(m_pMainBitmap, pTObject->GetMaterialBitmap(), loc.m_X + m_pMainBitmap->w, loc.m_Y);
        draw_sprite(m_pFGColor->GetBitmap(), pTObject->GetFGColorBitmap(), loc.m_X + m_pFGColor->GetBitmap()->w, loc.m_Y);
        if (pTObject->HasBGColor())
            draw_sprite(m_pBGColor->GetBitmap(), pTObject->GetBGColorBitmap(), loc.m_X + m_pBGColor->GetBitmap()->w, loc.m_Y);
    }
    else if (loc.m_X >= m_pMainBitmap->w - pTObject->GetFGColorBitmap()->w)
    {
        draw_sprite(m_pMainBitmap, pTObject->GetMaterialBitmap(), loc.m_X - m_pMainBitmap->w, loc.m_Y);
        draw_sprite(m_pFGColor->GetBitmap(), pTObject->GetFGColorBitmap(), loc.m_X - m_pFGColor->GetBitmap()->w, loc.m_Y);
        if (pTObject->HasBGColor())
            draw_sprite(m_pBGColor->GetBitmap(), pTObject->GetBGColorBitmap(), loc.m_X - m_pBGColor->GetBitmap()->w, loc.m_Y);
    }

    // Regular drawing
    draw_sprite(m_pMainBitmap, pTObject->GetMaterialBitmap(), loc.m_X, loc.m_Y);
    draw_sprite(m_pFGColor->GetBitmap(), pTObject->GetFGColorBitmap(), loc.m_X, loc.m_Y);
	if (pTObject->HasBGColor())
	{
		draw_sprite(m_pBGColor->GetBitmap(), pTObject->GetBGColorBitmap(), loc.m_X, loc.m_Y);
		g_SceneMan.RegisterTerrainChange(loc.m_X, loc.m_Y, pTObject->GetBitmapWidth(), pTObject->GetBitmapHeight(), g_MaskColor, true);
	}

	// Register terrain change
	g_SceneMan.RegisterTerrainChange(loc.m_X, loc.m_Y, pTObject->GetBitmapWidth(), pTObject->GetBitmapHeight(), g_MaskColor, false);

    // Add a box to the updated areas list to show there's been change to the materials layer
    m_UpdatedMateralAreas.push_back(Box(loc, pTObject->GetMaterialBitmap()->w, pTObject->GetMaterialBitmap()->h));

    // Apply all the child objects of the TO, and first reapply the team so all its children are guaranteed to be on the same team!
    pTObject->SetTeam(pTObject->GetTeam());
    for (list<SceneObject::SOPlacer>::const_iterator itr = pTObject->GetChildObjects().begin(); itr != pTObject->GetChildObjects().end() ; ++itr)
    {
// TODO: check if we're placing a brain, and have it replace the resident brain of the scene!
        // Copy and apply, transferring ownership of the new copy into the application
        g_SceneMan.AddSceneObject((*itr).GetPlacedCopy(pTObject));
    }

//    CleanAir();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsBoxBuried
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Checks whether a bounding box is completely buried in the terrain.

bool SLTerrain::IsBoxBuried(const Box &checkBox) const
{
    bool buried = true;

    // Center
// Not important for buriedness, only corners are, really
//    buried = buried && GetMaterialPixel(checkBox.GetCenter().m_X, checkBox.GetCenter().m_Y) != g_MaterialAir;

    // Corners
    buried = buried && !IsAirPixel(checkBox.GetCorner().m_X, checkBox.GetCorner().m_Y);
    buried = buried && !IsAirPixel(checkBox.GetCorner().m_X + checkBox.GetWidth(), checkBox.GetCorner().m_Y);
    buried = buried && !IsAirPixel(checkBox.GetCorner().m_X, checkBox.GetCorner().m_Y + checkBox.GetHeight());
    buried = buried && !IsAirPixel(checkBox.GetCorner().m_X + checkBox.GetWidth(), checkBox.GetCorner().m_Y + checkBox.GetHeight());

    return buried;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CleanAirBox
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Removes any color pixel in the color layer of this SLTerrain wherever
//                  there is an air material pixel in the material layer. Whether the scene is X-wrapped or Y-wrapped.

void SLTerrain::CleanAirBox(Box box, bool wrapsX, bool wrapsY)
{
    acquire_bitmap(m_pMainBitmap);
    acquire_bitmap(m_pFGColor->GetBitmap());

    int width = m_pMainBitmap->w;
    int height = m_pMainBitmap->h;
    unsigned char matPixel;

	for (int y = box.m_Corner.m_Y; y < box.m_Corner.m_Y + box.m_Height; ++y) {
		for (int x = box.m_Corner.m_X; x < box.m_Corner.m_X + box.m_Width; ++x) {
			float wrapX = x;
			float wrapY = y;

			//Fix coords in case of seam
			if (wrapsX)
			{
				if (wrapX < 0)
					wrapX = wrapX + m_pMainBitmap->w;

				if (wrapX >= m_pMainBitmap->w)
					wrapX = wrapX - m_pMainBitmap->w;
			}

			if (wrapsY)
			{
				if (wrapY < 0)
					wrapY = wrapY + m_pMainBitmap->h;

				if (wrapY >= m_pMainBitmap->h)
					wrapY = wrapY - m_pMainBitmap->h;
			}

			if (wrapX >= 0 && wrapY >=0 && wrapX < width && wrapY < height)
			{
				matPixel = _getpixel(m_pMainBitmap, wrapX, wrapY);
				if (matPixel == g_MaterialCavity) {
					_putpixel(m_pMainBitmap, wrapX, wrapY, g_MaterialAir);
					matPixel = g_MaterialAir;
				}
				if (matPixel == g_MaterialAir)
					_putpixel(m_pFGColor->GetBitmap(), wrapX, wrapY, g_MaskColor);
			}

        }
    }

    release_bitmap(m_pMainBitmap);
    release_bitmap(m_pFGColor->GetBitmap());
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CleanAir
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Removes any color pixel in the color layer of this SLTerrain wherever
//                  there is an air material pixel in the material layer.

void SLTerrain::CleanAir()
{
    acquire_bitmap(m_pMainBitmap);
    acquire_bitmap(m_pFGColor->GetBitmap());

    int width = m_pMainBitmap->w;
    int height = m_pMainBitmap->h;
    unsigned char matPixel;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            matPixel = _getpixel(m_pMainBitmap, x, y);
            if (matPixel == g_MaterialCavity) {
                _putpixel(m_pMainBitmap, x, y, g_MaterialAir);
                matPixel = g_MaterialAir;
            }
            if (matPixel == g_MaterialAir)
                _putpixel(m_pFGColor->GetBitmap(), x, y, g_MaskColor);
        }
    }

    release_bitmap(m_pMainBitmap);
    release_bitmap(m_pFGColor->GetBitmap());
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ClearAllMaterial
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Removes any FG and material pixels completely form teh terrain.
//                  For the editor mostly.

void SLTerrain::ClearAllMaterial()
{
    clear_to_color(m_pMainBitmap, g_MaskColor);
    clear_to_color(m_pFGColor->GetBitmap(), g_MaterialAir);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the state of this SLTerrain. Supposed to be done every frame.

void SLTerrain::Update()
{
    SceneLayer::Update();

    m_pFGColor->SetOffset(m_Offset);
    m_pBGColor->SetOffset(m_Offset);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  DrawBackground
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this SLTerrain's current scrolled position to a bitmap.

void SLTerrain::DrawBackground(BITMAP *pTargetBitmap, Box &targetBox, const Vector &scrollOverride)
{
    m_pBGColor->Draw(pTargetBitmap, targetBox, scrollOverride);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this SLTerrain's current scrolled position to a bitmap.

void SLTerrain::Draw(BITMAP *pTargetBitmap, Box &targetBox, const Vector &scrollOverride) const
{
    if (m_DrawMaterial)
    {
        SceneLayer::Draw(pTargetBitmap, targetBox, scrollOverride);
    }
    else
    {
        m_pFGColor->Draw(pTargetBitmap, targetBox, scrollOverride);
    }
}

} // namespace RTE