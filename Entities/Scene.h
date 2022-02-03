#ifndef _RTESCENE_
#define _RTESCENE_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            Scene.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Header file for the Scene class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "Entity.h"
#include "SceneMan.h"
#include "ActivityMan.h"
#include "Box.h"
#include "BunkerAssembly.h"

namespace RTE
{

class ContentFile;
class MovableObject;
class PathFinder;


//////////////////////////////////////////////////////////////////////////////////////////
// Class:           Scene
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Contains everything that defines a complete scene.
// Parent(s):       Entity.
// Class history:   08/02/2006 Scene created.

class Scene : public Entity {

	friend struct EntityLuaBindings;

//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable, method and friend function declarations

public:

	SerializableOverrideMethods;
	ClassInfoGetters;

	//Available placed objects sets
	enum PlacedObjectSets
	{
		PLACEONLOAD = 0,
		BLUEPRINT,
		AIPLAN,
		PLACEDSETSCOUNT
	};

    //////////////////////////////////////////////////////////////////////////////////////////
    // Nested class:    Area
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Something to bundle the properties of scene areas together
    // Parent(s):       Serializable.
    // Class history:   07/18/2008 Area created.

    class Area:
        public Serializable
    {

    friend class Scene;
    friend class AreaEditorGUI;
    friend class AreaPickerGUI;

	friend struct EntityLuaBindings;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Public member variable, method and friend function declarations

    public:

		SerializableClassNameGetter;
		SerializableOverrideMethods;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Constructor:     Area
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Constructor method used to instantiate a Area object in system
    //                  memory. Create() should be called before using the object.
    // Arguments:       None.

        Area() { Clear(); Create(); }
        Area(std::string name) { Clear(); m_Name = name; Create(); }
        Area(const Area &reference) { Clear(); Create(reference); }


    //////////////////////////////////////////////////////////////////////////////////////////
    // Virtual method:  Create
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Makes the Area object ready for use.
    // Arguments:       None.
    // Return value:    An error return value signaling sucess or any particular failure.
    //                  Anything below 0 is an error signal.

		int Create() override;


    //////////////////////////////////////////////////////////////////////////////////////////
    // Virtual method:  Create
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Creates a Area to be identical to another, by deep copy.
    // Arguments:       A reference to the Area to deep copy.
    // Return value:    An error return value signaling sucess or any particular failure.
    //                  Anything below 0 is an error signal.

		int Create(const Area &reference);


    //////////////////////////////////////////////////////////////////////////////////////////
    // Virtual method:  Reset
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Resets the entire Serializable, including its inherited members, to their
    //                  default settings or values.
    // Arguments:       None.
    // Return value:    None.

        void Reset() override { Clear(); }


    //////////////////////////////////////////////////////////////////////////////////////////
    // Virtual method:  AddBox
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Adds a Box to this' area coverage.
    // Arguments:       The Box to add. A copy will be made and added.
    // Return value:    Whether the Box was successfully added or not.

		bool AddBox(const Box &newBox);

        /// <summary>
        /// Removes the first Box in the Area that has the same Corner, Width and Height of the passed-in Box.
        /// </summary>
        /// <param name="boxToRemove">A Box whose values are used to determine what Box to remove.</param>
        /// <returns>Whether or not a Box was removed.</returns>
        bool RemoveBox(const Box &boxToRemove);


    //////////////////////////////////////////////////////////////////////////////////////////
    // Virtual method:  HasNoArea
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Shows whether this really has no Area at all, ie it doesn't have any
    //                  Box:es with both width and height.
    // Arguments:       None.
    // Return value:    Whether this Area actually covers any area.

		bool HasNoArea() const;


    //////////////////////////////////////////////////////////////////////////////////////////
    // Virtual method:  IsInside
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Shows whether a point is anywhere inside this Area's coverage.
    // Arguments:       The point to check if it's inside the Area, in absolute scene coordinates.
    // Return value:    Whether the point is inside any of this Area's Box:es.

		bool IsInside(const Vector &point) const;


    //////////////////////////////////////////////////////////////////////////////////////////
    // Virtual method:  IsInsideX
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Shows whether a coordinate is anywhere inside this Area's coverage, in the
    //                  X-axis only.
    // Arguments:       The x coord to check if it's inside the Area, in absolute scene units.
    // Return value:    Whether the point is inside any of this Area's Box:es in the X axis.

		bool IsInsideX(float pointX) const;


    //////////////////////////////////////////////////////////////////////////////////////////
    // Virtual method:  IsInsideY
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Shows whether a coordinate is anywhere inside this Area's coverage, in the
    //                  Y-axis only.
    // Arguments:       The x coord to check if it's inside the Area, in absolute scene units.
    // Return value:    Whether the point is inside any of this Area's Box:es in the Y axis.

		bool IsInsideY(float pointY) const;


    //////////////////////////////////////////////////////////////////////////////////////////
    // Virtual method:  MovePointInsideX
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Moves a coordinate to the closest value which is within any of this
    //                  Area's Box:es, in the X axis only.
    // Arguments:       The x coord to transform to the closest inside poistion in the x-axis.
    //                  Which direction to limit the search to. < 0 means can only look in the
    //                  negative dir, 0 means can look in both directions.
    // Return value:    Whether the point was moved at all to get inside this' x-space.

		bool MovePointInsideX(float &pointX, int direction = 0) const;


    //////////////////////////////////////////////////////////////////////////////////////////
    // Virtual method:  GetBoxInside
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Gets the first Box encountered in this that contains a specific point.
    // Arguments:       The point to check for Box collision, in absolute scene coordinates.
    // Return value:    Pointer to the first Box which was found to contain the point. 0 if
    //                  none was found. OWNERSHIP IS NOT TRANSFERRED!

		Box * GetBoxInside(const Vector &point);


    //////////////////////////////////////////////////////////////////////////////////////////
    // Virtual method:  RemoveBoxInside
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Removes the first Box encountered in this that contains a specific point.
    // Arguments:       The point to check for Box collision, in absolute scene coordinates.
    // Return value:    Copy of the Box that was removed. Will be  NoArea Box if none was found.

		Box RemoveBoxInside(const Vector &point);


    //////////////////////////////////////////////////////////////////////////////////////////
    // Virtual method:  GetCenterPoint
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Gets a center point for this of all the boxes waeighted by their sizes.
    // Arguments:       None.
    // Return value:    A center point of this area, can be outside the actual area though, if
    //                  pulled apart by two separate boxes, for example. 0,0 if this has no Area

		Vector GetCenterPoint() const;


    //////////////////////////////////////////////////////////////////////////////////////////
    // Virtual method:  GetRandomPoint
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Gets a random coordinate contained within any of this' Box:es.
    // Arguments:       None.
    // Return value:    A random point that is within this Area. 0,0 if this has no Area

		Vector GetRandomPoint() const;


    //////////////////////////////////////////////////////////////////////////////////////////
    // Virtual method:  GetName
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Gets the name of the Area
    // Arguments:       None.
    // Return value:    The name used to ID this Area.

		std::string GetName() const { return m_Name; }


    //////////////////////////////////////////////////////////////////////////////////////////
    // Protected member variable and method declarations

    protected:

        // The list of Box:es defining the Area in the owner Scene
        std::vector<Box> m_BoxList;
        // The name tag of this Area
        std::string m_Name;


    //////////////////////////////////////////////////////////////////////////////////////////
    // Private member variable and method declarations

    private:

		static const std::string c_ClassName; //!< A string with the friendly-formatted type name of this object.

    //////////////////////////////////////////////////////////////////////////////////////////
    // Method:          Clear
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Clears all the member variables of this Exit, effectively
    //                  resetting the members of this abstraction level only.
    // Arguments:       None.
    // Return value:    None.

        void Clear();

    };

friend class AreaEditor;
friend class AreaEditorGUI;
friend class AreaPickerGUI;

#define METABASE_AREA_NAME "MetabaseServiceArea"

enum NeighborDirection
{
    NODIR = -1,
    E = 0,
    SE,
    S,
    SW,
    W,
    NW,
    N,
    NE
};

// Concrete allocation and cloning definitions
EntityAllocation(Scene)


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     Scene
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a Scene object in system
//                  memory. Create() should be called before using the object.
// Arguments:       None.

    Scene() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Destructor:      ~Scene
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destructor method used to clean up a Scene object before deletion
//                  from system memory.
// Arguments:       None.

	~Scene() override { Destroy(true); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the Scene object ready for use.
// Arguments:       The Terrain to use. Ownership IS transferred!
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

	int Create(SLTerrain *pNewTerrain);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a Scene to be identical to another, by deep copy.
// Arguments:       A reference to the Scene to deep copy.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    int Create(const Scene &reference);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  LoadData
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Actually loads previously specified/created data into memory. Has
//                  to be done before using this Scene.
// Arguments:       Whetehr to actually place out all the sceneobjects associated with the
//                  Scene's definition. If not, they still remain in the internal placed
//                  objects list. This avoids messing with the MovableMan at all.
//                  Whether to do pathfinding init, which should be avoided if we are only
//                  loading and saving purposes of MetaMan, for example.
//					Whether to place actors and deployments (doors not affected).
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

	int LoadData(bool placeObjects = true, bool initPathfinding = true, bool placeUnits = true);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ExpandAIPlanAssemblySchemes
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Replace all assembly shemes by corresponding bunker assemblies in 
//					AI plan objects set.
// Arguments:       None.
// Return value:    None.

	int ExpandAIPlanAssemblySchemes();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SaveData
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves data currently in memory to disk.
// Arguments:       The filepath base to the where to save the Bitmap data. This means
//                  everything up to the extension. "FG" and "Mat" etc will be added.
// Return value:    An error return value signaling success or any particular failure.
//                  Anything below 0 is an error signal.

	int SaveData(std::string pathBase);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SavePreview
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves preview bitmap for this scene.
//
// Arguments:       The full filepath the where to save the Bitmap data.
// Return value:    None.

	int SavePreview(const std::string &bitmapPath);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ClearData
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears out any previously loaded bitmap data from memory.
// Arguments:       None.
// Return value:    An error return value signaling success or any particular failure.
//                  Anything below 0 is an error signal.

	int ClearData();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Reset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Resets the entire Scene, including its inherited members, to
//                  their default settings or values.
// Arguments:       None.
// Return value:    None.

    void Reset() override { Clear(); Entity::Reset(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Pure V. method:  Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the Scene object.
// Arguments:       Whether to only destroy the members defined in this derived class, or
//                  to destroy all inherited members also.
// Return value:    None.

    void Destroy(bool notInherited = false) override;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  MigrateToModule
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes this an original Preset in a different module than it was before.
//                  It severs ties deeply to the old module it was saved in.
// Arguments:       The ID of the new module.
// Return value:    Whether the migration was successful. If you tried to migrate to the
//                  same module it already was in, this would return false.

	bool MigrateToModule(int whichModule) override;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetLocation
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the specified location of this scene on the planet view
// Arguments:       None.
// Return value:    A Vector showing the location of this scene on the planet view.

    Vector GetLocation() const { return m_Location; }



//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetLocationOffset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the specified temporary location offset of this scene on the planet view.
// Arguments:       None.
// Return value:    A Vector showing the temporary location offset of this scene on the planet view.

    Vector GetLocationOffset() const { return m_LocationOffset; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetLocationOffset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the specified temporary location offset of this scene on the planet view.
// Arguments:       A Vector showing the temporary location offset of this scene on the planet view.
// Return value:    None.

    void SetLocationOffset(Vector newOffset) { m_LocationOffset = newOffset; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsMetagamePlayable
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows whether this is compatible with metagame play at all.
// Arguments:       None.
// Return value:    Whether this can be used on the metagame map.

    bool IsMetagamePlayable() const { return m_MetagamePlayable; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsRevealed
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows whether this is revealed on the metagame map.
// Arguments:       None.
// Return value:    Whether this can be seen on the metagame map yet.

    bool IsRevealed() const { return m_Revealed; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetTeamOwnership
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows which team owns this Scene in a Metagame, if any
// Arguments:       None.
// Return value:    The team that owns this site in a Metagame, if any

    int GetTeamOwnership() const { return m_OwnedByTeam; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetRoundIncome
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows how much income this Scene pulls in for its owning team each
//                  round of a metagame.
// Arguments:       None.
// Return value:    The income in oz that this generates each metagame round.

    float GetRoundIncome() const { return m_RoundIncome; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetBuildBudget
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows how much gold this Scene is budgeted to be built for this round.
// Arguments:       Which player's set budget to show.
// Return value:    The budget in oz that this is allocated to have built for this round.

    float GetBuildBudget(int player) const { return m_BuildBudget[player]; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetBuildBudgetRatio
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows how much of a player's budget this Scene is allocated to be
//                  built for this round.
// Arguments:       Which player's set budget ratio to show.
// Return value:    The budget in normalized ratio that this is allocated last round.

    float GetBuildBudgetRatio(int player) const { return m_BuildBudgetRatio[player]; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetAutoDesigned
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets whether this should be automatically designed by the AI plan
//                  even if owned by human players.
// Arguments:       What to set the setting to.
// Return value:    None.

    void SetAutoDesigned(bool autoDesigned = true) { m_AutoDesigned = autoDesigned; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetAutoDesigned
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tells whether this should be automatically designed by the AI plan
//                  even if owned by human players.
// Arguments:       None.
// Return value:    Whether this should be autodesigned or not.

    bool GetAutoDesigned() const { return m_AutoDesigned; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetTotalInvestment
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the total defense investment this scene has experienced by all
//                  teams since the metagame started.
// Arguments:       What to set the total investment in gold oz) to.
// Return value:    None.

    void SetTotalInvestment(float totalInvestment) { m_TotalInvestment = totalInvestment; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetTotalInvestment
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the total defense investment this scene has experienced by all
//                  teams since the metagame started.
// Arguments:       None.
// Return value:    The total investment in this scene.

    float GetTotalInvestment() const { return m_TotalInvestment; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetTerrain
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the SLTerrain.
// Arguments:       None.
// Return value:    A pointer to the SLTerrain. Ownership is NOT transferred!

    SLTerrain * GetTerrain() { return m_pTerrain; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetBackLayers
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets access to the background layer list.
// Arguments:       None.
// Return value:    A reference to the std::list containing all the background layers.
//                  Ownership is NOT transferred!

    std::list<SceneLayer *> & GetBackLayers() { return m_BackLayerList; }



//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AddArea
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds area to the list if this scene's areas.
// Arguments:       Area to add.
// Return value:    None.

	void AddArea(Scene::Area & newArea) { m_AreaList.push_back(newArea); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          FillUnseenLayer
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a new SceneLayer for a specific team and fills it with black
//                  pixels that end up being a specific size on the screen.
// Arguments:       A Vector with the desired dimensions of the unseen layer's chunky pixels.
//                  Which team to get the unseen layer for.
//                  Whether to create the unseen layers now, or wait until next time
//                  LoadData is called on this.
// Return value:    None.

    void FillUnseenLayer(Vector pixelSize, int team = Activity::TeamOne, bool createNow = true);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetUnseenLayer
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the unseen layer of a specific team.
// Arguments:       The new SceneLayer to use as the new unseen layer, Ownership IS XFERRED!
//                  Which team to get the unseen layer for.
// Return value:    None.

    void SetUnseenLayer(SceneLayer *pNewLayer, int team = Activity::TeamOne);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetUnseenLayer
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the unseen layer of a specific team.
// Arguments:       Which team to get the unseen layer for.
// Return value:    A pointer to the SceneLayer representing what hasn't been seen by a
//                  specific team yet. Ownership is NOT transferred!

    SceneLayer * GetUnseenLayer(int team = Activity::TeamOne) const { return team != Activity::NoTeam ? m_apUnseenLayer[team] : 0; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetSeenPixels
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the list of pixels that have been seen on a team's unseen layer.
// Arguments:       Which team to get the unseen layer for.
// Return value:    The list of pixel coordinates in the unseen layer's scale.

    std::list<Vector> & GetSeenPixels(int team = Activity::TeamOne) { return m_SeenPixels[team]; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ClearSeenPixels
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears the pixels that have been seen on a team's unseen layer.
// Arguments:       Which team to get the unseen layer for.
// Return value:    None.

    void ClearSeenPixels(int team = Activity::TeamOne);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CleanOrphanPixel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Checks a specific unseen pixel for only having two or less unseen
//                  neighbors, and if so, makes it seen.
// Arguments:       Coordinates to the pixel to check for orphaness.
//                  The direction we might be checking 'from', ie the neighbor we can
//                  already assume is seen without poking at the unseen map.
//                  Which team's unseen layer to check the pixel on.
// Return value:    Whether the pixel was deemed to be orphan and thus cleaned up.

    bool CleanOrphanPixel(int posX, int posY, NeighborDirection checkingFrom = NODIR, int team = Activity::TeamOne);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetDimensions
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the total dimensions (width and height) of the scene, in pixels.
// Arguments:       None.
// Return value:    A Vector describing the scene dimensions.

    Vector GetDimensions() const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetWidth
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the total width of the scene, in pixels.
// Arguments:       None.
// Return value:    An int describing the scene width.

    int GetWidth() const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetHeight
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the total height of the scene, in pixels.
// Arguments:       None.
// Return value:    An int describing the scene width.

    int GetHeight() const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          WrapsX
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether the scene wraps its scrolling around the X axis.
// Arguments:       None.
// Return value:    Whether the scene wraps around the X axis or not.

    bool WrapsX() const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          WrapsY
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether the scene wraps its scrolling around the Y axis.
// Arguments:       None.
// Return value:    Whether the scene wraps around the Y axis or not.

    bool WrapsY() const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          PlaceResidentBrain
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Places the individual brain of a single player which may be stationed
//                  on this Scene, and registers them as such in an Activity.
// Arguments:       The player's brain to place.
//                  The Activity to register the placed brains with. OWNERSHIP IS NOT TRANSFERRED!
// Return value:    If the brain was successfully found as resident and placed.

    bool PlaceResidentBrain(int player, Activity &newActivity);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          PlaceResidentBrains
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Places the individual brains of the various players which may be
//                  stationed on this Scene, and registers them as such in an Activity.
// Arguments:       The Activity to register the placed brains with. OWNERSHIP IS NOT TRANSFERRED!
// Return value:    How many brains were finally placed.

    int PlaceResidentBrains(Activity &newActivity);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          RetrieveResidentBrains
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Looks at the Activity and its players' registered brain Actors, and
//                  saves them as resident brains for this Scene. Done when a fight is over
//                  and the survivors remain!
// Arguments:       The Activity to check for registered brains. OWNERSHIP IS NOT TRANSFERRED!
// Return value:    How many brains were found registered with the passed in Activity.

    int RetrieveResidentBrains(Activity &oldActivity);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          RetrieveActorsAndDevices
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sucks up all the Actors and Devices currently active in MovableMan and
//                  puts them into this' list of objects to place on next load.
//                  Should be done AFTER RetrieveResidentBrains!
// Arguments:       The team to only retrieve Actors of. If NoTeam, then all will be grabbed.
//                  Whether to not get any brains at all.
// Return value:    How many objects were found knocking about in the world, and stored.

    int RetrieveActorsAndDevices(int onlyTeam = -1, bool noBrains = false);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetPlacedObjects
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the list of SceneObject:s which are placed in this scene on loading.
// Arguments:       Which set of placed objects to get. See the PlacedObjectSets enum.
// Return value:    The list of of placed objects. Ownership is NOT transferred!

    const std::list<SceneObject *> * GetPlacedObjects(int whichSet) const { return &m_PlacedObjects[whichSet]; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AddPlacedObject
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds a SceneObject to be placed in this scene. Ownership IS transferred!
// Arguments:       Which set of placed objects to add to. See the PlacedObjectSets enum.
//                  The SceneObject instance to add, OIT!
//                  Where in the list the object should be inserted. -1 means at the end
//                  of the list.
// Return value:    None.

    void AddPlacedObject(int whichSet, SceneObject *pObjectToAdd, int listOrder = -1);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          RemovePlacedObject
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Removes a SceneObject placed in this scene.
// Arguments:       Which set of placed objects to rem from. See the PlacedObjectSets enum.
//                  The list order number of the object to remove. If -1, the last one is removed.
// Return value:    None.

    void RemovePlacedObject(int whichSet, int whichToRemove = -1);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          PickPlacedObject
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns the last placed object that graphically overlaps an absolute
//                  point in the scene.
// Arguments:       Which set of placed objects to pick from. See the PlacedObjectSets enum.
//                  The point in absolute scene coordinates that will be used to pick the
//                  last placed SceneObject which overlaps it.
//                  An int which will be filled out with the order place of any found object
//                  in the list. if nothing is found, it will get a value of -1.
// Return value:    The last hit SceneObject, if any. Ownership is NOT transferred!

    const SceneObject * PickPlacedObject(int whichSet, Vector &scenePoint, int *pListOrderPlace = 0) const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          PickPlacedActorInRange
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns the last placed actor object that is closer than range to scenePoint
//
// Arguments:       Which set of placed objects to pick from. See the PlacedObjectSets enum.
//                  The point in absolute scene coordinates that will be used to pick the
//                  closest placed SceneObject near it.
//                  The range to check for nearby objects.
//                  An int which will be filled out with the order place of any found object
//                  in the list. if nothing is found, it will get a value of -1.
//
// Return value:    The closest actor SceneObject, if any. Ownership is NOT transferred!

const SceneObject * PickPlacedActorInRange(int whichSet, Vector &scenePoint, int range, int *pListOrderPlace) const;



//////////////////////////////////////////////////////////////////////////////////////////
// Method:          UpdatePlacedObjects
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updated the objects in the placed scene objects list of this. This is
//                  mostly for the editor to represent the items correctly.
// Arguments:       Which set of placed objects to update. See the PlacedObjectSets enum.
// Return value:    None.

    void UpdatePlacedObjects(int whichSet);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ClearPlacedObjectSet
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Removes all entries in a specific set of placed Objects.
// Arguments:       Which set of placed objects to clear. See the PlacedObjectSets enum.
// Return value:    How many things were removed in teh process of clearing that set.

    int ClearPlacedObjectSet(int whichSet);



//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetResidentBrain
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the resident brain Actor of a specific player from this scene,
//                  if there is any. OWNERSHIP IS NOT TRANSFERRED!
// Arguments:       Which player to get the resident brain of.
// Return value:    The SO containing the brain, or 0 if there aren't any of that player.

    SceneObject * GetResidentBrain(int player) const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetResidentBrain
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the resident brain Actor of a specific player from this scene,
//                  if there is any. Ownership IS transferred!
// Arguments:       Which player to set the resident brain of.
//                  The Actor to set as the resident brain of the specified player.
// Return value:    None.

    void SetResidentBrain(int player, SceneObject *pNewBrain);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetResidentBrainCount
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the number of brains currently residing in this scene.
// Arguments:       None.
// Return value:    The number of resident brains who are installed here.

    int GetResidentBrainCount() const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetArea
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds one or modifies an existing area of this Scene.
// Arguments:       The area to add or modify of the same name in this Scene. Ownership is
//                  NOT transferred!
// Return value:    Whether the specified area was previously defined in this scene.

    bool SetArea(Area &newArea);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          HasArea
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Checks for the existence of a specific Area identified by a name.
//                  This won't throw any errors to the console if the Area isn't found.
// Arguments:       The name of the Area to try to find in this Scene.
// Return value:    Whether the specified area is defined in this Scene.

    bool HasArea(std::string areaName);

	/// <summary>
	/// Gets a specified Area identified by name. Ownership is NOT transferred!
	/// </summary>
	/// <param name="areaName">The name of the Area to try to get.</param>
	/// <param name="luaWarnNotError">Whether to warn or error in the Lua console. True is warn, false is error.</param>
	/// <returns>A pointer to the Area asked for, or nullptr if no Area of that name was found.</returns>
	Area * GetArea(const std::string_view &areaName, bool luaWarnNotError);

	/// <summary>
	/// Gets a specified Area identified by name, showing a Lua error if it's not found. Ownership is NOT transferred!
	/// </summary>
	/// <param name="areaName">The name of the Area to try to get.</param>
	/// <returns>A pointer to the Area asked for, or nullptr if no Area of that name was found.</returns>
	Area * GetArea(const std::string &areaName) { return GetArea(areaName, false); }

	/// <summary>
	/// Gets a specified Area identified by name, showing a Lua warning if it's not found. Ownership is NOT transferred!
	/// Using this function will not add the area to the list of required areas which Scenario GUI uses to show compatible areas.
	/// </summary>
	/// <param name="areaName">The name of the Area to try to get.</param>
	/// <returns>A pointer to the Area asked for, or nullptr if no Area of that name was found.</returns>
	Area * GetOptionalArea(const std::string &areaName) { return GetArea(areaName, true); };


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          RemoveArea
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Removes a specific Area identified by a name.
// Arguments:       The name of the Area to try to remove.
// Return value:    Whether an Area of that name was found, and subsequently removed.

    bool RemoveArea(std::string areaName);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          WithinArea
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Checks if a point is within a specific named Area of this Scene. If
//                  no Area of the name is found, this just returns false without error.
// Arguments:       The name of the Area to try to check against.
//                  The point to see if it's within the specified Area.
// Return value:    Whether any Area of that name was found, AND the point falls within it.

    bool WithinArea(std::string areaName, const Vector &point) const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetGlobalAcc
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the global acceleration (in m/s^2) that is applied to all movable
//                  objects' velocities during every frame. Typically models gravity.
// Arguments:       None.
// Return value:    A Vector describing the global acceleration.

    Vector GetGlobalAcc() const { return m_GlobalAcc; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetGlobalAcc
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the global acceleration (in m/s^2) that is applied to all movable
//                  objects' velocities during every frame. Typically models gravity.
// Arguments:       A Vector describing the global acceleration.
// Return value:    None.

	void SetGlobalAcc(Vector newValue) { m_GlobalAcc = newValue; }



//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetMetasceneParent
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns parent scene name of this metascene.
// Arguments:       None.
// Return value:    Name of a parent scene.

	string GetMetasceneParent() const { return m_MetasceneParent; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetLocation
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the specified location of this Scene in the scene
// Arguments:       A Vector with the desired location of this Scene in the scene.
// Return value:    None.

    void SetLocation(const Vector& newLocation) { m_Location = newLocation; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetMetagamePlayable
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets whether this can be played in the Metagame map at all.
// Arguments:       Whether this is compatible with metagame play at all.
// Return value:    None.

    void SetMetagamePlayable(bool isPlayable) { m_MetagamePlayable = isPlayable; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetRevealed
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets whether this should show up on the Metagame map yet.
// Arguments:       Whether to reveal this on the metagame map or not.
// Return value:    None.

    void SetRevealed(bool isRevealed) { m_Revealed = isRevealed; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetTeamOwnership
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the team who owns this Scene in a Metagame
// Arguments:       The team who should now own this Scene
// Return value:    None.

    void SetTeamOwnership(int newTeam);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetBuildBudget
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets how much gold this Scene is budgeted to be built for this round.
// Arguments:       The player whom is setting the budget.
//                  The budget in oz that this is allocated to have built for this round.
// Return value:    None.

    void SetBuildBudget(int player, float budget) { m_BuildBudget[player] = budget; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetBuildBudgetRatio
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets how much of a player's budget this Scene is budgeted to be build
//                  for each turn.
// Arguments:       The player whom is setting the budget ratio.
//                  The budget in normalized ratio that this is allocated of the total.
// Return value:    None.

    void SetBuildBudgetRatio(int player, float budgetRatio) { m_BuildBudgetRatio[player] = budgetRatio; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CalcBuildBudgetUse
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Figure out exactly how much of the build budget would be used if
//                  as many blueprint objects as can be afforded and exists would be built.
// Arguments:       The player for whom we are calculating this budget use.
//                  An optional int that will be filled with number of objects that can
//                  acutally be built.
//                  An optional int that will be filled with number of objects that can
//                  built out of the AI plan set, AFTER the blueprints are built.
// Return value:    The amount of funds that would be applied to the building of objects.

    float CalcBuildBudgetUse(int player, int *pAffordCount = 0, int *pAffordAIPlanCount = 0) const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ApplyAIPlan
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Puts the pre-built AI base plan into effect by transferring as many
//                  pieces as the current base budget allows from the AI plan to the actual
//                  blueprints to be built at this Scene.
// Arguments:       The AI player whom is putting his plans into motion.
//                  An optional int that will be filled with number of objects that were
//                  acutally moved from the AI plan to the blueprints.
// Return value:    The value of the AI plan objects that were put onto the blueprints.

    float ApplyAIPlan(int player, int *pObjectsApplied = 0);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ApplyBuildBudget
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Actually builds as many objects in the specific player's Blueprint
//                  list as can be afforded by his build budget. The budget is deducted
//                  accordingly.
// Arguments:       The player whom is using his budget.
//                  An optional int that will be filled with number of objects that were
//                  acutally built.
// Return value:    The amount of funds that were applied to the building of objects.

    float ApplyBuildBudget(int player, int *pObjectsBuilt = 0);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          RemoveAllPlacedActors
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Remove all actors that are in the placed set of objects to load for
//                  this scene. All except for an optionally specified team, that is.
// Arguments:       Remove all actors but of this team.
// Return value:    How many actors were actually removed.

    int RemoveAllPlacedActors(int exceptTeam = -1);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetOwnerOfAllDoors
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the ownership of all doors placed in this scene to a specific team
// Arguments:       The team to change the ownership to
//                  The player which placed these doors.
// Return value:    How many doors were actually affected.

    int SetOwnerOfAllDoors(int team, int player);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsScanScheduled
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tells whether a specific team has scheduled an orbital Scan of this.
// Arguments:       The team to check for.
// Return value:    Whether the scan has been scheduled and paid for.

    bool IsScanScheduled(int team) const { return m_ScanScheduled[team]; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetScheduledScan
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets this to be orbitally scanned by a specific team on next load.
// Arguments:       The team to schedule the scan for.
//                  Whether to actually schedule the scan or clear it.
// Return value:    None.

    void SetScheduledScan(int team, bool scan = true) { m_ScanScheduled[team] = scan; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ResetPathFinding
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Recalculates all of the pathfinding data. This is very expensive, so
//                  do very rarely!
// Arguments:       None.
// Return value:    None.

    void ResetPathFinding();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          UpdatePathFinding
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Recalculates only the areas of the pathfinding data that have been
//                  marked as outdated.
// Arguments:       None.
// Return value:    None.

    void UpdatePathFinding();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          PathFindingUpdated
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tells whether the pathfinding data has been updated in the last frame.
// Arguments:       None.
// Return value:    Whether the pathfinding data was recalculated fully or partially.

    bool PathFindingUpdated() { return m_PathfindingUpdated; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CalculatePath
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Calculates and returns the least difficult path between two points on
//                  the current scene. Takes both distance and materials into account.
// Arguments:       Start and end positions on the scene to find the path between.
//                  A list which will be filled out with waypoints between the start and end.
// Return value:    The total minimum difficulty cost calculated between the two points on
//                  the scene.

    float CalculatePath(const Vector &start, const Vector &end, std::list<Vector> &pathResult, float digStrenght = 1);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CalculateScenePath
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Calculates the least difficult path between two points on
//                  the current scene. Takes both distance and materials into account.
//                  A list of waypoints can be retrived from Scene.ScenePath.
//                  For exposing CalculatePath to Lua.
// Arguments:       Start and end positions on the scene to find the path between.
//                  If the path should be moved to the ground or not.
//                  The maximum material strength any actor traveling along the paht can
//                  dig through.
// Return value:    The number of waypoints from start to goal, or -1 if no path.

    int CalculateScenePath(const Vector start, const Vector end, bool movePathToGround, float digStrength = 1);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetScenePathSize
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets how many waypoints there are in the ScenePath currently
// Arguments:       None.
// Return value:    The number of waypoints in the ScenePath.

	int GetScenePathSize() const { return m_ScenePath.size(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Lock
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Locks all dynamic internal scene bitmaps so that manipulaitons of the
//                  scene's color and matter representations can take place.
//                  Doing it in a separate method like this is more efficient because
//                  many bitmap manipulaitons can be performed between a lock and unlock.
//                  UnlockScene() should always be called after accesses are completed.
// Arguments:       None.
// Return value:    None.

    void Lock();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Unlock
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Unlocks the scene's bitmaps and prevents access to display memory.
//                  Doing it in a separate method like this is more efficient because
//                  many bitmap accesses can be performed between a lock and an unlock.
//                  UnlockScene() should only be called after LockScene().
// Arguments:       None.
// Return value:    None.

    void Unlock();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsLocked
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether the entire scene is currently locked or not.
// Arguments:       None.
// Return value:    Whether the entire scene is currently locked or not.

    bool IsLocked() const { return m_Locked; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the state of this Scene. Supposed to be done every frame
//                  before drawing.
// Arguments:       None.
// Return value:    None.

	void Update();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsMetagameInternal
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Whether this scene is a temprorary metagame scene and should 
//					not be used anywhere except in metagame.
// Arguments:       None.
// Return value:    Whether scene belongs to metagame or not.

	bool IsMetagameInternal() const { return m_IsMetagameInternal; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetMetagameInternal
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets whether this scene is a temprorary metagame scene and should 
//					not be used anywhere except in metagame.
// Arguments:       New value.
// Return value:    None.

	void SetMetagameInternal(bool newValue) { m_IsMetagameInternal = newValue; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetPreviewBitmap
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns preview bitmap pointer for this scene.
// Arguments:       None.
// Return value:    Pointer to preview bitmap.

	BITMAP * GetPreviewBitmap() const { return m_pPreviewBitmap; };

    // Holds the path calculated by CalculateScenePath
    std::list<Vector> m_ScenePath;

//////////////////////////////////////////////////////////////////////////////////////////
// Protected member variable and method declarations

protected:

    // Member variables
    static Entity::ClassInfo m_sClass;

    // Position of the site/scene on the planet in the site selection menu view, relative to the center of the planet
    Vector m_Location;
    // Temporary location offset used to correct scene position when scene dots overlap.
    Vector m_LocationOffset;
    // Whether at all eligible for the Metagame
    bool m_MetagamePlayable;
    // Whether this is revealed on the metagame planet map yet or not
    bool m_Revealed;
    // Owned by which Team, if any (<0 if none)
    int m_OwnedByTeam;
    // Total income this place generates per Metagame round for its owner team
    float m_RoundIncome;
    // The special placed brain actors of each player that inhabit this Scene, OWNED here
    SceneObject *m_ResidentBrains[Players::MaxPlayerCount];
    // Budget in oz this place is allocated per player for a metagame round for building (applying) blueprint objects.
    float m_BuildBudget[Players::MaxPlayerCount];
    // Budget in ratio of the player for a metagame round. This is used to re-set the BuildBudget to match the ratio
    // that a player budgeted to this site in the previous turn.
    float m_BuildBudgetRatio[Players::MaxPlayerCount];
    // Whether this should be automatically designed by the AI Plan even if it's owned by a human player
    bool m_AutoDesigned;
    // The total amount of gold (in oz) that has been invested in the defenses of this site, by all teams
    float m_TotalInvestment;
    // Terrain definition
    SLTerrain *m_pTerrain;
    // Pathfinding graph and logic. Owned by this
    PathFinder *m_pPathFinder;
    // Is set to true on any frame the pathfinding data has been updated
    bool m_PathfindingUpdated;
    // Timers for when to do an update of all or only part of the pathfinding data
    Timer m_FullPathUpdateTimer;
    Timer m_PartialPathUpdateTimer;
    // SceneObject:s to be placed in the scene, divided up by different sets - OWNED HERE
    std::list<SceneObject *> m_PlacedObjects[PLACEDSETSCOUNT];
    // List of background layers, first is the closest to the terrain, last is closest to the back
    std::list<SceneLayer *> m_BackLayerList;
    // Dimensions of the pixels of the unseen layers, when they are dynamically generated. If 0, the layer was not generated
    Vector m_UnseenPixelSize[Activity::MaxTeamCount];
    // Layers representing the unknown areas for each team
    SceneLayer *m_apUnseenLayer[Activity::MaxTeamCount];
    // Which pixels of the unseen map have just been revealed this frame, in the coordinates of the unseen map
    std::list<Vector> m_SeenPixels[Activity::MaxTeamCount];
    // Pixels on the unseen map deemed to be orphans and cleaned up, will be moved to seen pixels next update
    std::list<Vector> m_CleanedPixels[Activity::MaxTeamCount];
    // Whether this Scene is scheduled to be orbitally scanned by any team
    bool m_ScanScheduled[Activity::MaxTeamCount];

    // List of all the specified Area:s of the scene
    std::list<Area> m_AreaList;
    // Whether the scene's bitmaps are locked or not.
    bool m_Locked;
    // The global acceleration vector in m/s^2. (think gravity/wind)
    Vector m_GlobalAcc;
    // Names of all Schemes and selected assemblies for them
    std::map<std::string, const BunkerAssembly *> m_SelectedAssemblies;
    // Amounts of limited assemblies
    std::map<std::string, int> m_AssembliesCounts;
	// Scene preview bitmap
	BITMAP * m_pPreviewBitmap;
	// Scene preview source file
	ContentFile m_PreviewBitmapFile;
	// Name of a scene which can be replaced by this scene in MetaGame
	// Scenes with m_MetaSceneParent field set will be invisible for editors and activities unless
	// ShowMetaScenes flag in settings.ini is set
	string m_MetasceneParent;
	// Whether this scene must be shown anywhere in UIs
	bool m_IsMetagameInternal;

	std::list<Deployment *>m_Deployments;

//////////////////////////////////////////////////////////////////////////////////////////
// Private member variable and method declarations

private:

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this Scene, effectively
//                  resetting the members of this abstraction level only.
// Arguments:       None.
// Return value:    None.

    void Clear();


    // Disallow the use of some implicit methods.
    Scene(const Scene &reference) = delete;
    void operator=(const Scene &rhs) = delete;

};

} // namespace RTE

#endif // File