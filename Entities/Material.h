#ifndef _RTEMATERIAL_
#define _RTEMATERIAL_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            Material.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Header file for the Material class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "Entity.h"
#include "Color.h"
#include "ContentFile.h"

namespace RTE
{

//////////////////////////////////////////////////////////////////////////////////////////
// Class:           Material
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Represents a material and holds all the relevant data.
// Parent(s):       Entity.
// Class history:   05/19/2002 Material created.
//                  07/30/2006 Converted to Object, since it has name etc.

class Material:
    public Entity
{


//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable, method and friend function declarations

public:


// Concrete allocation and cloning definitions
EntityAllocation(Material)


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     Material
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a Material object in system
//                  memory. Create() should be called before using the object.
// Arguments:       None.

    Material() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     Material
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Copy constructor method used to instantiate a Material object
//                  identical to an already existing one.
// Arguments:       A Material object which is passed in by reference.

    Material(const Material &reference) { if (this != &reference) { Clear(); Create(reference); } }


//////////////////////////////////////////////////////////////////////////////////////////
// Operator:        Material assignment
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     An assignment operator for setting one Material equal to another.
// Arguments:       A Material reference.
// Return value:    A reference to the changed Atom.

    Material & operator=(const Material &rhs) { if (this != &rhs) { Destroy(); Create(rhs); } return *this; }

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the Material object ready for use.
// Arguments:       None.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Create();
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a Material to be identical to another, by deep copy.
// Arguments:       A reference to the Material to deep copy.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    int Create(const Material &reference);


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
// Description:     Resets the entire Material, including its inherited members, to their
//                  default settings or values.
// Arguments:       None.
// Return value:    None.

    virtual void Reset() { Clear(); Entity::Reset(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this Material to an output stream for
//                  later recreation with Create(istream &stream);
// Arguments:       A Writer that the Material will save itself to.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Save(Writer &writer) const;


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
// Description:     Gets the class name of this Material.
// Arguments:       None.
// Return value:    A string with the friendly-formatted type name of this Material.

    virtual const std::string & GetClassName() const { return m_sClass.GetName(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetTexture
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the texture bitmap of this Material, if any is associate with it.
// Arguments:       None.
// Return value:    A string with the friendly-formatted type name of this Material.

    BITMAP * GetTexture() const { return m_pTexture; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetSettleMaterialID
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     If this material transforms into something else when settling into the
//                  terrain, this will return that different material index. If not, it will
//                  just return the regular index of this material.
// Arguments:       None.
// Return value:    The the settling material index of this MOSPixel, or the regular index.

    virtual unsigned char GetSettleMaterialID() const { return settleMaterial != 0 ? settleMaterial : id; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  UsesOwnColor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicated whether or not to use the Material's own color when a pixel
//                  of this Material is knocked loose from the terrain.
// Arguments:       None.
// Return value:    A bool that if false, means tthe terrain pixel's color should be applied
//                  instead.

    bool UsesOwnColor() const { return m_UseOwnColor; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:		GetId  
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Id tag, or index into the material palette. 0 - 255.
// Arguments:       None.
// Return value:    Returns Id tag, or index into the material palette. 0 - 255.

	unsigned char GetId() const { return id; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:			GetRestitution
//////////////////////////////////////////////////////////////////////////////////////////
// Description:		Elasticity coefficient (no bounce 0.0 - 1.0 max)
//					A float scalar value that defines the restitution. 1.0 = no kinetic
//					energy is lost in a collision, 0.0 = all energy is lost (plastic).
// Arguments:       None.
// Return value:    Returns elasticity coefficient (no bounce 0.0 - 1.0 max)

	float GetRestitution() const { return restitution; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:			GetFriction
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Friction coefficient (no friction 0.0 - 1.0 max)
//					A float scalar value that defines the friction. 1.0 = will snag onto
//					everything, 0.0 = will glide with no friction.
// Arguments:       None.
// Return value:    Returns friction coefficient (no friction 0.0 - 1.0 max)

	float GetFriction() const { return friction; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:			GetStickiness
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Stickiness coefficient (no sticky 0.0 - 1.0 max)
//					Determines the likelyhood of somehting of this material sticking when a collision occurs
// Arguments:       None.
// Return value:    Returns stickiness coefficient (no sticky 0.0 - 1.0 max)

	float GetStickiness() const { return stickiness; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:			GetStrength
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     The impulse force that a particle needs to knock loose a terrian pixel of
//					this material. In kg * m/s.
// Arguments:       None.
// Return value:    Returns impulse force that a particle needs to knock loose a terrian pixel of
//					this material. In kg * m/s.

	float GetStrength() const { return strength; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:			GetVolumeDensity
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Density in Kg/L
// Arguments:       None.
// Return value:    Returns Density in Kg/L

	float GetVolumeDensity() const { return volumeDensity; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:			GetPixelDensity
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Density in kg/pixel, usually calculated from the KG per Volume L property
// Arguments:       None.
// Return value:    Returns Density in kg/pixel, usually calculated from the KG per Volume L property

	float GetPixelDensity() const { return pixelDensity; }
	

//////////////////////////////////////////////////////////////////////////////////////////
// Method:			IsScrap
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Whether this material is scrap material made form gibs of things that have already been blown apart
// Arguments:       None.
// Return value:    Returns whether this material is scrap material

	bool IsScrap() const { return isScrap; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:			GetSettleMaterial
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     The material to spawn instead of this one for special effects, etc.
//					0 here means to spawn this material.
// Arguments:       None.
// Return value:    Returns the material to spawn instead of this one for special effects, etc.

	unsigned char GetSettleMaterial() const { return settleMaterial; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:			
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     
// Arguments:       None.
// Return value:    

	unsigned char GetSpawnMaterial() const { return spawnMaterial; }



    // Id tag, or index into the material palette. 0 - 255.
    unsigned char id;
    // Elasticity coefficient (no bounce 0.0 - 1.0 max)
    // A float scalar value that defines the restitution. 1.0 = no kinetic
    // energy is lost in a collision, 0.0 = all energy is lost (plastic).
    float restitution;
    // Friction coefficient (no friction 0.0 - 1.0 max)
    // A float scalar value that defines the friction. 1.0 = will snag onto
    // everything, 0.0 = will glide with no friction.
    float friction;
    // Stickiness coefficient (no sticky 0.0 - 1.0 max)
    // Determines the likelyhood of somehting of this material sticking when a collision occurs
    float stickiness;
    // The impulse force that a particle needs to knock loose a terrian pixel of
    // this material. In kg * m/s.
    float strength;
    // Density in Kg/L
    float volumeDensity;
    // Density in kg/pixel, usually calculated from the KG per Volume L property
    float pixelDensity;
    // How much impulse gib limit increases per litre of this object
    float gibImpulseLimitPerLitre;
    // How much wound gib limit increases per litre of this object
    float gibWoundLimitPerLitre; 
    // The priority that a pixel of this material has to be displayed, etc.
    // The higher the number, the higher the priority.
    int priority;
    // The material to turn particles of this into when they settle on the terrain.
    // 0 here means to spawn this material.
    int settleMaterial;
    // The material to spawn instead of this one for special effects, etc.
    // 0 here means to spawn this material.
    int spawnMaterial;
    // Whether this material is scrap material made form gibs of things that have already been blown apart
    bool isScrap;
    // The natural color of this material.
    Color color;


//////////////////////////////////////////////////////////////////////////////////////////
// Protected member variable and method declarations

protected:

    static Entity::ClassInfo m_sClass;
    // The file pointing to the terrain texture of this Material
    ContentFile m_TextureFile;
    // The texture of this material, used when building an SLTerrain
    BITMAP *m_pTexture;
    // Whether or not to use the own color when a pixel of this material is knocked loose
    // from the terrain. If 0, then the terrain pixel's color will be applied instead.
    bool m_UseOwnColor;


//////////////////////////////////////////////////////////////////////////////////////////
// Private member variable and method declarations

private:


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this Material, effectively
//                  resetting the members of this abstraction level only.
// Arguments:       None.
// Return value:    None.

    void Clear();


};

} // namespace RTE

#endif // File