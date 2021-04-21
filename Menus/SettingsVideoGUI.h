#ifndef _RTESETTINGSVIDEOGUI_
#define _RTESETTINGSVIDEOGUI_

namespace RTE {

	class GUIControlManager;
	class GUICollectionBox;
	class GUIComboBox;
	class GUILabel;
	class GUIButton;
	class GUICheckbox;
	class GUIEvent;

	/// <summary>
	/// Handling for video settings through the game settings user interface.
	/// </summary>
	class SettingsVideoGUI {

	public:

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a SettingsVideoGUI object in system memory and make it ready for use.
		/// </summary>
		/// <param name="parentControlManager">Pointer to the parent GUIControlManager which holds all the GUIControls of this SettingsVideoGUI.</param>
		explicit SettingsVideoGUI(GUIControlManager *parentControlManager);
#pragma endregion

#pragma region Concrete Methods
		/// <summary>
		/// Enables or disables the Video Settings menu.
		/// </summary>
		/// <param name="enable">Show and enable or hide and disable the Video Settings menu.</param>
		void SetEnabled(bool enable = true) const;

		/// <summary>
		/// User input handling for the Video Settings menu.
		/// </summary>
		void HandleInputEvents(GUIEvent &guiEvent);
#pragma endregion

	private:

		/// <summary>
		/// 
		/// </summary>
		struct PresetResolutionRecord {
			int Width; //!<
			int Height; //!<
			bool Upscaled; //!<

			/// <summary>
			/// 
			/// </summary>
			PresetResolutionRecord(int width, int height, bool upscaled) : Width(width), Height(height), Upscaled(upscaled) {};

			/// <summary>
			/// Makes UI displayable string with resolution info.
			/// </summary>
			/// <returns>String with resolution info.</returns>
			std::string MakeResolutionString() const { return std::to_string(Width) + "x" + std::to_string(Height) + (Upscaled ? " Upscaled (" + std::to_string(Width / 2) + "x" + std::to_string(Height / 2) + ")" : ""); }

			/// <summary>
			/// 
			/// </summary>
			/// <param name="rhs"></param>
			/// <returns></returns>
			bool operator<(const PresetResolutionRecord &rhs) const {
				if (Width == rhs.Width && Height == rhs.Height) {
					return Upscaled != rhs.Upscaled;
				} else if (Width == rhs.Width) {
					return Height < rhs.Height;
				}
				return Width < rhs.Width;
			}
		};

		/// <summary>
		/// 
		/// </summary>
		struct CustomResolutionRecord {
			int Width; //!<
			int Height; //!<
			bool Upscaled; //!<
			int GraphicsDriver; //!<
		};

		GUIControlManager *m_GUIControlManager; //!< The GUIControlManager which holds all the GUIControls of this menu. Not owned by this.

		GUICollectionBox *m_VideoSettingsBox; //!< The GUICollectionBox that contains all the Audio Settings menu GUI elements.

		GUIButton *m_WindowedButton; //!<
		GUIButton *m_BorderlessButton; //!<
		GUIButton *m_UpscaledBorderlessButton; //!<
		GUIButton *m_DedicatedFullscreenButton; //!<

		GUICollectionBox *m_PresetResolutionBox; //!<
		GUIComboBox *m_PresetResolutionComboBox; //!<
		GUIButton *m_PresetResolutionApplyButton; //!<
		GUILabel *m_PresetResolutionMessageLabel; //!<
		std::set<PresetResolutionRecord> m_PresetResolutions; //!<

		GUICollectionBox *m_CustomResolutionBox; //!<

		GUICollectionBox *m_ResolutionChangeDialogBox; //!<
		GUIButton *m_ConfirmResolutionChangeButton; //!<
		GUIButton *m_ConfirmResolutionChangeFullscreenButton; //!<
		GUIButton *m_CancelResolutionChangeButton; //!<

		bool m_ResolutionChangeToUpscaled; //!<

#pragma region Video Settings Handling
		/// <summary>
		/// Updates the contents of the screen resolution combo box.
		/// </summary>
		void PopulateResolutionsComboBox();
#pragma endregion

		// Disallow the use of some implicit methods.
		SettingsVideoGUI(const SettingsVideoGUI &reference) = delete;
		SettingsVideoGUI & operator=(const SettingsVideoGUI &rhs) = delete;
	};
}
#endif