#ifndef _RTEMOPIXEL_
#define _RTEMOPIXEL_

#include "MovableObject.h"

namespace RTE {

	class Atom;

	/// <summary>
	/// A movable object with mass that is graphically represented by a single pixel.
	/// </summary>
	class MOPixel : public MovableObject {

	public:

		EntityAllocation(MOPixel)
		ClassInfoGetters
		SerializableOverrideMethods

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a MOPixel object in system memory. Create() should be called before using the object.
		/// </summary>
		MOPixel() { Clear(); }

		/// <summary>
		/// Convenience constructor to both instantiate a MOPixel in memory and Create it at the same time.
		/// </summary>
		/// <param name="color">A Color object specifying the color of this MOPixel.</param>
		/// <param name="mass">A float specifying the object's mass in Kilograms (kg).</param>
		/// <param name="position">A Vector specifying the initial position.</param>
		/// <param name="velocity">A Vector specifying the initial velocity.</param>
		/// <param name="atom">An Atom that will collide with the terrain.</param>
		/// <param name="lifetime">The amount of time in ms this MOPixel will exist. 0 means unlimited.</param>
		MOPixel(Color color, const float mass, const Vector &position, const Vector &velocity, Atom *atom, const unsigned long lifetime = 0) { Clear(); Create(color, mass, position, velocity, atom, lifetime); }

		/// <summary>
		/// Makes the MOPixel object ready for use.
		/// </summary>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		virtual int Create();

		/// <summary>
		/// Makes the MOPixel object ready for use.
		/// </summary>
		/// <param name="color">A Color object specifying the color of this MOPixel.</param>
		/// <param name="mass">A float specifying the object's mass in Kilograms (kg).</param>
		/// <param name="position">A Vector specifying the initial position.</param>
		/// <param name="velocity">A Vector specifying the initial velocity.</param>
		/// <param name="atom">An Atom that will collide with the terrain.</param>
		/// <param name="lifetime">The amount of time in ms this MOPixel will exist. 0 means unlimited.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		virtual int Create(Color color, const float mass, const Vector &position, const Vector &velocity, Atom *atom, const unsigned long lifetime = 0);

		/// <summary>
		/// Creates a MOPixel to be identical to another, by deep copy.
		/// </summary>
		/// <param name="reference">A reference to the MOPixel to deep copy.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create(const MOPixel &reference);
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Destructor method used to clean up a MOPixel object before deletion from system memory.
		/// </summary>
		virtual ~MOPixel() { Destroy(true); }

		/// <summary>
		/// Destroys and resets (through Clear()) the MOPixel object.
		/// </summary>
		/// <param name="notInherited">Whether to only destroy the members defined in this derived class, or to destroy all inherited members also.</param>
		virtual void Destroy(bool notInherited = false);

		/// <summary>
		/// Resets the entire MOPixel, including its inherited members, to their default settings or values.
		/// </summary>
		virtual void Reset() { Clear(); MovableObject::Reset(); }
#pragma endregion

#pragma region Getters and Setters
		/// <summary>
		/// Gets the drawing priority of this MOPixel, if two things were overlap when copying to the terrain, the higher priority MO would end up getting drawn.
		/// </summary>
		/// <returns>The drawing priority of this MOPixel.</returns>
		virtual int GetDrawPriority() const;

		/// <summary>
		/// Gets the main Material of this MOPixel.
		/// </summary>
		/// <returns>The Material of this MOPixel.</returns>
		virtual const Material * GetMaterial() const;

		/// <summary>
		/// Indicates whether this MOPixel is made of Gold or not.
		/// </summary>
		/// <returns>Whether this MOPixel is of Gold or not.</returns>
		virtual bool IsGold() const { return m_MOType == TypeGeneric && GetMaterial()->GetIndex() == c_GoldMaterialID; }

		/// <summary>
		/// Gets the current Atom of this MOPixel.
		/// </summary>
		/// <returns>A const reference to the current Atom.</returns>
		const Atom * GetAtom() const { return m_Atom; }

		/// <summary>
		/// Replaces the current Atom of this MOPixel with a new one.
		/// </summary>
		/// <param name="newAtom">A reference to the new Atom. Ownership IS transferred!</param>
		void SetAtom(Atom *newAtom);

		/// <summary>
		/// Gets the color of this MOPixel.
		/// </summary>
		/// <returns>A Color object describing the color.</returns>
		Color GetColor() const { return m_Color; }

		/// <summary>
		/// Sets the color value of this MOPixel.
		/// </summary>
		/// <param name="newColor">A Color object specifying the new color index value.</param>
		void SetColor(Color newColor) { m_Color = newColor; }

		/// <summary>
		/// Travel distance until the bullet start to lose lethality.
		/// </summary>
		/// <returns>The factor that modifies the base value.</returns>
		float GetMaxLethalRangeFactor() const { return m_MaxLethalRange; }

		/// <summary>
		/// Travel distance until the bullet start to lose lethality.
		/// </summary>
		/// <param name="range">The distance in pixels.</param>
		void SetLethalRange(float range);
#pragma endregion

#pragma region Virtual Override Methods
		/// <summary>
		/// Updates this MOPixel. Supposed to be done every frame.
		/// </summary>
		virtual void Update();

		/// <summary>
		/// Travels this MOPixel, using its physical representation.
		/// </summary>
		virtual void Travel();

		/// <summary>
		/// Calculates the collision response when another MO's Atom collides with this MO's physical representation.
		/// The effects will be applied directly to this MO, and also represented in the passed in HitData.
		/// </summary>
		/// <param name="hitData">Reference to the HitData struct which describes the collision. This will be modified to represent the results of the collision.</param>
		/// <returns>Whether the collision has been deemed valid. If false, then disregard any impulses in the HitData.</returns>
		virtual bool CollideAtPoint(HitData &hitData);

		/// <summary>
		/// Does the calculations necessary to detect whether this MO appears to have has settled in the world and is at rest or not. IsAtRest() retrieves the answer.
		/// </summary>
		virtual void RestDetection();

		/// <summary>
		/// Defines what should happen when this MOPixel hits and then bounces off of something. This is called by the owned Atom/AtomGroup of this MOPixel during travel.
		/// </summary>
		/// <param name="hd">The HitData describing the collision in detail.</param>
		/// <returns>Whether the MOPixel should immediately halt any travel going on after this bounce.</returns>
		virtual bool OnBounce(HitData &hd) { return false; }

		/// <summary>
		/// Defines what should happen when this MOPixel hits and then sink into something. This is called by the owned Atom/AtomGroup of this MOPixel during travel.
		/// </summary>
		/// <param name="hd">The HitData describing the collision in detail.</param>
		/// <returns>Whether the MOPixel should immediately halt any travel going on after this sinkage.</returns>
		virtual bool OnSink(HitData &hd) { return false; }

		/// <summary>
		/// Draws this MOPixel's current graphical representation to a BITMAP of choice.
		/// </summary>
		/// <param name="targetBitmap">A pointer to a BITMAP to draw on.</param>
		/// <param name="targetPos">The absolute position of the target bitmap's upper left corner in the Scene.</param>
		/// <param name="mode">In which mode to draw in. See the DrawMode enumeration for the modes.</param>
		/// <param name="onlyPhysical">Whether to not draw any extra 'ghost' items of this MOPixel, indicator arrows or hovering HUD text and so on.</param>
		virtual void Draw(BITMAP *targetBitmap, const Vector &targetPos = Vector(), DrawMode mode = g_DrawColor, bool onlyPhysical = false) const;
#pragma endregion

	protected:

		static Entity::ClassInfo m_sClass; //!< ClassInfo for this class.

		Atom *m_Atom; //!< The single Atom that is responsible for collisions of this MOPixel.
		Color m_Color; //!< Color representation of this MOPixel. 

		float m_DistanceTraveled; //!< An estimate of how far this MO has traveled since its creation.	

		float m_LethalRange; //!< After this distance in meters, the damage output of this MO decrease. Default is half a screen plus sharp length for pixels spawned from a gun and one screen otherwise.
		float m_MinLethalRange; //!< Lower bound multiplier for setting LethalRange at random. 1.0 equals one screen.
		float m_MaxLethalRange; //!< Upper bound multiplier for setting LethalRange at random. 1.0 equals one screen.
		float m_LethalSharpness; //!< When Sharpness has decreased below this threshold the MO becomes m_HitsMOs = false. Default is Sharpness * 0.5.

	private:

		/// <summary>
		/// Clears all the member variables of this MOPixel, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();
	};
}
#endif