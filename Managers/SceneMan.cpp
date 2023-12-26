//////////////////////////////////////////////////////////////////////////////////////////
// File:            SceneMan.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Source file for the SceneMan class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files
#include "NetworkServer.h"
#include "NetworkClient.h"

#include "SceneMan.h"
#include "PresetMan.h"
#include "FrameMan.h"
#include "ThreadMan.h"
#include "ActivityMan.h"
#include "UInputMan.h"
#include "CameraMan.h"
#include "ConsoleMan.h"
#include "PrimitiveMan.h"
#include "SettingsMan.h"
#include "Scene.h"
#include "SLTerrain.h"
#include "SLBackground.h"
#include "TerrainObject.h"
#include "MovableObject.h"
#include "ContentFile.h"
#include "AHuman.h"
#include "ACRocket.h"
#include "MOPixel.h"
#include "Atom.h"
#include "Material.h"
// Temp
#include "Controller.h"

#include "tracy/Tracy.hpp"

namespace RTE
{

#define CLEANAIRINTERVAL 200000

const std::string SceneMan::c_ClassName = "SceneMan";
std::vector<std::pair<int, BITMAP *>> SceneMan::m_IntermediateSettlingBitmaps;

// Stored as a thread-local instead of in the class, because multithreaded Lua scripts will interfere otherwise
thread_local Vector s_LastRayHitPos;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SceneMan::Clear()
{
    m_DefaultSceneName = "Tutorial Bunker";
    m_pSceneToLoad = nullptr;
    m_PlaceObjects = true;
	m_PlaceUnits = true;
    m_pCurrentScene = nullptr;
    m_MOIDsGrid = SpatialPartitionGrid();
    m_pDebugLayer = nullptr;

    m_LayerDrawMode = g_LayerNormal;

    m_MatNameMap.clear();
	m_apMatPalette.fill(nullptr);
    m_MaterialCount = 0;

	m_MaterialCopiesVector.clear();

    m_pUnseenRevealSound = nullptr;
    m_DrawRayCastVisualizations = false;
    m_DrawPixelCheckVisualizations = false;
    m_LastUpdatedScreen = 0;
    m_SecondStructPass = false;
//    m_CalcTimer.Reset();
    m_CleanTimer.Reset();

	if (m_pOrphanSearchBitmap)
		destroy_bitmap(m_pOrphanSearchBitmap);
	m_pOrphanSearchBitmap = create_bitmap_ex(8, MAXORPHANRADIUS , MAXORPHANRADIUS);

	m_ScrapCompactingHeight = 25;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SceneMan::Initialize() const {
	// Can't create these earlier in the static declaration because allegro_init needs to be called before create_bitmap
	m_IntermediateSettlingBitmaps = {
		{ 16, create_bitmap_ex(8, 16, 16) },
		{ 32, create_bitmap_ex(8, 32, 32) },
		{ 48, create_bitmap_ex(8, 48, 48) },
		{ 64, create_bitmap_ex(8, 64, 64) },
		{ 96, create_bitmap_ex(8, 96, 96) },
		{ 128, create_bitmap_ex(8, 128, 128) },
		{ 192, create_bitmap_ex(8, 192, 192) },
		{ 256, create_bitmap_ex(8, 256, 256) },
		{ 384, create_bitmap_ex(8, 384, 384) },
		{ 512, create_bitmap_ex(8, 512, 512) }
	};
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int SceneMan::Create(std::string readerFile)
{
    Reader *reader = new Reader();
    if (reader->Create(readerFile.c_str()))
        g_ConsoleMan.PrintString("ERROR: Could not find Scene definition file!");

    Serializable::Create(*reader);
    delete reader;

    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Material * SceneMan::AddMaterialCopy(Material *mat)
{
	Material * matCopy = dynamic_cast<Material *>(mat->Clone());
	if (matCopy)
		m_MaterialCopiesVector.push_back(matCopy);

	return matCopy;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int SceneMan::LoadScene(Scene *pNewScene, bool placeObjects, bool placeUnits) {
	if (!pNewScene) {
		return -1;
	}

	g_MovableMan.PurgeAllMOs();
	g_PostProcessMan.ClearScenePostEffects();

	if (m_pCurrentScene) {
        // Ensure all async pathing requests are complete
        m_pCurrentScene->BlockUntilAllPathingRequestsComplete();
		delete m_pCurrentScene;
		m_pCurrentScene = nullptr;
	}

	g_NetworkServer.LockScene(true);

    m_pCurrentScene = pNewScene;
    if (m_pCurrentScene->LoadData(placeObjects, true, placeUnits) < 0)
    {
        g_ConsoleMan.PrintString("ERROR: Loading scene \'" + m_pCurrentScene->GetPresetName() + "\' failed! Has it been properly defined?");
		g_NetworkServer.LockScene(false);
		return -1;
    }

    // Report successful load to the console
    g_ConsoleMan.PrintString("SYSTEM: Scene \"" + m_pCurrentScene->GetPresetName() + "\" was loaded");

    // Set the proper scales of the unseen obscuring SceneLayers
    SceneLayer *pUnseenLayer = 0;
    for (int team = Activity::TeamOne; team < Activity::MaxTeamCount; ++team)
    {
        if (!g_ActivityMan.GetActivity()->TeamActive(team))
            continue;
        SceneLayer *pUnseenLayer = m_pCurrentScene->GetUnseenLayer(team);
        if (pUnseenLayer && pUnseenLayer->GetBitmap())
        {
            // Calculate how many times smaller the unseen map is compared to the entire terrain's dimensions, and set it as the scale factor on the Unseen layer
            pUnseenLayer->SetScaleFactor(Vector((float)m_pCurrentScene->GetTerrain()->GetBitmap()->w / (float)pUnseenLayer->GetBitmap()->w, (float)m_pCurrentScene->GetTerrain()->GetBitmap()->h / (float)pUnseenLayer->GetBitmap()->h));
        }
    }

    // Get the unseen reveal sound
    if (!m_pUnseenRevealSound)
        m_pUnseenRevealSound = dynamic_cast<SoundContainer *>(g_PresetMan.GetEntityPreset("SoundContainer", "Unseen Reveal Blip")->Clone());

//    m_pCurrentScene->GetTerrain()->CleanAir();

    // Re-initialize our threadman so it sets up our renderable game state properly
    g_ThreadMan.Initialize();

    const int cellSize = 20;
    m_MOIDsGrid = SpatialPartitionGrid(GetSceneWidth(), GetSceneHeight(), cellSize);

    // Create the Debug SceneLayer
    if (m_DrawRayCastVisualizations || m_DrawPixelCheckVisualizations) {
        delete m_pDebugLayer;
        BITMAP *pBitmap = create_bitmap_ex(8, GetSceneWidth(), GetSceneHeight());
        clear_to_color(pBitmap, g_MaskColor);
        m_pDebugLayer = new SceneLayer();
        m_pDebugLayer->Create(pBitmap, true, Vector(), m_pCurrentScene->WrapsX(), m_pCurrentScene->WrapsY(), Vector(1.0, 1.0));
        pBitmap = nullptr;
    }

    // Finally draw the IDs of the MOs to the MOID grid for the first time
    g_MovableMan.UpdateDrawMOIDs();

	g_NetworkServer.LockScene(false);
	g_NetworkServer.ResetScene();

    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int SceneMan::SetSceneToLoad(std::string sceneName, bool placeObjects, bool placeUnits)
{
    // Use the name passed in to load the preset requested
    const Scene *pSceneRef = dynamic_cast<const Scene *>(g_PresetMan.GetEntityPreset("Scene", sceneName));

    if (!pSceneRef)
    {
        g_ConsoleMan.PrintString("ERROR: Finding Scene preset \'" + sceneName + "\' failed! Has it been properly defined?");
        return -1;
    }

    // Store the scene reference to load later
    SetSceneToLoad(pSceneRef, placeObjects, placeUnits);

    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int SceneMan::LoadScene()
{
    // In case we have no set Scene reference to load from, do something graceful about it
    if (!m_pSceneToLoad)
    {
        // Try to use the Scene the current Activity is associated with
        if (g_ActivityMan.GetActivity())
            SetSceneToLoad(g_ActivityMan.GetActivity()->GetSceneName());

        // If that failed, then resort to the default scene name
        if (SetSceneToLoad(m_DefaultSceneName) < 0)
        {
            g_ConsoleMan.PrintString("ERROR: Couldn't start because no Scene has been specified to load!");
            return -1;
        }
    }

    return LoadScene(dynamic_cast<Scene *>(m_pSceneToLoad->Clone()), m_PlaceObjects, m_PlaceUnits);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int SceneMan::LoadScene(std::string sceneName, bool placeObjects, bool placeUnits)
{
    // First retrieve and set up the preset reference
    int error = SetSceneToLoad(sceneName, placeObjects, placeUnits);
    if (error < 0)
        return error;
    // Now actually load and start it
    error = LoadScene();
    return error;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int SceneMan::ReadProperty(const std::string_view &propName, Reader &reader)
{
    StartPropertyList(return Serializable::ReadProperty(propName, reader));

    MatchProperty("AddMaterial",
    {
        // Get this before reading Object, since if it's the last one in its datafile, the stream will show the parent file instead
        std::string objectFilePath = reader.GetCurrentFilePath();

        // Don't use the << operator, because it adds the material to the PresetMan before we get a chance to set the proper ID!
        Material *pNewMat = new Material;
        ((Serializable *)(pNewMat))->Create(reader);

        // If the initially requested material slot is available, then put it there
        // But if it's not available, then check if any subsequent one is, looping around the palette if necessary
        for (int tryId = pNewMat->GetIndex(); tryId < c_PaletteEntriesNumber; ++tryId)
        {
            // We found an empty slot in the Material palette!
            if (m_apMatPalette.at(tryId) == nullptr)
            {
                // If the final ID isn't the same as the one originally requested by the data file, then make the mapping so
                // subsequent ID references to this within the same data module can be translated to the actual ID of this material
                if (tryId != pNewMat->GetIndex())
                    g_PresetMan.AddMaterialMapping(pNewMat->GetIndex(), tryId, reader.GetReadModuleID());

                // Assign the final ID to the material and register it in the palette
                pNewMat->SetIndex(tryId);

                // Ensure out-of-bounds material is unbreakable
                if (tryId == MaterialColorKeys::g_MaterialOutOfBounds) {
                    RTEAssert(pNewMat->GetIntegrity() == std::numeric_limits<float>::max(), "Material with index " + std::to_string(MaterialColorKeys::g_MaterialOutOfBounds) + " (i.e out-of-bounds material) has a finite integrity!\n This should be infinity (-1).");
                }

                m_apMatPalette.at(tryId) = pNewMat;
                m_MatNameMap.insert(std::pair<std::string, unsigned char>(std::string(pNewMat->GetPresetName()), pNewMat->GetIndex()));
                // Now add the instance, when ID has been registered!
                g_PresetMan.AddEntityPreset(pNewMat, reader.GetReadModuleID(), reader.GetPresetOverwriting(), objectFilePath);
                ++m_MaterialCount;
                break;
            }
            // We reached the end of the Material palette without finding any empty slots.. loop around to the start
            else if (tryId >= c_PaletteEntriesNumber - 1)
                tryId = 0;
            // If we've looped around without finding anything, break and throw error
            else if (tryId == pNewMat->GetIndex() - 1)
            {
// TODO: find the closest matching mateiral and map to it?
                RTEAbort("Tried to load material \"" + pNewMat->GetPresetName() + "\" but the material palette (256 max) is full! Try consolidating or removing some redundant materials, or removing some entire data modules.");
                break;
            }
        }
    });

    EndPropertyList;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int SceneMan::Save(Writer &writer) const {
	g_ConsoleMan.PrintString("ERROR: Tried to save SceneMan, screen does not make sense");

	Serializable::Save(writer);

	for (int i = 0; i < m_MaterialCount; ++i) {
		writer.NewPropertyWithValue("AddMaterial", *(m_apMatPalette.at(i)));
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SceneMan::Destroy()
{
    for (int i = 0; i < c_PaletteEntriesNumber; ++i)
        delete m_apMatPalette[i];

    delete m_pCurrentScene;
    delete m_pDebugLayer;
    delete m_pUnseenRevealSound;

	destroy_bitmap(m_pOrphanSearchBitmap);
	m_pOrphanSearchBitmap = 0;

	for (const auto &[bitmapSize, bitmapPtr] : m_IntermediateSettlingBitmaps) {
		destroy_bitmap(bitmapPtr);
	}

    Clear();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Vector SceneMan::GetSceneDim() const
{
	if (m_pCurrentScene) {
		RTEAssert(m_pCurrentScene->GetTerrain() && m_pCurrentScene->GetTerrain()->GetBitmap(), "Trying to get terrain info before there is a scene or terrain!");
		return m_pCurrentScene->GetDimensions();
	}
    return Vector();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int SceneMan::GetSceneWidth() const
{
	if (g_NetworkClient.IsConnectedAndRegistered()) {
		return g_NetworkClient.GetSceneWidth();
	}

	if (m_pCurrentScene)
        return m_pCurrentScene->GetWidth();
    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int SceneMan::GetSceneHeight() const
{
//    RTEAssert(m_pCurrentScene, "Trying to get terrain info before there is a scene or terrain!");
    if (m_pCurrentScene)
        return m_pCurrentScene->GetHeight();
    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool SceneMan::SceneWrapsX() const
{
	if (g_NetworkClient.IsConnectedAndRegistered()) {
		return g_NetworkClient.SceneWrapsX();
	}

	if (m_pCurrentScene)
        return m_pCurrentScene->WrapsX();
    return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool SceneMan::SceneWrapsY() const
{
    if (m_pCurrentScene)
        return m_pCurrentScene->WrapsY();
    return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Directions SceneMan::GetSceneOrbitDirection() const {
    if (m_pCurrentScene) {
        SLTerrain *terrain = m_pCurrentScene->GetTerrain();
        if (terrain) {
            return terrain->GetOrbitDirection();
        }
    }

    return Directions::Up;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

SLTerrain * SceneMan::GetTerrain()
{
//    RTEAssert(m_pCurrentScene, "Trying to get terrain matter before there is a scene or terrain!");
    if (m_pCurrentScene) {
        return m_pCurrentScene->GetTerrain();
    }

    return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BITMAP *SceneMan::GetDebugBitmap() const {
    RTEAssert(m_pDebugLayer, "Tried to get debug bitmap but debug layer doesn't exist. Note that the debug layer is only created under certain circumstances.");
    return m_pDebugLayer->GetBitmap();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

unsigned char SceneMan::GetTerrMatter(int pixelX, int pixelY)
{
    RTEAssert(m_pCurrentScene, "Trying to get terrain matter before there is a scene or terrain!");

    WrapPosition(pixelX, pixelY);

    if (m_pDebugLayer && m_DrawPixelCheckVisualizations) { m_pDebugLayer->SetPixel(pixelX, pixelY, 5); }

    BITMAP *pTMatBitmap = m_pCurrentScene->GetTerrain()->GetMaterialBitmap();
	if (pTMatBitmap == nullptr) {
		return g_MaterialAir;
	}

    // If it's still below or to the sides out of bounds after
    // what is supposed to be wrapped, shit is out of bounds.
    if (pixelX < 0 || pixelX >= pTMatBitmap->w || pixelY >= pTMatBitmap->h)
        return g_MaterialAir;

    // If above terrain bitmap, return air material.
    if (pixelY < 0)
        return g_MaterialAir;

    return getpixel(pTMatBitmap, pixelX, pixelY);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

MOID SceneMan::GetMOIDPixel(int pixelX, int pixelY, int ignoreTeam)
{
    WrapPosition(pixelX, pixelY);

    if (m_pDebugLayer && m_DrawPixelCheckVisualizations) { 
        m_pDebugLayer->SetPixel(pixelX, pixelY, 5); 
    }

    const std::vector<MOID> &moidList = m_MOIDsGrid.GetMOIDsAtPosition(pixelX, pixelY, ignoreTeam, true);
    MOID moid = g_MovableMan.GetMOIDPixel(pixelX, pixelY, moidList);

	return moid;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Material const * SceneMan::GetMaterial(const std::string &matName)
{
    std::map<std::string, unsigned char>::iterator itr = m_MatNameMap.find(matName);
    if (itr == m_MatNameMap.end())
    {
        g_ConsoleMan.PrintString("ERROR: Material of name: " + matName + " not found!");
        return 0;
    }
    else
        return m_apMatPalette.at((*itr).second);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Vector SceneMan::GetGlobalAcc() const
{
    RTEAssert(m_pCurrentScene, "Trying to get terrain matter before there is a scene or terrain!");
    return m_pCurrentScene->GetGlobalAcc();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SceneMan::LockScene()
{
//    RTEAssert(!m_pCurrentScene->IsLocked(), "Hey, locking already locked scene!");
    if (m_pCurrentScene && !m_pCurrentScene->IsLocked())
    {
        m_pCurrentScene->Lock();
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SceneMan::UnlockScene()
{
//    RTEAssert(m_pCurrentScene->IsLocked(), "Hey, unlocking already unlocked scene!");
    if (m_pCurrentScene && m_pCurrentScene->IsLocked())
    {
        m_pCurrentScene->Unlock();
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool SceneMan::SceneIsLocked() const
{
    RTEAssert(m_pCurrentScene, "Trying to check if scene is locked before there is a scene or terrain!");
    return m_pCurrentScene->IsLocked();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SceneMan::RegisterMOIDDrawing(int moid, int left, int top, int right, int bottom) {
    if (const MovableObject *mo = g_MovableMan.GetMOFromID(moid)) {
        m_MOIDsGrid.Add(IntRect(left, top, right, bottom), *mo);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SceneMan::RegisterMOIDDrawing(int moid, const Vector &center, float radius) {
    if (radius != 0.0F) {
		RegisterMOIDDrawing(moid, center.m_X - radius, center.m_Y - radius, center.m_X + radius, center.m_Y + radius);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SceneMan::ClearAllMOIDDrawings() {
    m_MOIDsGrid.Reset();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool SceneMan::WillPenetrate(const int posX,
                             const int posY,
                             const Vector &impulse)
{
    RTEAssert(m_pCurrentScene, "Trying to access scene before there is one!");

    if (!m_pCurrentScene->GetTerrain()->IsWithinBounds(posX, posY))
        return false;

    unsigned char materialID = getpixel(m_pCurrentScene->GetTerrain()->GetMaterialBitmap(), posX, posY);
    float integrity = GetMaterialFromID(materialID)->GetIntegrity();
    return impulse.MagnitudeIsGreaterThan(integrity);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int SceneMan::RemoveOrphans(int posX, int posY, int radius, int maxArea, bool remove)
{
	if (radius > MAXORPHANRADIUS)
		radius = MAXORPHANRADIUS;

	clear_to_color(m_pOrphanSearchBitmap, g_MaterialAir);
	int area = RemoveOrphans(posX, posY, posX, posY, 0, radius, maxArea, false);
	if (remove && area <= maxArea)
	{
		clear_to_color(m_pOrphanSearchBitmap, g_MaterialAir);
		RemoveOrphans(posX, posY, posX, posY, 0, radius, maxArea, true);
	}

	return area;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int SceneMan::RemoveOrphans(int posX, int posY,
							int centerPosX, int centerPosY,
							int accumulatedArea, int radius, int maxArea, bool remove)
{
	int area = 0;
	int bmpX = 0;
	int bmpY = 0;

	BITMAP * mat = m_pCurrentScene->GetTerrain()->GetMaterialBitmap();

	if (posX < 0 || posY < 0 || posX >= mat->w || posY >= mat->h) return 0;

    unsigned char materialID = _getpixel(mat, posX, posY);
    if (materialID == g_MaterialAir && (posX != centerPosX || posY != centerPosY))
		return 0;
	else
	{
		bmpX = posX - (centerPosX - radius / 2);
		bmpY = posY - (centerPosY - radius / 2);

		// We reached the border of orphan-searching area and
		// there are still material pixels there -> the area is not an orphaned teran piece, abort search
		if (bmpX <= 0 || bmpY <= 0 || bmpX >= radius - 1 || bmpY >= radius - 1)
			return MAXORPHANRADIUS * MAXORPHANRADIUS + 1;
		else
		// Check if pixel was already checked
		{
			if (_getpixel(m_pOrphanSearchBitmap, bmpX, bmpY) != g_MaterialAir)
				return 0;
		}
	}

	_putpixel(m_pOrphanSearchBitmap, bmpX, bmpY, materialID);
	area++;

	// We're clear to remove the pixel
	if (remove)
	{
		Material const * sceneMat = GetMaterialFromID(materialID);
		Material const * spawnMat;
        spawnMat = sceneMat->GetSpawnMaterial() ? GetMaterialFromID(sceneMat->GetSpawnMaterial()) : sceneMat;
		float sprayScale = 0.1;
        Color spawnColor;
        if (spawnMat->UsesOwnColor())
            spawnColor = spawnMat->GetColor();
        else
            spawnColor.SetRGBWithIndex(m_pCurrentScene->GetTerrain()->GetFGColorPixel(posX, posY));

        // No point generating a key-colored MOPixel
        if (spawnColor.GetIndex() != g_MaskColor)
        {
			// TEST COLOR
			// spawnColor = 5;

            // Get the new pixel from the pre-allocated pool, should be faster than dynamic allocation
            // Density is used as the mass for the new MOPixel
			float tempMax = 2.0F * sprayScale;
			float tempMin = tempMax / 2.0F;
            MOPixel *pixelMO = new MOPixel(spawnColor,
                                           spawnMat->GetPixelDensity(),
                                           Vector(posX, posY),
                                           Vector(-RandomNum(tempMin, tempMax),
                                                  -RandomNum(tempMin, tempMax)),
                                           new Atom(Vector(), spawnMat->GetIndex(), 0, spawnColor, 2),
                                           0);

            pixelMO->SetToHitMOs(spawnMat->GetIndex() == c_GoldMaterialID);
            pixelMO->SetToGetHitByMOs(false);
            g_MovableMan.AddParticle(pixelMO);
            pixelMO = 0;
        }
        m_pCurrentScene->GetTerrain()->SetFGColorPixel(posX, posY, g_MaskColor);
		RegisterTerrainChange(posX, posY, 1, 1, g_MaskColor, false);
        m_pCurrentScene->GetTerrain()->SetMaterialPixel(posX, posY, g_MaterialAir);
	}

	int xoff[8] = { -1,  0,  1, -1,  1, -1,  0,  1};
	int yoff[8] = { -1, -1, -1,  0,  0,  1,  1,  1};

	for (int c = 0; c < 8; c++)
	{
		area += RemoveOrphans(posX + xoff[c], posY + yoff[c], centerPosX, centerPosY, area, radius, maxArea, remove);
		if (accumulatedArea + area > maxArea)
			break;
	}

	return area;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SceneMan::RegisterTerrainChange(int x, int y, int w, int h, unsigned char color, bool back)
{
	if (!g_NetworkServer.IsServerModeEnabled())
		return;

	// Crop if it's out of scene as both the client and server will not tolerate out of bitmap coords while packing/unpacking
	if (y < 0)
		y = 0;

	if (y + h >= GetSceneHeight())
		h = GetSceneHeight() - y - 1;

	if (y >= GetSceneHeight() || h <= 0)
		return;

	if (w == 1)
	{
		if (x >= GetSceneWidth())
		{
			if (!SceneWrapsX())
				return;
			x = x - GetSceneWidth();
		}
		if (x < 0)
		{
			if (!SceneWrapsX())
				return;
			x = GetSceneWidth() + x;
		}
	}
	else
	{
		// Divide region if crossing the seam
		if (x + w >= GetSceneWidth() || x < 0)
		{
			// Crossing right part of the scene
			if (x + w >= GetSceneWidth())
			{
				// Left part, on the scene
				NetworkServer::NetworkTerrainChange tc1;
				tc1.x = x;
				tc1.y = y;
				tc1.w = GetSceneWidth() - x;
				tc1.h = h;
				tc1.back = back;
				tc1.color = color;
				g_NetworkServer.RegisterTerrainChange(tc1);

				// Discard out of scene part if scene is not wrapped
				if (!SceneWrapsX())
					return;

				// Right part, out of scene
				NetworkServer::NetworkTerrainChange tc2;
				tc2.x = 0;
				tc2.y = y;
				tc2.w = w - (GetSceneWidth() - x);
				tc2.h = h;
				tc2.back = back;
				tc2.color = color;

				g_NetworkServer.RegisterTerrainChange(tc2);
				return;
			}

			if (x < 0)
			{
				// Right part, on the scene
				NetworkServer::NetworkTerrainChange tc2;
				tc2.x = 0;
				tc2.y = y;
				tc2.w = w + x;
				tc2.h = h;
				tc2.back = back;
				tc2.color = color;
				g_NetworkServer.RegisterTerrainChange(tc2);

				// Discard out of scene part if scene is not wrapped
				if (!SceneWrapsX())
					return;

				// Left part, out of the scene
				NetworkServer::NetworkTerrainChange tc1;
				tc1.x = GetSceneWidth() + x;
				tc1.y = y;
				tc1.w = -x;
				tc1.h = h;
				tc1.back = back;
				tc1.color = color;
				g_NetworkServer.RegisterTerrainChange(tc1);
				return;
			}
		}
	}

	NetworkServer::NetworkTerrainChange tc;
	tc.x = x;
	tc.y = y;
	tc.w = w;
	tc.h = h;
	tc.back = back;
	tc.color = color;
	g_NetworkServer.RegisterTerrainChange(tc);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool SceneMan::TryPenetrate(int posX,
                            int posY,
                            const Vector &impulse,
                            const Vector &velocity,
                            float &retardation,
                            const float airRatio,
                            const int numPenetrations,
						    const int removeOrphansRadius,
					        const int removeOrphansMaxArea,
					        const float removeOrphansRate)
{
    RTEAssert(m_pCurrentScene, "Trying to access scene before there is one!");

    if (!m_pCurrentScene->GetTerrain()->IsWithinBounds(posX, posY))
        return false;

    WrapPosition(posX, posY);
    unsigned char materialID = _getpixel(m_pCurrentScene->GetTerrain()->GetMaterialBitmap(), posX, posY);
    if (materialID == g_MaterialAir)
    {
//        RTEAbort("Why are we penetrating air??");
        return true;
    }
    Material const * sceneMat = GetMaterialFromID(materialID);
    Material const * spawnMat;

	float sprayScale = 0.1F;
	float sqrImpMag = impulse.GetSqrMagnitude();

    // Test if impulse force is enough to penetrate
    if (sqrImpMag >= (sceneMat->GetIntegrity() * sceneMat->GetIntegrity()))
    {
        if (numPenetrations <= 3)
        {
            spawnMat = sceneMat->GetSpawnMaterial() ? GetMaterialFromID(sceneMat->GetSpawnMaterial()) : sceneMat;
            Color spawnColor;
            if (spawnMat->UsesOwnColor())
                spawnColor = spawnMat->GetColor();
            else
                spawnColor.SetRGBWithIndex(m_pCurrentScene->GetTerrain()->GetFGColorPixel(posX, posY));

            // No point generating a key-colored MOPixel
            if (spawnColor.GetIndex() != g_MaskColor)
            {
                // Get the new pixel from the pre-allocated pool, should be faster than dynamic allocation
                // Density is used as the mass for the new MOPixel
/*                MOPixel *pixelMO = dynamic_cast<MOPixel *>(MOPixel::InstanceFromPool());
                pixelMO->Create(spawnColor,
                                spawnMat.pixelDensity,
                                Vector(posX, posY),
                                Vector(-RandomNum((velocity.m_X * sprayScale) / 2 , velocity.m_X * sprayScale),
                                       -RandomNum((velocity.m_Y * sprayScale) / 2 , velocity.m_Y * sprayScale)),
//                                               -(impulse * (sprayScale * RandomNum() / spawnMat.density)),
                                new Atom(Vector(), spawnMat, 0, spawnColor, 2),
                                0);
*/
				float tempMaxX = velocity.m_X * sprayScale;
				float tempMinX = tempMaxX / 2.0F;
				float tempMaxY = velocity.m_Y * sprayScale;
				float tempMinY = tempMaxY / 2.0F;
                MOPixel *pixelMO = new MOPixel(spawnColor,
                                               spawnMat->GetPixelDensity(),
                                               Vector(posX, posY),
                                               Vector(-RandomNum(tempMinX, tempMaxX),
                                                      -RandomNum(tempMinY, tempMaxY)),
//                                              -(impulse * (sprayScale * RandomNum() / spawnMat.density)),
                                               new Atom(Vector(), spawnMat->GetIndex(), 0, spawnColor, 2),
                                               0);

// TODO: Make material IDs more robust!")
                pixelMO->SetToHitMOs(spawnMat->GetIndex() == c_GoldMaterialID);
                pixelMO->SetToGetHitByMOs(false);
                g_MovableMan.AddParticle(pixelMO);
                pixelMO = 0;
            }
            m_pCurrentScene->GetTerrain()->SetFGColorPixel(posX, posY, g_MaskColor);
			RegisterTerrainChange(posX, posY, 1, 1, g_MaskColor, false);

            m_pCurrentScene->GetTerrain()->SetMaterialPixel(posX, posY, g_MaterialAir);
        }
// TODO: Improve / tweak randomized pushing away of terrain")
        else if (RandomNum() <= airRatio)
        {
            m_pCurrentScene->GetTerrain()->SetFGColorPixel(posX, posY, g_MaskColor);
			RegisterTerrainChange(posX, posY, 1, 1, g_MaskColor, false);

			m_pCurrentScene->GetTerrain()->SetMaterialPixel(posX, posY, g_MaterialAir);
        }

        // Save the impulse force effects of the penetrating particle.
//        retardation = -sceneMat.density;
        retardation = -(sceneMat->GetIntegrity() / std::sqrt(sqrImpMag));

		// If this is a scrap pixel, or there is no background pixel 'supporting' the knocked-loose pixel, make the column above also turn into particles.
		if (m_ScrapCompactingHeight > 0 && (sceneMat->IsScrap() || _getpixel(m_pCurrentScene->GetTerrain()->GetBGColorBitmap(), posX, posY) == g_MaskColor)) {
            // Get quicker direct access to bitmaps
            BITMAP *pFGColor = m_pCurrentScene->GetTerrain()->GetFGColorBitmap();
            BITMAP *pBGColor = m_pCurrentScene->GetTerrain()->GetBGColorBitmap();
            BITMAP *pMaterial = m_pCurrentScene->GetTerrain()->GetMaterialBitmap();

            int testMaterialID = g_MaterialAir;
            MOPixel *pixelMO = 0;
            Color spawnColor;
			float sprayMag = std::sqrt(velocity.GetMagnitude() * sprayScale);
			Vector sprayVel;

			for (int testY = posY - 1; testY > posY - m_ScrapCompactingHeight && testY >= 0; --testY) {
				if ((testMaterialID = _getpixel(pMaterial, posX, testY)) != g_MaterialAir) {
					sceneMat = GetMaterialFromID(testMaterialID);

					if (sceneMat->IsScrap() || _getpixel(pBGColor, posX, testY) == g_MaskColor) {
						if (RandomNum() < 0.7F) {
							spawnMat = sceneMat->GetSpawnMaterial() ? GetMaterialFromID(sceneMat->GetSpawnMaterial()) : sceneMat;
							if (spawnMat->UsesOwnColor()) {
								spawnColor = spawnMat->GetColor();
							} else {
								spawnColor.SetRGBWithIndex(m_pCurrentScene->GetTerrain()->GetFGColorPixel(posX, testY));
							}
							if (spawnColor.GetIndex() != g_MaskColor) {
								// Send terrain pixels flying at a diminishing rate the higher the column goes.
								sprayVel.SetXY(0, -sprayMag * (1.0F - (static_cast<float>(posY - testY) / static_cast<float>(m_ScrapCompactingHeight))));
								sprayVel.RadRotate(RandomNum(-c_HalfPI, c_HalfPI));

								pixelMO = new MOPixel(spawnColor, spawnMat->GetPixelDensity(), Vector(posX, testY), sprayVel, new Atom(Vector(), spawnMat->GetIndex(), 0, spawnColor, 2), 0);

								pixelMO->SetToHitMOs(spawnMat->GetIndex() == c_GoldMaterialID);
								pixelMO->SetToGetHitByMOs(false);
								g_MovableMan.AddParticle(pixelMO);
								pixelMO = 0;
							}
							RemoveOrphans(posX + testY % 2 ? -1 : 1, testY, removeOrphansRadius + 5, removeOrphansMaxArea + 10, true);
						}

						RegisterTerrainChange(posX, testY, 1, 1, g_MaskColor, false);
						_putpixel(pFGColor, posX, testY, g_MaskColor);
						_putpixel(pMaterial, posX, testY, g_MaterialAir);
					} else {
						break;
					}
                }
            }
        }

		// Remove orphaned regions if told to by parent MO who travelled an atom which tries to penetrate terrain
		if (removeOrphansRadius && removeOrphansMaxArea && removeOrphansRate > 0 && RandomNum() < removeOrphansRate)
		{
			RemoveOrphans(posX, posY, removeOrphansRadius, removeOrphansMaxArea, true);
			/*PALETTE palette;
			get_palette(palette);
			save_bmp("Orphan.bmp", m_pOrphanSearchBitmap, palette);*/
		}

        return true;
    }
    return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

MovableObject * SceneMan::DislodgePixel(int posX, int posY) {
	int materialID = getpixel(m_pCurrentScene->GetTerrain()->GetMaterialBitmap(), posX, posY);
	if (materialID <= MaterialColorKeys::g_MaterialAir) {
		return nullptr;
	}
	const Material *sceneMat = GetMaterialFromID(static_cast<uint8_t>(materialID));
	const Material *spawnMat = sceneMat->GetSpawnMaterial() ? GetMaterialFromID(sceneMat->GetSpawnMaterial()) : sceneMat;

	Color spawnColor;
	if (spawnMat->UsesOwnColor()) {
		spawnColor = spawnMat->GetColor();
	} else {
		spawnColor.SetRGBWithIndex(m_pCurrentScene->GetTerrain()->GetFGColorPixel(posX, posY));
	}
	// No point generating a key-colored MOPixel.
	if (spawnColor.GetIndex() == ColorKeys::g_MaskColor) {
		return nullptr;
	}
	Atom *pixelAtom = new Atom(Vector(), spawnMat->GetIndex(), nullptr, spawnColor, 2);
	MOPixel *pixelMO = new MOPixel(spawnColor, spawnMat->GetPixelDensity(), Vector(static_cast<float>(posX), static_cast<float>(posY)), Vector(), pixelAtom, 0);
	pixelMO->SetToHitMOs(spawnMat->GetIndex() == c_GoldMaterialID);
	g_MovableMan.AddParticle(pixelMO);

	m_pCurrentScene->GetTerrain()->SetFGColorPixel(posX, posY, ColorKeys::g_MaskColor);
	RegisterTerrainChange(posX, posY, 1, 1, ColorKeys::g_MaskColor, false);
	m_pCurrentScene->GetTerrain()->SetMaterialPixel(posX, posY, MaterialColorKeys::g_MaterialAir);

	return pixelMO;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SceneMan::MakeAllUnseen(Vector pixelSize, const int team)
{
    RTEAssert(m_pCurrentScene, "Messing with scene before the scene exists!");
	if (team < Activity::TeamOne || team >= Activity::MaxTeamCount)
		return;

    m_pCurrentScene->FillUnseenLayer(pixelSize, team);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool SceneMan::LoadUnseenLayer(std::string bitmapPath, int team)
{
    ContentFile bitmapFile(bitmapPath.c_str());
    SceneLayer *pUnseenLayer = new SceneLayer();
    if (pUnseenLayer->Create(bitmapFile.GetAsBitmap(COLORCONV_NONE, false), true, Vector(), m_pCurrentScene->WrapsX(), m_pCurrentScene->WrapsY(), Vector(1.0, 1.0)) < 0)
    {
        g_ConsoleMan.PrintString("ERROR: Loading background layer " + pUnseenLayer->GetPresetName() + "\'s data failed!");
        return false;
    }

    // Pass in ownership here
    m_pCurrentScene->SetUnseenLayer(pUnseenLayer, team);
    return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool SceneMan::AnythingUnseen(const int team)
{
    RTEAssert(m_pCurrentScene, "Checking scene before the scene exists when checking if anything is unseen!");

    return m_pCurrentScene->GetUnseenLayer(team) != 0;
// TODO: Actually check all pixels on the map too?
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Vector SceneMan::GetUnseenResolution(const int team) const
{
    RTEAssert(m_pCurrentScene, "Checking scene before the scene exists when getting unseen resolution!");
	if (team < Activity::TeamOne || team >= Activity::MaxTeamCount)
		return Vector(1, 1);

    SceneLayer *pUnseenLayer = m_pCurrentScene->GetUnseenLayer(team);
    if (pUnseenLayer)
        return pUnseenLayer->GetScaleFactor();

    return Vector(1, 1);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool SceneMan::IsUnseen(const int posX, const int posY, const int team)
{
    RTEAssert(m_pCurrentScene, "Checking scene before the scene exists when checking if a position is unseen!");
	if (team < Activity::TeamOne || team >= Activity::MaxTeamCount)
		return false;

    SceneLayer *pUnseenLayer = m_pCurrentScene->GetUnseenLayer(team);
    if (pUnseenLayer)
    {
        // Translate to the scaled unseen layer's coordinates
        Vector scale = pUnseenLayer->GetScaleFactor();
        int scaledX = posX / scale.m_X;
        int scaledY = posY / scale.m_Y;
        return getpixel(pUnseenLayer->GetBitmap(), scaledX, scaledY) != g_MaskColor;
    }

    return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool SceneMan::RevealUnseen(const int posX, const int posY, const int team)
{
    RTEAssert(m_pCurrentScene, "Checking scene before the scene exists when revealing an unseen position!");
	if (team < Activity::TeamOne || team >= Activity::MaxTeamCount)
		return false;

    SceneLayer *pUnseenLayer = m_pCurrentScene->GetUnseenLayer(team);
    if (pUnseenLayer)
    {
        // Translate to the scaled unseen layer's coordinates
        Vector scale = pUnseenLayer->GetScaleFactor();
        int scaledX = posX / scale.m_X;
        int scaledY = posY / scale.m_Y;

        // Make sure we're actually revealing an unseen pixel that is ON the bitmap!
        int pixel = getpixel(pUnseenLayer->GetBitmap(), scaledX, scaledY);
        if (pixel != g_MaskColor && pixel != -1)
        {
            // Add the pixel to the list of now seen pixels so it can be visually flashed
            m_pCurrentScene->GetSeenPixels(team).push_back(Vector(scaledX, scaledY));
            // Clear to key color that pixel on the map so it won't be detected as unseen again
            putpixel(pUnseenLayer->GetBitmap(), scaledX, scaledY, g_MaskColor);
            // Play the reveal sound, if there's not too many already revealed this frame
            if (g_SettingsMan.BlipOnRevealUnseen() && m_pUnseenRevealSound && m_pCurrentScene->GetSeenPixels(team).size() < 5)
                m_pUnseenRevealSound->Play(Vector(posX, posY));
            // Show that we actually cleared an unseen pixel
            return true;
        }
    }

    return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool SceneMan::RestoreUnseen(const int posX, const int posY, const int team)
{
    RTEAssert(m_pCurrentScene, "Checking scene before the scene exists when making a position unseen!");
	if (team < Activity::TeamOne || team >= Activity::MaxTeamCount)
		return false;

    SceneLayer *pUnseenLayer = m_pCurrentScene->GetUnseenLayer(team);
    if (pUnseenLayer)
    {
        // Translate to the scaled unseen layer's coordinates
        Vector scale = pUnseenLayer->GetScaleFactor();
        int scaledX = posX / scale.m_X;
        int scaledY = posY / scale.m_Y;

        // Make sure we're actually revealing an unseen pixel that is ON the bitmap!
        int pixel = getpixel(pUnseenLayer->GetBitmap(), scaledX, scaledY);
        if (pixel != g_BlackColor && pixel != -1)
        {
            // Add the pixel to the list of now seen pixels so it can be visually flashed
            m_pCurrentScene->GetSeenPixels(team).push_back(Vector(scaledX, scaledY));
            // Clear to key color that pixel on the map so it won't be detected as unseen again
            putpixel(pUnseenLayer->GetBitmap(), scaledX, scaledY, g_BlackColor);
            // Play the reveal sound, if there's not too many already revealed this frame
            //if (g_SettingsMan.BlipOnRevealUnseen() && m_pUnseenRevealSound && m_pCurrentScene->GetSeenPixels(team).size() < 5)
            //    m_pUnseenRevealSound->Play(g_SceneMan.TargetDistanceScalar(Vector(posX, posY)));
            // Show that we actually cleared an unseen pixel
            return true;
        }
    }

    return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SceneMan::RevealUnseenBox(const int posX, const int posY, const int width, const int height, const int team)
{
    RTEAssert(m_pCurrentScene, "Checking scene before the scene exists when revealing an unseen area!");
	if (team < Activity::TeamOne || team >= Activity::MaxTeamCount)
		return;

    SceneLayer *pUnseenLayer = m_pCurrentScene->GetUnseenLayer(team);
    if (pUnseenLayer)
    {
        // Translate to the scaled unseen layer's coordinates
        Vector scale = pUnseenLayer->GetScaleFactor();
        int scaledX = posX / scale.m_X;
        int scaledY = posY / scale.m_Y;
        int scaledW = width / scale.m_X;
        int scaledH = height / scale.m_Y;

        // Fill the box
        rectfill(pUnseenLayer->GetBitmap(), scaledX, scaledY, scaledX + scaledW, scaledY + scaledH, g_MaskColor);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SceneMan::RestoreUnseenBox(const int posX, const int posY, const int width, const int height, const int team)
{
    RTEAssert(m_pCurrentScene, "Checking scene before the scene exists when making an area unseen!");
	if (team < Activity::TeamOne || team >= Activity::MaxTeamCount)
		return;

    SceneLayer *pUnseenLayer = m_pCurrentScene->GetUnseenLayer(team);
    if (pUnseenLayer)
    {
        // Translate to the scaled unseen layer's coordinates
        Vector scale = pUnseenLayer->GetScaleFactor();
        int scaledX = posX / scale.m_X;
        int scaledY = posY / scale.m_Y;
        int scaledW = width / scale.m_X;
        int scaledH = height / scale.m_Y;

        // Fill the box
        rectfill(pUnseenLayer->GetBitmap(), scaledX, scaledY, scaledX + scaledW, scaledY + scaledH, g_BlackColor);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//TODO Every raycast should use some shared line drawing method (or maybe something more efficient if it exists, that needs looking into) instead of having a ton of duplicated code.
bool SceneMan::CastUnseenRay(int team, const Vector &start, const Vector &ray, Vector &endPos, int strengthLimit, int skip, bool reveal)
{
    if (!m_pCurrentScene->GetUnseenLayer(team))
        return false;

    int hitCount = 0, error, dom, sub, domSteps, skipped = skip;
    int intPos[2], delta[2], delta2[2], increment[2];
    bool affectedAny = false;
    unsigned char materialID;
    Material const * foundMaterial;
    int totalStrength = 0;
    // Save the projected end of the ray pos
    endPos = start + ray;

    intPos[X] = std::floor(start.m_X);
    intPos[Y] = std::floor(start.m_Y);
    delta[X] = std::floor(start.m_X + ray.m_X) - intPos[X];
    delta[Y] = std::floor(start.m_Y + ray.m_Y) - intPos[Y];

    if (delta[X] == 0 &&  delta[Y] == 0)
        return false;

    /////////////////////////////////////////////////////
    // Bresenham's line drawing algorithm preparation

    if (delta[X] < 0)
    {
        increment[X] = -1;
        delta[X] = -delta[X];
    }
    else
        increment[X] = 1;

    if (delta[Y] < 0)
    {
        increment[Y] = -1;
        delta[Y] = -delta[Y];
    }
    else
        increment[Y] = 1;

    // Scale by 2, for better accuracy of the error at the first pixel
    delta2[X] = delta[X] << 1;
    delta2[Y] = delta[Y] << 1;

    // If X is dominant, Y is submissive, and vice versa.
    if (delta[X] > delta[Y]) {
        dom = X;
        sub = Y;
    }
    else {
        dom = Y;
        sub = X;
    }

    error = delta2[sub] - delta[dom];

    /////////////////////////////////////////////////////
    // Bresenham's line drawing algorithm execution

    for (domSteps = 0; domSteps < delta[dom]; ++domSteps)
    {
        intPos[dom] += increment[dom];
        if (error >= 0)
        {
            intPos[sub] += increment[sub];
            error -= delta2[dom];
        }
        error += delta2[sub];

        // Only check pixel if we're not due to skip any, or if this is the last pixel
        if (++skipped > skip || domSteps + 1 == delta[dom])
        {
            // Scene wrapping
            g_SceneMan.WrapPosition(intPos[X], intPos[Y]);
            // Reveal if we can, save the result
			if (reveal)
				affectedAny = RevealUnseen(intPos[X], intPos[Y], team) || affectedAny;
			else
				affectedAny = RestoreUnseen(intPos[X], intPos[Y], team) || affectedAny;

            // Check the strength of the terrain to see if we can penetrate further
            materialID = GetTerrMatter(intPos[X], intPos[Y]);
            // Get the material object
            foundMaterial = GetMaterialFromID(materialID);
            // Add the encountered material's strength to the tally
            totalStrength += foundMaterial->GetIntegrity();
            // See if we have hit the limits of our ray's strength
            if (totalStrength >= strengthLimit)
            {
                // Save the position of the end of the ray where blocked
                endPos.SetXY(intPos[X], intPos[Y]);
                break;
            }
            // Reset skip counter
            skipped = 0;
            if (m_pDebugLayer && m_DrawRayCastVisualizations) { m_pDebugLayer->SetPixel(intPos[X], intPos[Y], 13); }
        }
    }

    return affectedAny;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool SceneMan::CastSeeRay(int team, const Vector &start, const Vector &ray, Vector &endPos, int strengthLimit, int skip)
{
	return CastUnseenRay(team, start, ray, endPos, strengthLimit, skip, true);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool SceneMan::CastUnseeRay(int team, const Vector &start, const Vector &ray, Vector &endPos, int strengthLimit, int skip)
{
	return CastUnseenRay(team, start, ray, endPos, strengthLimit, skip, false);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool SceneMan::CastMaterialRay(const Vector &start, const Vector &ray, unsigned char material, Vector &result, int skip, bool wrap)
{

    int hitCount = 0, error, dom, sub, domSteps, skipped = skip;
    int intPos[2], delta[2], delta2[2], increment[2];
    bool foundPixel = false;

    intPos[X] = std::floor(start.m_X);
    intPos[Y] = std::floor(start.m_Y);
    delta[X] = std::floor(start.m_X + ray.m_X) - intPos[X];
    delta[Y] = std::floor(start.m_Y + ray.m_Y) - intPos[Y];

    if (delta[X] == 0 &&  delta[Y] == 0)
        return false;

    /////////////////////////////////////////////////////
    // Bresenham's line drawing algorithm preparation

    if (delta[X] < 0)
    {
        increment[X] = -1;
        delta[X] = -delta[X];
    }
    else
        increment[X] = 1;

    if (delta[Y] < 0)
    {
        increment[Y] = -1;
        delta[Y] = -delta[Y];
    }
    else
        increment[Y] = 1;

    // Scale by 2, for better accuracy of the error at the first pixel
    delta2[X] = delta[X] << 1;
    delta2[Y] = delta[Y] << 1;

    // If X is dominant, Y is submissive, and vice versa.
    if (delta[X] > delta[Y]) {
        dom = X;
        sub = Y;
    }
    else {
        dom = Y;
        sub = X;
    }

    error = delta2[sub] - delta[dom];

    /////////////////////////////////////////////////////
    // Bresenham's line drawing algorithm execution

    for (domSteps = 0; domSteps < delta[dom]; ++domSteps)
    {
        intPos[dom] += increment[dom];
        if (error >= 0)
        {
            intPos[sub] += increment[sub];
            error -= delta2[dom];
        }
        error += delta2[sub];

        // Only check pixel if we're not due to skip any, or if this is the last pixel
        if (++skipped > skip || domSteps + 1 == delta[dom])
        {
            // Scene wrapping, if necessary
            if (wrap)
                g_SceneMan.WrapPosition(intPos[X], intPos[Y]);

            // See if we found the looked-for pixel of the correct material
            if (GetTerrMatter(intPos[X], intPos[Y]) == material)
            {
                // Save result and report success
                foundPixel = true;
                result.SetXY(intPos[X], intPos[Y]);
                // Save last ray pos
                s_LastRayHitPos.SetXY(intPos[X], intPos[Y]);
                break;
            }

            skipped = 0;

            if (m_pDebugLayer && m_DrawRayCastVisualizations) { m_pDebugLayer->SetPixel(intPos[X], intPos[Y], 13); }
        }
    }

    return foundPixel;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

float SceneMan::CastMaterialRay(const Vector &start, const Vector &ray, unsigned char material, int skip)
{
    Vector result;
    if (CastMaterialRay(start, ray, material, result, skip))
    {
        // Calculate the length between the start and the found material pixel coords
        result -= start;
        return result.GetMagnitude();
    }

    // Signal that we didn't hit anything
    return -1;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool SceneMan::CastNotMaterialRay(const Vector &start, const Vector &ray, unsigned char material, Vector &result, int skip, bool checkMOs)
{
    int hitCount = 0, error, dom, sub, domSteps, skipped = skip;
    int intPos[2], delta[2], delta2[2], increment[2];
    bool foundPixel = false;

    intPos[X] = std::floor(start.m_X);
    intPos[Y] = std::floor(start.m_Y);
    delta[X] = std::floor(start.m_X + ray.m_X) - intPos[X];
    delta[Y] = std::floor(start.m_Y + ray.m_Y) - intPos[Y];

    if (delta[X] == 0 &&  delta[Y] == 0)
        return false;

    /////////////////////////////////////////////////////
    // Bresenham's line drawing algorithm preparation

    if (delta[X] < 0)
    {
        increment[X] = -1;
        delta[X] = -delta[X];
    }
    else
        increment[X] = 1;

    if (delta[Y] < 0)
    {
        increment[Y] = -1;
        delta[Y] = -delta[Y];
    }
    else
        increment[Y] = 1;

    // Scale by 2, for better accuracy of the error at the first pixel
    delta2[X] = delta[X] << 1;
    delta2[Y] = delta[Y] << 1;

    // If X is dominant, Y is submissive, and vice versa.
    if (delta[X] > delta[Y]) {
        dom = X;
        sub = Y;
    }
    else {
        dom = Y;
        sub = X;
    }

    error = delta2[sub] - delta[dom];

    /////////////////////////////////////////////////////
    // Bresenham's line drawing algorithm execution

    for (domSteps = 0; domSteps < delta[dom]; ++domSteps)
    {
        intPos[dom] += increment[dom];
        if (error >= 0)
        {
            intPos[sub] += increment[sub];
            error -= delta2[dom];
        }
        error += delta2[sub];

        // Only check pixel if we're not due to skip any, or if this is the last pixel
        if (++skipped > skip || domSteps + 1 == delta[dom])
        {
            // Scene wrapping, if necessary
            g_SceneMan.WrapPosition(intPos[X], intPos[Y]);

            // See if we found the looked-for pixel of the correct material,
            // Or an MO is blocking the way
            if (GetTerrMatter(intPos[X], intPos[Y]) != material ||
                (checkMOs && g_SceneMan.GetMOIDPixel(intPos[X], intPos[Y], Activity::NoTeam) != g_NoMOID))
            {
                // Save result and report success
                foundPixel = true;
                result.SetXY(intPos[X], intPos[Y]);
                // Save last ray pos
                s_LastRayHitPos.SetXY(intPos[X], intPos[Y]);
                break;
            }

            skipped = 0;
            if (m_pDebugLayer && m_DrawRayCastVisualizations) { m_pDebugLayer->SetPixel(intPos[X], intPos[Y], 13); }
        }
    }

    return foundPixel;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

float SceneMan::CastNotMaterialRay(const Vector &start, const Vector &ray, unsigned char material, int skip, bool checkMOs)
{
    Vector result;
    if (CastNotMaterialRay(start, ray, material, result, skip, checkMOs))
    {
        // Calculate the length between the start and the found material pixel coords
        result -= start;
        return result.GetMagnitude();
    }

    // Signal that we didn't hit anything
    return -1;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

float SceneMan::CastStrengthSumRay(const Vector &start, const Vector &end, int skip, unsigned char ignoreMaterial)
{
    Vector ray = g_SceneMan.ShortestDistance(start, end);
    float strengthSum = 0;

    int error, dom, sub, domSteps, skipped = skip;
    int intPos[2], delta[2], delta2[2], increment[2];
    bool foundPixel = false;
    unsigned char materialID;
    Material foundMaterial;

    intPos[X] = std::floor(start.m_X);
    intPos[Y] = std::floor(start.m_Y);
    delta[X] = std::floor(start.m_X + ray.m_X) - intPos[X];
    delta[Y] = std::floor(start.m_Y + ray.m_Y) - intPos[Y];

    if (delta[X] == 0 &&  delta[Y] == 0)
        return false;

    /////////////////////////////////////////////////////
    // Bresenham's line drawing algorithm preparation

    if (delta[X] < 0)
    {
        increment[X] = -1;
        delta[X] = -delta[X];
    }
    else
        increment[X] = 1;

    if (delta[Y] < 0)
    {
        increment[Y] = -1;
        delta[Y] = -delta[Y];
    }
    else
        increment[Y] = 1;

    // Scale by 2, for better accuracy of the error at the first pixel
    delta2[X] = delta[X] << 1;
    delta2[Y] = delta[Y] << 1;

    // If X is dominant, Y is submissive, and vice versa.
    if (delta[X] > delta[Y]) {
        dom = X;
        sub = Y;
    }
    else
    {
        dom = Y;
        sub = X;
    }

    error = delta2[sub] - delta[dom];

    /////////////////////////////////////////////////////
    // Bresenham's line drawing algorithm execution

    for (domSteps = 0; domSteps < delta[dom]; ++domSteps)
    {
        intPos[dom] += increment[dom];
        if (error >= 0)
        {
            intPos[sub] += increment[sub];
            error -= delta2[dom];
        }
        error += delta2[sub];

        // Only check pixel if we're not due to skip any, or if this is the last pixel
        if (++skipped > skip || domSteps + 1 == delta[dom])
        {
            // Scene wrapping, if necessary
            g_SceneMan.WrapPosition(intPos[X], intPos[Y]);

            // Sum all strengths
            materialID = GetTerrMatter(intPos[X], intPos[Y]);
            if (materialID != g_MaterialAir && materialID != ignoreMaterial) {
                strengthSum += GetMaterialFromID(materialID)->GetIntegrity();
            }

            skipped = 0;

            if (m_pDebugLayer && m_DrawRayCastVisualizations) { m_pDebugLayer->SetPixel(intPos[X], intPos[Y], 13); }
        }
    }

    return strengthSum;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

float SceneMan::CastMaxStrengthRay(const Vector &start, const Vector &end, int skip, unsigned char ignoreMaterial) {
    return CastMaxStrengthRayMaterial(start, end, skip, ignoreMaterial)->GetIntegrity();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const Material * SceneMan::CastMaxStrengthRayMaterial(const Vector &start, const Vector &end, int skip, unsigned char ignoreMaterial) {
    Vector ray = g_SceneMan.ShortestDistance(start, end);
    const Material *strongestMaterial = GetMaterialFromID(MaterialColorKeys::g_MaterialAir);

    int error, dom, sub, domSteps, skipped = skip;
    int intPos[2], delta[2], delta2[2], increment[2];
    bool foundPixel = false;

    intPos[X] = std::floor(start.m_X);
    intPos[Y] = std::floor(start.m_Y);
    delta[X] = std::floor(start.m_X + ray.m_X) - intPos[X];
    delta[Y] = std::floor(start.m_Y + ray.m_Y) - intPos[Y];

	if (delta[X] == 0 && delta[Y] == 0) {
		return strongestMaterial;
	}

    /////////////////////////////////////////////////////
    // Bresenham's line drawing algorithm preparation

    if (delta[X] < 0)
    {
        increment[X] = -1;
        delta[X] = -delta[X];
    }
    else
        increment[X] = 1;

    if (delta[Y] < 0)
    {
        increment[Y] = -1;
        delta[Y] = -delta[Y];
    }
    else
        increment[Y] = 1;

    // Scale by 2, for better accuracy of the error at the first pixel
    delta2[X] = delta[X] << 1;
    delta2[Y] = delta[Y] << 1;

    // If X is dominant, Y is submissive, and vice versa.
    if (delta[X] > delta[Y]) {
        dom = X;
        sub = Y;
    }
    else
    {
        dom = Y;
        sub = X;
    }

    error = delta2[sub] - delta[dom];

    /////////////////////////////////////////////////////
    // Bresenham's line drawing algorithm execution

    for (domSteps = 0; domSteps < delta[dom]; ++domSteps)
    {
        intPos[dom] += increment[dom];
        if (error >= 0)
        {
            intPos[sub] += increment[sub];
            error -= delta2[dom];
        }
        error += delta2[sub];

        // Only check pixel if we're not due to skip any, or if this is the last pixel
        if (++skipped > skip || domSteps + 1 == delta[dom])
        {
            // Scene wrapping, if necessary
            g_SceneMan.WrapPosition(intPos[X], intPos[Y]);

            // Sum all strengths
            unsigned char materialID = GetTerrMatter(intPos[X], intPos[Y]);
            if (materialID != g_MaterialAir && materialID != ignoreMaterial) {
                const Material *foundMaterial = GetMaterialFromID(materialID);
                if (foundMaterial->GetIntegrity() > strongestMaterial->GetIntegrity()) {
                    strongestMaterial = foundMaterial;
                }
            }

            skipped = 0;

            if (m_pDebugLayer && m_DrawRayCastVisualizations) { m_pDebugLayer->SetPixel(intPos[X], intPos[Y], 13); }
        }
    }

    return strongestMaterial;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool SceneMan::CastStrengthRay(const Vector &start, const Vector &ray, float strength, Vector &result, int skip, unsigned char ignoreMaterial, bool wrap)
{
    int hitCount = 0, error, dom, sub, domSteps, skipped = skip;
    int intPos[2], delta[2], delta2[2], increment[2];
    bool foundPixel = false;
    unsigned char materialID;
    Material const * foundMaterial;

    intPos[X] = std::floor(start.m_X);
    intPos[Y] = std::floor(start.m_Y);
    delta[X] = std::floor(start.m_X + ray.m_X) - intPos[X];
    delta[Y] = std::floor(start.m_Y + ray.m_Y) - intPos[Y];

    if (delta[X] == 0 &&  delta[Y] == 0)
        return false;

    /////////////////////////////////////////////////////
    // Bresenham's line drawing algorithm preparation

    if (delta[X] < 0)
    {
        increment[X] = -1;
        delta[X] = -delta[X];
    }
    else
        increment[X] = 1;

    if (delta[Y] < 0)
    {
        increment[Y] = -1;
        delta[Y] = -delta[Y];
    }
    else
        increment[Y] = 1;

    // Scale by 2, for better accuracy of the error at the first pixel
    delta2[X] = delta[X] << 1;
    delta2[Y] = delta[Y] << 1;

    // If X is dominant, Y is submissive, and vice versa.
    if (delta[X] > delta[Y]) {
        dom = X;
        sub = Y;
    }
    else {
        dom = Y;
        sub = X;
    }

    error = delta2[sub] - delta[dom];

    /////////////////////////////////////////////////////
    // Bresenham's line drawing algorithm execution

    for (domSteps = 0; domSteps < delta[dom]; ++domSteps)
    {
        intPos[dom] += increment[dom];
        if (error >= 0)
        {
            intPos[sub] += increment[sub];
            error -= delta2[dom];
        }
        error += delta2[sub];

        // Only check pixel if we're not due to skip any, or if this is the last pixel
        if (++skipped > skip || domSteps + 1 == delta[dom])
        {
            // Scene wrapping, if necessary
            if (wrap)
                g_SceneMan.WrapPosition(intPos[X], intPos[Y]);

            materialID = GetTerrMatter(intPos[X], intPos[Y]);
            // Ignore the ignore material
            if (materialID != ignoreMaterial)
            {
                // Get the material object
                foundMaterial = GetMaterialFromID(materialID);

                // See if we found a pixel of equal or more strength than the threshold
                if (foundMaterial->GetIntegrity() >= strength)
                {
                    // Save result and report success
                    foundPixel = true;
                    result.SetXY(intPos[X], intPos[Y]);
                    // Save last ray pos
                    s_LastRayHitPos.SetXY(intPos[X], intPos[Y]);
                    break;
                }
            }
            skipped = 0;

            if (m_pDebugLayer && m_DrawRayCastVisualizations) { m_pDebugLayer->SetPixel(intPos[X], intPos[Y], 13); }
        }
    }

    // If no pixel of sufficient strength was found, set the result to the final tried position
    if (!foundPixel)
        result.SetXY(intPos[X], intPos[Y]);

    return foundPixel;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool SceneMan::CastWeaknessRay(const Vector &start, const Vector &ray, float strength, Vector &result, int skip, bool wrap)
{
    int hitCount = 0, error, dom, sub, domSteps, skipped = skip;
    int intPos[2], delta[2], delta2[2], increment[2];
    bool foundPixel = false;
    unsigned char materialID;
    Material const *foundMaterial;

    intPos[X] = std::floor(start.m_X);
    intPos[Y] = std::floor(start.m_Y);
    delta[X] = std::floor(start.m_X + ray.m_X) - intPos[X];
    delta[Y] = std::floor(start.m_Y + ray.m_Y) - intPos[Y];

    if (delta[X] == 0 &&  delta[Y] == 0)
        return false;

    /////////////////////////////////////////////////////
    // Bresenham's line drawing algorithm preparation

    if (delta[X] < 0)
    {
        increment[X] = -1;
        delta[X] = -delta[X];
    }
    else
        increment[X] = 1;

    if (delta[Y] < 0)
    {
        increment[Y] = -1;
        delta[Y] = -delta[Y];
    }
    else
        increment[Y] = 1;

    // Scale by 2, for better accuracy of the error at the first pixel
    delta2[X] = delta[X] << 1;
    delta2[Y] = delta[Y] << 1;

    // If X is dominant, Y is submissive, and vice versa.
    if (delta[X] > delta[Y]) {
        dom = X;
        sub = Y;
    }
    else {
        dom = Y;
        sub = X;
    }

    error = delta2[sub] - delta[dom];

    /////////////////////////////////////////////////////
    // Bresenham's line drawing algorithm execution

    for (domSteps = 0; domSteps < delta[dom]; ++domSteps)
    {
        intPos[dom] += increment[dom];
        if (error >= 0)
        {
            intPos[sub] += increment[sub];
            error -= delta2[dom];
        }
        error += delta2[sub];

        // Only check pixel if we're not due to skip any, or if this is the last pixel
        if (++skipped > skip || domSteps + 1 == delta[dom])
        {
            // Scene wrapping, if necessary
            if (wrap)
                g_SceneMan.WrapPosition(intPos[X], intPos[Y]);

            materialID = GetTerrMatter(intPos[X], intPos[Y]);
            foundMaterial = GetMaterialFromID(materialID);

            // See if we found a pixel of equal or less strength than the threshold
            if (foundMaterial->GetIntegrity() <= strength)
            {
                // Save result and report success
                foundPixel = true;
                result.SetXY(intPos[X], intPos[Y]);
                // Save last ray pos
                s_LastRayHitPos.SetXY(intPos[X], intPos[Y]);
                break;
            }

            skipped = 0;

            if (m_pDebugLayer && m_DrawRayCastVisualizations) { m_pDebugLayer->SetPixel(intPos[X], intPos[Y], 13); }
        }
    }

    // If no pixel of sufficient strength was found, set the result to the final tried position
    if (!foundPixel)
        result.SetXY(intPos[X], intPos[Y]);

    return foundPixel;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

MOID SceneMan::CastMORay(const Vector &start, const Vector &ray, MOID ignoreMOID, int ignoreTeam, unsigned char ignoreMaterial, bool ignoreAllTerrain, int skip)
{
    int hitCount = 0, error, dom, sub, domSteps, skipped = skip;
    int intPos[2], delta[2], delta2[2], increment[2];
    MOID hitMOID = g_NoMOID;
    unsigned char hitTerrain = 0;

    intPos[X] = std::floor(start.m_X);
    intPos[Y] = std::floor(start.m_Y);
    delta[X] = std::floor(start.m_X + ray.m_X) - intPos[X];
    delta[Y] = std::floor(start.m_Y + ray.m_Y) - intPos[Y];

    if (delta[X] == 0 && delta[Y] == 0)
        return g_NoMOID;

    /////////////////////////////////////////////////////
    // Bresenham's line drawing algorithm preparation

    if (delta[X] < 0)
    {
        increment[X] = -1;
        delta[X] = -delta[X];
    }
    else
        increment[X] = 1;

    if (delta[Y] < 0)
    {
        increment[Y] = -1;
        delta[Y] = -delta[Y];
    }
    else
        increment[Y] = 1;

    // Scale by 2, for better accuracy of the error at the first pixel
    delta2[X] = delta[X] << 1;
    delta2[Y] = delta[Y] << 1;

    // If X is dominant, Y is submissive, and vice versa.
    if (delta[X] > delta[Y]) {
        dom = X;
        sub = Y;
    }
    else {
        dom = Y;
        sub = X;
    }

    error = delta2[sub] - delta[dom];

    /////////////////////////////////////////////////////
    // Bresenham's line drawing algorithm execution

    for (domSteps = 0; domSteps < delta[dom]; ++domSteps)
    {
        intPos[dom] += increment[dom];
        if (error >= 0)
        {
            intPos[sub] += increment[sub];
            error -= delta2[dom];
        }
        error += delta2[sub];

        // Only check pixel if we're not due to skip any, or if this is the last pixel
        if (++skipped > skip || domSteps + 1 == delta[dom])
        {

            // Scene wrapping, if necessary
            g_SceneMan.WrapPosition(intPos[X], intPos[Y]);

            // Detect MOIDs
            hitMOID = GetMOIDPixel(intPos[X], intPos[Y], ignoreTeam);
            if (hitMOID != g_NoMOID && hitMOID != ignoreMOID && g_MovableMan.GetRootMOID(hitMOID) != ignoreMOID)
            {
                // Save last ray pos
                s_LastRayHitPos.SetXY(intPos[X], intPos[Y]);
                return hitMOID;
            }

            // Detect terrain hits
            if (!ignoreAllTerrain)
            {
                hitTerrain = g_SceneMan.GetTerrMatter(intPos[X], intPos[Y]);
                if (hitTerrain != g_MaterialAir && hitTerrain != ignoreMaterial)
                {
                    // Save last ray pos
                    s_LastRayHitPos.SetXY(intPos[X], intPos[Y]);
                    return g_NoMOID;
                }
            }

            skipped = 0;

            if (m_pDebugLayer && m_DrawRayCastVisualizations) { m_pDebugLayer->SetPixel(intPos[X], intPos[Y], 13); }
        }
    }

    // Didn't hit anything but air
    return g_NoMOID;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool SceneMan::CastFindMORay(const Vector &start, const Vector &ray, MOID targetMOID, Vector &resultPos, unsigned char ignoreMaterial, bool ignoreAllTerrain, int skip)
{
    int hitCount = 0, error, dom, sub, domSteps, skipped = skip;
    int intPos[2], delta[2], delta2[2], increment[2];
    MOID hitMOID = g_NoMOID;
    unsigned char hitTerrain = 0;

    intPos[X] = std::floor(start.m_X);
    intPos[Y] = std::floor(start.m_Y);
    delta[X] = std::floor(start.m_X + ray.m_X) - intPos[X];
    delta[Y] = std::floor(start.m_Y + ray.m_Y) - intPos[Y];

    if (delta[X] == 0 && delta[Y] == 0)
        return g_NoMOID;

    /////////////////////////////////////////////////////
    // Bresenham's line drawing algorithm preparation

    if (delta[X] < 0)
    {
        increment[X] = -1;
        delta[X] = -delta[X];
    }
    else
        increment[X] = 1;

    if (delta[Y] < 0)
    {
        increment[Y] = -1;
        delta[Y] = -delta[Y];
    }
    else
        increment[Y] = 1;

    // Scale by 2, for better accuracy of the error at the first pixel
    delta2[X] = delta[X] << 1;
    delta2[Y] = delta[Y] << 1;

    // If X is dominant, Y is submissive, and vice versa.
    if (delta[X] > delta[Y]) {
        dom = X;
        sub = Y;
    }
    else {
        dom = Y;
        sub = X;
    }

    error = delta2[sub] - delta[dom];

    /////////////////////////////////////////////////////
    // Bresenham's line drawing algorithm execution

    for (domSteps = 0; domSteps < delta[dom]; ++domSteps)
    {
        intPos[dom] += increment[dom];
        if (error >= 0)
        {
            intPos[sub] += increment[sub];
            error -= delta2[dom];
        }
        error += delta2[sub];

        // Only check pixel if we're not due to skip any, or if this is the last pixel
        if (++skipped > skip || domSteps + 1 == delta[dom])
        {
            // Scene wrapping, if necessary
            g_SceneMan.WrapPosition(intPos[X], intPos[Y]);

            // Detect MOIDs
            hitMOID = GetMOIDPixel(intPos[X], intPos[Y], Activity::NoTeam);
            if (hitMOID == targetMOID || g_MovableMan.GetRootMOID(hitMOID) == targetMOID)
            {
                // Found target MOID, so save result and report success
                resultPos.SetXY(intPos[X], intPos[Y]);
                // Save last ray pos
                s_LastRayHitPos.SetXY(intPos[X], intPos[Y]);
                return true;
            }

            // Detect terrain hits
            if (!ignoreAllTerrain)
            {
                hitTerrain = g_SceneMan.GetTerrMatter(intPos[X], intPos[Y]);
                if (hitTerrain != g_MaterialAir && hitTerrain != ignoreMaterial)
                {
                    // Save last ray pos
                    s_LastRayHitPos.SetXY(intPos[X], intPos[Y]);
                    return false;
                }
            }

            skipped = 0;

            if (m_pDebugLayer && m_DrawRayCastVisualizations) { m_pDebugLayer->SetPixel(intPos[X], intPos[Y], 13); }
        }
    }

    // Didn't hit the target
    return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

float SceneMan::CastObstacleRay(const Vector &start, const Vector &ray, Vector &obstaclePos, Vector &freePos, MOID ignoreMOID, int ignoreTeam, unsigned char ignoreMaterial, int skip)
{
    int hitCount = 0, error, dom, sub, domSteps, skipped = skip;
    int intPos[2], delta[2], delta2[2], increment[2];
    bool hitObstacle = false;

    intPos[X] = std::floor(start.m_X);
    intPos[Y] = std::floor(start.m_Y);
    delta[X] = std::floor(start.m_X + ray.m_X) - intPos[X];
    delta[Y] = std::floor(start.m_Y + ray.m_Y) - intPos[Y];
    // The fraction of a pixel that we start from, to be added to the integer result positions for accuracy
    Vector startFraction(start.m_X - intPos[X], start.m_Y - intPos[Y]);

    if (delta[X] == 0 && delta[Y] == 0) {
        return -1.0f;
    }

    /////////////////////////////////////////////////////
    // Bresenham's line drawing algorithm preparation

    if (delta[X] < 0) {
        increment[X] = -1;
        delta[X] = -delta[X];
    } else {
        increment[X] = 1;
    }

    if (delta[Y] < 0) {
        increment[Y] = -1;
        delta[Y] = -delta[Y];
    } else {
        increment[Y] = 1;
    }

    // Scale by 2, for better accuracy of the error at the first pixel
    delta2[X] = delta[X] * 2;
    delta2[Y] = delta[Y] * 2;

    // If X is dominant, Y is submissive, and vice versa.
    if (delta[X] > delta[Y]) {
        dom = X;
        sub = Y;
    } else {
        dom = Y;
        sub = X;
    }

    error = delta2[sub] - delta[dom];

    /////////////////////////////////////////////////////
    // Bresenham's line drawing algorithm execution

    for (domSteps = 0; domSteps < delta[dom]; ++domSteps)
    {
        intPos[dom] += increment[dom];
        if (error >= 0)
        {
            intPos[sub] += increment[sub];
            error -= delta2[dom];
        }
        error += delta2[sub];

        // Only check pixel if we're not due to skip any, or if this is the last pixel
        if (++skipped > skip || domSteps + 1 == delta[dom])
        {
            // Scene wrapping, if necessary
            g_SceneMan.WrapPosition(intPos[X], intPos[Y]);

            unsigned char checkMat = GetTerrMatter(intPos[X], intPos[Y]);
            MOID checkMOID = GetMOIDPixel(intPos[X], intPos[Y], ignoreTeam);

            // Translate any found MOID into the root MOID of that hit MO
            if (checkMOID != g_NoMOID)
            {
                MovableObject *pHitMO = g_MovableMan.GetMOFromID(checkMOID);
                if (pHitMO)
                {
                    checkMOID = pHitMO->GetRootID();
                }
            }

            // See if we found the looked-for pixel of the correct material,
            // Or an MO is blocking the way
            if ((checkMat != g_MaterialAir && checkMat != ignoreMaterial) || (checkMOID != g_NoMOID && checkMOID != ignoreMOID)) {
                hitObstacle = true;
                obstaclePos.SetXY(intPos[X], intPos[Y]);
                // Save last ray pos
                s_LastRayHitPos.SetXY(intPos[X], intPos[Y]);
                break;
            } else {
                freePos.SetXY(intPos[X], intPos[Y]);
            }

            skipped = 0;

            if (m_pDebugLayer && m_DrawRayCastVisualizations) { m_pDebugLayer->SetPixel(intPos[X], intPos[Y], 13); }
        } else {
            freePos.SetXY(intPos[X], intPos[Y]);
        }
    }

    // Add the pixel fraction to the free position if there were any free pixels
    if (domSteps != 0) {
        freePos += startFraction;
    }

    if (hitObstacle)
    {
        // Add the pixel fraction to the obstacle position, to acoid losing precision
        obstaclePos += startFraction;
        if (domSteps == 0) {
            // If there was an obstacle on the start position, return 0 as the distance to obstacle
            return 0.0F;
        } else {
            // Calculate the length between the start and the found material pixel coords
            return g_SceneMan.ShortestDistance(obstaclePos, start).GetMagnitude();
        }
    }

    // Didn't hit anything but air
    return -1.0F;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const Vector& SceneMan::GetLastRayHitPos()
{
    // The absolute end position of the last ray cast
    return s_LastRayHitPos;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

float SceneMan::FindAltitude(const Vector &from, int max, int accuracy, bool fromSceneOrbitDirection)
{
// TODO: Also make this avoid doors
    Vector temp(from);
    ForceBounds(temp);

    Directions orbitDirection = Directions::Up;
    if (fromSceneOrbitDirection && m_pCurrentScene) {
        orbitDirection = m_pCurrentScene->GetTerrain()->GetOrbitDirection();
    }

    float yDir = max > 0 ? max : g_SceneMan.GetSceneHeight();
    yDir *= orbitDirection == Directions::Up ? 1.0 : -1.0f;
    Vector direction = Vector(0, yDir);

    float result = g_SceneMan.CastNotMaterialRay(temp, direction, g_MaterialAir, accuracy);
    // If we didn't find anything but air, then report max height
    if (result < 0) {
        result = max > 0 ? max : g_SceneMan.GetSceneHeight();
    }

    return orbitDirection == Directions::Up ? result : g_SceneMan.GetSceneHeight() - result;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool SceneMan::OverAltitude(const Vector &point, int threshold, int accuracy)
{
    Vector temp(point);
    ForceBounds(temp);
    return g_SceneMan.CastNotMaterialRay(temp, Vector(0, threshold), g_MaterialAir, accuracy) < 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Vector SceneMan::MovePointToGround(const Vector &from, int maxAltitude, int accuracy)
{
    // Todo, instead of a nograv area maybe best to tag certain areas as NoGrav. As otherwise it's tricky to keep track of when things are removed
    if (m_pCurrentScene) {
        Scene::Area* noGravArea = m_pCurrentScene->GetOptionalArea("NoGravityArea");
        if (noGravArea && noGravArea->IsInside(from)) {
            return from;
        }
    }

    Vector temp(from);
    ForceBounds(temp);

    float altitude = FindAltitude(temp, g_SceneMan.GetSceneHeight(), accuracy);
    
    // If there's no ground beneath us, do nothing
    if (altitude == g_SceneMan.GetSceneHeight()) {
        return temp;
    }

    // Only move down if we're above the maxAltitude over the ground
    Vector groundPoint(temp.m_X, temp.m_Y + (altitude > maxAltitude ? altitude - maxAltitude : 0));
    return groundPoint;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool SceneMan::IsWithinBounds(const int pixelX, const int pixelY, const int margin) const
{
    if (m_pCurrentScene)
        return m_pCurrentScene->GetTerrain()->IsWithinBounds(pixelX, pixelY, margin);

    return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool SceneMan::ForceBounds(int &posX, int &posY) const
{
    RTEAssert(m_pCurrentScene, "Trying to access scene before there is one!");
    return m_pCurrentScene->GetTerrain()->ForceBounds(posX, posY);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool SceneMan::ForceBounds(Vector &pos) const
{
    RTEAssert(m_pCurrentScene, "Trying to access scene before there is one!");

    int posX = std::floor(pos.m_X);
    int posY = std::floor(pos.m_Y);

    bool wrapped = m_pCurrentScene->GetTerrain()->ForceBounds(posX, posY);

    pos.m_X = posX + (pos.m_X - std::floor(pos.m_X));
    pos.m_Y = posY + (pos.m_Y - std::floor(pos.m_Y));

    return wrapped;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool SceneMan::WrapPosition(int &posX, int &posY) const
{
    RTEAssert(m_pCurrentScene, "Trying to access scene before there is one!");
    return m_pCurrentScene->GetTerrain()->WrapPosition(posX, posY);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool SceneMan::WrapPosition(Vector &pos) const
{
    RTEAssert(m_pCurrentScene, "Trying to access scene before there is one!");

    int posX = std::floor(pos.m_X);
    int posY = std::floor(pos.m_Y);

    bool wrapped = m_pCurrentScene->GetTerrain()->WrapPosition(posX, posY);

    pos.m_X = posX + (pos.m_X - std::floor(pos.m_X));
    pos.m_Y = posY + (pos.m_Y - std::floor(pos.m_Y));

    return wrapped;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Vector SceneMan::SnapPosition(const Vector &pos, bool snap) const
{
    Vector snappedPos = pos;

    if (snap)
    {
        snappedPos.m_X = std::floor((pos.m_X / SCENESNAPSIZE) + 0.5) * SCENESNAPSIZE;
        snappedPos.m_Y = std::floor((pos.m_Y / SCENESNAPSIZE) + 0.5) * SCENESNAPSIZE;
    }

    return snappedPos;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Vector SceneMan::ShortestDistance(Vector pos1, Vector pos2, bool checkBounds) const
{
    if (!m_pCurrentScene)
        return Vector();

    if (checkBounds)
    {
        WrapPosition(pos1);
        WrapPosition(pos2);
    }

    Vector distance = pos2 - pos1;
    float sceneWidth = m_pCurrentScene->GetWidth();
    float sceneHeight = m_pCurrentScene->GetHeight();

    if (m_pCurrentScene->GetTerrain()->WrapsX())
    {
        if (distance.m_X > 0)
        {
            if (distance.m_X > (sceneWidth / 2))
                distance.m_X -= sceneWidth;
        }
        else
        {
            if (abs(distance.m_X) > (sceneWidth / 2))
                distance.m_X += sceneWidth;
        }
    }

    if (m_pCurrentScene->GetTerrain()->WrapsY())
    {
        if (distance.m_Y > 0)
        {
            if (distance.m_Y > (sceneHeight / 2))
                distance.m_Y -= sceneHeight;
        }
        else
        {
            if (abs(distance.m_Y) > (sceneHeight / 2))
                distance.m_Y += sceneHeight;
        }
    }

    return distance;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

float SceneMan::ShortestDistanceX(float val1, float val2, bool checkBounds, int direction) const
{
    if (!m_pCurrentScene)
        return 0;

    if (checkBounds)
    {
        int x1 = val1;
        int x2 = val2;
        int crap = 0;
        WrapPosition(x1, crap);
        WrapPosition(x2, crap);
        val1 = x1;
        val2 = x2;
    }

    float distance = val2 - val1;
    float sceneWidth = m_pCurrentScene->GetWidth();

    if (m_pCurrentScene->GetTerrain()->WrapsX())
    {
        if (distance > 0)
        {
            if (distance > (sceneWidth / 2))
                distance -= sceneWidth;
        }
        else
        {
            if (abs(distance) > (sceneWidth / 2))
                distance += sceneWidth;
        }

        // Apply direction constraint if wrapped
        if (direction > 0 && distance < 0)
            distance += sceneWidth;
        else if (direction < 0 && distance > 0)
            distance -= sceneWidth;
    }

    return distance;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

float SceneMan::ShortestDistanceY(float val1, float val2, bool checkBounds, int direction) const
{
    if (!m_pCurrentScene)
        return 0;

    if (checkBounds)
    {
        int y1 = val1;
        int y2 = val2;
        int crap = 0;
        WrapPosition(crap, y1);
        WrapPosition(crap, y2);
        val1 = y1;
        val2 = y2;
    }

    float distance = val2 - val1;
    float sceneHeight = m_pCurrentScene->GetHeight();

    if (m_pCurrentScene->GetTerrain()->WrapsY())
    {
        if (distance > 0)
        {
            if (distance > (sceneHeight / 2))
                distance -= sceneHeight;
        }
        else
        {
            if (abs(distance) > (sceneHeight / 2))
                distance += sceneHeight;
        }

        // Apply direction constraint if wrapped
        if (direction > 0 && distance < 0)
            distance += sceneHeight;
        else if (direction < 0 && distance > 0)
            distance -= sceneHeight;
    }

    return distance;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool SceneMan::ObscuredPoint(int x, int y, int team)
{
    bool obscured = m_pCurrentScene->GetTerrain()->GetPixel(x, y) != g_MaterialAir || GetMOIDPixel(x, y, Activity::NoTeam) != g_NoMOID;

    if (team != Activity::NoTeam)
        obscured = obscured || IsUnseen(x, y, team);

    return obscured;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int SceneMan::WrapRect(const IntRect &wrapRect, std::list<IntRect> &outputList)
{
    // Always add at least one copy of the unwrapped rect
    int addedTimes = 1;
    outputList.push_back(wrapRect);

    // Only bother with wrap checking if the scene actually wraps around in X
    if (SceneWrapsX())
    {
        int sceneWidth = GetSceneWidth();

        if (wrapRect.m_Left < 0)
        {
            outputList.push_back(wrapRect);
            outputList.back().m_Left += sceneWidth;
            outputList.back().m_Right += sceneWidth;
            addedTimes++;
        }
        if (wrapRect.m_Right >= sceneWidth)
        {
            outputList.push_back(wrapRect);
            outputList.back().m_Left -= sceneWidth;
            outputList.back().m_Right -= sceneWidth;
            addedTimes++;
        }
    }

    // Only bother with wrap checking if the scene actually wraps around in Y
    if (SceneWrapsY())
    {
        int sceneHeight = GetSceneHeight();

        if (wrapRect.m_Top < 0)
        {
            outputList.push_back(wrapRect);
            outputList.back().m_Top += sceneHeight;
            outputList.back().m_Bottom += sceneHeight;
            addedTimes++;
        }
        if (wrapRect.m_Bottom >= sceneHeight)
        {
            outputList.push_back(wrapRect);
            outputList.back().m_Top -= sceneHeight;
            outputList.back().m_Bottom -= sceneHeight;
            addedTimes++;
        }
    }

    return addedTimes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int SceneMan::WrapBox(const Box &wrapBox, std::list<Box> &outputList)
{
    // Unflip the input box, or checking will be tedious
    Box flipBox(wrapBox);
    flipBox.Unflip();

    // Always add at least one copy of the unwrapped rect
    int addedTimes = 1;
    outputList.push_back(flipBox);

    // Only bother with wrap checking if the scene actually wraps around in X
    if (SceneWrapsX())
    {
        int sceneWidth = GetSceneWidth();

        if (flipBox.m_Corner.m_X < 0)
        {
            outputList.push_back(flipBox);
            outputList.back().m_Corner.m_X += sceneWidth;
            addedTimes++;
        }
        if (flipBox.m_Corner.m_X + flipBox.m_Width >= sceneWidth)
        {
            outputList.push_back(flipBox);
            outputList.back().m_Corner.m_X -= sceneWidth;
            addedTimes++;
        }
    }

    // Only bother with wrap checking if the scene actually wraps around in Y
    if (SceneWrapsY())
    {
        int sceneHeight = GetSceneHeight();

        if (flipBox.m_Corner.m_Y < 0)
        {
            outputList.push_back(flipBox);
            outputList.back().m_Corner.m_Y += sceneHeight;
            addedTimes++;
        }
        if (flipBox.m_Corner.m_Y + flipBox.m_Height >= sceneHeight)
        {
            outputList.push_back(flipBox);
            outputList.back().m_Corner.m_Y -= sceneHeight;
            addedTimes++;
        }
    }

    return addedTimes;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

Vector SceneMan::Lerp(float scaleStart, float scaleEnd, Vector startPos, Vector endPos, float progressScalar) const
{
    if (!m_pCurrentScene) {
        return RTE::Lerp(scaleStart, scaleEnd, startPos, endPos, progressScalar);
    }

    Vector startToEnd = ShortestDistance(startPos, endPos);
    Vector lerped = startPos + (startToEnd * RTE::Lerp(scaleStart, scaleEnd, 0.0F, 1.0F, progressScalar));
    WrapPosition(lerped);
    return lerped;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool SceneMan::AddSceneObject(SceneObject *sceneObject) {
	bool result = false;
	if (sceneObject) {
		if (MovableObject *sceneObjectAsMovableObject = dynamic_cast<MovableObject *>(sceneObject)) {
			return g_MovableMan.AddMO(sceneObjectAsMovableObject);
		} else if (TerrainObject *sceneObjectAsTerrainObject = dynamic_cast<TerrainObject *>(sceneObject)) {
			result = m_pCurrentScene && sceneObjectAsTerrainObject->PlaceOnTerrain(m_pCurrentScene->GetTerrain());
			if (result) {
				Box airBox(sceneObjectAsTerrainObject->GetPos() + sceneObjectAsTerrainObject->GetBitmapOffset(), static_cast<float>(sceneObjectAsTerrainObject->GetBitmapWidth()), static_cast<float>(sceneObjectAsTerrainObject->GetBitmapHeight()));
				m_pCurrentScene->GetTerrain()->CleanAirBox(airBox, GetScene()->WrapsX(), GetScene()->WrapsY());
			}
		}
	}
	delete sceneObject;
	return result;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SceneMan::Update(int screenId) {
	ZoneScoped;
    
    SLTerrain *terrain = g_ThreadMan.GetDrawableGameState().m_Terrain;
    if (!terrain) {
		return;
	}

	m_LastUpdatedScreen = screenId;

    const Vector &offset = g_CameraMan.GetOffset(screenId);

	if (m_pDebugLayer) {
        m_pDebugLayer->SetOffset(offset);
    }

	terrain->SetOffset(offset);
	terrain->Update();

	// Background layers may scroll in fractions of the real offset and need special care to avoid jumping after having traversed wrapped edges, so they need the total offset without taking wrapping into account.
    const Vector &unwrappedOffset = g_CameraMan.GetUnwrappedOffset(screenId);
	for (SLBackground *backgroundLayer : m_pCurrentScene->GetBackLayers()) {
		backgroundLayer->SetOffset(unwrappedOffset);
		backgroundLayer->Update();
	}

	// Update the unseen obstruction layer for this team's screen view, if there is one.
    const int teamId = g_CameraMan.GetScreenTeam(screenId);
	if (SceneLayer *unseenLayer = (teamId != Activity::NoTeam) ? m_pCurrentScene->GetUnseenLayer(teamId) : nullptr) {
        unseenLayer->SetOffset(offset);
    }

    // TODO_MULTITHREAD this should be updated in a sim update, not render update
    // Is this even necessary, though?
	if (m_CleanTimer.GetElapsedSimTimeMS() > CLEANAIRINTERVAL) {
		terrain->CleanAir();
		m_CleanTimer.Reset();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SceneMan::Draw(BITMAP *targetBitmap, BITMAP *targetGUIBitmap, const Vector &targetPos, bool skipBackgroundLayers, bool skipTerrain) {
	SLTerrain *terrain = g_ThreadMan.GetDrawableGameState().m_Terrain;
    if (!terrain) {
        return;
    }

	// Set up the target box to draw to on the target bitmap, if it is larger than the scene in either dimension.
	Box targetBox(Vector(), static_cast<float>(targetBitmap->w), static_cast<float>(targetBitmap->h));

	if (!terrain->WrapsX() && targetBitmap->w > GetSceneWidth()) {
		targetBox.SetCorner(Vector(static_cast<float>((targetBitmap->w - GetSceneWidth()) / 2), targetBox.GetCorner().GetY()));
		targetBox.SetWidth(static_cast<float>(GetSceneWidth()));
	}
	if (!terrain->WrapsY() && targetBitmap->h > GetSceneHeight()) {
		targetBox.SetCorner(Vector(targetBox.GetCorner().GetX(), static_cast<float>((targetBitmap->h - GetSceneHeight()) / 2)));
		targetBox.SetHeight(static_cast<float>(GetSceneHeight()));
	}

	switch (m_LayerDrawMode) {
		case LayerDrawMode::g_LayerTerrainMatter:
			terrain->SetLayerToDraw(SLTerrain::LayerType::MaterialLayer);
			terrain->Draw(targetBitmap, targetBox);
			break;
		default:
			if (!skipBackgroundLayers) {
				for (std::list<SLBackground *>::reverse_iterator backgroundLayer = m_pCurrentScene->GetBackLayers().rbegin(); backgroundLayer != m_pCurrentScene->GetBackLayers().rend(); ++backgroundLayer) {
					(*backgroundLayer)->Draw(targetBitmap, targetBox);
				}
			}
			if (!skipTerrain) {
				terrain->SetLayerToDraw(SLTerrain::LayerType::BackgroundLayer);
				terrain->Draw(targetBitmap, targetBox);
			}

            g_ThreadMan.SetRenderTarget(targetBitmap);
            g_ThreadMan.SetRenderOffset(targetPos);
            float proportionOfTime = g_TimerMan.GetPredictedProportionOfUpdateCompletion();
            for (auto& renderFunc : g_ThreadMan.GetDrawableGameState().m_RenderQueue) {
                renderFunc(proportionOfTime);
            }

			if (!skipTerrain) {
				terrain->SetLayerToDraw(SLTerrain::LayerType::ForegroundLayer);
				terrain->Draw(targetBitmap, targetBox);
			}
            if (!g_FrameMan.IsInMultiplayerMode()) {
                int teamId = g_CameraMan.GetScreenTeam(m_LastUpdatedScreen);
				if (SceneLayer *unseenLayer = (teamId != Activity::NoTeam) ? m_pCurrentScene->GetUnseenLayer(teamId) : nullptr) {
                    unseenLayer->Draw(targetBitmap, targetBox);
                }
			}

            // TODO_MULTITHREAD
#ifndef MULTITHREAD_SIM_AND_RENDER
            g_MovableMan.DrawHUD(targetGUIBitmap, targetPos, m_LastUpdatedScreen);
            g_PrimitiveMan.DrawPrimitives(m_LastUpdatedScreen, targetGUIBitmap, targetPos);
#endif
			g_ActivityMan.GetActivity()->DrawGUI(targetGUIBitmap, targetPos, m_LastUpdatedScreen);

#ifdef DRAW_NOGRAV_BOXES
            if (Scene::Area* noGravArea = m_pCurrentScene->GetArea("NoGravityArea")) {
                const std::vector<Box>& boxList = noGravArea->GetBoxes();
                g_FrameMan.SetTransTableFromPreset(TransparencyPreset::MoreTrans);
                drawing_mode(DRAW_MODE_TRANS, 0, 0, 0);

                std::list<Box> wrappedBoxes;
                for (std::vector<Box>::const_iterator bItr = boxList.begin(); bItr != boxList.end(); ++bItr)
                {
                    wrappedBoxes.clear();
                    g_SceneMan.WrapBox(*bItr, wrappedBoxes);

                    for (std::list<Box>::iterator wItr = wrappedBoxes.begin(); wItr != wrappedBoxes.end(); ++wItr)
                    {
                        Vector adjCorner = (*wItr).GetCorner() - targetPos;
                        rectfill(targetBitmap, adjCorner.m_X, adjCorner.m_Y, adjCorner.m_X + (*wItr).GetWidth(), adjCorner.m_Y + (*wItr).GetHeight(), g_RedColor);
                    }
                }
            }
#endif

			if (m_pDebugLayer) {
                m_pDebugLayer->Draw(targetBitmap, targetBox);
                m_pDebugLayer->ClearBitmap(g_MaskColor);
            }

			break;
	}

//#ifdef DEBUG_BUILD
    // Draw scene seams
    LinePrimitive horizontal(0, Vector(0, 0), Vector(GetSceneWidth(), 0), ColorKeys::g_GreenColor);
    LinePrimitive vertical(0, Vector(0, 0), Vector(0, GetSceneHeight()), ColorKeys::g_GreenColor);
    horizontal.Draw(targetBitmap, targetPos);
    vertical.Draw(targetBitmap, targetPos);
//#endif
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SceneMan::ClearSeenPixels()
{
    if (!m_pCurrentScene)
        return;

    for (int team = Activity::TeamOne; team < Activity::MaxTeamCount; ++team)
        m_pCurrentScene->ClearSeenPixels(team);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SceneMan::ClearCurrentScene() {
    m_pCurrentScene = nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

BITMAP * SceneMan::GetIntermediateBitmapForSettlingIntoTerrain(int moDiameter) const {
	int bitmapSizeNeeded = static_cast<int>(std::ceil(static_cast<float>(moDiameter) / 16.0F)) * 16;
	for (const auto &[bitmapSize, bitmapPtr] : m_IntermediateSettlingBitmaps) {
		if (std::min(bitmapSize, bitmapSizeNeeded) >= bitmapSizeNeeded) {
			return bitmapPtr;
		}
	}
	return m_IntermediateSettlingBitmaps.back().second;
}
} // namespace RTE