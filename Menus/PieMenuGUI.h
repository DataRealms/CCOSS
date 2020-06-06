#ifndef _PIEMENUGUI_
#define _PIEMENUGUI_

#include "Controller.h"
#include "Icon.h"

namespace RTE {
	class GUIFont;
	class MovableObject;
	class Actor;

	class PieMenuGUI {

	public:

		enum PieSliceIndex {
			PSI_NONE = 0,
			// Inventory management
			PSI_PICKUP,
			PSI_DROP,
			PSI_NEXTITEM,
			PSI_PREVITEM,
			PSI_RELOAD,
			// Menu and GUI activations
			PSI_BUYMENU,
			PSI_STATS,
			PSI_MINIMAP,
			PSI_FORMSQUAD,
			PSI_CEASEFIRE,
			// AI Modes
			PSI_SENTRY,
			PSI_PATROL,
			PSI_BRAINHUNT,
			PSI_GOLDDIG,
			PSI_GOTO,
			PSI_RETURN,
			PSI_STAY,
			PSI_DELIVER,
			PSI_SCUTTLE,
			// Editor stuff
			PSI_DONE,
			PSI_LOAD,
			PSI_SAVE,
			PSI_NEW,
			PSI_PICK,
			PSI_MOVE,
			PSI_REMOVE,
			PSI_INFRONT,
			PSI_BEHIND,
			PSI_ZOOMIN,
			PSI_ZOOMOUT,
			PSI_TEAM1,
			PSI_TEAM2,
			PSI_TEAM3,
			PSI_TEAM4,
			// Custom
			PSI_SCRIPTED,
			// How many pie slice types there are
			PSI_COUNT
		};

#pragma region Slice
		class Slice : public Serializable {

		public:
			/// <summary>
			/// An enum for the directions a slice can be placed in, in a given pie menu.
			/// </summary>
			enum SliceDirection {
				NONE = 0,
				UP,
				RIGHT,
				DOWN,
				LEFT
			};

#pragma region Creation
			/// <summary>
			/// Constructor method used to instantiate a Slice object in system memory. Create() should be called before using the object.
			/// </summary>
			Slice() { Clear(); }

			/// <summary>
			/// Constructor method used to instantiate a Slice object in system memory. Create() should be called before using the object.
			/// </summary>
			/// <param name="description">The description of the pie slice.</param>
			/// <param name="sliceType">The type of the pie slice.</param>
			/// <param name="direction">The direction of the pie slice.</param>
			/// <param name="enabled">Whether the slice is enabled. Defaults to true.</param>
			Slice(std::string description, PieSliceIndex sliceType, SliceDirection direction, bool enabled = true) { Clear(); m_Description = description; m_SliceType = sliceType; m_Direction = direction; m_Enabled = enabled; }

			/// <summary>
			/// Makes the Slice object ready for use.
			/// </summary>
			/// <returns>An error return value signaling sucess or any particular failure. Anything below 0 is an error signal.</returns>
			virtual int Create();

			/// <summary>
			/// Creates a Slice to be identical to another, by deep copy.
			/// </summary>
			/// <param name="reference">A reference to the Slice to deep copy.</param>
			/// <returns>An error return value signaling sucess or any particular failure. Anything below 0 is an error signal.</returns>
			virtual int Create(const Slice &reference);

			/// <summary>
			/// Makes the Serializable ready for use. Pie slices being read from a file will always be scripted SliceType.
			/// </summary>
			/// <param name="reader">A Reader that the Serializable will create itself from.</param>
			/// <param name="checkType">Whether there is a class name in the stream to check against to make sure the correct type is being read from the stream.</param>
			/// <param name="doCreate">Whether to do any additional initialization of the object after reading in all the properties from the Reader. This is done by calling Create().</param>
			/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
			virtual int Create(Reader &reader, bool checkType = true, bool doCreate = true) { m_SliceType = PSI_SCRIPTED; return Serializable::Create(reader, checkType, doCreate); }
#pragma endregion

#pragma region Destruction
			/// <summary>
			/// Resets the entire Serializable, including its inherited members, to their default settings or values.
			/// </summary>
			virtual void Reset() { Clear(); }
#pragma endregion

#pragma region INI Handling
			/// <summary>
			/// Reads a property value from a Reader stream. If the name isn't recognized by this class, then ReadProperty of the parent class is called.
			/// If the property isn't recognized by any of the base classes, false is returned, and the Reader's position is untouched.
			/// </summary>
			/// <param name="propName">The name of the property to be read.</param>
			/// <param name="reader">A Reader lined up to the value of the property to be read.</param>
			/// <returns>
			/// An error return value signaling whether the property was successfully read or not.
			/// 0 means it was read successfully, and any nonzero indicates that a property of that name could not be found in this or base classes.
			/// </returns>
			virtual int ReadProperty(std::string propName, Reader &reader);

			/// <summary>
			/// Saves the complete state of this Slice to an output stream for later recreation with Create(Reader &reader).
			/// </summary>
			/// <param name="writer">A Writer that the Slice will save itself with. Anything below 0 is an error signal.</param>
			/// <returns>An error return value signaling sucess or any particular failure.</returns>
			virtual int Save(Writer &writer) const;
#pragma endregion

#pragma region Getters and Setters
			/// <summary>
			/// Gets the class name of Slice.
			/// </summary>
			/// <returns>The class name of Slice.</returns>
			const std::string &GetClassName() const { return m_sClassName; }

			/// <summary>
			/// Gets the type of this Slice.
			/// </summary>
			/// <returns>The type of this Slice.</returns>
			const PieSliceIndex &GetType() const { return m_SliceType; }

			/// <summary>
			/// Gets the description this Slice shows when selected in its pie menu.
			/// </summary>
			/// <returns>The description of this Slice.</returns>
			const std::string &GetDescription() const { return m_Description; }

			/// <summary>
			/// Gets the direction this Slice is set to be at in its pie menu.
			/// </summary>
			/// <returns>The direction of this Slice.</returns>
			const SliceDirection &GetDirection() const { return m_Direction; }

			/// <summary>
			/// Sets the direction this Slice should be at in its pie menu.
			/// </summary>
			/// <param name="direction">The direction to set for the Slice.</param>
			void SetDirection(SliceDirection direction) { m_Direction = direction; }

			/// <summary>
			/// Gets whether or not this Slice is enabled.
			/// </summary>
			/// <returns>Whether or not this Slice is enabled.</returns>
			bool IsEnabled() const { return m_Enabled; }

			/// <summary>
			/// Sets whether or not this Slice should be enabled.
			/// </summary>
			/// <param name="enabled">Whether or not this Slice should be enabled.</param>
			void SetEnabled(bool enabled) { m_Enabled = enabled; }

			/// <summary>
			/// Gets the icon for this Slice.
			/// </summary>
			/// <returns>The icon for this Slice.</returns>
			const Icon &GetIcon() const { return m_Icon; }

			/// <summary>
			/// Gets the start angle this Slice's area is set to be at in its pie menu.
			/// </summary>
			/// <returns>The start angle of this Slice's area.</returns>
			float GetAreaStart() const { return m_AreaStart; }

			/// <summary>
			/// Sets the start angle this Slice's area should be at in its pie menu.
			/// </summary>
			/// <param name="areaStart">The start angle to set for the Slice's area.</param>
			void SetAreaStart(float areaStart) { m_AreaStart = areaStart; }

			/// <summary>
			/// Gets the arc length this of this Slice's area.
			/// </summary>
			/// <returns>The arc length of this Slice's area.</returns>
			float GetAreaArc() const { return m_AreaArc; }

			/// <summary>
			/// Sets the arc length of this Slice's area.
			/// </summary>
			/// <param name="areaArc">The arc length to set for the Slice's area.</param>
			void SetAreaArc(float areaArc) { m_AreaArc = areaArc; }

			/// <summary>
			/// Gets the mid angle this Slice's area is set to be at in its pie menu.
			/// </summary>
			/// <returns>The mid angle of this Slice's area.</returns>
			float GetMidAngle() const { return m_MidAngle; }

			/// <summary>
			/// Sets the mid angle this Slice's area should be at in its pie menu.
			/// </summary>
			/// <param name="midAngle">The mid angle to set for the Slice's area.</param>
			void SetMidAngle(float midAngle) { m_MidAngle = midAngle; }

			/// <summary>
			/// Gets the file path of the scripted file this Slice should run when activated. Empty if it's not a scripted type. Empty if its SliceType isn't scripted.
			/// </summary>
			/// <returns>The file path to the script file this Slice should load when activated.</returns>
			const std::string &GetScriptPath() const { return m_ScriptPath; }

			/// <summary>
			/// Gets the name of the Lua function to run when this Slice is activated as a scripted pie menu option. Empty if its SliceType isn't scripted.
			/// </summary>
			/// <returns>The Lua function name this Slice should execute when activated.</returns>
			const std::string &GetFunctionName() const { return m_FunctionName; }
#pragma endregion

		private:
			static const std::string m_sClassName; //!< ClassName for Slice.

			PieMenuGUI::PieSliceIndex m_SliceType; //!< The Slice type, also serves as icon index.
			std::string m_Description; //!< Description of what this slice option does.
			SliceDirection m_Direction; //!< The desired direction/location of this on the pie menu.
			bool m_Enabled; //!< Whether this slice is enabled or disabled and greyed out.
			Icon m_Icon; //!< The icon of this pie slice.

			float m_AreaStart; //!< The start angle of this' area on the pie menu, counted in radians from straight out right and going counter clockwise.
			float m_AreaArc; //!< The arc length of the slice area, so that the icon should be drawn at the areastart + halfway of this.
			float m_MidAngle; //!< Mid angle, basically m_AreaStart + (m_AreaArc / 2).

			std::string m_ScriptPath; //!< Path to the script file this should run if this is a scripted selection.
			std::string m_FunctionName; //!< Name of the function in the script that this should run if a scripted pie option.
			
			/// <summary>
			/// Clears all the member variables of this Slice, effectively resetting the members of this abstraction level only.
			/// </summary>
			void Clear();
		};
#pragma endregion

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
		int Create(Controller *controller, Actor *focusActor = 0);
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Destructor method used to clean up a PieMenuGUI object before deletion from system memory.
		/// </summary>
		virtual ~PieMenuGUI() { Destroy(); }

		/// <summary>
		/// Destroys and resets (through Clear()) the PieMenuGUI object.
		/// </summary>
		void Destroy() { destroy_bitmap(m_BGBitmap); destroy_bitmap(m_EquippedItemBGBitmap); Clear(); }

		/// <summary>
		/// Resets the entire PieMenuGUI, including its inherited members, to their default settings or values.
		/// </summary>
		virtual void Reset() { Clear(); }
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
		const Vector &GetPos() const { return m_CenterPos; }

		/// <summary>
		/// Sets the absolute center position of this in the scene.
		/// </summary>
		/// <param name="newPos">A Vector describing the current absolute position in pixels, in the scene.</param>
		void SetPos(const Vector &newPos) { m_CenterPos = newPos; }

		/// <summary>
		/// Gets the the command issued by this menu in the last update, i.e. the Slice type of the currently activated Slice or None if no slice was activated.
		/// </summary>
		/// <returns>The Slice type which has been picked activated, or None if none has been picked. See the PieSliceIndex enum for Slice types.</returns>
		int GetPieCommand() const { return m_ActivatedSlice == nullptr ? PSI_NONE : m_ActivatedSlice->GetType(); }
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
		bool AddSlice(Slice &newSlice, bool takeAnyFreeCardinal = false);

		/// <summary>
		/// Gets the current Slices in the menu.
		/// </summary>
		std::vector<Slice *> GetCurrentSlices() const { return m_CurrentSlices; };

		/// <summary>
		/// Makes sure all currently added Slices' areas are set up correctly position and sizewise on the pie menu circle.
		/// </summary>
		void RealignSlices();

		/// <summary>
		/// Gets a specific Slice based on an angle on the pie menu.
		/// </summary>
		/// <param name="angle">An angle on the circle, in radins, CCW from straight out right.</param>
		/// <returns>The Slice which exists on that angle, if any. 0 if not. Ownership is NOT transferred!</returns>
		const Slice *GetSliceOnAngle(float angle) const;

		/// <summary>
		/// Reports whether and which Slice has been activated by the player. This may happen even though the player isn't done with the pie menu.
		/// </summary>
		/// <returns>The Slice which has been picked by the player, if any. 0 if not. Ownership is NOT transferred!</returns>
		const Slice *SliceActivated() const { return m_ActivatedSlice; }
#pragma endregion

#pragma region Lua Slice Handling
		/// <summary>
		/// Adds a Slice to the map of custom Lua slices.
		/// </summary>
		/// <param name="newSlice">The Slice to add to the map of custom Lua slices.</param>
		static void StoreCustomLuaSlice(Slice newSlice) { } //s_AllCustomLuaSlices[newSlice.GetDescription() + "::" + newSlice.GetFunctionName()] = newSlice; }

		/// <summary>
		/// Adds a Slice to the menu, searching by description and functionName for a slice defined in INI. Should only be used by Lua.
		/// </summary>
		/// <param name="description">The description to search for.</param>
		/// <param name="functionName">The functionName to search for.</param>
		/// <param name="direction">The direction the Slice should be added at.</param>
		/// <param name="isEnabled">Whether the Slice should be enabled or disabled.</param>
		/// <returns>Whether or not the Slice was succesfully added.</returns>
		bool AddSliceLua(const std::string &description, const std::string &functionName, PieMenuGUI::Slice::SliceDirection direction, bool isEnabled);

		/// <summary>
		/// Removes a Slice from the menu, searching for it by description and functionName. Should only be used by Lua.
		/// </summary>
		/// <param name="description">The description to search for.</param>
		/// <param name="functionName">The functionName to search for.</param>
		/// <returns>The Slice that was removed.</returns>
		Slice RemoveSliceLua(const std::string &description, const std::string &functionName);

		/// <summary>
		/// Alters a Slice in the menu, searching for it by description and functionName, and setting its direction and enabled status according to the given arguments. Should only be used by Lua.
		/// </summary>
		/// <param name="description">The description to search for.</param>
		/// <param name="functionName">The functionName to search for.</param>
		/// <param name="direction">The direction the Slice should be moved to.</param>
		/// <param name="isEnabled">Whether the Slice should be enabled or disabled.</param>
		void AlterSliceLua(const std::string &description, const std::string &functionName, PieMenuGUI::Slice::SliceDirection direction, bool isEnabled);
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

		static std::unordered_map<std::string, Slice> s_AllCustomLuaSlices; //<! All Slices ever added to this pie-menu, serves as directory of Slices available to add.
		static BITMAP *s_Cursor; //<! A static pointer to the bitmap to use as the cursor in any menu.
		static const int s_EnablingDelay = 50; //<! Time in ms for how long it takes to enable/disable.
		
		Timer m_EnablingTimer; //<! Timer for the appear and disappear animations.
		Timer m_HoverTimer; //<! Timer to measure how long to hold a hovered over slice.
		
		Controller *m_Controller; //<! The Controller which controls this menu. Not owned.
		Actor *m_Actor; //<! The actor that this menu is attached to and getting some options from.
		Actor *m_LastKnownActor; //<! This actor pointer is not cleared every time, I'm not touching the original to not ruin something. ?????
		
		PieEnabled m_PieEnabled; //<! The enabled state of the menu.
		Vector m_CenterPos; //<! The center position of this in the scene.
		
		const Slice *m_HoveredSlice; //<! The Slice currently being hovered over.
		const Slice *m_ActivatedSlice; //<! The currently activated Slice, if there is one, or 0 if there's not.
		const Slice *m_AlreadyActivatedSlice; //<! The Slice that was most recently activated by pressing Primary. Used to avoid duplicate activation when releasing the pie menu.
		
		//<! The cardinal axis slices, owned here
		Slice m_UpSlice;
		Slice m_RightSlice;
		Slice m_DownSlice;
		Slice m_LeftSlice; 

		//<! The slices between the cardinal axes, owned here
		std::list<Slice> m_UpRightSlices;
		std::list<Slice> m_UpLeftSlices;
		std::list<Slice> m_DownLeftSlices;
		std::list<Slice> m_DownRightSlices;

		std::vector<Slice *> m_CurrentSlices; //<! All the Slices, in order and aligned, not owned here, just pointing to the ones above.
		int m_SliceGroupCount; //<! How many groups there currently are in the menu.

		bool m_Wobbling; //<! Special mode where the menu circle expands and contracts continuously, while being effectively disabled.
		bool m_Freeze; //<! Special mode where the menu circle is frozen at the current m_InnerRadius.

		int m_InnerRadius; //<! The current radius of the innermost circle of the pie menu, in pixels.
		int m_EnabledRadius; //<! When fully enabled, the inner radius is this, in pixels.
		int m_Thickness; //<! The thickness of the pie menu circle, in pixels.
		float m_CursorAngle; //<! Position of the cursor on the circle, in radians, counterclockwise from straight out to rhe right.
		
		BITMAP *m_BGBitmap; //<! The intermediary bitmap used to first draw the menu background, which will be blitted to the final draw target surface.
		bool m_RedrawBG; //<! Whether we need to redraw the BG bitmap.


		//TODO all this should be in another GUI
		static const size_t s_InventoryIconsPerRow = 5; //<! The number of inventory icons in each row of the drawn inventory.
		static const size_t s_InventoryIconSpacer = 2; //<! The space in between each icon in the drawn inventory.
		static const size_t s_InventoryIconMaxWidth = 50; //<! The max width of inventory icons in the inventory display.
		static const size_t s_InventoryIconMaxHeight = 25; //<! The max height of inventory icons in the inventory display.
		static const size_t s_InventoryEquippedIconHighlightWidth = 4; //<! The width of the highlight around the equipped inventory item.

		GUIFont *m_InventoryMassDisplayFont; //<! The font used to display the mass of each inventory object.
		BITMAP *m_EquippedItemBGBitmap; //<! The intermediary bitmap used to first draw the equipped item background, which will be blitted to the final draw target surface.

		/// <summary>
		/// Sets a slice to be selected.
		/// </summary>
		/// <param name="sliceToSelect">The slice to be selected. Has to be a slice currently in this menu.</param>
		/// <param name="moveCursorToSlice">Whether to also move the cursor to the center of the newly selected slice. Defaults to false.</param>
		/// <returns>Whether or not this resulted in a different slice being selected. Also returns false if a null slice was passed in.</returns>
		bool SelectSlice(const Slice *sliceToSelect, bool moveCursorToSlice = false);

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

		//TODO This and the three methods below it should be moved out into GameActivity or some other standalone GUI.
		/// <summary>
		/// Handles drawing actor inventory above the pie menu.
		/// </summary>
		/// <param name="targetBitmap">A pointer to the BITMAP to draw on. Generally a screen BITMAP.</param>
		/// <param name="drawPos">The seam corrected position at which the PieMenu is being drawn.</param>
		void DrawActorInventory(BITMAP *targetBitmap, const Vector &drawPos) const;

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
		PieMenuGUI(const PieMenuGUI &reference) {}
		PieMenuGUI &operator=(const PieMenuGUI &rhs) { return *this; }
	};
}
#endif