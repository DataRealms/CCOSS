#ifndef _RTECONSTANTS_
#define _RTECONSTANTS_

namespace RTE {

#pragma region Type Definitions
	// TODO: Look into not using distinctive types for IDs.
	typedef int MOID; //!< Distinctive type definition for MovableObject IDs.
	typedef int MID; //!< Distinctive type definition for Material IDs.
#pragma endregion

#pragma region Userdata Constants
	static const std::string c_UserScenesModuleName = "UserScenes.rte"; //!< Module name where user created Scenes are saved.
	static const std::string c_UserScriptedSavesModuleName = "UserSavedGames.rte"; //!< Module name where user scripted Activity saves are saved.
	static const std::string c_UserConquestSavesModuleName = "UserSavesConquest.rte"; //!< Module name where user conquest saves are saved.
#pragma endregion

#pragma region Physics Constants
	static constexpr float c_PPM = 20.0F; //!< Pixels per Meter constant.
	static constexpr float c_MPP = 1.0F / c_PPM; //!< Meters per Pixel constant.
	static constexpr float c_PPL = 1000.0F / ((100.0F / c_PPM) * (100.0F / c_PPM) * (100.0F / c_PPM)); //!< Pixels per Liter constant.
	static constexpr float c_LPP = 1.0F / c_PPL; //!< Liters per Pixel constant.

	static constexpr int c_DefaultAtomGroupResolution = 5; //!< The global default AtomGroup resolution setting.
#pragma endregion

#pragma region Time Constants
	static constexpr float c_DefaultDeltaTimeS = 0.0166666F; //!< The default simulation update step size, in seconds.
	static constexpr float c_RealToSimCap = 0.0166666F; //!< The default cap of number of ticks that the real time can add to the tick accumulator each update.
#pragma endregion

#pragma region AI Constants
    static constexpr float c_PathFindingDefaultDigStrength = 35.0F; //!< A default pathfinder penetration value that'll allow pathing through corpses, debris, and such stuff.
#pragma endregion

#pragma region Graphics Constants
	static constexpr int c_DefaultResX = 960; //!< Default game window width.
	static constexpr int c_DefaultResY = 540; //!< Default game window height.

	static constexpr int c_MinResX = 640; //!< Minimum reasonable game window width.
	static constexpr int c_MinResY = 360; //!< Minimum reasonable game window height.

	static constexpr int c_ScenePreviewWidth = 170; //< Width of the scene preview bitmap.
	static constexpr int c_ScenePreviewHeight = 80; //< Height of the scene preview bitmap.

	static constexpr unsigned short c_MaxScreenCount = 4; //!< Maximum number of player screens.
	static constexpr unsigned short c_PaletteEntriesNumber = 256; //!< Number of indexes in the graphics palette.
	static constexpr unsigned short c_MOIDLayerBitDepth = 16; //!< Bit depth of MOID layer bitmap.
	static constexpr unsigned short c_GoldMaterialID = 2; //!< Index of gold material in the material palette.

	static constexpr int c_BlendAmountStep = 5; //!< The multiple to round blend amounts to when creating new color maps. This is to reduce the ridiculous amount of color map variants that can be created.

	enum ColorKeys {
		g_InvalidColor = -1,
		g_MaskColor = 0, //!< Mask color for all 8bpp bitmaps (palette index 0 (255,0,255)). This color is fully transparent.
		g_BlackColor = 245,
		g_WhiteColor = 254,
		g_RedColor = 13,
		g_GreenColor = 147,
		g_YellowGlowColor = 117,
		g_NoMOID = 255
	};

	enum MaterialColorKeys {
		g_MaterialAir = 0,
		g_MaterialOutOfBounds = 1,
		g_MaterialCavity = 1,
		g_MaterialGold = 2,
		g_MaterialSand = 8,
		g_MaterialGrass = 128,
		g_MaterialRubber = 176,
		g_MaterialDoor = 181
	};

	enum DotGlowColor { NoDot, YellowDot, RedDot, BlueDot };

	enum DrawBlendMode {
		NoBlend,
		BlendBurn,
		BlendColor,
		BlendDifference,
		BlendDissolve,
		BlendDodge,
		BlendInvert,
		BlendLuminance,
		BlendMultiply,
		BlendSaturation,
		BlendScreen,
		BlendTransparency,
		BlendModeCount
	};

	enum BlendAmountLimits { MinBlend = 0, MaxBlend = 100 };

	enum TransparencyPreset { LessTrans = 25, HalfTrans = 50, MoreTrans = 75 };

	enum SpriteAnimMode {
		NOANIM,
		ALWAYSLOOP,
		ALWAYSRANDOM,
		ALWAYSPINGPONG,
		LOOPWHENACTIVE,
		LOOPWHENOPENCLOSE,
		PINGPONGOPENCLOSE,
		OVERLIFETIME,
		ONCOLLIDE,
		SpriteAnimModeCount
	};

	// GUI colors
	#define c_GUIColorWhite makecol(255, 255, 255)
	#define c_GUIColorYellow makecol(255, 255, 128)
	#define c_GUIColorRed makecol(255, 100, 100)
	#define c_GUIColorGreen makecol(128, 255, 128)
	#define c_GUIColorCyan makecol(127, 255, 255)
	#define c_GUIColorLightBlue makecol(109, 117, 170)
	#define c_GUIColorBlue makecol(59, 65, 83)
	#define c_GUIColorDarkBlue makecol(12, 20, 39)
	#define c_GUIColorGray makecol(232, 232, 248)

	#define c_PlayerSlotColorDefault makecol(161, 109, 20)
	#define c_PlayerSlotColorHovered makecol(203, 130, 56)
	#define c_PlayerSlotColorDisabled makecol(104, 67, 15)
	static constexpr std::array<float, 16> c_Quad {
			1.0f, 1.0f, 1.0f, 0.0f,
			1.0f, -1.0f, 1.0f, 1.0f,
			-1.0f, 1.0f, 0.0f, 0.0f,
			-1.0f, -1.0f, 0.0f, 1.0f};
#pragma endregion

#pragma region Math Constants
	static constexpr float c_TwoPI = 6.28318531F;
	static constexpr float c_OneAndAHalfPI = 4.71238898F;
	static constexpr float c_PI = 3.14159265F;
	static constexpr float c_HalfPI = 1.57079633F;
	static constexpr float c_QuarterPI = 0.78539816F;
	static constexpr float c_EighthPI = 0.39269908F;
	static constexpr float c_SixteenthPI = 0.19634954F;
#pragma endregion

#pragma region Audio Constants
	static constexpr std::array<const char*, 3> c_SupportedAudioFormats = { ".wav", ".ogg", ".flac" };

	static constexpr unsigned short c_MaxSoftwareChannels = 128;
	static constexpr unsigned short c_MaxVirtualChannels = 1024;
	static constexpr unsigned short c_MaxPlayingSoundsPerContainer = 128;
	/* TODO These have been temporarily replaced with variables in settingsman to allow for easy tweaking. This needs to be undone once our soundscape is sorted out.
	static constexpr unsigned short c_ListenerZOffset = 50; //!< The Z offset for Audio listeners. Can be used to lessen harsh panning if panning effect strength is at max.
	static constexpr unsigned short c_MinimumDistanceForPanning = 50; //!< The minimum distance before which sound panning should not occur. Not relevant for immobile sounds or in splitscreen.
	*/
	static constexpr unsigned short c_DefaultAttenuationStartDistance = 100; //!< The default start distance for attenuating sounds. Individual sounds can have different values for this.
	static constexpr unsigned int c_SoundMaxAudibleDistance = 10000; //!< The maximum distance at which any sound can possibly be heard, after which point it will have 0 volume. Arbitrary default suggested by FMOD.
#pragma endregion

#pragma region Network Constants
	static constexpr unsigned short c_MaxClients = 4;
	static constexpr unsigned short c_FramesToRemember = 2;
	static constexpr unsigned short c_MaxLayersStoredForNetwork = 5;
	static constexpr unsigned short c_MaxPixelLineBufferSize = 8192;

	// Defaults are picked so that if the box can't be compressed it should somewhat fit into one UDP packet.
	// Reducing box area introduces slight overhead due to more messages and hence more headers being sent.
	// Box area must be divisible by 8 bytes for copying reasons.
	static constexpr int c_ServerDefaultBoxWidth = 32;
	static constexpr int c_ServerDefaultBoxHeight = 44;
#pragma endregion

#pragma region Input Constants
	/// <summary>
	/// Enumeration for different types of input devices.
	/// </summary>
	enum InputDevice {
		DEVICE_KEYB_ONLY = 0,
		DEVICE_MOUSE_KEYB,
		DEVICE_GAMEPAD_1,
		DEVICE_GAMEPAD_2,
		DEVICE_GAMEPAD_3,
		DEVICE_GAMEPAD_4,
		DEVICE_COUNT
	};

	/// <summary>
	/// Enumeration for different elements the input scheme is composed of.
	/// </summary>
	enum InputElements {
		INPUT_L_UP = 0,
		INPUT_L_DOWN,
		INPUT_L_LEFT,
		INPUT_L_RIGHT,
		INPUT_AIM_UP,
		INPUT_AIM_DOWN,
		INPUT_AIM_LEFT,
		INPUT_AIM_RIGHT,
		INPUT_FIRE,
		INPUT_AIM,
		INPUT_PIEMENU_ANALOG,
		INPUT_PIEMENU_DIGITAL,
		INPUT_JUMP,
		INPUT_CROUCH,
		INPUT_NEXT,
		INPUT_PREV,
		INPUT_WEAPON_CHANGE_NEXT,
		INPUT_WEAPON_CHANGE_PREV,
		INPUT_WEAPON_PICKUP,
		INPUT_WEAPON_DROP,
		INPUT_WEAPON_RELOAD,
		INPUT_START,
		INPUT_BACK,
		INPUT_R_UP,
		INPUT_R_DOWN,
		INPUT_R_LEFT,
		INPUT_R_RIGHT,
		INPUT_COUNT
	};

	static const std::array<const std::string_view, InputElements::INPUT_COUNT> c_InputElementNames = {
		"Move Up",			// INPUT_L_UP
		"Move Down",		// INPUT_L_DOWN
		"Move Left",		// INPUT_L_LEFT
		"Move Right",		// INPUT_L_RIGHT
		"Aim Up",			// INPUT_AIM_UP
		"Aim Down",			// INPUT_AIM_DOWN
		"Aim Left",			// INPUT_AIM_LEFT
		"Aim Right",		// INPUT_AIM_RIGHT
		"Fire/Activate",	// INPUT_FIRE
		"Sharp Aim",		// INPUT_AIM
		"Pie Menu Analog",	// INPUT_PIEMENU_ANALOG
		"Pie Menu Digital",	// INPUT_PIEMENU_DIGITAL
		"Jump",				// INPUT_JUMP
		"Crouch",			// INPUT_CROUCH
		"Next Body",		// INPUT_NEXT
		"Prev. Body",		// INPUT_PREV
		"Next Device",		// INPUT_WEAPON_CHANGE_NEXT
		"Prev. Device",		// INPUT_WEAPON_CHANGE_PREV
		"Pick Up Device",	// INPUT_WEAPON_PICKUP
		"Drop Device",		// INPUT_WEAPON_DROP
		"Reload Weapon",	// INPUT_WEAPON_RELOAD
		"Start",			// INPUT_START
		"Back",				// INPUT_BACK
		"Analog Aim Up",	// INPUT_R_UP
		"Analog Aim Down",	// INPUT_R_DOWN
		"Analog Aim Left",	// INPUT_R_LEFT
		"Analog Aim Right"	// INPUT_R_RIGHT
	};

	/// <summary>
	/// Enumeration for mouse button types.
	/// </summary>
	enum MouseButtons {
		MOUSE_NONE = -1,
		MOUSE_LEFT = 1,
		MOUSE_MIDDLE,
		MOUSE_RIGHT,
		MAX_MOUSE_BUTTONS
	};

	/// <summary>
	/// Enumeration for joystick button types.
	/// </summary>
	enum JoyButtons {
		JOY_NONE = -1,
		JOY_1 = 0,
		JOY_2,
		JOY_3,
		JOY_4,
		JOY_5,
		JOY_6,
		JOY_7,
		JOY_8,
		JOY_9,
		JOY_10,
		JOY_11,
		JOY_12,
		JOY_13,
		JOY_15,
		JOY_16,
		JOY_17,
		JOY_18,
		JOY_19,
		JOY_20,
		JOY_21,
		JOY_22,
		MAX_JOY_BUTTONS
	};

	/// <summary>
	/// Enumeration for joystick direction types.
	/// </summary>
	enum JoyDirections { JOYDIR_ONE = 0, JOYDIR_TWO };

	/// <summary>
	/// Enumeration for joystick dead zone types.
	/// Square deadzone cuts-off any input from every axis separately. For example if x-axis has less than 20% input and y-axis has more, x-axis input is ignored.
	/// Circle uses a round zone to capture stick position on both axis then cut-off if this position is inside the round dead zone.
	/// </summary>
	enum DeadZoneType { CIRCLE = 0, SQUARE = 1 };
#pragma endregion

#pragma region Global Enumerations
	/// <summary>
	/// Enumeration all available players.
	/// </summary>
	enum Players {
		NoPlayer = -1,
		PlayerOne = 0,
		PlayerTwo,
		PlayerThree,
		PlayerFour,
		MaxPlayerCount
	};

	/// <summary>
	/// Enumeration and supporting maps for cardinal directions, as well as None and Any.
	/// </summary>
	enum Directions { None = -1, Up, Down, Left, Right, Any };

	static const std::unordered_map<std::string_view, Directions> c_DirectionNameToDirectionsMap = {
		{"None", Directions::None},
		{"Up", Directions::Up},
		{"Down", Directions::Down},
		{"Left", Directions::Left},
		{"Right", Directions::Right},
		{"Any", Directions::Any}
	};

	static const std::unordered_map<Directions, const float> c_DirectionsToRadiansMap = {
		{Directions::Up, c_HalfPI},
		{Directions::Down, c_OneAndAHalfPI},
		{Directions::Left, c_PI},
		{Directions::Right, 0.0F}
	};
#pragma endregion

#pragma region Un-Definitions
	// Allegro defines these via define in astdint.h and Boost with stdlib go crazy so we need to undefine them manually.
	#undef int8_t
	#undef uint8_t
	#undef int16_t
	#undef uint16_t
	#undef int32_t
	#undef uint32_t
	#undef intptr_t
	#undef uintptr_t
	#undef LONG_LONG
	#undef int64_t
	#undef uint64_t
#pragma endregion
}
#endif
