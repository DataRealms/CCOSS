#ifndef _RTEADOOR_
#define _RTEADOOR_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            ADoor.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Header file for the ADoor class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "Actor.h"

struct BITMAP;

namespace RTE
{

class Attachable;


//////////////////////////////////////////////////////////////////////////////////////////
// Class:           ADoor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     A sliding or swinging door.
// Parent(s):       Actor.
// Class history:   11/20/2007 ADoor created.

class ADoor:
    public Actor
{


//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable, method and friend function declarations

public:


enum DoorState
{
    CLOSED = 0,
    OPENING,
    OPEN,
    CLOSING,
    DoorStateCount
};


    //////////////////////////////////////////////////////////////////////////////////////////
    // Nested class:    Sensor
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     The ray-casting sensor which triggers the door opening or closing,
    //                  depending on the team of the Actor that broke the ray.
    // Parent(s):       Serializable.
    // Class history:   11/20/2007 Sensor created.

    class Sensor:
        public Serializable
    {

    friend class ADoor;


    //////////////////////////////////////////////////////////////////////////////////////////
    // Public member variable, method and friend function declarations

    public:


    //////////////////////////////////////////////////////////////////////////////////////////
    // Constructor:     Sensor
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Constructor method used to instantiate a Sensor object in system
    //                  memory. Create() should be called before using the object.
    // Arguments:       None.

        Sensor() { Clear(); }


    //////////////////////////////////////////////////////////////////////////////////////////
    // Virtual method:  Create
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Makes the Sensor object ready for use.
    // Arguments:       None.
    // Return value:    An error return value signaling sucess or any particular failure.
    //                  Anything below 0 is an error signal.

        virtual int Create();


    //////////////////////////////////////////////////////////////////////////////////////////
    // Virtual method:  Create
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Creates a Sensor to be identical to another, by deep copy.
    // Arguments:       A reference to the Sensor to deep copy.
    // Return value:    An error return value signaling sucess or any particular failure.
    //                  Anything below 0 is an error signal.

        virtual int Create(const Sensor &reference);


    //////////////////////////////////////////////////////////////////////////////////////////
    // Virtual method:  ReadProperty
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Reads a property value from a Reader stream. If the name isn't
    //                  recognized by this class, then ReadProperty of the parent class
    //                  is called. If the property isn't recognized by any of the base classes,
    //                  false is returned, and the Reader's position is untouched.
    // Arguments:       The name of the property to be read.
    //                  A Reader lined up to the value of the property to be read.
    // Return value:    An error return value signaling whether the property was successfully
    //                  read or not. 0 means it was read successfully, and any nonzero indicates
    //                  that a property of that name could not be found in this or base classes.

        virtual int ReadProperty(std::string propName, Reader &reader);


    //////////////////////////////////////////////////////////////////////////////////////////
    // Virtual method:  Reset
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Resets the entire Serializable, including its inherited members, to their
    //                  default settings or values.
    // Arguments:       None.
    // Return value:    None.

        virtual void Reset() { Clear(); }


    //////////////////////////////////////////////////////////////////////////////////////////
    // Virtual method:  Save
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Saves the complete state of this Sensor to an output stream for
    //                  later recreation with Create(Reader &reader);
    // Arguments:       A Writer that the Sensor will save itself with.
    // Return value:    An error return value signaling sucess or any particular failure.
    //                  Anything below 0 is an error signal.

        virtual int Save(Writer &writer) const;


    //////////////////////////////////////////////////////////////////////////////////////////
    // Virtual method:  GetClassName
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Gets the class name of this Entity.
    // Arguments:       None.
    // Return value:    A string with the friendly-formatted type name of this object.

        virtual const std::string & GetClassName() const { return m_sClassName; }


    //////////////////////////////////////////////////////////////////////////////////////////
    // Virtual method:  GetStartOffset
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Gets the starting position offset of this sensor from the owning ADoor
    //                  position.
    // Arguments:       None.
    // Return value:    The starting coordinates relative to the m_Pos of this' ADoor.

        virtual Vector GetStartOffset() const { return m_StartOffset; }


	//////////////////////////////////////////////////////////////////////////////////////////
	// Virtual method:  SetStartOffset
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Sets the starting position offset of this sensor from the owning ADoor
	//                  position.
	// Arguments:       The starting coordinates relative to the m_Pos of this' ADoor..
	// Return value:    None

		virtual void SetStartOffset(Vector startOffsetValue) { m_StartOffset = startOffsetValue; }


    //////////////////////////////////////////////////////////////////////////////////////////
    // Virtual method:  GetSensorRay
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Gets the sensor ray vector out fromt he start offset's position.
    // Arguments:       None.
    // Return value:    The sensor ray

        virtual Vector GetSensorRay() const { return m_SensorRay; }


	//////////////////////////////////////////////////////////////////////////////////////////
	// Virtual method:  SetSensorRay
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Sets the sensor ray vector out fromt he start offset's position.
	// Arguments:       The sensor ray.
	// Return value:    None

		virtual void SetSensorRay(Vector sensorRayValue)  { m_SensorRay = sensorRayValue; }


    //////////////////////////////////////////////////////////////////////////////////////////
    // Virtual method:  SenseActor
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Casts the ray along the sensor vector and returns any Actor that was
    //                  found along it.
    // Arguments:       The position, rotation and flipping of of the owner.
    //                  Which MOID to ignore, if any.
    // Return value:    The root Actor of the first MOID hit by the sensor ray. 0 if none.

        virtual Actor * SenseActor(Vector &doorPos, Matrix &doorRot, bool doorHFlipped = false, MOID ignoreMOID = g_NoMOID);


    //////////////////////////////////////////////////////////////////////////////////////////
    // Protected member variable and method declarations

    protected:

        // Member variables
        static const std::string m_sClassName;
        // The offset of the sensor ray start relative the position of its ADoor
        Vector m_StartOffset;
        // The ray out from the offset
        Vector m_SensorRay;
        // How many pixels to skip between sensing pixels
        int m_Skip;


    //////////////////////////////////////////////////////////////////////////////////////////
    // Private member variable and method declarations

    private:

    //////////////////////////////////////////////////////////////////////////////////////////
    // Method:          Clear
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Clears all the member variables of this Sensor, effectively
    //                  resetting the members of this abstraction level only.
    // Arguments:       None.
    // Return value:    None.

        void Clear();

    };


// Concrete allocation and cloning definitions
EntityAllocation(ADoor)


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     ADoor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a ADoor object in system
//                  memory. Create() should be called before using the object.
// Arguments:       None.

    ADoor() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Destructor:      ~ADoor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destructor method used to clean up a ADoor object before deletion
//                  from system memory.
// Arguments:       None.

    virtual ~ADoor() { Destroy(true); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the ADoor object ready for use.
// Arguments:       A Reader that the ADoor will create itself with.
//                  Whether there is a class name in the stream to check against to make
//                  sure the correct type is being read from the stream.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Create();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a ADoor to be identical to another, by deep copy.
// Arguments:       A reference to the ADoor to deep copy.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Create(const ADoor &reference);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  ReadProperty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reads a property value from a Reader stream. If the name isn't
//                  recognized by this class, then ReadProperty of the parent class
//                  is called. If the property isn't recognized by any of the base classes,
//                  false is returned, and the Reader's position is untouched.
// Arguments:       The name of the property to be read.
//                  A Reader lined up to the value of the property to be read.
// Return value:    An error return value signaling whether the property was successfully
//                  read or not. 0 means it was read successfully, and any nonzero indicates
//                  that a property of that name could not be found in this or base classes.

    virtual int ReadProperty(std::string propName, Reader &reader);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Reset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Resets the entire ADoor, including its inherited members, to their
//                  default settings or values.
// Arguments:       None.
// Return value:    None.

    virtual void Reset() { Clear(); Actor::Reset(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this ADoor to an output stream for
//                  later recreation with Create(Reader &reader);
// Arguments:       A Writer that the ADoor will save itself with.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Save(Writer &writer) const;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the SceneLayer object.
// Arguments:       Whether to only destroy the members defined in this derived class, or
//                  to destroy all inherited members also.
// Return value:    None.

    virtual void Destroy(bool notInherited = false);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetClass
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the ClassInfo instance of this Entity.
// Arguments:       None.
// Return value:    A reference to the ClassInfo of this' class.

    virtual const Entity::ClassInfo & GetClass() const { return m_sClass; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetClassName
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the class name of this Entity.
// Arguments:       None.
// Return value:    A string with the friendly-formatted type name of this object.

    virtual const std::string & GetClassName() const { return m_sClass.GetName(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetMass
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the mass value of this ACrab, including the mass of its
//                  currently attached body parts and inventory.
// Arguments:       None.
// Return value:    A float describing the mass value in Kilograms (kg).

    virtual float GetMass() const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetDoor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the moving door Attachable
// Arguments:       None.
// Return value:    A pointer to the door Attachable of this. Ownership is NOT transferred!

    Attachable * GetDoor() const { return m_pDoor; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetID
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the MOID of this MovableObject for this frame.
// Arguments:       A MOID specifying the MOID that this MovableObject is
//                  assigned for this frame.
// Return value:    None.

    virtual void SetID(const MOID newID);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsControllable
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tells whether the player can switch control to this at all
// Arguments:       None.
// Return value:    Whether a player can control this at all.

    virtual bool IsControllable() const { return false; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetDoorState
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the current state of the door.
// Arguments:       None.
// Return value:    An int encoding the hatch state. See the DoorState enum.

    DoorState GetDoorState() const { return m_DoorState; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          OpenDoor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Opens the door, if it's closed.
// Arguments:       None.
// Return value:    None.

    virtual void OpenDoor();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CloseDoor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Closes the door, if it's open.
// Arguments:       None.
// Return value:    None.

    virtual void CloseDoor();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetClosedByDefault
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets whether this ADoor closes after a while by default (or opens)
// Arguments:       Whether the door by default goes to a closed position. If not, then it
//                  will open after a while.
// Return value:    None.

    virtual void SetClosedByDefault(bool closedByDefault) { m_ClosedByDefault = closedByDefault; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GibThis
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gibs this, effectively destroying it and creating multiple gibs or
//                  pieces in its place.
// Arguments:       The impulse (kg * m/s) of the impact causing the gibbing to happen.
//					The internal blast impulse which will push the gibs away from the center.
//                  A pointer to an MO which the gibs shuold not be colliding with!
// Return value:    None.

    virtual void GibThis(Vector impactImpulse = Vector(), float internalBlast = 10, MovableObject *pIgnoreMO = 0);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  IsOnScenePoint
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether this' current graphical representation overlaps
//                  a point in absolute scene coordinates.
// Arguments:       The point in absolute scene coordinates.
// Return value:    Whether this' graphical rep overlaps the scene point.

    virtual bool IsOnScenePoint(Vector &scenePoint) const;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  MaterialDrawOverride
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Used to temporarily remove or add back the material drawing of this
//                  in the scene. Used for making pathfinding work through doors.
// Arguments:       Whether to enable the override or not.
// Return value:    None.

    virtual void MaterialDrawOverride(bool enable);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates this MovableObject. Supposed to be done every frame.
// Arguments:       None.
// Return value:    None.

    virtual void Update();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this ADoor's current graphical representation to a
//                  BITMAP of choice.
// Arguments:       A pointer to a BITMAP to draw on.
//                  The absolute position of the target bitmap's upper left corner in the Scene.
//                  In which mode to draw in. See the DrawMode enumeration for the modes.
//                  Whether to not draw any extra 'ghost' items of this MovableObject,
//                  indicator arrows or hovering HUD text and so on.
// Return value:    None.

    virtual void Draw(BITMAP *pTargetBitmap,
                      const Vector &targetPos = Vector(),
                      DrawMode mode = g_DrawColor,
                      bool onlyPhysical = false) const;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  DrawHUD
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this Actor's current graphical HUD overlay representation to a
//                  BITMAP of choice.
// Arguments:       A pointer to a BITMAP to draw on.
//                  The absolute position of the target bitmap's upper left corner in the Scene.
//                  Which player's screen this is being drawn to. May affect what HUD elements
//                  get drawn etc.
// Return value:    None.

    virtual void DrawHUD(BITMAP *pTargetBitmap, const Vector &targetPos = Vector(), int whichScreen = 0, bool playerControlled = false);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          RemoveAnyRandomWounds
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Removes a specified amount of wounds from the actor and all standard attachables.
// Arguments:       Amount of wounds to remove.
// Return value:    Damage taken from removed wounds.

	virtual int RemoveAnyRandomWounds(int amount);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetMOIDs
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Puts all MOIDs associated with this MO and all it's descendants into MOIDs vector
// Arguments:       Vector to store MOIDs
// Return value:    None.

	virtual void GetMOIDs(std::vector<MOID> &MOIDs) const;

//////////////////////////////////////////////////////////////////////////////////////////
// Protected member variable and method declarations

protected:


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  UpdateChildMOIDs
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes this MO register itself and all its attached children in the
//                  MOID register and get ID:s for itself and its children for this frame.
// Arguments:       The MOID index to register itself and its children in.
//                  The MOID of the root MO of this MO, ie the highest parent of this MO.
//                  0 means that this MO is the root, ie it is owned by MovableMan.
//                  Whether this MO should make a new MOID to use for itself, or to use
//                  the same as the last one in the index (presumably its parent),
// Return value:    None.

    virtual void UpdateChildMOIDs(std::vector<MovableObject *> &MOIDIndex,
                                  MOID rootMOID = g_NoMOID,
                                  bool makeNewMOID = true);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  DrawDoorMaterial
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws the material under the pos of the door attachable, to create
//                  terrain collision detection for the doors.
// Arguments:       None.
// Return value:    None.

    virtual void DrawDoorMaterial();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  EraseDoorMaterial
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Floodfills the material area under the last pos of the door attachable
//                  that matches the material index of it. This is to get rid of the
//                  material footprint made with DrawDoorMaterial when the door part starts
//                  to move.
// Arguments:       None.
// Return value:    Whether the fill erasure was successful (if the same material as the
//                  door was found and erased).

    virtual bool EraseDoorMaterial();


    // Member variables
    static Entity::ClassInfo m_sClass;

    // Actual door module that moves. Owned by this
    Attachable *m_pDoor;
    // Open and closed offsets from this' pos - these effectively replace the door's parent offset
    Vector m_OpenOffset;
    Vector m_ClosedOffset;
    // Open and closed ABSOLUTE angles of the door attachable, in radians
    float m_OpenAngle;
    float m_ClosedAngle;
    // Current door action state.
    DoorState m_DoorState;
    // Timer for opening and closing the door
    Timer m_DoorMoveTimer;
    // The time it takes to open or close the door, in ms.
    int m_DoorMoveTime;   
    // Whether the clsoed position is the default
    bool m_ClosedByDefault;
    // How long the door stays in the non-default state before returning to the default
    int m_ResetDefaultDelay;
    // Timer for the resetting to default
    Timer m_ResetDefaultTimer;
    // All the sensors for detecting Actor:s approaching the door
    std::list<Sensor> m_Sensors;
    // The delay between each sensing pass, in ms
    long m_SensorInterval;
    // Times the exit interval
    Timer m_SensorTimer;
    // Whether to draw the door's silhouette to the terrain material layer when fully open/closed
    bool m_DrawWhenOpen;
    bool m_DrawWhenClosed;
    // The position the door attachable had when its material was drawn to the material bitmap.
    // This is used to erase the previous material representation.
    Vector m_LastDoorMaterialPos;
    // Whether the door material is currently drawn onto the material layer
    bool m_DoorMaterialDrawn;
    // Whether the drawing override is enabled (meaning drawing has been removed and will NOT happen)
    bool m_MaterialDrawOverride;
    // THe ID of the door material drawn to the terrain
    unsigned char m_DoorMaterialID;

	SoundContainer m_DoorMoveStartSound; //!< Sound played when the door starts moving from fully open/closed position towards the opposite end.
	SoundContainer m_DoorMoveSound; //!< Sound played while the door is moving, between open/closed position.
	SoundContainer m_DoorMoveEndSound; //!< Sound played when the door stops moving and is at fully open/closed position.

//////////////////////////////////////////////////////////////////////////////////////////
// Private member variable and method declarations

private:

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this ADoor, effectively
//                  resetting the members of this abstraction level only.
// Arguments:       None.
// Return value:    None.

    void Clear();


    // Disallow the use of some implicit methods.
    ADoor(const ADoor &reference);
    ADoor & operator=(const ADoor &rhs);

};

} // namespace RTE

#endif // File