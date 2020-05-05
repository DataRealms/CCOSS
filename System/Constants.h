#ifndef _RTECONSTANTS_
#define _RTECONSTANTS_

namespace RTE {

#pragma region Type Definitions
	// TODO: Look into not using distinctive types for IDs.
	typedef int MOID; //!< Distinctive type definition for MovableObject IDs.
	typedef int MID; //!< Distinctive type definition for Material IDs.
#pragma endregion

#pragma region Graphics Constants
	static constexpr unsigned short c_MaxScreenCount = 4; //!< Maximum number of player screens.
	static constexpr unsigned short c_PaletteEntriesNumber = 256; //!< Number of indexes in the graphics palette.
	static constexpr unsigned short c_MOIDLayerBitDepth = 16; //!< Bit depth of MOID layer bitmap.
	static constexpr unsigned short c_GoldMaterialID = 2; //!< Index of gold material in the material palette.

	enum ColorKeys {
		g_MaskColor = 0, //!< Mask color for all 8bpp bitmaps (palette index 0 (255,0,255)). This color is fully transparent.
		//g_MOIDMaskColor = 0, //!< Mask color for 8bpp MOID layer bitmaps (palette index 0 (255,0,255)). This color is fully transparent.
		g_MOIDMaskColor = 0xF81F, //!< Mask color for 16bpp MOID layer bitmaps (255,0,255). This color is fully transparent.
		//g_MOIDMaskColor = 0xFF00FF, //!< Mask color for 32bpp MOID layer bitmaps (255,0,255). This color is fully transparent.
		g_BlackColor = 245,
		g_WhiteColor = 254,
		g_RedColor = 13,
		g_YellowGlowColor = 117,
		g_NoMOID = 255
	};

	enum DotGlowColor { NoDot = 0, YellowDot, RedDot, BlueDot };
	enum TransparencyPreset { LessTrans = 0, HalfTrans, MoreTrans };

	// GUI colors
	#define c_GUIColorWhite makecol(255, 255, 255)
	#define c_GUIColorYellow makecol(255, 255, 128)
	#define c_GUIColorRed makecol(255, 100, 100)
	#define c_GUIColorGreen makecol(128, 255, 128)
	#define c_GUIColorLightBlue makecol(109, 117, 170)
	#define c_GUIColorBlue makecol(59, 65, 83)
	#define c_GUIColorDarkBlue makecol(12, 20, 39)
	#define c_GUIColorGray makecol(232, 232, 248)

	#define c_PlayerSlotColorDefault makecol(161, 109, 20) 
	#define c_PlayerSlotColorHovered makecol(203, 130, 56) 
	#define c_PlayerSlotColorDisabled makecol(104, 67, 15)
#pragma endregion

#pragma region Math Constants
	static constexpr float c_TwoPI = 6.2831853F;
	static constexpr float c_PI = 3.1415926F;
	static constexpr float c_HalfPI = 1.5707963F;
	static constexpr float c_QuarterPI = 0.7853982F;
	static constexpr float c_EighthPI = 0.3926991F;
	static constexpr float c_SixteenthPI = 0.1963495F;
#pragma endregion

#pragma region Audio Constants
	static constexpr unsigned short c_MaxSoftwareChannels = 128;
	static constexpr unsigned short c_MaxVirtualChannels = 1024;
	static constexpr unsigned short c_MaxPlayingSoundsPerContainer = 64;
	/* TODO These have been temporarily replaced with variables in settingsman to allow for easy tweaking. This needs to be undone once our soundscape is sorted out.
	static constexpr unsigned short c_ListenerZOffset = 50; //!< The Z offset for Audio listeners. Can be used to lessen harsh panning if panning effect strength is at max.
	static constexpr unsigned short c_MinimumDistanceForPanning = 50; //!< The minimum distance before which sound panning should not occur. Not relevant for immobile sounds or in splitscreen.
	*/
	static constexpr unsigned short c_DefaultAttenuationStartDistance = 100; //!< The default start distance for attenuating sounds. Individual sounds can have different values for this.
	static constexpr unsigned int c_SoundMaxAudibleDistance = 100000; //!< The maximum distance at which any sound can possibly be heard, after which point it will have 0 volume. Arbitrary default suggested by FMOD.
#pragma endregion

#pragma region Network Constants
	static constexpr unsigned short c_MaxClients = 4;
	static constexpr unsigned short c_MaxLayersStoredForNetwork = 10;
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