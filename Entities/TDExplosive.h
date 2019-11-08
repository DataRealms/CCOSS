#ifndef _RTETDEXPLOSIVE_
#define _RTETDEXPLOSIVE_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            TDExplosive.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Header file for the TDExplosive class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "ThrownDevice.h"
#include <map>

namespace RTE
{

class Magazine;


//////////////////////////////////////////////////////////////////////////////////////////
// Class:           TDExplosive
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     A thrown device that explodes into particles/shards after activation.
// Parent(s):       ThrownDevice.
// Class history:   08/8/2004 TDExplosive created.

class TDExplosive:
    public ThrownDevice
{


//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable, method and friend function declarations

public:


// Concrete allocation and cloning definitions
ENTITYALLOCATION(TDExplosive)


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     TDExplosive
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a TDExplosive object in system
//                  memory. Create() should be called before using the object.
// Arguments:       None.

    TDExplosive() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Destructor:      ~TDExplosive
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destructor method used to clean up a TDExplosive object before deletion
//                  from system memory.
// Arguments:       None.

    virtual ~TDExplosive() { Destroy(true); }

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the TDExplosive object ready for use.
// Arguments:       None.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Create();
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a TDExplosive to be identical to another, by deep copy.
// Arguments:       A reference to the TDExplosive to deep copy.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    int Create(const TDExplosive &reference);


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
// Description:     Resets the entire TDExplosive, including its inherited members, to their
//                  default settings or values.
// Arguments:       None.
// Return value:    None.

    virtual void Reset() { Clear(); ThrownDevice::Reset(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this TDExplosive to an output stream for
//                  later recreation with Create(Reader &reader);
// Arguments:       A Writer that the TDExplosive will save itself with.
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
// Virtual method:   GetClassName
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the class name of this Entity.
// Arguments:       None.
// Return value:    A string with the friendly-formatted type name of this object.

    virtual const std::string & GetClassName() const { return m_sClass.GetName(); }

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetMass
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the mass value of this TDExplosive, including the mass of Magazine
//                  may have inserted.
// Arguments:       None.
// Return value:    A float describing the mass value in Kilograms (kg).

    virtual float GetMass() const;
*/
/*
//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetParentOffset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the current position offset of this TDExplosive relative to the
//                  parent Actor's position, if attached.
// Arguments:       None.
// Return value:    A const reference to the current parent offset.

    const Vector & GetParentOffset() const { return m_ParentOffset; }
*/
/*
//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  RestDetection
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Does the calculations necessary to detect whether this MO appears to
//                  have has settled in the world and is at rest or not. IsAtRest()
//                  retreves the answer.
// Arguments:       None.
// Return value:    None.

    virtual void RestDetection();
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Detach
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Detaches this Attachable from its host MOSprite
// Arguments:       None.
// Return value:    None.

    virtual void Detach();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Activate
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Activates this Device's features. Depending on whether it can activate
//                  when being held, it will activate and start countdown to boom.
// Arguments:       None.
// Return value:    None.

    virtual void Activate();

/*
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
*/

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
// Description:     Draws this TDExplosive's current graphical representation to a
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
// Description:     Draws this' current graphical HUD overlay representation to a
//                  BITMAP of choice.
// Arguments:       A pointer to a BITMAP to draw on.
//                  The absolute position of the target bitmap's upper left corner in the Scene.
//                  Which player's screen this is being drawn to. May affect what HUD elements
//                  get drawn etc.
// Return value:    None.

    virtual void DrawHUD(BITMAP *pTargetBitmap, const Vector &targetPos = Vector(), int whichScreen = 0, bool playerControlled = false);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  IsAnimatedManually
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     If true then the m_Frame property is not changed by the Update function
// Arguments:       None.
// Return value:    Whether this TDExplosive is animated manually.

	bool IsAnimatedManually() const { return m_IsAnimatedManually; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  SetAnimatedManually
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets whether this TDExplosive is animated manually.
// Arguments:       Manual animation flag value.
// Return value:    None.

	void SetAnimatedManually(bool newValue) { m_IsAnimatedManually = newValue; }


//////////////////////////////////////////////////////////////////////////////////////////
// Protected member variable and method declarations

protected:

    // Member variables.
    static Entity::ClassInfo m_sClass;

    // THIS IS FOR BACKWARDS COMPATIBILITY - now creates gibs instead
    // Special temporary var used at load-time to keep track of how many particles the user intends to specify for each type.
    int m_NumberToAdd;
    // Whether this starts activation timer once it is first thrown, like a hand grenade vs a dumb bomb
    bool m_ActivatesWhenReleased;
	// If true m_Frame is not changed during an update hence the animation is done by external Lua code
	bool m_IsAnimatedManually;


//////////////////////////////////////////////////////////////////////////////////////////
// Private member variable and method declarations

private:


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this TDExplosive, effectively
//                  resetting the members of this abstraction level only.
// Arguments:       None.
// Return value:    None.

    void Clear();


    // Disallow the use of some implicit methods.
    TDExplosive(const TDExplosive &reference);
    TDExplosive & operator=(const TDExplosive &rhs);

};

} // namespace RTE

#endif // File