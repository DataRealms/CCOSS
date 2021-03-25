#ifndef _RTEADOOR_
#define _RTEADOOR_

#include "Actor.h"
#include "ADSensor.h"

namespace RTE {

	class Attachable;

	/// <summary>
	/// A sliding or swinging door.
	/// </summary>
	class ADoor : public Actor {

	public:

		EntityAllocation(ADoor)
		SerializableOverrideMethods
		ClassInfoGetters

		enum DoorState {
			CLOSED = 0,
			OPENING,
			OPEN,
			CLOSING,
			STOPPED,
			DOORSTATECOUNT
		};

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a ADoor object in system memory. Create() should be called before using the object.
		/// </summary>
		ADoor() { Clear(); }

		/// <summary>
		/// Makes the ADoor object ready for use.
		/// </summary>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create() override { return Actor::Create(); }

		/// <summary>
		/// Creates a ADoor to be identical to another, by deep copy.
		/// </summary>
		/// <param name="reference">A reference to the ADoor to deep copy.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create(const ADoor &reference);
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Destructor method used to clean up a ADoor object before deletion from system memory.
		/// </summary>
		~ADoor() override { Destroy(true); }

		/// <summary>
		/// Destroys and resets (through Clear()) the ADoor object.
		/// </summary>
		/// <param name="notInherited">Whether to only destroy the members defined in this derived class, or to destroy all inherited members also.</param>
		void Destroy(bool notInherited = false) override;

		/// <summary>
		/// Resets the entire ADoor, including its inherited members, to their default settings or values.
		/// </summary>
		void Reset() override { Clear(); Actor::Reset(); }
#pragma endregion

#pragma region Getters and Setters
		/// <summary>
		/// Gets the moving door Attachable of this ADoor
		/// </summary>
		/// <returns>A pointer to the door Attachable of this. Ownership is NOT transferred!</returns>
		Attachable * GetDoor() const { return m_Door; }

		/// <summary>
		/// Sets the moving door Attachable for this ADoor.
		/// </summary>
		/// <param name="newDoor">The new moving door attachable to use.</param>
		void SetDoor(Attachable *newDoor);

		/// <summary>
		/// Gets the current state of the door.
		/// </summary>
		/// <returns>The current state of this ADoor. See the DoorState enum.</returns>
		DoorState GetDoorState() const { return m_DoorState; }

		/// <summary>
		/// Sets whether this ADoor closes (or opens) after a while by default.
		/// </summary>
		/// <param name="closedByDefault">Whether the door by default goes to a closed position. If not, then it will open after a while.</param>
		void SetClosedByDefault(bool closedByDefault) { m_ClosedByDefault = closedByDefault; }

		/// <summary>
		/// Tells whether the player can switch control to this at all.
		/// </summary>
		/// <returns>Whether a player can control this at all.</returns>
		bool IsControllable() const override { return false; }

		/// <summary>
		/// Gets this ADoor's door move start sound. Ownership is NOT transferred!
		/// </summary>
		/// <returns>The SoundContainer for this ADoor's door move start sound.</returns>
		SoundContainer * GetDoorMoveStartSound() const { return m_DoorMoveStartSound; }

		/// <summary>
		/// Sets this ADoor's door move start sound. Ownership IS transferred!
		/// </summary>
		/// <param name="newSound">The new SoundContainer for this ADoor's door move start sound.</param>
		void SetDoorMoveStartSound(SoundContainer *newSound) { m_DoorMoveStartSound = newSound; }

		/// <summary>
		/// Gets this ADoor's door move sound. Ownership is NOT transferred!
		/// </summary>
		/// <returns>The SoundContainer for this ADoor's door move sound.</returns>
		SoundContainer * GetDoorMoveSound() const { return m_DoorMoveSound; }

		/// <summary>
		/// Sets this ADoor's door move sound. Ownership IS transferred!
		/// </summary>
		/// <param name="newSound">The new SoundContainer for this ADoor's door move sound.</param>
		void SetDoorMoveSound(SoundContainer *newSound) { m_DoorMoveSound = newSound; }

		/// <summary>
		/// Gets this ADoor's door direction change sound. Ownership is NOT transferred!
		/// </summary>
		/// <returns>The SoundContainer for this ADoor's door direction change sound.</returns>
		SoundContainer * GetDoorDirectionChangeSound() const { return m_DoorDirectionChangeSound; }

		/// <summary>
		/// Sets this ADoor's door direction change sound. Ownership IS transferred!
		/// </summary>
		/// <param name="newSound">The new SoundContainer for this ADoor's door direction change sound.</param>
		void SetDoorDirectionChangeSound(SoundContainer *newSound) { m_DoorDirectionChangeSound = newSound; }

		/// <summary>
		/// Gets this ADoor's door move end sound. Ownership is NOT transferred!
		/// </summary>
		/// <returns>The SoundContainer for this ADoor's door move end sound.</returns>
		SoundContainer * GetDoorMoveEndSound() const { return m_DoorMoveEndSound; }

		/// <summary>
		/// Sets this ADoor's door move end sound. Ownership IS transferred!
		/// </summary>
		/// <param name="newSound">The new SoundContainer for this ADoor's door move end sound.</param>
		void SetDoorMoveEndSound(SoundContainer *newSound) { m_DoorMoveEndSound = newSound; }
#pragma endregion

#pragma region Concrete Methods
		/// <summary>
		/// Opens the door if it's closed.
		/// </summary>
		void OpenDoor();

		/// <summary>
		/// Closes the door if it's open.
		/// </summary>
		void CloseDoor();

		/// <summary>
		/// Force the door to stop at the exact position it is.
		/// </summary>
		void StopDoor();

		/// <summary>
		/// Used to temporarily remove or add back the material drawing of this in the scene. Used for making pathfinding work through doors.
		/// </summary>
		/// <param name="enable">Whether to enable the override or not.</param>
		void MaterialDrawOverride(bool enable);
#pragma endregion

#pragma region Virtual Override Methods
		/// <summary>
		/// Destroys this ADoor and creates its specified Gibs in its place with appropriate velocities.
		/// Any Attachables are removed and also given appropriate velocities.
		/// </summary>
		/// <param name="impactImpulse">The impulse (kg * m/s) of the impact causing the gibbing to happen.</param>
		/// <param name="movableObjectToIgnore">A pointer to an MO which the Gibs and Attachables should not be colliding with.</param>
		void GibThis(const Vector &impactImpulse = Vector(), MovableObject *movableObjectToIgnore = nullptr) override;

		/// <summary>
		/// Updates this ADoor. Supposed to be done every frame.
		/// </summary>
		void Update() override;

		/// <summary>
		/// Draws this ADoor's current graphical HUD overlay representation to a BITMAP of choice.
		/// </summary>
		/// <param name="targetBitmap">A pointer to a BITMAP to draw on.</param>
		/// <param name="targetPos">The absolute position of the target bitmap's upper left corner in the Scene.</param>
		/// <param name="whichScreen">Which player's screen this is being drawn to. May affect what HUD elements get drawn etc.</param>
		/// <param name="playerControlled">Whether or not this MovableObject is currently player controlled (not applicable for ADoor)</param>
		void DrawHUD(BITMAP *targetBitmap, const Vector &targetPos = Vector(), int whichScreen = 0, bool playerControlled = false) override;
#pragma endregion

	protected:

		static Entity::ClassInfo m_sClass; //!< ClassInfo for this class.

		int m_InitialSpriteAnimDuration; //!< This stores the original SpriteAnimDuration value so we can drive the death spin-up animation using LERP. For internal use only.

		std::list<ADSensor> m_Sensors; //!< All the sensors for detecting Actors approaching the door.
		Timer m_SensorTimer; //!< Times the exit interval.
		long m_SensorInterval; //!< The delay between each sensing pass in ms.

		Attachable *m_Door; //!< Actual door module that moves. Owned by this.

		DoorState m_DoorState; //!< Current door action state.
		DoorState m_DoorStateOnStop; //!< The state this door was in when it was stopped. For internal use only.

		bool m_ClosedByDefault; //!< Whether the closed position is the default.

		Vector m_OpenOffset; //!< Open offset from this' position - these effectively replace the door's parent offset.
		Vector m_ClosedOffset; //!< Closed offset from this' position - these effectively replace the door's parent offset.

		float m_OpenAngle; //!< Open absolute angle of the door attachable, in radians.
		float m_ClosedAngle; //!< Closed absolute angle of the door attachable, in radians.

		Timer m_DoorMoveTimer; //!< Timer for opening and closing the door.
		int m_DoorMoveTime; //!< The time it takes to open or close the door in ms.

		bool m_ResumeAfterStop; //!< Whether the door is starting movement after being forced stopped. For internal use only.
		bool m_ChangedDirectionAfterStop; //!< Whether the door changed directions while moving between states. For internal use only.
		double m_DoorMoveStopTime; //!< The elapsed time of m_DoorMoveTimer when the door was forced stopped. For internal use only.
		
		Timer m_ResetToDefaultStateTimer; //!< Timer for the resetting to the default state.
		int m_ResetToDefaultStateDelay; //!< How long the door stays in the non-default state before returning to the default state.

		bool m_DrawMaterialLayerWhenOpen; //!< Whether to draw the door's silhouette to the terrain material layer when fully open.
		bool m_DrawMaterialLayerWhenClosed; //!< Whether to draw the door's silhouette to the terrain material layer when fully closed.

		unsigned char m_DoorMaterialID; //!< The ID of the door material drawn to the terrain.
		bool m_DoorMaterialDrawn; //!< Whether the door material is currently drawn onto the material layer.
		bool m_DoorMaterialTempErased; //!< Whether the drawing override is enabled and the door material is erased to allow better pathfinding.
		Vector m_LastDoorMaterialPos; //!< The position the door attachable had when its material was drawn to the material bitmap. This is used to erase the previous material representation.

		SoundContainer *m_DoorMoveStartSound; //!< Sound played when the door starts moving from fully open/closed position towards the opposite end.
		SoundContainer *m_DoorMoveSound; //!< Sound played while the door is moving between open/closed position.
		SoundContainer *m_DoorDirectionChangeSound; //!< Sound played when the door is interrupted while moving and changes directions. 
		SoundContainer *m_DoorMoveEndSound; //!< Sound played when the door stops moving and is at fully open/closed position.

	private:

#pragma region Update Breakdown
		/// <summary>
		/// Iterates through the sensor list looking for actors and acts accordingly. Resets to the default state if none are found and past the delay timer. This is called from Update().
		/// </summary>
		void UpdateSensors();

		/// <summary>
		/// Updates the door attachable position and movement based on the current state of this ADoor. This is called from Update().
		/// </summary>
		void UpdateDoorAttachableActions();
#pragma endregion

		/// <summary>
		/// Shared method for the door opening/closing sequence. This is called from OpenDoor() and CloseDoor().
		/// </summary>
		void SharedDoorControls();

		/// <summary>
		/// Draws the material under the position of the door attachable, to create terrain collision detection for the doors.
		/// </summary>
		void DrawDoorMaterial();

		/// <summary>
		/// Flood-fills the material area under the last position of the door attachable that matches the material index of it.
		/// This is to get rid of the material footprint made with DrawDoorMaterial when the door part starts to move.
		/// </summary>
		/// <param name="updateMaterialArea">Whether to update the MaterialArea after erasing or not. Used for DrawDoorMaterial().</param>
		/// <param name="keepMaterialDrawnFlag">Whether to keep the DoorMaterialDrawn flag or not. Used for MaterialDrawOverride().</param>
		/// <returns>Whether the fill erasure was successful (if the same material as the door was found and erased).</returns>
		bool EraseDoorMaterial(bool updateMaterialArea = true, bool keepMaterialDrawnFlag = false);

		/// <summary>
		/// Clears all the member variables of this ADoor, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();

		// Disallow the use of some implicit methods.
		ADoor(const ADoor &reference) = delete;
		ADoor & operator=(const ADoor &rhs) = delete;
	};
}
#endif