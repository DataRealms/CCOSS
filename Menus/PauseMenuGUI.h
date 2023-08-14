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
	class SettingsGUI;
	class ModManagerGUI;
	class SaveLoadMenuGUI;

	/// <summary>
	/// Handling for the pause menu screen composition and interaction.
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
		/// Constructor method used to instantiate a PauseMenuGUI object in system memory and make it ready for use.
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

#pragma region Setters
		/// <summary>
		/// Sets the "Back to Main Menu" button text to the menu we will be going back to.
		/// </summary>
		/// <param name="menuName">The target menu name, e.g. "Conquest" will result in "Back to Conquest Menu".</param>
		void SetBackButtonTargetName(const std::string &menuName);
#pragma endregion

#pragma region Concrete Methods
		/// <summary>
		/// Enables or disables buttons depending on the current Activity.
		/// </summary>
		void EnableOrDisablePauseMenuFeatures();

		/// <summary>
		/// Stores the current frame to be used as a backdrop of the PauseMenuGUI to fake the pausing of the Activity.
		/// </summary>
		void StoreFrameForUseAsBackdrop();

		/// <summary>
		/// Updates the PauseMenuGUI state.
		/// </summary>
		/// <returns>The result of the PauseMenuGUI input and event update. See PauseMenuUpdateResult enumeration.</returns>
		PauseMenuUpdateResult Update();

		/// <summary>
		/// Draws the PauseMenuGUI to the screen.
		/// </summary>
		void Draw();
#pragma endregion

	private:

		/// <summary>
		/// Enumeration for the different sub-menu screens of the pause menu.
		/// </summary>
		enum PauseMenuScreen {
			MainScreen,
			SaveOrLoadGameScreen,
			SettingsScreen,
			ModManagerScreen,
			ScreenCount
		};

		/// <summary>
		/// Enumeration for all the different buttons of the pause menu.
		/// </summary>
		enum PauseMenuButton {
			BackToMainButton,
			SaveOrLoadGameButton,
			SettingsButton,
			ModManagerButton,
			ResumeButton,
			ButtonCount
		};

		std::unique_ptr<GUIControlManager> m_GUIControlManager; //!< The GUIControlManager which owns all the GUIControls of the PauseMenuGUI.
		GUICollectionBox *m_ActiveDialogBox; // The currently active GUICollectionBox in any of the pause menu screens that acts as a dialog box and requires drawing an overlay.

		/// <summary>
		/// BITMAP that is used for the backdrop of the pause menu.
		/// This will store the current frame to fake pausing the Activity and make it seem like it's just frozen in the background, while in reality we completely left the game loop and entered an alternative "main menu".
		/// This is the trick behind this whole bullshit implementation, because we can't ACTUALLY pause the sim and remain in Activity since input stops being processed.
		/// </summary>
		BITMAP *m_BackdropBitmap;

		PauseMenuScreen m_ActiveMenuScreen; //!< The currently active pause menu screen that is being updated and drawn to the screen. See PauseMenuScreen enumeration.
		PauseMenuUpdateResult m_UpdateResult; //!< The result of the PauseMenuGUI update. See PauseMenuUpdateResult enumeration.

		Timer m_ResumeButtonBlinkTimer; //!< Activity resume button blink timer.

		std::unique_ptr<SaveLoadMenuGUI> m_SaveLoadMenu; //!< The settings menu screen.
		std::unique_ptr<SettingsGUI> m_SettingsMenu; //!< The settings menu screen.
		std::unique_ptr<ModManagerGUI> m_ModManagerMenu; //!< The mod manager menu screen.

		// TODO: Rework this hacky garbage implementation when setting button font at runtime without loading a different skin is fixed.
		// Right now the way this works is the font graphic has different character visuals for uppercase and lowercase and the visual change happens by applying the appropriate case string when hovering/unhovering.
		std::array<std::string, PauseMenuButton::ButtonCount> m_ButtonHoveredText; //!< Array containing uppercase strings of the pause menu buttons text that are used to display the larger font when a button is hovered over.
		std::array<std::string, PauseMenuButton::ButtonCount> m_ButtonUnhoveredText; //!< Array containing lowercase strings of the pause menu buttons text that are used to display the smaller font when a button is not hovered over.
		GUIButton *m_HoveredButton; //!< The currently hovered pause menu button.
		int m_PrevHoveredButtonIndex; //!< The index of the previously hovered pause menu button in the main menu button array.

		bool m_SavingButtonsDisabled; //!< Whether the save and load buttons are disabled and hidden.
		bool m_ModManagerButtonDisabled; //!< Whether the mod manager button is disabled and hidden.

		/// <summary>
		/// GUI elements that compose the pause menu screen.
		/// </summary>
		GUICollectionBox *m_PauseMenuBox;
		std::array<GUIButton *, PauseMenuButton::ButtonCount> m_PauseMenuButtons;

#pragma region Menu Screen Handling
		/// <summary>
		/// Sets the PauseMenuGUI to display a menu screen.
		/// </summary>
		/// <param name="screenToShow">Which menu screen to display. See PauseMenuScreen enumeration.</param>
		/// <param name="playButtonPressSound">Whether to play a sound if the menu screen change is triggered by a button press.</param>
		void SetActiveMenuScreen(PauseMenuScreen screenToShow, bool playButtonPressSound = true);
#pragma endregion

#pragma region Update Breakdown
		/// <summary>
		/// Handles returning to the pause menu from one of the sub-menus if the player requested to return via the back button or the esc key. Also handles closing active dialog boxes with the esc key.
		/// </summary>
		/// <param name="backButtonPressed">Whether the player requested to return to the pause menu from one of the sub-menus via back button.</param>
		void HandleBackNavigation(bool backButtonPressed);

		/// <summary>
		/// Handles the player interaction with the PauseMenuGUI GUI elements.
		/// </summary>
		/// <returns>Whether the player requested to return to the main menu.</returns>
		bool HandleInputEvents();

		/// <summary>
		/// Updates the currently hovered button text to give the hovered visual and updates the previously hovered button to remove the hovered visual.
		/// </summary>
		/// <param name="hoveredButton">Pointer to the currently hovered button, if any. Acquired by GUIControlManager::GetControlUnderPoint.</param>
		void UpdateHoveredButton(const GUIButton *hoveredButton);

		/// <summary>
		/// Animates (blinking) the resume game button.
		/// </summary>
		void BlinkResumeButton();
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