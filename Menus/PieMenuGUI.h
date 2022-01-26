#ifndef _PIEMENUGUI_
#define _PIEMENUGUI_

#include "PieSlice.h"
#include "Timer.h"
#include "Vector.h"
#include "Matrix.h"

namespace RTE {

	class Controller;
	class MovableObject;
	class GUIFont;
	class Actor;

	/// <summary>
	/// A GUI for displaying pie menus.
	/// </summary>
	class PieMenuGUI : public Entity {
		
		friend class PieSlice;

	public:

		EntityAllocation(PieMenuGUI)
		SerializableOverrideMethods
		ClassInfoGetters

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a PieMenuGUI object in system memory. Create() should be called before using the object.
		/// </summary>
		PieMenuGUI() { Clear(); }

		/// <summary>
		/// Makes the PieMenuGUI object ready for use.
		/// </summary>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create() override;

		/// <summary>
		/// Makes the PieMenuGUI object ready for use.
		/// </summary>
		/// <param name="owner">The Actor which should act as the owner for this PieMenuGUI.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create(Actor *owner) { SetOwner(owner); return Create(); }

		/// <summary>
		/// Creates a PieMenuGUI to be identical to another, by deep copy.
		/// </summary>
		/// <param name="reference">A reference to the Attachable to deep copy.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create(const PieMenuGUI &reference);
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Destructor method used to clean up a PieMenuGUI object before deletion from system memory.
		/// </summary>
		~PieMenuGUI() override { Destroy(true); }

		/// <summary>
		/// Destroys and resets (through Clear()) the PieMenuGUI object.
		/// </summary>
		/// <param name="notInherited">Whether to only destroy the members defined in this derived class, or to destroy all inherited members also.</param>
		void Destroy(bool notInherited = false) override;

		/// <summary>
		/// Resets the entire PieMenuGUI, including its inherited members, to their default settings or values.
		/// </summary>
		void Reset() override { Clear(); Entity::Reset(); }

		/// <summary>
		/// Resets and removes all PieSlices from the PieMenuGUI so that new ones can be added.
		/// </summary>
		void ResetSlices();
#pragma endregion

#pragma region Getters and Setters
		/// <summary>
		/// Sets the owner Actor of this PieMenuGUI, ensuring this is that Actor's PieMenuGUI, and updates PieSlice sources accordingly. Ownership is NOT transferred!
		/// </summary>
		/// <param name="newOwner">The new owner Actor for this PieMenuGUI. Ownership is NOT transferred!</param>
		void SetOwner(Actor *newOwner);

		/// <summary>
		/// Gets the currently in-use Controller for this PieMenuGUI - either the menu Controller if there's one set, or the owning Actor's Controller. Ownership IS NOT transferred!
		/// </summary>
		/// <returns>The currently in-use Controller for this PieMenuGUI.</returns>
		Controller * GetController();

		/// <summary>
		/// Sets the menu Controller used by this PieMenuGUI, separate from any Controller on its owner Actor. Ownership is NOT transferred!
		/// </summary>
		/// <param name="menuController">The new Controller for this PieMenuGUI. Ownership is NOT transferred!</param>
		void SetMenuController(Controller *menuController) { m_MenuController = menuController; }

		/// <summary>
		/// Gets the currently affected MovableObject. Ownership is NOT transferred!
		/// </summary>
		/// <returns>The MovableObject this PieMenuGUI affects. Ownership is NOT transferred!</returns>
		const MovableObject * GetAffectedObject() const { return m_AffectedObject; }

		/// <summary>
		/// Sets the MovableObject this PieMenuGUI should affect. Ownership is NOT transferred!
		/// </summary>
		/// <param name="affectedObject">The new MovableObject affected by this PieMenuGUI. Ownership is NOT transferred!</param>
		void SetAffectedObject(MovableObject *affectedObject) { m_AffectedObject = affectedObject; }

		/// <summary>
		/// Gets the absolute center position of this PieMenuGUI.
		/// </summary>
		/// <returns>A Vector describing the current absolute position of this PieMenuGUI in pixels.</returns>
		const Vector & GetPos() const { return m_CenterPos; }

		/// <summary>
		/// Sets the absolute center position of this PieMenuGUI in the scene.
		/// </summary>
		/// <param name="newPos">A Vector describing the new absolute position of this PieMenuGUI in pixels, in the scene.</param>
		void SetPos(const Vector &newPos) { m_CenterPos = newPos; }

		/// <summary>
		/// Gets the absolute rotation of this PieMenuGUI.
		/// </summary>
		/// <returns>A Matrix describing the current absolute rotation of this PieMenuGUI.</returns>
		const Matrix & GetRotation() const { return m_Rotation; }

		/// <summary>
		/// Gets the absolute rotation of this PieMenuGUI in radians.
		/// </summary>
		/// <returns>The absolute rotation of this PieMenuGUI in radians.</returns>
		float GetRotAngle() const { return m_Rotation.GetRadAngle(); }

		/// <summary>
		/// Sets the absolute rotation of this PieMenuGUI.
		/// </summary>
		/// <param name="newRotation">A Matrix describing the new rotation of this PieMenuGUI.</param>
		void SetRotation(const Matrix &newRotation) { m_Rotation = newRotation; }

		/// <summary>
		/// Sets the absolute rotation of this PieMenuGUI to the specified rad angle.
		/// </summary>
		/// <param name="rotAngle">The angle in radians describing the new rotation of this PieMenuGUI.</param>
		void SetRotAngle(float rotAngle) { m_Rotation.SetRadAngle(rotAngle); }

		/// <summary>
		/// Gets whether or not the PieMenuGUI is enabled.
		/// </summary>
		/// <returns>Whether or not the PieMenuGUI.</returns>
		bool IsEnabled() const { return (m_EnabledState == EnabledState::Enabled || m_EnabledState == EnabledState::Enabling) && m_MenuMode != MenuMode::Wobble; }

		/// <summary>
		/// Gets whether or not the PieMenuGUI is in the process of enabling or disabling.
		/// </summary>
		/// <returns>Whether or not the PieMenuGUI is in the process of enabling or disabling.</returns>
		bool IsEnablingOrDisabling() const { return (m_EnabledState == EnabledState::Enabling || m_EnabledState == EnabledState::Disabling) && m_MenuMode != MenuMode::Wobble; }

		/// <summary>
		/// Gets whether or not the PieMenuGUI is at all visible.
		/// </summary>
		/// <returns>Whether the PieMenuGUI is visible.</returns>
		bool IsVisible() const { return m_EnabledState != EnabledState::Disabled || m_MenuMode == MenuMode::Freeze || m_MenuMode == MenuMode::Wobble; }

		/// <summary>
		/// Enables or disables the PieMenuGUI and animates it in and out of view.
		/// </summary>
		/// <param name="enable">Whether to enable or disable the PieMenuGUI.</param>
		/// <param name="playSounds">Whether or not to play appropriate sounds when the menu is enabled or disabled.</param>
		void SetEnabled(bool enable, bool playSounds = true);
#pragma endregion

#pragma region Special Animation Handling
		/// <summary>
		/// Plays the disabling animation, regardless of whether the PieMenuGUI was enabled or not.
		/// </summary>
		void DoDisableAnimation() { m_InnerRadius = c_FullRadius; m_MenuMode = MenuMode::Normal; m_EnableDisableAnimationTimer.Reset(); m_EnabledState = EnabledState::Disabling; }

		/// <summary>
		/// Sets this PieMenuGUI to normal MenuMode.
		/// </summary>
		void SetAnimationModeToNormal() { if (m_MenuMode != MenuMode::Normal) { m_MenuMode = MenuMode::Normal; m_EnabledState = EnabledState::Disabled; } }

		/// <summary>
		/// Plays an animation of the background circle expanding and contracting continuously. The PieMenuGUI is effectively disabled while doing this.
		/// This animation will continue until the next call to SetEnabled.
		/// </summary>
		void Wobble() { m_MenuMode = MenuMode::Wobble; }

		/// <summary>
		/// Makes the background circle freeze at a certain radius until SetEnabled is called. The PieMenuGUI is effectively disabled while doing this.
		/// </summary>
		/// <param name="radius">The radius to make the background circle freeze at.</param>
		void FreezeAtRadius(int radius) { m_MenuMode = MenuMode::Freeze; m_InnerRadius = radius; m_BGBitmapNeedsRedrawing = true; }
#pragma endregion

#pragma region PieSlice Handling
		/// <summary>
		/// Gets the command issued by this PieMenuGUI in the last update, i.e. the PieSlice Type of the currently activated PieSlice, or None if no slice was activated.
		/// </summary>
		/// <returns>The PieSlice type which has been picked, or None if none has been picked.</returns>
		PieSlice::PieSliceIndex GetPieCommand() const { return m_ActivatedSlice == nullptr ? PieSlice::PieSliceIndex::PSI_NONE : m_ActivatedSlice->GetType(); }

		/// <summary>
		/// Gets a const reference to the vector containing pointers to all the PieSlices in this PieMenuGUI.
		/// </summary>
		/// <returns>A const reference to the vector containing pointers to all the PieSlices in this PieMenuGUI.</returns>
		const std::vector<PieSlice *> & GetPieSlices() const { return m_CurrentSlices; }

		/// <summary>
		/// Gets the first found PieSlice with the given PieSlice Type, if there are any.
		/// </summary>
		/// <returns>The first found PieSlice with the given PieSlice Type, or nullptr if there are none with that Type in this PieMenuGUI.</returns>
		PieSlice * GetPieSliceByType(PieSlice::PieSliceIndex pieSliceType) const;
		
		/// <summary>
		/// Adds a PieSlice to the PieMenuGUI, setting its original source to the specified sliceSource. Ownership IS transferred.
		/// The slice will be placed in the appropriate PieQuadrant for its Direction, with Any Direction using the first available PieQuadrant.
		/// If allowQuadrantOverflow is true, the PieSlice will be added to the next available PieQuadrant in Direction order.
		/// Note that if the slice could not be added, it will be deleted to avoid memory leaks, and the method will return false.
		/// </summary>
		/// <param name="newPieSlice">The new PieSlice to add. Ownership IS transferred.</param>
		/// <param name="sliceSource">The source of the added PieSlice. Should be nullptr for slices not added by Entities.</param>
		/// <param name="allowQuadrantOverflow">Whether the new PieSlice can be placed in PieQuadrants other than the one specified by its Direction, if that PieQuadrant is full.</param>
		/// <returns>Whether or not the PieSlice was added successfully.</returns>
		bool AddPieSlice(PieSlice *newPieSlice, const Entity *sliceSource, bool allowQuadrantOverflow = false);

		/// <summary>
		/// Removes any PieSlices in this PieMenuGUI whose original source matches the passed in Entity.
		/// </summary>
		/// <param name="originalSource">The original source whose PieSlices should be removed.</param>
		void RemovePieSlicesByOriginalSource(const Entity *originalSource) { for (PieQuadrant &pieQuadrant : m_PieQuadrants) { pieQuadrant.RemovePieSlicesByOriginalSource(originalSource); } ReloadCurrentPieSlices(); }
#pragma endregion

#pragma region Updating
		/// <summary>
		/// Updates the state of this PieMenuGUI each frame.
		/// </summary>
		void Update();

		/// <summary>
		/// Draws the PieMenuGUI.
		/// </summary>
		/// <param name="targetBitmap">A pointer to a BITMAP to draw on. Generally a screen BITMAP.</param>
		/// <param name="targetPos">The absolute position of the target bitmap's upper left corner in the scene.</param>
		void Draw(BITMAP *targetBitmap, const Vector &targetPos = Vector()) const;
#pragma endregion

#pragma region Event Handling
		/// <summary>
		/// Add the passed in MovableObject and function as a listener for when this PieMenuGUI is opened.
		/// </summary>
		/// <param name="listeningObject">The MovableObject listening.</param>
		/// <param name="listenerFunction">The function to be run on the MovableObject.</param>
		void AddPieMenuOpenListener(const MovableObject *listeningObject, const std::function<void()> &listenerFunction) { m_OnPieMenuOpenListeners.insert({ listeningObject, listenerFunction }); }

		/// <summary>
		/// Removes the passed in MovableObject and its listening function as a listener for when this PieMenuGUI is opened.
		/// </summary>
		/// <param name="objectToRemove">The MovableObject whose listening function should be removed.</param>
		/// <returns>Whether or not the MovableObject was found and removed as a listener.</returns>
		bool RemovePieMenuOpenListener(const MovableObject *objectToRemove) { return m_OnPieMenuOpenListeners.erase(objectToRemove) == 1; }

		/// <summary>
		/// Add the passed in MovableObject and function as a listener for when this PieMenuGUI is closed.
		/// </summary>
		/// <param name="listeningObject">The MovableObject listening.</param>
		/// <param name="listenerFunction">The function to be run on the MovableObject.</param>
		void AddPieMenuCloseListener(const MovableObject *listeningObject, const std::function<void()> &listenerFunction) { m_OnPieMenuCloseListeners.insert({ listeningObject, listenerFunction }); }

		/// <summary>
		/// Removes the passed in MovableObject and its listening function as a listener for when this PieMenuGUI is closed.
		/// </summary>
		/// <param name="objectToRemove">The MovableObject whose listening function should be removed.</param>
		/// <returns>Whether or not the MovableObject was found and removed as a listener.</returns>
		bool RemovePieMenuCloseListener(const MovableObject *objectToRemove) { return m_OnPieMenuCloseListeners.erase(objectToRemove) == 1; }
#pragma endregion

	private:
		static const int c_QuadrantSlotCount = 5; //!< The maximum number of PieSlices a PieQuadrant can have.
		static constexpr float c_PieSliceSlotSize = c_HalfPI / static_cast<float>(c_QuadrantSlotCount); //!< The size of one PieSlice slot in PieQuadrants.

		struct PieQuadrant {
			bool Enabled = false; //!< Whether this PieQuadrant is enabled and visible or disabled.
			Directions Direction; //!< The direction of this PieQuadrant.

			std::unique_ptr<PieSlice> MiddlePieSlice; //!< A unique_ptr to the middle PieSlice of this PieQuadrant.
			std::array<std::unique_ptr<PieSlice>, c_QuadrantSlotCount / 2> LeftPieSlices; //!< An array of unique_ptrs to the left side PieSlices of this PieQuadrant.
			std::array<std::unique_ptr<PieSlice>, c_QuadrantSlotCount / 2> RightPieSlices; //!< An array of unique_ptrs to the right side PieSlices of this PieQuadrant.
			std::array<const PieSlice *, c_QuadrantSlotCount> SlotsForPieSlices; //!< An array representing the slots in this PieQuadrant, via pointers to the PieSlices filling each slot.

			/// <summary>
			/// Resets this PieQuadrant and deletes all slices in it.
			/// </summary>
			void Reset();

			/// <summary>
			///  Creates a PieQuadrant to be identical to another, by deep copy.
			/// </summary>
			/// <param name="reference">A reference to the PieQuadrant to deep copy.</param>
			void Create(const PieQuadrant &reference);

			/// <summary>
			/// Gets whether or not this PieQuadrant contains the given PieSlice.
			/// </summary>
			/// <returns>Whether or not this PieQuadrant contains the given PieSlice.</returns>
			bool ContainsSlice(const PieSlice *sliceToCheck) const { return sliceToCheck == MiddlePieSlice.get() || sliceToCheck == LeftPieSlices.at(0).get() || sliceToCheck == RightPieSlices.at(0).get() || sliceToCheck == LeftPieSlices.at(1).get() || sliceToCheck == RightPieSlices.at(1).get(); }

			/// <summary>
			/// Gets a vector of non-owning pointers to the PieSlices in this PieQuadrant.
			/// The vector of PieSlices will default to INI order, i.e. starting with the middle and interleaving left and right slices in order.
			/// Alternatively it can go in CCW order, getting the outermost right slice and moving inwards through the middle and then left slices.
			/// </summary>
			/// <param name="inCCWOrder">Whether to get flattened slices in counter-clockwise order. Defaults to false.</param>
			std::vector<PieSlice *> GetFlattenedSlices(bool inCCWOrder = false) const;

			/// <summary>
			/// Aligns all PieSlices in this PieQuadrant, setting their angle and size details.
			/// </summary>
			void RealignSlices();

			/// <summary>
			/// Adds the PieSlice to the quadrant. Ownership IS transferred.
			/// </summary>
			bool AddPieSlice(PieSlice *pieSliceToAdd);

			/// <summary>
			/// Removes any PieSlices in this PieQuadrant whose original source matches the passed in Entity.
			/// </summary>
			/// <param name="originalSource">The original source whose PieSlices should be removed.</param>
			void RemovePieSlicesByOriginalSource(const Entity *originalSource);
		};

		/// <summary>
		/// Enumeration for enabled states when enabling/disabling the PieMenuGUI.
		/// </summary>
		enum class EnabledState { Enabling, Enabled, Disabling, Disabled };

		/// <summary>
		/// Enumeration for the modes a PieMenuGUI can have.
		/// </summary>
		enum class MenuMode { Normal, Wobble, Freeze };

		/// <summary>
		/// Enumeration for the different item separator modes available to the PieMenuGUI.
		/// </summary>
		enum class IconSeparatorMode { Line, Circle };

		static const int c_EnablingDelay = 50; //!< Time in ms for how long it takes to enable/disable.
		static const int c_FullRadius = 58; //!< The radius the menu should have when fully enabled, in pixels.

		static Entity::ClassInfo m_sClass; //!< ClassInfo for this class.
		static BITMAP *s_CursorBitmap; //!< A static pointer to the bitmap to use as the cursor in any menu.

		GUIFont *m_LargeFont; //!< A pointer to the large font from FrameMan. Not owned here.

		Actor *m_Owner; //!< The owner Actor of this PieMenuGUI. Note that PieMenuGUIs do not necessarily need to have a owner.
		Controller *m_MenuController; //!< The Controller which controls this PieMenuGUI. Separate from the Controller of the owner or affected object (if there is one).
		MovableObject *m_AffectedObject; //!< The MovableObject this PieMenuGUI affects, if any. Only applies if there's no owner.
		MenuMode m_MenuMode; //!< The mode this PieMenuGUI is in. See MenuMode enum for more details.
		Vector m_CenterPos; //!< The center position of this PieMenuGUI in the scene.
		Matrix m_Rotation; //!< The rotation of this PieMenuGUI. Generally 0 for top level PieMenuGUIs.

		EnabledState m_EnabledState; //!< The enabled state of the PieMenuGUI.
		Timer m_EnableDisableAnimationTimer; //!< Timer for progressing enabling/disabling animations.
		Timer m_HoverTimer; //!< Timer to measure how long to hold a hovered over slice.

		IconSeparatorMode m_IconSeparatorMode; //!< The icon separator mode of this PieMenuGUI.
		int m_BackgroundThickness; //!< The thickness of the PieMenuGUI's background, in pixels.
		int m_BackgroundSeparatorSize; //!< The size of the PieMenuGUI's background separators, in pixels. Used differently based on the menu's IconSeparatorMode.
		bool m_DrawBackgroundTransparent; //!< Whether or not the PieMenuGUI's background should be drawn transparently.
		int m_BackgroundColor; //!< The color used for drawing the PieMenuGUI's background.
		int m_BackgroundBorderColor; //!< The color used for drawing borders for the PieMenuGUI's background.
		int m_SelectedItemBackgroundColor; //!< The color used for drawing selected PieMenuGUI items' backgrounds.

		std::array<PieQuadrant, 4> m_PieQuadrants; //!< The array of PieQuadrants that make up this PieMenuGUI. Quadrants may be individually enabled or disabled, affecting what's drawn.
		const PieSlice *m_HoveredSlice; //!< The PieSlice currently being hovered over.
		const PieSlice *m_ActivatedSlice; //!< The currently activated PieSlice, if there is one, or 0 if there's not.
		const PieSlice *m_AlreadyActivatedSlice; //!< The PieSlice that was most recently activated by pressing primary. Used to avoid duplicate activation when disabling.
		std::vector<PieSlice *> m_CurrentSlices; //!< All the PieSlices in this PieMenuGUI in INI order. Not owned here, just pointing to the ones above.

		std::unordered_map<const MovableObject *, std::function<void()>> m_OnPieMenuOpenListeners; //!< Unordered map of MovableObject pointers to functions to be called when the PieMenuGUI starts to open. Pointers are NOT owned.
		std::unordered_map<const MovableObject *, std::function<void()>> m_OnPieMenuCloseListeners; //!< Unordered map of MovableObject pointers to functions to be called when the PieMenuGUI starts to close. Pointers are NOT owned.

		int m_InnerRadius; //!< The current radius of the innermost circle of the pie menu, in pixels.
		bool m_CursorInVisiblePosition; //!< Whether or not this PieMenuGUI's cursor is in a visible position and should be shown.
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

#pragma region Draw Breakdown
		/// <summary>
		/// Handles figuring out the position to draw the PieMenuGUI at, accounting for any Scene seams.
		/// </summary>
		/// <param name="targetBitmap">A pointer to the BITMAP to draw on. Generally a screen BITMAP.</param>
		/// <param name="targetPos">The absolute position of the target bitmap's upper left corner in the scene.</param>
		/// <param name="drawPos">Out param, a Vector to be filled in with the position at which the PieMenuGUI should be drawn.</param>
		void CalculateDrawPosition(const BITMAP *targetBitmap, const Vector &targetPos, Vector &drawPos) const;

		/// <summary>
		/// Handles drawing icons for PieSlices' visual representation in the PieMenuGUI.
		/// </summary>
		/// <param name="targetBitmap">A pointer to the BITMAP to draw on. Generally a screen BITMAP.</param>
		/// <param name="drawPos">The seam corrected position at which the PieMenuGUI is being drawn.</param>
		void DrawPieIcons(BITMAP *targetBitmap, const Vector &drawPos) const;

		/// <summary>
		/// Handles drawing the cursor and description text for selected PieSlices.
		/// </summary>
		/// <param name="targetBitmap">A pointer to the BITMAP to draw on. Generally a screen BITMAP.</param>
		/// <param name="drawPos">The seam corrected position at which the PieMenuGUI is being drawn.</param>
		void DrawPieCursorAndPieSliceDescriptions(BITMAP *targetBitmap, const Vector &drawPos) const;
#pragma endregion

		/// <summary>
		/// Clears and refills the vector of current PieSlices for this PieMenuGUI. Also expands PieSlices where applicable, since these operations will always happen together.
		/// </summary>
		void ReloadCurrentPieSlices();

		/// <summary>
		/// Expands any PieSlices that border onto another PieQuadrant's unfilled slot, so they visually occupy that empty slot.
		/// </summary>
		void ExpandPieSliceIntoEmptySpaceIfPossible();

		/// <summary>
		/// Sets the given PieSlice as the selected one for this PieMenuGUI.
		/// </summary>
		/// <param name="pieSliceToSelect">The PieSlice to be selected. Has to be a PieSlice currently in this PieMenuGUI.</param>
		/// <param name="moveCursorToSlice">Whether to also move the cursor to the center of the newly selected PieSlice. Defaults to false.</param>
		/// <returns>Whether or not this resulted in a different PieSlice being selected. Also returns false if no PieSlice was selected.</returns>
		bool SelectPieSlice(const PieSlice *pieSliceToSelect, bool moveCursorToSlice = false);

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
