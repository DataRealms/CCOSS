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

class Emission :
	public Entity
{
	friend class AEmitter;
	friend class PEmitter;

	//////////////////////////////////////////////////////////////////////////////////////////
	// Public member variable, method and friend function declarations

public:

	// Concrete allocation and cloning definitions
	EnitityAllocation(Emission)

	//////////////////////////////////////////////////////////////////////////////////////////
	// Constructor:     Emission
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Constructor method used to instantiate a Emission object in system
	//                  memory. Create() should be called before using the object.
	// Arguments:       None.

	Emission() { Clear(); }

	/*
	//////////////////////////////////////////////////////////////////////////////////////////
	// Virtual method:  Create
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Makes the Emission object ready for use.
	// Arguments:       None.
	// Return value:    An error return value signaling sucess or any particular failure.
	//                  Anything below 0 is an error signal.

	virtual int Create();
	*/

	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:          Create
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Creates a Emission to be identical to another, by deep copy.
	// Arguments:       A reference to the Emission to deep copy.
	// Return value:    An error return value signaling sucess or any particular failure.
	//                  Anything below 0 is an error signal.

	int Create(const Emission &reference);


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
	// Description:     Saves the complete state of this Emission to an output stream for
	//                  later recreation with Create(Reader &reader);
	// Arguments:       A Writer that the Emission will save itself with.
	// Return value:    An error return value signaling sucess or any particular failure.
	//                  Anything below 0 is an error signal.

	virtual int Save(Writer &writer) const;


	//////////////////////////////////////////////////////////////////////////////////////////
	// Virtual method:  GetClassName
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Gets the class name of this Entity.
	// Arguments:       None.
	// Return value:    A string with the friendly-formatted type name of this object.

	//virtual const std::string & GetClassName() const { return m_sClassName; }


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
	// Virtual method:  GetEmissionParticlePreset
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Gets the reference particle to be emitted. Owenership is NOT transferred!
	// Arguments:       None.
	// Return value:    A pointer to the particle to be emitted. Not transferred!

	virtual const MovableObject * GetEmissionParticlePreset() { return m_pEmission; }


	//////////////////////////////////////////////////////////////////////////////////////////
	// Virtual method:  GetRate
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Gets the rate at which these emissions are made, in particles per minute.
	// Arguments:       None.
	// Return value:    The emission rate in PPM.

	virtual float GetRate() const { return m_PPM; }

	//////////////////////////////////////////////////////////////////////////////////////////
	// Virtual method:  SetRate
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Sets the rate at which these emissions are made, in particles per minute.
	// Arguments:       The emission rate in PPM.
	// Return value:    None.

	virtual void SetRate(float newPPM) { m_PPM = newPPM; }


	//////////////////////////////////////////////////////////////////////////////////////////
	// Virtual method:  GetBurstSize
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Gets the number of extra particles that are bursted at the beginning of
	//                  emission.
	// Arguments:       None.
	// Return value:    The burst size.

	virtual int GetBurstSize() const { return m_BurstSize; }


	//////////////////////////////////////////////////////////////////////////////////////////
	// Virtual method:  SetBurstSize
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Sets the number of extra particles that are bursted at the beginning of
	//                  emission.
	// Arguments:       The burst size.
	// Return value:    None.

	virtual void SetBurstSize(int newSize) { m_BurstSize = newSize; }


	//////////////////////////////////////////////////////////////////////////////////////////
	// Virtual method:  GetSpread
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Gets the angle spread of velocity of the emitted MO's to each side of
	//                  the m_EmitAngle angle. in radians. PI/2 would mean that MO's fly out to
	//                  one side only, with the m_Rotation defining the middle of that half circle.
	// Arguments:       None.
	// Return value:    The emission spread in radians.

	virtual float GetSpread() const { return m_Spread; }


	//////////////////////////////////////////////////////////////////////////////////////////
	// Virtual method:  SetSpread
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Sets the angle spread of velocity of the emitted MO's to each side of
	//                  the m_EmitAngle angle. in radians. PI/2 would mean that MO's fly out to
	//                  one side only, with the m_Rotation defining the middle of that half circle.
	// Arguments:       The emission spread in radians.
	// Return value:    None.

	virtual void SetSpread(float newSpread) { m_Spread = newSpread; }


	//////////////////////////////////////////////////////////////////////////////////////////
	// Virtual method:  GetMinVelocity
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Gets the specified minimum velocity an emitted MO can have when emitted.
	// Arguments:       None.
	// Return value:    The min emission velocity in m/s.

	virtual float GetMinVelocity() const { return m_MinVelocity; }


	//////////////////////////////////////////////////////////////////////////////////////////
	// Virtual method:  SetMinVelocity
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Sets the specified minimum velocity an emitted MO can have when emitted.
	// Arguments:       The min emission velocity in m/s.
	// Return value:    None.

	virtual void SetMinVelocity(float newVel) { m_MinVelocity = newVel; }


	//////////////////////////////////////////////////////////////////////////////////////////
	// Virtual method:  GetMaxVelocity
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Gets the specified maximum velocity an emitted MO can have when emitted.
	// Arguments:       None.
	// Return value:    The max emission velocity in m/s.

	virtual float GetMaxVelocity() const { return m_MaxVelocity; }


	//////////////////////////////////////////////////////////////////////////////////////////
	// Virtual method:  SetMaxVelocity
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Gets the specified maximum velocity an emitted MO can have when emitted.
	// Arguments:       The max emission velocity in m/s.
	// Return value:    None.

	virtual void SetMaxVelocity(float newVel) { m_MaxVelocity = newVel; }


	//////////////////////////////////////////////////////////////////////////////////////////
	// Virtual method:  GetLifeVariation
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Gets the specified variation in lifetime of the emitted particles.
	// Arguments:       None.
	// Return value:    The life variation rationally expressed.. 0.1 = up to 10% varitaion.

	virtual float GetLifeVariation() const { return m_LifeVariation; }


	//////////////////////////////////////////////////////////////////////////////////////////
	// Virtual method:  SetLifeVariation
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Sets the specified variation in lifetime of the emitted particles.
	// Arguments:       The life variation rationally expressed.. 0.1 = up to 10% varitaion.
	// Return value:    None.

	virtual void SetLifeVariation(float newVariation)  { m_LifeVariation = newVariation; }


	//////////////////////////////////////////////////////////////////////////////////////////
	// Virtual method:  PushesEmitter
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Indicates whether this emission is supposed to push its emitter back
	//                  because of recoil.
	// Arguments:       None.
	// Return value:    Whether recoil pushing is enabled or not for this emitter.

	virtual bool PushesEmitter() const { return m_PushesEmitter; }


	//////////////////////////////////////////////////////////////////////////////////////////
	// Virtual method:  SetPushesEmitter
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Sets whether this emission is supposed to push its emitter back
	//                  because of recoil.
	// Arguments:       Whether recoil pushing is enabled or not for this emitter.
	// Return value:    None.

	virtual void SetPushesEmitter(bool newValue) { m_PushesEmitter = newValue; }


	//////////////////////////////////////////////////////////////////////////////////////////
	// Virtual method:  IsEmissionTime
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Shows whether this should be emitting now or not, based on what its
	//                  start and end timers are set to.
	// Arguments:       None.
	// Return value:    Whether this should be emitting right now.

	virtual bool IsEmissionTime() { return m_StartTimer.IsPastSimTimeLimit() && !m_StopTimer.IsPastSimTimeLimit(); }


	//////////////////////////////////////////////////////////////////////////////////////////
	// Virtual method:  ResetEmissionTimers
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Resets the emission timers so they start counting time as to wheter
	//                  emissions are clearer.
	// Arguments:       None.
	// Return value:    None.

	virtual void ResetEmissionTimers() { m_StartTimer.Reset(); m_StopTimer.Reset(); }


	//////////////////////////////////////////////////////////////////////////////////////////
	// Virtual method:  InheritsVelocity
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     How much of the root parent's velocity this emission inherit
	// Arguments:       None.
	// Return value:    The proportion of the velocity inherited. 0.1 = 10% inheritance.

	virtual float InheritsVelocity() { return m_InheritsVel; }


	//////////////////////////////////////////////////////////////////////////////////////////
	// Virtual method:  GetOffset
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Gets offset of the emission point from Emitter's sprite center, which gets rotated with owner Emitter
	// Arguments:       None.
	// Return value:    Returns emission offset.

	virtual Vector GetOffset() const { return m_Offset; }

	//////////////////////////////////////////////////////////////////////////////////////////
	// Virtual method:  SetOffset
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Sets offset of the emission point from Emitter's sprite center, which gets rotated with owner Emitter
	// Arguments:       New offset value.
	// Return value:    None.

	virtual void SetOffset(Vector offset) { m_Offset = offset; }

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