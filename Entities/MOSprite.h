#ifndef _RTEMOSPRITE_
#define _RTEMOSPRITE_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            MOSprite.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Header file for the MOSprite class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "MovableObject.h"
#include "Box.h"

namespace RTE
{

class AEmitter;


//////////////////////////////////////////////////////////////////////////////////////////
// Abstract class:  MOSprite
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     A movable object with mass that is graphically represented by a
//                  BITMAP.
// Parent(s):       MovableObject.
// Class history:   03/18/2001 MOSprite created.

class MOSprite:
    public MovableObject
{


//////////////////////////////////////////////////////////////////////////////////////////
// Public member variable, method and friend function declarations

public:

	SerializableOverrideMethods
	ClassInfoGetters

    enum SpriteAnimMode
    {
        NOANIM = 0,
        ALWAYSLOOP,
        ALWAYSRANDOM,
        ALWAYSPINGPONG,
        LOOPWHENMOVING,
        LOOPWHENOPENCLOSE,
        PINGPONGOPENCLOSE,
		OVERLIFETIME,
		ONCOLLIDE,
        SpriteAnimModeCount
    };


//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     MOSprite
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a MOSprite object in system
//                  memory. Create() should be called before using the object.
// Arguments:       None.

    MOSprite() { Clear(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Destructor:      ~MOSprite
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destructor method used to clean up a MOSprite object before deletion
//                  from system memory.
// Arguments:       None.

	~MOSprite() override { Destroy(true); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the MOSprite object ready for use.
// Arguments:       A pointer to ContentFile that represents the bitmap file that will be
//                  used to create the Sprite.
//                  The number of frames in the Sprite's animation.
//                  A float specifying the object's mass in Kilograms (kg).
//                  A Vector specifying the initial position.
//                  A Vector specifying the initial velocity.
//                  The amount of time in ms this MovableObject will exist. 0 means unlim.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

	int Create(ContentFile spriteFile, const int frameCount = 1, const float mass = 1, const Vector &position = Vector(0, 0), const Vector &velocity = Vector(0, 0), const unsigned long lifetime = 0);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates a MOSprite to be identical to another, by deep copy.
// Arguments:       A reference to the MOSprite to deep copy.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

	int Create(const MOSprite &reference);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the MOSprite object ready for use.
// Arguments:       None.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

   int Create() override;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Reset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Resets the entire MOSprite, including its inherited members, to their
//                  default settings or values.
// Arguments:       None.
// Return value:    None.

    void Reset() override { Clear(); MovableObject::Reset(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the MOSprite object.
// Arguments:       Whether to only destroy the members defined in this derived class, or
//                  to destroy all inherited members also.
// Return value:    None.

    void Destroy(bool notInherited = false) override;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetRadius
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the largest radius of this in pixels.
// Arguments:       None.
// Return value:    The radius from its center to the edge of its graphical representation.

	float GetRadius() const override { return m_SpriteRadius; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetDiameter
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the largest diameter of this in pixels.
// Arguments:       None.
// Return value:    The largest diameter across its graphical representation.

    float GetDiameter() const override { return m_SpriteDiameter; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetAboveHUDPos
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the absoltue position of the top of this' HUD stack.
// Arguments:       None.
// Return value:    A Vector with the absolute position of this' HUD stack top point.

	Vector GetAboveHUDPos() const override { return m_Pos + Vector(0, -GetRadius()); }

// TODO: Improve this one! Really crappy fit
//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetBoundingBox
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the oriented bounding box which is guaranteed to contain this,
//                  taking rotation etc into account. It's not guaranteed to be fit
//                  perfectly though. TODO: MAKE FIT BETTER
// Arguments:       None.
// Return value:    A Box which is guaranteed to contain this. Does nto take wrapping into
//                  account, and parts of this box may be out of bounds!

	Box GetBoundingBox() const { return Box(m_Pos + Vector(-GetRadius(), -GetRadius()), GetDiameter(), GetDiameter()); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetSpriteFrame
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets a frame of this MOSprite's BITMAP array.
// Arguments:       Which frame to get.
// Return value:    A pointer to the requested frame of this MOSprite's BITMAP array.
//                  Ownership is NOT transferred!

    BITMAP * GetSpriteFrame(int whichFrame = 0) const { return (whichFrame >= 0 && whichFrame < m_FrameCount) ? m_aSprite[whichFrame] : 0; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetSpriteWidth
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the width of the bitmap of this MOSprite
// Arguments:       0.
// Return value:    Sprite width if loaded.

    int GetSpriteWidth() const { return m_aSprite[0] ? m_aSprite[0]->w : 0; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetSpriteHeight
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the height of the bitmap of this MOSprite
// Arguments:       0.
// Return value:    Sprite height if loaded.

    int GetSpriteHeight() const { return m_aSprite[0] ? m_aSprite[0]->h : 0; }



//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetFrameCount
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the number of frames in this MOSprite's animation.
// Arguments:       None.
// Return value:    The frame count.

    int GetFrameCount() const { return m_FrameCount; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetSpriteOffset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the offset that the BITMAP has from the position of this
//                  MOSprite.
// Arguments:       None.
// Return value:    A vector with the offset.

    Vector GetSpriteOffset() const { return m_SpriteOffset; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  IsHFlipped
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns whether this MOSprite is being drawn flipped horizontally
//                  (along the vertical axis), or not.
// Arguments:       None.
// Return value:    Whether flipped or not.

	bool IsHFlipped() const override { return m_HFlipped; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetRotMatrix
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the current rotational Matrix of of this.
// Arguments:       None.
// Return value:    The rotational Matrix of this MovableObject.

	Matrix GetRotMatrix() const override { return m_Rotation; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetRotAngle
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the current rotational angle of of this, in radians.
// Arguments:       None.
// Return value:    The rotational angle of this, in radians.

	float GetRotAngle() const override { return m_Rotation.GetRadAngle(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetAngularVel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the current angular velocity of this MovableObject. Positive is
//                  a counter-clockwise rotation.
// Arguments:       None.
// Return value:    The angular velocity in radians per second.

	float GetAngularVel() const override { return m_AngularVel; }


/* Can't do this since sprite is owned by ContentMan.
//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetSprite
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Replaces this MOSprite's BITMAP and deletes the old one.
// Arguments:       A pointer to a BITMAP.
// Return value:    None.

    void SetSprite(BITMAP *pSprite) { delete m_aSprite; m_aSprite = pSprite; }
*/

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetSpriteOffset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the offset that the BITMAP has from the position of this
//                  MOSprite.
// Arguments:       A vector with the new offset.
// Return value:    None.

    void SetSpriteOffset(const Vector &newOffset) { m_SpriteOffset = newOffset; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetFrame
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Hard-sets the frame this sprite is supposed to show.
// Arguments:       An unsigned int pecifiying the new frame.
// Return value:    None.

    void SetFrame(unsigned int newFrame);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetNextFrame
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Hard-sets the frame this sprite is supposed to show, to the
//                  consecutive one after the current one. If currently the last fame is
//                  this will set it to the be the first, looping the animation.
// Arguments:       None.
// Return value:    Whether the animation looped or not with this setting.

    bool SetNextFrame();


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetFrame
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tells which frame is currently set to show.
// Arguments:       None.
// Return value:    An unsigned int describing the current frame.

    unsigned int GetFrame() const { return m_Frame; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetSpriteAnimMode
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the animation mode.
// Arguments:       The animation mode we want to set.
// Return value:    None.

    void SetSpriteAnimMode(int animMode = NOANIM) { m_SpriteAnimMode = (SpriteAnimMode)animMode; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetSpriteAnimMode
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the animation mode.
// Arguments:       None.
// Return value:    The animation mode currently in effect.

    int GetSpriteAnimMode() const { return m_SpriteAnimMode; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virutal method:  SetHFlipped
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets whether this MOSprite should be drawn flipped horizontally
//                  (along the vertical axis).
// Arguments:       A bool with the new value.
// Return value:    None.

	void SetHFlipped(const bool flipped) override { m_HFlipped = flipped; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetRotAngle
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the current absolute angle of rotation of this MovableObject.
// Arguments:       The new absolute angle in radians.
// Return value:    None.

	void SetRotAngle(float newAngle) override { m_Rotation.SetRadAngle(newAngle); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetAngularVel
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the current angular velocity of this MovableObject. Positive is
//                  a counter clockwise rotation.
// Arguments:       The new angular velocity in radians per second.
// Return value:    None.

	void SetAngularVel(float newRotVel) override { m_AngularVel = newRotVel; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetGraphicalIcon
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets a bitmap showing a good identifyable icon of this, for use in
//                  GUI lists etc.
// Arguments:       None.
// Return value:    A good identifyable graphical representation of this in a BITMAP, if
//                  available. If not, 0 is returned. Ownership is NOT TRANSFERRED!

    BITMAP * GetGraphicalIcon() override { return m_aSprite[0]; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  IsTooFast
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether this MO is moving or rotating stupidly fast in a way
//                  that will screw up the simulation.
// Arguments:       None.
// Return value:    Whether this is either moving or rotating too fast.

	bool IsTooFast() const override { return m_Vel.GetLargest() > 500.0f || fabs(m_AngularVel) > (2000.0f / (GetRadius() + 1.0f)); }
    //bool IsTooFast() const override { return m_Vel.GetLargest() > 500 || fabs(m_AngularVel) > 100.0f; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  FixTooFast
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Slows the speed of anyhting that is deemed to be too fast to within
//                  acceptable rates.
// Arguments:       None.
// Return value:    None.

	void FixTooFast() override { while (IsTooFast()) { m_Vel *= 0.5; m_AngularVel *= 0.5; } }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  IsOnScenePoint
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether this' current graphical representation overlaps
//                  a point in absolute scene coordinates.
// Arguments:       The point in absolute scene coordinates.
// Return value:    Whether this' graphical rep overlaps the scene point.

	bool IsOnScenePoint(Vector &scenePoint) const override;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  RotateOffset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Takes a vector which is offset from the center of this when not rotated
//                  or flipped, and then rotates and/or flips it to transform it into this'
//                  'local space', if applicable.
// Arguments:       A vector which is supposed to be offset from this' center when upright.
// Return value:    The resulting vector whihch has been flipped and rotated as appropriate.

	Vector RotateOffset(const Vector &offset) const override;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  UnRotateOffset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Takes a vector which is offset from the center of this when not rotated
//                  or flipped, and then rotates and/or flips it to transform it into this'
//                  'local space', but in REVERSE.
// Arguments:       A vector which is supposed to be offset from this' center when upright.
// Return value:    The resulting vector whihch has been flipped and rotated as appropriate.

	Vector UnRotateOffset(const Vector &offset) const;

    /// <summary>
    /// Adjusts an absolute angle based on wether this MOSprite is flipped.
    /// </summary>
    /// <param name="angle">The input angle in radians.</param>
    /// <returns>The output angle in radians, which will be unaltered if this MOSprite is not flipped.</returns>
    float FacingAngle(float angle) const { return (m_HFlipped ? c_PI : 0) + (angle * static_cast<float>(GetFlipFactor())); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetEntryWound
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets entry wound emitter for this MOSprite
// Arguments:       Emitter preset name and module name
// Return value:    None

	void SetEntryWound(std::string presetName, std::string moduleName);

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetExitWound
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets exit wound emitter for this MOSprite
// Arguments:       Emitter preset name and module name
// Return value:    None

	void SetExitWound(std::string presetName, std::string moduleName);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetEntryWoundPresetName
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns entry wound emitter preset name for this MOSprite
// Arguments:       None
// Return value:    Wound emitter preset name

	std::string GetEntryWoundPresetName() const;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetExitWoundPresetName
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns exit wound emitter preset name for this MOSprite
// Arguments:       None
// Return value:    Wound emitter preset name

	std::string GetExitWoundPresetName() const;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetSpriteAnimDuration
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns animation duration in ms
// Arguments:       None
// Return value:    Animation duration in ms

	int GetSpriteAnimDuration() const { return m_SpriteAnimDuration; }

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  SetSpriteAnimDuration
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets animation duration in ms
// Arguments:       Animation duration in ms
// Return value:    Mone

	void SetSpriteAnimDuration(int newDuration) { m_SpriteAnimDuration = newDuration; }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates this MovableObject. Supposed to be done every frame.
// Arguments:       None.
// Return value:    None.

	void Update() override;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this MOSprite's current graphical representation to a
//                  BITMAP of choice.
// Arguments:       A pointer to a BITMAP to draw on.
//                  The absolute position of the target bitmap's upper left corner in the Scene.
//                  In which mode to draw in. See the DrawMode enumeration for the modes.
//                  Whether to not draw any extra 'ghost' items of this MovableObject,
//                  indicator arrows or hovering HUD text and so on.
// Return value:    None.

	void Draw(BITMAP *pTargetBitmap, const Vector &targetPos = Vector(), DrawMode mode = g_DrawColor, bool onlyPhysical = false) const override;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetFlipFactor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns a positive or negative number value to multiply with for external calculations. 
// Arguments:       None.
// Return value:    1 for not flipped, -1 for flipped.

	int GetFlipFactor() const { return m_HFlipped ? -1 : 1; }


//////////////////////////////////////////////////////////////////////////////////////////
// Protected member variable and method declarations

protected:

    // Member variables
    static Entity::ClassInfo m_sClass;

    Matrix m_Rotation; // Rotational matrix of this MovableObject.
    Matrix m_PrevRotation; // Rotational matrix of this MovableObject, last frame.
    float m_AngularVel; // The angular velocity by which this MovableObject rotates, in radians per second (r/s).
    float m_PrevAngVel; // Previous frame's angular velocity.
    ContentFile m_SpriteFile;
    // Array of pointers to BITMAP:s representing the multiple frames of this sprite
    BITMAP **m_aSprite;
    // Number of frames, or elements in the m_aSprite array.
    unsigned int m_FrameCount;
    Vector m_SpriteOffset;
    // State, which frame of sprite to be drawn.
    unsigned int m_Frame;
    // Which animation to use for the body
    SpriteAnimMode m_SpriteAnimMode;
    // Body animation duration, how long in ms that each full body animation cycle takes.
    int m_SpriteAnimDuration;
    // The timer to keep track of the body animation
    Timer m_SpriteAnimTimer;
    // Keep track of animation direction (mainly for ALWAYSPINGPONG), true is decreasing frame, false is increasing frame
    bool m_SpriteAnimIsReversingFrames;
    // Whether flipped horizontally or not.
    bool m_HFlipped;
    // The precalculated maximum possible radius and diameter of this, in pixels
    float m_SpriteRadius;
    float m_SpriteDiameter;
    // A counter to count the oscillations in rotation, in order to detect settling.
    int m_AngOscillations;
    // Whether to disable the settle material ID when this gets drawn as material
    bool m_SettleMaterialDisabled;
    // Entry wound template
    const AEmitter *m_pEntryWound;
    // Exit wound template
    const AEmitter *m_pExitWound;


//////////////////////////////////////////////////////////////////////////////////////////
// Private member variable and method declarations

private:

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this MOSprite, effectively
//                  resetting the members of this abstraction level only.
// Arguments:       None.
// Return value:    None.

    void Clear();


    // Disallow the use of some implicit methods.
	MOSprite(const MOSprite &reference) = delete;
	MOSprite & operator=(const MOSprite &rhs) = delete;

};

} // namespace RTE

#endif // File