#ifndef _RTESCENEMAN_
#define _RTESCENEMAN_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            SceneMan.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Header file for the SceneMan class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "Serializable.h"
#include "Timer.h"
#include "Box.h"
#include "Singleton.h"

#include "ActivityMan.h"

#define g_SceneMan SceneMan::Instance()

namespace RTE
{

class Scene;
class SceneLayer;
class SLTerrain;
class SceneObject;
class TerrainObject;
class MovableObject;
class Material;
class SoundContainer;
struct PostEffect;

// Different modes to draw the SceneLayers in
enum LayerDrawMode
{
    g_LayerNormal = 0,
    g_LayerTerrainMatter,
    g_LayerMOID
};

enum
{
    REGULAR_MAT_OFFSET = 8,
    g_MaterialAir = 0,
    g_MaterialDefault = 1,
    g_MaterialOutOfBounds = 1,
    g_MaterialCavity = 1,
    g_MaterialGold = 2,
    g_MaterialGrass = 128,
    g_MaterialFlesh = 145,
    g_MaterialSand = 8,
    g_MaterialDoor = 181
};

#define SCENEGRIDSIZE 24
#define SCENESNAPSIZE 12
#define MAXORPHANRADIUS 11

//////////////////////////////////////////////////////////////////////////////////////////
// Struct:          IntRect
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     A simple rectangle with integer coordinates.
// Parent(s):       None.
// Class history:   8/4/2007 IntRect created.

struct IntRect
{
    int m_Left;
    int m_Top;
    int m_Right;
    int m_Bottom;

    IntRect() { m_Left = m_Top = m_Right = m_Bottom = 0; }
    IntRect(int left, int top, int right, int bottom) { m_Left = left; m_Top = top; m_Right = right; m_Bottom = bottom; }
    bool Intersects(const IntRect &rhs) { return m_Left < rhs.m_Right && m_Right > rhs.m_Left && m_Top < rhs.m_Bottom && m_Bottom > rhs.m_Top; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IntersectionCut
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     If this and the passed in IntRect intersect, this will be modified to
//                  represent the boolean AND of the two. If it doens't intersect, nothing
//                  happens and false is returned.
// Arguments:       The other IntRect to cut against.
// Return value:    Whether an intersection was detected and this was cut down to the AND.

    bool IntersectionCut(const IntRect &rhs);


};


//////////////////////////////////////////////////////////////////////////////////////////
// Class:           SceneMan
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     The singleton manager of all terrain and backgrounds in the RTE.
// Parent(s):       Singleton, Serializable.
// Class history:   12/25/2001 SceneMan created.

class SceneMan : public Singleton<SceneMan>, public Serializable {
	friend class SettingsMan;

//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable, method and friend function declarations

public:

	SerializableClassNameGetter;
	SerializableOverrideMethods;


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     SceneMan
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a SceneMan object in system
//                  memory. Create() should be called before using the object.
// Arguments:       None.

    SceneMan() { m_pOrphanSearchBitmap = 0; Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Destructor:      ~SceneMan
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destructor method used to clean up a SceneMan object before deletion
//                  from system memory.
// Arguments:       None.

	~SceneMan() { Destroy(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the SceneMan object ready for use.
// Arguments:       A string with the filepath to a Reader file from screen this SceneMan's
//                  data should be created.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

	int Create(std::string readerFile);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetDefaultSceneName
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the instance name of the default Scene to be loaded if nothing
//                  else is available.
// Arguments:       The default scene instance name.
// Return value:    None.

    void SetDefaultSceneName(std::string defaultSceneName) { m_DefaultSceneName = defaultSceneName; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetDefaultSceneName
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the name of the default A to be loaded if nothing
//                  else is available.
// Arguments:       None.
// Return value:    The default Scene instance name.

    std::string GetDefaultSceneName() const { return m_DefaultSceneName; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  LoadScene
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Actually loads a new Scene into memory. has to be done before using
//                  this object.
// Arguments:       The instance of the Scene, ownership IS transferred!
//                  Whether the scene should actually apply all its SceneObject:s placed
//                  in its definition.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

	int LoadScene(Scene *pNewScene, bool placeObjects = true, bool placeUnits = true);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  SetSceneToLoad
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Stores a Scene reference to be loaded later into the SceneMan.
// Arguments:       The instance reference of the Scene, ownership IS NOT (!!) transferred!
//                  Whether the scene should actually apply all its SceneObject:s placed
//                  in its definition.
// Return value:    None.

	void SetSceneToLoad(const Scene *pLoadScene, bool placeObjects = true, bool placeUnits = true) { m_pSceneToLoad = pLoadScene; m_PlaceObjects = placeObjects; m_PlaceUnits = placeUnits; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  SetSceneToLoad
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets a scene to load later, by preset name.
// Arguments:       The name of the Scene preset instance to load.
//                  Whether the scene should actually apply all its SceneObject:s placed
//                  in its definition.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

	int SetSceneToLoad(std::string sceneName, bool placeObjects = true, bool placeUnits = true);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  GetSceneToLoad
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the stored Scene reference to be loaded later into the SceneMan.
// Arguments:       None.
// Return value:    The instance reference of the Scene, ownership IS NOT (!!) transferred!

	const Scene * GetSceneToLoad() { return m_pSceneToLoad; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  LoadScene
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Actually loads the Scene set to be loaded in SetSceneToLoad.
// Arguments:       None.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

	int LoadScene();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  LoadScene
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Loads a Scene right now, by preset name.
// Arguments:       The name of the Scene preset instance to load.
//                  Whether the scene should actually apply all its SceneObject:s placed
//                  in its definition.
//                  Whether the scene should actually deploy all units placed in its definition.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

	int LoadScene(std::string sceneName, bool placeObjects = true, bool placeUnits = true);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  LoadScene
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Loads a Scene right now, by preset name.
// Arguments:       The name of the Scene preset instance to load.
//                  Whether the scene should actually apply all its SceneObject:s placed
//                  in its definition.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

	int LoadScene(std::string sceneName, bool placeObjects = true) { return LoadScene(sceneName, placeObjects, true); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  Reset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Resets the entire SceneMan, including its inherited members, to
//                  their default settings or values.
// Arguments:       None.
// Return value:    None.

	void Reset() override { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the SceneMan object.
// Arguments:       None.
// Return value:    None.

    void Destroy();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetScene
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the currently loaded scene, if any.
// Arguments:       None.
// Return value:    The scene, ownership IS NOT TRANSFERRED!

    Scene * GetScene() const { return m_pCurrentScene; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetSceneDim
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the total dimensions (width and height) of the scene, in pixels.
// Arguments:       None.
// Return value:    A Vector describing the scene dimensions.

    Vector GetSceneDim() const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetSceneWidth
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the total width of the scene, in pixels.
// Arguments:       None.
// Return value:    An int describing the scene width.

    int GetSceneWidth() const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetSceneHeight
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the total height of the scene, in pixels.
// Arguments:       None.
// Return value:    An int describing the scene width.

    int GetSceneHeight() const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetMaterialPalette
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets access to the whole material palette array of 256 entries.
// Arguments:       None.
// Return value:    A const reference to the material palette array.

	const std::array<Material *, c_PaletteEntriesNumber> & GetMaterialPalette() const { return m_apMatPalette; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetMaterial
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets a specific material by name. Ownership is NOT transferred!
// Arguments:       The string name of the Material to get.
// Return value:    A pointer to the requested material, or 0 if no material with that
//                  name was found.

    Material const * GetMaterial(const std::string &matName);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetMaterialFromID
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets a specific material from the material palette. Ownership is NOT
//                  transferred!
// Arguments:       The unsigned char index specifying screen material to get (0-255).
// Return value:    A reference to the requested material. OWNERSHIP IS NOT TRANSFERRED!

    Material const * GetMaterialFromID(unsigned char screen) { return screen >= 0 && screen < c_PaletteEntriesNumber && m_apMatPalette[screen] ?  m_apMatPalette[screen] : m_apMatPalette[g_MaterialAir]; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SceneWrapsX
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether the scene wraps its scrolling around the X axis.
// Arguments:       None.
// Return value:    Whether the scene wraps around the X axis or not.

    bool SceneWrapsX() const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SceneWrapsY
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether the scene wraps its scrolling around the Y axis.
// Arguments:       None.
// Return value:    Whether the scene wraps around the Y axis or not.

    bool SceneWrapsY() const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetOffset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the offset (scroll position) of the terrain.
// Arguments:       None.
// Return value:    A Vector describing the offset (scroll) of the terrain in pixels.

    Vector GetOffset(int screen = 0) const { return m_Offset[screen]/* - m_DeltaOffset*/; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetOffsetX
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the horizontal offset (scroll position) of the terrain.
// Arguments:       None.
// Return value:    A long describing the horizontal offset (scroll) of the terrain in
//                  pixels.

    long GetOffsetX(int screen = 0) const { return m_Offset[screen].m_X; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetOffsetY
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the vertical offset (scroll position) of the terrain.
// Arguments:       None.
// Return value:    A long describing the vertical offset (scroll) of the terrain in
//                  pixels.

    long GetOffsetY(int screen = 0) const { return m_Offset[screen].m_Y; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetDeltaOffset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the difference in current offset and that of the Update() before.
// Arguments:       None.
// Return value:    A Vector describing the delta offset in pixels.

    const Vector & GetDeltaOffset(int screen = 0) const { return m_DeltaOffset[screen]; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetScreenOcclusion
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the amount that a specific screen is occluded by a GUI panel or
//                  something of the sort. This will affect how the scroll target
//                  translates into the offset of the screen, in order to keep the target
//                  centered on the screen.
// Arguments:       Which screen you want to get the occlusion value of.
// Return value:    A Vector that indicates the amount of occlusion of the screen.

    Vector & GetScreenOcclusion(int screen = 0) { return m_ScreenOcclusion[screen]; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetTerrain
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the SLTerrain, or 0 if no scene is loaded.
// Arguments:       None.
// Return value:    A pointer to the SLTerrain. Ownership is NOT transferred!

    SLTerrain * GetTerrain();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetMOColorBitmap
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the bitmap of the intermediary collection SceneLayer that all
//                  MovableObject:s draw themselves onto before it itself gets drawn onto
//                  the screen back buffer.
// Arguments:       None.
// Return value:    A BITMAP pointer to the MO bitmap. Ownership is NOT transferred!

    BITMAP * GetMOColorBitmap() const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetDebugBitmap
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the bitmap of the SceneLayer that debug graphics is drawn onto.
//                  Will only return valid BITMAP if building with DEBUG_BUILD.
// Arguments:       None.
// Return value:    A BITMAP pointer to the debug bitmap. Ownership is NOT transferred!

    BITMAP * GetDebugBitmap() const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetMOIDBitmap
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the bitmap of the SceneLayer that all MovableObject:s draw thir
//                  current (for the frame only!) MOID's onto.
// Arguments:       None.
// Return value:    A BITMAP pointer to the MO bitmap. Ownership is NOT transferred!

    BITMAP * GetMOIDBitmap() const;

// TEMP!
//////////////////////////////////////////////////////////////////////////////////////////
// Method:          MOIDClearCheck
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes sure the MOID bitmap layer is completely of NoMOID color.
//                  If found to be not, dumps MOID layer and the FG actor color layer for
//                  debugging.
// Arguments:       None.
// Return value:    Was it clear?

    bool MOIDClearCheck();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetLayerDrawMode
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the current drawing mode of the SceneMan.
// Arguments:       None.
// Return value:    The current layer draw mode, see the LayerDrawMode enumeration for the
//                  different possible mode settings.

    int GetLayerDrawMode() const { return m_LayerDrawMode; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetTerrMatter
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets a specific pixel from the total material representation of
//                  this Scene. LockScene() must be called before using this method.
// Arguments:       The X and Y coordinates of screen material pixel to get.
// Return value:    An unsigned char specifying the requested pixel's material index.

    unsigned char GetTerrMatter(int pixelX, int pixelY);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetMOIDPixel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets a MOID from pixel coordinates in the Scene. LockScene() must be
//                  called before using this method.
// Arguments:       The X and Y coordinates of screen Scene pixel to get the MO from.
// Return value:    The MOID currently at the specified pixel location.

    MOID GetMOIDPixel(int pixelX, int pixelY);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetGlobalAcc
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the global acceleration (in m/s^2) that is applied to all movable
//                  objects' velocities during every frame. Typically models gravity.
// Arguments:       None.
// Return value:    A Vector describing the global acceleration.

    Vector GetGlobalAcc() const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetOzPerKg
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets how many Ounces there are in a metric Kilogram
// Arguments:       None.
// Return value:    A float describing the Oz/Kg ratio.

    float GetOzPerKg() const { return 35.27396; }
        
    
//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetKgPerOz
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets how many metric Kilograms there are in an Ounce.
// Arguments:       None.
// Return value:    A float describing the Kg/Oz ratio.

    float GetKgPerOz() const { return 0.02834952; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetLayerDrawMode
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the drawing mode of the SceneMan, to easily view what's going on
//                  in the different internal SceneLayer:s.
// Arguments:       The layer mode to draw in, see the LayerDrawMode enumeration for the
//                  different possible settings.
// Return value:    None.

    void SetLayerDrawMode(int mode) { m_LayerDrawMode = mode; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetOffset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the offset (scroll position) of the terrain.
// Arguments:       Two longs that specify the new offset values.
//                  Which screen you want to set the offset of.
// Return value:    None.

    void SetOffset(const long offsetX, const long offsetY, int screen = 0);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetOffset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the offset (scroll position) of the terrain.
// Arguments:       A Vector that specifies the new offset.
//                  Which screen you want to set the offset of.
// Return value:    None.

    void SetOffset(const Vector &offset, int screen = 0) { m_Offset[screen] = offset.GetFloored(); CheckOffset(screen); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetOffsetX
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the horizontal offset (scroll position) of the terrain.
// Arguments:       A long that specifies the new horizontal offset value.
//                  Which screen you want to set the offset of.
// Return value:    None.

    void SetOffsetX(const long offsetX, int screen = 0) { m_Offset[screen].m_X = offsetX; CheckOffset(screen); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetOffsetY
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the vertical offset (scroll position) of the terrain.
// Arguments:       A long that specifies the new vertical offset value.
//                  Which screen you want to set the offset of.
// Return value:    None.

    void SetOffsetY(const long offsetY, int screen = 0) { m_Offset[screen].m_Y = offsetY; CheckOffset(screen); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetScroll
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the offset (scroll position) of the terrain to center on
//                  specific world coordinates. If the coordinate to center on is close
//                  to the terrain border edges, the view will not scroll outside the
//                  borders.
// Arguments:       A Vector that specifies the coordinates to center the terrain scroll
//                  on.
//                  Which screen you want to set the offset of.
// Return value:    None.

    void SetScroll(const Vector &center, int screen = 0);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetScreenTeam
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the team associated with a specific screen
// Arguments:       Which screen you want to set the team of.
//                  The team to set it to.
// Return value:    None.

    void SetScreenTeam(int screen, int team) { m_ScreenTeam[screen] = team; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetScreenOcclusion
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the amount that a specific screen is occluded by a GUI panel or
//                  something of the sort. This will affect how the scroll target
//                  translates into the offset of the screen, in order to keep the target
//                  centered on the screen.
// Arguments:       A Vector that specifies the amount of occlusion of the screen.
//                  Which screen you want to set the occlusion of.
// Return value:    None.

    void SetScreenOcclusion(const Vector &occlusion, int screen = 0) { m_ScreenOcclusion[screen] = occlusion; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetScrollTarget
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Interpolates a smooth scroll of the view from wherever it is now,
//                  towards centering on a new scroll target over time.
// Arguments:       The new target vector in *scene coordinates*.
//                  The normalized speed at screen the view scrolls. 0 being no movement,
//                  and 1.0 being instant movement to the target in one frame.
//                  Whether the target was wrapped around the scene this frame or not.
//                  Which screen you want to set the offset of.
// Return value:    None.

    void SetScrollTarget(const Vector &targetCenter,
                         float speed = 0.1,
                         bool targetWrapped = false,
                         int screen = 0);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetScreenTeam
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the team associated with a specific screen
// Arguments:       Which screen you want to get the team of.
// Return value:    The team associated with that team.

    int GetScreenTeam(int screen) { return m_ScreenTeam[screen]; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetScrollTarget
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the currently set scroll target, screen is where the center of the
//                  specific screen is trying to line up with.
// Arguments:       Which screen to get the target for.
// Return value:    Current target vector in *scene coordinates*.

	const Vector & GetScrollTarget(int screen = 0) const;



//////////////////////////////////////////////////////////////////////////////////////////
// Method:          TargetDistanceScalar
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Calculates a scalar of how distant a certain point in the world is
//                  from the currently closest scroll target of all active screens.
// Arguments:       Which world coordinate point to check distance to/from.
// Return value:    A normalized scalar representing the distance between the closest
//                  scroll target of all active screens, to the passed in point. 0 means
//                  it's the point is within half a screen's width of the target, and
//                  1.0 means it's on the clear opposite side of the scene.

    float TargetDistanceScalar(Vector point);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CheckOffset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes sure the current offset won't create a view of outside the scene.
//                  If that is found to be the case, the offset is corrected so that the
//                  view rectangle is as close to the old offset as possible, but still
//                  entirely within the scene world.
// Arguments:       Which screen you want to set the offset of.
// Return value:    None.

    void CheckOffset(int screen = 0);

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsScrolling
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether the view is currently doing a scroll interpolation
//                  animation, or not.
// Arguments:       None.
// Return value:    Whether currently scrolling or not.

    bool IsScrolling() { return m_ScrollTimeLeft != 0; }
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          LockScene
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Locks all dynamic internal scene bitmaps so that manipulaitons of the
//                  scene's color and matter representations can take place.
//                  Doing it in a separate method like this is more efficient because
//                  many bitmap manipulaitons can be performed between a lock and unlock.
//                  UnlockScene() should always be called after accesses are completed.
// Arguments:       None.
// Return value:    None.

    void LockScene();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          UnlockScene
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Unlocks the scene's bitmaps and prevents access to display memory.
//                  Doing it in a separate method like this is more efficient because
//                  many bitmap accesses can be performed between a lock and an unlock.
//                  UnlockScene() should only be called after LockScene().
// Arguments:       None.
// Return value:    None.

    void UnlockScene();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SceneIsLocked
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether the entire scene is currently locked or not.
// Arguments:       None.
// Return value:    Whether the entire scene is currently locked or not.

    bool SceneIsLocked() const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          RegisterMOIDDrawing
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Registers an area of the MOID layer to be cleared upon finishing this
//                  sim update. Should be done every time anything is drawn the MOID layer.
// Arguments:       The coordinates of the new area on the MOID layer to clear upon the
//                  end of this sim update.
// Return value:    None.

    void RegisterMOIDDrawing(int left, int top, int right, int bottom) { m_MOIDDrawings.push_back(IntRect(left, top, right, bottom)); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          RegisterMOIDDrawing
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Registers an area of the MOID layer to be cleared upon finishing this
//                  sim update. Should be done every time anything is drawn the MOID layer.
// Arguments:       The center coordinates and a radius around it of the new area on the
//                  MOID layer to clear upon the end of this sim update.
// Return value:    None.
  
    void RegisterMOIDDrawing(const Vector &center, float radius);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ClearAllMOIDDrawings
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all registered drawn areas of the MOID layer to the g_NoMOID
//                  color and clears the registrations too. Should be done each sim update.
// Arguments:       None.
// Return value:    None.

    void ClearAllMOIDDrawings();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ClearMOIDRect
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Resets a specific rectangle of the scene's MOID layer to not contain
//                  any MOID data anymore. Sets it all to NoMOID. Will take care of wrapping.
// Arguments:       The coordinates of the rectangle to be reset to NoMOID.
// Return value:    None.

    void ClearMOIDRect(int left, int top, int right, int bottom);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          WillPenetrate
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Test whether a pixel of the scene would be knocked loose and
//                  turned into a MO by another particle of a certain material going at a
//                  certain velocity. Scene needs to be locked to do this!
// Arguments:       The X and Y coords of the scene pixel that is collided with.
//                  The velocity of the incoming particle.
//                  The mass of the incoming particle.
// Return value:    A bool indicating wether the scene pixel would be knocked loose or
//                  not. If the pixel location specified happens to be of the air
//                  material (0) false will be returned here.

    bool WillPenetrate(const int posX,
                       const int posY,
                       const Vector &velocity,
                       const float mass) { return WillPenetrate(posX, posY, velocity * mass); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          WillPenetrate
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Test whether a pixel of the scene would be knocked loose and
//                  turned into a MO by a certian impulse force. Scene needs to be locked
//                  to do this!
// Arguments:       The X and Y coords of the scene pixel that is collided with.
//                  The impulse force vector, in Kg * m/s.
// Return value:    A bool indicating wether the scene pixel would be knocked loose or
//                  not. If the pixel location specified happens to be of the air
//                  material (0) false will be returned here.

    bool WillPenetrate(const int posX,
                       const int posY,
                       const Vector &impulse);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          TryPenetrate
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Calculate whether a pixel of the scene would be knocked loose and
//                  turned into a MO by another particle of a certain material going at a
//                  certain velocity. If so, the incoming particle will knock loose the
//                  specified pixel in the scene and momentarily take its place.
//                  Scene needs to be locked to do this!
// Arguments:       The X and Y coord of the scene pixel that is to be collided with.
//                  The impulse force exerted on the terrain pixel. If this magnitude
//                  exceeds the strength threshold of the material of the terrain pixel
//                  hit, the terrain pixel will be knocked loose an turned into an MO.
//                  The velocity of the the point hitting the terrain here.
//                  A float reference screen will be set to the factor with screen to
//                  multiply the collision velocity to get the resulting retardation 
//                  (negative acceleration) that occurs when a penetration happens.
//                  The normalized probability ratio between 0.0 and 1.0 that determines
//                  the chance of a penetration to remove a pixel from the scene and
//                  thus replace it with and air pixel. 1.0 = always, 0.0 = never.
//                  How many consecutive penetrations in a row immediately before this try.
//					The size of the area to look for orphaned terrain elements.
//					Max area or orphaned area to remove.
//					Orphan area removal trigger rate.
// Return value:    A bool indicating wether the scene pixel was knocked loose or not.
//                  If the pixel location specified happens to be of the air material (0)
//                  false will be returned here.

    bool TryPenetrate(const int posX,
                      const int posY,
                      const Vector &impulse,
                      const Vector &velocity,
                      float &retardation,
                      const float airRatio,
                      const int numPenetrations = 0,
					  const int removeOrphansRadius = 0,
					  const int removeOrphansMaxArea = 0,
					  const float removeOrphansRate = 0.0);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          RemoveOrphans
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns the area of an orphaned region at specified coordinates. 
// Arguments:       Coordinates to check for region, whether the orphaned region should be converted into MOPixels and region removed.
//					Area of orphaned object calculated during recursve function call to check if we're out of limits
//					Size of the are to look for orphaned objects
//					Max area of orphaned object to remove
//					Whether to actually remove orphaned pixels or not
//					Whether to clear internal terrain tracking bitmap or not
// Return value:    The area of orphaned region at posX,posY

    int RemoveOrphans(int posX, int posY, int radius, int maxArea, bool remove = false);

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          RemoveOrphans
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns the area of an orphaned region at specified coordinates. 
// Arguments:       Coordinates to check for region, whether the orphaned region should be converted into MOPixels and region removed.
//					Coordinates of initial terrain penetration to check, which serves as a center of orphaned object detection.
//					Area of orphaned object calculated during recursve function call to check if we're out of limits
//					Size of the are to look for orphaned objects
//					Max area of orphaned object to remove
//					Whether to actually remove orphaned pixels or not
//					Whether to clear internal terrain tracking bitmap or not
// Return value:    The area of orphaned region at posX,posY

    int RemoveOrphans(int posX,
					  int posY,
					  int centerPosX,
					  int centerPosY,
					  int accumulatedArea,
					  int radius,
					  int maxArea,
					  bool remove = false);

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          MakeAllUnseen
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets one team's view of the scene to be unseen, using a generated map
//                  of a specific resolution chunkiness.
// Arguments:       The dimensions of the pixels that should make up the unseen layer.
//                  The team we're talking about.
// Return value:    None.

    void MakeAllUnseen(Vector pixelSize, const int team);


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
// Arguments:       The path to the bitmap to use as the unseen layer.
//                  Which team we're talking about.
// Return value:    Whether the loading was successful or not.

    bool LoadUnseenLayer(std::string bitmapPath, const int team);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AnythingUnseen
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tells whether a team has anything still unseen on the scene.
// Arguments:       The team we're talking about.
// Return value:    A bool indicating whether that team has anyhting yet unseen.

    bool AnythingUnseen(const int team);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetUnseenResolution
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows what the resolution factor of the unseen map to the entire Scene
//                  is, in both axes.
// Arguments:       The team we're talking about.
// Return value:    A vector witht he factors in each element representing the factors.

    Vector GetUnseenResolution(const int team) const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsUnseen
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Checks whether a pixel is in an unseen area on of a specific team.
// Arguments:       The X and Y coords of the scene pixel that is to be checked.
//                  The team we're talking about.
// Return value:    A bool indicating whether that point is yet unseen.

    bool IsUnseen(const int posX, const int posY, const int team);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          RevealUnseen
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reveals a pixel on the unseen map for a specific team, if there is any.
// Arguments:       The X and Y coord of the scene pixel that is to be revealed.
//                  The team to reveal for.
// Return value:    A bool indicating whether there was an unseen pixel revealed there.

    bool RevealUnseen(const int posX, const int posY, const int team);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          RestoreUnseen
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Hides a pixel on the unseen map for a specific team, if there is any.
// Arguments:       The X and Y coord of the scene pixel that is to be revealed.
//                  The team to hide for.
// Return value:    A bool indicating whether there was a seen pixel hidden there.

    bool RestoreUnseen(const int posX, const int posY, const int team);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          RevealUnseenBox
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reveals a box on the unseen map for a specific team, if there is any.
// Arguments:       The X and Y coords of the upper left corner of the box to be revealed.
//                  The width and height of the box to be revealed, in scene units (pixels)
//                  The team to reveal for.
// Return value:    None.

    void RevealUnseenBox(const int posX, const int posY, const int width, const int height, const int team);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          RestoreUnseenBox
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Restores a box on the unseen map for a specific team, if there is any.
// Arguments:       The X and Y coords of the upper left corner of the box to be revealed.
//                  The width and height of the box to be restored, in scene units (pixels)
//                  The team to restore for.
// Return value:    None.

    void RestoreUnseenBox(const int posX, const int posY, const int width, const int height, const int team);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CastUnseenRay
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Traces along a vector and reveals or hides pixels on the unseen layer of a team
//                  as long as the accumulated material strengths traced through the terrain
//                  don't exceed a specific value.
// Arguments:       The team to see for.
//                  The starting position.
//                  The vector to trace along.
//                  A Vector that will be set to the position of where the sight ray was
//                  terminated. If it reached the end, it will be set to the end of the ray.
//                  The material strength limit where 
//                  For every pixel checked along the line, how many to skip between them
//                  for optimization reasons. 0 = every pixel is checked.
//					Whether the ray should reveal or restore unseen layer
// Return value:    Whether any unseen pixels were revealed as a result of this seeing.

	bool CastUnseenRay(int team, const Vector &start, const Vector &ray, Vector &endPos, int strengthLimit, int skip, bool reveal);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CastSeeRay
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Traces along a vector and reveals pixels on the unseen layer of a team
//                  as long as the accumulated material strengths traced through the terrain
//                  don't exceed a specific value.
// Arguments:       The team to see for.
//                  The starting position.
//                  The vector to trace along.
//                  A Vector that will be set to the position of where the sight ray was
//                  terminated. If it reached the end, it will be set to the end of the ray.
//                  The material strength limit where 
//                  For every pixel checked along the line, how many to skip between them
//                  for optimization reasons. 0 = every pixel is checked.
// Return value:    Whether any unseen pixels were revealed as a result of this seeing.

    bool CastSeeRay(int team, const Vector &start, const Vector &ray, Vector &endPos, int strengthLimit, int skip = 0);

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CastUnseeRay
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Traces along a vector and hides pixels on the unseen layer of a team
//                  as long as the accumulated material strengths traced through the terrain
//                  don't exceed a specific value.
// Arguments:       The team to see for.
//                  The starting position.
//                  The vector to trace along.
//                  A Vector that will be set to the position of where the sight ray was
//                  terminated. If it reached the end, it will be set to the end of the ray.
//                  The material strength limit where 
//                  For every pixel checked along the line, how many to skip between them
//                  for optimization reasons. 0 = every pixel is checked.
// Return value:    Whether any unseen pixels were revealed as a result of this seeing.

	bool CastUnseeRay(int team, const Vector &start, const Vector &ray, Vector &endPos, int strengthLimit, int skip = 0);




//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CastMaterialRay
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Traces along a vector and gets the location of the first encountered
//                  pixel of a specific material in the terrain.
// Arguments:       The starting position.
//                  The vector to trace along.
//                  The material ID to look for.
//                  A reference to the vector screen will be filled out with the absolute
//                  location of the found terrain pixel of the above material.
//                  For every pixel checked along the line, how many to skip between them
//                  for optimization reasons. 0 = every pixel is checked.
//                  Whetehr the ray should wrap around the scene if it crosses a seam.
// Return value:    Whether the material was found along the ray. If not, the fourth
//                  parameter will not have been altered (and may still not be 0!)

    bool CastMaterialRay(const Vector &start, const Vector &ray, unsigned char material, Vector &result, int skip = 0, bool wrap = true);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CastMaterialRay
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Traces along a vector and returns how far along that ray there is an
//                  encounter with a pixel of a specific material in the terrain.
// Arguments:       The starting position.
//                  The vector to trace along.
//                  The material ID to look for.
//                  For every pixel checked along the line, how many to skip between them
//                  for optimization reasons. 0 = every pixel is checked.
// Return value:    How far along, in pixel units, the ray the material pixel was encountered.
//                  If no pixel of the right material was found, < 0 is returned.

    float CastMaterialRay(const Vector &start, const Vector &ray, unsigned char material, int skip = 0);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CastNotMaterialRay
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Traces along a vector and gets the location of the first encountered
//                  pixel that is NOT of a specific material in the scene's terrain.
// Arguments:       The starting position.
//                  The vector to trace along.
//                  The material ID to find something OTHER than.
//                  A reference to the vector screen will be filled out with the absolute
//                  location of the found terrain pixel of the above material.
//                  For every pixel checked along the line, how many to skip between them
//                  for optimization reasons. 0 = every pixel is checked.
//                  Whether to check for MO layer collisions as well, not just terrain.
// Return value:    Whether the a pixel other than the material was found along the ray.
//                  If not, the fourth parameter will not have been altered (and may still not be 0!)

    bool CastNotMaterialRay(const Vector &start, const Vector &ray, unsigned char material, Vector &result, int skip = 0, bool checkMOs = false);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CastNotMaterialRay
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Traces along a vector and returns how far along that ray there is an
//                  encounter with a pixel of OTHER than a specific material in the terrain.
// Arguments:       The starting position.
//                  The vector to trace along.
//                  The material ID to find something OTHER than.
//                  For every pixel checked along the line, how many to skip between them
//                  for optimization reasons. 0 = every pixel is checked.
//                  Whether to check for MO layer collisions as well, not just terrain.
// Return value:    How far along, in pixel units, the ray the pixel of any other material
//                  was encountered. If no pixel of the right material was found, < 0 is returned.

    float CastNotMaterialRay(const Vector &start, const Vector &ray, unsigned char material, int skip = 0, bool checkMOs = false);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CastStrengthSumRay
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Traces along a vector and returns how the sum of all encountered pixels'
//                  material strength values. This will take wrapping into account.
// Arguments:       The starting position.
//                  The ending position.
//                  For every pixel checked along the line, how many to skip between them
//                  for optimization reasons. 0 = every pixel is checked.
//                  A material ID to ignore, IN ADDITION to Air.
// Return value:    The sum of all encountered pixels' material strength vales. So if it was
//                  all Air, then 0 is returned (Air's strength value is 0).

    float CastStrengthSumRay(const Vector &start, const Vector &end, int skip = 0, unsigned char ignoreMaterial = g_MaterialAir);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CastMaxStrengthRay
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Traces along a vector and returns the strongest of all encountered pixels'
//                  material strength values exept doors.
//                  This will take wrapping into account.
// Arguments:       The starting position.
//                  The ending position.
//                  For every pixel checked along the line, how many to skip between them
//                  for optimization reasons. 0 = every pixel is checked.
// Return value:    The max of all encountered pixels' material strength vales. So if it was
//                  all Air, then 0 is returned (Air's strength value is 0).

    float CastMaxStrengthRay(const Vector &start, const Vector &end, int skip);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CastStrengthRay
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Traces along a vector and shows where along that ray there is an
//                  encounter with a pixel of a material with strength more than or equal
//                  to a specific value.
// Arguments:       The starting position.
//                  The vector to trace along.
//                  The strength value of screen any found to be equal or more than will
//                  terminate the ray.
//                  A reference to the vector screen will be filled out with the absolute
//                  location of the found terrain pixel of less than or equal to above strength.
//                  For every pixel checked along the line, how many to skip between them
//                  for optimization reasons. 0 = every pixel is checked.
//                  A material ID to ignore, IN ADDITION to Air.
//                  Whetehr the ray should wrap around the scene if it crosses a seam.
// Return value:    Whether a material of equal or more strength was found along the ray.
//                  If not, the fourth parameter have been set to last position of the ray.

    bool CastStrengthRay(const Vector &start, const Vector &ray, float strength, Vector &result, int skip = 0, unsigned char ignoreMaterial = g_MaterialAir, bool wrap = true);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CastWeaknessRay
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Traces along a vector and shows where along that ray there is an
//                  encounter with a pixel of a material with strength less than or equal
//                  to a specific value.
// Arguments:       The starting position.
//                  The vector to trace along.
//                  The strength value of screen any found to be equal or less than will
//                  terminate the ray.
//                  A reference to the vector screen will be filled out with the absolute
//                  location of the found terrain pixel of less than or equal to above strength.
//                  For every pixel checked along the line, how many to skip between them
//                  for optimization reasons. 0 = every pixel is checked.
//                  Whetehr the ray should wrap around the scene if it crosses a seam.
// Return value:    Whether a material of equal or less strength was found along the ray.
//                  If not, the fourth parameter have been set to last position of the ray.

    bool CastWeaknessRay(const Vector &start, const Vector &ray, float strength, Vector &result, int skip = 0, bool wrap = true);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CastMORay
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Traces along a vector and returns MOID of the first non-ignored
//                  non-NoMOID MO encountered. If a non-air terrain pixel is encountered
//                  first, g_NoMOID will be returned.
// Arguments:       The starting position.
//                  The vector to trace along.
//                  An MOID to ignore. Any child MO's of this MOID will also be ignored.
//                  To enable ignoring of all MOIDs associated with an object of a specific
//                  team which also has team ignoring enabled itself.
//                  A specific material ID to ignore hits with.
//                  Whether to ignore all terrain hits or not.
//                  For every pixel checked along the line, how many to skip between them
//                  for optimization reasons. 0 = every pixel is checked.
// Return value:    The MOID of the hit non-ignored MO, or g_NoMOID if terrain or no MO was hit.

    MOID CastMORay(const Vector &start, const Vector &ray, MOID ignoreMOID = g_NoMOID, int ignoreTeam = Activity::NoTeam, unsigned char ignoreMaterial = 0, bool ignoreAllTerrain = false, int skip = 0);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CastFindMORay
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Traces along a vector and shows where a specific MOID has been found.
// Arguments:       The starting position.
//                  The vector to trace along.
//                  An MOID to find. Any child MO's of this MOID will also be found. ------------ ???
//                  A reference to the vector screen will be filled out with the absolute
//                  location of the found MO pixel of the above MOID.
//                  A specific material ID to ignore hits with.
//                  Whether to ignore all terrain hits or not.
//                  For every pixel checked along the line, how many to skip between them
//                  for optimization reasons. 0 = every pixel is checked.
// Return value:    Whether the target MOID was found along the ray or not.

    bool CastFindMORay(const Vector &start, const Vector &ray, MOID targetMOID, Vector &resultPos, unsigned char ignoreMaterial = 0, bool ignoreAllTerrain = false, int skip = 0);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CastObstacleRay
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Traces along a vector and returns the length of how far the trace went
//                  without hitting any non-ignored terrain material or MOID at all.
// Arguments:       The starting position.
//                  The vector to trace along.
//                  A reference to the vector screen will be filled out with the absolute
//                  location of the first obstacle, or the end of the ray if none was hit.
//                  A reference to the vector screen will be filled out with the absolute
//                  location of the last free position before hitting an obstacle, or the
//                  end of the ray if none was hit. This is only altered if thre are any
//                  free pixels encountered.
//                  An MOID to ignore. Any child MO's of this MOID will also be ignored.
//                  To enable ignoring of all MOIDs associated with an object of a specific
//                  team which also has team ignoring enabled itself.
//                  A specific material ID to ignore hits with.
//                  For every pixel checked along the line, how many to skip between them
//                  for optimization reasons. 0 = every pixel is checked.
// Return value:    How far along, in pixel units, the ray the pixel of any obstacle was
//                  encountered. If no pixel of the right material was found, < 0 is returned.
//                  If an obstacle on the starting position was encountered, 0 is returned.

    float CastObstacleRay(const Vector &start, const Vector &ray, Vector &obstaclePos, Vector &freePos, MOID ignoreMOID = g_NoMOID, int ignoreTeam = Activity::NoTeam, unsigned char ignoreMaterial = 0, int skip = 0);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetLastRayHitPos
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the abosulte pos of where the last cast ray hit somehting.
// Arguments:       None.
// Return value:    A vector witht he absoltue pos of where the last ray cast hit somehting.

    const Vector & GetLastRayHitPos() { return m_LastRayHitPos; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          FindAltitude
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Calculates the altitide of a certain point above the terrain, measured
//                  in pixels.
// Arguments:       The max altitude you care to check for. 0 Means check the whole scene's height.
//                  The accuracy within screen measurement is acceptable. Higher number
//                  here means less calculation.
// Return value:    The altitude over the terrain, in pixels.

    float FindAltitude(const Vector &from, int max = 0, int accuracy = 0);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          OverAltitude
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Calculates the altitide of a certain point above the terrain, measured
//                  in pixels, and then tells if that point is over a certain value.
// Arguments:       The altitude threshold you want to check for.
//                  The accuracy within screen measurement is acceptable. Higher number
//                  here means less costly.
// Return value:    Whether the point is over the threshold altitude or not.

    bool OverAltitude(const Vector &point, int threshold, int accuracy = 0);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          MovePointToGround
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Takes an arbitrary point in the air and calculates it to be straight
//                  down at a certain maximum distance from the ground.
// Arguments:       The point to start from. Should be in the air, or the same point will
//                  be returned (null operation)
//                  The max altitude in px you want the point to be above the ground.
//                  The accuracy within screen measurement is acceptable. Higher number
//                  here means less calculation.
// Return value:    The new point screen is no higher than accuracy + max altitude over
//                  the terrain.

    Vector MovePointToGround(const Vector &from, int maxAltitude = 0, int accuracy = 0);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          StructuralCalc
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Calculates the structural integrity of the Terrain during a set time
//                  and turns structurally unsound areas into MovableObject:s.
// Arguments:       The amount of time in ms to use for these calculations this frame.
// Return value:    None.

    void StructuralCalc(unsigned long calcTime);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsWithinBounds
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns whether the integer coordinates passed in are within the
//                  bounds of the current Scene, considering its wrapping.
// Arguments:       Int coordinates.
//                  A margin 
// Return value:    Whether within bounds or not, considering wrapping.

    bool IsWithinBounds(const int pixelX, const int pixelY, const int margin = 0);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ForceBounds
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Wraps or bounds a position coordinate if it is off bounds of the
//                  Scene, depending on the wrap settings of this Scene.
// Arguments:       The X and Y coordinates of the position to wrap, if needed.
// Return value:    Whether wrapping was performed or not. (Does not report on bounding)

    bool ForceBounds(int &posX, int &posY);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ForceBounds
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Wraps or bounds a position coordinate if it is off bounds of the
//                  Scene, depending on the wrap settings of this Scene.
// Arguments:       The vector coordinates of the position to wrap, if needed.
// Return value:    Whether wrapping was performed or not. (Does not report on bounding)

    bool ForceBounds(Vector &pos);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          WrapPosition
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Only wraps a position coordinate if it is off bounds of the Scene
//                  and wrapping in the corresponding axes are turned on.
// Arguments:       The X and Y coordinates of the position to wrap, if needed.
// Return value:    Whether wrapping was performed or not.

    bool WrapPosition(int &posX, int &posY);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          WrapPosition
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Only wraps a position coordinate if it is off bounds of the Scene
//                  and wrapping in the corresponding axes are turned on.
// Arguments:       The vector coordinates of the position to wrap, if needed.
// Return value:    Whether wrapping was performed or not.

    bool WrapPosition(Vector &pos);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SnapPosition
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns a position snapped to the current scene grid.
// Arguments:       The vector coordinates of the position to snap.
//                  Whether to actually snap or not. This is useful for cleaner toggle code.
// Return value:    The new snapped position.

    Vector SnapPosition(const Vector &pos, bool snap = true);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ShortestDistance
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Calculates the shortest distance between two points in scene
//                  coordinates, taking into account all wrapping and out of bounds of the
//                  two points.
// Arguments:       The two Vector coordinates of the two positions to find the shortest
//                  distance between.
//                  Whether to check if the passed in points are outside the scene, and to
//                  wrap them if they are.
// Return value:    The resulting vector screen shows the shortest distance, spanning over
//                  wrapping borders etc. Basically the ideal pos2 - pos1.

    Vector ShortestDistance(Vector pos1, Vector pos2, bool checkBounds = false);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ShortestDistanceX
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Calculates the shortest distance between two x values in scene
//                  coordinates, taking into account all wrapping and out of bounds of the
//                  two values.
// Arguments:       The X coordinates of the two values to find the shortest distance between.
//                  Whether to check if the passed in points are outside the scene, and to
//                  wrap them if they are.
//                  Whether to constrain the distance to only be in a certain direction:
//                  0 means no constraint, < 0 means only look in the negative dir, etc.
//                  If the scene doesn't wrap in the constraint's direction, the constraint
//                  will be ignored.
// Return value:    The resulting X value screen shows the shortest distance, spanning over
//                  wrapping borders etc. Basically the ideal val2 - val1.

    float ShortestDistanceX(float val1, float val2, bool checkBounds = false, int direction = 0);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ShortestDistanceY
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Calculates the shortest distance between two Y values in scene
//                  coordinates, taking into account all wrapping and out of bounds of the
//                  two values.
// Arguments:       The Y coordinates of the two values to find the shortest distance between.
//                  Whether to check if the passed in points are outside the scene, and to
//                  wrap them if they are.
//                  Whether to constrain the distance to only be in a certain direction:
//                  0 means no constraint, < 0 means only look in the negative dir, etc.
//                  If the scene doesn't wrap in the constraint's direction, the constraint
//                  will be ignored.
// Return value:    The resulting Y value screen shows the shortest distance, spanning over
//                  wrapping borders etc. Basically the ideal val2 - val1.

    float ShortestDistanceY(float val1, float val2, bool checkBounds = false, int direction = 0);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ObscuredPoint
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tells whether a point on the scene is obscured by MOID or Terrain
//                  non-air material.
// Arguments:       The point on the scene to check.
//                  Wheter to also check for unseen areas of a specific team. -1 means
//                  don't check this.
// Return value:    Whether that point is obscured/obstructed or not.

    bool ObscuredPoint(Vector &point, int team = -1) { return ObscuredPoint(point.GetFloorIntX(), point.GetFloorIntY());  }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ObscuredPoint
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tells whether a point on the scene is obscured by MOID or Terrain
//                  non-air material.
// Arguments:       The point on the scene to check.
//                  Wheter to also check for unseen areas of a specific team. -1 means
//                  don't check this.
// Return value:    Whether that point is obscured/obstructed or not.

    bool ObscuredPoint(int x, int y, int team = -1);

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SceneRectsIntersect
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tells whether two IntRect:s in the scene intersect, while taking
//                  wrapping into account.
// Arguments:       The point on the scene to check.
// Return value:    Whether that point is obscured/obstructed or not.

    bool SceneRectsIntersect(int x, int y);
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          WrapRect
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Takes a rect and adds all possible wrapped appearances of that rect
//                  to a passed-in list. IF if a passed in rect straddles the seam of a
//                  wrapped scene axis, it will be added twice to the output list. If it
//                  doesn't straddle any seam, it will be only added once.
// Arguments:       The IntRect to check for wrapping of and add to the output list below.
//                  A reference to a list of IntRect:s that will only be added to, never
//                  cleared.
// Return value:    How many additions of the passed in rect was added to the list. 1 if
//                  no wrapping need was detected, up to 4 possible (if straddling both seams)

    int WrapRect(const IntRect &wrapRect, std::list<IntRect> &outputList);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          WrapBox
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Takes a Box and adds all possible scenewrapped appearances of that Box
//                  to a passed-in list. IF if a passed in rect straddles the seam of a
//                  wrapped scene axis, it will be added twice to the output list. If it
//                  doesn't straddle any seam, it will be only added once.
// Arguments:       The IntRect to check for wrapping of and add to the output list below.
//                  A reference to a list of IntRect:s that will only be added to, never
//                  cleared.
// Return value:    How many additions of the passed in Box was added to the list. 1 if
//                  no wrapping need was detected, up to 4 possible (if straddling both seams)

    int WrapBox(const Box &wrapBox, std::list<Box> &outputList);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AddTerrainObject
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Takes TerrainObject and applies it to the terrain
//					OWNERSHIP NOT TRANSFERED!
// Arguments:       TerrainObject to add.
// Return value:    True on success.

	bool AddTerrainObject(TerrainObject *pObject);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AddSceneObject
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Takes any scene object and adds it to the scene in the appropriate way.
//                  If it's a TerrainObject, then it gets applied to the terrain, if it's
//                  an MO, it gets added to the correct type group in MovableMan.
// Arguments:       The SceneObject to add. Ownership IS transferred!
// Return value:    Whether the SceneObject was successfully added or not. Either way,
//                  ownership was transferred. If no success, the object was deleted.

    bool AddSceneObject(SceneObject *pObject);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the state of this SceneMan. Supposed to be done every frame
//                  before drawing.
// Arguments:       Which screen to update for.
// Return value:    None.

    void Update(int screen = 0);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this SceneMan's current graphical representation to a
//                  BITMAP of choice.
// Arguments:       A pointer to a BITMAP to draw on, appropriately sized for the split
//                  screen segment.
//                  The offset into the scene where the target bitmap's upper left corner
//                  is located.
// Return value:    None.

    void Draw(BITMAP *pTargetBitmap, BITMAP *pTargetGUIBitmap,  const Vector &targetPos = Vector(), bool skipSkybox = false, bool skipTerrain = false);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ClearMOColorLayer
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears the color MO layer. Should be done every frame.
// Arguments:       None.
// Return value:    None.

    void ClearMOColorLayer();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ClearMOIDLayer
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears the MOID layer. Should be done every frame.
// Arguments:       None.
// Return value:    None.

    void ClearMOIDLayer();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ClearSeenPixels
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears the list of pixels on the unseen map that have been revealed.
// Arguments:       None.
// Return value:    None.

    void ClearSeenPixels();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AddMaterialCopy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a copy of passed material and stores it into internal vector 
//					to make sure there's only one material owner		
// Arguments:       Material to add.
// Return value:    Pointer to stored material.

    Material * AddMaterialCopy(Material *mat);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          RegisterTerrainChange
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Registers terrain change event for the network server to be then sent to clients.
// Arguments:       x,y - scene coordinates of change, w,h - size of the changed region, 
//					color - changed color for one-pixel events, 
//					back - if true, then background bitmap was changed if false then foreground.
// Return value:    None.

	void RegisterTerrainChange(int x, int y, int w, int h, unsigned char color, bool back);


	//	Struct to register terrain change events
	struct TerrainChange
	{
		int x;
		int y;
		int w;
		int h;
		unsigned char color;
		bool back;
	};

	/// <summary>
	/// Sets the current scene pointer to null
	/// </summary>
	void ClearCurrentScene();

	//////////////////////////////////////////////////////////////////////////////////////////
	// Protected member variable and method declarations

  protected:


    // Default Scene name to load if nothing else is specified
    std::string m_DefaultSceneName;
    // Scene reference to load from, if any. NOT OWNED - a clone of this actually gets loaded
    const Scene *m_pSceneToLoad;
    // Whether to place objects later when loading a clone of the above scene
    bool m_PlaceObjects;
	// Whether to place units and deployments when loading scence
	bool m_PlaceUnits;

    // Current scene being used
    Scene *m_pCurrentScene;
    // Color MO layer
    SceneLayer *m_pMOColorLayer;
    // MovableObject ID layer
    SceneLayer *m_pMOIDLayer;
    // All the areas drawn within on the MOID layer since last Update
    std::list<IntRect> m_MOIDDrawings;

    // Debug layer for seeing cast rays etc
    SceneLayer *m_pDebugLayer;
    // The absolute end position of the last ray cast
    Vector m_LastRayHitPos;
    // The mode we're drawing layers in to the screen
    int m_LayerDrawMode;

    // Material palette stuff
    std::map<std::string, unsigned char> m_MatNameMap;
    // This gets filled with holes, not contigous from 0 onward, but whatever the ini specifies. The Material objects are owned here
	std::array<Material *, c_PaletteEntriesNumber> m_apMatPalette;
    // The total number of added materials so far
    int m_MaterialCount;

	// Non original materials added by inheritance
	std::vector<Material *> m_MaterialCopiesVector;

    // The position of the upper left corner of the view.
    Vector m_Offset[c_MaxScreenCount];
    // The difference in current offset and the Update() before.
    Vector m_DeltaOffset[c_MaxScreenCount];
    // The final offset target of the current scroll interpolation, in scene coordinates!
    Vector m_ScrollTarget[c_MaxScreenCount];
    // The team associated with each screen.
    int m_ScreenTeam[c_MaxScreenCount];
    // The amount screen a screen is occluded or covered by GUI, etc
    Vector m_ScreenOcclusion[c_MaxScreenCount];
    // The normalized speed at screen the view scrolls.
    // 0 being no movement, and 1.0 being instant movement to the target in one frame.
    float m_ScrollSpeed[c_MaxScreenCount];
    // Scroll timer for making scrolling work framerate independently
    Timer m_ScrollTimer[c_MaxScreenCount];
    // Whether the ScrollTarget got wrapped around the world this frame or not.
    bool m_TargetWrapped[c_MaxScreenCount];
    // Keeps track of how many times and in screen directions the wrapping seam has been crossed.
    // This is used fo keeping the background layers' scroll from jumping when wrapping around.
    // X and Y
    int m_SeamCrossCount[c_MaxScreenCount][2];

    // Sound of an unseen pixel on an unseen layer being revealed.
    SoundContainer *m_pUnseenRevealSound;

    bool m_DrawRayCastVisualizations; //!< Whether to visibly draw RayCasts to the Scene debug Bitmap.
    bool m_DrawPixelCheckVisualizations; //!< Whether to visibly draw pixel checks (GetTerrMatter and GetMOIDPixel) to the Scene debug Bitmap.

    // The last screen everything has been updated to
    int m_LastUpdatedScreen;
    // Whether we're in second pass of the structural computations.
    // Second pass is where structurally unsound areas of the Terrain are turned into
    // MovableObject:s.
    bool m_SecondStructPass;
        
    // The Timer that keeps track of how much time there is left for
    // structural calculations each frame.
    Timer m_CalcTimer;

    // The Timer to measure time between cleanings of the color layer of the Terrain.
    Timer m_CleanTimer;
	// Bitmap to look for orphaned regions
	BITMAP * m_pOrphanSearchBitmap;


//////////////////////////////////////////////////////////////////////////////////////////
// Private member variable and method declarations

private:

	static const std::string c_ClassName; //!< A string with the friendly-formatted type name of this object.

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this SceneMan, effectively
//                  resetting the members of this abstraction level only.
// Arguments:       None.
// Return value:    None.

    void Clear();

    
    // Disallow the use of some implicit methods.
	SceneMan(const SceneMan &reference) = delete;
	SceneMan & operator=(const SceneMan &rhs) = delete;

};

} // namespace RTE

#endif // File
