#ifndef _PIEMENUGUI_
#define _PIEMENUGUI_

#include "PieSlice.h"
#include "Timer.h"
#include "Vector.h"

namespace RTE {

	class Controller;
	class MovableObject;
	class GUIFont;

	/// <summary>
	/// A GUI for displaying pie menus.
	/// </summary>
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
		/// <param name="controller">A pointer to a Controller which will control this Menu. Ownership is NOT transferred!</param>
		/// <param name="affectedObject">The object that this menu is will affect. Ownership is NOT transferred! This is optional.</param>
		/// <returns>An error return value signaling sucess or any particular failure. Anything below 0 is an error signal.</returns>
		int Create(Controller *controller, MovableObject *affectedObject = nullptr);
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Resets the entire PieMenuGUI, including its inherited members, to their default settings or values.
		/// </summary>
		void Reset() { Clear(); }
#pragma endregion

#pragma region Getters and Setters
		/// <summary>
		/// Sets the controller used by this. Ownership is NOT transferred!
		/// </summary>
		/// <param name="controller">The new controller for this menu. Ownership is NOT transferred!</param>
		void SetController(Controller *controller) { m_MenuController = controller; }

		/// <summary>
		/// Gets the currently affected MovableObject. Ownership is NOT transferred!
		/// </summary>
		/// <returns>The MovableObject this menu affects. Ownership is NOT transferred!</returns>
		const MovableObject * GetAffectedObject() const { return m_AffectedObject; }

		/// <summary>
		/// Sets the MovableObject this menu should affect. Ownership is NOT transferred!
		/// </summary>
		/// <param name="affectedObject">The new MovableObject affected by this menu. Ownership is NOT transferred!</param>
		void SetAffectedObject(MovableObject *affectedObject) { m_AffectedObject = affectedObject; }

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
		/// Gets whether the menu is enabled or not.
		/// </summary>
		/// <returns>Whether the menu is enabled.</returns>
		bool IsEnabled() const { return (m_EnabledState == EnabledState::Enabled || m_EnabledState == EnabledState::Enabling) && m_MenuMode != MenuMode::Wobble; }

		/// <summary>
		/// Gets whether the menu is in the process of enabling or disabling.
		/// </summary>
		/// <returns></returns>
		bool IsEnablingOrDisabling() const { return (m_EnabledState == EnabledState::Enabling || m_EnabledState == EnabledState::Disabling) && m_MenuMode != MenuMode::Wobble; }

		/// <summary>
		/// Gets whether the menu is at all visible or not.
		/// </summary>
		/// <returns>Whether the menu is visible.</returns>
		bool IsVisible() const { return m_EnabledState != EnabledState::Disabled; }

		/// <summary>
		/// Enables or disables the menu and animates it in and out of view.
		/// </summary>
		/// <param name="enable">Whether to enable or disable the menu.</param>
		void SetEnabled(bool enable);

		/// <summary>
		/// Gets the the command issued by this menu in the last update, i.e. the PieSlice type of the currently activated PieSlice or None if no slice was activated.
		/// </summary>
		/// <returns>The PieSlice type which has been picked activated, or None if none has been picked. See the PieSliceIndex enum for PieSlice types.</returns>
		PieSlice::PieSliceIndex GetPieCommand() const { return m_ActivatedSlice == nullptr ? PieSlice::PieSliceIndex::PSI_NONE : m_ActivatedSlice->GetType(); }
#pragma endregion

#pragma region Special Animation Handling
		/// <summary>
		/// Just plays the disabling animation, regardless of whether the menu was enabled or not.
		/// </summary>
		void DoDisableAnimation() { m_InnerRadius = m_FullRadius; m_MenuMode = MenuMode::Normal; m_EnableDisableAnimationTimer.Reset(); m_EnabledState = EnabledState::Disabling; }

		/// <summary>
		/// Plays an animation of the pie menu circle expanding and contracting continuously. The menu is effectively disabled while doing this. It will continue until the next call to SetEnabled.
		/// </summary>
		void Wobble() { m_MenuMode = MenuMode::Wobble; }

		/// <summary>
		/// Makes the background circle freeze at a certain radius until SetEnabled is called. The menu is effectively disabled while doing this.
		/// </summary>
		/// <param name="radius">The radius to make the background circle freeze at.</param>
		void FreezeAtRadius(int radius) { m_MenuMode = MenuMode::Freeze; m_InnerRadius = radius; m_BGBitmapNeedsRedrawing = true; }
#pragma endregion

#pragma region PieSlice Handling
		/// <summary>
		/// Resets and removes all Slices from the menu so that new ones can be added.
		/// </summary>
		void ResetSlices();

		/// <summary>
		/// Adds a PieSlice to the menu. It will be placed according to what's already in there, and what placement apriority parameters it has.
		/// </summary>
		/// <param name="newPieSlice">The new slice to add.</param>
		/// <param name="takeAnyFreeCardinal">Whether the new PieSlice can be placed on the closest free cardinal if the one specified in it isn't free. If false, it will be placed in a corner spot as close to its desired direction as possible.</param>
		/// <returns>Whether the PieSlice was added successfully. If there wasn't enough room or there was duplicate PieSlice, then this will return false.</returns>
		bool AddSlice(PieSlice &newPieSlice, bool takeAnyFreeCardinal = false);

		/// <summary>
		/// Gets the current Slices in the menu.
		/// </summary>
		std::vector<PieSlice *> GetCurrentSlices() const { return m_CurrentSlices; };

		/// <summary>
		/// Makes sure all currently added Slices' areas are set up correctly position and sizewise on the pie menu circle.
		/// </summary>
		void RealignSlices();
#pragma endregion

#pragma region Lua PieSlice Handling
		/// <summary>
		/// Adds a PieSlice to the map of custom Lua slices.
		/// </summary>
		/// <param name="newPieSlice">The PieSlice to add to the map of custom Lua slices.</param>
		static void StoreCustomLuaPieSlice(PieSlice newPieSlice) { s_AllCustomLuaSlices[newPieSlice.GetDescription() + "::" + newPieSlice.GetFunctionName()] = newPieSlice; }

		/// <summary>
		/// Adds a PieSlice to the menu, searching by description and functionName for a slice defined in INI. Should only be used by Lua.
		/// </summary>
		/// <param name="description">The description to search for.</param>
		/// <param name="functionName">The functionName to search for.</param>
		/// <param name="direction">The direction the PieSlice should be added at.</param>
		/// <param name="isEnabled">Whether the PieSlice should be enabled or disabled.</param>
		/// <returns>Whether or not the PieSlice was succesfully added.</returns>
		bool AddPieSliceLua(const std::string &description, const std::string &functionName, PieSlice::SliceDirection direction, bool isEnabled);

		/// <summary>
		/// Removes a PieSlice from the menu, searching for it by description and functionName. Should only be used by Lua.
		/// </summary>
		/// <param name="description">The description to search for.</param>
		/// <param name="functionName">The functionName to search for.</param>
		/// <returns>The PieSlice that was removed.</returns>
		PieSlice RemovePieSliceLua(const std::string &description, const std::string &functionName);

		/// <summary>
		/// Alters a PieSlice in the menu, searching for it by description and functionName, and setting its direction and enabled status according to the given arguments. Should only be used by Lua.
		/// </summary>
		/// <param name="description">The description to search for.</param>
		/// <param name="functionName">The functionName to search for.</param>
		/// <param name="direction">The direction the PieSlice should be moved to.</param>
		/// <param name="isEnabled">Whether the PieSlice should be enabled or disabled.</param>
		void AlterPieSliceLua(const std::string &description, const std::string &functionName, PieSlice::SliceDirection direction, bool isEnabled);
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

		/// <summary>
		/// Enumeration for enabled states when enabling/disabling the PieMenuGUI.
		/// </summary>
		enum class EnabledState { Enabling, Enabled, Disabling, Disabled };

		/// <summary>
		/// Enumeration for the the modes a PieMenuGUI can have.
		/// </summary>
		enum class MenuMode { Normal, Wobble, Freeze };

		/// <summary>
		/// Enumeration for the different item separator modes available to the PieMenuGUI.
		/// </summary>
		enum class IconSeparatorMode { Line, Circle };

		static const int c_EnablingDelay = 50; //!< Time in ms for how long it takes to enable/disable.

		//TODO replace this with proper presetman handling for pie slices, instead of adding them by name like this
		static std::unordered_map<std::string, PieSlice> s_AllCustomLuaSlices; //!< All Slices ever added to this pie-menu, serves as directory of Slices available to add.
		static BITMAP *s_CursorBitmap; //!< A static pointer to the bitmap to use as the cursor in any menu.

		GUIFont *m_LargeFont; //!< A pointer to the large font from FrameMan. Not owned here.

		Controller *m_MenuController; //!< The Controller which controls this menu. Separate from the Controller of the affected object (if there is one).
		MovableObject *m_AffectedObject; //!< The MovableObject this menu affects, if any.
		MenuMode m_MenuMode; //!< The mdoe this menu is in. See MenuMode enum for more details.
		Vector m_CenterPos; //!< The center position of this in the scene.

		EnabledState m_EnabledState; //!< The enabled state of the menu.
		Timer m_EnableDisableAnimationTimer; //!< Timer for progressing enabling/disabling animations.
		Timer m_HoverTimer; //!< Timer to measure how long to hold a hovered over slice.

		IconSeparatorMode m_IconSeparatorMode; //!< The icon separator mode of this menu.
		int m_BackgroundThickness; //!< The thickness of the menu's background, in pixels.
		int m_BackgroundSeparatorSize; //!< The size of the menu's background separators, in pixels. Used differently based on the menu's IconSeparatorMode.
		bool m_DrawBackgroundTransparent; //!< Whether or not the menu's background should be drawn transparently.
		int m_BackgroundColor; //!< The colour used for drawing the menu's background.
		int m_BackgroundBorderColor; //!< The colour used for drawing borders for the menu's background.
		int m_SelectedItemBackgroundColor; //!< The colour used for drawing selected menu items' backgrounds.
		
		const PieSlice *m_HoveredSlice; //!< The PieSlice currently being hovered over.
		const PieSlice *m_ActivatedSlice; //!< The currently activated PieSlice, if there is one, or 0 if there's not.
		const PieSlice *m_AlreadyActivatedSlice; //!< The PieSlice that was most recently activated by pressing Primary. Used to avoid duplicate activation when releasing the pie menu.
		
		/// <summary>
		/// The cardinal axis slices, owned here.
		/// </summary>
		PieSlice m_UpSlice;
		PieSlice m_LeftSlice;
		PieSlice m_DownSlice;
		PieSlice m_RightSlice;

		/// <summary>
		/// The slices between the cardinal axes, owned here.
		/// </summary>
		std::list<PieSlice> m_UpRightSlices;
		std::list<PieSlice> m_UpLeftSlices;
		std::list<PieSlice> m_DownLeftSlices;
		std::list<PieSlice> m_DownRightSlices;

		std::vector<PieSlice *> m_CurrentSlices; //!< All the PieSlices, in order and aligned. Not owned here, just pointing to the ones above.
		int m_SliceGroupCount; //!< How many groups there currently are in the menu.

		int m_FullRadius; //!< The radius the menu should have when fully enabled, in pixels.
		int m_InnerRadius; //!< The current radius of the innermost circle of the pie menu, in pixels.
		float m_CursorAngle; //!< Position of the cursor on the circle, in radians, counterclockwise from straight out to the right.
		
		BITMAP *m_BGBitmap; //!< The intermediary bitmap used to first draw the menu background, which will be blitted to the final draw target surface.
		bool m_BGBitmapNeedsRedrawing; //!< Whether the BG bitmap should be redrawn during the next Update call.

#pragma region Update Breakdown
		/// <summary>
		/// Handles the wobbling portion of Update.
		/// </summary>
		void UpdateWobbling();

		/// <summary>
		/// Handles the enabling and disabling part of Update.
		/// </summary>
		void UpdateEnablingAndDisablingProgress();

		/// <summary>
		/// Handles the mouse input when updating.
		/// </summary>
		/// <returns>Whether or not enough input was received to do something.</returns>
		bool HandleMouseInput();

		/// <summary>
		/// Handles the keyboard or controller input when updating.
		/// </summary>
		/// <returns>Whether or not enough input was received to do something.</returns>
		bool HandleNonMouseInput();

		/// <summary>
		/// Handles the slice activation part of Update.
		/// </summary>
		void UpdateSliceActivation();

		/// <summary>
		/// Redraws the predrawn background bitmap so it's up-to-date.
		/// </summary>
		void UpdatePredrawnMenuBackgroundBitmap();
#pragma endregion

		/// <summary>
		/// Sets a slice to be selected.
		/// </summary>
		/// <param name="pieSliceToSelect">The slice to be selected. Has to be a slice currently in this menu.</param>
		/// <param name="moveCursorToSlice">Whether to also move the cursor to the center of the newly selected slice. Defaults to false.</param>
		/// <returns>Whether or not this resulted in a different slice being selected. Also returns false if a null slice was passed in.</returns>
		bool SelectPieSlice(const PieSlice *pieSliceToSelect, bool moveCursorToSlice = false);

#pragma region Draw Breakdown
		/// <summary>
		/// Handles figuring out the position to draw the menu at, accounting for any Scene seams.
		/// </summary>
		/// <param name="targetBitmap">A pointer to the BITMAP to draw on. Generally a screen BITMAP.</param>
		/// <param name="targetPos">The absolute position of the target bitmap's upper left corner in the scene.</param>
		/// <param name="drawPos">Out param, a Vector to be filled in with the position at which the menu should be drawn.</param>
		void CalculateDrawPositionAccountingForSeamsAndFont(const BITMAP *targetBitmap, const Vector &targetPos, Vector &drawPos) const;

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
		void DrawPieCursorAndSliceDescriptions(BITMAP *targetBitmap, const Vector &drawPos) const;
#pragma endregion

		/// <summary>
		/// Clears all the member variables of this PieMenuGUI, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();

		// Disallow the use of some implicit methods.
		PieMenuGUI(const PieMenuGUI &reference) = delete;
		PieMenuGUI & operator=(const PieMenuGUI & rhs) = delete;
	};
}
#endif
