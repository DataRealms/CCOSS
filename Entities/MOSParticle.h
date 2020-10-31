#ifndef _RTEMOSPARTICLE_
#define _RTEMOSPARTICLE_

#include "MOSprite.h"

namespace RTE {

	class Atom;

	/// <summary>
	/// A small animated sprite that plays its animation and changes the animation and playback speed when it collides with other things.
	/// </summary>
	class MOSParticle : public MOSprite {

	public:

		EntityAllocation(MOSParticle)
		ClassInfoGetters
		SerializableOverrideMethods

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a MOSParticle object in system memory. Create() should be called before using the object.
		/// </summary>
		MOSParticle() { Clear(); }

		/// <summary>
		/// Makes the MOSParticle object ready for use.
		/// </summary>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create() override;

		/// <summary>
		/// Makes the MOSParticle object ready for use.
		/// </summary>
		/// <param name="spriteFile">A pointer to ContentFile that represents the bitmap file that will be used to create the Sprite.</param>
		/// <param name="frameCount">The number of frames in the Sprite's animation.</param>
		/// <param name="mass">A float specifying the object's mass in Kilograms (kg).</param>
		/// <param name="position">A Vector specifying the initial position.</param>
		/// <param name="velocity">A Vector specifying the initial velocity.</param>
		/// <param name="lifetime">The amount of time in ms this MOSParticle will exist. 0 means unlimited.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create(ContentFile spriteFile, const int frameCount = 1, const float mass = 1, const Vector &position = Vector(0, 0), const Vector &velocity = Vector(0, 0), const unsigned long lifetime = 0) {
			MOSprite::Create(spriteFile, frameCount, mass, position, velocity, lifetime);
			return 0;
		}

		/// <summary>
		/// Creates a MOSParticle to be identical to another, by deep copy.
		/// </summary>
		/// <param name="reference">A reference to the MOSParticle to deep copy.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create(const MOSParticle &reference);
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Destructor method used to clean up a MOSParticle object before deletion from system memory.
		/// </summary>
		~MOSParticle() override { Destroy(true); }

		/// <summary>
		/// Destroys and resets (through Clear()) the SceneLayer object.
		/// </summary>
		/// <param name="notInherited">Whether to only destroy the members defined in this derived class, or to destroy all inherited members also.</param>
		void Destroy(bool notInherited = false) override;

		/// <summary>
		/// Resets the entire MOSParticle, including its inherited members, to their default settings or values.
		/// </summary>
		void Reset() override { Clear(); MOSprite::Reset(); }
#pragma endregion

#pragma region Getters and Setters
		/// <summary>
		/// Gets the drawing priority of this MOSParticle. If two things are overlapping when copying to the terrain, the higher priority MO would end up getting drawn.
		/// </summary>
		/// <returns>The drawing priority of this MOSParticle.</returns>
		int GetDrawPriority() const override;

		/// <summary>
		/// Gets the main material of this MOSParticle.
		/// </summary>
		/// <returns>The material of this MOSParticle.</returns>
		const Material * GetMaterial() const override;

		/// <summary>
		/// Gets the current Atom of this MOSParticle.
		/// </summary>
		/// <returns>A const reference to the current Atom.</returns>
		const Atom * GetAtom() const { return m_Atom; }

		/// <summary>
		/// Replaces the current Atom of this MOSParticle with a new one.
		/// </summary>
		/// <param name="newAtom">A reference to the new Atom.</param>
		void SetAtom(Atom *newAtom);
#pragma endregion

#pragma region Virtual Override Methods
		/// <summary>
		/// Travels this MOSParticle, using its physical representation.
		/// </summary>
		void Travel() override;

		/// <summary>
		/// Calculates the collision response when another MO's Atom collides with this MO's physical representation. 
		/// The effects will be applied directly to this MO, and also represented in the passed in HitData.
		/// </summary>
		/// <param name="hitData">Reference to the HitData struct which describes the collision. This will be modified to represent the results of the collision.</param>
		/// <returns>Whether the collision has been deemed valid. If false, then disregard any impulses in the HitData.</returns>
		bool CollideAtPoint(HitData &hitData) override { return true; }

		/// <summary>
		/// Does the calculations necessary to detect whether this MO appears to have has settled in the world and is at rest or not. IsAtRest() retrieves the answer.
		/// </summary>
		void RestDetection() override;

		/// <summary>
		/// Defines what should happen when this MOSParticle hits and then bounces off of something. This is called by the owned Atom/AtomGroup of this MOSParticle during travel.
		/// </summary>
		/// <param name="hd">The HitData describing the collision in detail.</param>
		/// <returns>Whether the MOSParticle should immediately halt any travel going on after this bounce.</returns>
		bool OnBounce(HitData &hd) override { return false; }

		/// <summary>
		/// Defines what should happen when this MOSParticle hits and then sink into something. This is called by the owned Atom/AtomGroup of this MOSParticle during travel.
		/// </summary>
		/// <param name="hd">The HitData describing the collision in detail.</param>
		/// <returns>Whether the MOSParticle should immediately halt any travel going on after this sinkage.</returns>
		bool OnSink(HitData &hd) override { return false; }

		/// <summary>
		/// Draws this MOSParticle's current graphical representation to a BITMAP of choice.
		/// </summary>
		/// <param name="targetBitmap">A pointer to a BITMAP to draw on.</param>
		/// <param name="targetPos">The absolute position of the target bitmap's upper left corner in the Scene.</param>
		/// <param name="mode">In which mode to draw in. See the DrawMode enumeration for the modes.</param>
		/// <param name="onlyPhysical">Whether to not draw any extra 'ghost' items of this MOSParticle, indicator arrows or hovering HUD text and so on.</param>
		void Draw(BITMAP *targetBitmap, const Vector &targetPos = Vector(), DrawMode mode = g_DrawColor, bool onlyPhysical = false) const override;
#pragma endregion

	protected:

		static Entity::ClassInfo m_sClass; //!< ClassInfo for this class.

		Atom *m_Atom; //!< The Atom that will be the physical representation of this MOSParticle.
		float m_TimeRest; //!< Accumulated time in seconds that did not cause a frame change.

	private:

		/// <summary>
		/// Sets the screen effect to draw at the final post-processing stage.
		/// </summary>
		void SetPostScreenEffectToDraw() const;

		/// <summary>
		/// Clears all the member variables of this MOSParticle, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();

		// Disallow the use of some implicit methods.
		MOSParticle(const MOSParticle &reference) = delete;
		MOSParticle & operator=(const MOSParticle &rhs) = delete;
	};
}
#endif