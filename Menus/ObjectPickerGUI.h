#ifndef _OBJECTPICKERGUI_
#define _OBJECTPICKERGUI_

#include "Controller.h"

namespace RTE {

	class Controller;
	class GUIScreen;
	class GUIInput;
	class GUIControlManager;
	class GUICollectionBox;
	class GUIListBox;
	class GUILabel;
	class SceneObject;

	/// <summary>
	/// A GUI for picking object instances for placement to the Scene in Cortex Command editors.
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
		/// Destructor method used to clean up a ObjectPickerGUI object before deletion from system memory.
		/// </summary>
		~ObjectPickerGUI() { Destroy(); }

		/// <summary>
		/// Destroys and resets (through Clear()) the ObjectPickerGUI object.
		/// </summary>
		void Destroy();
#pragma endregion

#pragma region Getters and Setters
		/// <summary>
		/// Reports whether the menu is at all visible or not.
		/// </summary>
		/// <returns></returns>
		bool IsVisible() const { return m_PickerEnabled != PickerState::Disabled; }

		/// <summary>
		/// Reports whether the menu is enabled or not.
		/// </summary>
		/// <returns></returns>
		bool IsEnabled() const { return m_PickerEnabled == PickerState::Enabled || m_PickerEnabled == PickerState::Enabling; }

		/// <summary>
		/// Enables or disables the menu. This will animate it in and out of view.
		/// </summary>
		/// <param name="enable">Whether to enable or disable the menu.</param>
		void SetEnabled(bool enable = true);

		/// <summary>
		/// Sets the controller used by this. The ownership of the controller is NOT transferred!
		/// </summary>
		/// <param name="controller">The new controller for this menu. Ownership is NOT transferred</param>
		void SetController(Controller *controller) { m_Controller = controller; }

		/// <summary>
		/// Sets where on the screen that this GUI is being drawn to. If upper left corner, then 0, 0. This will affect the way the mouse is positioned etc.
		/// </summary>
		/// <param name="newPosX">The new X position of this entire GUI on the screen.</param>
		/// <param name="newPosY">The new Y position of this entire GUI on the screen.</param>
		void SetPosOnScreen(int newPosX, int newPosY);

		/// <summary>
		/// Sets which DataModule space to be picking objects from. If -1, then let the player pick from all loaded modules.
		/// </summary>
		/// <param name="moduleSpaceID">The ID of the module to let the player pick objects from. All official modules' objects will always be presented, in addition to the one passed in here.</param>
		void SetModuleSpace(int moduleSpaceID = -1);

		/// <summary>
		/// Sets which DataModule space to be picking objects from. If -1, then let the player pick from all loaded modules.
		/// </summary>
		/// <param name="showType">The ID of the module to let the player pick objects from. All official modules' objects will always be presented, in addition to the one passed in here.</param>
		void ShowOnlyType(const std::string_view &showType = "All") { m_ShowType = showType; UpdateGroupsList(); }

		/// <summary>
		/// Makes a group of a specific name show up in the picker, IF that group is currently shown!
		/// </summary>
		/// <param name="groupName">The name of the group to show in the picker.</param>
		/// <returns>Whether the group was found and switched to successfully.</returns>
		bool ShowSpecificGroup(const std::string_view &groupName);

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
		/// Sets whether a data module shown in the item menu should be expanded or not.
		/// </summary>
		/// <param name="whichModule">The module ID to set as expanded.</param>
		/// <param name="expanded">Whether should be expanded or not.</param>
		void SetModuleExpanded(int whichModule, bool expanded = true);

		/// <summary>
		/// Gets the next object in the objects list, even if the picker is disabled.
		/// </summary>
		/// <returns>The next object in the picker list, looping around if necessary. 0 if no object can be selected.</returns>
		const SceneObject * GetNextObject();

		/// <summary>
		/// Gets the previous object in the objects list, even if the picker is disabled.
		/// </summary>
		/// <returns>The previous object in the picker list, looping around if necessary. 0 if no object can be selected.</returns>
		const SceneObject * GetPrevObject();

		/// <summary>
		/// Reports whether and which Object has been picked by the player. This may happen even though the player isn't done with the picker, like if a different object is picked each time the user selects something else in the objects list.
		/// </summary>
		/// <returns>Whether an object has been picked by the player. Nullptr if not. Ownership is NOT transferred!</returns>
		const SceneObject * ObjectPicked() const { return m_PickedObject; }

		/// <summary>
		/// Reports whether the user has finished using the picker, and the final picked Object is returned.
		/// </summary>
		/// <returns>Whether an object has been positively and finally picked by the player. Nullptr if not. Ownership is NOT transferred!</returns>
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

	protected:

		static BITMAP *s_Cursor; //!< The cursor image shared by all pickers.
		Vector m_CursorPos; //!< Screen position of the cursor.

		GUIScreen *m_GUIScreen; //!< GUI Screen for use by the in-game GUI.
		GUIInput *m_GUIInput; //!< Input controller.
		GUIControlManager *m_GUIController; //!< The control manager which holds all the controls.
		GUICollectionBox *m_ParentBox; //!< Collection box of the picker GUI.
		GUICollectionBox *m_PopupBox; //!< Collection box of the buy popups that contain information about items.
		GUILabel *m_PopupText; //!< Label displaying the item popup description.
		GUIListBox *m_GroupsList; //!< The ListBox which lists all the groups.
		GUIListBox *m_ObjectsList; //!< The ListBox which lists all the objects in the currently selected group.

		Controller *m_Controller; //!< Controller which controls this menu. Not owned.

		int m_PickerEnabled; //!< Visibility state of the object picker.
		int m_PickerFocus; //!< Focus state.
		int m_FocusChange; //!< Focus change direction - 0 if none, negative if back, positive if forward.

		float m_MenuSpeed; //!< Speed at which the menus appear and disappear.

		int m_ModuleSpaceID; //!< The DataModule ID of the non-official module that this picker should be restricted to, in addition to all the official modules as well. If -1, the picker will be able to pick from ALL loaded DataModules.
		std::string m_ShowType; //!< Only show objects of this type. Empty string or "All" will show objects of all types.

		int m_SelectedGroupIndex; //!< Which Group in the groups list box we have selected.
		int m_SelectedObjectIndex; //!< Which object in the Objects list box we have selected.
		int m_LastHoveredMouseIndex; //!< Which object was last hovered over by the mouse in the objects list.

		int m_NativeTechModule; //!< The ID of the DataModule that contains the native Tech of the Player using this menu.
		float m_ForeignCostMult; //!< The multiplier of costs of any foreign tech items.

		const SceneObject *m_PickedObject; //!< Currently picked object. This is 0 until the user actually picks something, not just has the cursor over it. Not owned by this.

		Timer m_RepeatStartTimer; //!< Measures the time to when to start repeating inputs when they're held down.
		Timer m_RepeatTimer; //!< Measures the interval between input repeats.

		bool *m_ExpandedModules; //!< The modules that have been expanded in the item list.

		/// <summary>
		/// Adds all groups with a specific type already defined in PresetMan to the current Objects list.
		/// </summary>
		void UpdateGroupsList();

		/// <summary>
		/// Adds all objects of the currently selected group to the Objects list.
		/// </summary>
		/// <param name="selectTop">Whether to reset the selection to the top of the list when we're done updating this.</param>
		void UpdateObjectsList(bool selectTop = true);

	private:

		/// <summary>
		/// Enumeration for ObjectPicker states when enabling/disabling the ObjectPicker.
		/// </summary>
		enum PickerState { Enabling, Enabled, Disabling, Disabled };

		/// <summary>
		/// Enumeration for the ObjectPicker columns ListBox focus states.
		/// </summary>
		enum PickerFocus { Groups, Objects, FocusCount };

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