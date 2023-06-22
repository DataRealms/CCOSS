#include "PauseMenuGUI.h"

#include "WindowMan.h"
#include "FrameMan.h"
#include "ConsoleMan.h"

#include "GUI.h"
#include "AllegroScreen.h"
#include "GUIInputWrapper.h"
#include "GUICollectionBox.h"
#include "GUIButton.h"

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PauseMenuGUI::Clear() {
		m_GUIControlManager = nullptr;

		m_UpdateResult = PauseMenuUpdateResult::NoEvent;

		m_BackdropBitmap = nullptr;

		m_PauseMenuButtons.fill(nullptr);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PauseMenuGUI::Create(AllegroScreen *guiScreen, GUIInputWrapper *guiInput) {
		m_GUIControlManager = std::make_unique<GUIControlManager>();
		RTEAssert(m_GUIControlManager->Create(guiScreen, guiInput, "Base.rte/GUIs/Skins/Menus", "MainMenuSubMenuSkin.ini"), "Failed to create GUI Control Manager and load it from Base.rte/GUIs/Skins/Menus/MainMenuSubMenuSkin.ini");
		//RTEAssert(m_GUIControlManager->Create(guiScreen, guiInput, "Base.rte/GUIs/Skins/Menus", "MainMenuScreenSkin.ini"), "Failed to create GUI Control Manager and load it from Base.rte/GUIs/Skins/Menus/MainMenuSubMenuSkin.ini");
		m_GUIControlManager->Load("Base.rte/GUIs/PauseMenuGUI.ini");

		int rootBoxMaxWidth = g_WindowMan.FullyCoversAllDisplays() ? g_WindowMan.GetPrimaryWindowDisplayWidth() / g_WindowMan.GetResMultiplier() : g_WindowMan.GetResX();

		dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("root"))->Resize(rootBoxMaxWidth, g_WindowMan.GetResY());
		dynamic_cast<GUICollectionBox *>(m_GUIControlManager->GetControl("PauseScreen"))->CenterInParent(true, true);

		m_PauseMenuButtons[PauseMenuButton::BackToMainButton] = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonBackToMain"));
		m_PauseMenuButtons[PauseMenuButton::ResumeButton] = dynamic_cast<GUIButton *>(m_GUIControlManager->GetControl("ButtonResume"));


		if (m_BackdropBitmap) {
			destroy_bitmap(m_BackdropBitmap);
		}
		BITMAP *backbuffer = g_FrameMan.GetBackBuffer32();
		m_BackdropBitmap = create_bitmap_ex(bitmap_color_depth(backbuffer), backbuffer->w, backbuffer->h);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PauseMenuGUI::StoreFrameForUseAsBackdrop() {
		BITMAP *backbuffer = g_FrameMan.GetBackBuffer32();
		blit(backbuffer, m_BackdropBitmap, 0, 0, 0, 0, backbuffer->w, backbuffer->h);

		set_trans_blender(96, 96, 96, 96);
		draw_trans_sprite(m_BackdropBitmap, g_FrameMan.GetOverlayBitmap32(), 0, 0);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	PauseMenuGUI::PauseMenuUpdateResult PauseMenuGUI::Update() {
		m_UpdateResult = PauseMenuUpdateResult::NoEvent;

		if (g_ConsoleMan.IsEnabled() && !g_ConsoleMan.IsReadOnly()) {
			return m_UpdateResult;
		}

		int mousePosX;
		int mousePosY;
		m_GUIControlManager->GetManager()->GetInputController()->GetMousePosition(&mousePosX, &mousePosY);
		m_GUIControlManager->Update();

		GUIEvent guiEvent;
		while (m_GUIControlManager->GetEvent(&guiEvent)) {
			if (guiEvent.GetType() == GUIEvent::Command) {
				if (guiEvent.GetControl() == m_PauseMenuButtons[PauseMenuButton::ResumeButton]) {
					g_GUISound.BackButtonPressSound()->Play();
					m_UpdateResult = PauseMenuUpdateResult::ActivityResumed;
				} else if (guiEvent.GetControl() == m_PauseMenuButtons[PauseMenuButton::BackToMainButton]) {
					g_GUISound.BackButtonPressSound()->Play();
					m_UpdateResult = PauseMenuUpdateResult::BackToMain;
				}
			}
			if (guiEvent.GetType() == GUIEvent::Notification && (guiEvent.GetMsg() == GUIButton::Focused && dynamic_cast<GUIButton *>(guiEvent.GetControl()))) {
				g_GUISound.SelectionChangeSound()->Play();
			}
		}

		return m_UpdateResult;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PauseMenuGUI::Draw() {
		blit(m_BackdropBitmap, g_FrameMan.GetBackBuffer32(), 0, 0, 0, 0, m_BackdropBitmap->w, m_BackdropBitmap->h);
		m_GUIControlManager->Draw();
		m_GUIControlManager->DrawMouse();
	}
}