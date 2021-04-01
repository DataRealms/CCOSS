#ifndef _InventoryMenuGUI_
#define _InventoryMenuGUI_

#include "StandardIncludes.h"
#include "Vector.h"
#include "MovableObject.h"

namespace RTE {

	class InventoryMenuGUI {

	public:
#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate an InventoryMenuGUI object in system memory. Create() should be called before using the object.
		/// </summary>
		InventoryMenuGUI() { Clear(); }

		/// <summary>
		/// Makes the InventoryMenuGUI object ready for use.
		/// </summary>
		/// <param name="inventoryActor">The Actor whose inventory this GUI will display. Ownership is NOT TRANSFERRED! This is optional.</param>
		/// <returns>An error return value signaling sucess or any particular failure. Anything below 0 is an error signal.</returns>
		int Create(Actor *inventoryActor);
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Destructor method used to clean up a InventoryMenuGUI object before deletion from system memory.
		/// </summary>
		virtual ~InventoryMenuGUI() { Destroy(); }

		/// <summary>
		/// Destroys and resets (through Clear()) the InventoryMenuGUI object.
		/// </summary>
		void Destroy() { destroy_bitmap(m_BGBitmap); Clear(); }

		/// <summary>
		/// Resets the entire InventoryMenuGUI, including its inherited members, to their default settings or values.
		/// </summary>
		virtual void Reset() { Clear(); }
#pragma endregion

#pragma region Getters and Setters
		/// <summary>
		/// Gets the Actor whose inventory this GUI will display. The ownership of the Actor is NOT transferred!
		/// </summary>
		/// <returns>The Actor whose inventory this GUI will display. Ownership is NOT transferred!</returns>
		const Actor *GetActor() const { return m_Actor; }

		/// <summary>
		/// Sets the Actor whose inventory this GUI will display. The ownership of the Actor is NOT transferred!
		/// </summary>
		/// <param name="actor">The new Actor whose inventory this GUI will display. Ownership is NOT transferred!</param>
		void SetActor(Actor *actor) { m_Actor = actor; }
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
		static const size_t s_InventoryIconsPerRow = 5; //<! The number of inventory icons in each row of the drawn inventory.
		static const size_t s_InventoryIconSpacer = 2; //<! The space in between each icon in the drawn inventory.
		static const size_t s_InventoryIconMaxWidth = 50; //<! The max width of inventory icons in the inventory display.
		static const size_t s_InventoryIconMaxHeight = 25; //<! The max height of inventory icons in the inventory display.
		static const size_t s_InventoryEquippedIconHighlightWidth = 4; //<! The width of the highlight around the equipped inventory item.

		Actor *m_Actor; //<! The Actor whose inventory this GUI will display.

		BITMAP *m_BGBitmap; //<! The intermediary bitmap used to first draw the menu background, which will be blitted to the final draw target surface.
		GUIFont *m_MassDisplayFont; //<! The font used to display the mass of each inventory object.

		/// <summary>
		/// Takes a vector of MovableObject pointers representing a bisected and rejoined inventory as well as an equipped item or null pointer, and transforms it into appropriately filled rows.
		/// Additionally, calculates the number of unfilled spots in the top and bottom row (excluding the middle row), the total number of rows, and the maximum sprite dimensions of the inventory.
		/// </summary>
		/// <param name="organizedInventory">The inventory that will be transformed into rows.</param>
		/// <param name="inventoryObjectRowRemainders">A pair of floats which will be filled in with the remainders for the top and bottom rows.</param>
		/// <param name="inventoryObjectRows">A vector of rows (arrays of MovableObject pointers) that will be filled in with the transformed rows.</param>
		/// <param name="inventoryIconSize">A Vector that will be filled in with the dimensions of the largest item(s) in the inventory, limited to the max width and height constants.</param>
		void TransformOrganizedInventoryIntoRowsAndCalculateRemaindersAndIconSizes(const std::vector<MovableObject *> &organizedInventory, std::pair<float, float> &inventoryObjectRowRemainders, std::vector<std::array<MovableObject *, s_InventoryIconsPerRow>> &inventoryObjectRows, Vector &inventoryIconSize) const;

		/// <summary>
		/// Draws icons and mass indicators based on the given vector of inventory object rows.
		/// </summary>
		/// <param name="targetBitmap">A pointer to the BITMAP to draw on. Generally a screen BITMAP.</param>
		/// <param name="drawPos">The seam corrected position at which the PieMenu is being drawn.</param>
		/// <param name="totalOrganizedInventorySize">The total size of the organized inventory that will be drawn.</param>
		/// <param name="equippedItem">A pointer to the equipped item if there is one.</param>
		/// <param name="inventoryObjectRowRemainders">A pair of floats representing the number of unfilled spots in the top and bottom row of the transformed organized inventory.</param>
		/// <param name="inventoryObjectRows">A vector of rows (arrays of MovableObject pointers) that represents the organized inventory, transformed into rows.</param>
		/// <param name="inventoryIconSize">A Vector containing the dimensions of the largest item(s) in the inventory, limited to the max width and height constants.</param>
		void DrawInventoryItemIconsAndMassIndicators(BITMAP *targetBitmap, const Vector &drawPos, float totalOrganizedInventorySize, const MovableObject *equippedItem, std::pair<float, float> inventoryObjectRowRemainders, const std::vector<std::array<MovableObject *, s_InventoryIconsPerRow>> &inventoryObjectRows, const Vector &inventoryIconSize) const;

		/// <summary>
		/// 
		/// </summary>
		/// <param name="targetBitmap">A pointer to the BITMAP to draw on. Generally a screen BITMAP.</param>
		/// <param name="currentInventoryIconDrawPos">The position at which the current inventory row should start being drawn.</param>
		/// <param name="equippedItem">A pointer to the equipped item if there is one.</param>
		/// <param name="inventoryObjectRows">The row of the organized inventory that will be drawn.</param>
		/// <param name="inventoryIconSize">A Vector containing the dimensions of the largest item(s) in the inventory, limited to the max width and height constants.</param>
		void DrawInventoryItemIconRow(BITMAP *targetBitmap, Vector &currentInventoryIconDrawPos, const MovableObject *equippedItem, const std::array<MovableObject *, s_InventoryIconsPerRow> &inventoryObjectRow, const Vector &inventoryIconSize) const;

		/// <summary>
		/// Clears all the member variables of this PieMenuGUI, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();

		// Forbidding copying
		InventoryMenuGUI(const InventoryMenuGUI &reference) {}
		InventoryMenuGUI &operator=(const InventoryMenuGUI &rhs) { return *this; }
	};
}
#endif