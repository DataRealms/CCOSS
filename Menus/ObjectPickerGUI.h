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
		/// Selects the specified group from the groups list and updates the objects list to show the group's objects.
		/// </summary>
		/// <param name="groupName">The name of the group to select in the picker.</param>
		/// <returns>Whether the group was found and switched to successfully.</returns>
		bool SelectSpecificGroup(const std::string_view &groupName);

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
		/// Sets whether a DataModule shown in the item menu should be expanded or not.
		/// </summary>
		/// <param name="whichModule">The module ID to set as expanded.</param>
		/// <param name="expanded">Whether should be expanded or not.</param>
		void SetModuleExpanded(int whichModule, bool expanded = true);
#pragma endregion

#pragma region Object Picking Handling
		/// <summary>
		/// Gets the next object in the objects list, even if the picker is disabled.
		/// </summary>
		/// <returns>The next object in the picker list, looping around if necessary. 0 if no object can be selected.</returns>
		const SceneObject * GetNextObject() { return GetNextOrPrevObject(false); }

		/// <summary>
		/// Gets the previous object in the objects list, even if the picker is disabled.
		/// </summary>
		/// <returns>The previous object in the picker list, looping around if necessary. 0 if no object can be selected.</returns>
		const SceneObject * GetPrevObject() { return GetNextOrPrevObject(true); }

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

		int m_SelectedGroupIndex; //!< Which Group in the groups list box we have selected.
		int m_SelectedObjectIndex; //!< Which object in the objects list box we have selected.
		const SceneObject *m_PickedObject; //!< Currently picked object. This is 0 until the user actually picks something, not just has the cursor over it. Not owned by this.

		Timer m_RepeatStartTimer; //!< Measures the time to when to start repeating inputs when they're held down.
		Timer m_RepeatTimer; //!< Measures the interval between input repeats.

		std::vector<bool> m_ExpandedModules; //!< The modules that have been expanded in the item list.

#pragma region List Handling
		/// <summary>
		/// Sets the currently focused list in the picker. For list item highlighting and non-mouse input handling.
		/// </summary>
		/// <param name="listToFocusOn">The list to focus on. See PickerFocus enumeration.</param>
		/// <returns>Whether a focus change was made or not.</returns>
		bool SetListFocus(PickerFocus listToFocusOn);

		/// <summary>
		/// 
		/// </summary>
		/// <param name="selectNext"></param>
		/// <param name="selectPrev"></param>
		void SelectGroup(bool selectNext = false, bool selectPrev = false);

		/// <summary>
		/// Gets the next or previous item in the objects list and sets it as the current pick, even if the picker is disabled.
		/// </summary>
		/// <param name="getPrev">Whether to get the previous object or the next one. Gets the next object by default.</param>
		/// <returns>The next object in the picker list, looping around if necessary. 0 if no object can be selected.</returns>
		const SceneObject * GetNextOrPrevObject(bool getPrev = false);

		/// <summary>
		/// Displays the popup box with the description of the selected item in the objects list.
		/// </summary>
		void ShowDescriptionPopupBox();

		/// <summary>
		/// Adds all groups with a specific type already defined in PresetMan to the current objects list.
		/// </summary>
		void UpdateGroupsList();

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