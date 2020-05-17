#ifndef _RTEMOPIXEL_
#define _RTEMOPIXEL_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            MOPixel.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Header file for the MOPixel class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "MovableObject.h"
#include "Color.h"

namespace RTE
{

class Atom;


//////////////////////////////////////////////////////////////////////////////////////////
// Class:           MOPixel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     A movable object with mass that is graphically represented by
//                  dynamic drawing stuff.
// Parent(s):       MovableObject.
// Class history:   03/18/2001 MOPixel created.

class MOPixel:
    public MovableObject
{


//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable, method and friend function declarations

public:


// Concrete allocation and cloning definitions
EntityAllocation(MOPixel)


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     MOPixel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a MOPixel object in system
//                  memory. Create() should be called before using the object.
// Arguments:       None.

    MOPixel() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     MOPixel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Convenience constructor to both instantiate a MOPixel in memory and
//                  and Create it at the same time.
// Arguments:       See Create() below.

    MOPixel(Color color,
            const float mass,
            const Vector &position,
            const Vector &velocity,
            Atom *atom,
            const unsigned long lifetime = 0) { Clear(); Create(color, mass, position, velocity, atom, lifetime); }


//////////////////////////////////////////////////////////////////////////////////////////
// Destructor:      ~MOPixel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destructor method used to clean up a MOPixel object before deletion
//                  from system memory.
// Arguments:       None.

    virtual ~MOPixel() { Destroy(true); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the MOPixel object ready for use.
// Arguments:       None.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Create();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the MOPixel object ready for use.
// Arguments:       A Color object specifying the color of this MOPixel.
//                  A float specifying the object's mass in Kilograms (kg).
//                  A Vector specifying the initial position.
//                  A Vector specifying the initial velocity.
//                  An Atom that will collide with the terrain.
//                  The amount of time in ms this MovableObject will exist. 0 means unlim.
//                  The material Id that this MovableObject will draw upon a material
//                  bitmap when being recycled.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Create(Color color,
                       const float mass,
                       const Vector &position,
                       const Vector &velocity,
                       Atom *atom,
                       const unsigned long lifetime = 0);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a MOPixel to be identical to another, by deep copy.
// Arguments:       A reference to the MOPixel to deep copy.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    int Create(const MOPixel &reference);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Reset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Resets the entire MOPixel, including its inherited members, to their
//                  default settings or values.
// Arguments:       None.
// Return value:    None.

    virtual void Reset() { Clear(); MovableObject::Reset(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the MOPixel object.
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


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetMaterial
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the main Material of this MovableObject.
// Arguments:       None.
// Return value:    The the Material of this MovableObject.

    virtual Material const * GetMaterial() const;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetDrawPriority
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the drawing priority of this MovableObject, if two things were
//                  overlap when copying to the terrain, the higher priority MO would
//                  end up getting drawn.
// Arguments:       None.
// Return value:    The the priority  of this MovableObject. Higher number, the higher
//                  priority.

    virtual int GetDrawPriority() const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetColor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the color of this MOPixel.
// Arguments:       None.
// Return value:    A Color object describing the color.

    Color GetColor() const { return m_Color; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetAtom
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the current Atom of this MOPixel.
// Arguments:       None.
// Return value:    A const reference to the current Atom.

    const Atom * GetAtom() const { return m_pAtom; }

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  HitsMOs
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets whether this MovableObject is set to collide with other
//                  MovableObject:s during travel.
// Arguments:       None.
// Return value:    Whether to hit other MO's during travel, or not.

    virtual bool HitsMOs() const;
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetColor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the color value of this MOPixel.
// Arguments:       A Color object specifying the new color index value.
// Return value:    None.

    void SetColor(Color newColor) { m_Color = newColor; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetAtom
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Replaces the current Atom of this MOPixel with a new one.
// Arguments:       A reference to the new Atom. Ownership IS transferred!
// Return value:    None.

    void SetAtom(Atom *newAtom);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetLethalRange
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Travel distance until the bullet start to lose lethality.
// Arguments:       The distance in pixels.
// Return value:    None.

    void SetLethalRange(float range);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetMaxLethalRangeFactor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Travel distance until the bullet start to lose lethality.
// Arguments:       None
// Return value:    The factor that modifies the base value.

    float GetMaxLethalRangeFactor() const { return m_MaxLethalRange; }


/*
//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetToHitMOs
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets this MovableObject to collide with other MovableObjects during
//                  travel.
// Arguments:       Whether to hit other MO's during travel, or not.
// Return value:    None.

    void SetToHitMOs(bool hitMOs = true);
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  IsGold
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether this MO is made of Gold or not.
// Arguments:       None.
// Return value:    Whether this MovableObject is of Gold or not.

    virtual bool IsGold() const { return m_MOType == TypeGeneric && GetMaterial()->GetIndex() == c_GoldMaterialID; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  CollideAtPoint
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Calculates the collision response when another MO's Atom collides with
//                  this MO's physical representation. The effects will be applied
//                  directly to this MO, and also represented in the passed in HitData.
// Arguments:       Reference to the HitData struct which describes the collision. This
//                  will be modified to represent the results of the collision.
// Return value:    Whether the collision has been deemed valid. If false, then disregard
//                  any impulses in the Hitdata.

    virtual bool CollideAtPoint(HitData &hitData);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          OnBounce
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Defines what should happen when this MovableObject hits and then
//                  bounces off of something. This is called by the owned Atom/AtomGroup
//                  of this MovableObject during travel.
// Arguments:       The HitData describing the collision in detail.
// Return value:    Wheter the MovableObject should immediately halt any travel going on
//                  after this bounce.

    virtual bool OnBounce(HitData &hd);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          OnSink
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Defines what should happen when this MovableObject hits and then
//                  sink into something. This is called by the owned Atom/AtomGroup
//                  of this MovableObject during travel.
// Arguments:       The HitData describing the collision in detail.
// Return value:    Wheter the MovableObject should immediately halt any travel going on
//                  after this sinkage.

    virtual bool OnSink(HitData &hd);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  RestDetection
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Does the calculations necessary to detect whether this MO appears to
//                  have has settled in the world and is at rest or not. IsAtRest()
//                  retreves the answer.
// Arguments:       None.
// Return value:    None.

    virtual void RestDetection();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Travel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Travels this MOSRotatin, using its physical representation.
// Arguments:       None.
// Return value:    None.

    virtual void Travel();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates this MOPixel. Supposed to be done every frame.
// Arguments:       None.
// Return value:    None.

    virtual void Update();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this MOPixel's current graphical representation to a
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
// Protected member variable and method declarations

protected:

    // Member variables
    static Entity::ClassInfo m_sClass;
    Color m_Color;
    Vector m_PrevPos;
    Atom *m_pAtom;
    float m_MinLethalRange;     // Lower bound multiplier for setting LethalRange at random. 1.0 equals one screen.
    float m_MaxLethalRange;     // Upper bound multiplier for setting LethalRange at random. 1.0 equals one screen.

//////////////////////////////////////////////////////////////////////////////////////////
// Private member variable and method declarations

private:

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this MOPixel, effectively
//                  resetting the members of this abstraction level only.
// Arguments:       None.
// Return value:    None.

    void Clear();

    // An estimate of how far this MO has traveled since its creation.
    float m_DistanceTraveled;
    // After this distance in meters, the damage output of this MO decrease. Default is half a scren plus sharp length for pixels spawned from a gun and one screen otherwise.
    float m_LethalRange;
    // When Sharpness has decreased below this threshold the MO becomes m_HitsMOs=false. Default is Sharpness*0.5
    float m_LethalSharpness;

};

} // namespace RTE

#endif // File