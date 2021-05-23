#ifndef _InventoryMenuGUI_
#define _InventoryMenuGUI_

#include "StandardIncludes.h"
#include "Timer.h"
#include "Vector.h"

#include "GUI.h"
#include "GUIControlManager.h"
#include "AllegroScreen.h"
#include "AllegroInput.h"

namespace RTE {

	class Controller;
	class MovableObject;
	class Actor;
	class Icon;
	class AllegroBitmap;
	class GUIFont;
	class GUICollectionBox;
	class GUILabel;
	class GUIButton;
	class GUIScrollbar;

	/// <summary>
	/// A GUI menu for managing inventories.
	/// </summary>
	class InventoryMenuGUI {

	public:

		/// <summary>
		/// Enumeration for the the modes an InventoryMenuGUI can have.
		/// </summary>
		enum class MenuMode { Carousel, Full, Transfer };

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate an InventoryMenuGUI object in system memory. Create() should be called before using the object.
		/// </summary>
		InventoryMenuGUI() { Clear(); }

		/// <summary>
		/// Makes the InventoryMenuGUI object ready for use.
		/// </summary>
		/// <param name="activityPlayerController">A pointer to a Controller which will control this Menu. Ownership is NOT transferred!</param>
		/// <param name="inventoryActor">The Actor whose inventory this GUI will display. Ownership is NOT transferred!</param>
		/// <param name="menuMode">The mode this menu should use when it's enabled. Defaults to Carousel.</param>
		/// <returns>An error return value signaling sucess or any particular failure. Anything below 0 is an error signal.</returns>
		int Create(Controller *activityPlayerController, Actor *inventoryActor = nullptr, MenuMode menuMode = MenuMode::Carousel);
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Resets the entire InventoryMenuGUI, including its inherited members, to their default settings or values.
		/// </summary>
		virtual void Reset() { Clear(); }
#pragma endregion

#pragma region Getters and Setters
		/// <summary>
		/// Sets the controller used by this. The ownership of the controller is NOT transferred!
		/// </summary>
		/// <param name="controller">The new controller for this menu. Ownership is NOT transferred!</param>
		void SetController(Controller *controller) { m_MenuController = controller; }

		/// <summary>
		/// Gets the Actor whose inventory this GUI will display. The ownership of the Actor is NOT transferred!
		/// </summary>
		/// <returns>The Actor whose inventory this GUI will display. Ownership is NOT transferred!</returns>
		const Actor *GetInventoryActor() const { return m_InventoryActor; }

		/// <summary>
		/// Sets the Actor whose inventory this GUI will display. The ownership of the Actor is NOT transferred!
		/// </summary>
		/// <param name="actor">The new Actor whose inventory this GUI will display. Ownership is NOT transferred!</param>
		void SetInventoryActor(Actor *newInventoryActor);

		/// <summary>
		/// Gets the MenuMode this InventoryMenuGUI is currently in.
		/// </summary>
		/// <returns>The current MenuMode of this InventoryMenuGUI.</returns>
		MenuMode GetMenuMode() { return m_MenuMode; }

		/// <summary>
		/// Sets the MenuMode for this InventoryMenuGUI to be in.
		/// </summary>
		/// <param name="newMenuMode">The new MenuMode of this InventoryMenuGUI.</param>
		void SetMenuMode(MenuMode newMenuMode) { m_MenuMode = newMenuMode; }

		/// <summary>
		/// Gets whether the menu is enabled or not.
		/// </summary>
		/// <returns>Whether the menu is enabled.</returns>
		bool IsEnabled() const { return m_EnabledState == EnabledState::Enabled || m_EnabledState == EnabledState::Enabling; }

		/// <summary>
		/// Gets whether the menu is enabled and not in carousel mode or not.
		/// </summary>
		/// <returns>Whether the menu is enabled and not in carousel mode</returns>
		bool IsEnabledAndNotCarousel() const { return IsEnabled() && m_MenuMode != MenuMode::Carousel; }

		/// <summary>
		/// Gets whether the menu is in the process of enabling or disabling.
		/// </summary>
		/// <returns></returns>
		bool IsEnablingOrDisabling() const { return m_EnabledState == EnabledState::Enabling || m_EnabledState == EnabledState::Disabling; }

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
#pragma endregion

#pragma region Full And Transfer Mode Getters and Setters
		/// <summary>
		/// Clears the selected item for this InventoryMenuGUI.
		/// </summary>
		void ClearSelectedItem();

		/// <summary>
		/// Sets the selected item for this InventoryMenuGUI.
		/// If the button pointer or object pointer are null, or both indices are null, the selected item will instead be cleared.
		/// </summary>
		/// <param name="selectedItemButton">A pointer to the GUIButton that is selected.</param>
		/// <param name="selectedItemObject">A pointer to the MovableObject that is selected.</param>
		/// <param name="inventoryIndex">The index of this selected item in the displayed inventory. -1 means the item is not in the inventory.</param>
		/// <param name="equippedItemIndex">The index of this selected item in the vector of equipped items. -1 means the item is not in the inventory.</param>
		/// <param name="isBeingDragged">Whether or not the selected item is being dragged.</param>
		void SetSelectedItem(GUIButton *selectedItemButton, MovableObject *selectedItemObject, int inventoryIndex, int equippedItemIndex, bool isBeingDragged);
#pragma endregion

#pragma region Updating
		/// <summary>
		/// Updates the state of this GUI each frame.
		/// </summary>
		virtual void Update();

		/// <summary>
		/// Draws the GUI.
		/// </summary>
		/// <param name="targetBitmap">A pointer to a BITMAP to draw on. Generally a screen BITMAP.</param>
		/// <param name="targetPos">The absolute position of the target bitmap's upper left corner in the scene.</param>
		virtual void Draw(BITMAP *targetBitmap, const Vector &targetPos = Vector()) const;
#pragma endregion

	private:
		
		/// <summary>
		/// A struct containing all information required to drawn and animate a carousel item box in Carousel MenuMode.
		/// </summary>
		struct CarouselItemBox {
			MovableObject *Item;
			bool IsForEquippedItems;
			Vector FullSize;
			Vector CurrentSize;
			Vector Pos;
			Vector IconCenterPosition;
			std::pair<bool, bool> RoundedAndBorderedSides;

			/// <summary>
			/// Fills the passed in vector of Bitmaps and float with the appropriate graphical icons and mass for the Item of this CarouselItemBox.
			/// If this CarouselItemBox IsForEquippedItem, it will instead used the passed in pointer to the vector of equipped items.
			/// </summary>
			/// <param name="itemIcons">A vector of Bitmaps to be filled in with the icon(s) for this CarouselItemBox's item(s).</param>
			/// <param name="totalItemMass">A float to be filled in with the total mass of this CarouselItemBox's item(s).</param>
			/// <param name="equippedItems">A pointer to the vector of equipped items to be used if the CarouselItemBox IsForEquippedItem.</param>
			void GetIconsAndMass(std::vector<BITMAP *> &itemIcons, float &totalItemMass, const std::vector<MovableObject *> *equippedItems) const;
		};

		/// <summary>
		/// A struct containing all information required to describe a selected item in Full/Transfer MenuMode.
		/// </summary>
		struct GUISelectedItem {
			GUIButton *Button;
			MovableObject *Object;
			int InventoryIndex;
			int EquippedItemIndex;
			bool IsBeingDragged;
			int DragHoldCount;

			/// <summary>
			/// Whether this selected item was being dragged for long enough that it matters. This helps make dragging not cause problems during instant clicks and releases.
			/// </summary>
			/// <returns>Whether this selected item was being dragged for long enough that it matters.</returns>
			bool DragWasHeldForLongEnough() const { return IsBeingDragged && DragHoldCount > 5; }
		};

		/// <summary>
		/// Enumeration for enabled states when enabling/disabling the InventoryMenuGUI.
		/// </summary>
		enum class EnabledState { Enabling, Enabled, Disabling, Disabled };

		/// <summary>
		/// Enumeration for which direction an inventory is being swapped in.
		/// </summary>
		enum class CarouselAnimationDirection { Left = -1, None, Right};

		static const int c_ItemsPerRow = 5; //!< The number of items per row of the inventory display. MUST be an odd nubmer. Used in all MenuModes.
		static const int c_MinimumItemPadding = 1; //!< The padding between item icons and their containing boxes. Items will have at least this much padding on all sides. Used in all MenuModes.

		static const int c_CarouselMenuVerticalOffset = 85; //!< How high above its target the carousel will be. Used in Carousel MenuModes.
		static const Vector c_CarouselBoxMaxSize; //!< The size of the largest item box in the carousel, i.e. the one in the middle. Used in Carousel MenuMode.
		static const Vector c_CarouselBoxMinSize; //!< The size of the smallest item box in the carousel, i.e. the ones at the ends, excluding the exiting box, which is actually one size step smaller. Used in Carousel MenuMode.
		static const Vector c_CarouselBoxSizeStep; //!< The size step between each item box in the carousel. Used in Carousel MenuMode.
		static const int c_CarouselBoxCornerRadius; //!< The radius of the circles used to make rounded corners for carousel boxes. Calculated automatically based on max and min size. Used in Carousel MenuMode.
		static const int c_MultipleItemInBoxOffset = 3; //!< How many pixels up and right items should be offset if there are more than one in a item box. Used in Carousel MenuMode.

		static const int c_FullViewPageItemLimit = c_ItemsPerRow * 3; //!< The default number of rows in the full inventory display. Used in Full/Transfer MenuModes.
		static const int c_FullMenuVerticalOffset = 35; //!< How high above its target the full GUI will be. Used in Full/Transfer MenuModes.

		static BITMAP *s_CursorBitmap; //!< The cursor image shared by all GUIs.

		GUIFont *m_SmallFont; //!< A pointer to the small font from FrameMan. Not owned here.
		GUIFont *m_LargeFont; //!< A pointer to the large font from FrameMan. Not owned here.

		Controller *m_MenuController; //!< The Controller which controls this menu.
		Actor *m_InventoryActor; //!< The Actor whose inventory this GUI will display.
		MenuMode m_MenuMode; //!< The mode this menu is in. See MenuMode enum for more details.
		Vector m_CenterPos; //!< The center position of this menu in the scene.

		EnabledState m_EnabledState; //!< The enabled state of the menu.
		Timer m_EnableDisableAnimationTimer; //!< Timer for progressing enabling/disabling animations.
		
		bool m_InventoryActorIsHuman; //!< Whether the Actor whose inventory this GUI will display is an AHuman.
		std::vector<MovableObject *> m_InventoryActorEquippedItems; //!< A vector of pointers to the equipped items of the Actor whose inventory this GUI will display, if applicable.

		bool m_CarouselDrawEmptyBoxes; //!< Whether or not the carousel should draw empty item boxes. Used in Carousel MenuMode.
		bool m_CarouselBackgroundTransparent; //!< Whether or not the carousel's background should be drawn transparently. Used in Carousel MenuMode.
		int m_CarouselBackgroundBoxColor; //!< The color used for the background box of the carousel. Used in Carousel MenuMode.
		Vector m_CarouselBackgroundBoxBorderSize; //!< The size of the border around the background box of the carousel. Used in Carousel MenuMode.
		int m_CarouselBackgroundBoxBorderColor; //!< The color used for the border of the background box of the carousel. Used in Carousel MenuMode.
		
		CarouselAnimationDirection m_CarouselAnimationDirection; //!< Which direction the carousel is currently animating in, if any. Used in Carousel MenuMode animations.
		Timer m_CarouselAnimationTimer; //!< Timer for progressing carousel animations.
		std::array<std::unique_ptr<CarouselItemBox>, c_ItemsPerRow> m_CarouselItemBoxes; //!< An array of CarouselItemBoxes that make up the carousel. Used in Carousel MenuMode.
		std::unique_ptr<CarouselItemBox> m_CarouselExitingItemBox; //!< A special case CarouselItemBox used to display the item that is exiting during animations. Used in Carousel MenuMode.
		std::unique_ptr<BITMAP> m_CarouselBitmap; //!< The intermediary Bitmap onto which the carousel's items and mass indicators are drawn. It is then drawn onto the Bitmap the carousel is drawn to. Used in Carousel MenuMode.
		std::unique_ptr<BITMAP> m_CarouselBGBitmap; //!< The intermediary Bitmap onto which the carousel's background boxes are drawn. It is then drawn onto the Bitmap the carousel is drawn to. Used in Carousel MenuMode.

		bool m_GUIDisplayOnly; //!< Whether this GUI is display only, or can be interacted with, and thereby affect the inventory it's displaying. Used in Full/Transfer MenuModes.
		bool m_GUIShowEmptyRows; //!< Whether this GUI should show empty rows, up to the FullPageViewItemLimit.
		Vector m_GUICursorPos; //!< Screen position of the cursor. Used in Full/Transfer MenuModes.
		Vector m_PreviousGUICursorPos; //!< The previous screen position of the cursor. Used in Full/Transfer MenuModes.
		std::unique_ptr<GUISelectedItem> m_GUISelectedItem; //!< The selected item for this InventoryMenuGUI. Used in Full/Transfer MenuModes.

		Timer m_GUIRepeatStartTimer; //!< Measures the time to when to start repeating inputs when they're held down. Used in Full/Transfer MenuModes.
		Timer m_GUIRepeatTimer; //!< Measures the interval between input repeats. Used in Full/Transfer MenuModes.
		GUIButton *m_KeyboardOrControllerHighlightedButton; //!< A pointer to the GUIButton currently hightlighted by the keyboard or controller.

		std::unique_ptr<GUIControlManager> m_GUIControlManager; //!< The control manager which holds all the controls. All GUI elements are only used in Full/Transfer MenuModes.
		std::unique_ptr<AllegroScreen> m_GUIScreen; //!< The GUIScreen interface that will be used by this InventoryMenuGUI's GUIControlManager.
		std::unique_ptr<AllegroInput> m_GUIInput; //!< The GUIInput interface that will be used by this InventoryMenuGUI's GUIControlManager.
		
		GUICollectionBox *m_GUITopLevelBox; //!< The top-level GUICollectionBox of the InventoryMenuGUI.
		Vector m_GUITopLevelBoxFullSize; //!< A Vector holding the full size of the top level box for enabling/disabling animations.
		GUILabel *m_GUIInformationText; //!< A GUILabel to used to display useful information while operating the menu.
		bool m_GUIShowInformationText; //!< Whether or information text explaining how to use the menu should be showing.
		GUIButton *m_GUIInformationToggleButton; //!< A GUIButton for showing and hiding the information text.
		const Icon *m_GUIInformationToggleButtonIcon; //!< A pointer to the PresetMan pie icon for information, used here for the information toggle button. Not Owned here.

		GUICollectionBox *m_GUIEquippedItemsBox; //!< A GUICollectionBox for equipped items and related content.
		GUIButton *m_GUISwapSetButton; //!< A GUIButton for swapping equipment sets.
		GUIButton *m_GUIEquippedItemButton; //!< A GUIButton containing the equipped item if any.
		GUIButton *m_GUIOffhandEquippedItemButton; //!< A GUIButton containing the offhand equipped item if any.
		GUIButton *m_GUIReloadButton; //!< A GUIButton for reloading the selected item.
		const Icon *m_GUIReloadButtonIcon; //!< A pointer to the PresetMan pie icon for reloading, used here for the reload button. Not Owned here.
		GUIButton *m_GUIDropButton; //!< A GUIButton for dropping the selected item.
		const Icon *m_GUIDropButtonIcon; //!< T pointer to the PresetMan pie icon for dropping items, used here for the drop button. Not Owned here.
		
		GUICollectionBox *m_GUIInventoryItemsBox; //!< A GUICollectionBox for inventory items and the scrollbar.
		GUIScrollbar *m_GUIInventoryItemsScrollbar; //!< A GUIScrollbar for scrolling through inventory items.
		std::vector<std::pair<MovableObject *, GUIButton *>> m_GUIInventoryItemButtons; //!< A vector of pairs of MovableObject pointers and GUIButton pointers, connecting inventory GUIButtons to their corresponding MovableObjects.

		GUICollectionBox *m_GUIPopupBox; //!< A GUICollectionBox for the popup showing item information.
		GUILabel *m_GUIPopupText; //!< A GUILabel displaying the item popup description.

#pragma region Create Breakdown
		/// <summary>
		/// Gets whether the InventoryMenuGUI is ready to be activated in Carousel MenuMode. If it's not, that mode should be setup.
		/// </summary>
		/// <returns>Whether or not the InventoryMenuGUI is ready to be activated in carousel MenuMode.</returns>
		bool CarouselModeReadyForUse() { return m_CarouselBitmap && m_CarouselBGBitmap; }

		/// <summary>
		/// Makes the InventoryMenuGUI object ready for Carousel MenuMode use.
		/// </summary>
		/// <returns>An error return value signaling sucess or any particular failure. Anything below 0 is an error signal.</returns>
		int SetupCarouselMode();

		/// <summary>
		/// Gets whether the InventoryMenuGUI is ready to be activated in Full or Transfer MenuMode. If it's not, that mode should be setup.
		/// </summary>
		/// <returns>Whether or not the InventoryMenuGUI is ready to be activated in Full or Transfer MenuMode.</returns>
		bool FullOrTransferModeReadyForUse() { return m_GUIControlManager && m_GUIScreen && m_GUIInput; }

		/// <summary>
		/// Makes the InventoryMenuGUI object ready for Full or Transfer MenuMode use.
		/// </summary>
		/// <returns>An error return value signaling sucess or any particular failure. Anything below 0 is an error signal.</returns>
		int SetupFullOrTransferMode();
#pragma endregion

#pragma region Update Breakdown
		/// <summary>
		/// Handles MenuMode specific updating for when the InventoryMenuGUI is in Carousel MenuMode.
		/// </summary>
		void UpdateCarouselMode();

		/// <summary>
		/// Handles sorting out positions and sizes of CarouselItemBoxes when in Carousel MenuMode.
		/// </summary>
		void UpdateCarouselItemBoxSizesAndPositions();

		/// <summary>
		/// Handles MenuMode specific updating for when the InventoryMenuGUI is in Full MenuMode.
		/// </summary>
		void UpdateFullMode();

		/// <summary>
		/// Handles visibility and content for equipped item buttons in Full MenuMode. Does not deal with button input.
		/// </summary>
		void UpdateFullModeEquippedItemButtons();

		/// <summary>
		/// Handles hiding and showing the scrollbar, and calculating everything related to it in Full MenuMode.
		/// </summary>
		/// <param name="inventory">A pointer to the inventory this InventoryMenuGUI is displaying.</param>
		void UpdateFullModeScrollbar(const std::deque<MovableObject *> *inventory);

		/// <summary>
		/// Handles content and enabled status for inventory item buttons in Full MenuMode. Does not deal with button input.
		/// </summary>
		/// <param name="inventory">A pointer to the inventory this InventoryMenuGUI is displaying.</param>
		void UpdateFullModeInventoryItemButtons(const std::deque<MovableObject *> *inventory);

		/// <summary>
		/// Handles everything for displaying information text in Full MenuMode.
		/// </summary>
		/// <param name="inventory">A pointer to the inventory this InventoryMenuGUI is displaying.</param>
		void UpdateFullModeInformationText(const std::deque<MovableObject *> *inventory);

		/// <summary>
		/// Handles updating icons for non-item buttons in Full MenuMode.
		/// </summary>
		void UpdateFullModeNonItemButtonIcons();

		/// <summary>
		/// Handles MenuMode specific updating for when the InventoryMenuGUI is in Transfer MenuMode.
		/// </summary>
		void UpdateTransferMode();
#pragma endregion

#pragma region GUI Input Handling
		/// <summary>
		/// Player input handling for all devices.
		/// </summary>
		void HandleInput();

		/// <summary>
		/// Player mouse input event handling of the GUIControls of this InventoryMenuGUI.
		/// </summary>
		/// <returns>Whether the mouse was released this frame after being held long enough to count, useful to prevent duplicate button handling when releasing the left click.</returns>
		bool HandleMouseInput();

		/// <summary>
		/// Player keyboard or controller input event handling of the GUIControls of this InventoryMenuGUI.
		/// </summary>
		void HandleNonMouseInput();

		/// <summary>
		/// Handles item button press command events from the GUIControls of this InventoryMenuGUI by selecting/de-selecting the corresponding item.
		/// </summary>
		/// <param name="pressedButton">A pointer to the GUIButton that was pressed.</param>
		/// <param name="buttonObject">A pointer to the MovableObject the GUIButton represents.</param>
		/// <param name="buttonEquippedItemIndex">The index of this button in the vector of equipped items if applicable. The default value of -1 means it's not an equipped item.</param>
		/// <param name="buttonHeld">Whether or not the button was held.</param>
		void HandleItemButtonPressOrHold(GUIButton *pressedButton, MovableObject *buttonObject, int buttonEquippedItemIndex = -1, bool buttonHeld = false);
#pragma endregion

#pragma region GUI Button Actions
		/// <summary>
		/// Handles set swap button press command events from the GUIControls of this InventoryMenuGUI.
		/// </summary>
		void SwapEquippedItemSet() {}

		/// <summary>
		/// Swaps an equipped item with one in the inventory Actor's inventory.
		/// </summary>
		/// <param name="equippedItem">A pointer to the equipped item being swapped out.</param>
		/// <param name="inventoryItemIndex">The index in the inventory of the item being swapped in.</param>
		void SwapEquippedItemAndInventoryItem(MovableObject *equippedItemToSwapOut, int inventoryItemIndexToSwapIn);

		/// <summary>
		/// Reloads the selected item if it is equipped, or swaps to it and then reloads it if it isn't.
		/// </summary>
		void ReloadSelectedItem();

		/// <summary>
		/// Drops the selected item, roating its drop velocity in the direction of the passed in dropDirection Vector.
		/// </summary>
		/// <param name="dropDirection">A pointer to a Vector containing the direction the selected item should be dropped in. Nullptr means standard dropping will be used.</param>
		void DropSelectedItem(const Vector *dropDirection = nullptr);
#pragma endregion

#pragma region Draw Breakdown
		/// <summary>
		/// Draws the InventoryMenuGUI when it's in Carousel MenuMode.
		/// </summary>
		/// <param name="targetBitmap">A pointer to a BITMAP to draw on. Generally a screen BITMAP.</param>
		/// <param name="drawPos">The position at which to draw the carousel.</param>
		void DrawCarouselMode(BITMAP *targetBitmap, const Vector &drawPos) const;

		/// <summary>
		/// Draws the InventoryMenuGUI when it's in Full MenuMode.
		/// </summary>
		/// <param name="targetBitmap">A pointer to a BITMAP to draw on. Generally a screen BITMAP.</param>
		/// <param name="drawPos">The position at which to draw the GUI.</param>
		void DrawFullMode(BITMAP *targetBitmap, const Vector &drawPos) const;

		/// <summary>
		/// Draws the specified CarouselItemBox's background to the carousel background Bitmap.
		/// </summary>
		/// <param name="itemBoxToDraw">The CarouselItemBox to draw.</param>
		void DrawCarouselItemBoxBackground(const CarouselItemBox &itemBoxToDraw) const;

		/// <summary>
		/// Draws the specified CarouselItemBox's item(s) and mass text to the carousel Bitmap.
		/// </summary>
		/// <param name="itemBoxToDraw">The CarouselItemBox to draw.</param>
		/// <param name="carouselAllegroBitmap">An AllegroBitmap of the bitmap the CarouselItemBox should draw its foreground to. Used for drawing mass strings, and predefined to avoid needless creation.</param>
		void DrawCarouselItemBoxForeground(const CarouselItemBox &itemBoxToDraw, AllegroBitmap *carouselAllegroBitmap) const;
#pragma endregion

		/// <summary>
		/// Clears all the member variables of this PieMenuGUI, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();

		// Disallow the use of some implicit methods.
		InventoryMenuGUI(const InventoryMenuGUI &reference) = delete;
		InventoryMenuGUI & operator=(const InventoryMenuGUI &rhs) = delete;
	};
}
#endif