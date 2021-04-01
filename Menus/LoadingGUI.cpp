#include "LoadingGUI.h"
#include "Writer.h"
#include "SceneLayer.h"
#include "SettingsMan.h"
#include "PresetMan.h"
#include "FrameMan.h"

#include "GUI/GUI.h"
#include "GUI/GUICollectionBox.h"
#include "GUI/GUIListBox.h"
#include "GUI/AllegroScreen.h"
#include "GUI/AllegroBitmap.h"
#include "GUI/AllegroInput.h"

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void LoadingGUI::Clear() {
		m_ControlManager = nullptr;
		m_GUIInput = nullptr;
		m_GUIScreen = nullptr;
		m_LoadingLogWriter = nullptr;
		m_ProgressListboxBitmap = nullptr;
		m_PosX = 0;
		m_PosY = 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void LoadingGUI::InitLoadingScreen() {
		g_FrameMan.LoadPalette("Base.rte/palette.bmp");
		g_FrameMan.ClearBackBuffer32();

		m_GUIInput.reset(new AllegroInput(-1));
		m_GUIScreen.reset(new AllegroScreen(g_FrameMan.GetBackBuffer32()));

		if (!m_ControlManager) {
			m_ControlManager.reset(new GUIControlManager());
			if (!m_ControlManager->Create(m_GUIScreen.get(), m_GUIInput.get(), "Base.rte/GUIs/Skins/MainMenu", "LoadingSkin.ini")) {
				RTEAbort("Failed to create GUI Control Manager and load it from Base.rte/GUIs/Skins/MainMenu/LoadingSkin.ini");
			}
			m_ControlManager->Load("Base.rte/GUIs/LoadingGUI.ini");
		}
		if (!g_SettingsMan.DisableLoadingScreen()) { CreateProgressReportListbox(); }

		SceneLayer loadingSplash;
		loadingSplash.Create(ContentFile("Base.rte/GUIs/Title/LoadingSplash.png"), false, Vector(), true, false, Vector(1.0F, 0));

		// Hardcoded offset to make room for the loading box only if DisableLoadingScreen is false.
		int loadingSplashOffset = g_SettingsMan.DisableLoadingScreen() ? 14 : 120;
		loadingSplash.SetOffset(Vector(static_cast<float>(((loadingSplash.GetBitmap()->w - g_FrameMan.GetResX()) / 2) + loadingSplashOffset), 0));

		// Draw onto wrapped strip centered vertically on the screen
		Box splashBox(Vector(0, static_cast<float>((g_FrameMan.GetResY() - loadingSplash.GetBitmap()->h) / 2)), static_cast<float>(g_FrameMan.GetResX()), static_cast<float>(loadingSplash.GetBitmap()->h));
		loadingSplash.Draw(g_FrameMan.GetBackBuffer32(), splashBox);

		g_FrameMan.FlipFrameBuffers();

		// Overwrite Settings.ini after all the managers are created to fully populate the file. Up until this moment Settings.ini is populated only with minimal required properties to run.
		// When the overwrite happens there is a short delay which causes the screen to remain black, so this is done here after the flip to mask that black screen.
		if (g_SettingsMan.SettingsNeedOverwrite()) { g_SettingsMan.UpdateSettingsFile(); }

		if (!m_LoadingLogWriter) {
			m_LoadingLogWriter.reset(new Writer("LogLoading.txt"));
			if (!m_LoadingLogWriter->WriterOK()) {
				ShowMessageBox("Failed to instantiate the Loading Log writer!\nModule loading will proceed without being logged!");
				m_LoadingLogWriter.reset();
			}
		}

		g_PresetMan.LoadAllDataModules();

		Destroy();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void LoadingGUI::CreateProgressReportListbox() {
		// Place and clear the sectionProgress box
		dynamic_cast<GUICollectionBox *>(m_ControlManager->GetControl("root"))->SetSize(g_FrameMan.GetResX(), g_FrameMan.GetResY());
		GUIListBox *listBox = dynamic_cast<GUIListBox *>(m_ControlManager->GetControl("ProgressBox"));

		// Make the box a bit bigger if there's room in higher, HD resolutions
		if (g_FrameMan.GetResX() >= 960) { listBox->Resize((g_FrameMan.GetResX() / 3) - 12, listBox->GetHeight()); }

		// Make the loading progress box fill the right third of the screen
		listBox->SetPositionRel(g_FrameMan.GetResX() - listBox->GetWidth() - 12, (g_FrameMan.GetResY() / 2) - (listBox->GetHeight() / 2));
		listBox->ClearList();

		if (!m_ProgressListboxBitmap) {
			listBox->SetVisible(false);
			m_ProgressListboxBitmap = create_bitmap_ex(8, listBox->GetWidth(), listBox->GetHeight());
			clear_to_color(m_ProgressListboxBitmap, 54);
			rect(m_ProgressListboxBitmap, 0, 0, listBox->GetWidth() - 1, listBox->GetHeight() - 1, 33);
			rect(m_ProgressListboxBitmap, 1, 1, listBox->GetWidth() - 2, listBox->GetHeight() - 2, 33);
			m_PosX = listBox->GetXPos();
			m_PosY = listBox->GetYPos();
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void LoadingGUI::Destroy() {
		if (m_ProgressListboxBitmap) { destroy_bitmap(m_ProgressListboxBitmap); }
		Clear();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void LoadingGUI::LoadingSplashProgressReport(const std::string &reportString, bool newItem) {
		if (System::IsLoggingToCLI()) { System::PrintLoadingToCLI(reportString, newItem); }

		if (newItem) {
			// Write out the last line to the log file before starting a new one and scroll the bitmap upwards.
			if (g_LoadingGUI.m_LoadingLogWriter) { *g_LoadingGUI.m_LoadingLogWriter << reportString << "\n"; }
			if (g_LoadingGUI.m_ProgressListboxBitmap) { blit(g_LoadingGUI.m_ProgressListboxBitmap, g_LoadingGUI.m_ProgressListboxBitmap, 2, 12, 2, 2, g_LoadingGUI.m_ProgressListboxBitmap->w - 3, g_LoadingGUI.m_ProgressListboxBitmap->h - 12); }
		}

		if (g_LoadingGUI.m_ProgressListboxBitmap) {
			AllegroBitmap drawBitmap(g_LoadingGUI.m_ProgressListboxBitmap);

			// Clear current line.
			rectfill(g_LoadingGUI.m_ProgressListboxBitmap, 2, g_LoadingGUI.m_ProgressListboxBitmap->h - 12, g_LoadingGUI.m_ProgressListboxBitmap->w - 3, g_LoadingGUI.m_ProgressListboxBitmap->h - 3, 54);
			// Print new line
			g_FrameMan.GetSmallFont()->DrawAligned(&drawBitmap, 5, g_LoadingGUI.m_ProgressListboxBitmap->h - 12, reportString.c_str(), GUIFont::Left);
			// DrawAligned - MaxWidth is useless here, so we're just drawing lines manually.
			vline(g_LoadingGUI.m_ProgressListboxBitmap, g_LoadingGUI.m_ProgressListboxBitmap->w - 2, g_LoadingGUI.m_ProgressListboxBitmap->h - 12, g_LoadingGUI.m_ProgressListboxBitmap->h - 2, 33);
			vline(g_LoadingGUI.m_ProgressListboxBitmap, g_LoadingGUI.m_ProgressListboxBitmap->w - 1, g_LoadingGUI.m_ProgressListboxBitmap->h - 12, g_LoadingGUI.m_ProgressListboxBitmap->h - 2, 33);

			// Draw onto current frame buffer.
			blit(g_LoadingGUI.m_ProgressListboxBitmap, g_FrameMan.GetBackBuffer32(), 0, 0, g_LoadingGUI.m_PosX, g_LoadingGUI.m_PosY, g_LoadingGUI.m_ProgressListboxBitmap->w, g_LoadingGUI.m_ProgressListboxBitmap->h);

			g_FrameMan.FlipFrameBuffers();
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void LoadingGUI::GUIControlManagerDeleter::operator()(GUIControlManager *ptr) const { ptr->Destroy(); }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void LoadingGUI::AllegroInputDeleter::operator()(AllegroInput *ptr) const { ptr->Destroy(); }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void LoadingGUI::AllegroScreenDeleter::operator()(AllegroScreen *ptr) const { ptr->Destroy(); }

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void LoadingGUI::WriterDeleter::operator()(Writer *ptr) const { ptr->EndWrite(); }
}