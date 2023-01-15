#ifndef _RTECAMERAMAN_
#define _RTECAMERAMAN_

#include "Singleton.h"
#include "Timer.h"
#include "Vector.h"

#define g_CameraMan CameraMan::Instance()

namespace RTE {

	/// <summary>
	/// The singleton manager of the camera for each player.
	/// </summary>
	class CameraMan : public Singleton<CameraMan> {
		friend class SettingsMan;

	public:

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a CameraMan object in system memory. Create() should be called before using the object.
		/// </summary>
		CameraMan() { Clear(); }

		/// <summary>
		/// Makes the CameraMan object ready for use.
		/// </summary>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Initialize() const { return 0; }
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Destructor method used to clean up a CameraMan object before deletion from system memory.
		/// </summary>
		~CameraMan() { Destroy(); }

		/// <summary>
		/// Destroys and resets (through Clear()) the CameraMan object.
		/// </summary>
		void Destroy() { Clear(); }
#pragma endregion

#pragma region Screen Handling
		/// <summary>
		/// Gets the offset (scroll position) of the terrain.
		/// </summary>
		/// <param name="screenId">Which screen you want to get the offset of.</param>
		/// <returns>The offset for the given screen.</returns>
		Vector GetOffset(int screenId = 0) const { return m_Screens[screenId].Offset; }

		/// <summary>
		/// Sets the offset (scroll position) of the terrain.
		/// </summary>
		/// <param name="offset">The new offset value.</param>
		/// <param name="screenId">Which screen you want to set the offset of.</param>
		void SetOffset(const Vector &offset, int screenId = 0);

		/// <summary>
		/// Gets the difference in current offset and that of the Update() before.
		/// </summary>
		/// <returns>The delta offset in pixels.</returns>
		Vector GetDeltaOffset(int screenId = 0) const { return m_Screens[screenId].DeltaOffset; }

		/// <summary>
		/// Gets the offset (scroll position) of the terrain, without taking wrapping into account.
		/// </summary>
		/// <param name="screenId">Which screen you want to get the offset of.</param>
		/// <returns>The offset for the given screen.</returns>
		Vector GetUnwrappedOffset(int screenId = 0) const;

		/// <summary>
		/// Sets the offset (scroll position) of the terrain to center on specific world coordinates.
		/// If the coordinate to center on is close to the terrain border edges, the view will not scroll outside the borders.
		/// </summary>
		/// <param name="center">The coordinates to center the terrain scroll on.</param>
		/// <param name="screenId">Which screen you want to set the offset of.</param>
		void SetScroll(const Vector &center, int screenId = 0);

		/// <summary>
		/// Gets the team associated with a specific screen.
		/// </summary>
		/// <param name="screenId">Which screen you want to get the team of.</param>
		/// <returns>The team associated with the screen.</returns>
		int GetScreenTeam(int screenId = 0) const { return m_Screens[screenId].ScreenTeam; }

		/// <summary>
		/// Sets the team associated with a specific screen.
		/// </summary>
		/// <param name="team">The team to set the screen to.</param>
		/// <param name="screenId">Which screen you want to set the team of.</param>
		void SetScreenTeam(int team, int screenId = 0) { m_Screens[screenId].ScreenTeam = team; }

		/// <summary>
		/// Gets the amount that a specific screen is occluded by a GUI panel or something of the sort.
		/// This will affect how the scroll target translates into the offset of the screen, in order to keep the target centered on the screen.
		/// </summary>
		/// <param name="screenId">Which screen you want to get the team of.</param>
		/// <returns>A vector indicating the screen occlusion amount.</returns>
		Vector & GetScreenOcclusion(int screenId = 0) { return m_Screens[screenId].ScreenOcclusion; }

		/// <summary>
		/// Sets the amount that a specific screen is occluded by a GUI panel or something of the sort.
		/// This will affect how the scroll target translates into the offset of the screen, in order to keep the target centered on the screen.
		/// </summary>
		/// <param name="occlusion">The amount of occlusion of the screen.</param>
		/// <param name="screenId">Which screen you want to set the occlusion of.</param>
		void SetScreenOcclusion(const Vector &occlusion, int screenId = 0) { m_Screens[screenId].ScreenOcclusion = occlusion; }

		/// <summary>
		/// Gets the currently set scroll target, i.e. where the center of the specific screen is trying to line up with.
		/// </summary>
		/// <param name="screenId">Which screen to get the target for.</param>
		/// <returns>Current target vector in Scene coordinates.</returns>
		Vector GetScrollTarget(int screenId = 0) const;

		/// <summary>
		/// Interpolates a smooth scroll of the view from wherever it is now, towards centering on a new scroll target over time.
		/// </summary>
		/// <param name="targetCenter">The new target vector in Scene coordinates.</param>
		/// <param name="speed">The normalized speed at screen the view scrolls. 0 being no movement, and 1.0 being instant movement to the target in one frame.</param>
		/// <param name="targetWrapped">Whether the target was wrapped around the scene this frame or not.</param>
		/// <param name="screenId">Which screen you want to set the scroll offset of.</param>
		void SetScrollTarget(const Vector &targetCenter, float speed = 0.1F, bool targetWrapped = false, int screenId = 0);

		/// <summary>
		/// Calculates a scalar of how distant a certain point in the world is from the currently closest scroll target of all active screens.
		/// </summary>
		/// <param name="point">The world coordinate point to check distance to/from.</param>
		/// <returns>
		/// A normalized scalar representing the distance between the closest scroll target of all active screens, to the passed in point.
		/// 0 means it's the point is within half a screen's width of the target, and 1.0 means it's on the clear opposite side of the scene.
		/// </returns>
		float TargetDistanceScalar(const Vector &point) const;

		/// <summary>
		/// Makes sure the current offset won't create a view of outside the scene.
		/// If that is found to be the case, the offset is corrected so that the view rectangle
		/// is as close to the old offset as possible, but still entirely within the scene world.
		/// </summary>
		/// <param name="screenId">Which screen you want to check the offset of.</param>
		void CheckOffset(int screenId = 0);

		/// <summary>
		/// Gets the frame width/height for a given screen.
		/// </summary>
		/// <param name="screenId">Which screen you want to get frame width/height of.</param>
		/// <returns>The frame width (x) and height (y).</returns>
		Vector GetFrameSize(int screenId = 0);
#pragma endregion

#pragma region Screen Shake Getters and Setters
		/// <summary>
		/// Gets the screen shake strength multiplier.
		/// </summary>
		/// <returns>The screen shake strength multiplier.</returns>
		float GetScreenShakeStrength() const { return m_ScreenShakeStrength; }

		/// <summary>
		/// Sets the screen shake strength multiplier.
		/// </summary>
		/// <param name="newValue">New value for the screen shake strength multiplier.</param>
		void SetScreenShakeStrength(float newValue) { m_ScreenShakeStrength = newValue; }

		/// <summary>
		/// Gets how quickly screen shake decays, per second.
		/// </summary>
		/// <returns>The screen shake decay.</returns>
		float GetScreenShakeDecay() const { return m_ScreenShakeDecay; }

		/// <summary>
		/// Gets the maximum amount of screen shake time, i.e. the number of seconds screen shake will happen until ScreenShakeDecay reduces it to zero.
		/// </summary>
		/// <returns>The maximum screen shake time, in seconds.</returns>
		float GetMaxScreenShakeTime() const { return m_MaxScreenShakeTime; }

		/// <summary>
		/// Gets how much the screen should shake per unit of energy from gibbing (i.e explosions), when screen shake amount is auto-calculated.
		/// </summary>
		/// <returns>The default shakiness per unit of gib energy.</returns>
		float GetDefaultShakePerUnitOfGibEnergy() const { return m_DefaultShakePerUnitOfGibEnergy; }

		/// <summary>
		/// Gets how much the screen should shake per unit of energy for recoil, when screen shake amount is auto-calculated.
		/// </summary>
		/// <returns>The default shakiness per unit of recoil energy.</returns>
		float GetDefaultShakePerUnitOfRecoilEnergy() const { return m_DefaultShakePerUnitOfRecoilEnergy; }

		/// <summary>
		/// The maximum amount of screen shake recoil can cause, when screen shake is auto-calculated. This is ignored by per-firearm shake settings.
		/// </summary>
		/// <returns>The maximum auto-calculated recoil shakiness.</returns>
		float GetDefaultShakeFromRecoilMaximum() const { return m_DefaultShakeFromRecoilMaximum; }
#pragma endregion

#pragma region Screen Shake Actions
		/// <summary>
		/// Increases the magnitude of screen shake.
		/// This is used for spatially located screen-shake, and will automatically determine which screens have shake applied
		/// If the screen-shake position is outside our view, it'll gradually weaken and fade away depending on distance.
		/// </summary>
		/// <param name="magnitude">The amount of screen shake.</param>
		/// <param name="position">The spatial location of the screen-shake event.</param>
		void AddScreenShake(float magnitude, const Vector &position);

		/// <summary>
		/// Increases the magnitude of screen shake.
		/// </summary>
		/// <param name="magnitude">The amount of screen shake to add.</param>
		/// <param name="screenId">Which screen you want to add screen-shake to.</param>
		void AddScreenShake(float magnitude, int screenId = 0) { m_Screens[screenId].ScreenShakeMagnitude += magnitude; }

		/// <summary>
		/// Sets the magnitude of screen shake.
		/// </summary>
		/// <param name="magnitude">The amount of screen shake.</param>
		/// <param name="screenId">Which screen you want to set screen-shake for.</param>
		void SetScreenShake(float magnitude, int screenId = 0) { m_Screens[screenId].ScreenShakeMagnitude = magnitude; }

		/// <summary>
		/// Applies screen shake to be at least magnitude.
		/// </summary>
		/// <param name="magnitude">The amount of screen shake.</param>
		/// <param name="screenId">Which screen you want to set screen-shake for.</param>
		void ApplyScreenShake(float magnitude, int screenId = 0) { m_Screens[screenId].ScreenShakeMagnitude = std::max(magnitude, m_Screens[screenId].ScreenShakeMagnitude); }
#pragma endregion

#pragma region Concrete Methods
		/// <summary>
		/// Updates the state of this CameraMan. Supposed to be done every frame before drawing.
		/// </summary>
		void Update(int screenId);
#pragma endregion

	private:

		/// <summary>
		/// A screen. Each player should have one of these.
		/// </summary>
		/// TODO: This is a struct right now, as it has been torn verbatim out of SceneMan. In future it should be a proper class with methods, instead of CameraMan handling everything.
		struct Screen {
			int ScreenTeam = 0; //!< The team associated with this Screen.

			Vector Offset; //!< The position of the upper left corner of the view.
			Vector DeltaOffset; //!< The difference in current offset and the Update() before.
			Vector ScrollTarget; //!< The final offset target of the current scroll interpolation, in scene coordinates.

			Timer ScrollTimer; //!< Scroll timer for making scrolling work framerate independently.
			float ScrollSpeed = 0; //!< The normalized speed the screen's view scrolls. 0 being no movement, and 1.0 being instant movement to the target in one frame.

			bool TargetWrapped = false; //!< Whether the ScrollTarget got wrapped around the world this frame or not.
			std::array<int, 2> SeamCrossCount = { 0, 0 }; //!< Keeps track of how many times and in screen directions the wrapping seam has been crossed. This is used for keeping the background layers' scroll from jumping when wrapping around. X and Y.

			Vector ScreenOcclusion; //!< The amount a screen is occluded or covered by GUI, etc.

			float ScreenShakeMagnitude = 0; //!< The magnitude of screen shake that is currently being applied.
		};

		float m_ScreenShakeStrength; //!< A global multiplier applied to screen shaking strength.
		float m_ScreenShakeDecay; //!< How quickly screen shake falls off.
		float m_MaxScreenShakeTime; //!< The maximum amount of screen shake time, i.e. the number of seconds screen shake will happen until ScreenShakeDecay reduces it to zero.
		float m_DefaultShakePerUnitOfGibEnergy; //!< How much the screen should shake per unit of energy from gibbing (i.e explosions), when screen shake amount is auto-calculated.
		float m_DefaultShakePerUnitOfRecoilEnergy; //!< How much the screen should shake per unit of energy for recoil, when screen shake amount is auto-calculated.
		float m_DefaultShakeFromRecoilMaximum; //!< The maximum amount of screen shake recoil can cause, when screen shake is auto-calculated. This is ignored by per-firearm shake settings.

		std::array<Screen, c_MaxScreenCount> m_Screens;

		/// <summary>
		/// Clears all the member variables of this CameraMan, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();

		// Disallow the use of some implicit methods.
		CameraMan(const CameraMan &reference) = delete;
		CameraMan & operator=(const CameraMan &rhs) = delete;
	};
}
#endif