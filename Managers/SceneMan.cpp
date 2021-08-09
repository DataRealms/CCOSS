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

#include "SceneMan.h"
#include "PresetMan.h"
#include "FrameMan.h"
#include "ActivityMan.h"
#include "UInputMan.h"
#include "ConsoleMan.h"
#include "PrimitiveMan.h"
#include "SettingsMan.h"
#include "Scene.h"
#include "SLTerrain.h"
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

namespace RTE
{

#define CLEANAIRINTERVAL 200000
#define COMPACTINGHEIGHT 25

const std::string SceneMan::c_ClassName = "SceneMan";


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IntersectionCut
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     If this and the passed in IntRect intersect, this will be modified to
//                  represent the boolean AND of the two. If it doens't intersect, nothing
//                  happens and false is returned.

bool IntRect::IntersectionCut(const IntRect &rhs)
{
    if (Intersects(rhs))
    {
        m_Left = MAX(m_Left, rhs.m_Left);
        m_Right = MIN(m_Right, rhs.m_Right);
        m_Top = MAX(m_Top, rhs.m_Top);
        m_Bottom = MIN(m_Bottom, rhs.m_Bottom);
        return true;
    }

    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this SceneMan, effectively
//                  resetting the members of this abstraction level only.

void SceneMan::Clear()
{
    m_DefaultSceneName = "Tutorial Bunker";
    m_pSceneToLoad = 0;
    m_PlaceObjects = true;
	m_PlaceUnits = true;
    m_pCurrentScene = 0;
    m_pMOColorLayer = 0;
    m_pMOIDLayer = 0;
    m_MOIDDrawings.clear();
    m_pDebugLayer = nullptr;
    m_LastRayHitPos.Reset();

    m_LayerDrawMode = g_LayerNormal;

    m_MatNameMap.clear();
	m_apMatPalette.fill(nullptr);
    m_MaterialCount = 0;

	m_MaterialCopiesVector.clear();

    for (int i = 0; i < c_MaxScreenCount; ++i) {
        m_Offset[i].Reset();
        m_DeltaOffset[i].Reset();
        m_ScrollTarget[i].Reset();
        m_ScreenTeam[i] = Activity::NoTeam;
        m_ScrollSpeed[i] = 0.1;
        m_ScrollTimer[i].Reset();
        m_ScreenOcclusion[i].Reset();
        m_TargetWrapped[i] = false;
        m_SeamCrossCount[i][X] = 0;
        m_SeamCrossCount[i][Y] = 0;
    }

    m_pUnseenRevealSound = 0;
    m_DrawRayCastVisualizations = false;
    m_DrawPixelCheckVisualizations = false;
    m_LastUpdatedScreen = 0;
    m_SecondStructPass = false;
//    m_CalcTimer.Reset();
    m_CleanTimer.Reset();

	if (m_pOrphanSearchBitmap)
		destroy_bitmap(m_pOrphanSearchBitmap);
	m_pOrphanSearchBitmap = create_bitmap_ex(8, MAXORPHANRADIUS , MAXORPHANRADIUS);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the SceneMan object ready for use.

int SceneMan::Create(std::string readerFile)
{
    Reader *reader = new Reader();
    if (reader->Create(readerFile.c_str()))
        g_ConsoleMan.PrintString("ERROR: Could not find Scene definition file!");

    Serializable::Create(*reader);
    delete reader;

    return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AddMaterialCopy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a copy of passed material and stores it into internal vector 
//					to make sure there's only one material owner. Ownership not transfered.
// Arguments:       Material to add.
// Return value:    Pointer to stored material.
Material * SceneMan::AddMaterialCopy(Material *mat)
{
	Material * matCopy = dynamic_cast<Material *>(mat->Clone());
	if (matCopy)
		m_MaterialCopiesVector.push_back(matCopy);

	return matCopy;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  LoadScene
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Actually loads a new Scene into memory. has to be done before using
//                  this object.

int SceneMan::LoadScene(Scene *pNewScene, bool placeObjects, bool placeUnits) {
	if (!pNewScene) {
		return -1;
	}

	g_MovableMan.PurgeAllMOs();
	g_PostProcessMan.ClearScenePostEffects();

	if (m_pCurrentScene) {
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

    // Re-create the MoveableObject:s color SceneLayer
    delete m_pMOColorLayer;
    BITMAP *pBitmap = create_bitmap_ex(8, GetSceneWidth(), GetSceneHeight());
    clear_to_color(pBitmap, g_MaskColor);
    m_pMOColorLayer = new SceneLayer();
    m_pMOColorLayer->Create(pBitmap, true, Vector(), m_pCurrentScene->WrapsX(), m_pCurrentScene->WrapsY(), Vector(1.0, 1.0));
    pBitmap = 0;

    // Re-create the MoveableObject:s ID SceneLayer
    delete m_pMOIDLayer;
    pBitmap = create_bitmap_ex(c_MOIDLayerBitDepth, GetSceneWidth(), GetSceneHeight());
    clear_to_color(pBitmap, g_NoMOID);
    m_pMOIDLayer = new SceneLayer();
    m_pMOIDLayer->Create(pBitmap, false, Vector(), m_pCurrentScene->WrapsX(), m_pCurrentScene->WrapsY(), Vector(1.0, 1.0));
    pBitmap = 0;

    // Create the Debug SceneLayer
    if (m_DrawRayCastVisualizations || m_DrawPixelCheckVisualizations) {
        delete m_pDebugLayer;
        pBitmap = create_bitmap_ex(8, GetSceneWidth(), GetSceneHeight());
        clear_to_color(pBitmap, g_MaskColor);
        m_pDebugLayer = new SceneLayer();
        m_pDebugLayer->Create(pBitmap, true, Vector(), m_pCurrentScene->WrapsX(), m_pCurrentScene->WrapsY(), Vector(1.0, 1.0));
        pBitmap = nullptr;
    }

    // Finally draw the ID:s of the MO:s to the MOID layers for the first time
    g_MovableMan.UpdateDrawMOIDs(m_pMOIDLayer->GetBitmap());

	g_NetworkServer.LockScene(false);
	g_NetworkServer.ResetScene();

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetSceneToLoad
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets a scene to load later, by preset name.

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


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  LoadScene
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Actually loads the Scene set to be loaded in SetSceneToLoad.

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


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  LoadScene
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Loads a Scene right now, by preset name.

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


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReadProperty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads a property value from a reader stream. If the name isn't
//                  recognized by this class, then ReadProperty of the parent class
//                  is called. If the property isn't recognized by any of the base classes,
//                  false is returned, and the reader's position is untouched.

int SceneMan::ReadProperty(const std::string_view &propName, Reader &reader)
{
    if (propName == "AddScene")
        g_PresetMan.GetEntityPreset(reader);
    else if (propName == "AddTerrain")
        g_PresetMan.GetEntityPreset(reader);
    else if (propName == "AddTerrainDebris")
        g_PresetMan.GetEntityPreset(reader);
    else if (propName == "AddTerrainObject")
        g_PresetMan.GetEntityPreset(reader);
    else if (propName == "AddMaterial")
    {
        // Get this before reading Object, since if it's the last one in its datafile, the stream will show the parent file instead
        string objectFilePath = reader.GetCurrentFilePath();

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
                m_apMatPalette.at(tryId) = pNewMat;
                m_MatNameMap.insert(pair<string, unsigned char>(string(pNewMat->GetPresetName()), pNewMat->GetIndex()));
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
    }
    else
        return Serializable::ReadProperty(propName, reader);

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this SceneMan with a Writer for
//                  later recreation with Create(Reader &reader);

int SceneMan::Save(Writer &writer) const {
	g_ConsoleMan.PrintString("ERROR: Tried to save SceneMan, screen does not make sense");

	Serializable::Save(writer);

	for (int i = 0; i < m_MaterialCount; ++i) {
		writer.NewPropertyWithValue("AddMaterial", *(m_apMatPalette.at(i)));
	}

	return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the SceneMan object.

void SceneMan::Destroy()
{
    for (int i = 0; i < c_PaletteEntriesNumber; ++i)
        delete m_apMatPalette[i];

    delete m_pCurrentScene;
    delete m_pDebugLayer;
    delete m_pMOIDLayer;
    delete m_pMOColorLayer;
    delete m_pUnseenRevealSound;

	destroy_bitmap(m_pOrphanSearchBitmap);
	m_pOrphanSearchBitmap = 0;

    Clear();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetSceneDim
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the total dimensions (width and height) of the scene, in pixels.

Vector SceneMan::GetSceneDim() const
{
	if (m_pCurrentScene) {
		RTEAssert(m_pCurrentScene->GetTerrain() && m_pCurrentScene->GetTerrain()->GetBitmap(), "Trying to get terrain info before there is a scene or terrain!");
		return m_pCurrentScene->GetDimensions();
	}
    return Vector();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetSceneWidth
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the total width of the scene, in pixels.

int SceneMan::GetSceneWidth() const
{
	if (g_NetworkClient.IsConnectedAndRegistered()) {
		return g_NetworkClient.GetSceneWidth();
	}

	if (m_pCurrentScene)
        return m_pCurrentScene->GetWidth();
    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetSceneHeight
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the total height of the scene, in pixels.

int SceneMan::GetSceneHeight() const
{
//    RTEAssert(m_pCurrentScene, "Trying to get terrain info before there is a scene or terrain!");
    if (m_pCurrentScene)
        return m_pCurrentScene->GetHeight();
    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SceneWrapsX
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether the scene wraps its scrolling around the X axis.

bool SceneMan::SceneWrapsX() const
{
	if (g_NetworkClient.IsConnectedAndRegistered()) {
		return g_NetworkClient.SceneWrapsX();
	}

	if (m_pCurrentScene)
        return m_pCurrentScene->WrapsX();
    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SceneWrapsY
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether the scene wraps its scrolling around the Y axis.

bool SceneMan::SceneWrapsY() const
{
    if (m_pCurrentScene)
        return m_pCurrentScene->WrapsY();
    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetTerrain
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the SLTerrain.

SLTerrain * SceneMan::GetTerrain()
{
//    RTEAssert(m_pCurrentScene, "Trying to get terrain matter before there is a scene or terrain!");
    if (m_pCurrentScene)
        return m_pCurrentScene->GetTerrain();

    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetMOColorBitmap
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the bitmap of the intermediary collection SceneLayer that all
//                  MovableObject:s draw themselves onto before it itself gets drawn onto
//                  the screen back buffer.

BITMAP * SceneMan::GetMOColorBitmap() const { return m_pMOColorLayer->GetBitmap(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetDebugBitmap
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the bitmap of the SceneLayer that debug graphics is drawn onto.
//                  Will only return valid BITMAP if building with DEBUG_BUILD.

BITMAP *SceneMan::GetDebugBitmap() const {
    RTEAssert(m_pDebugLayer, "Tried to get debug bitmap but debug layer doesn't exist. Note that the debug layer is only created under certain circumstances.");
    return m_pDebugLayer->GetBitmap();
}



//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetMOIDBitmap
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the bitmap of the SceneLayer that all MovableObject:s draw their
//                  current (for the frame only!) MOID's onto.

BITMAP * SceneMan::GetMOIDBitmap() const { return m_pMOIDLayer->GetBitmap(); }

// TEMP!
//////////////////////////////////////////////////////////////////////////////////////////
// Method:          MOIDClearCheck
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes sure the MOID bitmap layer is completely of NoMOID color.
//                  If found to be not, dumps MOID layer and the FG actor color layer for
//                  debugging.

bool SceneMan::MOIDClearCheck()
{
    BITMAP *pMOIDMap = m_pMOIDLayer->GetBitmap();
    int badMOID = g_NoMOID;
    for (int y = 0; y < pMOIDMap->h; ++y)
    {
        for (int x = 0; x < pMOIDMap->w; ++x)
        {
            if ((badMOID = _getpixel(pMOIDMap, x, y)) != g_NoMOID)
            {
                g_FrameMan.SaveBitmapToPNG(pMOIDMap, "MOIDCheck");
                g_FrameMan.SaveBitmapToPNG(m_pMOColorLayer->GetBitmap(), "MOIDCheck");
                RTEAbort("Bad MOID of MO detected: " + g_MovableMan.GetMOFromID(badMOID)->GetPresetName());
                return false;
            }
        }
    }
    return true;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetTerrMatter
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets a specific pixel from the total material representation of
//                  this Scene. LockScene() must be called before using this method.

unsigned char SceneMan::GetTerrMatter(int pixelX, int pixelY)
{
    RTEAssert(m_pCurrentScene, "Trying to get terrain matter before there is a scene or terrain!");

    WrapPosition(pixelX, pixelY);

    if (m_pDebugLayer && m_DrawPixelCheckVisualizations) { m_pDebugLayer->SetPixel(pixelX, pixelY, 5); }

    BITMAP *pTMatBitmap = m_pCurrentScene->GetTerrain()->GetMaterialBitmap();

    // If it's still below or to the sides out of bounds after
    // what is supposed to be wrapped, shit is out of bounds.
    if (pixelX < 0 || pixelX >= pTMatBitmap->w || pixelY >= pTMatBitmap->h)
//        return g_MaterialOutOfBounds;
        return g_MaterialAir;

    // If above terrain bitmap, return air material.
    if (pixelY < 0)
        return g_MaterialAir;

    return getpixel(pTMatBitmap, pixelX, pixelY);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetMOIDPixel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets a MOID from pixel coordinates in the Scene. LockScene() must be
//                  called before using this method.

MOID SceneMan::GetMOIDPixel(int pixelX, int pixelY)
{
    WrapPosition(pixelX, pixelY);

    if (m_pDebugLayer && m_DrawPixelCheckVisualizations) { m_pDebugLayer->SetPixel(pixelX, pixelY, 5); }

    if (pixelX < 0 ||
       pixelX >= m_pMOIDLayer->GetBitmap()->w ||
       pixelY < 0 ||
       pixelY >= m_pMOIDLayer->GetBitmap()->h)
        return g_NoMOID;

    return getpixel(m_pMOIDLayer->GetBitmap(), pixelX, pixelY);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetMaterial
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets a specific material by name. Ownership is NOT transferred!

Material const * SceneMan::GetMaterial(const std::string &matName)
{
    map<std::string, unsigned char>::iterator itr = m_MatNameMap.find(matName);
    if (itr == m_MatNameMap.end())
    {
        g_ConsoleMan.PrintString("ERROR: Material of name: " + matName + " not found!");
        return 0;
    }
    else
        return m_apMatPalette.at((*itr).second);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetGlobalAcc
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the global acceleration (in m/s^2) that is applied to all movable
//                  objects' velocities during every frame. Typically models gravity.

Vector SceneMan::GetGlobalAcc() const
{
    RTEAssert(m_pCurrentScene, "Trying to get terrain matter before there is a scene or terrain!");
    return m_pCurrentScene->GetGlobalAcc();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetOffset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the offset (scroll position) of the terrain.

void SceneMan::SetOffset(const long offsetX, const long offsetY, int screen)
{
    if (screen >= c_MaxScreenCount)
        return;

    m_Offset[screen].m_X = offsetX;
    m_Offset[screen].m_Y = offsetY;
    CheckOffset(screen);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetScroll
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the offset (scroll position) of the terrain to center on
//                  specific world coordinates. If the coordinate to center on is close
//                  to the terrain border edges, the view will not scroll outside the
//                  borders.

void SceneMan::SetScroll(const Vector &center, int screen)
{
    if (screen >= c_MaxScreenCount)
        return;

	if (g_FrameMan.IsInMultiplayerMode())
	{
		m_Offset[screen].m_X = center.GetFloorIntX() - (g_FrameMan.GetPlayerFrameBufferWidth(screen) / 2);
		m_Offset[screen].m_Y = center.GetFloorIntY() - (g_FrameMan.GetPlayerFrameBufferHeight(screen) / 2);
	}
	else 
	{
		m_Offset[screen].m_X = center.GetFloorIntX() - (g_FrameMan.GetResX() / 2);
		m_Offset[screen].m_Y = center.GetFloorIntY() - (g_FrameMan.GetResY() / 2);
	}

    CheckOffset(screen);

// *** Temp hack
//    m_OffsetX = -m_OffsetX;
//    m_OffsetY = -m_OffsetY;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetScrollTarget
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Interpolates a smooth scroll of the view to a new offset over time.

void SceneMan::SetScrollTarget(const Vector &targetCenter,
                               float speed,
                               bool targetWrapped,
                               int screen)
{
    // See if it would make sense to automatically wrap
    if (!targetWrapped)
    {
        SLTerrain *pTerrain = m_pCurrentScene->GetTerrain();
        // If the difference is more than half the scene width, then wrap
        if ((pTerrain->WrapsX() && fabs(targetCenter.m_X - m_ScrollTarget[screen].m_X) > pTerrain->GetBitmap()->w / 2) ||
            (pTerrain->WrapsY() && fabs(targetCenter.m_Y - m_ScrollTarget[screen].m_Y) > pTerrain->GetBitmap()->h / 2))
            targetWrapped = true;
    }

    m_ScrollTarget[screen].m_X = targetCenter.m_X;
    m_ScrollTarget[screen].m_Y = targetCenter.m_Y;
    m_ScrollSpeed[screen] = speed;
    // Don't override a set wrapping, it will be reset to false upon a drawn frame
    m_TargetWrapped[screen] = m_TargetWrapped[screen] || targetWrapped;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const Vector & SceneMan::GetScrollTarget(int screen) const {
	 const Vector & offsetTarget = (g_NetworkClient.IsConnectedAndRegistered()) ? g_NetworkClient.GetFrameTarget() : m_ScrollTarget[screen];
	 return offsetTarget;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          TargetDistanceScalar
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Calculates a scalar of how distant a certain point in the world is
//                  from the currently closest scroll target of all active screens.

float SceneMan::TargetDistanceScalar(Vector point)
{
    if (!m_pCurrentScene)
        return 0;

    int screenCount = g_FrameMan.GetScreenCount();
    int screenRadius = MAX(g_FrameMan.GetPlayerScreenWidth(), g_FrameMan.GetPlayerScreenHeight()) / 2;
    int sceneRadius = MAX(m_pCurrentScene->GetWidth(), m_pCurrentScene->GetHeight()) / 2;
    // Avoid divide by zero problems if scene and screen radius are the same
    if (screenRadius == sceneRadius)
        sceneRadius += 100;
    float distance = 0;
    float scalar = 0;
    float closestScalar = 1.0;

    for (int screen = 0; screen < screenCount; ++screen)
    {
        distance = ShortestDistance(point, m_ScrollTarget[screen]).GetMagnitude();

        // Check if we're off the screen and then fall off
        if (distance > screenRadius)
        {
            // Get ratio of how close to the very opposite of teh scene the point is
            scalar = 0.5 + 0.5 * (distance - screenRadius) / (sceneRadius - screenRadius);
        }
        // Full audio if within the screen
        else
            scalar = 0;

        // See if this screen's distance scalar is the closest one yet
        if (scalar < closestScalar)
            closestScalar = scalar;
    }

    // Return the scalar that was shows the closest scroll target of any current screen to the point
    return closestScalar;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CheckOffset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes sure the current offset won't create a view of outside the scene.
//                  If that is found to be the case, the offset is corrected so that the
//                  view rectangle is as close to the old offset as possible, but still
//                  entirely within the scene world.

void SceneMan::CheckOffset(int screen)
{
    RTEAssert(m_pCurrentScene, "Trying to check offset before there is a scene or terrain!");

    // Handy
    SLTerrain *pTerrain = m_pCurrentScene->GetTerrain();
    RTEAssert(pTerrain, "Trying to get terrain matter before there is a scene or terrain!");

    if (!pTerrain->WrapsX() && m_Offset[screen].m_X < 0)
        m_Offset[screen].m_X = 0;

    if (!pTerrain->WrapsY() && m_Offset[screen].m_Y < 0)
        m_Offset[screen].m_Y = 0;

    int frameWidth = g_FrameMan.GetResX();
    int frameHeight = g_FrameMan.GetResY();
    frameWidth = frameWidth / (g_FrameMan.GetVSplit() ? 2 : 1);
    frameHeight = frameHeight / (g_FrameMan.GetHSplit() ? 2 : 1);

	if (g_FrameMan.IsInMultiplayerMode())
	{
		frameWidth = g_FrameMan.GetPlayerFrameBufferWidth(screen);
		frameHeight = g_FrameMan.GetPlayerFrameBufferHeight(screen);
	}

    if (!pTerrain->WrapsX() && m_Offset[screen].m_X >= pTerrain->GetBitmap()->w - frameWidth)
        m_Offset[screen].m_X = pTerrain->GetBitmap()->w - frameWidth;

    if (!pTerrain->WrapsY() && m_Offset[screen].m_Y >= pTerrain->GetBitmap()->h - frameHeight)
        m_Offset[screen].m_Y = pTerrain->GetBitmap()->h - frameHeight;

    if (!pTerrain->WrapsX() && m_Offset[screen].m_X >= pTerrain->GetBitmap()->w - frameWidth)
        m_Offset[screen].m_X = pTerrain->GetBitmap()->w - frameWidth;

    if (!pTerrain->WrapsY() && m_Offset[screen].m_Y >= pTerrain->GetBitmap()->h - frameHeight)
        m_Offset[screen].m_Y = pTerrain->GetBitmap()->h - frameHeight;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          LockScene
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Locks all dynamic internal scene bitmaps so that manipulaitons of the
//                  scene's color and matter representations can take place.
//                  Doing it in a separate method like this is more efficient because
//                  many bitmap manipulaitons can be performed between a lock and unlock.
//                  UnlockScene() should always be called after accesses are completed.

void SceneMan::LockScene()
{
//    RTEAssert(!m_pCurrentScene->IsLocked(), "Hey, locking already locked scene!");
    if (!m_pCurrentScene->IsLocked())
    {
        m_pCurrentScene->Lock();
        m_pMOColorLayer->LockBitmaps();
        m_pMOIDLayer->LockBitmaps();
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          UnlockScene
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Unlocks the scene's bitmaps and prevents access to display memory.
//                  Doing it in a separate method like this is more efficient because
//                  many bitmap accesses can be performed between a lock and an unlock.
//                  UnlockScene() should only be called after LockScene().

void SceneMan::UnlockScene()
{
//    RTEAssert(m_pCurrentScene->IsLocked(), "Hey, unlocking already unlocked scene!");
    if (m_pCurrentScene->IsLocked())
    {
        m_pCurrentScene->Unlock();
        m_pMOColorLayer->UnlockBitmaps();
        m_pMOIDLayer->UnlockBitmaps();
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SceneIsLocked
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether the entire scene is currently locked or not.

bool SceneMan::SceneIsLocked() const
{
    RTEAssert(m_pCurrentScene, "Trying to check offset before there is a scene or terrain!");
    return m_pCurrentScene->IsLocked();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          RegisterMOIDDrawing
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Registers an area of the MOID layer to be cleared upon finishing this
//                  sim update. Should be done every time anything is drawn the MOID layer.

void SceneMan::RegisterMOIDDrawing(const Vector &center, float radius)
{
    if (radius != 0)
        RegisterMOIDDrawing(center.m_X - radius, center.m_Y - radius, center.m_X + radius, center.m_Y + radius);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ClearAllMOIDDrawings
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all registered drawn areas of the MOID layer to the g_NoMOID
//                  color and clears the registrations too. Should be done each sim update.

void SceneMan::ClearAllMOIDDrawings()
{
    for (list<IntRect>::iterator itr = m_MOIDDrawings.begin(); itr != m_MOIDDrawings.end(); ++itr)
        ClearMOIDRect(itr->m_Left, itr->m_Top, itr->m_Right, itr->m_Bottom);

    m_MOIDDrawings.clear();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ClearMOIDRect
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Resets a specific rectangle of the scene's MOID layer to not contain
//                  any MOID data anymore. Sets it all to NoMOID. Will take care of wrapping.

void SceneMan::ClearMOIDRect(int left, int top, int right, int bottom)
{
    // Draw the first unwrapped rect
    rectfill(m_pMOIDLayer->GetBitmap(), left, top, right, bottom, g_NoMOID);

    // Draw wrapped rectangles
    if (g_SceneMan.SceneWrapsX())
    {
        int sceneWidth = m_pCurrentScene->GetWidth();

        if (left < 0)
        {
            int wrapLeft = left + sceneWidth;
            int wrapRight = sceneWidth - 1;
            rectfill(m_pMOIDLayer->GetBitmap(), wrapLeft, top, wrapRight, bottom, g_NoMOID);
        }
        if (right >= sceneWidth)
        {
            int wrapLeft = 0;
            int wrapRight = right - sceneWidth;
            rectfill(m_pMOIDLayer->GetBitmap(), wrapLeft, top, wrapRight, bottom, g_NoMOID);
        }
    }
    if (g_SceneMan.SceneWrapsY())
    {
        int sceneHeight = m_pCurrentScene->GetHeight();

        if (top < 0)
        {
            int wrapTop = top + sceneHeight;
            int wrapBottom = sceneHeight - 1;
            rectfill(m_pMOIDLayer->GetBitmap(), left, wrapTop, right, wrapBottom, g_NoMOID);
        }
        if (bottom >= sceneHeight)
        {
            int wrapTop = 0;
            int wrapBottom = bottom - sceneHeight;
            rectfill(m_pMOIDLayer->GetBitmap(), left, wrapTop, right, wrapBottom, g_NoMOID);
        }
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          WillPenetrate
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Test whether a pixel of the scene would be knocked loose and
//                  turned into a MO by a certian impulse force. Scene needs to be locked
//                  to do this!

bool SceneMan::WillPenetrate(const int posX,
                             const int posY,
                             const Vector &impulse)
{
    RTEAssert(m_pCurrentScene, "Trying to access scene before there is one!");

    if (!m_pCurrentScene->GetTerrain()->IsWithinBounds(posX, posY))
        return false;

    float impMag = impulse.GetMagnitude();
    unsigned char materialID = getpixel(m_pCurrentScene->GetTerrain()->GetMaterialBitmap(), posX, posY);

    return impMag >= GetMaterialFromID(materialID)->GetIntegrity();
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          RemoveOrphans
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns the area of an orphaned region at specified coordinates and remoes the region if requested.

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

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          RemoveOrphans
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns the area of an orphaned region at specified coordinates. 

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
				TerrainChange tc1;
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
				TerrainChange tc2;
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
				TerrainChange tc2;
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
				TerrainChange tc1;
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

	TerrainChange tc;
	tc.x = x;
	tc.y = y;
	tc.w = w;
	tc.h = h;
	tc.back = back;
	tc.color = color;
	g_NetworkServer.RegisterTerrainChange(tc);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          TryPenetrate
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Calculate whether a pixel of the scene would be knocked loose and
//                  turned into a MO by another particle of a certain material going at a
//                  certain velocity. If so, the incoming particle will knock loose the
//                  specified pixel in the scene and momentarily take its place.
//                  Use PenetrationResult() to retrieve the resulting effects on the
//                  incoming particle if it manages to knock the scene pixel out.

bool SceneMan::TryPenetrate(const int posX,
                            const int posY,
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

    unsigned char materialID = _getpixel(m_pCurrentScene->GetTerrain()->GetMaterialBitmap(), posX, posY);
    if (materialID == g_MaterialAir)
    {
//        RTEAbort("Why are we penetrating air??");
        return true;
    }
    Material const * sceneMat = GetMaterialFromID(materialID);
    Material const * spawnMat;

    float sprayScale = 0.1;
//    float spraySpread = 10.0;
    float impMag = impulse.GetMagnitude();

    // Test if impulse force is enough to penetrate
    if (impMag >= sceneMat->GetIntegrity())
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
        retardation = -(sceneMat->GetIntegrity() / impMag);

        // If this is a scrap pixel, or there is no background pixel 'supporting' the knocked-loose pixel, make the column above also turn into particles
        if (sceneMat->IsScrap() || _getpixel(m_pCurrentScene->GetTerrain()->GetBGColorBitmap(), posX, posY) == g_MaskColor)
        {
            // Get quicker direct access to bitmaps
            BITMAP *pFGColor = m_pCurrentScene->GetTerrain()->GetFGColorBitmap();
            BITMAP *pBGColor = m_pCurrentScene->GetTerrain()->GetBGColorBitmap();
            BITMAP *pMaterial = m_pCurrentScene->GetTerrain()->GetMaterialBitmap();

            int testMaterialID = g_MaterialAir;
            MOPixel *pixelMO = 0;
            Color spawnColor;
            float sprayMag = velocity.GetLargest() * sprayScale;
            Vector sprayVel;

            // Look at pixel above to see if it isn't air and has support, or should fall down
            for (int testY = posY - 1; testY > posY - COMPACTINGHEIGHT && testY >= 0; --testY)
            {
                // Check if there is a material pixel above
                if ((testMaterialID = _getpixel(pMaterial, posX, testY)) != g_MaterialAir)
                {
                    sceneMat = GetMaterialFromID(testMaterialID);

                    // No support in the background layer, or is scrap material, so make particle of some of them
                    if (sceneMat->IsScrap() || _getpixel(pBGColor, posX, testY) == g_MaskColor)
                    {
                        //  Only generate  particles of some of 'em
                        if (RandomNum() > 0.75F)
                        {
                            // Figure out the mateiral and color of the new spray particle
                            spawnMat = sceneMat->GetSpawnMaterial() ? GetMaterialFromID(sceneMat->GetSpawnMaterial()) : sceneMat;
                            if (spawnMat->UsesOwnColor())
                                spawnColor = spawnMat->GetColor();
                            else
                                spawnColor.SetRGBWithIndex(m_pCurrentScene->GetTerrain()->GetFGColorPixel(posX, testY));

                            // No point generating a key-colored MOPixel
                            if (spawnColor.GetIndex() != g_MaskColor)
                            {
                                // Figure out the randomized velocity the spray should have upward
								sprayVel.SetXY(sprayMag* RandomNormalNum() * 0.5F, (-sprayMag * 0.5F) + (-sprayMag * RandomNum(0.0F, 0.5F)));

                                // Create the new spray pixel
								pixelMO = new MOPixel(spawnColor, spawnMat->GetPixelDensity(), Vector(posX, testY), sprayVel, new Atom(Vector(), spawnMat->GetIndex(), 0, spawnColor, 2), 0);

                                // Let it loose into the world
                                pixelMO->SetToHitMOs(spawnMat->GetIndex() == c_GoldMaterialID);
                                pixelMO->SetToGetHitByMOs(false);
                                g_MovableMan.AddParticle(pixelMO);
                                pixelMO = 0;
                            }

							// Remove orphaned terrain left from hits and scrap damage
							RemoveOrphans(posX + testY%2 ? -1 : 1, testY, 5, 25, true);
						}

                        // Clear the terrain pixel now when the particle has been generated from it
						RegisterTerrainChange(posX, testY, 1, 1, g_MaskColor, false);
                        _putpixel(pFGColor, posX, testY, g_MaskColor);
                        _putpixel(pMaterial, posX, testY, g_MaterialAir);
                    }
                    // There is support, so stop checking
                    else
                        break;
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


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          MakeAllUnseen
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets one team's view of the scene to be unseen, using a generated map
//                  of a specific resolution chunkiness.

void SceneMan::MakeAllUnseen(Vector pixelSize, const int team)
{
    RTEAssert(m_pCurrentScene, "Messing with scene before the scene exists!");
	if (team < Activity::TeamOne || team >= Activity::MaxTeamCount) 
		return;

    m_pCurrentScene->FillUnseenLayer(pixelSize, team);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          MakeAllSeen
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets one team's view of the scene to be all seen.
// Arguments:       The team we're talking about.
// Return value:    None.

    void MakeAllSeen(const int team);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          LoadUnseenLayer
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Loads a bitmap from file and use it as the unseen layer for a team.

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


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AnythingUnseen
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tells whether a team has anything still unseen on the scene.

bool SceneMan::AnythingUnseen(const int team)
{
    RTEAssert(m_pCurrentScene, "Checking scene before the scene exists!");

    return m_pCurrentScene->GetUnseenLayer(team) != 0;
// TODO: Actually check all pixels on the map too?
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetUnseenResolution
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows what the resolution factor of the unseen map to the entire Scene
//                  is, in both axes.

Vector SceneMan::GetUnseenResolution(const int team) const
{
    RTEAssert(m_pCurrentScene, "Checking scene before the scene exists!");
	if (team < Activity::TeamOne || team >= Activity::MaxTeamCount) 
		return Vector(1, 1);

    SceneLayer *pUnseenLayer = m_pCurrentScene->GetUnseenLayer(team);
    if (pUnseenLayer)
        return pUnseenLayer->GetScaleFactor();

    return Vector(1, 1);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsUnseen
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Checks whether a pixel is in an unseen area on of a specific team.

bool SceneMan::IsUnseen(const int posX, const int posY, const int team)
{
    RTEAssert(m_pCurrentScene, "Checking scene before the scene exists!");
	if (team < Activity::TeamOne || team >= Activity::MaxTeamCount) 
		return false;

    SceneLayer *pUnseenLayer = m_pCurrentScene->GetUnseenLayer(team);
    if (pUnseenLayer)
    {
        // Translate to the scaled unseen layer's coordinates
        Vector scale = pUnseenLayer->GetScaleInverse();
        int scaledX = posX * scale.m_X;
        int scaledY = posY * scale.m_Y;
        return getpixel(pUnseenLayer->GetBitmap(), scaledX, scaledY) != g_MaskColor;
    }

    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          RevealUnseen
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reveals a pixel on the unseen map for a specific team, if there is any.

bool SceneMan::RevealUnseen(const int posX, const int posY, const int team)
{
    RTEAssert(m_pCurrentScene, "Checking scene before the scene exists!");
	if (team < Activity::TeamOne || team >= Activity::MaxTeamCount) 
		return false;

    SceneLayer *pUnseenLayer = m_pCurrentScene->GetUnseenLayer(team);
    if (pUnseenLayer)
    {
        // Translate to the scaled unseen layer's coordinates
        Vector scale = pUnseenLayer->GetScaleInverse();
        int scaledX = posX * scale.m_X;
        int scaledY = posY * scale.m_Y;

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


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          RestoreUnseen
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Hides a pixel on the unseen map for a specific team, if there is any.

bool SceneMan::RestoreUnseen(const int posX, const int posY, const int team)
{
    RTEAssert(m_pCurrentScene, "Checking scene before the scene exists!");
	if (team < Activity::TeamOne || team >= Activity::MaxTeamCount) 
		return false;

    SceneLayer *pUnseenLayer = m_pCurrentScene->GetUnseenLayer(team);
    if (pUnseenLayer)
    {
        // Translate to the scaled unseen layer's coordinates
        Vector scale = pUnseenLayer->GetScaleInverse();
        int scaledX = posX * scale.m_X;
        int scaledY = posY * scale.m_Y;

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


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          RevealUnseenBox
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reveals a box on the unseen map for a specific team, if there is any.

void SceneMan::RevealUnseenBox(const int posX, const int posY, const int width, const int height, const int team)
{
    RTEAssert(m_pCurrentScene, "Checking scene before the scene exists!");
	if (team < Activity::TeamOne || team >= Activity::MaxTeamCount) 
		return;

    SceneLayer *pUnseenLayer = m_pCurrentScene->GetUnseenLayer(team);
    if (pUnseenLayer)
    {
        // Translate to the scaled unseen layer's coordinates
        Vector scale = pUnseenLayer->GetScaleInverse();
        int scaledX = posX * scale.m_X;
        int scaledY = posY * scale.m_Y;
        int scaledW = width * scale.m_X;
        int scaledH = height * scale.m_Y;

        // Fill the box
        rectfill(pUnseenLayer->GetBitmap(), scaledX, scaledY, scaledX + scaledW, scaledY + scaledH, g_MaskColor);
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          RestoreUnseenBox
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Restore a box on the unseen map for a specific team, if there is any.

void SceneMan::RestoreUnseenBox(const int posX, const int posY, const int width, const int height, const int team)
{
    RTEAssert(m_pCurrentScene, "Checking scene before the scene exists!");
	if (team < Activity::TeamOne || team >= Activity::MaxTeamCount) 
		return;

    SceneLayer *pUnseenLayer = m_pCurrentScene->GetUnseenLayer(team);
    if (pUnseenLayer)
    {
        // Translate to the scaled unseen layer's coordinates
        Vector scale = pUnseenLayer->GetScaleInverse();
        int scaledX = posX * scale.m_X;
        int scaledY = posY * scale.m_Y;
        int scaledW = width * scale.m_X;
        int scaledH = height * scale.m_Y;

        // Fill the box
        rectfill(pUnseenLayer->GetBitmap(), scaledX, scaledY, scaledX + scaledW, scaledY + scaledH, g_BlackColor);
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CastUnseenRay
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Traces along a vector and reveals or hides pixels on the unseen layer of a team
//                  as long as the accumulated material strengths traced through the terrain
//                  don't exceed a specific value.

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

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CastSeeRay
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Traces along a vector and reveals pixels on the unseen layer of a team
//                  as long as the accumulated material strengths traced through the terrain
//                  don't exceed a specific value.

bool SceneMan::CastSeeRay(int team, const Vector &start, const Vector &ray, Vector &endPos, int strengthLimit, int skip)
{
	return CastUnseenRay(team, start, ray, endPos, strengthLimit, skip, true);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CastUnseeRay
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Traces along a vector and hides pixels on the unseen layer of a team
//                  as long as the accumulated material strengths traced through the terrain
//                  don't exceed a specific value.

bool SceneMan::CastUnseeRay(int team, const Vector &start, const Vector &ray, Vector &endPos, int strengthLimit, int skip)
{
	return CastUnseenRay(team, start, ray, endPos, strengthLimit, skip, false);
}



//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CastMaterialRay
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Traces along a vector and gets the location of the first encountered
//                  pixel of a specific material in the terrain.

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
                m_LastRayHitPos.SetXY(intPos[X], intPos[Y]);
                break;
            }

            skipped = 0;

            if (m_pDebugLayer && m_DrawRayCastVisualizations) { m_pDebugLayer->SetPixel(intPos[X], intPos[Y], 13); }
        }
    }

    return foundPixel;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CastMaterialRay
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Traces along a vector and returns how far along that ray there is an
//                  encounter with a pixel of a specific material in the terrain.

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


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CastNotMaterialRay
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Traces along a vector and gets the location of the first encountered
//                  pixel that is NOT of a specific material in the scene's terrain.

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
                (checkMOs && g_SceneMan.GetMOIDPixel(intPos[X], intPos[Y]) != g_NoMOID))
            {
                // Save result and report success
                foundPixel = true;
                result.SetXY(intPos[X], intPos[Y]);
                // Save last ray pos
                m_LastRayHitPos.SetXY(intPos[X], intPos[Y]);
                break;
            }

            skipped = 0;
            if (m_pDebugLayer && m_DrawRayCastVisualizations) { m_pDebugLayer->SetPixel(intPos[X], intPos[Y], 13); }
        }
    }

    return foundPixel;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CastNotMaterialRay
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Traces along a vector and returns how far along that ray there is an
//                  encounter with a pixel of OTHER than a specific material in the terrain.

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


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CastStrengthSumRay
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Traces along a vector and returns how the sum of all encountered pixels'
//                  material strength values. This will take wrapping into account.

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
            if (materialID != g_MaterialAir && materialID != ignoreMaterial)
                strengthSum += GetMaterialFromID(materialID)->GetIntegrity();

            skipped = 0;

            if (m_pDebugLayer && m_DrawRayCastVisualizations) { m_pDebugLayer->SetPixel(intPos[X], intPos[Y], 13); }
        }
    }

    return strengthSum;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CastMaxStrengthRay
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Traces along a vector and returns the strongest of all encountered pixels'
//                  material strength values exept doors.
//                  This will take wrapping into account.

float SceneMan::CastMaxStrengthRay(const Vector &start, const Vector &end, int skip)
{
    Vector ray = g_SceneMan.ShortestDistance(start, end);
    float maxStrength = 0;

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
            if (materialID != g_MaterialDoor)
                maxStrength = std::max(maxStrength, GetMaterialFromID(materialID)->GetIntegrity());

            skipped = 0;

            if (m_pDebugLayer && m_DrawRayCastVisualizations) { m_pDebugLayer->SetPixel(intPos[X], intPos[Y], 13); }
        }
    }
    
    return maxStrength;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CastStrengthRay
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Traces along a vector and shows where along that ray there is an
//                  encounter with a pixel of a material with strength more than or equal
//                  to a specific value.


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
                    m_LastRayHitPos.SetXY(intPos[X], intPos[Y]);
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


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CastWeaknessRay
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Traces along a vector and shows where along that ray there is an
//                  encounter with a pixel of a material with strength less than or equal
//                  to a specific value.

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
                m_LastRayHitPos.SetXY(intPos[X], intPos[Y]);
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


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CastMORay
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Traces along a vector and returns MOID of the first non-ignored
//                  non-NoMOID MO encountered. If a non-air terrain pixel is encountered
//                  first, 0 will be returned.

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
            hitMOID = GetMOIDPixel(intPos[X], intPos[Y]);
            if (hitMOID != g_NoMOID && hitMOID != ignoreMOID && g_MovableMan.GetRootMOID(hitMOID) != ignoreMOID)
            {
                // Check if we're supposed to ignore the team of what we hit
                if (ignoreTeam != Activity::NoTeam)
                {
                    const MovableObject *pHitMO = g_MovableMan.GetMOFromID(hitMOID);
                    pHitMO = pHitMO ? pHitMO->GetRootParent() : 0;
                    // Yup, we are supposed to ignore this!
                    if (pHitMO && pHitMO->IgnoresTeamHits() && pHitMO->GetTeam() == ignoreTeam)
                    {
                        ;
                    }
                    else
                    {
                        // Save last ray pos
                        m_LastRayHitPos.SetXY(intPos[X], intPos[Y]);
                        return hitMOID;
                    }
                }
                // Legit hit
                else
                {
                    // Save last ray pos
                    m_LastRayHitPos.SetXY(intPos[X], intPos[Y]);
                    return hitMOID;
                }
            }

            // Detect terrain hits
            if (!ignoreAllTerrain)
            {
                hitTerrain = g_SceneMan.GetTerrMatter(intPos[X], intPos[Y]);
                if (hitTerrain != g_MaterialAir && hitTerrain != ignoreMaterial)
                {
                    // Save last ray pos
                    m_LastRayHitPos.SetXY(intPos[X], intPos[Y]);
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


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CastFindMORay
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Traces along a vector and shows where a specific MOID has been found.

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
            hitMOID = GetMOIDPixel(intPos[X], intPos[Y]);
            if (hitMOID == targetMOID || g_MovableMan.GetRootMOID(hitMOID) == targetMOID)
            {
                // Found target MOID, so save result and report success
                resultPos.SetXY(intPos[X], intPos[Y]);
                // Save last ray pos
                m_LastRayHitPos.SetXY(intPos[X], intPos[Y]);
                return true;
            }

            // Detect terrain hits
            if (!ignoreAllTerrain)
            {
                hitTerrain = g_SceneMan.GetTerrMatter(intPos[X], intPos[Y]);
                if (hitTerrain != g_MaterialAir && hitTerrain != ignoreMaterial)
                {
                    // Save last ray pos
                    m_LastRayHitPos.SetXY(intPos[X], intPos[Y]);
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


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CastObstacleRay
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Traces along a vector and returns the length of how far the trace went
//                  without hitting any non-ignored terrain material or MOID at all.

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

    if (delta[X] == 0 && delta[Y] == 0)
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

            unsigned char checkMat = GetTerrMatter(intPos[X], intPos[Y]);
            MOID checkMOID = GetMOIDPixel(intPos[X], intPos[Y]);

            // Translate any found MOID into the root MOID of that hit MO
            if (checkMOID != g_NoMOID)
            {
                MovableObject *pHitMO = g_MovableMan.GetMOFromID(checkMOID);
                if (pHitMO)
                {
                    checkMOID = pHitMO->GetRootID();
                    // Check if we're supposed to ignore the team of what we hit
                    if (ignoreTeam != Activity::NoTeam)
                    {
                        pHitMO = pHitMO->GetRootParent();
                        // We are indeed supposed to ignore this object because of its ignoring of its specific team
                        if (pHitMO && pHitMO->IgnoresTeamHits() && pHitMO->GetTeam() == ignoreTeam)
                            checkMOID = g_NoMOID;
                    }
                }
            }

            // See if we found the looked-for pixel of the correct material,
            // Or an MO is blocking the way
            if ((checkMat != g_MaterialAir && checkMat != ignoreMaterial) || (checkMOID != g_NoMOID && checkMOID != ignoreMOID))
            {
                hitObstacle = true;
                obstaclePos.SetXY(intPos[X], intPos[Y]);
                // Save last ray pos
                m_LastRayHitPos.SetXY(intPos[X], intPos[Y]);
                break;
            }
            else
                freePos.SetXY(intPos[X], intPos[Y]);

            skipped = 0;

            if (m_pDebugLayer && m_DrawRayCastVisualizations) { m_pDebugLayer->SetPixel(intPos[X], intPos[Y], 13); }
        }
        else
            freePos.SetXY(intPos[X], intPos[Y]);
    }

    // Add the pixel fraction to the free position if there were any free pixels
    if (domSteps != 0)
        freePos += startFraction;

    if (hitObstacle)
    {
        // Add the pixel fraction to the obstacle position, to acoid losing precision
        obstaclePos += startFraction;
        // If there was an obstacle on the start position, return 0 as the distance to obstacle
        if (domSteps == 0)
            return 0;
        // Calculate the length between the start and the found material pixel coords
        else
            return g_SceneMan.ShortestDistance(obstaclePos, start).GetMagnitude();
    }

    // Didn't hit anything but air
    return -1.0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          FindAltitude
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Calculates the altitide of a certain point above the terrain, measured
//                  in pixels.

float SceneMan::FindAltitude(const Vector &from, int max, int accuracy)
{
// TODO: Also make this avoid doors
    Vector temp(from);
    ForceBounds(temp);

    float result = g_SceneMan.CastNotMaterialRay(temp, Vector(0, (max > 0 ? max : g_SceneMan.GetSceneHeight())), g_MaterialAir, accuracy);
    // If we didn't find anything but air, then report max height
    if (result < 0)
        result = max > 0 ? max : g_SceneMan.GetSceneHeight();
    return result;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          OverAltitude
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Calculates the altitide of a certain point above the terrain, measured
//                  in pixels, and then tells if that point is over a certain value.

bool SceneMan::OverAltitude(const Vector &point, int threshold, int accuracy)
{
    Vector temp(point);
    ForceBounds(temp);
    return g_SceneMan.CastNotMaterialRay(temp, Vector(0, threshold), g_MaterialAir, accuracy) < 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          MovePointToGround
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Takes an arbitrary point in the air and calculates it to be straight
//                  down at a certain maximum distance from the ground.

Vector SceneMan::MovePointToGround(const Vector &from, int maxAltitude, int accuracy)
{
    Vector temp(from);
    ForceBounds(temp);

    float altitude = FindAltitude(temp, g_SceneMan.GetSceneHeight(), accuracy);
    // Only move down if we're above the maxAltitude over the ground
    Vector groundPoint(temp.m_X, temp.m_Y + (altitude > maxAltitude ? altitude - maxAltitude : 0));
    return groundPoint;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          StructuralCalc
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Calculates the structural integrity of the Terrain during a set time
//                  and turns structurally unsound areas into MovableObject:s.

void SceneMan::StructuralCalc(unsigned long calcTime) {

// TODO: Develop this!")
    return;


    if (calcTime <= 0)
        return;
    // Pad the time a little for FPS smoothness.
    calcTime -= 1;
    m_CalcTimer.Reset();

    SLTerrain *pTerrain = g_SceneMan.GetTerrain();
    BITMAP *pColBitmap = pTerrain->GetFGColorBitmap();
    BITMAP *pMatBitmap = pTerrain->GetMaterialBitmap();
    BITMAP *pStructBitmap = pTerrain->GetStructuralBitmap();
    int posX, posY, height = pColBitmap->h, width = pColBitmap->w;

    // Lock all bitmaps involved, outside the loop.
    acquire_bitmap(pColBitmap);
    acquire_bitmap(pMatBitmap);
    acquire_bitmap(pStructBitmap);

    // Preprocess bottom row to have full support.
    for (posX = width - 1; posX >= 0; --posX)
        putpixel(pStructBitmap, posX, height - 1, 255);

    // Start on the second row from bottom.
    for (posY = height - 2; posY >= 0 && !m_CalcTimer.IsPastSimMS(calcTime); --posY) {
        for (posX = width - 1; posX >= 0; --posX) {
            getpixel(pColBitmap, posX, posY);
            getpixel(pMatBitmap, posX, posY);
            getpixel(pStructBitmap, posX, posY);
        }
    }

    // Unlock all bitmaps involved.
    release_bitmap(pColBitmap);
    release_bitmap(pMatBitmap);
    release_bitmap(pStructBitmap);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsWithinBounds
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns whether the integer coordinates passed in are within the
//                  bounds of the current Scene, considering its wrapping.

bool SceneMan::IsWithinBounds(const int pixelX, const int pixelY, const int margin)
{
    if (m_pCurrentScene)
        return m_pCurrentScene->GetTerrain()->IsWithinBounds(pixelX, pixelY, margin);

    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ForceBounds
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Wraps or bounds a position coordinate if it is off bounds of the
//                  Scene, depending on the wrap settings of this Scene.

bool SceneMan::ForceBounds(int &posX, int &posY)
{
    RTEAssert(m_pCurrentScene, "Trying to access scene before there is one!");
    return m_pCurrentScene->GetTerrain()->ForceBounds(posX, posY);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ForceBounds
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Wraps or bounds a position coordinate if it is off bounds of the
//                  Scene, depending on the wrap settings of this Scene.

bool SceneMan::ForceBounds(Vector &pos)
{
    RTEAssert(m_pCurrentScene, "Trying to access scene before there is one!");

    int posX = std::floor(pos.m_X);
    int posY = std::floor(pos.m_Y);

    bool wrapped = m_pCurrentScene->GetTerrain()->ForceBounds(posX, posY);

    pos.m_X = posX + (pos.m_X - std::floor(pos.m_X));
    pos.m_Y = posY + (pos.m_Y - std::floor(pos.m_Y));

    return wrapped;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          WrapPosition
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Only wraps a position coordinate if it is off bounds of the Scene
//                  and wrapping in the corresponding axes are turned on.

bool SceneMan::WrapPosition(int &posX, int &posY)
{
    RTEAssert(m_pCurrentScene, "Trying to access scene before there is one!");
    return m_pCurrentScene->GetTerrain()->WrapPosition(posX, posY);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          WrapPosition
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Only wraps a position coordinate if it is off bounds of the Scene
//                  and wrapping in the corresponding axes are turned on.

bool SceneMan::WrapPosition(Vector &pos)
{
    RTEAssert(m_pCurrentScene, "Trying to access scene before there is one!");

    int posX = std::floor(pos.m_X);
    int posY = std::floor(pos.m_Y);

    bool wrapped = m_pCurrentScene->GetTerrain()->WrapPosition(posX, posY);

    pos.m_X = posX + (pos.m_X - std::floor(pos.m_X));
    pos.m_Y = posY + (pos.m_Y - std::floor(pos.m_Y));

    return wrapped;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SnapPosition
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns a position snapped to the current scene grid.

Vector SceneMan::SnapPosition(const Vector &pos, bool snap)
{
    Vector snappedPos = pos;

    if (snap)
    {
        snappedPos.m_X = std::floor((pos.m_X / SCENESNAPSIZE) + 0.5) * SCENESNAPSIZE;
        snappedPos.m_Y = std::floor((pos.m_Y / SCENESNAPSIZE) + 0.5) * SCENESNAPSIZE;
    }

    return snappedPos;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ShortestDistance
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Calculates the shortest distance between two points in scene
//                  coordinates, taking into account all wrapping and out of bounds of the
//                  two points.

Vector SceneMan::ShortestDistance(Vector pos1, Vector pos2, bool checkBounds)
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


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ShortestDistanceX
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Calculates the shortest distance between two X values in scene
//                  coordinates, taking into account all wrapping and out of bounds of the
//                  two values.

float SceneMan::ShortestDistanceX(float val1, float val2, bool checkBounds, int direction)
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


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ShortestDistanceY
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Calculates the shortest distance between two Y values in scene
//                  coordinates, taking into account all wrapping and out of bounds of the
//                  two values.

float SceneMan::ShortestDistanceY(float val1, float val2, bool checkBounds, int direction)
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


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ObscuredPoint
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tells whether a point on the scene is obscured by MOID or Terrain
//                  non-air material.

bool SceneMan::ObscuredPoint(int x, int y, int team)
{
    bool obscured = m_pMOIDLayer->GetPixel(x, y) != g_NoMOID || m_pCurrentScene->GetTerrain()->GetPixel(x, y) != g_MaterialAir;

    if (team != Activity::NoTeam)
        obscured = obscured || IsUnseen(x, y, team);

    return obscured;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          WrapRect
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Takes a rect and adds all possible scenewrapped appearances of that rect
//                  to a passed-in list. IF if a passed in rect straddles the seam of a
//                  wrapped scene axis, it will be added twice to the output list. If it
//                  doesn't straddle any seam, it will be only added once.

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


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          WrapBox
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Takes a Box and adds all possible scenewrapped appearances of that Box
//                  to a passed-in list. IF if a passed in rect straddles the seam of a
//                  wrapped scene axis, it will be added twice to the output list. If it
//                  doesn't straddle any seam, it will be only added once.

int SceneMan::WrapBox(const Box &wrapBox, list<Box> &outputList)
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


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AddTerrainObject
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Takes TerrainObject and applies it to the terrain
//					OWNERSHIP NOT TRANSFERED!

bool SceneMan::AddTerrainObject(TerrainObject *pObject)
{
    if (!pObject)
        return false;

    bool result =  m_pCurrentScene->GetTerrain()->ApplyObject(pObject);
	if (result)
	{
		Vector corner = pObject->GetPos() + pObject->GetBitmapOffset();
		Box box = Box(corner, pObject->GetBitmapWidth(), pObject->GetBitmapHeight());
		
		m_pCurrentScene->GetTerrain()->CleanAirBox(box, GetScene()->WrapsX(), GetScene()->WrapsY());
	}
	return result;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AddSceneObject
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Takes any scene object and adds it to the scene in the appropriate way.
//                  If it's a TerrainObject, then it gets applied to the terrain, if it's
//                  an MO, it gets added to the correct type group in MovableMan.

bool SceneMan::AddSceneObject(SceneObject *pObject)
{
    if (!pObject)
        return false;

    // Find out what kind it is and apply accordingly
    if (MovableObject *pMO = dynamic_cast<MovableObject *>(pObject))
    {
        // No need to clean up here, AddMO takes ownership and takes care of it in either case
        return g_MovableMan.AddMO(pMO);
    }
    else if (TerrainObject *pTO = dynamic_cast<TerrainObject *>(pObject))
    {
        bool result = m_pCurrentScene->GetTerrain()->ApplyObject(pTO);
        // Have to clean up the added object here, since ApplyObject doesn't take ownership
        delete pTO;
        pObject = pTO = 0;
        return result;
    }

    delete pObject;
    pObject = 0;
    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the state of this SceneMan. Supposed to be done every frame
//                  before drawing.

void SceneMan::Update(int screen)
{
    if (m_pCurrentScene == nullptr) {
        return;
    }

    // Record screen was the last updated screen
    m_LastUpdatedScreen = screen;

    // Update the scene, only if doing the first screen, since it only needs done once per update
    if (screen == 0)
        m_pCurrentScene->Update();

    // Handy
    SLTerrain *pTerrain = m_pCurrentScene->GetTerrain();

    // Learn about the unseen layer, if any
    int team = m_ScreenTeam[screen];
    SceneLayer *pUnseenLayer = team != Activity::NoTeam ? m_pCurrentScene->GetUnseenLayer(team) : 0;

    ////////////////////////////////
    // Scrolling interpolation

    // Only adjust wrapping if this is the frame to be drawn
    if (g_TimerMan.DrawnSimUpdate())
    {
        // Adjust for wrapping, if the scrolltarget jumped a seam this frame, as reported by
        // whatever client set it (the scrolltarget) this frame. This is to avoid big,
        // scene-wide jumps in scrolling when traversing the seam.
        if (m_TargetWrapped[screen])
        {
            if (pTerrain->WrapsX())
            {
                if (m_ScrollTarget[screen].m_X < (pTerrain->GetBitmap()->w / 2))
                {
                    m_Offset[screen].m_X -= pTerrain->GetBitmap()->w;
                    m_SeamCrossCount[screen][X] += 1;
                }
                else
                {
                    m_Offset[screen].m_X += pTerrain->GetBitmap()->w;
                    m_SeamCrossCount[screen][X] -= 1;
                }
            }

            if (pTerrain->WrapsY())
            {
                if (m_ScrollTarget[screen].m_Y < (pTerrain->GetBitmap()->h / 2))
                {
                    m_Offset[screen].m_Y -= pTerrain->GetBitmap()->h;
                    m_SeamCrossCount[screen][Y] += 1;
                }
                else
                {
                    m_Offset[screen].m_Y += pTerrain->GetBitmap()->h;
                    m_SeamCrossCount[screen][Y] -= 1;
                }
            }
        }
        m_TargetWrapped[screen] = false;
    }

    Vector oldOffset(m_Offset[screen]);

    // Get the offset target, since the scroll target is centered on the target in scene units.
    Vector offsetTarget;
	if (g_FrameMan.IsInMultiplayerMode())
	{
		offsetTarget.m_X = m_ScrollTarget[screen].m_X - (g_FrameMan.GetPlayerFrameBufferWidth(screen) / 2);
		offsetTarget.m_Y = m_ScrollTarget[screen].m_Y - (g_FrameMan.GetPlayerFrameBufferHeight(screen) / 2);
	}
	else 
	{
		offsetTarget.m_X = m_ScrollTarget[screen].m_X - (g_FrameMan.GetResX() / (g_FrameMan.GetVSplit() ? 4 : 2));
		offsetTarget.m_Y = m_ScrollTarget[screen].m_Y - (g_FrameMan.GetResY() / (g_FrameMan.GetHSplit() ? 4 : 2));
	}

    // Take the occlusion of the screens into account,
    // so that the scrolltarget is still centered on the terrain-visible portion of the screen.
    offsetTarget -= m_ScreenOcclusion[screen] / 2;

    if (offsetTarget.GetFloored() != m_Offset[screen].GetFloored())
    {
        Vector scrollVec(offsetTarget - m_Offset[screen]);
        // Figure out the scroll progress this frame, and cap it at 1.0
        float scrollProgress = m_ScrollSpeed[screen] * m_ScrollTimer[screen].GetElapsedRealTimeMS() * 0.05;
        if (scrollProgress > 1.0)
            scrollProgress = 1.0;
// TODO: Check if rounding is appropriate?
        SetOffset(m_Offset[screen] + (scrollVec * scrollProgress).GetRounded(), screen);
    }

    /////////////////////////////////
    // Apply offsets to SceneLayer:s

    m_pMOColorLayer->SetOffset(m_Offset[screen]);
    m_pMOIDLayer->SetOffset(m_Offset[screen]);

    if (m_pDebugLayer) { m_pDebugLayer->SetOffset(m_Offset[screen]); }

    pTerrain->SetOffset(m_Offset[screen]);
    pTerrain->Update();

    // Scroll the unexplored/unseen layer, if there is one
    if (pUnseenLayer)
    {
        // Update the unseen obstruction layer for this team's screen view
        pUnseenLayer->SetOffset(m_Offset[screen]);
    }

    // Background layers may scroll in fractions of the real offset, and need special care to avoid jumping after having traversed wrapped edges
    // Reconstruct and give them the total offset, not taking any wrappings into account
    Vector offsetUnwrapped = m_Offset[screen];
    offsetUnwrapped.m_X += pTerrain->GetBitmap()->w * m_SeamCrossCount[screen][X];
    offsetUnwrapped.m_Y += pTerrain->GetBitmap()->h * m_SeamCrossCount[screen][Y];

    for (list<SceneLayer *>::iterator itr = m_pCurrentScene->GetBackLayers().begin(); itr != m_pCurrentScene->GetBackLayers().end(); ++itr)
        (*itr)->SetOffset(offsetUnwrapped);

    // Calculate delta offset.
    m_DeltaOffset[screen] = m_Offset[screen] - oldOffset;

    // Reset the timer so we can know the real time diff next frame
    m_ScrollTimer[screen].Reset();

    // Clean the color layer of the Terrain
    if (m_CleanTimer.GetElapsedSimTimeMS() > CLEANAIRINTERVAL)
    {
        pTerrain->CleanAir();
        m_CleanTimer.Reset();
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this SceneMan's current graphical representation to a
//                  BITMAP of choice.

void SceneMan::Draw(BITMAP *pTargetBitmap, BITMAP *pTargetGUIBitmap, const Vector &targetPos, bool skipSkybox, bool skipTerrain)
{
    if (m_pCurrentScene == nullptr) {
        return;
    }
    // Handy
    SLTerrain *pTerrain = m_pCurrentScene->GetTerrain();

    // Learn about the unseen layer, if any
    int team = m_ScreenTeam[m_LastUpdatedScreen];
    SceneLayer *pUnseenLayer = team != Activity::NoTeam ? m_pCurrentScene->GetUnseenLayer(team) : 0;

    // Set up the target box to draw to on the target bitmap, if it is larger than the scene in either dimension
    Box targetBox(Vector(0, 0), pTargetBitmap->w, pTargetBitmap->h);

    if (!pTerrain->WrapsX() && pTargetBitmap->w > GetSceneWidth())
    {
        targetBox.m_Corner.m_X = (pTargetBitmap->w - GetSceneWidth()) / 2;
        targetBox.m_Width = GetSceneWidth();
    }
    if (!pTerrain->WrapsY() && pTargetBitmap->h > GetSceneHeight())
    {
        targetBox.m_Corner.m_Y = (pTargetBitmap->h - GetSceneHeight()) / 2;
        targetBox.m_Height = GetSceneHeight();
    }

    switch (m_LayerDrawMode)
    {
        case g_LayerTerrainMatter:
            pTerrain->SetToDrawMaterial(true);
            pTerrain->Draw(pTargetBitmap, targetBox);
            break;
        case g_LayerMOID:
            m_pMOIDLayer->Draw(pTargetBitmap, targetBox);
            break;
        // Draw normally
        default:
			if (skipSkybox)
			{

			} 
			else
			{
				// Background Layers
				for (list<SceneLayer *>::reverse_iterator itr = m_pCurrentScene->GetBackLayers().rbegin(); itr != m_pCurrentScene->GetBackLayers().rend(); ++itr)
					(*itr)->Draw(pTargetBitmap, targetBox);
			}

			if (!skipTerrain)
				// Terrain background
				pTerrain->DrawBackground(pTargetBitmap, targetBox);
            // Movables' color layer
            m_pMOColorLayer->Draw(pTargetBitmap, targetBox);
            // Terrain foreground
            pTerrain->SetToDrawMaterial(false);
			if (!skipTerrain)
				pTerrain->Draw(pTargetBitmap, targetBox);

            // Obscure unexplored/unseen areas
            if (pUnseenLayer && !g_FrameMan.IsInMultiplayerMode())
            {
                // Draw the unseen obstruction layer so it obscures the team's view
                pUnseenLayer->DrawScaled(pTargetBitmap, targetBox);
            }

            // Actor and gameplay HUDs and GUIs
            g_MovableMan.DrawHUD(pTargetGUIBitmap, targetPos, m_LastUpdatedScreen);
			g_PrimitiveMan.DrawPrimitives(m_LastUpdatedScreen, pTargetGUIBitmap, targetPos);
//            g_ActivityMan.GetActivity()->Draw(pTargetBitmap, targetPos, m_LastUpdatedScreen);
            g_ActivityMan.GetActivity()->DrawGUI(pTargetGUIBitmap, targetPos, m_LastUpdatedScreen);

//            std::snprintf(str, sizeof(str), "Normal Layer Draw Mode\nHit M to cycle modes");

            if (m_pDebugLayer) { m_pDebugLayer->Draw(pTargetBitmap, targetBox); }
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ClearMOColorLayer
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears the color MO layer. Should be done every frame.

void SceneMan::ClearMOColorLayer()
{
    clear_to_color(m_pMOColorLayer->GetBitmap(), g_MaskColor);

    if (m_pDebugLayer) { clear_to_color(m_pDebugLayer->GetBitmap(), g_MaskColor); }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ClearMOIDLayer
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears the MOID layer. Should be done every frame.
//					Looks like it never actually called anymore

void SceneMan::ClearMOIDLayer()
{
    clear_to_color(m_pMOIDLayer->GetBitmap(), g_NoMOID);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ClearSeenPixels
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears the list of pixels on the unseen map that have been revealed.

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

} // namespace RTE
