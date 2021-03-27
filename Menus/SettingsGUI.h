/*


		/// <summary>
		/// Updates the contents of the screen resolution combo box.
		/// </summary>
		void UpdateResolutionCombo();

		/// <summary>
		/// Updates the position of the volume sliders, based on what the AudioMan is currently set to.
		/// </summary>
		void UpdateVolumeSliders();


		enum OptionsButtons {
			FULLSCREENORWINDOWED = 0,
			P1NEXT,
			P2NEXT,
			P3NEXT,
			P4NEXT,
			P1PREV,
			P2PREV,
			P3PREV,
			P4PREV,
			P1CONFIG,
			P2CONFIG,
			P3CONFIG,
			P4CONFIG,
			P1CLEAR,
			P2CLEAR,
			P3CLEAR,
			P4CLEAR,
			UPSCALEDFULLSCREEN,
			OPTIONSBUTTONCOUNT
		};

		enum OptionsCheckboxes {
			FLASHONBRAINDAMAGE = 0,
			BLIPONREVEALUNSEEN,
			SHOWFOREIGNITEMS,
			SHOWTOOLTIPS,
			OPTIONSCHECKBOXCOUNT
		};


		enum OptionsLabels {
			P1DEVICE = 0,
			P2DEVICE,
			P3DEVICE,
			P4DEVICE,
			OPTIONSLABELCOUNT
		};

		enum OptionsFocus {
			MUSICVOLUME = 0,
			SOUNDVOLUME
		};

		enum GamepadType {
			DPAD = 0,
			DANALOG,
			XBOX360
		};

		enum ConfigSteps {
			KEYBOARDSTEPS = 16,
			MOUSESTEPS = 11,
			DPADSTEPS = 13,
			DANALOGSTEPS = 19,
			XBOX360STEPS = 19
		};

		enum ConfigLabels {
			CONFIGTITLE = 0,
			CONFIGRECOMMENDATION,
			CONFIGINSTRUCTION,
			CONFIGINPUT,
			CONFIGSTEPS,
			CONFIGLABELCOUNT
		};

		enum DeadZoneSliders {
			P1DEADZONESLIDER = 0,
			P2DEADZONESLIDER,
			P3DEADZONESLIDER,
			P4DEADZONESLIDER,
			DEADZONESLIDERCOUNT
		};


		/// <summary>
		/// Updates the text on the configuration labels, based on actual UInputMan settings.
		/// </summary>
		void UpdateDeviceLabels();

		/// <summary>
		/// Updates the contents of the control configuration screen.
		/// </summary>
		void UpdateConfigScreen();

		GUIButton *m_aOptionButton[OPTIONSBUTTONCOUNT]; //!< The options buttons
		GUILabel *m_aOptionsLabel[OPTIONSLABELCOUNT]; //!< Labels of the options screen
		GUILabel *m_aDeadZoneLabel[DEADZONESLIDERCOUNT]; //!< Labels of the options screen
		GUISlider *m_aDeadZoneSlider[DEADZONESLIDERCOUNT]; //!< Slider for dead zone controls
		GUICheckbox *m_aDeadZoneCheckbox[DEADZONESLIDERCOUNT]; //!< Checkboxes for dead zone controls
		GUICheckbox *m_aOptionsCheckbox[OPTIONSCHECKBOXCOUNT]; //!< Checkboxes of the options screen

		GUIComboBox *m_pResolutionCombo; //!< Resolution combobox

		//!< Option sound sliders
		GUILabel *m_pSoundLabel;
		GUILabel *m_pMusicLabel;
		GUISlider *m_pSoundSlider;
		GUISlider *m_pMusicSlider;

		GUIButton *m_pBackToOptionsButton; //!< Back to options from the test and config screens
		GUIButton *m_pSkipButton; //!< Skip button for config screen
		GUIButton *m_pDefaultButton; //!< Defualt button for config screen

		int m_ConfiguringPlayer; //!< Which player's control scheme we are currently configuring
		int m_ConfiguringDevice; //!< Which type of device we are currently configuring
		int m_ConfiguringGamepad; //!< Which type of gamepad we are currently configuring
		int m_ConfigureStep; //!< Which step in current configure sequence

		GUILabel *m_pConfigLabel[CONFIGLABELCOUNT]; //!< Labels of the control config screen

		//!< Controller diagram bitmaps
		BITMAP **m_aDPadBitmaps;
		BITMAP **m_aDualAnalogBitmaps;
		//!< Controller diagram panel
		GUICollectionBox *m_pRecommendationBox;
		GUICollectionBox *m_pRecommendationDiagram;

		GUIButton *m_pConfigSkipButton; //!< Skip forward one config step button
		GUIButton *m_pConfigBackButton; //!< Go back one config step button

		//!< Gamepad type selection UI elements
		GUICollectionBox *m_pDPadTypeBox;
		GUICollectionBox *m_pDAnalogTypeBox;
		GUICollectionBox *m_pXBox360TypeBox;
		GUICollectionBox *m_pDPadTypeDiagram;
		GUICollectionBox *m_pDAnalogTypeDiagram;
		GUICollectionBox *m_pXBox360TypeDiagram;
		GUIButton *m_pDPadTypeButton;
		GUIButton *m_pDAnalogTypeButton;
		GUIButton *m_pXBox360TypeButton;

		GUICollectionBox *m_ResolutionChangeDialog;
		GUIButton *m_ButtonConfirmResolutionChange;
		GUIButton *m_ButtonCancelResolutionChange;
		GUIButton *m_ButtonConfirmResolutionChangeFullscreen;
		bool m_ResolutionChangeToUpscaled;

		// Max available resolutions.
		int m_MaxResX;
		int m_MaxResY;







*/