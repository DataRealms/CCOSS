#ifndef _RTESCENEOBJECT_
#define _RTESCENEOBJECT_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            SceneObject.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Header file for the SceneObject class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files, forward declarations, namespace stuff

#include "Entity.h"
#include "Matrix.h"

struct BITMAP;

namespace RTE
{

//////////////////////////////////////////////////////////////////////////////////////////
// Abstract class:  SceneObject
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     The base class shared by Both TerrainObject:s and MovableObject:s, ie
//                  anything that can be places in a scene.
// Parent(s):       Entity.
// Class history:   8/6/2007 SceneObject created.

class SceneObject : public Entity {


//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable, method and friend function declarations

public:

	SerializableOverrideMethods;
	ClassInfoGetters;

	/// <summary>
	/// Enumeration for the different buyable modes of this SceneObject.
	/// </summary>
	enum class BuyableMode { NoRestrictions, BuyMenuOnly, ObjectPickerOnly };


    //////////////////////////////////////////////////////////////////////////////////////////
    // Nested class:    SOPlacer
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Acts as a small memory object that only holds a pointer to a reference
    //                  instance and the most essential properties to eventually place a copy
    //                  of that reference when needed.
    // Parent(s):       Serializable.
    // Class history:   11/25/2007 SOPlacer created.

    class SOPlacer:
        public Serializable
    {

    //////////////////////////////////////////////////////////////////////////////////////////
    // Public member variable, method and friend function declarations

    public:

		SerializableClassNameGetter;
		SerializableOverrideMethods;


    //////////////////////////////////////////////////////////////////////////////////////////
    // Constructor:     SOPlacer
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Constructor method used to instantiate a SOPlacer object in system
    //                  memory. Create() should be called before using the object.
    // Arguments:       None.

        SOPlacer() { Clear(); }


    //////////////////////////////////////////////////////////////////////////////////////////
    // Virtual method:  Create
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Creates a SOPlacer to be identical to another, by deep copy.
    // Arguments:       A reference to the SOPlacer to deep copy.
    // Return value:    An error return value signaling sucess or any particular failure.
    //                  Anything below 0 is an error signal.

        int Create(const SOPlacer &reference);


    //////////////////////////////////////////////////////////////////////////////////////////
    // Virtual method:  Reset
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Resets the entire Serializable, including its inherited members, to their
    //                  default settings or values.
    // Arguments:       None.
    // Return value:    None.

		void Reset() override { Clear(); }


    //////////////////////////////////////////////////////////////////////////////////////////
    // Virtual method:  GetObjectReference
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Gets the object reference to be placed. Owenership is NOT transferred!
    // Arguments:       None.
    // Return value:    A pointer to the reference object to be copied and placed. Not transferred!

		const SceneObject * GetObjectReference() { return m_pObjectReference; }


    //////////////////////////////////////////////////////////////////////////////////////////
    // Method:          GetOffset
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Gets the place offset from the parent's position/origin. If in a scene
    //                  this will yield the absolute scene coordinates.
    // Arguments:       None.
    // Return value:    The offset in pixels from the parent's position where this gets spawned.

        Vector GetOffset() const { return m_Offset; }



    //////////////////////////////////////////////////////////////////////////////////////////
    // Method:          SetOffset
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Sets the place offset from the parent's position/origin.
    // Arguments:       New offset.
    // Return value:    None.

        void SetOffset(Vector newOffset) { m_Offset = newOffset; }


    //////////////////////////////////////////////////////////////////////////////////////////
    // Method:          GetRotation
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Gets the rotation angle of the object to be placed, in radians.
    // Arguments:       None.
    // Return value:    The placement rotational angle, in radians.

        float GetRotation() const { return m_RotAngle; }


    //////////////////////////////////////////////////////////////////////////////////////////
    // Method:          GetHFlipped
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Gets whether the placement is horizontally flipped or not.
    // Arguments:       None.
    // Return value:    The horizontal flipping of the placement.

        bool GetHFlipped() const { return m_HFlipped; }


    //////////////////////////////////////////////////////////////////////////////////////////
    // Method:          GetTeam
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Gets which team this is to be assigned to when placed.
    // Arguments:       None.
    // Return value:    The team number this is to be assigned to when placed.

        int GetTeam() const { return m_Team; }


    //////////////////////////////////////////////////////////////////////////////////////////
    // Method:          SetTeam
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Sets which team this is to be assigned to when placed.
    // Arguments:       The team number this is to be assigned to when placed.
    // Return value:    None.

        void SetTeam(int team) { m_Team = team; }


    //////////////////////////////////////////////////////////////////////////////////////////
    // Method:          GetPlacedCopy
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Makes a copy of the preset instance, and applies the placement
    //                  properties of this to it, finally returning it WITH OWNERSHIP.
    // Arguments:       The parent to place as offset from. If 0 is passed, the placement
    //                  properties will be applied as absolutes instead of relative.
    // Return value:    The new copy with correct placement applied. OWNERSHIP IS TRANSFERRED!

        SceneObject * GetPlacedCopy(const SceneObject *pParent = 0) const;

    //////////////////////////////////////////////////////////////////////////////////////////
    // Protected member variable and method declarations

    protected:

        // The pointer to the preset instance, that copies of which will be placed. Not Owned!
        const SceneObject *m_pObjectReference;
        // Offset placement position from owner/parent's position/origin.
        Vector m_Offset;
        // The placement's rotational angle in radians.
        float m_RotAngle;
        // Whether horizontal flipping is part of the placement.
        bool m_HFlipped;
        // The team of the placed object
        int m_Team;


    //////////////////////////////////////////////////////////////////////////////////////////
    // Private member variable and method declarations

    private:

		static const std::string c_ClassName; //!< A string with the friendly-formatted type name of this object.

    //////////////////////////////////////////////////////////////////////////////////////////
    // Method:          Clear
    //////////////////////////////////////////////////////////////////////////////////////////
    // Description:     Clears all the member variables of this SOPlacer, effectively
    //                  resetting the members of this abstraction level only.
    // Arguments:       None.
    // Return value:    None.

        void Clear();

    };


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     SceneObject
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a SceneObject object in system
//                  memory. Create() should be called before using the object.
// Arguments:       None.

    SceneObject() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Destructor:      ~SceneObject
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destructor method used to clean up a SceneObject object before deletion
//                  from system memory.
// Arguments:       None.

	~SceneObject() override { Destroy(true); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the SceneObject object ready for use.
// Arguments:       None.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

	int Create() override { return 0; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates an SceneObject to be identical to another, by deep copy.
// Arguments:       A reference to the SceneObject to deep copy.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

	int Create(const SceneObject &reference);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Reset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Resets the entire SceneObject, including its inherited members, to their
//                  default settings or values.
// Arguments:       None.
// Return value:    None.

	void Reset() override { Clear(); Entity::Reset(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Pure V. method:  Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the SceneObject object.
// Arguments:       Whether to only destroy the members defined in this derived class, or
//                  to destroy all inherited members also.
// Return value:    None.

	void Destroy(bool notInherited = false) override;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetPos
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the absolute position of this SceneObject.
// Arguments:       None.
// Return value:    A Vector describing the current absolute position in pixels.

	const Vector & GetPos() const { return m_Pos; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetPos
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the absolute position of this SceneObject in the scene.
// Arguments:       A Vector describing the current absolute position in pixels.
// Return value:    None.

	void SetPos(const Vector &newPos) { m_Pos = newPos; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  IsHFlipped
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns whether this is being drawn flipped horizontally (around the
//                  vertical axis), or not.
// Arguments:       None.
// Return value:    Whether flipped or not.

    virtual bool IsHFlipped() const { return false; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetRotMatrix
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the current rotational Matrix of of this MovableObject.
// Arguments:       None.
// Return value:    The rotational Matrix of this MovableObject.

    virtual Matrix GetRotMatrix() const { return Matrix(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetRotAngle
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the current rotational angle of of this, in radians.
// Arguments:       None.
// Return value:    The rotational angle of this, in radians.

    virtual float GetRotAngle() const { return 0; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virutal method:  SetHFlipped
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets whether this should be drawn flipped horizontally (around the
//                  vertical axis).
// Arguments:       A bool with the new value.
// Return value:    None.

    virtual void SetHFlipped(const bool flipped) {}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetRotAngle
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the current absolute angle of rotation of this.
// Arguments:       The new absolute angle in radians.
// Return value:    None.

    virtual void SetRotAngle(float newAngle) {}


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetTeam
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets which team this belongs to.
// Arguments:       The assigned team number.
// Return value:    None.

	virtual void SetTeam(int team) { m_Team = team; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetTeam
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets which team this belongs to.
// Arguments:       None.
// Return value:    The currently assigned team number.

	int GetTeam() const { return m_Team; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetPlacedByPlayer
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets which player placed this object in the scene, if any.
// Arguments:       The player responsible for placing this is in the scene, if any.
// Return value:    None.

	void SetPlacedByPlayer(int player) { m_PlacedByPlayer = player; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetPlacedByPlayer
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets which player placed this object in the scene, if any.
// Arguments:       None.
// Return value:    The player responsible for placing this is in the scene, if any.

	int GetPlacedByPlayer() const { return m_PlacedByPlayer; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetGoldValue
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the cost to purchase this item, in oz's of gold.
// Arguments:       If this is supposed to be adjusted for a specific Tech's subjective
//                  value, then pass in the native DataModule ID of that tech. 0 means
//                  no Tech is specified and the base value is returned.
//                  How much to multiply the value if this happens to be a foreign Tech.
//                  How much to multiply the value if this happens to be a native Tech.
// Return value:    The cost, in oz of gold.

    virtual float GetGoldValue(int nativeModule = 0, float foreignMult = 1.0, float nativeMult = 1.0) const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetGoldValue
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the cost to purchase this item, in oz's of gold.
// Arguments:       The cost, in oz of gold.
// Return value:    None.

	void SetGoldValue(float value) { m_OzValue = value; } 


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetGoldValueOld
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     DOES THE SAME THING AS GetGoldValue, USED ONLY TO PRESERVE LUA COMPATIBILITY

	virtual float GetGoldValueOld(int nativeModule, float foreignMult) const { return GetGoldValue(nativeModule, foreignMult, 1.0); }

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetGoldValueString
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets a descriptive string describing the cost to purchase this item,
//                  in oz's of gold.
// Arguments:       If this is supposed to be adjusted for a specific Tech's subjective
//                  value, then pass in the native DataModule ID of that tech. 0 means
//                  no Tech is specified and the base value is returned.
//                  How much to multiply the value if this happens to be a foreign Tech.
// Return value:    The cost, described in a friendly to read string: "100oz", or "Free"

    std::string GetGoldValueString(int nativeModule = 0, float foreignMult = 1.0, float nativeMult = 1.0) const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetTotalValue
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the total liquidation value of this, including everything inside.
// Arguments:       If this is supposed to be adjusted for a specific Tech's subjective
//                  value, then pass in the native DataModule ID of that tech. 0 means
//                  no Tech is specified and the base value is returned.
//                  How much to multiply the value if this happens to be a foreign Tech.
// Return value:    The current value of this and all contained assets.

    virtual float GetTotalValue(int nativeModule = 0, float foreignMult = 1.0, float nativeMult = 1.0) const { return GetGoldValue(nativeModule, foreignMult, nativeMult); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          IsBuyable
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Shows whether this should appear in teh buy menus at all.
// Arguments:       None.
// Return value:    Buyable or not.

    bool IsBuyable() const { return m_Buyable; }

	/// <summary>
	/// Gets whether this SceneObject is available only in the BuyMenu list when buyable.
	/// </summary>
	/// <returns>Whether this SceneObject is available only in the BuyMenu list when buyable.</returns>
	bool IsBuyableInBuyMenuOnly() const { return m_BuyableMode == BuyableMode::BuyMenuOnly; }

	/// <summary>
	/// Gets whether this SceneObject is available only in the ObjectPicker list when buyable.
	/// </summary>
	/// <returns>Whether this SceneObject is available only in the ObjectPicker list when buyable.</returns>
	bool IsBuyableInObjectPickerOnly() const { return m_BuyableMode == BuyableMode::ObjectPickerOnly; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetGraphicalIcon
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets a bitmap showing a good identifyable icon of this, for use in
//                  GUI lists etc.
// Arguments:       None.
// Return value:    A good identifyable graphical representation of this in a BITMAP, if
//                  available. If not, 0 is returned. Ownership is NOT TRANSFERRED!

    virtual BITMAP * GetGraphicalIcon() const { return nullptr; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  IsOnScenePoint
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether this' current graphical representation overlaps
//                  a point in absolute scene coordinates.
// Arguments:       The point in absolute scene coordinates.
// Return value:    Whether this' graphical rep overlaps the scene point.

    virtual bool IsOnScenePoint(Vector &scenePoint) const  { return false; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates this SceneObject. Supposed to be done every frame.
// Arguments:       None.
// Return value:    None.

	virtual void Update() { return; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this SceneObject's current graphical representation to a BITMAP of
//                  choice.
// Arguments:       A pointer to a BITMAP to draw on.
//                  The absolute position of the target bitmap's upper left corner in the Scene.
//                  In which mode to draw in. See the DrawMode enumeration for the modes.
//                  Whether to not draw any extra 'ghost' items of this MovableObject,
//                  like indicator arrows or hovering HUD text and so on.
// Return value:    None.

	virtual void Draw(BITMAP *pTargetBitmap, const Vector &targetPos = Vector(), DrawMode mode = g_DrawColor, bool onlyPhysical = false) const = 0;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  DrawTeamMark
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws team sign this terrain object belongs to.
// Arguments:       A pointer to a BITMAP to draw on.
//                  The absolute position of the target bitmap's upper left corner in the Scene.
// Return value:    None.

	void DrawTeamMark(BITMAP *pTargetBitmap, const Vector &targetPos = Vector()) const;

//////////////////////////////////////////////////////////////////////////////////////////
// Protected member variable and method declarations

protected:

    // Forbidding copying
	SceneObject(const SceneObject &reference) = delete;
    SceneObject & operator=(const SceneObject &rhs) { return *this; }


    // Member variables
    static Entity::ClassInfo m_sClass;
    // Absolute position of the center of this in the scene, in pixels
    Vector m_Pos;
    // How much this SceneObject costs to purchase, in oz's of gold.
    float m_OzValue;
    // Whether this shows up in the buy menu at all
    bool m_Buyable;

	BuyableMode m_BuyableMode; //!< In which buy lists this SceneObject is available when buyable.

    // The team this object belongs to. -1 if none.
    int m_Team;
    // The player this was placed by in edit mode
    int m_PlacedByPlayer;


//////////////////////////////////////////////////////////////////////////////////////////
// Private member variable and method declarations

private:

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this SceneObject, effectively
//                  resetting the members of this abstraction level only.
// Arguments:       None.
// Return value:    None.

    void Clear();

};

} // namespace RTE

#endif // File