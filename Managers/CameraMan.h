#ifndef _RTECAMERAMAN_
#define _RTECAMERAMAN_

#include "Constants.h"
#include "Singleton.h"
#include "Timer.h"
#include "Vector.h"

#define g_CameraMan CameraMan::Instance()

namespace RTE {

	class Vector;

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
		int Initialize();
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Destructor method used to clean up a CameraMan object before deletion from system memory.
		/// </summary>
		~CameraMan() { Destroy(); }

		/// <summary>
		/// Destroys and resets (through Clear()) the CameraMan object.
		/// </summary>
		void Destroy();
#pragma endregion

#pragma region Getters and Setters
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
		/// Gets the maximum amount of screenshakiness, in how many seconds until ScreenShakeDecay reduces it to zero.
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

#pragma region Screen Handling
		/// <summary>
		/// Sets the offset (scroll position) of the terrain.
		/// </summary>
		/// <param name="offset">The new offset value.</param>
		/// <param name="screenId">Which screen you want to set the offset of.</param>
		void SetOffset(const Vector &offset, int screenId = 0);

		/// <summary>
		/// Gets the offset (scroll position) of the terrain.
		/// </summary>
		/// <param name="screenId">Which screen you want to get the offset of.</param>
		/// <returns>The offset for the given screen.</returns>
		Vector GetOffset(int screenId = 0) const { return m_Screens[screenId].m_Offset; }

		/// <summary>
		/// Gets the difference in current offset and that of the Update() before.
		/// </summary>
		/// <returns>The delta offset in pixels.</returns>
		Vector GetDeltaOffset(int screenId = 0) const { return m_Screens[screenId].m_DeltaOffset; }

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
		void SetScroll(const Vector& center, int screenId = 0);

		/// <summary>
		/// Sets the team associated with a specific screen.
		/// </summary>
		/// <param name="team">The team to set the screen to.</param>
		/// <param name="screenId">Which screen you want to set the team of.</param>
		void SetScreenTeam(int team, int screenId = 0) { m_Screens[screenId].m_ScreenTeam = team; }

		/// <summary>
		/// Gets the team associated with a specific screen.
		/// </summary>
		/// <param name="screenId">Which screen you want to get the team of.</param>
		/// <returns>The team associated with the screen.</returns>
		int GetScreenTeam(int screenId = 0) const { return m_Screens[screenId].m_ScreenTeam; }

		/// <summary>
		/// Sets the amount that a specific screen is occluded by a GUI panel or something of the sort. 
		/// This will affect how the scroll target translates into the offset of the screen, in order to keep the target centered on the screen.
		/// </summary>
		/// <param name="occlusion">The amount of occlusion of the screen.</param>
		/// <param name="screenId">Which screen you want to set the occlusion of.</param>
		void SetScreenOcclusion(const Vector &occlusion, int screenId = 0) { m_Screens[screenId].m_ScreenOcclusion = occlusion; }

		/// <summary>
		/// Gets the amount that a specific screen is occluded by a GUI panel or something of the sort. 
		/// This will affect how the scroll target translates into the offset of the screen, in order to keep the target centered on the screen.
		/// </summary>
		/// <returns>A vector indicating the screen occlusion amount.</returns>
		/// <param name="screenId">Which screen you want to get the team of.</param>
		Vector& GetScreenOcclusion(int screenId = 0) { return m_Screens[screenId].m_ScreenOcclusion; }

		/// <summary>
		/// Interpolates a smooth scroll of the view from wherever it is now, towards centering on a new scroll target over time.
		/// </summary>
		/// <param name="targetCenter">The new target vector in *scene coordinates*.</param>
		/// <param name="speed">The normalized speed at screen the view scrolls. 0 being no movement, and 1.0 being instant movement to the target in one frame.</param>
		/// <param name="targetWrapped">Whether the target was wrapped around the scene this frame or not.</param>
		/// <param name="screenId">Which screen you want to set the scroll offset of.</param>
		void SetScrollTarget(const Vector &targetCenter, float speed = 0.1F, bool targetWrapped = false, int screenId = 0);

		/// <summary>
		/// Gets the currently set scroll target, screen is where the center of the specific screen is trying to line up with.
		/// </summary>
		/// <param name="screenId">Which screen to get the target for.</param>
		/// <returns>Current target vector in *scene coordinates*.</returns>
		Vector GetScrollTarget(int screenId = 0) const;

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
		/// <param name="screen">Which screen you want to check the offset of.</param>
		void CheckOffset(int screenId = 0);

		/// <summary>
		/// Gets the frame width/height for a given screen.
		/// </summary>
		/// <param name="screen">Which screen you want to get frame width/height.</param>
		/// <returns>The frame width (x) and height (y).</returns>
		Vector GetFrameSize(int screenId = 0);
#pragma endregion

#pragma region Screen Shake
		/// <summary>
		/// Increases the magnitude of screen shake. 
		/// This is used for spatially located screenshake, and will automatically determine which screens have shake applied
		/// If the screenshake position is outside our view, it'll gradually weaken and fade away depending on distance.
		/// </summary>
		/// <param name="magnitude">The amount of screen shake.</param>
		/// <param name="position">The spatial location of the screen-shake event.</param>
		void AddScreenShake(float magnitude, const Vector& position);

		/// <summary>
		/// Increases the magnitude of screen shake.
		/// </summary>
		/// <param name="magnitude">The amount of screen shake to add.</param>
		/// <param name="screenId">Which screen you want to add screenshake to.</param>
		void AddScreenShake(float magnitude, int screenId = 0) { m_Screens[screenId].m_ScreenShakeMagnitude += magnitude; }

		/// <summary>
		/// Sets the magnitude of screen shake.
		/// </summary>
		/// <param name="magnitude">The amount of screen shake.</param>
		/// <param name="screenId">Which screen you want to set screenshake for.</param>
		void SetScreenShake(float magnitude, int screenId = 0) { m_Screens[screenId].m_ScreenShakeMagnitude = magnitude; }

		/// <summary>
		/// Applies screen shake to be at least magnitude.
		/// </summary>
		/// <param name="magnitude">The amount of screen shake.</param>
		/// <param name="screenId">Which screen you want to set screenshake for.</param>
		void ApplyScreenShake(float magnitude, int screenId = 0) { m_Screens[screenId].m_ScreenShakeMagnitude = std::max(magnitude, m_Screens[screenId].m_ScreenShakeMagnitude); }
#pragma endregion

#pragma region Concrete Methods
		/// <summary>
		/// Updates the state of this CameraMan. Supposed to be done every frame before drawing.
		/// </summary>
		void Update(int screenId);
#pragma endregion

	private:

		/// <summary>
		/// A screen. Each player should have one of these
		/// </summary>
		/// TODO: This is a struct right now, as it has been torn verbatim out of SceneMan. In future it should be a proper class with methods, instead of CameraMan handling everything.
		struct Screen {
			// The position of the upper left corner of the view.
			Vector m_Offset;
			// The difference in current offset and the Update() before.
			Vector m_DeltaOffset;
			// The final offset target of the current scroll interpolation, in scene coordinates!
			Vector m_ScrollTarget;
			// The team associated with each screen.
			int m_ScreenTeam;
			// The amount a screen is occluded or covered by GUI, etc
			Vector m_ScreenOcclusion;
			// The normalized speed at screen the view scrolls.
			// 0 being no movement, and 1.0 being instant movement to the target in one frame.
			float m_ScrollSpeed;
			// Scroll timer for making scrolling work framerate independently
			Timer m_ScrollTimer;
			// Whether the ScrollTarget got wrapped around the world this frame or not.
			bool m_TargetWrapped;
			// Keeps track of how many times and in screen directions the wrapping seam has been crossed.
			// This is used fo keeping the background layers' scroll from jumping when wrapping around.
			// X and Y
			int m_SeamCrossCount[2];
			// The magnitude of screen shake that is currently being applied
			float m_ScreenShakeMagnitude;
		};

		float m_ScreenShakeStrength; //!< A global multiplier applied to screen shaking strength.
		float m_ScreenShakeDecay; //!< How quickly screen shake falls off.
		float m_MaxScreenShakeTime; //!< The maximum amount of screenshakiness, in how many seconds until ScreenShakeDecay reduces it to zero.
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