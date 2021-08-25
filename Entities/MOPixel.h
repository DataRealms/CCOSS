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

		EntityAllocation(MOPixel);
		ClassInfoGetters;
		SerializableOverrideMethods;

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
		int Create() override;

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
		int Create(Color color, const float mass, const Vector &position, const Vector &velocity, Atom *atom, const unsigned long lifetime = 0);

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
		 ~MOPixel() override { Destroy(true); }

		/// <summary>
		/// Destroys and resets (through Clear()) the MOPixel object.
		/// </summary>
		/// <param name="notInherited">Whether to only destroy the members defined in this derived class, or to destroy all inherited members also.</param>
		void Destroy(bool notInherited = false) override;

		/// <summary>
		/// Resets the entire MOPixel, including its inherited members, to their default settings or values.
		/// </summary>
		void Reset() override { Clear(); MovableObject::Reset(); }
#pragma endregion

#pragma region Getters and Setters
		/// <summary>
		/// Gets the drawing priority of this MOPixel, if two things were overlap when copying to the terrain, the higher priority MO would end up getting drawn.
		/// </summary>
		/// <returns>The drawing priority of this MOPixel.</returns>
		int GetDrawPriority() const override;

		/// <summary>
		/// Gets the main Material of this MOPixel.
		/// </summary>
		/// <returns>The Material of this MOPixel.</returns>
		const Material * GetMaterial() const override;

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

		/// <summary>
		/// Gets the longest a trail can be drawn, in pixels.
		/// </summary>
		/// <returns>The new max length, in pixels. If 0, no trail is drawn.</returns>
		int GetTrailLength() const;

		/// <summary>
		/// Sets the longest a trail can be drawn, in pixels.
		/// </summary>
		/// <param name="trailLength">The new max length, in pixels. If 0, no trail is drawn.</param>
		void SetTrailLength(int trailLength);
#pragma endregion

#pragma region Virtual Override Methods
		/// <summary>
		/// Travels this MOPixel, using its physical representation.
		/// </summary>
		void Travel() override;

		/// <summary>
		/// Calculates the collision response when another MO's Atom collides with this MO's physical representation.
		/// The effects will be applied directly to this MO, and also represented in the passed in HitData.
		/// </summary>
		/// <param name="hitData">Reference to the HitData struct which describes the collision. This will be modified to represent the results of the collision.</param>
		/// <returns>Whether the collision has been deemed valid. If false, then disregard any impulses in the HitData.</returns>
		bool CollideAtPoint(HitData &hitData) override;

		/// <summary>
		/// Does the calculations necessary to detect whether this MO appears to have has settled in the world and is at rest or not. IsAtRest() retrieves the answer.
		/// </summary>
		void RestDetection() override;

		/// <summary>
		/// Defines what should happen when this MOPixel hits and then bounces off of something. This is called by the owned Atom/AtomGroup of this MOPixel during travel.
		/// </summary>
		/// <param name="hd">The HitData describing the collision in detail.</param>
		/// <returns>Whether the MOPixel should immediately halt any travel going on after this bounce.</returns>
		bool OnBounce(HitData &hd) override { return false; }

		/// <summary>
		/// Defines what should happen when this MOPixel hits and then sink into something. This is called by the owned Atom/AtomGroup of this MOPixel during travel.
		/// </summary>
		/// <param name="hd">The HitData describing the collision in detail.</param>
		/// <returns>Whether the MOPixel should immediately halt any travel going on after this sinkage.</returns>
		bool OnSink(HitData &hd) override { return false; }

		/// <summary>
		/// Updates this MOPixel. Supposed to be done every frame.
		/// </summary>
		void Update() override;

		/// <summary>
		/// Draws this MOPixel's current graphical representation to a BITMAP of choice.
		/// </summary>
		/// <param name="targetBitmap">A pointer to a BITMAP to draw on.</param>
		/// <param name="targetPos">The absolute position of the target bitmap's upper left corner in the Scene.</param>
		/// <param name="mode">In which mode to draw in. See the DrawMode enumeration for the modes.</param>
		/// <param name="onlyPhysical">Whether to not draw any extra 'ghost' items of this MOPixel, indicator arrows or hovering HUD text and so on.</param>
		void Draw(BITMAP *targetBitmap, const Vector &targetPos = Vector(), DrawMode mode = g_DrawColor, bool onlyPhysical = false) const override;
#pragma endregion

	protected:

		static Entity::ClassInfo m_sClass; //!< ClassInfo for this class.

		Atom *m_Atom; //!< The single Atom that is responsible for collisions of this MOPixel.
		Color m_Color; //!< Color representation of this MOPixel. 

		float m_DistanceTraveled; //!< An estimate of how far this MO has traveled since its creation.	

		float m_LethalRange; //!< After this distance in meters, the MO has a chance to no longer hit MOs, and its Lifetime decreases. Defaults to the length of a player's screen.
		float m_MinLethalRange; //!< Lower bound multiplier for setting LethalRange at random. By default, 1.0 equals one screen.
		float m_MaxLethalRange; //!< Upper bound multiplier for setting LethalRange at random. By default, 1.0 equals one screen.
		float m_LethalSharpness; //!< When Sharpness has decreased below this threshold the MO becomes m_HitsMOs = false. Default is Sharpness * 0.5.

	private:

		/// <summary>
		/// Sets the screen effect to draw at the final post-processing stage.
		/// </summary>
		void SetPostScreenEffectToDraw() const;

		/// <summary>
		/// Clears all the member variables of this MOPixel, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();
	};
}
#endif