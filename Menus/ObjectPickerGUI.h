#ifndef _OBJECTPICKERGUI_
#define _OBJECTPICKERGUI_

#include "Controller.h"

#include "GUI.h"
#include "GUIControlManager.h"
#include "AllegroScreen.h"
#include "AllegroInput.h"

namespace RTE {

	class Controller;
	class GUICollectionBox;
	class GUIListBox;
	class GUILabel;
	class SceneObject;

	/// <summary>
	/// A GUI for picking object instances for placement to the Scene in various editors.
	/// </summary>
	class ObjectPickerGUI {

	public:

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a ObjectPickerGUI object in system memory. Create() should be called before using the object.
		/// </summary>
		ObjectPickerGUI() { Clear(); }

		/// <summary>
		/// Makes the ObjectPickerGUI object ready for use.
		/// </summary>
		/// <param name="controller">A pointer to a Controller which will control this Menu. Ownership is NOT transferred!</param>
		/// <param name="whichModuleSpace">Which DataModule space to be picking from. -1 means pick from all objects loaded in all DataModules.</param>
		/// <param name="onlyOfType">Which lowest common denominator type to be showing.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create(Controller *controller, int whichModuleSpace = -1, const std::string_view &onlyOfType = "All");
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Resets the entire ObjectPickerGUI, including its inherited members, to their default settings or values.
		/// </summary>
		void Reset() { Clear(); }
#pragma endregion

#pragma region Getters and Setters
		/// <summary>
		/// Reports whether the menu is at all visible or not.
		/// </summary>
		/// <returns></returns>
		bool IsVisible() const { return m_PickerState != PickerState::Disabled; }

		/// <summary>
		/// Reports whether the menu is enabled or not.
		/// </summary>
		/// <returns></returns>
		bool IsEnabled() const { return m_PickerState == PickerState::Enabled || m_PickerState == PickerState::Enabling; }

		/// <summary>
		/// Enables or disables the menu. This will animate it in and out of view.
		/// </summary>
		/// <param name="enable">Whether to enable or disable the menu.</param>
		void SetEnabled(bool enable = true);

		/// <summary>
		/// Sets the controller used by this. The ownership of the controller is NOT transferred!
		/// </summary>
		/// <param name="controller">The new controller for this menu. Ownership is NOT transferred!</param>
		void SetController(Controller *controller) { m_Controller = controller; }

		/// <summary>
		/// Sets where on the screen that this GUI is being drawn to. If upper left corner, then 0, 0. This will affect the way the mouse is positioned etc.
		/// </summary>
		/// <param name="newPosX">The new X position of this entire GUI on the screen.</param>
		/// <param name="newPosY">The new Y position of this entire GUI on the screen.</param>
		void SetPosOnScreen(int newPosX, int newPosY) const { m_GUIControlManager->SetPosOnScreen(newPosX, newPosY); }

		/// <summary>
		/// Sets which DataModule space to be picking objects from. If -1, then let the player pick from all loaded modules.
		/// </summary>
		/// <param name="newModuleSpaceID">The ID of the module to let the player pick objects from. All official module objects will always be presented, in addition to the one passed in here.</param>
		void SetModuleSpace(int newModuleSpaceID = -1) { if (newModuleSpaceID != m_ModuleSpaceID) { m_ModuleSpaceID = newModuleSpaceID; UpdateGroupsList(); } }

		/// <summary>
		/// Sets which DataModule space to be picking objects from. If -1, then let the player pick from all loaded modules.
		/// </summary>
		/// <param name="showType">The ID of the module to let the player pick objects from. All official module objects will always be presented, in addition to the one passed in here.</param>
		void ShowOnlyType(const std::string_view &showType = "All") { m_ShowType = showType; UpdateGroupsList(); }

		/// <summary>
		/// Sets which DataModule ID should be treated as the native tech of the user of this menu.
		/// </summary>
		/// <param name="whichModule">The module ID to set as the native one. 0 means everything is native.</param>
		void SetNativeTechModule(int whichModule);

		/// <summary>
		/// Sets the multiplier of the cost of any foreign Tech items.
		/// </summary>
		/// <param name="newMultiplier">The scalar multiplier of the costs of foreign Tech items.</param>
		void SetForeignCostMultiplier(float newMultiplier) { m_ForeignCostMult = newMultiplier; }

		/// <summary>
		/// Selects the specified group name in the groups list and updates the objects list to show the group's objects.
		/// </summary>
		/// <param name="groupName">The name of the group to select in the picker.</param>
		/// <returns>Whether the group was found and switched to successfully.</returns>
		bool SelectGroupByName(const std::string_view &groupName);
#pragma endregion

#pragma region Object Picking Handling
		/// <summary>
		/// Gets the next object in the objects list, even if the picker is disabled.
		/// </summary>
		/// <returns>The next object in the picker list, looping around if necessary. If the next object is an invalid SceneObject (e.g. a module subgroup) then this will recurse until a valid object is found.</returns>
		const SceneObject * GetNextObject() { SelectNextOrPrevObject(false); const SceneObject *object = GetSelectedObject(); return object ? object : GetNextObject(); }

		/// <summary>
		/// Gets the previous object in the objects list, even if the picker is disabled.
		/// </summary>
		/// <returns>The previous object in the picker list, looping around if necessary. If the previous object is an invalid SceneObject (e.g. a module subgroup) then this will recurse until a valid object is found.</returns>
		const SceneObject * GetPrevObject() { SelectNextOrPrevObject(true); const SceneObject *object = GetSelectedObject(); return object ? object : GetPrevObject(); }

		/// <summary>
		/// Reports whether and which object has been picked by the player. There may be an object picked even when the player is not done with the picker, as scrolling through objects (but not mousing over them) picks them.
		/// </summary>
		/// <returns>A pointer to the object picked by the player, or nullptr if none was picked. Ownership is NOT transferred!</returns>
		const SceneObject * ObjectPicked() const { return m_PickedObject; }

		/// <summary>
		/// Reports whether the player has finished using the picker, and the final picked object is returned.
		/// </summary>
		/// <returns>The object the player picked before they closed the picker, or nullptr if none was picked. Ownership is NOT transferred!< / returns>
		const SceneObject * DonePicking() const { return (!IsEnabled() && m_PickedObject) ? m_PickedObject : nullptr; }
#pragma endregion

#pragma region Concrete Methods
		/// <summary>
		/// Updates the state of this ObjectPickerGUI each frame.
		/// </summary>
		void Update();

		/// <summary>
		/// Draws the ObjectPickerGUI to the specified BITMAP.
		/// </summary>
		/// <param name="drawBitmap">The BITMAP to draw on.</param>
		void Draw(BITMAP *drawBitmap) const;
#pragma endregion

	private:

		/// <summary>
		/// Enumeration for ObjectPicker states when enabling/disabling the ObjectPicker.
		/// </summary>
		enum class PickerState { Enabling, Enabled, Disabling, Disabled };

		/// <summary>
		/// Enumeration for the ObjectPicker columns ListBox focus states.
		/// </summary>
		enum class PickerFocus { GroupList, ObjectList };

		static BITMAP *s_Cursor; //!< The cursor image shared by all pickers.
		Vector m_CursorPos; //!< Screen position of the cursor.

		std::unique_ptr<AllegroScreen> m_GUIScreen; //!< The GUIScreen interface that will be used by this ObjectPickerGUI's GUIControlManager.
		std::unique_ptr<AllegroInput> m_GUIInput; //!< The GUIInput interface that will be used by this ObjectPickerGUI's GUIControlManager.
		std::unique_ptr<GUIControlManager> m_GUIControlManager; //!< The control manager which holds all the controls.
		GUICollectionBox *m_ParentBox; //!< Collection box of the picker GUI.
		GUICollectionBox *m_PopupBox; //!< Collection box of the buy popups that contain information about items.
		GUILabel *m_PopupText; //!< Label displaying the item popup description.
		GUIListBox *m_GroupsList; //!< The ListBox which lists all the groups.
		GUIListBox *m_ObjectsList; //!< The ListBox which lists all the objects in the currently selected group.

		Controller *m_Controller; //!< Controller which controls this menu. Not owned.

		PickerState m_PickerState; //!< Visibility state of the object picker.
		PickerFocus m_PickerFocus; //!< The currently focused list in the Picker.
		float m_OpenCloseSpeed; //!< Speed at which the picker appears and disappears.

		int m_ModuleSpaceID; //!< The DataModule ID of the non-official module that this picker should be restricted to, in addition to all the official modules as well. If -1, the picker will be able to pick from ALL loaded DataModules.
		std::string m_ShowType; //!< Only show objects of this type. Empty string or "All" will show objects of all types.
		int m_NativeTechModuleID; //!< The ID of the DataModule that contains the native Tech of the Player using this menu.
		float m_ForeignCostMult; //!< The multiplier of costs of any foreign tech items.

		int m_ShownGroupIndex; //!< Which group in the groups list is currently showing it's objects list.
		int m_SelectedGroupIndex; //!< Which group in the groups list box we have selected.
		int m_SelectedObjectIndex; //!< Which object in the objects list box we have selected.
		const SceneObject *m_PickedObject; //!< Currently picked object. This may be a valid object even if the player is not done with the picker, as scrolling through objects (but not mousing over them) picks them. Not owned by this.

		Timer m_RepeatStartTimer; //!< Measures the time to when to start repeating inputs when they're held down.
		Timer m_RepeatTimer; //!< Measures the interval between input repeats.

		std::vector<bool> m_ExpandedModules; //!< The modules that have been expanded in the item list.

#pragma region General List Handling
		/// <summary>
		/// Sets the currently focused list in the picker. For list item highlighting and non-mouse input handling.
		/// </summary>
		/// <param name="listToFocusOn">The list to focus on. See PickerFocus enumeration.</param>
		/// <returns>Whether a focus change was made or not.</returns>
		bool SetListFocus(PickerFocus listToFocusOn);
#pragma endregion

#pragma region Group List Handling
		/// <summary>
		/// Selects the specified group index in the groups list and updates the objects list to show the group's objects.
		/// </summary>
		/// <param name="groupIndex">The group index to select. Removing any selection (with index -1) will be overridden and the currently shown group will be selected instead.</param>
		/// <param name="updateObjectsList">Whether to update the objects list after making the selection or not.</param>
		void SelectGroupByIndex(int groupIndex, bool updateObjectsList = true);

		/// <summary>
		/// Selects the next or previous group from the one that is currently selected in the groups list.
		/// </summary>
		/// <param name="selectPrev">Whether to select the previous group. Next group will be selected by default.</param>
		void SelectNextOrPrevGroup(bool selectPrev = false);

		/// <summary>
		/// Adds all groups with a specific type already defined in PresetMan that are within the set ModuleSpaceID and aren't empty to the current groups list.
		/// </summary>
		void UpdateGroupsList();
#pragma endregion

#pragma region Object List Handling
		/// <summary>
		/// Gets the SceneObject from the currently selected index in the objects list. Ownership is NOT transferred!
		/// </summary>
		/// <returns>The SceneObject of the currently selected index in the objects list. Nullptr if no valid object is selected (eg. a module subgroup).</returns>
		const SceneObject * GetSelectedObject();

		/// <summary>
		/// Selects the specified object index in the objects list.
		/// </summary>
		/// <param name="objectIndex">The object index to select.</param>
		/// <param name="playSelectionSound">Whether to play the selection change sound or not.</param>
		void SelectObjectByIndex(int objectIndex, bool playSelectionSound = true);

		/// <summary>
		/// Selects the next or previous object from the one that is currently selected in the objects list.
		/// </summary>
		/// <param name="getPrev">Whether to select the previous object. Next object will be selected by default.</param>
		void SelectNextOrPrevObject(bool getPrev = false);

		/// <summary>
		/// Add the expandable DataModule group separator in the objects list with appropriate name and icon.
		/// </summary>
		/// <param name="moduleID">The DataModule ID to add group separator for.</param>
		void AddObjectsListModuleGroup(int moduleID);

		/// <summary>
		/// Sets whether a DataModule group separator shown in the objects list should be expanded or collapsed.
		/// </summary>
		/// <param name="moduleID">The module ID to set as expanded or collapsed.</param>
		/// <param name="expanded">Whether should be expanded or not.</param>
		void SetObjectsListModuleGroupExpanded(int moduleID, bool expanded = true);

		/// <summary>
		/// Toggles the expansion/collapse of a DataModule group separator in the objects list.
		/// </summary>
		/// <param name="moduleID">The module ID to toggle for.</param>
		void ToggleObjectsListModuleGroupExpansion(int moduleID);

		/// <summary>
		/// Displays the popup box with the description of the selected item in the objects list.
		/// </summary>
		void ShowDescriptionPopupBox();

		/// <summary>
		/// Adds all objects of the currently selected group to the objects list.
		/// </summary>
		/// <param name="selectTop">Whether to reset the selection to the top of the list when we're done updating this.</param>
		void UpdateObjectsList(bool selectTop = true);
#pragma endregion

#pragma region Update Breakdown
		/// <summary>
		/// Player input handling for all types of input devices.
		/// </summary>
		/// <returns>True if the picker was set to be closed or by making a selection from the objects list.</returns>
		bool HandleInput();

		/// <summary>
		/// Player mouse input event handling of the GUIControls of this ObjectPickerGUI.
		/// </summary>
		/// <returns>True if the picker was set to be closed by clicking off it or by making a selection from the objects list.</returns>
		bool HandleMouseEvents();

		/// <summary>
		/// Open/close animation handling and GUI element enabling/disabling.
		/// </summary>
		void AnimateOpenClose();
#pragma endregion

		/// <summary>
		/// Clears all the member variables of this ObjectPickerGUI, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();

		// Disallow the use of some implicit methods.
		ObjectPickerGUI(const ObjectPickerGUI &reference) = delete;
		ObjectPickerGUI & operator=(const ObjectPickerGUI &rhs) = delete;
	};
}
#endif