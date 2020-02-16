#include "GUISound.h"

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void GUISound::Clear() {
		m_SplashSound.Reset();
		m_EnterMenuSound.Reset();
		m_ExitMenuSound.Reset();
		m_FocusChangeSound.Reset();
		m_SelectionChangeSound.Reset();
		m_ItemChangeSound.Reset();
		m_ButtonPressSound.Reset();
		m_BackButtonPressSound.Reset();
		m_ConfirmSound.Reset();
		m_UserErrorSound.Reset();
		m_TestSound.Reset();
		m_PieMenuEnterSound.Reset();
		m_PieMenuExitSound.Reset();
		m_HoverChangeSound.Reset();
		m_HoverDisabledSound.Reset();
		m_SlicePickedSound.Reset();
		m_DisabledPickedSound.Reset();
		m_FundsChangedSound.Reset();
		m_ActorSwitchSound.Reset();
		m_BrainSwitchSound.Reset();
		m_CameraTravelSound.Reset();
		m_AreaPickedSound.Reset();
		m_ObjectPickedSound.Reset();
		m_PurchaseMadeSound.Reset();
		m_PlacementBlip.Reset();
		m_PlacementThud.Reset();
		m_PlacementGravel.Reset();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int GUISound::Create() {
		// Interface sounds should not be pitched to reinforce the appearance of time decoupling between simulation and UI.

		m_SplashSound.Sound::Create("Base.rte/Sounds/GUIs/MetaStart.wav", false, 0);

		m_EnterMenuSound.Sound::Create("Base.rte/Sounds/GUIs/MenuEnter.wav", false, 0);

		m_ExitMenuSound.Sound::Create("Base.rte/Sounds/GUIs/MenuExit1.wav", false, 0);
		m_ExitMenuSound.Sound::AddSample("Base.rte/Sounds/GUIs/MenuExit2.wav");

		m_FocusChangeSound.Sound::Create("Base.rte/Sounds/GUIs/FocusChange.wav", false, 0);

		m_SelectionChangeSound.Sound::Create("Base.rte/Sounds/GUIs/SelectionChange.wav", false, 0);

		m_ItemChangeSound.Sound::Create("Base.rte/Sounds/GUIs/ItemChange.wav", false, 0);

		m_ButtonPressSound.Sound::Create("Base.rte/Sounds/GUIs/ButtonPress.wav", false, 0);

		m_BackButtonPressSound.Sound::Create("Base.rte/Sounds/GUIs/BackButtonPress.wav", false, 0);

		m_ConfirmSound.Sound::Create("Base.rte/Sounds/GUIs/MenuExit2.wav", false, 0);

		m_UserErrorSound.Sound::Create("Base.rte/Sounds/GUIs/UserError.wav", false, 0);

		m_TestSound.Sound::Create("Base.rte/Sounds/GUIs/Test.wav", false, 0);

		m_PieMenuEnterSound.Sound::Create("Base.rte/Sounds/GUIs/PieMenuEnter.wav", false, 0);

		m_PieMenuExitSound.Sound::Create("Base.rte/Sounds/GUIs/PieMenuExit.wav", false, 0);

		//		m_HoverChangeSound.Sound::Create("Base.rte/Sounds/GUIs/SelectionChange.wav", false, 0);
		m_HoverChangeSound = m_SelectionChangeSound;

		//		m_HoverDisabledSound.Sound::Create("Base.rte/Sounds/GUIs/PlacementBlip.wav", false, 0);
		m_HoverDisabledSound = m_PlacementBlip;

		m_SlicePickedSound.Sound::Create("Base.rte/Sounds/GUIs/SlicePicked.wav", false, 0);

		//		m_DisabledPickedSound.Sound::Create("Base.rte/Sounds/GUIs/PieMenuExit.wav", false, 0);
		m_DisabledPickedSound = m_PieMenuExitSound;

		m_FundsChangedSound.Sound::Create("Base.rte/Sounds/GUIs/FundsChanged1.wav", false, 0);
		m_FundsChangedSound.Sound::AddSample("Base.rte/Sounds/GUIs/FundsChanged2.wav");
		m_FundsChangedSound.Sound::AddSample("Base.rte/Sounds/GUIs/FundsChanged3.wav");
		m_FundsChangedSound.Sound::AddSample("Base.rte/Sounds/GUIs/FundsChanged4.wav");
		m_FundsChangedSound.Sound::AddSample("Base.rte/Sounds/GUIs/FundsChanged5.wav");
		m_FundsChangedSound.Sound::AddSample("Base.rte/Sounds/GUIs/FundsChanged6.wav");

		m_ActorSwitchSound.Sound::Create("Base.rte/Sounds/GUIs/ActorSwitch.wav", false, 0);

		m_BrainSwitchSound.Sound::Create("Base.rte/Sounds/GUIs/BrainSwitch.wav", false, 0);

		m_CameraTravelSound.Sound::Create("Base.rte/Sounds/GUIs/CameraTravel1.wav", false, 0);
		m_CameraTravelSound.Sound::AddSample("Base.rte/Sounds/GUIs/CameraTravel2.wav");
		m_CameraTravelSound.Sound::AddSample("Base.rte/Sounds/GUIs/CameraTravel3.wav");

		//		m_AreaPickedSound.Sound::Create("Base.rte/Sounds/GUIs/MenuEnter.wav", false, 0);
		m_AreaPickedSound = m_EnterMenuSound;

		//		m_ObjectPickedSound.Sound::Create("Base.rte/Sounds/GUIs/MenuEnter.wav", false, 0);
		m_ObjectPickedSound = m_EnterMenuSound;

		//		m_PurchaseMadeSound.Sound::Create("Base.rte/Sounds/GUIs/MenuEnter.wav", false, 0);
		m_PurchaseMadeSound = m_EnterMenuSound;

		m_PlacementBlip.Sound::Create("Base.rte/Sounds/GUIs/PlacementBlip.wav", false, 0);

		m_PlacementThud.Sound::Create("Base.rte/Sounds/GUIs/PlacementThud1.wav", false, 0);
		m_PlacementThud.Sound::AddSample("Base.rte/Sounds/GUIs/PlacementThud2.wav");

		m_PlacementGravel.Sound::Create("Base.rte/Sounds/GUIs/PlacementGravel1.wav", false, 0);
		m_PlacementGravel.Sound::AddSample("Base.rte/Sounds/GUIs/PlacementGravel2.wav");
		m_PlacementGravel.Sound::AddSample("Base.rte/Sounds/GUIs/PlacementGravel3.wav");
		m_PlacementGravel.Sound::AddSample("Base.rte/Sounds/GUIs/PlacementGravel4.wav");

		return 0;
	}
}


