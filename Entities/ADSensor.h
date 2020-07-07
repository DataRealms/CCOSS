#ifndef _RTEADSENSOR_
#define _RTEADSENSOR_

#include "Serializable.h"
#include "Matrix.h"

namespace RTE {

	class Actor;

	/// <summary>
	/// The ray-casting sensor which triggers the door opening or closing, depending on the team of the Actor that broke the ray.
	/// </summary>
	class ADSensor : public Serializable {

	public:

		SerializableOverrideMethods

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate an ADSensor object in system memory. Create() should be called before using the object.
		/// </summary>
		ADSensor() { Clear(); }

		/// <summary>
		/// Makes the ADSensor object ready for use.
		/// </summary>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		virtual int Create() { return Serializable::Create(); }

		/// <summary>
		/// Creates an ADSensor to be identical to another, by deep copy.
		/// </summary>
		/// <param name="reference">A reference to the ADSensor to deep copy.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		virtual int Create(const ADSensor &reference);
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Destructor method used to clean up an ADSensor object before deletion from system memory.
		/// </summary>
		virtual ~ADSensor() { Destroy(); }

		/// <summary>
		/// Destroys and resets (through Clear()) the ADSensor object.
		/// </summary>
		/// <param name="notInherited">Whether to only destroy the members defined in this derived class, or to destroy all inherited members also.</param>
		virtual void Destroy(bool notInherited = false) { Clear(); }
#pragma endregion

#pragma region Getters and Setters
		/// <summary>
		/// Gets the starting position offset of this ADSensor from the owning ADoor position.
		/// </summary>
		/// <returns>The starting coordinates relative to the m_Pos of this' ADoor.</returns>
		Vector GetStartOffset() const { return m_StartOffset; }

		/// <summary>
		/// Sets the starting position offset of this ADSensor from the owning ADoor position.
		/// </summary>
		/// <param name="startOffsetValue">The new starting coordinates relative to the m_Pos of this' ADoor.</param>
		void SetStartOffset(const Vector &startOffsetValue) { m_StartOffset = startOffsetValue; }

		/// <summary>
		/// Gets the sensor ray vector out from the start offset's position.
		/// </summary>
		/// <returns>The sensor ray vector.</returns>
		Vector GetSensorRay() const { return m_SensorRay; }

		/// <summary>
		/// Sets the sensor ray vector out from the start offset's position.
		/// </summary>
		/// <param name="sensorRayValue">The new sensor ray vector.</param>
		void SetSensorRay(const Vector &sensorRayValue) { m_SensorRay = sensorRayValue; }
#pragma endregion

#pragma region Concrete Methods
		/// <summary>
		/// Casts the ray along the sensor vector and returns any Actor that was found along it.
		/// </summary>
		/// <param name="doorPos">Position of this ADSensor's ADoor.</param>
		/// <param name="doorRot">Rotation of this ADSensor's ADoor.</param>
		/// <param name="doorHFlipped">Flipping of this ADSensor's ADoor.</param>
		/// <param name="ignoreMOID">Which MOID to ignore, if any.</param>
		/// <returns>The root Actor of the first MOID hit by the sensor ray. 0 if none.</returns>
		Actor * SenseActor(const Vector &doorPos, const Matrix &doorRot, bool doorHFlipped = false, MOID ignoreMOID = g_NoMOID);
#pragma endregion

#pragma region Class Info
		/// <summary>
		/// Gets the class name of this Entity.
		/// </summary>
		/// <returns>A string with the friendly-formatted type name of this object.</returns>
		virtual const std::string & GetClassName() const { return m_sClassName; }
#pragma endregion

	protected:

		static const std::string m_sClassName; //!< A string with the friendly formatted type name of this object.
		
		Vector m_StartOffset; //!< The offset of the sensor ray start relative to the position of its ADoor.
		Vector m_SensorRay; //!< The ray out from the offset.
		
		short m_Skip; //!< How many pixels to skip between sensing pixels.

	private:

		/// <summary>
		/// Clears all the member variables of this ADSensor, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();
	};
}
#endif