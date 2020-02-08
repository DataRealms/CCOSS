#ifndef _RTEPEMITTER_
#define _RTEPEMITTER_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            PEmitter.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Header file for the PEmitter class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "MOSParticle.h"
#include "Emission.h"

namespace RTE
{


//////////////////////////////////////////////////////////////////////////////////////////
// Class:           PEmitter
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     
// Parent(s):       MOSParticle.
// Class history:   02/29/2004 PEmitter created.

class PEmitter :
	public MOSParticle
{


	//////////////////////////////////////////////////////////////////////////////////////////
	// Public member variable, method and friend function declarations

public:


	// Concrete allocation and cloning definitions
	ENTITYALLOCATION(PEmitter)


		//////////////////////////////////////////////////////////////////////////////////////////
		// Constructor:     PEmitter
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Constructor method used to instantiate a PEmitter object in system
		//                  memory. Create() should be called before using the object.
		// Arguments:       None.

		PEmitter() { Clear(); }


	//////////////////////////////////////////////////////////////////////////////////////////
	// Destructor:      ~PEmitter
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Destructor method used to clean up a PEmitter object before deletion
	//                  from system memory.
	// Arguments:       None.

	virtual ~PEmitter() { Destroy(true); }


	//////////////////////////////////////////////////////////////////////////////////////////
	// Virtual method:  Create
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Makes the PEmitter object ready for use.
	// Arguments:       None.
	// Return value:    An error return value signaling sucess or any particular failure.
	//                  Anything below 0 is an error signal.

	virtual int Create();


	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:          Create
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Creates a PEmitter to be identical to another, by deep copy.
	// Arguments:       A reference to the PEmitter to deep copy.
	// Return value:    An error return value signaling sucess or any particular failure.
	//                  Anything below 0 is an error signal.

	int Create(const PEmitter &reference);


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
	// Description:     Resets the entire PEmitter, including its inherited members, to their
	//                  default settings or values.
	// Arguments:       None.
	// Return value:    None.

	virtual void Reset() { Clear(); MOSParticle::Reset(); }


	//////////////////////////////////////////////////////////////////////////////////////////
	// Virtual method:  Save
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Saves the complete state of this PEmitter to an output stream for
	//                  later recreation with Create(Reader &reader);
	// Arguments:       A Writer that the PEmitter will save itself with.
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


	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:          IsEmitting
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Indicates whether this PEmitter is currently enabled and emitting.
	// Arguments:       None.
	// Return value:    Whether it's emitting or not.

	bool IsEmitting() const { return m_EmitEnabled; }


	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:          ResetEmissionTimers
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Reset the timers of all emissions so they will start/stop at the 
	//                  correct relative offsets from now.
	// Arguments:       None.
	// Return value:    None.

	void ResetEmissionTimers();


	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:          EnableEmission
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Sets this PEmitter to start emitting at the set rate, or to stop.
	// Arguments:       Whether to enable or disable emission.
	// Return value:    None.

	void EnableEmission(bool enable = true);


	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:          EstimateImpulse
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Calculates the forces this emitter applies on any parent.
	// Arguments:       Whether to calculate a burst update or not.
	// Return value:    The approximate impulse generated by the emitter.

	float EstimateImpulse(bool burst = false);


	/*
	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:          GetEmitRate
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Gets the rate at which this PEmitter emits its particles.
	// Arguments:       None.
	// Return value:    A float with the rate in #/min.

	float GetEmitRate() const { return m_PPM; }


	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:          GetBurstCount
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Gets the number of particles that will be emitted in one shot upon
	//                  a triggered burst of this PEmitter.
	// Arguments:       None.
	// Return value:    The number of emitted particles a burst should have. 0 means burst
	//                  are disabled.

	int GetBurstCount() const { return m_BurstSize; }
	*/

	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:          GetBurstScale
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Gets the scale factor that will be applied to the regular spread and
	//                  emission velocity to get the burst particle parameters.
	// Arguments:       None.
	// Return value:    The scale factor.

	float GetBurstScale() const { return m_BurstScale; }


	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:          GetEmitAngle
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Gets the angle of direction that the emitted particles will be shot at.
	// Arguments:       None.
	// Return value:    A float with the angle in radians.

	float GetEmitAngle() const { return m_EmitAngle.GetRadAngle(); }


	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:          GetEmitVector
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     A vector in the direction, including the rotation of the emitter, that
	//                  the emitted particles will be shot at.
	// Arguments:       None.
	// Return value:    A unit vector.

	Vector GetEmitVector() const { return Vector(1, 0).RadRotate(m_HFlipped ? c_PI + m_Rotation.GetRadAngle() - m_EmitAngle.GetRadAngle() : m_Rotation.GetRadAngle() + m_EmitAngle.GetRadAngle()); }


	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:          GetRecoilVector
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     A vector in the opposite direction, including the rotation of the 
	//                  emitter, that the emitted particles will be shot at.
	// Arguments:       None.
	// Return value:    A unit vector.

	Vector GetRecoilVector() const { return Vector(-1, 0).RadRotate(m_HFlipped ? c_PI + m_Rotation.GetRadAngle() - m_EmitAngle.GetRadAngle() : m_Rotation.GetRadAngle() + m_EmitAngle.GetRadAngle()); }


	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:          GetBurstSpacing
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Gets the BurstSpacing for this emitter.
	// Arguments:       None.
	// Return value:    The BurstSpacing in ms.

	float GetBurstSpacing() const { return m_BurstSpacing; }

	/*
	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:          GetEmitSpread
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Gets the angle spread of velocity of the emitted MO's to each side of
	//                  the angle of emission of this PEmitter.
	// Arguments:       None.
	// Return value:    A float with the spread in r's. PI/2 would mean that MO's fly out to
	//                  one side only, with the m_EmitAngle defining the middle of that half
	//                  circle.

	float GetEmitSpread() const { return m_Spread; }


	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:          GetEmitVelMin
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Gets the min end of the range the velocity of a particle being emitted
	//                  by this PEmitter can have.
	// Arguments:       None.
	// Return value:    A float with the min vel possible for an emitted particle.

	float GetEmitVelMin() const { return m_MinVelocity; }


	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:          GetEmitVelMax
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Gets the max end of the range the velocity of a particle being emitted
	//                  by this PEmitter can have.
	// Arguments:       None.
	// Return value:    A float with the max vel possible for an emitted particle.

	float GetEmitVelMax() const { return m_MaxVelocity; }
	*/

	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:          GetThrottle
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Gets the normalized throttle scalar which controls how to affect the
	//                  emission rate as per the emisison rate range. Depricated for Lua, use
	//                  the Throttle property instead.
	// Arguments:       None.
	// Return value:    A float with the normalized throttle scalar. 1.0 means max throttle,
	//                  0 means normal, -1.0 means least emission rate.

	float GetThrottle() const { return m_Throttle; }
	/*
	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:          SetEmitRate
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Sets the rate at which this PEmitter emits its particles.
	// Arguments:       A float with the rate in #/min.
	// Return value:    None.

	void SetEmitRate(const float rate) { m_PPM = rate; }


	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:          SetBurstCount
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Sets the number of particles that will be emitted in one shot upon
	//                  a triggered burst of this PEmitter.
	// Arguments:       The number of emitted particles a burst should have. 0 means burst
	//                  are disabled.
	// Return value:    None.

	void SetBurstCount(const int count) { m_BurstSize = count; }
	*/

	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:          SetBurstScale
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Sets the scale factor that will be applied to the regular spread and
	//                  emission velocity to get the burst particle parameters.
	// Arguments:       The scale factor.
	// Return value:    None.

	void SetBurstScale(const float scale) { m_BurstScale = scale; }


	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:          SetBurstSpacing
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Sets the BurstSpacing for this emitter.
	// Arguments:       The BurstSpacing in ms.
	// Return value:    None.

	void SetBurstSpacing(const float spacing) { m_BurstSpacing = spacing; }


	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:          SetFlashScale
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Sets the display scale factor of the flash effect. This is purely
	//                  visual.
	// Arguments:       The scale factor of the flash draw.
	// Return value:    None.

	void SetFlashScale(float flashScale = 1.0f) { m_FlashScale = flashScale; }


	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:          SetEmitAngle
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Sets the angle of direction that the emitted particles will be shot at.
	// Arguments:       A float with the angle in radians.
	// Return value:    None.

	void SetEmitAngle(const float angle) { m_EmitAngle.SetRadAngle(angle); }


	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:          SetThrottle
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Sets the normalized throttle scalar which controls how to affect the
	//                  emission rate as per the emisison rate range.
	// Arguments:       A float with the normalized throttle scalar. 1.0 means max throttle,
	//                  0 means normal, -1.0 means least emission rate.
	// Return value:    None.

	void SetThrottle(float throttle) { m_Throttle = throttle > 1.0f ? 1.0f : (throttle < -1.0f ? -1.0f : throttle); }

	/*
	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:          SetEmitSpread
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Sets the angle spread of velocity of the emitted MO's to each side of
	//                  angle of emission of this PEmitter.
	// Arguments:       A float with the spread in r's. PI/2 would mean that MO's fly out to
	//                  one side only, with the m_EmitAngle defining the middle of that half
	//                  circle.
	// Return value:    None.

	void SetEmitSpread(const float spread) { m_Spread = spread; }


	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:          SetEmitVelMin
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Sets the min end of the range the velocity of a particle being emitted
	//                  by this PEmitter can have.
	// Arguments:       A float with the min vel possible for an emitted particle.
	// Return value:    None.

	void SetEmitVelMin(const float minVel) { m_MinVelocity = minVel; }


	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:          SetEmitVelMax
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Sets the max end of the range the velocity of a particle being emitted
	//                  by this PEmitter can have.
	// Arguments:       A float with the max vel possible for an emitted particle.
	// Return value:    None.

	void SetEmitVelMax(const float maxVel) { m_MaxVelocity = maxVel; }
	*/

	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:          TriggerBurst
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Triggers a one-shot burst of emissions in the number that has
	//                  previously been set. The burst will happen during the next Update of
	//                  this PEmitter.
	// Arguments:       None.
	// Return value:    None.

	void TriggerBurst() { m_BurstTriggered = true; }


	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:          CanTriggerBurst
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Checks if it is possible to trigger a one-shot burst of emissions during
	//                  the next Update of this PEmitter.
	// Arguments:       None.
	// Return value:    If it is possible to trigger a burst.

	bool CanTriggerBurst() { if (m_BurstSpacing <= 0 || m_BurstTimer.IsPastSimMS(m_BurstSpacing)) return true; return false; }


	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:          IsSetToBurst
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Indicates whether this PEmitter is set to burst next update or not.
	// Arguments:       None.
	// Return value:    Whether a burst is gonna happen or not..

	bool IsSetToBurst() const { return m_BurstTriggered; }


	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:          AlarmOnEmit
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Registers a new AlarmEvent if this emitter has a loudness above zero.
	// Arguments:       Team that will ignore this AlarmEvent.
	// Return value:    None.

	void AlarmOnEmit(int Team) const { if (m_LoudnessOnEmit > 0) g_MovableMan.RegisterAlarmEvent(AlarmEvent(m_Pos, Team, m_LoudnessOnEmit)); }


	//////////////////////////////////////////////////////////////////////////////////////////
	// Virtual method:  ResetAllTimers
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Resest all the timers used by this. Can be emitters, etc. This is to
	//                  prevent backed up emissions to come out all at once while this has been
	//                  held dormant in an inventory.
	// Arguments:       None.
	// Return value:    None.

	virtual void ResetAllTimers() { m_BurstTimer.Reset(); m_LastEmitTmr.Reset(); }

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
	// Description:     Draws this PEmitter's current graphical representation to a
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

	// The list of MO instances that get emitted
	std::list<Emission> m_EmissionList;
	// Sounds
	Sound m_EmissionSound;
	Sound m_BurstSound;
	Sound m_EndSound;
	// Whether emitting is currently enabled or not.
	bool m_EmitEnabled;
	// Whether or not the it was emitting last frame or not.
	bool m_WasEmitting;
	// The number of emissions emitted since emission was last enabled
	long m_EmitCount;
	// The max number of emissions to emit per emit being enabled
	long m_EmitCountLimit;
	// The range negative throttle has on emission rate. 1.0 means the rate can be throttled down to 0%, 0 means negative throttle has no effect
	double m_MinThrottleRange;
	// The range positive throttle has on emission rate. 1.0 means the rate can be throttled up to 200%, 0 means negative throttle has no effect
	double m_MaxThrottleRange;
	// The normalized throttle which controls the MSPE between 1.0 * m_MSPERange and -1.0 * m_MSPERange. 0 means emit the regular m_PPM amount.
	float m_Throttle;
	// Whether or not this' emissions ignore hits with itself, even if they are set to hit other MOs.
	bool m_EmissionsIgnoreThis;
	// The scale factor that will be applied to the regular spread and emission
	// velocity to get the the burst particle parameters.
	float m_BurstScale;
	// Indicates that a burst is set to happen during the next Update.
	bool m_BurstTriggered;
	// The shortest possible time between bursts, in ms
	float m_BurstSpacing;
	// Measures the shortest possible time between bursts
	Timer m_BurstTimer;
	// The angle of the direction the emitted particles will head in.
	// The m_Roataion of this PEmitter will be added to this angle.
	Matrix m_EmitAngle;
	// Offset of the emission point from this' sprite center, which gets rotated with this
	Vector m_EmissionOffset;
	// Timer for timing how long ago the last particle was emitted. 0 means no limit.
	Timer m_LastEmitTmr;
	// Flash offset.
	//    Vector m_FlashOff; nah dont need it
	// Flash display scale
	float m_FlashScale;
	// How large impulse this emitter generates when bursting
	float m_AvgBurstImpulse;
	// How large impulse this emitter generates when firing
	float m_AvgImpulse;
	// How far this is audiable (in screens) when emitting as a jetpack or craft engine
	float m_LoudnessOnEmit;
	// Whether to only display flash on bursts, and not on any emission frame.
	bool m_FlashOnlyOnBurst;


	//////////////////////////////////////////////////////////////////////////////////////////
	// Private member variable and method declarations

private:


	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:          Clear
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Clears all the member variables of this PEmitter, effectively
	//                  resetting the members of this abstraction level only.
	// Arguments:       None.
	// Return value:    None.

	void Clear();


	// Disallow the use of some implicit methods.
	PEmitter(const PEmitter &reference);
	PEmitter & operator=(const PEmitter &rhs);

};

} // namespace RTE

#endif // File