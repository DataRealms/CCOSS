#ifndef _RTEPAUSEMENUGUI_
#define _RTEPAUSEMENUGUI_

#include "Timer.h"

struct BITMAP;

namespace RTE {

	class AllegroScreen;
	class GUIInputWrapper;
	class GUIControlManager;
	class GUICollectionBox;
	class GUIButton;

	/// <summary>
	///
	/// </summary>
	class PauseMenuGUI {

	public:

		/// <summary>
		/// Enumeration for the results of the PauseMenuGUI input and event update.
		/// </summary>
		enum class PauseMenuUpdateResult {
			NoEvent,
			BackToMain,
			ActivityResumed
		};

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a PauseMenuGUI object in system memory and makes it ready for use.
		/// </summary>
		/// <param name="guiScreen">Pointer to a GUIScreen interface that will be used by this PauseMenuGUI's GUIControlManager. Ownership is NOT transferred!</param>
		/// <param name="guiInput">Pointer to a GUIInput interface that will be used by this PauseMenuGUI's GUIControlManager. Ownership is NOT transferred!</param>
		PauseMenuGUI(AllegroScreen *guiScreen, GUIInputWrapper *guiInput) { Clear(); Create(guiScreen, guiInput); }

		/// <summary>
		/// Makes the PauseMenuGUI object ready for use.
		/// </summary>
		/// <param name="guiScreen">Pointer to a GUIScreen interface that will be used by this PauseMenuGUI's GUIControlManager. Ownership is NOT transferred!</param>
		/// <param name="guiInput">Pointer to a GUIInput interface that will be used by this PauseMenuGUI's GUIControlManager. Ownership is NOT transferred!</param>
		void Create(AllegroScreen *guiScreen, GUIInputWrapper *guiInput);
#pragma endregion

#pragma region Concrete Methods
		/// <summary>
		///
		/// </summary>
		void StoreFrameForUseAsBackdrop();

		/// <summary>
		/// Updates the MainMenuGUI state.
		/// </summary>
		/// <returns>The result of the MainMenuGUI input and event update. See MainMenuUpdateResult enumeration.</returns>
		PauseMenuUpdateResult Update();

		/// <summary>
		/// Draws the PauseMenuGUI to the screen.
		/// </summary>
		void Draw();
#pragma endregion

	private:

		/// <summary>
		/// Enumeration for all the different buttons of the pause menu.
		/// </summary>
		enum PauseMenuButton {
			BackToMainButton,
			SaveGameButton,
			LoadLastSaveButton,
			SettingsButton,
			ResumeButton,
			ButtonCount
		};

		std::unique_ptr<GUIControlManager> m_GUIControlManager; //!< The GUIControlManager which owns all the GUIControls of the PauseMenuGUI.

		PauseMenuUpdateResult m_UpdateResult; //!< The result of the PauseMenuGUI update. See PauseMenuUpdateResult enumeration.

		BITMAP *m_BackdropBitmap; //!<

		Timer m_ResumeButtonBlinkTimer; //!< Activity resume button blink timer.

		// TODO: Rework this hacky garbage implementation when setting button font at runtime without loading a different skin is fixed.
		// Right now the way this works is the font graphic has different character visuals for uppercase and lowercase and the visual change happens by applying the appropriate case string when hovering/unhovering.
		std::array<std::string, PauseMenuButton::ButtonCount> m_ButtonHoveredText; //!< Array containing uppercase strings of the pause menu buttons text that are used to display the larger font when a button is hovered over.
		std::array<std::string, PauseMenuButton::ButtonCount> m_ButtonUnhoveredText; //!< Array containing lowercase strings of the pause menu buttons text that are used to display the smaller font when a button is not hovered over.
		GUIButton *m_HoveredButton; //!< The currently hovered pause menu button.
		int m_PrevHoveredButtonIndex; //!< The index of the previously hovered pause menu button in the main menu button array.

		/// <summary>
		/// GUI elements that compose the main menu screen.
		/// </summary>
		GUICollectionBox *m_PauseMenuBox;
		std::array<GUIButton *, PauseMenuButton::ButtonCount> m_PauseMenuButtons;

#pragma region Update Breakdown
		/// <summary>
		/// Animates (blinking) the resume game button.
		/// </summary>
		void BlinkResumeButton();

		/// <summary>
		/// Updates the currently hovered button text to give the hovered visual and updates the previously hovered button to remove the hovered visual.
		/// </summary>
		/// <param name="hoveredButton">Pointer to the currently hovered button, if any. Acquired by GUIControlManager::GetControlUnderPoint.</param>
		void UpdateHoveredButton(const GUIButton *hoveredButton);
#pragma endregion

		/// <summary>
		/// Clears all the member variables of this PauseMenuGUI, effectively resetting the members of this object.
		/// </summary>
		void Clear();

		// Disallow the use of some implicit methods.
		PauseMenuGUI(const PauseMenuGUI &reference) = delete;
		PauseMenuGUI & operator=(const PauseMenuGUI &rhs) = delete;
	};
}
#endif