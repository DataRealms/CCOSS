#ifndef _RTEPIEMENU_
#define _RTEPIEMENU_

#include "PieQuadrant.h"
#include "Controller.h"
#include "Matrix.h"

namespace RTE {

	class Controller;
	class MovableObject;
	class GUIFont;
	class Actor;

	/// <summary>
	/// A PieMenu for managing interactions with objects and Actors.
	/// </summary>
	class PieMenu : public Entity {

		friend class PieSlice;

	public:

		EntityAllocation(PieMenu)
		SerializableOverrideMethods
		ClassInfoGetters

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a PieMenu object in system memory. Create() should be called before using the object.
		/// </summary>
		PieMenu() { Clear(); }

		/// <summary>
		/// Makes the PieMenu object ready for use.
		/// </summary>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create() override;

		/// <summary>
		/// Makes the PieMenu object ready for use.
		/// </summary>
		/// <param name="owner">The Actor which should act as the owner for this PieMenu.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create(Actor *owner) { SetOwner(owner); return Create(); }

		/// <summary>
		/// Creates a PieMenu to be identical to another, by deep copy.
		/// </summary>
		/// <param name="reference">A reference to the Attachable to deep copy.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create(const PieMenu &reference);
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Destructor method used to clean up a PieMenu object before deletion from system memory.
		/// </summary>
		~PieMenu() override { Destroy(true); }

		/// <summary>
		/// Destroys and resets (through Clear()) the PieMenu object.
		/// </summary>
		/// <param name="notInherited">Whether to only destroy the members defined in this derived class, or to destroy all inherited members also.</param>
		void Destroy(bool notInherited = false) override;

		/// <summary>
		/// Resets the entire PieMenu, including its inherited members, to their default settings or values.
		/// </summary>
		void Reset() override { Clear(); Entity::Reset(); }
#pragma endregion

#pragma region Getters and Setters
		/// <summary>
		/// Gets the owner Actor of this PieMenu. Ownership is NOT transferred!
		/// </summary>
		/// <returns>The owner Actor of this PieMenu. Ownership is NOT transferred!</returns>
		const Actor * GetOwner() const { return m_Owner; }

		/// <summary>
		/// Sets the owner Actor of this PieMenu, ensuring this is that Actor's PieMenu, and updates PieSlice sources accordingly. Ownership is NOT transferred!
		/// </summary>
		/// <param name="newOwner">The new owner Actor for this PieMenu. Ownership is NOT transferred!</param>
		void SetOwner(Actor *newOwner);

		/// <summary>
		/// Gets the currently in-use Controller for this PieMenu - either the menu Controller if there's one set, or the owning Actor's Controller. Ownership IS NOT transferred!
		/// </summary>
		/// <returns>The currently in-use Controller for this PieMenu.</returns>
		Controller * GetController() const;

		/// <summary>
		/// Sets the menu Controller used by this PieMenu, separate from any Controller on its owner Actor. Ownership is NOT transferred!
		/// </summary>
		/// <param name="menuController">The new Controller for this PieMenu. Ownership is NOT transferred!</param>
		void SetMenuController(Controller *menuController) { m_MenuController = menuController; }

		/// <summary>
		/// Gets the currently affected MovableObject. Ownership is NOT transferred!
		/// </summary>
		/// <returns>The MovableObject this PieMenu affects. Ownership is NOT transferred!</returns>
		const MovableObject * GetAffectedObject() const { return m_AffectedObject; }

		/// <summary>
		/// Sets the MovableObject this PieMenu should affect. Ownership is NOT transferred!
		/// </summary>
		/// <param name="affectedObject">The new MovableObject affected by this PieMenu. Ownership is NOT transferred!</param>
		void SetAffectedObject(MovableObject *affectedObject) { m_AffectedObject = affectedObject; }

		/// <summary>
		/// Gets whether this PieMenu is a sub-PieMenu, i.e. it's owned by a PieSlice.
		/// </summary>
		/// <returns>Whether or not this PieMenu is a sub-PieMenu.</returns>
		bool IsSubPieMenu() const { return m_DirectionIfSubPieMenu != Directions::None; }

		/// <summary>
		/// Gets the absolute center position of this PieMenu.
		/// </summary>
		/// <returns>A Vector describing the current absolute position of this PieMenu in pixels.</returns>
		const Vector & GetPos() const { return m_CenterPos; }

		/// <summary>
		/// Sets the absolute center position of this PieMenu in the scene.
		/// </summary>
		/// <param name="newPos">A Vector describing the new absolute position of this PieMenu in pixels, in the scene.</param>
		void SetPos(const Vector &newPos);

		/// <summary>
		/// Gets the absolute rotation of this PieMenu.
		/// </summary>
		/// <returns>A Matrix describing the current absolute rotation of this PieMenu.</returns>
		const Matrix & GetRotation() const { return m_Rotation; }

		/// <summary>
		/// Gets the absolute rotation of this PieMenu in radians.
		/// </summary>
		/// <returns>The absolute rotation of this PieMenu in radians.</returns>
		float GetRotAngle() const { return m_Rotation.GetRadAngle(); }

		/// <summary>
		/// Sets the absolute rotation of this PieMenu.
		/// </summary>
		/// <param name="newRotation">A Matrix describing the new rotation of this PieMenu.</param>
		void SetRotation(const Matrix &newRotation) { m_Rotation = newRotation; }

		/// <summary>
		/// Sets the absolute rotation of this PieMenu to the specified rad angle.
		/// </summary>
		/// <param name="rotAngle">The angle in radians describing the new rotation of this PieMenu.</param>
		void SetRotAngle(float rotAngle) { m_Rotation.SetRadAngle(rotAngle); }

		/// <summary>
		/// Gets the full inner radius of this PieMenu.
		/// </summary>
		/// <returns>Gets the full inner radius of this PieMenu.</returns>
		int GetFullInnerRadius() const { return m_FullInnerRadius; }

		/// <summary>
		/// Sets the full inner radius of this PieMenu and recreates the background bitmap if it's changed.
		/// </summary>
		/// <param name="fullInnerRadius">The new full inner radius of this PieMenu.</param>
		void SetFullInnerRadius(int fullInnerRadius) { if (m_FullInnerRadius != fullInnerRadius) { m_FullInnerRadius = fullInnerRadius; RecreateBackgroundBitmaps(); } }

		/// <summary>
		/// Gets whether or not the PieMenu is enabled or in the process of being enabled, and is not in wobble mode.
		/// </summary>
		/// <returns>Whether or not the PieMenu is enabled or in the process of being enabled.</returns>
		bool IsEnabled() const { return (m_EnabledState == EnabledState::Enabled || m_EnabledState == EnabledState::Enabling) && m_MenuMode != MenuMode::Wobble; }

		/// <summary>
		/// Gets whether or not the PieMenu is in the process of being enabled.
		/// </summary>
		/// <returns>Whether or not the PieMenu is in the process of being enabled.</returns>
		bool IsEnabling() const { return m_EnabledState == EnabledState::Enabling; }

		/// <summary>
		/// Gets whether or not the PieMenu is in the process of being disabled.
		/// </summary>
		/// <returns>Whether or not the PieMenu is in the process of being disabled.</returns>
		bool IsDisabling() const { return m_EnabledState == EnabledState::Enabling && m_MenuMode != MenuMode::Wobble; }

		/// <summary>
		/// Gets whether or not the PieMenu is in the process of enabling or disabling.
		/// </summary>
		/// <returns>Whether or not the PieMenu is in the process of enabling or disabling.</returns>
		bool IsEnablingOrDisabling() const { return (m_EnabledState == EnabledState::Enabling || m_EnabledState == EnabledState::Disabling) && m_MenuMode != MenuMode::Wobble; }

		/// <summary>
		/// Gets whether or not the PieMenu is at all visible.
		/// </summary>
		/// <returns>Whether the PieMenu is visible.</returns>
		bool IsVisible() const { return m_EnabledState != EnabledState::Disabled || m_MenuMode == MenuMode::Freeze || m_MenuMode == MenuMode::Wobble; }

		/// <summary>
		/// Gets whether or not the PieMenu is in the normal animation mode.
		/// </summary>
		/// <returns>Whether or not the PieMenu is in the normal animation mode.</returns>
		bool IsInNormalAnimationMode() const { return m_MenuMode == MenuMode::Normal; }

		/// <summary>
		/// Enables or disables the PieMenu and animates it in and out of view.
		/// </summary>
		/// <param name="enable">Whether to enable or disable the PieMenu.</param>
		/// <param name="playSounds">Whether or not to play appropriate sounds when the menu is enabled or disabled.</param>
		void SetEnabled(bool enable, bool playSounds = true);

		/// <summary>
		/// Gets whether this PieMenu has an open sub-PieMenu.
		/// </summary>
		/// <returns>Whether or not this PieMenu has an open sub-PieMenu.</returns>
		bool HasSubPieMenuOpen() const { return m_ActiveSubPieMenu != nullptr; }
#pragma endregion

#pragma region Special Animation Handling
		/// <summary>
		/// Sets this PieMenu to normal MenuMode.
		/// </summary>
		void SetAnimationModeToNormal() { if (m_MenuMode != MenuMode::Normal) { m_MenuMode = MenuMode::Normal; m_EnabledState = EnabledState::Disabled; } }

		/// <summary>
		/// Plays the disabling animation, regardless of whether the PieMenu was enabled or not.
		/// </summary>
		void DoDisableAnimation() { m_CurrentInnerRadius = m_FullInnerRadius; m_MenuMode = MenuMode::Normal; m_EnableDisableAnimationTimer.Reset(); m_EnabledState = EnabledState::Disabling; }

		/// <summary>
		/// Plays an animation of the background circle expanding and contracting continuously. The PieMenu is effectively disabled while doing this.
		/// This animation will continue until the next call to SetEnabled.
		/// </summary>
		void Wobble() { m_MenuMode = MenuMode::Wobble; }

		/// <summary>
		/// Makes the background circle freeze at a certain radius until SetEnabled is called. The PieMenu is effectively disabled while doing this.
		/// </summary>
		/// <param name="radius">The radius to make the background circle freeze at.</param>
		void FreezeAtRadius(int radius) { m_MenuMode = MenuMode::Freeze; m_CurrentInnerRadius = radius; m_BGBitmapNeedsRedrawing = true; }
#pragma endregion

#pragma region PieSlice Handling
		/// <summary>
		/// Gets the activated PieSlice for this PieMenu in the last update. If there is a sub-PieMenu open for this PieMenu, it gets that activated PieSlice instead.
		/// </summary>
		/// <returns>The activated PieSlice for this PieMenu.</returns>
		const PieSlice * GetActivatedPieSlice() const;

		/// <summary>
		/// Gets the command issued by this PieMenu in the last update, i.e. the PieSlice SliceType of the currently activated PieSlice, or None if no slice was activated.
		/// </summary>
		/// <returns>The PieSlice type which has been picked, or None if none has been picked.</returns>
		PieSlice::SliceType GetPieCommand() const;

		/// <summary>
		/// Gets a const reference to the vector containing pointers to all the PieSlices in this PieMenu.
		/// </summary>
		/// <returns>A const reference to the vector containing pointers to all the PieSlices in this PieMenu.</returns>
		const std::vector<PieSlice *> & GetPieSlices() const { return m_CurrentPieSlices; }

		/// <summary>
		/// Gets the first found PieSlice with the passed in preset name, if there is one. Ownership is NOT transferred!
		/// </summary>
		/// <param name="presetName">The preset name to look for.</param>
		/// <returns>The first found PieSlice with the passed in preset name, or nullptr if there are no PieSlices with that preset name in this PieMenu.</returns>
		PieSlice * GetFirstPieSliceByPresetName(const std::string &presetName) const;

		/// <summary>
		/// Gets the first found PieSlice with the passed in PieSlice SliceType, if there is one. Ownership is NOT transferred!
		/// </summary>
		/// <param name="pieSliceType">The type of PieSlice to look for.</param>
		/// <returns>The first found PieSlice with the passed in PieSlice SliceType, or nullptr if there are no PieSlices with that SliceType in this PieMenu.</returns>
		PieSlice * GetFirstPieSliceByType(PieSlice::SliceType pieSliceType) const;

		/// <summary>
		/// Adds a PieSlice to the PieMenu, setting its original source to the specified sliceSource. Ownership IS transferred!
		/// The slice will be placed in the appropriate PieQuadrant for its Direction, with Any Direction using the first available PieQuadrant.
		/// If allowQuadrantOverflow is true, the PieSlice will be added to the next available PieQuadrant in Direction order.
		/// Note that if the slice could not be added, it will be deleted to avoid memory leaks, and the method will return false.
		/// </summary>
		/// <param name="pieSliceToAdd">The new PieSlice to add. Ownership IS transferred.</param>
		/// <param name="pieSliceOriginalSource">The source of the added PieSlice. Should be nullptr for slices not added by Entities.</param>
		/// <param name="allowQuadrantOverflow">Whether the new PieSlice can be placed in PieQuadrants other than the one specified by its Direction, if that PieQuadrant is full.</param>
		/// <returns>Whether or not the PieSlice was added successfully.</returns>
		bool AddPieSlice(PieSlice *pieSliceToAdd, const Entity *pieSliceOriginalSource, bool allowQuadrantOverflow = false);

		/// <summary>
		/// Adds a PieSlice to the PieMenu, with the same conditions as AddPieSlice above, but only if no PieSlice exists in this PieMenu with the same PresetName (optionally with the same original source). Ownership IS transferred!
		/// </summary>
		/// <param name="pieSliceToAdd">The new PieSlice to add. Ownership IS transferred.</param>
		/// <param name="pieSliceOriginalSource">The source of the added PieSlice. Should be nullptr for slices not added by Entities.</param>
		/// <param name="onlyCheckPieSlicesWithSameOriginalSource">Whether all PieSlices in the PieMenu should be checked to see if there are no duplicates, or only those with the same original source.</param>
		/// <param name="allowQuadrantOverflow">Whether the new PieSlice can be placed in PieQuadrants other than the one specified by its Direction, if that PieQuadrant is full.</param>
		/// <returns>Whether or not the PieSlice was added successfully.</returns>
		bool AddPieSliceIfPresetNameIsUnique(PieSlice *pieSliceToAdd, const Entity *pieSliceOriginalSource, bool onlyCheckPieSlicesWithSameOriginalSource = false, bool allowQuadrantOverflow = false);

		/// <summary>
		/// Removes and returns the passed in PieSlice from this PieMenu if it's in the PieMenu. Ownership IS transferred to the caller!
		/// </summary>
		/// <param name="pieSliceToRemove">The PieSlice to remove from this PieMenu. Ownership IS transferred to the caller!</param>
		/// <returns>The removed PieSlice, if it was in the PieMenu.</returns>
		PieSlice * RemovePieSlice(const PieSlice *pieSliceToRemove);

		/// <summary>
		/// Removes any PieSlices in this PieMenu whose preset name matches the passed in preset name.
		/// </summary>
		/// <param name="presetNameToRemoveBy">The preset name to check against.</param>
		/// <returns>Whether or not any PieSlices were removed from this PieMenu.</returns>
		bool RemovePieSlicesByPresetName(const std::string &presetNameToRemoveBy);

		/// <summary>
		/// Removes any PieSlices in this PieMenu whose PieSlice SliceType matches the passed in PieSlice SliceType.
		/// </summary>
		/// <param name="pieSliceTypeToRemoveBy">The PieSlice SliceType to check against.</param>
		/// <returns>Whether or not any PieSlices were removed from this PieMenu.</returns>
		bool RemovePieSlicesByType(PieSlice::SliceType pieSliceTypeToRemoveBy);

		/// <summary>
		/// Removes any PieSlices in this PieMenu whose original source matches the passed in Entity.
		/// </summary>
		/// <param name="originalSource">The original source whose PieSlices should be removed.</param>
		/// <returns>Whether or not any PieSlices were removed from this PieMenu.</returns>
		bool RemovePieSlicesByOriginalSource(const Entity *originalSource);

		/// <summary>
		/// Replaces the first PieSlice with the second, ensuring original source, direction, middle slice eligibility, angles and slot count are maintained.
		/// The existing PieSlice is returned, and ownership IS transferred both ways!
		/// </summary>
		/// <param name="pieSliceToReplace">The PieSlice that will be replaced.</param>
		/// <param name="replacementPieSlice">The PieSlice that will replace the existing one. If this is nullptr, the existing one will just be removed.</param>
		/// <returns>The removed PieSlice, if there is one. Ownership IS transferred!</returns>
		PieSlice * ReplacePieSlice(const PieSlice *pieSliceToReplace, PieSlice *replacementPieSlice);
#pragma endregion

#pragma region Updating
		/// <summary>
		/// Updates the state of this PieMenu each frame.
		/// </summary>
		void Update();

		/// <summary>
		/// Draws the PieMenu.
		/// </summary>
		/// <param name="targetBitmap">A pointer to a BITMAP to draw on. Generally a screen BITMAP.</param>
		/// <param name="targetPos">The absolute position of the target bitmap's upper left corner in the scene.</param>
		void Draw(BITMAP *targetBitmap, const Vector &targetPos = Vector()) const;
#pragma endregion

#pragma region Event Handling
		/// <summary>
		/// Add the passed in MovableObject and function as a listener to be run while this PieMenu is open.
		/// </summary>
		/// <param name="listeningObject">The MovableObject listening.</param>
		/// <param name="listenerFunction">The function to be run on the MovableObject.</param>
		void AddWhilePieMenuOpenListener(const MovableObject *listeningObject, const std::function<void()> &listenerFunction) { if (listeningObject) { m_WhilePieMenuOpenListeners.try_emplace(listeningObject, listenerFunction); } }

		/// <summary>
		/// Removes the passed in MovableObject and its listening function as a listener for when this PieMenu is opened.
		/// </summary>
		/// <param name="objectToRemove">The MovableObject whose listening function should be removed.</param>
		/// <returns>Whether or not the MovableObject was found and removed as a listener.</returns>
		bool RemoveWhilePieMenuOpenListener(const MovableObject *objectToRemove) { return m_WhilePieMenuOpenListeners.erase(objectToRemove) == 1; }
#pragma endregion

	private:

		/// <summary>
		/// Enumeration for enabled states when enabling/disabling the PieMenu.
		/// </summary>
		enum class EnabledState { Enabling, Enabled, Disabling, Disabled };

		/// <summary>
		/// Enumeration for the modes a PieMenu can have.
		/// </summary>
		enum class MenuMode { Normal, Wobble, Freeze };

		/// <summary>
		/// Enumeration for the different item separator modes available to the PieMenu.
		/// </summary>
		enum class IconSeparatorMode { Line, Circle, Square };

		/// <summary>
		/// Enumeration for helping keyboard PieMenu navigation. Specifies the ways the cursor should move from one PieQuadrant to another.
		/// </summary>
		enum class MoveToPieQuadrantMode { Start, Middle, End };

		static constexpr int c_EnablingDelay = 50; //!< Time in ms for how long it takes to enable/disable.
		static constexpr int c_DefaultFullRadius = 58; //!< The radius the menu should have when fully enabled, in pixels.
		static constexpr int c_PieSliceWithSubPieMenuExtraThickness = 3; //!< The extra thickness to be added to PieSlices with a sub-PieMenu.

		static const std::unordered_map<std::string, IconSeparatorMode> c_IconSeparatorModeMap; //!< A map of strings to IconSeparatorModes to support string parsing for the IconSeparatorMode enum.
		static const std::unordered_map<ControlState, Directions> c_ControlStateDirections; //!< A map of ControlStates to Directions, for translating between the two.
		static const std::unordered_map<Directions, Directions> c_OppositeDirections; //!< A map of Directions to their opposite Directions, for zooming between PieQuadrants with the keyboard.
		static const std::unordered_map<Directions, Directions> c_CounterClockwiseDirections; //!< A map of Directions to the Direction that is CCW from them, for moving between PieQuadrants with the keyboard.

		static Entity::ClassInfo m_sClass; //!< ClassInfo for this class.
		static BITMAP *s_CursorBitmap; //!< A static pointer to the bitmap to use as the cursor in any menu.

		GUIFont *m_LargeFont; //!< A pointer to the large font from FrameMan. Not owned here.

		Actor *m_Owner; //!< The owner Actor of this PieMenu. Note that PieMenus do not necessarily need to have a owner.
		Controller *m_MenuController; //!< The Controller which controls this PieMenu. Separate from the Controller of the owner or affected object (if there is one).
		MovableObject *m_AffectedObject; //!< The MovableObject this PieMenu affects, if any. Only applies if there's no owner.
		Directions m_DirectionIfSubPieMenu; //!< The direction this sub-PieMenu is facing in. None if this is not a sub-PieMenu of another PieMenu.
		MenuMode m_MenuMode; //!< The mode this PieMenu is in. See MenuMode enum for more details.
		Vector m_CenterPos; //!< The center position of this PieMenu in the scene.
		Matrix m_Rotation; //!< The rotation of this PieMenu. Generally 0 for top level PieMenus.

		EnabledState m_EnabledState; //!< The enabled state of the PieMenu.
		Timer m_EnableDisableAnimationTimer; //!< Timer for progressing enabling/disabling animations.
		Timer m_HoverTimer; //!< Timer to measure how long to hold a hovered over slice.
		Timer m_SubPieMenuHoverOpenTimer; //!< Timer for opening sub-PieMenus when their owning slices are hovered over.

		IconSeparatorMode m_IconSeparatorMode; //!< The icon separator mode of this PieMenu.
		int m_FullInnerRadius; //!< The full inner radius of the PieMenu's background, in pixels.
		int m_BackgroundThickness; //!< The thickness of the PieMenu's background, in pixels.
		int m_BackgroundSeparatorSize; //!< The size of the PieMenu's background separators, in pixels. Used differently based on the menu's IconSeparatorMode.
		bool m_DrawBackgroundTransparent; //!< Whether or not the PieMenu's background should be drawn transparently.
		int m_BackgroundColor; //!< The color used for drawing the PieMenu's background.
		int m_BackgroundBorderColor; //!< The color used for drawing borders for the PieMenu's background.
		int m_SelectedItemBackgroundColor; //!< The color used for drawing selected PieMenu items' backgrounds.

		std::array<PieQuadrant, 4> m_PieQuadrants; //!< The array of PieQuadrants that make up this PieMenu. Quadrants may be individually enabled or disabled, affecting what's drawn.
		const PieSlice *m_HoveredPieSlice; //!< The PieSlice currently being hovered over.
		const PieSlice *m_ActivatedPieSlice; //!< The currently activated PieSlice, if there is one, or 0 if there's not.
		const PieSlice *m_AlreadyActivatedPieSlice; //!< The PieSlice that was most recently activated by pressing primary. Used to avoid duplicate activation when disabling.
		std::vector<PieSlice *> m_CurrentPieSlices; //!< All the PieSlices in this PieMenu in INI order. Not owned here, just pointing to the ones above.

		PieMenu *m_ActiveSubPieMenu; //!< The currently active sub-PieMenu, if any.

		std::unordered_map<const MovableObject *, std::function<void()>> m_WhilePieMenuOpenListeners; //!< Unordered map of MovableObject pointers to functions to be called while the PieMenu is open. Pointers are NOT owned.

		int m_CurrentInnerRadius; //!< The current radius of the innermost circle of the pie menu, in pixels.
		bool m_CursorInVisiblePosition; //!< Whether or not this PieMenu's cursor is in a visible position and should be shown.
		float m_CursorAngle; //!< Position of the cursor on the circle, in radians, counterclockwise from straight out to the right.

		BITMAP *m_BGBitmap; //!< The intermediary bitmap used to first draw the PieMenu background, which will be blitted to the final draw target surface.
		BITMAP *m_BGRotationBitmap; //!< The intermediary bitmap used to allow the PieMenu background to rotate, which will be pivoted onto the BG bitmap.
		BITMAP *m_BGPieSlicesWithSubPieMenuBitmap; //!< The intermediary bitmap used to support handling PieSlices with sub-PieMenus, which will be drawn onto the BG bitmap.
		bool m_BGBitmapNeedsRedrawing; //!< Whether the BG bitmap should be redrawn during the next Update call.
		bool m_BGPieSlicesWithSubPieMenuBitmapNeedsRedrawing; //!< Whether the BG bitmap for PieSlices with sub-PieMenus should be redrawn when the BGBitmap is redrawn.

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
		/// Handles the analog input when updating.
		/// </summary>
		/// <returns>Whether or not enough input was received to do something.</returns>
		bool HandleAnalogInput();

		/// <summary>
		/// Handles the digital input when updating.
		/// </summary>
		/// <returns>Whether or not enough input was received to do something.</returns>
		bool HandleDigitalInput();

		/// <summary>
		/// Handles the slice activation part of Update.
		/// </summary>
		void UpdateSliceActivation();

		/// <summary>
		/// Redraws the pre-drawn background bitmap so it's up-to-date.
		/// </summary>
		void UpdatePredrawnMenuBackgroundBitmap();
#pragma endregion

#pragma region Draw Breakdown
		/// <summary>
		/// Handles figuring out the position to draw the PieMenu at, accounting for any Scene seams.
		/// </summary>
		/// <param name="targetBitmap">A pointer to the BITMAP to draw on. Generally a screen BITMAP.</param>
		/// <param name="targetPos">The absolute position of the target bitmap's upper left corner in the scene.</param>
		/// <param name="drawPos">Out parameter, a Vector to be filled in with the position at which the PieMenu should be drawn.</param>
		void CalculateDrawPosition(const BITMAP *targetBitmap, const Vector &targetPos, Vector &drawPos) const;

		/// <summary>
		/// Handles drawing icons for PieSlices' visual representation in the PieMenu.
		/// </summary>
		/// <param name="targetBitmap">A pointer to the BITMAP to draw on. Generally a screen BITMAP.</param>
		/// <param name="drawPos">The seam corrected position at which the PieMenu is being drawn.</param>
		void DrawPieIcons(BITMAP *targetBitmap, const Vector &drawPos) const;

		/// <summary>
		/// Handles drawing the cursor and description text for selected PieSlices.
		/// </summary>
		/// <param name="targetBitmap">A pointer to the BITMAP to draw on. Generally a screen BITMAP.</param>
		/// <param name="drawPos">The seam corrected position at which the PieMenu is being drawn.</param>
		void DrawPieCursorAndPieSliceDescriptions(BITMAP *targetBitmap, const Vector &drawPos) const;
#pragma endregion

		/// <summary>
		/// Clears and refills the vector of current PieSlices for this PieMenu. Also realigns PieSlices and expands them into empty space if possible, to ensure everything is properly ready.
		/// </summary>
		void RepopulateAndRealignCurrentPieSlices();

		/// <summary>
		/// Expands any PieSlices that border onto another PieQuadrant's unfilled slot, so they visually occupy that empty slot.
		/// </summary>
		void ExpandPieSliceIntoEmptySpaceIfPossible();

		/// <summary>
		/// Recreates this PieMenu's background bitmap based on its full inner radius.
		/// </summary>
		void RecreateBackgroundBitmaps();

		/// <summary>
		/// Draws the background separators for this PieMenu, based on its IconSeparatorMode, onto the passed in bitmap.
		/// </summary>
		/// <param name="backgroundBitmapToDrawTo">The bitmap to draw the separators onto.</param>
		/// <param name="pieCircleCenterX">The center X position of the circle being separated.</param>
		/// <param name="pieCircleCenterY">The center Y position of the circle being separated.</param>
		/// <param name="subPieMenuRotationOffset">The rotation offset used if this is a sub-PieMenu.</param>
		void DrawBackgroundPieSliceSeparators(BITMAP *backgroundBitmapToDrawTo, int pieCircleCenterX, int pieCircleCenterY, float subPieMenuRotationOffset) const;

		/// <summary>
		/// Draws a background separator, based on this PieMenu's IconSeparatorMode, to the passed in bitmap.
		/// </summary>
		/// <param name="backgroundBitmapToDrawTo">The bitmap to draw the separator onto.</param>
		/// <param name="pieCircleCenterX">The center X position of the circle the separator is drawn onto.</param>
		/// <param name="pieCircleCenterY">The center Y position of the circle the separator is drawn onto.</param>
		/// <param name="rotAngle">The rotation of the separator, not used for all separator types.</param>
		/// <param name="isHoveredPieSlice">Whether the separator is being drawn for the PieMenu's hovered PieSlice.</param>
		/// <param name="pieSliceHasSubPieMenu">Whether the PieSlice whose separator is being drawn has a sub-PieMenu.</param>
		/// <param name="drawHalfSizedSeparator">Whether to draw a half-sized separator or a full-sized one. Defaults to drawing a full-sized one.</param>
		void DrawBackgroundPieSliceSeparator(BITMAP *backgroundBitmapToDrawTo, int pieCircleCenterX, int pieCircleCenterY, float rotAngle, bool isHoveredPieSlice, bool pieSliceHasSubPieMenu, bool drawHalfSizedSeparator = false) const;

		/// <summary>
		/// Sets the passed in PieSlice as the hovered PieSlice of this PieMenu. If nullptr is passed in, no PieSlice will be hovered.
		/// </summary>
		/// <param name="pieSliceToSetAsHovered">The PieSlice to consider hovered, if any. Has to be a PieSlice currently in this PieMenu.</param>
		/// <param name="moveCursorIconToSlice">Whether to also move the cursor icon to the center of the new hovered PieSlice and set it to be drawn (generally for non-mouse inputs). Defaults to false.</param>
		/// <returns>Whether or not the PieSlice to set as hovered was different from the already hovered PieSlice.</returns>
		bool SetHoveredPieSlice(const PieSlice *pieSliceToSetAsHovered, bool moveCursorToPieSlice = false);

		/// <summary>
		/// Prepares the passed in PieSlice's sub-PieMenu for use by setting flags, moving PieSlices around, and disabling PieQuadrants as appropriate.
		/// </summary>
		/// <param name="pieSliceWithSubPieMenu">The PieSlice with a sub-PieMenu that needs to be prepared.</param>
		/// <returns>Whether the sub-PieMenu was prepared. PieMenus with the SubPieMenu flag already set will not have action taken on them.</returns>
		bool PreparePieSliceSubPieMenuForUse(const PieSlice *pieSliceWithSubPieMenu) const;

		/// <summary>
		/// If the Controller for this PieMenu is mouse or gamepad controlled, sets up analog cursor angle limits and positions for when the pie menu is enabled or disabled. Also used when a sub-PieMenu of this PieMenu is disabled.
		/// </summary>
		/// <param name="enable">Whether the PieMenu is being enabled or disabled.</param>
		void PrepareAnalogCursorForEnableOrDisable(bool enable) const;

		/// <summary>
		/// Clears all the member variables of this PieMenu, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();

		// Disallow the use of some implicit methods.
		PieMenu(const PieMenu &reference) = delete;
		PieMenu & operator=(const PieMenu &rhs) = delete;
	};
}
#endif
