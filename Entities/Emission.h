#ifndef _RTEEMISSION_
#define _RTEEMISSION_

//////////////////////////////////////////////////////////////////////////////////////////
// File:			Emission.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Something to bundle the properties of an emission together.
// Parent(s):       Entity.
// Class history:   09/07/2004 Emission created as struct.
//                  07/21/2006 Emission turned into 'Serializable' class.

#include "Serializable.h"
#include "MovableObject.h"

namespace RTE
{


//////////////////////////////////////////////////////////////////////////////////////////
// Class:           Emission
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Something to bundle the properties of an emission together.
// Parent(s):       Entity.

class Emission : public Entity {
	friend class AEmitter;
	friend class PEmitter;

	//////////////////////////////////////////////////////////////////////////////////////////
	// Public member variable, method and friend function declarations

public:

	// Concrete allocation and cloning definitions
	EntityAllocation(Emission);
	SerializableOverrideMethods;
	ClassInfoGetters;

	//////////////////////////////////////////////////////////////////////////////////////////
	// Constructor:     Emission
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Constructor method used to instantiate a Emission object in system
	//                  memory. Create() should be called before using the object.
	// Arguments:       None.

	Emission() { Clear(); }


	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:          Create
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Creates a Emission to be identical to another, by deep copy.
	// Arguments:       A reference to the Emission to deep copy.
	// Return value:    An error return value signaling sucess or any particular failure.
	//                  Anything below 0 is an error signal.

	int Create(const Emission &reference);


	//////////////////////////////////////////////////////////////////////////////////////////
	// Virtual method:  Reset
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Resets the entire Serializable, including its inherited members, to their
	//                  default settings or values.
	// Arguments:       None.
	// Return value:    None.

	void Reset() override { Clear(); }


	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:  GetEmissionParticlePreset
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Gets the reference particle to be emitted. Owenership is NOT transferred!
	// Arguments:       None.
	// Return value:    A pointer to the particle to be emitted. Not transferred!

	const MovableObject * GetEmissionParticlePreset() { return m_pEmission; }


	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:  GetRate
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Gets the rate at which these emissions are made, in particles per minute.
	// Arguments:       None.
	// Return value:    The emission rate in PPM.

	float GetRate() const { return m_PPM; }

	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:  SetRate
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Sets the rate at which these emissions are made, in particles per minute.
	// Arguments:       The emission rate in PPM.
	// Return value:    None.

	void SetRate(float newPPM) { m_PPM = newPPM; }


	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:  GetBurstSize
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Gets the number of extra particles that are bursted at the beginning of
	//                  emission.
	// Arguments:       None.
	// Return value:    The burst size.

	int GetBurstSize() const { return m_BurstSize; }


	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:  SetBurstSize
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Sets the number of extra particles that are bursted at the beginning of
	//                  emission.
	// Arguments:       The burst size.
	// Return value:    None.

	void SetBurstSize(int newSize) { m_BurstSize = newSize; }


	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:  GetSpread
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Gets the angle spread of velocity of the emitted MO's to each side of
	//                  the m_EmitAngle angle. in radians. PI/2 would mean that MO's fly out to
	//                  one side only, with the m_Rotation defining the middle of that half circle.
	// Arguments:       None.
	// Return value:    The emission spread in radians.

	float GetSpread() const { return m_Spread; }


	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:  SetSpread
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Sets the angle spread of velocity of the emitted MO's to each side of
	//                  the m_EmitAngle angle. in radians. PI/2 would mean that MO's fly out to
	//                  one side only, with the m_Rotation defining the middle of that half circle.
	// Arguments:       The emission spread in radians.
	// Return value:    None.

	void SetSpread(float newSpread) { m_Spread = newSpread; }


	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:  GetMinVelocity
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Gets the specified minimum velocity an emitted MO can have when emitted.
	// Arguments:       None.
	// Return value:    The min emission velocity in m/s.

	float GetMinVelocity() const { return m_MinVelocity; }


	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:  SetMinVelocity
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Sets the specified minimum velocity an emitted MO can have when emitted.
	// Arguments:       The min emission velocity in m/s.
	// Return value:    None.

	void SetMinVelocity(float newVel) { m_MinVelocity = newVel; }


	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:  GetMaxVelocity
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Gets the specified maximum velocity an emitted MO can have when emitted.
	// Arguments:       None.
	// Return value:    The max emission velocity in m/s.

	float GetMaxVelocity() const { return m_MaxVelocity; }


	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:  SetMaxVelocity
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Gets the specified maximum velocity an emitted MO can have when emitted.
	// Arguments:       The max emission velocity in m/s.
	// Return value:    None.

	void SetMaxVelocity(float newVel) { m_MaxVelocity = newVel; }


	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:  GetLifeVariation
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Gets the specified variation in lifetime of the emitted particles.
	// Arguments:       None.
	// Return value:    The life variation rationally expressed.. 0.1 = up to 10% varitaion.

	float GetLifeVariation() const { return m_LifeVariation; }


	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:  SetLifeVariation
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Sets the specified variation in lifetime of the emitted particles.
	// Arguments:       The life variation rationally expressed.. 0.1 = up to 10% varitaion.
	// Return value:    None.

	void SetLifeVariation(float newVariation)  { m_LifeVariation = newVariation; }


	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:  PushesEmitter
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Indicates whether this emission is supposed to push its emitter back
	//                  because of recoil.
	// Arguments:       None.
	// Return value:    Whether recoil pushing is enabled or not for this emitter.

	bool PushesEmitter() const { return m_PushesEmitter; }


	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:  SetPushesEmitter
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Sets whether this emission is supposed to push its emitter back
	//                  because of recoil.
	// Arguments:       Whether recoil pushing is enabled or not for this emitter.
	// Return value:    None.

	void SetPushesEmitter(bool newValue) { m_PushesEmitter = newValue; }


	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:  IsEmissionTime
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Shows whether this should be emitting now or not, based on what its
	//                  start and end timers are set to.
	// Arguments:       None.
	// Return value:    Whether this should be emitting right now.

	bool IsEmissionTime() { return m_StartTimer.IsPastSimTimeLimit() && !m_StopTimer.IsPastSimTimeLimit(); }


	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:  ResetEmissionTimers
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Resets the emission timers so they start counting time as to wheter
	//                  emissions are clearer.
	// Arguments:       None.
	// Return value:    None.

	void ResetEmissionTimers() { m_StartTimer.Reset(); m_StopTimer.Reset(); }


	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:  InheritsVelocity
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     How much of the root parent's velocity this emission inherit
	// Arguments:       None.
	// Return value:    The proportion of the velocity inherited. 0.1 = 10% inheritance.

	float InheritsVelocity() { return m_InheritsVel; }


	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:  GetOffset
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Gets offset of the emission point from Emitter's sprite center, which gets rotated with owner Emitter
	// Arguments:       None.
	// Return value:    Returns emission offset.

	Vector GetOffset() const { return m_Offset; }

	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:  SetOffset
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Sets offset of the emission point from Emitter's sprite center, which gets rotated with owner Emitter
	// Arguments:       New offset value.
	// Return value:    None.

	void SetOffset(Vector offset) { m_Offset = offset; }

	//////////////////////////////////////////////////////////////////////////////////////////
	// Protected member variable and method declarations

protected:

	// Member variables
	//static const std::string m_sClassName;
	// Member variables
	static Entity::ClassInfo m_sClass;

	// The pointer to the preset instance, that copies of which will be emitted
	const MovableObject *m_pEmission;
	// Emission rate in Particles Per Minute 
	float m_PPM;
	// The number of particles in the first initial burst of emissions
	// that this AEmitter will generate upon emitting. 0 means none (duh).
	int m_BurstSize;
	// The accumulator for decoupling emission rate from the physics update rate.
	double m_Accumulator;
	// The angle spread of velocity of the emitted MO's to each
	// side of the m_EmitAngle angle. in radians.
	// PI/2 would mean that MO's fly out to one side only, with the
	// m_Rotation defining the middle of that half circle.
	float m_Spread;
	// The minimum velocity an emitted MO can have when emitted
	float m_MinVelocity;
	// The maximum velocity an emitted MO can have when emitted
	float m_MaxVelocity;
	// The variation in life time of each emitted aprticle, in percentage of the existing life time of the partilcle
	float m_LifeVariation;
	// Whether these emissions push the emitter around with recoil or not.
	bool m_PushesEmitter;
	// How much of the parents velocity this emission inherits
	float m_InheritsVel;
	// Timers for measuring when to start and stop this emission the actual times are the set time limits of these
	Timer m_StartTimer;
	Timer m_StopTimer;
	// Offset of the emission point from Emitter's sprite center, which gets rotated with owner Emitter
	Vector m_Offset;


	//////////////////////////////////////////////////////////////////////////////////////////
	// Private member variable and method declarations

private:

	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:          Clear
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Clears all the member variables of this Emission, effectively
	//                  resetting the members of this abstraction level only.
	// Arguments:       None.
	// Return value:    None.

	void Clear();

};

} // namespace RTE
#endif