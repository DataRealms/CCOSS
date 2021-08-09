#ifndef _RTEDEPLOYMENT_
#define _RTEDEPLOYMENT_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            Deployment.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Header file for the Deployment class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "RTETools.h"
#include "SceneObject.h"
#include "Vector.h"
#include "SceneMan.h"
//#include "MovableMan.h"

namespace RTE
{

class ContentFile;


//////////////////////////////////////////////////////////////////////////////////////////
// Class:           Deployment
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     A special SceneObject that specifies a Loadout of whatever Tech is
//                  relevant to be placed in a specific location in a Scene.
// Parent(s):       SceneObject.
// Class history:   02/27/2012 Deployment created.

class Deployment:
    public SceneObject
{


//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable, method and friend function declarations

public:


// Concrete allocation and cloning definitions
EntityAllocation(Deployment)
SerializableOverrideMethods
ClassInfoGetters

//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     Deployment
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a Deployment object in system
//                  memory. Create() should be called before using the object.
// Arguments:       None.

    Deployment() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Destructor:      ~Deployment
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destructor method used to clean up a Deployment object before deletion
//                  from system memory.
// Arguments:       None.

	~Deployment() override { Destroy(true); }


//////////////////////////////////////////////////////////////////////////////////////////
// Pure V. method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the Deployment object ready for use.
// Arguments:       None.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

   int Create() override;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the Deployment object ready for use.
// Arguments:       The name of the Loadout that this should invoke at this' position.
//                  Icon that represents this graphically.
//                  The radius around this deployment that gets checked if another
//                  actor/item of the same type and name already exists and will block
//                  re-spawning a new one by this.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

	int Create(string loadoutName, const Icon &icon, float spawnRadius);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a Deployment to be identical to another, by deep copy.
// Arguments:       A reference to the Deployment to deep copy.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    int Create(const Deployment &reference);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Reset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Resets the entire Deployment, including its inherited members, to
//                  their default settings or values.
// Arguments:       None.
// Return value:    None.

    void Reset() override { Clear(); SceneObject::Reset(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Pure V. method:  Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the Deployment object.
// Arguments:       Whether to only destroy the members defined in this derived class, or
//                  to destroy all inherited members also.
// Return value:    None.

    void Destroy(bool notInherited = false) override;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetGraphicalIcon
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets a bitmap showing a good identifyable icon of this, for use in
//                  GUI lists etc.
// Arguments:       None.
// Return value:    A good identifyable graphical representation of this in a BITMAP, if
//                  available. If not, 0 is returned. Ownership is NOT TRANSFERRED!

    BITMAP * GetGraphicalIcon() const override { return !m_Icon.GetBitmaps8().empty() ? m_Icon.GetBitmaps8().at(0) : nullptr; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetLoadoutName
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the name of the Loadout that this Deployment spawns.
// Arguments:       None.
// Return value:    The name of the Loadout preset that this Deployment spawns.

    const std::string & GetLoadoutName() { return m_LoadoutName; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetGraphicalIcon
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets a bitmap showing a good identifyable icon of this.
// Arguments:       None.
// Return value:    The Icon that represents this graphically.

	Icon GetIcon() { return m_Icon; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetSpawnRadius
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the radius around this deployment that gets checked if another
//                  actor/item of the same type and name already exists and will block
//                  re-spawning a new one by this
// Arguments:       None.
// Return value:    The radius this Deployment will be checking within.

	float GetSpawnRadius() const { return m_SpawnRadius; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  IsOnScenePoint
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether this' current graphical representation overlaps
//                  a point in absolute scene coordinates.
// Arguments:       The point in absolute scene coordinates.
// Return value:    Whether this' graphical rep overlaps the scene point.

	bool IsOnScenePoint(Vector &scenePoint) const override;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  CreateDeployedActor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates and returns the Actor that this Deployment dictates should
//                  spawn here. Ownership IS transferred!! All items of the Loadout of
//                  this Deployment will be added to the Actor's inventory as well (and
//                  also owned by it)
// Arguments:       Which in-game player to create the delivery for.
//                  A float which will be added to with the cost of the stuff returned here.
// Return value:    The Actor instance, if any, that this Deployment is supposed to spawn.
//                  OWNERSHIP IS TRANSFERRED!

	Actor * CreateDeployedActor(int player, float &costTally);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  CreateDeployedActor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates and returns the Actor that this Deployment dictates should
//                  spawn here. Ownership IS transferred!! All items of the Loadout of
//                  this Deployment will be added to the Actor's inventory as well (and
//                  also owned by it)
// Arguments:       Which in-game player to create the delivery for.
// Return value:    The Actor instance, if any, that this Deployment is supposed to spawn.
//                  OWNERSHIP IS TRANSFERRED!

	Actor * CreateDeployedActor();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  CreateDeployedObject
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates and returns the first Device that Deployment dictates should
//                  spawn here. Ownership IS transferred!! Only the first Device is created.
// Arguments:       Which in-game player to create the delivery for.
//                  A float which will be added to with the cost of the stuff returned here.
// Return value:    The Actor instance, if any, that this Deployment is supposed to spawn.
//                  OWNERSHIP IS TRANSFERRED!

	SceneObject * CreateDeployedObject(int player, float &costTally);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  CreateDeployedObject
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates and returns the first Device that Deployment dictates should
//                  spawn here. Ownership IS transferred!! Only the first Device is created.
// Arguments:       Which in-game player to create the delivery for.
// Return value:    The Actor instance, if any, that this Deployment is supposed to spawn.
//                  OWNERSHIP IS TRANSFERRED!

	SceneObject * CreateDeployedObject();



//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  DeploymentBlocked
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tests whether the Object this is supposed to spawn/deploy is blocked
//                  by an already exiting object in the a list being positioned within the
//                  spawn radius of this.
// Arguments:       Which in-game player to create the delivery for.
//                  A list of SceneObject:s that will be tested against to see if any
//                  sufficiently similar Object is positioned within the spawn radius of
//                  this.
// Return value:    Whether the deployment spawning is blocked by one of the Objects in
//                  the list.

	bool DeploymentBlocked(int player, const std::list<SceneObject *> &existingObjects);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetGoldValue
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the cost to purchase this item, in oz's of gold.
// Arguments:       If this is supposed to be adjusted for a specific Tech's subjective
//                  value, then pass in the native DataModule ID of that tech. 0 means
//                  no Tech is specified and the base value is returned.
//                  How much to multiply the value if this happens to be a foreign Tech.
// Return value:    The cost, in oz of gold.

	float GetGoldValue(int nativeModule = 0, float foreignMult = 1.0, float nativeMult = 1.0) const override { return GetTotalValue(nativeModule, foreignMult, nativeMult); }

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetGoldValueOld
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     DOES THE SAME THING AS GetGoldValue, USED ONLY TO PRESERVE LUA COMPATIBILITY

	float GetGoldValueOld(int nativeModule = 0, float foreignMult = 1.0) const override { return GetTotalValue(nativeModule, foreignMult, 1.0); }



//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetTotalValue
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the total liquidation value of a spawn of this, including
//                  everything carried by it.
// Arguments:       If this is supposed to be adjusted for a specific Tech's subjective
//                  value, then pass in the native DataModule ID of that tech. 0 means
//                  no Tech is specified and the base value is returned.
//                  How much to multiply the value if this happens to be a foreign Tech.
// Return value:    The current value of this and all contained assets.

	float GetTotalValue(int nativeModule = 0, float foreignMult = 1.0, float nativeMult = 1.0) const override;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetID
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Return this deployment's unique ID
// Arguments:       None.
// Return value:    This deployment's ID

	unsigned int GetID() const { return m_ID; };


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CloneID
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clones id from the specified deployment
// Arguments:      Deployment to clone Id from.
// Return value:    None

	void CloneID(Deployment * from) { if (from) m_ID = from->GetID(); };


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          NewID
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Generates new random ID for this deployment.
// Arguments:       None.
// Return value:    None.

	void NewID() { m_ID = RandomNum(1, 0xFFFF); };


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this Deployment's current graphical representation to a
//                  BITMAP of choice.
// Arguments:       A pointer to a BITMAP to draw on.
//                  The absolute position of the target bitmap's upper left corner in the Scene.
//                  In which mode to draw in. See the DrawMode enumeration for the modes.
//                  Whether to not draw any extra 'ghost' items of this MovableObject,
//                  like indicator arrows or hovering HUD text and so on.
// Return value:    None.

    void Draw(BITMAP *pTargetBitmap, const Vector &targetPos = Vector(), DrawMode mode = g_DrawColor, bool onlyPhysical = false) const override;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  IsHFlipped
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns whether this MOSprite is being drawn flipped horizontally
//                  (along the vertical axis), or not.
// Arguments:       None.
// Return value:    Whether flipped or not.

	bool IsHFlipped() const override { return m_HFlipped; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virutal method:  SetHFlipped
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets whether this should be drawn flipped horizontally (around the
//                  vertical axis).
// Arguments:       A bool with the new value.
// Return value:    None.

	void SetHFlipped(const bool flipped) override { m_HFlipped = flipped; }

//////////////////////////////////////////////////////////////////////////////////////////
// Protected member variable and method declarations

protected:

    // Member variables
    static Entity::ClassInfo m_sClass;

    // Name of the Loadout that shuold be placed at this' location in the Scene.
    string m_LoadoutName;
    // The Icon that graphically represents this
    Icon m_Icon;
    // The radius around this deployment that gets checked if another actor/item of the same type and name already exists and will block re-spawning a new one by this
    float m_SpawnRadius;
    // The radius around this deployment that gets checked if an actor spawned by this deployment is present. If it is, deployment is blocked.
    float m_WalkRadius;
	// Unique deployment id, assigned to units deployed by this deployment
	unsigned int m_ID;
	// Whether the deployment and it's loadout is flipped
	bool m_HFlipped;
	// Shared HFlipped arrow bitmaps
	static std::vector<BITMAP *> m_apArrowLeftBitmap;
	static std::vector<BITMAP *> m_apArrowRightBitmap;


//////////////////////////////////////////////////////////////////////////////////////////
// Private member variable and method declarations

private:

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this Deployment, effectively
//                  resetting the members of this abstraction level only.
// Arguments:       None.
// Return value:    None.

    void Clear();


    // Disallow the use of some implicit methods.
    Deployment(const Deployment &reference) = delete;
    void operator=(const Deployment &rhs) = delete;

};

} // namespace RTE

#endif // File