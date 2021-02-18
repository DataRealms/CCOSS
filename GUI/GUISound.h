#ifndef _GUISOUND_
#define _GUISOUND_

#include "SoundContainer.h"
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
		int Initialize();
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Destroys and resets (through Clear()) the GUISound object.
		/// </summary>
		void Destroy() { Clear(); }

		/// <summary>
		/// Destructor method used to clean up a GUISound object before deletion from system memory.
		/// </summary>
		~GUISound() { Destroy(); }
#pragma endregion

#pragma region Getters
		/// <summary>
		/// Gets juicy logo signature jingle Sound.
		/// </summary>
		/// <returns>Juicy logo signature jingle Sound.</returns>
		SoundContainer *SplashSound() { return &m_SplashSound; }

		/// <summary>
		/// Gets SoundContainer for enabling menu.
		/// </summary>
		/// <returns>SoundContainer for enabling menu.</returns>
		SoundContainer *EnterMenuSound() { return &m_EnterMenuSound; }

		/// <summary>
		/// Gets SoundContainer for disabling menu.
		/// </summary>
		/// <returns>SoundContainer for disabling menu.</returns>
		SoundContainer *ExitMenuSound() { return &m_ExitMenuSound; }

		/// <summary>
		/// Gets SoundContainer for changing focus.
		/// </summary>
		/// <returns>SoundContainer for changing focus.</returns>
		SoundContainer *FocusChangeSound() { return &m_FocusChangeSound; }

		/// <summary>
		/// Gets SoundContainer for selecting items in list, etc.
		/// </summary>
		/// <returns>SoundContainer for selecting items in list, etc.</returns>
		SoundContainer *SelectionChangeSound() { return &m_SelectionChangeSound; }

		/// <summary>
		/// Gets SoundContainer for adding or deleting items in list.
		/// </summary>
		/// <returns>SoundContainer for adding or deleting items in list.</returns>
		SoundContainer *ItemChangeSound() { return &m_ItemChangeSound; }

		/// <summary>
		/// Gets SoundContainer for button press.
		/// </summary>
		/// <returns>SoundContainer for button press.</returns>
		SoundContainer *ButtonPressSound() { return &m_ButtonPressSound; }

		/// <summary>
		/// Gets SoundContainer for button press of going back button.
		/// </summary>
		/// <returns>SoundContainer for button press of going back button.</returns>
		SoundContainer *BackButtonPressSound() { return &m_BackButtonPressSound; }

		/// <summary>
		/// Gets SoundContainer for confirming a selection.
		/// </summary>
		/// <returns>SoundContainer for confirming a selection.</returns>
		SoundContainer *ConfirmSound() { return &m_ConfirmSound; }

		/// <summary>
		/// Gets SoundContainer for erroneous input.
		/// </summary>
		/// <returns>SoundContainer for erroneous input.</returns>
		SoundContainer *UserErrorSound() { return &m_UserErrorSound; }

		/// <summary>
		/// Gets SoundContainer for testing volume when adjusting volume sliders.
		/// </summary>
		/// <returns>SoundContainer for testing volume when adjusting volume sliders.</returns>
		SoundContainer *TestSound() { return &m_TestSound; }

		/// <summary>
		/// Gets SoundContainer for opening pie menu.
		/// </summary>
		/// <returns>SoundContainer for opening pie menu.</returns>
		SoundContainer *PieMenuEnterSound() { return &m_PieMenuEnterSound; }

		/// <summary>
		/// Gets SoundContainer for closing pie menu.
		/// </summary>
		/// <returns>SoundContainer for closing pie menu.</returns>
		SoundContainer *PieMenuExitSound() { return &m_PieMenuExitSound; }

		/// <summary>
		/// Gets SoundContainer for when PieMenu hover arrow appears or changes slice.
		/// </summary>
		/// <returns>SoundContainer for when PieMenu hover arrow appears or changes slice.</returns>
		SoundContainer *HoverChangeSound() { return &m_HoverChangeSound; }

		/// <summary>
		/// Gets SoundContainer for when PieMenu hover arrow appears or changes to a disabled slice.
		/// </summary>
		/// <returns>SoundContainer for when PieMenu hover arrow appears or changes to a disabled slice.</returns>
		SoundContainer *HoverDisabledSound() { return &m_HoverDisabledSound; }

		/// <summary>
		/// Gets SoundContainer for picking a valid PieMenu slice.
		/// </summary>
		/// <returns>SoundContainer for picking a valid PieMenu slice.</returns>
		SoundContainer *SlicePickedSound() { return &m_SlicePickedSound; }

		/// <summary>
		/// Gets SoundContainer for erroneous input in PieMenu.
		/// </summary>
		/// <returns>SoundContainer for erroneous input in PieMenu.</returns>
		SoundContainer *DisabledPickedSound() { return &m_DisabledPickedSound; }

		/// <summary>
		/// Gets SoundContainer for when the funds of a team changes.
		/// </summary>
		/// <returns>SoundContainer for when the funds of a team changes.</returns>
		SoundContainer *FundsChangedSound() { return &m_FundsChangedSound; }

		/// <summary>
		/// Gets SoundContainer for switching between regular (non-brain) actors.
		/// </summary>
		/// <returns>SoundContainer for switching between regular (non-brain) actors.</returns>
		SoundContainer *ActorSwitchSound() { return &m_ActorSwitchSound; }

		/// <summary>
		/// Gets SoundContainer for switching to the brain shortcut.
		/// </summary>
		/// <returns>SoundContainer for switching to the brain shortcut.</returns>
		SoundContainer *BrainSwitchSound() { return &m_BrainSwitchSound; }

		/// <summary>
		/// Gets SoundContainer when camera is traveling between actors.
		/// </summary>
		/// <returns>SoundContainer when camera is traveling between actors.</returns>
		SoundContainer *CameraTravelSound() { return &m_CameraTravelSound; }

		/// <summary>
		/// Gets SoundContainer for making an area focus.
		/// </summary>
		/// <returns>SoundContainer for making an area focus.</returns>
		SoundContainer *AreaPickedSound() { return &m_AreaPickedSound; }

		/// <summary>
		/// Gets SoundContainer for making an object focus.
		/// </summary>
		/// <returns>SoundContainer for making an object focus.</returns>
		SoundContainer *ObjectPickedSound() { return &m_ObjectPickedSound; }

		/// <summary>
		/// Gets SoundContainer for making a purchase.
		/// </summary>
		/// <returns>SoundContainer for making a purchase.</returns>
		SoundContainer *PurchaseMadeSound() { return &m_PurchaseMadeSound; }

		/// <summary>
		/// Gets SoundContainer for placement of object to scene.
		/// </summary>
		/// <returns>SoundContainer for placement of object to scene.</returns>
		SoundContainer *PlacementBlip() { return &m_PlacementBlip; }

		/// <summary>
		/// Gets SoundContainer for placement of object to scene.
		/// </summary>
		/// <returns>SoundContainer for placement of object to scene.</returns>
		SoundContainer *PlacementThud() { return &m_PlacementThud; }

		/// <summary>
		/// Gets SoundContainer for gravely placement of object to scene.
		/// </summary>
		/// <returns>SoundContainer for gravely placement of object to scene.</returns>
		SoundContainer *PlacementGravel() { return &m_PlacementGravel; }
#pragma endregion

	protected:
		SoundContainer m_SplashSound; //! Juicy logo signature jingle sound.

		SoundContainer m_EnterMenuSound; //! SoundContainer for enabling menu.
		SoundContainer m_ExitMenuSound; //! SoundContainer for disabling menu.

		SoundContainer m_FocusChangeSound; //! SoundContainer for changing focus.

		SoundContainer m_SelectionChangeSound; //! SoundContainer for selecting items in list, etc.
		SoundContainer m_ItemChangeSound; //! SoundContainer for adding or deleting items in list.

		SoundContainer m_ButtonPressSound; //! SoundContainer for button press.
		SoundContainer m_BackButtonPressSound; //! SoundContainer for button press of going back button.
		SoundContainer m_ConfirmSound; //! SoundContainer for confirming a selection.
		SoundContainer m_UserErrorSound; //! SoundContainer for erroneous input.
		SoundContainer m_TestSound; //! SoundContainer for testing volume when adjusting volume sliders.

		SoundContainer m_PieMenuEnterSound; //! SoundContainer for opening pie menu.
		SoundContainer m_PieMenuExitSound; //! SoundContainer for closing pie menu.

		SoundContainer m_HoverChangeSound; //! SoundContainer for when PieMenu hover arrow appears or changes slice.
		SoundContainer m_HoverDisabledSound; //! SoundContainer for when PieMenu hover arrow appears or changes to a disabled slice.
		SoundContainer m_SlicePickedSound; //! SoundContainer for picking a valid PieMenu slice.
		SoundContainer m_DisabledPickedSound; //! SoundContainer for erroneous input in PieMenu.

		SoundContainer m_FundsChangedSound; //! SoundContainer for when the funds of a team changes.

		SoundContainer m_ActorSwitchSound; //! SoundContainer for switching between regular (non-brain) actors.
		SoundContainer m_BrainSwitchSound; //! SoundContainer for switching to the brain shortcut.

		SoundContainer m_CameraTravelSound; //! SoundContainer when camera is traveling between actors.

		SoundContainer m_AreaPickedSound; //! SoundContainer for making an area focus.
		SoundContainer m_ObjectPickedSound; //! SoundContainer for making an object focus.
		SoundContainer m_PurchaseMadeSound; //! SoundContainer for making a purchase.

		SoundContainer m_PlacementBlip; //! SoundContainer for placement of object to scene.
		SoundContainer m_PlacementThud; //! SoundContainer for placement of object to scene.
		SoundContainer m_PlacementGravel; //! SoundContainer for gravely placement of object to scene.

		/// <summary>
		/// Clears all the member variables of this GUISound, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();
	};
}
#endif