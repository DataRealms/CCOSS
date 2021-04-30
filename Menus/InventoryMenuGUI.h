#ifndef _InventoryMenuGUI_
#define _InventoryMenuGUI_

#include "StandardIncludes.h"
#include "Icon.h"
#include "Timer.h"
#include "Vector.h"

namespace RTE {

	class Controller;
	class MovableObject;
	class Actor;
	class GUIFont;
	class AllegroBitmap;

	/// <summary>
	/// A GUI menu for managing inventories.
	/// </summary>
	class InventoryMenuGUI {

	public:

		/// <summary>
		/// An enum for the the modes an InventoryMenuGUI can have.
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
		void SetController(Controller *controller) { m_ActivityPlayerController = controller; }

		/// <summary>
		/// Gets whether the menu is enabled or not.
		/// </summary>
		/// <returns>Whether the menu is enabled.</returns>
		bool IsEnabled() const { return m_EnabledState == EnabledState::Enabled || m_EnabledState == EnabledState::Enabling; }

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

		/// <summary>
		/// Gets the Actor whose inventory this GUI will display. The ownership of the Actor is NOT transferred!
		/// </summary>
		/// <returns>The Actor whose inventory this GUI will display. Ownership is NOT transferred!</returns>
		const Actor * GetInventoryActor() const { return m_InventoryActor; }

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
		/// A struct containing all information required to drawn and animate a carousel item box.
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
		/// Enumeration for enabled states when enabling/disabling the InventoryMenuGUI.
		/// </summary>
		enum class EnabledState { Enabling, Enabled, Disabling, Disabled };

		/// <summary>
		/// Enumeration for which direction an inventory is being swapped in.
		/// </summary>
		enum class CarouselAnimationDirection { Left = -1, None, Right};

		static const int c_ItemsPerRow = 5; //!< The number of items per row of the inventory display. MUST be an odd nubmer. Used for all MenuModes.
		static const int c_MinimumItemPadding = 1; //!< The padding between item icons and their containing boxes. Items will have at least this much padding on all sides. Used for all MenuModes.
		static const int c_MenuVerticalOffset = 30; //!< The offset the menu will have below it, to avoid drawing over its target. Used for all MenuModes.

		static const Vector c_CarouselBoxMaxSize; //!< The size of the largest item box in the carousel, i.e. the one in the middle. Used for Carousel MenuMode.
		static const Vector c_CarouselBoxMinSize; //!< The size of the smallest item box in the carousel, i.e. the ones at the ends, excluding the exiting box, which is actually one size step smaller. Used for Carousel MenuMode.
		static const Vector c_CarouselBoxSizeStep; //!< The size step between each item box in the carousel. Used for Carousel MenuMode.
		static const int c_CarouselBoxCornerRadius; //!< The radius of the circles used to make rounded corners for carousel boxes. Calculated automatically based on max and min size. Used for Carousel MenuMode.
		static const int c_MultipleItemInBoxOffset = 3; //!< How many pixels up and right items should be offset if there are more than one in a item box. Used for Carousel MenuMode.

		static const int c_ItemBoxSize = 50; //!< The size of item boxes in the inventory display. Used for Full/Transfer MenuModes.
		static const int c_DefaultNumberOfRows = 5; //!< The default number of rows in the inventory display. Used for Full/Transfer MenuModes.

		GUIFont *m_SmallFont; //!< A pointer to the small font from FrameMan. Not owned here.
		GUIFont *m_LargeFont; //!< A pointer to the large font from FrameMan. Not owned here.

		Controller *m_ActivityPlayerController; //!< The Controller which controls this menu. Separate from the Controller of the InventoryActor.
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
		
		std::unique_ptr<BITMAP> m_CarouselBGBitmap; //!< The intermediary Bitmap onto which the carousel's background boxes are drawn. It is then drawn onto the Bitmap the carousel is drawn to. Used in Carousel MenuMode.
		std::unique_ptr<BITMAP> m_CarouselBitmap; //!< The intermediary Bitmap onto which the carousel's items and mass indicators are drawn. It is then drawn onto the Bitmap the carousel is drawn to. Used in Carousel MenuMode.

		CarouselAnimationDirection m_CarouselAnimationDirection; //!< Which direction the carousel is currently animating in, if any. Used for Carousel MenuMode animations.
		Timer m_CarouselAnimationTimer; //!< Timer for progressing carousel animations.
		std::array<std::unique_ptr<CarouselItemBox>, c_ItemsPerRow> m_CarouselItemBoxes; //!< An array of CarouselItemBoxes that make up the carousel. Used in Carousel MenuMode.
		std::unique_ptr<CarouselItemBox> m_CarouselExitingItemBox; //!< A special case CarouselItemBox used to display the item that is exiting during animations. Used in Carousel MenuMode.

		bool m_DisplayOnly; //!< Whether this GUI is display only, or can be interacted with, and thereby affect the inventory it's displaying.
		std::vector<std::array<MovableObject *, c_ItemsPerRow>> m_InventoryObjectRows; //!< A collection describing the rows of items this InventoryMenuGUI is displaying, when not in Carousel MenuMode.
		/* Full mode stuff here, copied from other guis
		std::unique_ptr<AllegroScreen> m_GUIScreen; //!< The GUIScreen interface that will be used by this ObjectPickerGUI's GUIControlManager.
		std::unique_ptr<AllegroInput> m_GUIInput; //!< The GUIInput interface that will be used by this ObjectPickerGUI's GUIControlManager.
		std::unique_ptr<GUIControlManager> m_GUIControlManager; //!< The control manager which holds all the controls.
		GUICollectionBox *m_ParentBox; //!< Collection box of the picker GUI.
		GUICollectionBox *m_PopupBox; //!< Collection box of the buy popups that contain information about items.
		GUILabel *m_PopupText; //!< Label displaying the item popup description.
		GUIListBox *m_GroupsList; //!< The ListBox which lists all the groups.
		*/

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
		/// Handles MenuMode specific updating for when the InventoryMenuGUI is in Transfer MenuMode.
		/// </summary>
		void UpdateTransferMode();
#pragma endregion

#pragma region Draw Breakdown
		/// <summary>
		/// Draws the InventoryMenuGUI when it's in Carousel MenuMode.
		/// </summary>
		/// <param name="targetBitmap">A pointer to a BITMAP to draw on. Generally a screen BITMAP.</param>
		/// <param name="drawPos">The position at which to draw the carousel.</param>
		void DrawCarouselMode(BITMAP *targetBitmap, const Vector &drawPos) const;

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