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

	int GUISound::Initialize() {
		// Interface sounds should not be pitched to reinforce the appearance of time decoupling between simulation and UI.

		m_SplashSound.Create("Base.rte/Sounds/GUIs/MetaStart.flac", true, false);

		m_EnterMenuSound.Create("Base.rte/Sounds/GUIs/MenuEnter.flac", true, false);

		m_ExitMenuSound.Create("Base.rte/Sounds/GUIs/MenuExit1.flac", true, false);
		m_ExitMenuSound.GetTopLevelSoundSet().AddSound("Base.rte/Sounds/GUIs/MenuExit2.flac", true);

		m_FocusChangeSound.Create("Base.rte/Sounds/GUIs/FocusChange.flac", true, false);

		m_SelectionChangeSound.Create("Base.rte/Sounds/GUIs/SelectionChange.flac", true, false);

		m_ItemChangeSound.Create("Base.rte/Sounds/GUIs/ItemChange.flac", true, false);

		m_ButtonPressSound.Create("Base.rte/Sounds/GUIs/ButtonPress.flac", true, false);

		m_BackButtonPressSound.Create("Base.rte/Sounds/GUIs/BackButtonPress.flac", true, false);

		m_ConfirmSound.Create("Base.rte/Sounds/GUIs/MenuExit1.flac", true, false);

		m_UserErrorSound.Create("Base.rte/Sounds/GUIs/UserError.flac", true, false);

		m_TestSound.Create("Base.rte/Sounds/GUIs/Test.flac", true, false);

		m_PieMenuEnterSound.Create("Base.rte/Sounds/GUIs/PieMenuEnter.flac", true, false);

		m_PieMenuExitSound.Create("Base.rte/Sounds/GUIs/PieMenuExit.flac", true, false);

		//		m_HoverChangeSound.Create("Base.rte/Sounds/GUIs/SelectionChange.flac", true, false);
		m_HoverChangeSound = m_SelectionChangeSound;

		//		m_HoverDisabledSound.Create("Base.rte/Sounds/GUIs/PlacementBlip.flac", true, false);
		m_HoverDisabledSound = m_PlacementBlip;

		m_SlicePickedSound.Create("Base.rte/Sounds/GUIs/SlicePicked.flac", true, false);

		//		m_DisabledPickedSound.Create("Base.rte/Sounds/GUIs/PieMenuExit.flac", true, false);
		m_DisabledPickedSound = m_PieMenuExitSound;

		m_FundsChangedSound.Create("Base.rte/Sounds/GUIs/FundsChanged1.flac", true, false);
		m_FundsChangedSound.GetTopLevelSoundSet().AddSound("Base.rte/Sounds/GUIs/FundsChanged2.flac", true);
		m_FundsChangedSound.GetTopLevelSoundSet().AddSound("Base.rte/Sounds/GUIs/FundsChanged3.flac", true);
		m_FundsChangedSound.GetTopLevelSoundSet().AddSound("Base.rte/Sounds/GUIs/FundsChanged4.flac", true);
		m_FundsChangedSound.GetTopLevelSoundSet().AddSound("Base.rte/Sounds/GUIs/FundsChanged5.flac", true);
		m_FundsChangedSound.GetTopLevelSoundSet().AddSound("Base.rte/Sounds/GUIs/FundsChanged6.flac", true);
		m_FundsChangedSound.SetSoundOverlapMode(SoundContainer::SoundOverlapMode::RESTART);

		m_ActorSwitchSound.Create("Base.rte/Sounds/GUIs/ActorSwitch.flac", true, false);

		m_BrainSwitchSound.Create("Base.rte/Sounds/GUIs/BrainSwitch.flac", true, false);

		m_CameraTravelSound.Create("Base.rte/Sounds/GUIs/CameraTravel1.flac", true, false);
		m_CameraTravelSound.GetTopLevelSoundSet().AddSound("Base.rte/Sounds/GUIs/CameraTravel2.flac", true);
		m_CameraTravelSound.GetTopLevelSoundSet().AddSound("Base.rte/Sounds/GUIs/CameraTravel3.flac", true);

		//		m_AreaPickedSound.Create("Base.rte/Sounds/GUIs/MenuEnter.flac", true, false);
		m_AreaPickedSound = m_ConfirmSound;

		//		m_ObjectPickedSound.Create("Base.rte/Sounds/GUIs/MenuEnter.flac", true, false);
		m_ObjectPickedSound = m_ConfirmSound;

		//		m_PurchaseMadeSound.Create("Base.rte/Sounds/GUIs/MenuEnter.flac", true, false);
		m_PurchaseMadeSound = m_ConfirmSound;

		m_PlacementBlip.Create("Base.rte/Sounds/GUIs/PlacementBlip.flac", true, false);

		m_PlacementThud.Create("Base.rte/Sounds/GUIs/PlacementThud1.flac", true, false);
		m_PlacementThud.GetTopLevelSoundSet().AddSound("Base.rte/Sounds/GUIs/PlacementThud2.flac", true);

		m_PlacementGravel.Create("Base.rte/Sounds/GUIs/PlacementGravel1.flac", true, false);
		m_PlacementGravel.GetTopLevelSoundSet().AddSound("Base.rte/Sounds/GUIs/PlacementGravel2.flac", true);
		m_PlacementGravel.GetTopLevelSoundSet().AddSound("Base.rte/Sounds/GUIs/PlacementGravel3.flac", true);
		m_PlacementGravel.GetTopLevelSoundSet().AddSound("Base.rte/Sounds/GUIs/PlacementGravel4.flac", true);

		return 0;
	}
}
