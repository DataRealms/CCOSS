#ifndef _RTESETTINGSVIDEOGUI_
#define _RTESETTINGSVIDEOGUI_

namespace RTE {

	class GUIControlManager;
	class GUICollectionBox;
	class GUIComboBox;
	class GUILabel;
	class GUIButton;
	class GUIRadioButton;
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
		enum class ResolutionChangeType {
			Windowed,
			Borderless,
			UpscaledBorderless,
			Dedicated,
			UpscaledDedicated,
			ManuallyDefined
		};

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

		GUIControlManager *m_GUIControlManager; //!< The GUIControlManager which holds all the GUIControls of this menu. Not owned by this.

		GUICollectionBox *m_VideoSettingsBox; //!< The GUICollectionBox that contains all the Audio Settings menu GUI elements.

		GUIButton *m_WindowedButton; //!<
		GUIButton *m_BorderlessButton; //!<
		GUIButton *m_UpscaledBorderlessButton; //!<
		GUIButton *m_DedicatedButton; //!<
		GUIButton *m_UpscaledDedicatedButton; //!<

		GUIRadioButton *m_PresetResolutionRadioButton; //!<
		GUIRadioButton *m_CustomResolutionRadioButton; //!<

		GUICollectionBox *m_PresetResolutionBox; //!<
		GUIComboBox *m_PresetResolutionComboBox; //!<
		GUIButton *m_PresetResolutionApplyButton; //!<
		GUILabel *m_PresetResolutionMessageLabel; //!<
		std::vector<PresetResolutionRecord> m_PresetResolutions; //!<

		GUICollectionBox *m_CustomResolutionBox; //!<
		GUIButton *m_CustomResolutionApplyButton; //!<

		GUICollectionBox *m_ResolutionChangeDialogBox; //!<
		GUIButton *m_ConfirmResolutionChangeButton; //!<
		GUIButton *m_CancelResolutionChangeButton; //!<

		int m_NewGraphicsDriver;
		int m_NewResX; //!< New game window width that will take effect next time the FrameMan is started.
		int m_NewResY; //!< New game window height that will take effect next time the FrameMan is started.
		bool m_NewResUpscaled; //!< New window width/height multiple that will take effect next time the FrameMan is started.

#pragma region Video Settings Handling
		/// <summary>
		/// Checks whether the passed in width and height values make a supported resolution setting.
		/// </summary>
		/// <param name="width">Resolution width.</param>
		/// <param name="height">Resolution height.</param>
		/// <returns>Whether the resolution is supported or not.</returns>
		bool IsSupportedResolution(int width, int height) const;

		/// <summary>
		/// Updates the contents of the screen resolution combo box.
		/// </summary>
		void PopulateResolutionsComboBox();

		/// <summary>
		/// 
		/// </summary>
		/// <param name="resolutionChangeType"></param>
		/// <param name="newResX"></param>
		/// <param name="newResY"></param>
		/// <param name="newResMultiplier"></param>
		/// <param name="newGfxDriver"></param>
		void SetNewResolutionProperties(ResolutionChangeType resolutionChangeType, int newResX = 960, int newResY = 540, bool newResUpscaled = false, int newGfxDriver = 2);

		/// <summary>
		/// 
		/// </summary>
		/// <param name="presetResolutionRecord"></param>
		void SetNewResolutionPropertiesFromPreset(PresetResolutionRecord presetResolutionRecord) { SetNewResolutionProperties(ResolutionChangeType::ManuallyDefined, presetResolutionRecord.Width, presetResolutionRecord.Height, presetResolutionRecord.Upscaled); }

		/// <summary>
		/// 
		/// </summary>
		void ApplyNewResolution();
#pragma endregion

		// Disallow the use of some implicit methods.
		SettingsVideoGUI(const SettingsVideoGUI &reference) = delete;
		SettingsVideoGUI & operator=(const SettingsVideoGUI &rhs) = delete;
	};
}
#endif