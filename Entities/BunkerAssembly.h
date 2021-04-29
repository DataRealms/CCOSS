#ifndef _RTEBUNKERASSEMBLY_
#define _RTEBUNKERASSEMBLY_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            BunkerAssembly.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Header file for the BunkerAssembly class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "RTETools.h"
#include "TerrainObject.h"
#include "Vector.h"
#include "SceneMan.h"
#include "BunkerAssemblyScheme.h"
#include "Deployment.h"
//#include "MovableMan.h"

namespace RTE
{

class ContentFile;


//////////////////////////////////////////////////////////////////////////////////////////
// Class:           BunkerAssembly
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     An assembly of a few terrain objects.
//                  material layer and optional background layer.
// Parent(s):       SceneObject.
// Class history:   08/23/2002 BunkerAssembly created.

class BunkerAssembly:
    public TerrainObject
{


//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable, method and friend function declarations

public:

// Concrete allocation and cloning definitions
EntityAllocation(BunkerAssembly)
SerializableOverrideMethods
ClassInfoGetters

//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     BunkerAssembly
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a BunkerAssembly object in system
//                  memory. Create() should be called before using the object.
// Arguments:       None.

    BunkerAssembly() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Destructor:      ~BunkerAssembly
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destructor method used to clean up a BunkerAssembly object before deletion
//                  from system memory.
// Arguments:       None.

	~BunkerAssembly() override { Destroy(true); }


//////////////////////////////////////////////////////////////////////////////////////////
// Pure V. method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the BunkerAssembly object ready for use.
// Arguments:       None.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

   int Create() override;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the BunkerAssembly object ready for use.
// Arguments:       None.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

	int Create(BunkerAssemblyScheme * scheme);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a BunkerAssembly to be identical to another, by deep copy.
// Arguments:       A reference to the BunkerAssembly to deep copy.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    int Create(const BunkerAssembly &reference);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Reset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Resets the entire BunkerAssembly, including its inherited members, to
//                  their default settings or values.
// Arguments:       None.
// Return value:    None.

    void Reset() override { Clear(); SceneObject::Reset(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Pure V. method:  Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the BunkerAssembly object.
// Arguments:       Whether to only destroy the members defined in this derived class, or
//                  to destroy all inherited members also.
// Return value:    None.

    void Destroy(bool notInherited = false) override;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetParentAssemblySchemeName
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     
//                  
// Arguments:       None.
// Return value:    
//                  

    string GetParentAssemblySchemeName() const { return m_ParentAssemblyScheme; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  IsOnScenePoint
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether this' current graphical representation overlaps
//                  a point in absolute scene coordinates.
// Arguments:       The point in absolute scene coordinates.
// Return value:    Whether this' graphical rep overlaps the scene point.

	bool IsOnScenePoint(Vector &scenePoint) const override;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  GetDeployments
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Retrieves the list of random deployemtns selected to be deployed by this assembly
//					based on it's parent scheme MaxDeployments value. This list will always include all
//					brain deployments so it can be longer that MaxDeployments.
// Arguments:       None.
// Return value:    List of deployments.

	std::vector<Deployment *> GetDeployments();


	//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetTeam
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets which team this belongs to.
// Arguments:       The assigned team number.
// Return value:    None.

	void SetTeam(int team) override;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetPlacedObjects
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the list of SceneObject:s which are placed in this assembly on loading.
// Arguments:       None.
// Return value:    The list of of placed objects. Ownership is NOT transferred!

    const std::list<SceneObject *> * GetPlacedObjects() const { return &m_PlacedObjects; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AddPlacedObject
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Adds placed object to the internallist of placed objects for this assembly,
//					applies it's image to presentation bitmap and sets assembly price accordingly.
//					Added scene object MUST have coordinates relative to this assembly. 
// Arguments:       Object to add.
// Return value:    None.

	void AddPlacedObject(SceneObject * pSO);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetGraphicalIcon
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets a bitmap showing a good identifyable icon of this, for use in
//                  GUI lists etc.
// Arguments:       None.
// Return value:    A good identifyable graphical representation of this in a BITMAP, if
//                  available. If not, 0 is returned. Ownership is NOT TRANSFERRED!

	BITMAP * GetGraphicalIcon() const override { return m_pPresentationBitmap; };


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetSymmetricAssemblyName
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the name of an assembly symmetric to this one.
// Arguments:       None.
// Return value:    Symmetric assembly name.

	string GetSymmetricAssemblyName() const { return m_SymmetricAssembly; };


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetSymmetricAssemblyName
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the name of an assembly symmetric to this one.
// Arguments:       Symmetric assembly name.
// Return value:    None.

	void SetSymmetricAssemblyName(string newSymmetricAssembly) { m_SymmetricAssembly = newSymmetricAssembly; };

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this TerrainObject's current graphical representation to a
//                  BITMAP of choice.
// Arguments:       A pointer to a BITMAP to draw on.
//                  The absolute position of the target bitmap's upper left corner in the Scene.
//                  In which mode to draw in. See the DrawMode enumeration for the modes.
//                  Whether to not draw any extra 'ghost' items of this MovableObject,
//                  like indicator arrows or hovering HUD text and so on.
// Return value:    None.

    void Draw(BITMAP *pTargetBitmap, const Vector &targetPos = Vector(), DrawMode mode = g_DrawColor, bool onlyPhysical = false) const override;





//////////////////////////////////////////////////////////////////////////////////////////
// Protected member variable and method declarations

protected:

    // Member variables
    static Entity::ClassInfo m_sClass;
    // SceneObject:s to be placed in the scene, OWNED HERE
    std::list<SceneObject *> m_PlacedObjects;
	// Parent bunker assembly scheme
	std::string m_ParentAssemblyScheme;
	// Group proveded by parent scheme to which this assembly was added
	std::string m_ParentSchemeGroup;
	// Bitmap shown during draw and icon creation
	BITMAP * m_pPresentationBitmap;
	//Assembly symmetric to this one
	string m_SymmetricAssembly;

//////////////////////////////////////////////////////////////////////////////////////////
// Private member variable and method declarations

private:

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this BunkerAssembly, effectively
//                  resetting the members of this abstraction level only.
// Arguments:       None.
// Return value:    None.

    void Clear();


    // Disallow the use of some implicit methods.
    BunkerAssembly(const BunkerAssembly &reference) = delete;
    void operator=(const BunkerAssembly &rhs) = delete;

};

} // namespace RTE

#endif // File