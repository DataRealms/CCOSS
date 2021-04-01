#ifndef _PIEMENUGUI_
#define _PIEMENUGUI_

#include "PieSlice.h"
#include "Controller.h"
#include "Icon.h"

namespace RTE {

	class GUIFont;
	class MovableObject;
	class Actor;

	class PieMenuGUI {

	public:

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a PieMenuGUI object in system memory. Create() should be called before using the object.
		/// </summary>
		PieMenuGUI() { Clear(); }

		/// <summary>
		/// Makes the PieMenuGUI object ready for use.
		/// </summary>
		/// <param name="controller">A pointer to a Controller which will control this Menu. Ownership is NOT TRANSFERRED!</param>
		/// <param name="focusActor">The actor that this menu is currently associated with. Ownership is NOT TRANSFERRED! This is optional.</param>
		/// <returns>An error return value signaling sucess or any particular failure. Anything below 0 is an error signal.</returns>
		int Create(Controller *controller, Actor *focusActor = nullptr);
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Destructor method used to clean up a PieMenuGUI object before deletion from system memory.
		/// </summary>
		~PieMenuGUI() { Destroy(); }

		/// <summary>
		/// Destroys and resets (through Clear()) the PieMenuGUI object.
		/// </summary>
		void Destroy() { destroy_bitmap(m_BGBitmap); Clear(); }

		/// <summary>
		/// Resets the entire PieMenuGUI, including its inherited members, to their default settings or values.
		/// </summary>
		void Reset() { Clear(); }
#pragma endregion

#pragma region Getters and Setters
		/// <summary>
		/// Sets the controller used by this. The ownership of the controller is NOT transferred!
		/// </summary>
		/// <param name="controller">The new controller for this menu. Ownership is NOT transferred!</param>
		void SetController(Controller *controller) { m_Controller = controller; }

		/// <summary>
		/// Sets the currently attached-to Actor. It will populate this menu with some of its options. The ownership of the Actor is NOT transferred!
		/// </summary>
		/// <param name="actor">The new actor associated for this menu. Ownership is NOT transferred!</param>
		void SetActor(Actor *actor) { m_Actor = actor; m_LastKnownActor = actor; }

		/// <summary>
		/// Enables or disables the menu and animates it in and out of view.
		/// </summary>
		/// <param name="enable">Whether to enable or disable the menu.</param>
		void SetEnabled(bool enable);

		/// <summary>
		/// Gets whether the menu is enabled or not.
		/// </summary>
		/// <returns>Whether the menu is enabled.</returns>
		bool IsEnabled() const { return (m_PieEnabled == ENABLED || m_PieEnabled == ENABLING) && !m_Wobbling; }

		/// <summary>
		/// Gets whether the menu is at all visible or not.
		/// </summary>
		/// <returns>Whether the menu is visible.</returns>
		bool IsVisible() const { return m_PieEnabled != DISABLED; }

		/// <summary>
		/// Gets the absolute center position of this.
		/// </summary>
		/// <returns>A Vector describing the current absolute position in pixels.</returns>
		const Vector & GetPos() const { return m_CenterPos; }

		/// <summary>
		/// Sets the absolute center position of this in the scene.
		/// </summary>
		/// <param name="newPos">A Vector describing the current absolute position in pixels, in the scene.</param>
		void SetPos(const Vector &newPos) { m_CenterPos = newPos; }

		/// <summary>
		/// Gets the the command issued by this menu in the last update, i.e. the Slice type of the currently activated Slice or None if no slice was activated.
		/// </summary>
		/// <returns>The Slice type which has been picked activated, or None if none has been picked. See the PieSliceIndex enum for Slice types.</returns>
		int GetPieCommand() const { return m_ActivatedSlice == nullptr ? PieSlice::PSI_NONE : m_ActivatedSlice->GetType(); }
#pragma endregion

#pragma region Animation Handling
		/// <summary>
		/// Just plays the disabling animation, regardless of whether the menu was enabled or not. This is for an effect when actors are switched.
		/// </summary>
		void DisableAnim() { m_InnerRadius = m_EnabledRadius; m_Wobbling = false; m_EnablingTimer.Reset(); m_PieEnabled = DISABLING; }

		/// <summary>
		/// Plays an animation of the pie menu circle expanding and contracting continuously. The menu is effectively disabled while doing this. It will continue until the next call to SetEnabled.
		/// </summary>
		void WobbleAnim() { m_Wobbling = true; m_Freeze = false; }

		/// <summary>
		/// Makes the background circle freeze at a certain radius until SetEnabled is called. The menu is effectively disabled while doing this.
		/// </summary>
		/// <param name="radius">The radius to make the background circle freeze at.</param>
		void FreezeAtRadius(int radius) { m_Freeze = true; m_Wobbling = false; m_InnerRadius = radius; m_RedrawBG = true; }
#pragma endregion

#pragma region Slice Handling
		/// <summary>
		/// Resets and removes all Slices from the menu so that new ones can be added.
		/// </summary>
		void ResetSlices();

		/// <summary>
		/// Adds a Slice to the menu. It will be placed according to what's already in there, and what placement apriority parameters it has.
		/// </summary>
		/// <param name="newSlice">The new slice to add.</param>
		/// <param name="takeAnyFreeCardinal">Whether the new Slice can be placed on the closest free cardinal if the one specified in it isn't free. If false, it will be placed in a corner spot as close to its desired direction as possible.</param>
		/// <returns>Whether the Slice was added successfully. If there wasn't enough room or there was duplicate Slice, then this will return false.</returns>
		bool AddSlice(PieSlice &newSlice, bool takeAnyFreeCardinal = false);

		/// <summary>
		/// Gets the current Slices in the menu.
		/// </summary>
		std::vector<PieSlice *> GetCurrentSlices() const { return m_CurrentSlices; };

		/// <summary>
		/// Makes sure all currently added Slices' areas are set up correctly position and sizewise on the pie menu circle.
		/// </summary>
		void RealignSlices();

		/// <summary>
		/// Gets a specific Slice based on an angle on the pie menu.
		/// </summary>
		/// <param name="angle">An angle on the circle, in radins, CCW from straight out right.</param>
		/// <returns>The Slice which exists on that angle, if any. 0 if not. Ownership is NOT transferred!</returns>
		const PieSlice * GetSliceOnAngle(float angle) const;

		/// <summary>
		/// Reports whether and which Slice has been activated by the player. This may happen even though the player isn't done with the pie menu.
		/// </summary>
		/// <returns>The Slice which has been picked by the player, if any. 0 if not. Ownership is NOT transferred!</returns>
		const PieSlice * SliceActivated() const { return m_ActivatedSlice; }
#pragma endregion

#pragma region Lua Slice Handling
		/// <summary>
		/// Adds a Slice to the map of custom Lua slices.
		/// </summary>
		/// <param name="newSlice">The Slice to add to the map of custom Lua slices.</param>
		static void StoreCustomLuaSlice(PieSlice newSlice) { s_AllCustomLuaSlices[newSlice.GetDescription() + "::" + newSlice.GetFunctionName()] = newSlice; }

		/// <summary>
		/// Adds a Slice to the menu, searching by description and functionName for a slice defined in INI. Should only be used by Lua.
		/// </summary>
		/// <param name="description">The description to search for.</param>
		/// <param name="functionName">The functionName to search for.</param>
		/// <param name="direction">The direction the Slice should be added at.</param>
		/// <param name="isEnabled">Whether the Slice should be enabled or disabled.</param>
		/// <returns>Whether or not the Slice was succesfully added.</returns>
		bool AddSliceLua(const std::string &description, const std::string &functionName, PieSlice::SliceDirection direction, bool isEnabled);

		/// <summary>
		/// Removes a Slice from the menu, searching for it by description and functionName. Should only be used by Lua.
		/// </summary>
		/// <param name="description">The description to search for.</param>
		/// <param name="functionName">The functionName to search for.</param>
		/// <returns>The Slice that was removed.</returns>
		PieSlice RemoveSliceLua(const std::string &description, const std::string &functionName);

		/// <summary>
		/// Alters a Slice in the menu, searching for it by description and functionName, and setting its direction and enabled status according to the given arguments. Should only be used by Lua.
		/// </summary>
		/// <param name="description">The description to search for.</param>
		/// <param name="functionName">The functionName to search for.</param>
		/// <param name="direction">The direction the Slice should be moved to.</param>
		/// <param name="isEnabled">Whether the Slice should be enabled or disabled.</param>
		void AlterSliceLua(const std::string &description, const std::string &functionName, PieSlice::SliceDirection direction, bool isEnabled);
#pragma endregion

#pragma region Updating
		/// <summary>
		/// Updates the state of this Menu each frame.
		/// </summary>
		virtual void Update();

		/// <summary>
		/// Draws the menu.
		/// </summary>
		/// <param name="targetBitmap">A pointer to a BITMAP to draw on. Generally a screen BITMAP.</param>
		/// <param name="targetPos">The absolute position of the target bitmap's upper left corner in the scene.</param>
		virtual void Draw(BITMAP *targetBitmap, const Vector &targetPos = Vector()) const;
#pragma endregion

	private:

		enum PieEnabled {
			ENABLING = 0,
			ENABLED,
			DISABLING,
			DISABLED
		};

		enum PieIconSelection {
			PIS_NORMAL = 0,
			PIS_SELECTED,
			PIS_DISABLED,
			PIS_COUNT
		};

		static std::unordered_map<std::string, PieSlice> s_AllCustomLuaSlices; //<! All Slices ever added to this pie-menu, serves as directory of Slices available to add.
		static BITMAP *s_Cursor; //<! A static pointer to the bitmap to use as the cursor in any menu.
		static const int s_EnablingDelay = 50; //<! Time in ms for how long it takes to enable/disable.
		
		Timer m_EnablingTimer; //<! Timer for the appear and disappear animations.
		Timer m_HoverTimer; //<! Timer to measure how long to hold a hovered over slice.
		
		Controller *m_Controller; //<! The Controller which controls this menu. Not owned.
		Actor *m_Actor; //<! The actor that this menu is attached to and getting some options from.
		Actor *m_LastKnownActor; //<! This actor pointer is not cleared every time, I'm not touching the original to not ruin something. ?????
		
		PieEnabled m_PieEnabled; //<! The enabled state of the menu.
		Vector m_CenterPos; //<! The center position of this in the scene.
		
		const PieSlice *m_HoveredSlice; //<! The Slice currently being hovered over.
		const PieSlice *m_ActivatedSlice; //<! The currently activated Slice, if there is one, or 0 if there's not.
		const PieSlice *m_AlreadyActivatedSlice; //<! The Slice that was most recently activated by pressing Primary. Used to avoid duplicate activation when releasing the pie menu.
		
		//<! The cardinal axis slices, owned here
		PieSlice m_UpSlice;
		PieSlice m_RightSlice;
		PieSlice m_DownSlice;
		PieSlice m_LeftSlice;

		//<! The slices between the cardinal axes, owned here
		std::list<PieSlice> m_UpRightSlices;
		std::list<PieSlice> m_UpLeftSlices;
		std::list<PieSlice> m_DownLeftSlices;
		std::list<PieSlice> m_DownRightSlices;

		std::vector<PieSlice *> m_CurrentSlices; //<! All the Slices, in order and aligned, not owned here, just pointing to the ones above.
		int m_SliceGroupCount; //<! How many groups there currently are in the menu.

		bool m_Wobbling; //<! Special mode where the menu circle expands and contracts continuously, while being effectively disabled.
		bool m_Freeze; //<! Special mode where the menu circle is frozen at the current m_InnerRadius.

		int m_InnerRadius; //<! The current radius of the innermost circle of the pie menu, in pixels.
		int m_EnabledRadius; //<! When fully enabled, the inner radius is this, in pixels.
		int m_Thickness; //<! The thickness of the pie menu circle, in pixels.
		float m_CursorAngle; //<! Position of the cursor on the circle, in radians, counterclockwise from straight out to rhe right.
		
		BITMAP *m_BGBitmap; //<! The intermediary bitmap used to first draw the menu background, which will be blitted to the final draw target surface.
		bool m_RedrawBG; //<! Whether we need to redraw the BG bitmap.

		/// <summary>
		/// Sets a slice to be selected.
		/// </summary>
		/// <param name="sliceToSelect">The slice to be selected. Has to be a slice currently in this menu.</param>
		/// <param name="moveCursorToSlice">Whether to also move the cursor to the center of the newly selected slice. Defaults to false.</param>
		/// <returns>Whether or not this resulted in a different slice being selected. Also returns false if a null slice was passed in.</returns>
		bool SelectSlice(const PieSlice *sliceToSelect, bool moveCursorToSlice = false);

		/// <summary>
		/// Handles the wobbling portion of Update.
		/// </summary>
		void UpdateWobbling();

		/// <summary>
		/// Handles the enabling and disabling part of Update.
		/// </summary>
		void UpdateEnablingAndDisablingAnimations();

		/// <summary>
		/// Handles the analog input conversion part of Update.
		/// </summary>
		/// <returns>Whether or not enough input was received to do something.</returns>
		bool UpdateAnalogInput();

		/// <summary>
		/// Handles the digital input conversion part of Update.
		/// </summary>
		/// <returns>Whether or not enough input was received to do something.</returns>
		bool UpdateDigitalInput();

		/// <summary>
		/// Handles the slice activation part of Update.
		/// </summary>
		void UpdateSliceActivation();

		/// <summary>
		/// Handles background and slice separator redrawing.
		/// </summary>
		void RedrawMenuBackground();

		/// <summary>
		/// Handles figuring out the position to draw the menu at, accounting for any Scene seams.
		/// </summary>
		/// <param name="targetBitmap">A pointer to the BITMAP to draw on. Generally a screen BITMAP.</param>
		/// <param name="targetPos">The absolute position of the target bitmap's upper left corner in the scene.</param>
		/// <param name="menuFont">The font being used for text in the menu.</param>
		/// <param name="drawPos">Out param, a Vector to be filled in with the position at which the menu should be drawn.</param>
		void CalculateDrawPositionAccountingForSeamsAndFont(const BITMAP *targetBitmap, const Vector &targetPos, GUIFont *menuFont, Vector &drawPos) const;

		/// <summary>
		/// Handles drawing icons on pie menu slices.
		/// </summary>
		/// <param name="targetBitmap">A pointer to the BITMAP to draw on. Generally a screen BITMAP.</param>
		/// <param name="drawPos">The seam corrected position at which the pie menu is being drawn.</param>
		void DrawPieIcons(BITMAP *targetBitmap, const Vector &drawPos) const;

		/// <summary>
		/// Handles drawing the cursor and description text for selected slices.
		/// </summary>
		/// <param name="targetBitmap">A pointer to the BITMAP to draw on. Generally a screen BITMAP.</param>
		/// <param name="drawPos">The seam corrected position at which the pie menu is being drawn.</param>
		/// <param name="menuFont">The font being used for text in the menu.</param>
		void DrawPieCursorAndSliceDescriptions(BITMAP *targetBitmap, const Vector &drawPos, GUIFont *menuFont) const;

		/// <summary>
		/// Clears all the member variables of this PieMenuGUI, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();

		// Forbidding copying
		PieMenuGUI(const PieMenuGUI &reference) = delete;
		PieMenuGUI & operator=(const PieMenuGUI & rhs) = delete;
	};
}
#endif
