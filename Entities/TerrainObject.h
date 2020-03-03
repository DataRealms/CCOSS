#ifndef _RTETERRAINOBJECT_
#define _RTETERRAINOBJECT_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            TerrainObject.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Header file for the TerrainObject class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "RTETools.h"
#include "SceneObject.h"
#include "Vector.h"
#include "FrameMan.h"
#include "SceneMan.h"
//#include "MovableMan.h"

namespace RTE
{

class ContentFile;


//////////////////////////////////////////////////////////////////////////////////////////
// Class:           TerrainObject
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     A feature of the terrain, which includes foreground color layer,
//                  material layer and optional background layer.
// Parent(s):       SceneObject.
// Class history:   08/23/2002 TerrainObject created.

class TerrainObject:
    public SceneObject
{


//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable, method and friend function declarations

public:


// Concrete allocation and cloning definitions
ENTITYALLOCATION(TerrainObject)


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     TerrainObject
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a TerrainObject object in system
//                  memory. Create() should be called before using the object.
// Arguments:       None.

    TerrainObject() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Destructor:      ~TerrainObject
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destructor method used to clean up a TerrainObject object before deletion
//                  from system memory.
// Arguments:       None.

    virtual ~TerrainObject() { Destroy(true); }


//////////////////////////////////////////////////////////////////////////////////////////
// Pure V. method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the TerrainObject object ready for use.
// Arguments:       None.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Create();

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the TerrainObject object ready for use.
// Arguments:       The ContentFile representing the bitmap file to load as this
//                  TerrainObject's graphical representation. Ownership is transferred.
//                  Whether to draw transperently using a color key specified by the pixel
//                  in the upper left corner of the loaded bitmap.
//                  The initial scroll offset.
//                  Two bools that define whether the layer should wrap around or stop when
//                  scrolling beyond its bitmap's boundries.
//                  A vector whose components defines two different things, depending on
//                  wrapX/Y arguments. If a wrap argument is set to false, the
//                  corresponding component here will be interpreted as the width (X) or
//                  height (Y) (in pixels) of the total bitmap area that this layer is
//                  allowed to scroll across before stopping at an edge. If wrapping is
//                  set to true, the value in scrollInfo is simply the ratio of offset at
//                  which any scroll operations will be done in. A special command is if
//                  wrap is false and the corresponding component is -1.0, that signals
//                  that the own width or height should be used as scrollInfo input.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

// TODO: streamline interface")
    virtual int Create(ContentFile *pBitmapFile,
                       bool drawTrans,
                       Vector offset,
                       bool wrapX,
                       bool wrapY,
                       Vector scrollInfo);
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a TerrainObject to be identical to another, by deep copy.
// Arguments:       A reference to the TerrainObject to deep copy.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    int Create(const TerrainObject &reference);


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
// Description:     Resets the entire TerrainObject, including its inherited members, to
//                  their default settings or values.
// Arguments:       None.
// Return value:    None.

    virtual void Reset() { Clear(); SceneObject::Reset(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Pure V. method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this TerrainObject to an output stream for
//                  later recreation with Create(Reader &reader);
// Arguments:       A Writer that the TerrainObject will save itself with.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Save(Writer &writer) const;

/*
//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the TerrainObject object ready for use.
// Arguments:       An input stream that the TerrainObject will create itself from.
//                  Whether there is a class name in the stream to check against to make
//                  sure the correct type is being read from the stream.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Create(std::istream &stream, bool checkType = true);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the complete state of this TerrainObject to an output stream for
//                  later recreation with Create(istream &stream);
// Arguments:       An output stream that the TerrainObject will save itself to.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

    virtual int Save(std::ostream &stream) const;
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Pure V. method:  Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the TerrainObject object.
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
// Virtual method:  GetBitmapOffset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the offset from teh position to the upper left corner of this'
//                  bitmaps.
// Arguments:       None.
// Return value:    A Vector describing the bitmap offset, in pixels.

    virtual const Vector & GetBitmapOffset() const { return m_BitmapOffset; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:			GetBitmapWidth
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the width this' material bitmap.
// Arguments:       None.
// Return value:    Width of 'material' bitmap.

	const int GetBitmapWidth() const { return m_pMaterial->w; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:			GetBitmapHeight
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the height this' material bitmap.
// Arguments:       None.
// Return value:    Height of 'material' bitmap.

	const int GetBitmapHeight() const { return m_pMaterial->h; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetFGColorBitmap
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the BITMAP object that this TerrainObject uses for its fore-
//                  ground color representation.
// Arguments:       None.
// Return value:    A pointer to the foreground color BITMAP object. Ownership is not
//                  transferred.

    BITMAP * GetFGColorBitmap() const { return m_pFGColor; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetBGColorBitmap
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the BITMAP object that this TerrainObject uses for its back-
//                  ground color representation, if any. 
// Arguments:       None.
// Return value:    A pointer to the background color BITMAP object. Ownership is not
//                  transferred. This may be 0 if there is no BG info!

    BITMAP * GetBGColorBitmap() const { return m_pBGColor; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetMaterialBitmap
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the BITMAP object that this TerrainObject uses for its
//                  material representation.
// Arguments:       None.
// Return value:    A pointer to the material BITMAP object. Ownership is not
//                  transferred.

    BITMAP * GetMaterialBitmap() const { return m_pMaterial; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          HasBGColor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns whetehr this TerrainObject has any background color data.
// Arguments:       None.
// Return value:    Whether this TerrainOBject has any background color data.

    bool HasBGColor() const { return m_pBGColor != 0; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetGraphicalIcon
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets a bitmap showing a good identifyable icon of this, for use in
//                  GUI lists etc.
// Arguments:       None.
// Return value:    A good identifyable graphical representation of this in a BITMAP, if
//                  available. If not, 0 is returned. Ownership is NOT TRANSFERRED!

    virtual BITMAP * GetGraphicalIcon();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetChildObjects
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the list of child objects that should be placed when this is
//                  placed. Ownership of the list is NOT transferred!
// Arguments:       None.
// Return value:    A reference to the list of child objects.

    const std::list<SOPlacer> & GetChildObjects() const { return m_ChildObjects; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetTeam
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets which team this belongs to.
// Arguments:       The assigned team number.
// Return value:    None.

    virtual void SetTeam(int team);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  IsOnScenePoint
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether this' current graphical representation overlaps
//                  a point in absolute scene coordinates.
// Arguments:       The point in absolute scene coordinates.
// Return value:    Whether this' graphical rep overlaps the scene point.

    virtual bool IsOnScenePoint(Vector &scenePoint) const;


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

    virtual void Draw(BITMAP *pTargetBitmap,
                      const Vector &targetPos = Vector(),
                      DrawMode mode = g_DrawColor,
                      bool onlyPhysical = false) const;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetDisplayAsTerrain
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns whether this bunker module should be drawn as terrain object on minimap
//                  
// Arguments:       None.
// Return value:    Returns true if this module should be drawn as part of terrain on minimap.

	virtual bool const GetDisplayAsTerrain() { return m_DisplayAsTerrain; }


//////////////////////////////////////////////////////////////////////////////////////////
// Protected member variable and method declarations

protected:

    // Member variables
    static Entity::ClassInfo m_sClass;

    ContentFile m_FGColorFile;
    ContentFile m_MaterialFile;
    ContentFile m_BGColorFile;

    // Not owned by this
    BITMAP *m_pFGColor;
    BITMAP *m_pMaterial;
    BITMAP *m_pBGColor;

    // Offset from the position of this to the top left corner of the bitmap.
    // The inverson of this should point to a corner or pattern in the bitmaps which will snap well with a 24px grid
    Vector m_BitmapOffset;
    // Whether the offset has been defined and shuoldn't be automatically set
    bool m_OffsetDefined;

    // The objects that are placed along with this in the scene
    std::list<SOPlacer> m_ChildObjects;

	// If true, then this bunker module will be drawn as part of terrain on preview
	bool m_DisplayAsTerrain;

//////////////////////////////////////////////////////////////////////////////////////////
// Private member variable and method declarations

private:

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this TerrainObject, effectively
//                  resetting the members of this abstraction level only.
// Arguments:       None.
// Return value:    None.

    void Clear();


    // Disallow the use of some implicit methods.
    TerrainObject(const TerrainObject &reference) { RTEAbort("Tried to use forbidden method"); }
    void operator=(const TerrainObject &rhs) { RTEAbort("Tried to use forbidden method"); }

};

} // namespace RTE

#endif // File