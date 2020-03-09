#ifndef _GUISOUND_
#define _GUISOUND_

#include "Sound.h"
#include "Singleton.h"

#define g_GUISound GUISound::Instance()

namespace RTE {

	/// <summary>
	/// The singleton loader for all GUI sound effects.
	/// </summary>
	class GUISound : public Singleton<GUISound> {
		
	public:

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a GUISound object in system memory. Create() should be called before using the object.
		/// </summary>
		GUISound() { Clear(); }

		/// <summary>
		/// Creates all the GUI sound effects with Sound::Create and their additional samples with Sound::AddSample and makes the GUISound object ready for use.
		/// </summary>
		/// <returns>An error return value signaling success or any particular failure. Anything other than 0 is an error signal.</returns>
		virtual int Create();
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Destroys and resets (through Clear()) the GUISound object.
		/// </summary>
		void Destroy() { Clear(); }

		/// <summary>
		/// Destructor method used to clean up a GUISound object before deletion from system memory.
		/// </summary>
		virtual ~GUISound() { Destroy(); }
#pragma endregion

#pragma region Getters
		/// <summary>
		/// Gets juicy logo signature jingle Sound.
		/// </summary>
		/// <returns>Juicy logo signature jingle Sound.</returns>
		Sound SplashSound() const { return m_SplashSound; }

		/// <summary>
		/// Gets Sound for enabling menu.
		/// </summary>
		/// <returns>Sound for enabling menu.</returns>
		Sound EnterMenuSound() const { return m_EnterMenuSound; }

		/// <summary>
		/// Gets Sound for disabling menu.
		/// </summary>
		/// <returns>Sound for disabling menu.</returns>
		Sound ExitMenuSound() const { return m_ExitMenuSound; }

		/// <summary>
		/// Gets Sound for changing focus.
		/// </summary>
		/// <returns>Sound for changing focus.</returns>
		Sound FocusChangeSound() const { return m_FocusChangeSound; }

		/// <summary>
		/// Gets Sound for selecting items in list, etc.
		/// </summary>
		/// <returns>Sound for selecting items in list, etc.</returns>
		Sound SelectionChangeSound() const { return m_SelectionChangeSound; }

		/// <summary>
		/// Gets Sound for adding or deleting items in list.
		/// </summary>
		/// <returns>Sound for adding or deleting items in list.</returns>
		Sound ItemChangeSound() const { return m_ItemChangeSound; }

		/// <summary>
		/// Gets Sound for button press.
		/// </summary>
		/// <returns>Sound for button press.</returns>
		Sound ButtonPressSound() const { return m_ButtonPressSound; }

		/// <summary>
		/// Gets Sound for button press of going back button.
		/// </summary>
		/// <returns>Sound for button press of going back button.</returns>
		Sound BackButtonPressSound() const { return m_BackButtonPressSound; }

		/// <summary>
		/// Gets Sound for confirming a selection.
		/// </summary>
		/// <returns>Sound for confirming a selection.</returns>
		Sound ConfirmSound() const { return m_ConfirmSound; }

		/// <summary>
		/// Gets Sound for erroneous input.
		/// </summary>
		/// <returns>Sound for erroneous input.</returns>
		Sound UserErrorSound() const { return m_UserErrorSound; }

		/// <summary>
		/// Gets Sound for testing volume when adjusting volume sliders.
		/// </summary>
		/// <returns>Sound for testing volume when adjusting volume sliders.</returns>
		Sound TestSound() const { return m_TestSound; }

		/// <summary>
		/// Gets Sound for opening pie menu.
		/// </summary>
		/// <returns>Sound for opening pie menu.</returns>
		Sound PieMenuEnterSound() const { return m_PieMenuEnterSound; }

		/// <summary>
		/// Gets Sound for closing pie menu.
		/// </summary>
		/// <returns>Sound for closing pie menu.</returns>
		Sound PieMenuExitSound() const { return m_PieMenuExitSound; }

		/// <summary>
		/// Gets Sound for when PieMenu hover arrow appears or changes slice.
		/// </summary>
		/// <returns>Sound for when PieMenu hover arrow appears or changes slice.</returns>
		Sound HoverChangeSound() const { return m_HoverChangeSound; }

		/// <summary>
		/// Gets Sound for when PieMenu hover arrow appears or changes to a disabled slice.
		/// </summary>
		/// <returns>Sound for when PieMenu hover arrow appears or changes to a disabled slice.</returns>
		Sound HoverDisabledSound() const { return m_HoverDisabledSound; }

		/// <summary>
		/// Gets Sound for picking a valid PieMenu slice.
		/// </summary>
		/// <returns>Sound for picking a valid PieMenu slice.</returns>
		Sound SlicePickedSound() const { return m_SlicePickedSound; }

		/// <summary>
		/// Gets Sound for erroneous input in PieMenu.
		/// </summary>
		/// <returns>Sound for erroneous input in PieMenu.</returns>
		Sound DisabledPickedSound() const { return m_DisabledPickedSound; }

		/// <summary>
		/// Gets Sound for when the funds of a team changes.
		/// </summary>
		/// <returns>Sound for when the funds of a team changes.</returns>
		Sound FundsChangedSound() const { return m_FundsChangedSound; }

		/// <summary>
		/// Gets Sound for switching between regular (non-brain) actors.
		/// </summary>
		/// <returns>Sound for switching between regular (non-brain) actors.</returns>
		Sound ActorSwitchSound() const { return m_ActorSwitchSound; }

		/// <summary>
		/// Gets Sound for switching to the brain shortcut.
		/// </summary>
		/// <returns>Sound for switching to the brain shortcut.</returns>
		Sound BrainSwitchSound() const { return m_BrainSwitchSound; }

		/// <summary>
		/// Gets Sound when camera is traveling between actors.
		/// </summary>
		/// <returns>Sound when camera is traveling between actors.</returns>
		Sound CameraTravelSound() const { return m_CameraTravelSound; }

		/// <summary>
		/// Gets Sound for making an area focus.
		/// </summary>
		/// <returns>Sound for making an area focus.</returns>
		Sound AreaPickedSound() const { return m_AreaPickedSound; }

		/// <summary>
		/// Gets Sound for making an object focus.
		/// </summary>
		/// <returns>Sound for making an object focus.</returns>
		Sound ObjectPickedSound() const { return m_ObjectPickedSound; }

		/// <summary>
		/// Gets Sound for making a purchase.
		/// </summary>
		/// <returns>Sound for making a purchase.</returns>
		Sound PurchaseMadeSound() const { return m_PurchaseMadeSound; }

		/// <summary>
		/// Gets Sound for placement of object to scene.
		/// </summary>
		/// <returns>Sound for placement of object to scene.</returns>
		Sound PlacementBlip() const { return m_PlacementBlip; }

		/// <summary>
		/// Gets Sound for placement of object to scene.
		/// </summary>
		/// <returns>Sound for placement of object to scene.</returns>
		Sound PlacementThud() const { return m_PlacementThud; }

		/// <summary>
		/// Gets Sound for gravely placement of object to scene.
		/// </summary>
		/// <returns>Sound for gravely placement of object to scene.</returns>
		Sound PlacementGravel() const { return m_PlacementGravel; }
#pragma endregion

	protected:

		Sound m_SplashSound; //!< Juicy logo signature jingle sound.

		Sound m_EnterMenuSound; //!< Sound for enabling menu.
		Sound m_ExitMenuSound; //!< Sound for disabling menu.

		Sound m_FocusChangeSound; //!< Sound for changing focus.

		Sound m_SelectionChangeSound; //!< Sound for selecting items in list, etc.
		Sound m_ItemChangeSound; //!< Sound for adding or deleting items in list.

		Sound m_ButtonPressSound; //!< Sound for button press.
		Sound m_BackButtonPressSound; //!< Sound for button press of going back button.
		Sound m_ConfirmSound; //!< Sound for confirming a selection.
		Sound m_UserErrorSound; //!< Sound for erroneous input.
		Sound m_TestSound; //!< Sound for testing volume when adjusting volume sliders.

		Sound m_PieMenuEnterSound; //!< Sound for opening pie menu.
		Sound m_PieMenuExitSound; //!< Sound for closing pie menu.

		Sound m_HoverChangeSound; //!< Sound for when PieMenu hover arrow appears or changes slice.
		Sound m_HoverDisabledSound; //!< Sound for when PieMenu hover arrow appears or changes to a disabled slice.
		Sound m_SlicePickedSound; //!< Sound for picking a valid PieMenu slice.
		Sound m_DisabledPickedSound; //!< Sound for erroneous input in PieMenu.

		Sound m_FundsChangedSound; //!< Sound for when the funds of a team changes.

		Sound m_ActorSwitchSound; //!< Sound for switching between regular (non-brain) actors.
		Sound m_BrainSwitchSound; //!< Sound for switching to the brain shortcut.

		Sound m_CameraTravelSound; //!< Sound when camera is traveling between actors.

		Sound m_AreaPickedSound; //!< Sound for making an area focus.
		Sound m_ObjectPickedSound; //!< Sound for making an object focus.
		Sound m_PurchaseMadeSound; //!< Sound for making a purchase.

		Sound m_PlacementBlip; //!< Sound for placement of object to scene.
		Sound m_PlacementThud; //!< Sound for placement of object to scene.
		Sound m_PlacementGravel; //!< Sound for gravely placement of object to scene.

	private:

		/// <summary>
		/// Clears all the member variables of this GUISound, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();
	};
}
#endif