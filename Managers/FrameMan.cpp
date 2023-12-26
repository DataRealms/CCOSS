#include "FrameMan.h"

#include "WindowMan.h"
#include "PostProcessMan.h"
#include "PresetMan.h"
#include "PrimitiveMan.h"
#include "PerformanceMan.h"
#include "ActivityMan.h"
#include "CameraMan.h"
#include "ConsoleMan.h"
#include "SettingsMan.h"
#include "MovableMan.h"
#include "ThreadMan.h"
#include "UInputMan.h"

#include "SLTerrain.h"
#include "SLBackground.h"
#include "Scene.h"

#include "GUI.h"
#include "AllegroBitmap.h"
#include "AllegroScreen.h"

#include "GLCheck.h"
#include "glad/gl.h"

#include "tracy/Tracy.hpp"

namespace RTE {

	void BitmapDeleter::operator()(BITMAP *bitmap) const { destroy_bitmap(bitmap); }

	const std::array<std::function<void(int r, int g, int b, int a)>, DrawBlendMode::BlendModeCount> FrameMan::c_BlenderSetterFunctions = {
		nullptr, // NoBlend obviously has no blender, but we want to keep the indices matching with the enum.
		&set_burn_blender,
		&set_color_blender,
		&set_difference_blender,
		&set_dissolve_blender,
		&set_dodge_blender,
		&set_invert_blender,
		&set_luminance_blender,
		&set_multiply_blender,
		&set_saturation_blender,
		&set_screen_blender,
		nullptr // Transparency does not rely on the blender setting, it creates a map with the dedicated function instead of with the generic one.
	};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void FrameMan::Clear() {
		m_HSplit = false;
		m_VSplit = false;
		m_TwoPlayerVSplit = false;
		m_PlayerScreen.reset();
		m_PlayerScreenWidth = 0;
		m_PlayerScreenHeight = 0;
		m_ScreenDumpBuffer.reset();
		m_WorldDumpBuffer.reset();
		m_ScenePreviewDumpGradient.reset();
		m_ScreenDumpNamePlaceholder.reset();
		m_BackBuffer8.reset();
		m_BackBuffer32.reset();
		m_OverlayBitmap32.reset();
		m_DrawNetworkBackBuffer = false;
		m_StoreNetworkBackBuffer = false;
		m_NetworkFrameCurrent = 0;
		m_NetworkFrameReady = 1;
		m_PaletteFile = ContentFile("Base.rte/palette.bmp");
		m_BlackColor = 245;
		m_AlmostBlackColor = 245;
		m_ColorTablePruneTimer.Reset();
		m_GUIScreens.fill(nullptr);
		m_LargeFonts.fill(nullptr);
		m_SmallFonts.fill(nullptr);
		m_TextBlinkTimer.Reset();

		for (int screenCount = 0; screenCount < c_MaxScreenCount; ++screenCount) {
			m_ScreenText[screenCount].clear();
			m_TextDuration[screenCount] = -1;
			m_TextDurationTimer[screenCount].Reset();
			m_TextBlinking[screenCount] = 0;
			m_TextCentered[screenCount] = false;
			m_FlashScreenColor[screenCount] = -1;
			m_FlashedLastFrame[screenCount] = false;
			m_FlashTimer[screenCount].Reset();

			for (int bufferFrame = 0; bufferFrame < 2; bufferFrame++) {
				m_NetworkBackBufferIntermediate8[bufferFrame][screenCount].reset();
				m_NetworkBackBufferFinal8[bufferFrame][screenCount].reset();
				m_NetworkBackBufferIntermediateGUI8[bufferFrame][screenCount].reset();
				m_NetworkBackBufferFinalGUI8[bufferFrame][screenCount].reset();
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int FrameMan::Initialize() {
		set_color_depth(c_BPP);
		// Sets the allowed color conversions when loading bitmaps from files
		set_color_conversion(COLORCONV_MOST);

		LoadPalette(m_PaletteFile.GetDataPath());

		// Store the default palette for re-use when creating new color tables for different blend modes because the palette can be changed via scripts, and handling per-palette per-mode color tables is too much headache.
		get_palette(m_DefaultPalette);

		CreatePresetColorTables();
		SetTransTableFromPreset(TransparencyPreset::HalfTrans);
		CreateBackBuffers();

		ContentFile scenePreviewGradientFile("Base.rte/GUIs/PreviewSkyGradient.png");
		m_ScenePreviewDumpGradient = std::unique_ptr<BITMAP, BitmapDeleter>(scenePreviewGradientFile.GetAsBitmap(COLORCONV_8_TO_32, false));

		m_ScreenDumpNamePlaceholder = std::unique_ptr<BITMAP, BitmapDeleter>(create_bitmap_ex(24, 1, 1));
		clear_bitmap(m_ScreenDumpNamePlaceholder.get());

		// Use fastest compression in save_png().
		_png_compression_level = 1;

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int FrameMan::CreateBackBuffers() {
		int resX = g_WindowMan.GetResX();
		int resY = g_WindowMan.GetResY();

		// Create the back buffer, this is still in 8bpp, we will do any post-processing on the PostProcessing bitmap
		m_BackBuffer8 = std::unique_ptr<BITMAP, BitmapDeleter>(create_bitmap_ex(8, resX, resY));
		ClearBackBuffer8();

		// Create the post-processing buffer, it'll be used for glow effects etc
		m_BackBuffer32 = std::unique_ptr<BITMAP, BitmapDeleter>(create_bitmap_ex(c_BPP, resX, resY));
		ClearBackBuffer32();

		m_OverlayBitmap32 = std::unique_ptr<BITMAP, BitmapDeleter>(create_bitmap_ex(c_BPP, resX, resY));
		clear_to_color(m_OverlayBitmap32.get(), 0);

		// Create all the network 8bpp back buffers
		for (int i = 0; i < c_MaxScreenCount; i++) {
			for (int f = 0; f < 2; f++) {
				m_NetworkBackBufferIntermediate8[f][i] = std::unique_ptr<BITMAP, BitmapDeleter>(create_bitmap_ex(8, resX, resY));
				clear_to_color(m_NetworkBackBufferIntermediate8[f][i].get(), m_BlackColor);

				m_NetworkBackBufferIntermediateGUI8[f][i] = std::unique_ptr<BITMAP, BitmapDeleter>(create_bitmap_ex(8, resX, resY));
				clear_to_color(m_NetworkBackBufferIntermediateGUI8[f][i].get(), g_MaskColor);

				m_NetworkBackBufferFinal8[f][i] = std::unique_ptr<BITMAP, BitmapDeleter>(create_bitmap_ex(8, resX, resY));
				clear_to_color(m_NetworkBackBufferFinal8[f][i].get(), m_BlackColor);

				m_NetworkBackBufferFinalGUI8[f][i] = std::unique_ptr<BITMAP, BitmapDeleter>(create_bitmap_ex(8, resX, resY));
				clear_to_color(m_NetworkBackBufferFinalGUI8[f][i].get(), g_MaskColor);
			}
		}

		m_PlayerScreenWidth = m_BackBuffer8->w;
		m_PlayerScreenHeight = m_BackBuffer8->h;

		// Create the splitscreen buffer
		if (m_HSplit || m_VSplit) {
			m_PlayerScreen = std::unique_ptr<BITMAP, BitmapDeleter>(create_bitmap_ex(8, resX / (m_VSplit ? 2 : 1), resY / (m_HSplit ? 2 : 1)));
			clear_to_color(m_PlayerScreen.get(), m_BlackColor);
			set_clip_state(m_PlayerScreen.get(), 1);

			// Update these to represent the split screens
			m_PlayerScreenWidth = m_PlayerScreen->w;
			m_PlayerScreenHeight = m_PlayerScreen->h;
		}

		m_ScreenDumpBuffer = std::unique_ptr<BITMAP, BitmapDeleter>(create_bitmap_ex(24, m_BackBuffer32->w, m_BackBuffer32->h));

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void FrameMan::CreatePresetColorTables() {
		// Create RGB lookup table that supposedly speeds up calculation of other color tables.
		create_rgb_table(&m_RGBTable, m_DefaultPalette, nullptr);
		rgb_map = &m_RGBTable;

		// Create transparency color tables. Tables for other blend modes will be created on demand.
		int transparencyPresetCount = BlendAmountLimits::MaxBlend / c_BlendAmountStep;
		for (int index = 0; index <= transparencyPresetCount; ++index) {
			int presetBlendAmount = index * c_BlendAmountStep;
			std::array<int, 4> colorChannelBlendAmounts = { presetBlendAmount, presetBlendAmount, presetBlendAmount, BlendAmountLimits::MinBlend };
			int adjustedBlendAmount = 255 - (static_cast<int>(255.0F * (1.0F / static_cast<float>(transparencyPresetCount) * static_cast<float>(index))));

			m_ColorTables.at(DrawBlendMode::BlendTransparency).try_emplace(colorChannelBlendAmounts);
			create_trans_table(&m_ColorTables[DrawBlendMode::BlendTransparency].at(colorChannelBlendAmounts).first, m_DefaultPalette, adjustedBlendAmount, adjustedBlendAmount, adjustedBlendAmount, nullptr);
			m_ColorTables[DrawBlendMode::BlendTransparency].at(colorChannelBlendAmounts).second = -1;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void FrameMan::Destroy() {
		for (const GUIScreen *guiScreen : m_GUIScreens) {
			delete guiScreen;
		}
		for (const GUIFont *guiFont : m_LargeFonts) {
			delete guiFont;
		}
		for (const GUIFont *guiFont : m_SmallFonts) {
			delete guiFont;
		}
		Clear();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void FrameMan::Update() {
		// Remove all scheduled primitives, those will be re-added by updates from other entities.
		// This needs to happen here, otherwise if there are multiple sim updates during a single frame duplicates will be added to the primitive queue.
		g_PrimitiveMan.ClearPrimitivesQueue();

		// Prune unused color tables every 5 real minutes to prevent ridiculous memory usage over time.
		if (m_ColorTablePruneTimer.IsPastRealMS(300000)) {
			long long currentTime = g_TimerMan.GetAbsoluteTime() / 10000;
			for (std::unordered_map<std::array<int, 4>, std::pair<COLOR_MAP, long long>> &colorTableMap : m_ColorTables) {
				if (colorTableMap.size() >= 100) {
					std::vector<std::array<int, 4>> markedForDelete;
					markedForDelete.reserve(colorTableMap.size());
					for (const auto &[tableKey, tableData] : colorTableMap) {
						long long lastAccessTime = tableData.second;
						// Mark tables that haven't been accessed in the last minute for deletion. Avoid marking the transparency table presets, those will have lastAccessTime set to -1.
						if (lastAccessTime != -1 && (currentTime - lastAccessTime > 60)) { markedForDelete.emplace_back(tableKey); }
					}
					for (const std::array<int, 4> &keyToDelete : markedForDelete) {
						colorTableMap.erase(keyToDelete);
					}
				}
			}
			m_ColorTablePruneTimer.Reset();
		}

		// Update redundantly in sim update to ensure our values are exactly precise for the purposes of script GetOffset()
		int screenCount = (m_HSplit ? 2 : 1) * (m_VSplit ? 2 : 1);
		for (int playerScreen = 0; playerScreen < screenCount; ++playerScreen) {
			g_CameraMan.Update(playerScreen);
		}

		// Queue our MO renders
		Vector targetPos{};
		g_MovableMan.Draw(nullptr, targetPos);

		// TODO_MULTITHREAD
		//g_MovableMan.DrawHUD(nullptr, targetPos, playerScreen);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void FrameMan::ResetSplitScreens(bool hSplit, bool vSplit) {
		if (m_PlayerScreen) { release_bitmap(m_PlayerScreen.get()); }

		// Override screen splitting according to settings if needed
		if ((hSplit || vSplit) && !(hSplit && vSplit) && m_TwoPlayerVSplit) {
			hSplit = false;
			vSplit = m_TwoPlayerVSplit;
		}
		m_HSplit = hSplit;
		m_VSplit = vSplit;

		// Create the splitscreen buffer
		if (m_HSplit || m_VSplit) {
			m_PlayerScreen = std::unique_ptr<BITMAP, BitmapDeleter>(create_bitmap_ex(8, g_WindowMan.GetResX() / (m_VSplit ? 2 : 1), g_WindowMan.GetResY() / (m_HSplit ? 2 : 1)));
			clear_to_color(m_PlayerScreen.get(), m_BlackColor);
			set_clip_state(m_PlayerScreen.get(), 1);

			m_PlayerScreenWidth = m_PlayerScreen->w;
			m_PlayerScreenHeight = m_PlayerScreen->h;
		} else {
			// No splits, so set the screen dimensions equal to the back buffer
			m_PlayerScreenWidth = m_BackBuffer8->w;
			m_PlayerScreenHeight = m_BackBuffer8->h;
		}
		for (int i = 0; i < c_MaxScreenCount; ++i) {
			m_FlashScreenColor[i] = -1;
			m_FlashedLastFrame[i] = false;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Vector FrameMan::GetMiddleOfPlayerScreen(int whichPlayer) {
		Vector middleOfPlayerScreen;

		if (whichPlayer == -1 || IsInMultiplayerMode()) {
			middleOfPlayerScreen.SetXY(static_cast<float>(g_WindowMan.GetResX() / 2), static_cast<float>(g_WindowMan.GetResY() / 2));
		} else {
			int playerScreen = g_ActivityMan.GetActivity()->ScreenOfPlayer(whichPlayer);

			middleOfPlayerScreen.SetXY(static_cast<float>(m_PlayerScreenWidth / 2), static_cast<float>(m_PlayerScreenHeight / 2));
			if ((playerScreen == 1 && g_FrameMan.GetVSplit()) || playerScreen == 3) {
				middleOfPlayerScreen.SetX(middleOfPlayerScreen.GetX() + static_cast<float>(m_PlayerScreenWidth));
			}
			if ((playerScreen == 1 && g_FrameMan.GetHSplit()) || playerScreen >= 2) {
				middleOfPlayerScreen.SetY(middleOfPlayerScreen.GetY() + static_cast<float>(m_PlayerScreenHeight));
			}
		}
		return middleOfPlayerScreen;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int FrameMan::GetPlayerFrameBufferWidth(int whichPlayer) const {
		if (IsInMultiplayerMode()) {
			if (whichPlayer < 0 || whichPlayer >= c_MaxScreenCount) {
				int width = g_WindowMan.GetResX();
				for (int i = 0; i < c_MaxScreenCount; i++) {
					if (m_NetworkBackBufferFinal8[m_NetworkFrameReady][i] && (m_NetworkBackBufferFinal8[m_NetworkFrameReady][i]->w < width)) {
						width = m_NetworkBackBufferFinal8[m_NetworkFrameReady][i]->w;
					}
				}
				return width;
			} else {
				if (m_NetworkBackBufferFinal8[m_NetworkFrameReady][whichPlayer]) {
					return m_NetworkBackBufferFinal8[m_NetworkFrameReady][whichPlayer]->w;
				}
			}
		}
		return m_PlayerScreenWidth;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int FrameMan::GetPlayerFrameBufferHeight(int whichPlayer) const {
		if (IsInMultiplayerMode()) {
			if (whichPlayer < 0 || whichPlayer >= c_MaxScreenCount) {
				int height = g_WindowMan.GetResY();
				for (int i = 0; i < c_MaxScreenCount; i++) {
					if (m_NetworkBackBufferFinal8[m_NetworkFrameReady][i] && (m_NetworkBackBufferFinal8[m_NetworkFrameReady][i]->h < height)) {
						height = m_NetworkBackBufferFinal8[m_NetworkFrameReady][i]->h;
					}
				}
				return height;
			} else {
				if (m_NetworkBackBufferFinal8[m_NetworkFrameReady][whichPlayer]) {
					return m_NetworkBackBufferFinal8[m_NetworkFrameReady][whichPlayer]->h;
				}
			}
		}
		return m_PlayerScreenHeight;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int FrameMan::CalculateTextHeight(const std::string &text, int maxWidth, bool isSmall) {
		return isSmall ? GetSmallFont()->CalculateHeight(text, maxWidth) : GetLargeFont()->CalculateHeight(text, maxWidth);
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	std::string FrameMan::SplitStringToFitWidth(const std::string &stringToSplit, int widthLimit, bool useSmallFont) {
		GUIFont *fontToUse = GetFont(useSmallFont, false);
		auto SplitSingleLineAsNeeded = [this, &widthLimit, &fontToUse](std::string &lineToSplitAsNeeded) {
			int numberOfScreenWidthsForText = static_cast<int>(std::ceil(static_cast<float>(fontToUse->CalculateWidth(lineToSplitAsNeeded)) / static_cast<float>(widthLimit)));
			if (numberOfScreenWidthsForText > 1) {
				int splitInterval = static_cast<int>(std::ceil(static_cast<float>(lineToSplitAsNeeded.size()) / static_cast<float>(numberOfScreenWidthsForText)));
				for (int i = 1; i <= numberOfScreenWidthsForText; i++) {
					size_t newLineCharacterPosition = std::min(static_cast<size_t>(i * splitInterval + (i - 1)), lineToSplitAsNeeded.size());
					if (newLineCharacterPosition == lineToSplitAsNeeded.size()) {
						break;
					}
					lineToSplitAsNeeded.insert(newLineCharacterPosition, "\n");
				}
			}
		};

		std::string splitString;
		size_t previousNewLinePos = 0;
		size_t nextNewLinePos = stringToSplit.find("\n");
		if (nextNewLinePos != std::string::npos) {
			while (nextNewLinePos != std::string::npos) {
				nextNewLinePos = stringToSplit.find("\n", previousNewLinePos);
				std::string currentLine = stringToSplit.substr(previousNewLinePos, nextNewLinePos - previousNewLinePos);
				previousNewLinePos = nextNewLinePos + 1;

				SplitSingleLineAsNeeded(currentLine);
				splitString += currentLine;
				if (nextNewLinePos != std::string::npos) {
					splitString += "\n";
				}
			}
		} else {
			splitString = stringToSplit;
			SplitSingleLineAsNeeded(splitString);
		}

		return splitString;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int FrameMan::CalculateTextWidth(const std::string &text, bool isSmall) {
		return isSmall ? GetSmallFont()->CalculateWidth(text) : GetLargeFont()->CalculateWidth(text);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void FrameMan::SetScreenText(const std::string &message, int whichScreen, int blinkInterval, int displayDuration, bool centered) {
		// See if we can overwrite the previous message
		if (whichScreen >= 0 && whichScreen < c_MaxScreenCount && m_TextDurationTimer[whichScreen].IsPastRealMS(m_TextDuration[whichScreen])) {
			m_ScreenText[whichScreen] = message;
			m_TextDuration[whichScreen] = displayDuration;
			m_TextDurationTimer[whichScreen].Reset();
			m_TextBlinking[whichScreen] = blinkInterval;
			m_TextCentered[whichScreen] = centered;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void FrameMan::ClearScreenText(int whichScreen) {
		if (whichScreen >= 0 && whichScreen < c_MaxScreenCount) {
			m_ScreenText[whichScreen].clear();
			m_TextDuration[whichScreen] = -1;
			m_TextDurationTimer[whichScreen].Reset();
			m_TextBlinking[whichScreen] = 0;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void FrameMan::SetColorTable(DrawBlendMode blendMode, std::array<int, 4> colorChannelBlendAmounts) {
		RTEAssert(blendMode > DrawBlendMode::NoBlend && blendMode < DrawBlendMode::BlendModeCount, "Invalid DrawBlendMode or DrawBlendMode::NoBlend passed into FrameMan::SetColorTable. See DrawBlendMode enumeration for defined values.");

		for (int &colorChannelBlendAmount : colorChannelBlendAmounts) {
			colorChannelBlendAmount = RoundToNearestMultiple(std::clamp(colorChannelBlendAmount, static_cast<int>(BlendAmountLimits::MinBlend), static_cast<int>(BlendAmountLimits::MaxBlend)), c_BlendAmountStep);
		}

		bool usedPresetTransparencyTable = false;

		switch (blendMode) {
			case DrawBlendMode::NoBlend:
				RTEAbort("Somehow ended up attempting to set a color table for DrawBlendMode::NoBlend in FrameMan::SetColorTable! This should never happen!");
				return;
			case DrawBlendMode::BlendInvert:
			case DrawBlendMode::BlendDissolve:
				// Invert and Dissolve do nothing with the RGB channels values, so set all channels to Alpha channel value to avoid creating pointless variants.
				colorChannelBlendAmounts.fill(colorChannelBlendAmounts[3]);
				break;
			case DrawBlendMode::BlendTransparency:
				// Indexed transparency has dedicated maps that don't use alpha, so min it to attempt to load one of the presets, and in case there isn't one avoid creating a variant for each alpha value.
				colorChannelBlendAmounts[3] = BlendAmountLimits::MinBlend;
				usedPresetTransparencyTable = (colorChannelBlendAmounts[0] == colorChannelBlendAmounts[1]) && (colorChannelBlendAmounts[0] == colorChannelBlendAmounts[2]);
				break;
			default:
				break;
		}

		// New color tables will be created using the default palette loaded at FrameMan initialization because handling per-palette per-mode color tables is too much headache, even if it may possibly produce better blending results.
		if (m_ColorTables[blendMode].find(colorChannelBlendAmounts) == m_ColorTables[blendMode].end()) {
			m_ColorTables[blendMode].try_emplace(colorChannelBlendAmounts);

			std::array<int, 4> adjustedColorChannelBlendAmounts = { BlendAmountLimits::MinBlend, BlendAmountLimits::MinBlend, BlendAmountLimits::MinBlend, BlendAmountLimits::MinBlend };
			for (int index = 0; index < adjustedColorChannelBlendAmounts.size(); ++index) {
				adjustedColorChannelBlendAmounts[index] = 255 - (static_cast<int>(255.0F * 0.01F * static_cast<float>(colorChannelBlendAmounts[index])));
			}

			if (blendMode == DrawBlendMode::BlendTransparency) {
				// Paletted transparency has dedicated tables so better create one instead of generic for best result. Alpha is ignored here.
				create_trans_table(&m_ColorTables[DrawBlendMode::BlendTransparency].at(colorChannelBlendAmounts).first, m_DefaultPalette, adjustedColorChannelBlendAmounts[0], adjustedColorChannelBlendAmounts[1], adjustedColorChannelBlendAmounts[2], nullptr);
			} else {
				c_BlenderSetterFunctions[blendMode](adjustedColorChannelBlendAmounts[0], adjustedColorChannelBlendAmounts[1], adjustedColorChannelBlendAmounts[2], adjustedColorChannelBlendAmounts[3]);
				create_blender_table(&m_ColorTables[blendMode].at(colorChannelBlendAmounts).first, m_DefaultPalette, nullptr);
				// Reset the blender to avoid potentially screwing some true-color draw operation. Hopefully.
				c_BlenderSetterFunctions[blendMode](255, 255, 255, 255);
			}
		}
		color_map = &m_ColorTables[blendMode].at(colorChannelBlendAmounts).first;
		m_ColorTables[blendMode].at(colorChannelBlendAmounts).second = usedPresetTransparencyTable ? -1 : (g_TimerMan.GetAbsoluteTime() / 10000);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void FrameMan::SetTransTableFromPreset(TransparencyPreset transPreset) {
		RTEAssert(transPreset == TransparencyPreset::LessTrans || transPreset == TransparencyPreset::HalfTrans || transPreset == TransparencyPreset::MoreTrans, "Undefined transparency preset value passed in. See TransparencyPreset enumeration for defined values.");
		std::array<int, 4> colorChannelBlendAmounts = { transPreset, transPreset, transPreset, BlendAmountLimits::MinBlend };
		if (m_ColorTables[DrawBlendMode::BlendTransparency].find(colorChannelBlendAmounts) != m_ColorTables[DrawBlendMode::BlendTransparency].end()) {
			color_map = &m_ColorTables[DrawBlendMode::BlendTransparency].at(colorChannelBlendAmounts).first;
			m_ColorTables[DrawBlendMode::BlendTransparency].at(colorChannelBlendAmounts).second = -1;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void FrameMan::CreateNewNetworkPlayerBackBuffer(int player, int width, int height) {
		for (int f = 0; f < 2; f++) {
			m_NetworkBackBufferIntermediate8[f][player] = std::unique_ptr<BITMAP, BitmapDeleter>(create_bitmap_ex(8, width, height));
			m_NetworkBackBufferIntermediateGUI8[f][player] = std::unique_ptr<BITMAP, BitmapDeleter>(create_bitmap_ex(8, width, height));
			m_NetworkBackBufferFinal8[f][player] = std::unique_ptr<BITMAP, BitmapDeleter>(create_bitmap_ex(8, width, height));
			m_NetworkBackBufferFinalGUI8[f][player] = std::unique_ptr<BITMAP, BitmapDeleter>(create_bitmap_ex(8, width, height));
		}
		m_PlayerScreenWidth = width;
		m_PlayerScreenHeight = height;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool FrameMan::LoadPalette(const std::string &palettePath) {
		const std::string fullPalettePath = g_PresetMan.GetFullModulePath(palettePath);
		BITMAP *tempBitmap = load_bitmap(fullPalettePath.c_str(), m_Palette);
		RTEAssert(tempBitmap, ("Failed to load palette from bitmap with following path:\n\n" + fullPalettePath).c_str());

		set_palette(m_Palette);

		// Update what black is now with the loaded palette
		m_BlackColor = bestfit_color(m_Palette, 0, 0, 0);
		m_AlmostBlackColor = bestfit_color(m_Palette, 5, 5, 5);

		destroy_bitmap(tempBitmap);

		return true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int FrameMan::SaveBitmap(SaveBitmapMode modeToSave, const std::string &nameBase, BITMAP *bitmapToSave) {
		if ((modeToSave == WorldDump || modeToSave == ScenePreviewDump) && !g_ActivityMan.ActivityRunning()) {
			return 0;
		}
		if (nameBase.empty() || nameBase.size() <= 0) {
			return -1;
		}

		// TODO: Remove this once GCC13 is released and switched to. std::format and std::chrono::time_zone are not part of latest libstdc++.
#if defined(__GNUC__) && __GNUC__ < 13
		std::chrono::time_point now = std::chrono::system_clock::now();
		time_t currentTime = std::chrono::system_clock::to_time_t(now);
		tm *localCurrentTime = std::localtime(&currentTime);
		std::array<char, 32> formattedTimeAndDate = {};
		std::strftime(formattedTimeAndDate.data(), sizeof(formattedTimeAndDate), "%F_%H-%M-%S", localCurrentTime);

		std::array<char, 128> fullFileNameBuffer = {};
		// We can't get sub-second precision from timeBuffer so we'll append absolute time to not overwrite the same file when dumping multiple times per second.
		std::snprintf(fullFileNameBuffer.data(), sizeof(fullFileNameBuffer), "%s/%s_%s.%zi.png", System::GetScreenshotDirectory().c_str(), nameBase.c_str(), formattedTimeAndDate.data(), g_TimerMan.GetAbsoluteTime());

		std::string fullFileName(fullFileNameBuffer.data());
#else
		std::string fullFileName = std::format("{}{}_{:%F_%H-%M-%S}.png", System::GetScreenshotDirectory(), nameBase, std::chrono::current_zone()->to_local(std::chrono::system_clock::now()));
#endif

		bool saveSuccess = false;

		switch (modeToSave) {
			case SingleBitmap:
				if (bitmapToSave && save_png(nameBase.c_str(), bitmapToSave, m_Palette) == 0) {
					g_ConsoleMan.PrintString("SYSTEM: Bitmap was dumped to: " + nameBase);
					saveSuccess = true;
				}
				break;
			case ScreenDump:
				if (m_BackBuffer32 && m_ScreenDumpBuffer) {
					SaveScreenToBitmap();

					// Make a copy of the buffer because it may be overwritten mid thread and everything will be on fire.
					BITMAP *outputBitmap = create_bitmap_ex(bitmap_color_depth(m_ScreenDumpBuffer.get()), m_ScreenDumpBuffer->w, m_ScreenDumpBuffer->h);
					stretch_blit(m_ScreenDumpBuffer.get(), outputBitmap, 0, 0, m_ScreenDumpBuffer->w, m_ScreenDumpBuffer->h, 0, 0, outputBitmap->w, outputBitmap->h);

					auto saveScreenDump = [fullFileName](BITMAP *bitmapToSaveCopy) {
						// nullptr for the PALETTE parameter here because we're saving a 24bpp file and it's irrelevant.
						if (save_png(fullFileName.c_str(), bitmapToSaveCopy, nullptr) == 0) {
							g_ConsoleMan.PrintString("SYSTEM: Screen was dumped to: " + fullFileName);
						} else {
							g_ConsoleMan.PrintString("ERROR: Unable to save bitmap to: " + fullFileName);
						}
						destroy_bitmap(bitmapToSaveCopy);
					};
					std::thread saveThread(saveScreenDump, outputBitmap);
					saveThread.detach();

					saveSuccess = true;
				}
				break;
			case ScenePreviewDump:
			case WorldDump:
				if (!m_WorldDumpBuffer || (m_WorldDumpBuffer->w != g_SceneMan.GetSceneWidth() || m_WorldDumpBuffer->h != g_SceneMan.GetSceneHeight())) {
					m_WorldDumpBuffer = std::unique_ptr<BITMAP, BitmapDeleter>(create_bitmap_ex(c_BPP, g_SceneMan.GetSceneWidth(), g_SceneMan.GetSceneHeight()));
				}
				if (modeToSave == ScenePreviewDump) {
					DrawWorldDump(true);

					BITMAP *scenePreviewDumpBuffer = create_bitmap_ex(c_BPP, c_ScenePreviewWidth, c_ScenePreviewHeight);
					blit(m_ScenePreviewDumpGradient.get(), scenePreviewDumpBuffer, 0, 0, 0, 0, scenePreviewDumpBuffer->w, scenePreviewDumpBuffer->h);
					masked_stretch_blit(m_WorldDumpBuffer.get(), scenePreviewDumpBuffer, 0, 0, m_WorldDumpBuffer->w, m_WorldDumpBuffer->h, 0, 0, scenePreviewDumpBuffer->w, scenePreviewDumpBuffer->h);

					if (SaveIndexedPNG(fullFileName.c_str(), scenePreviewDumpBuffer) == 0) {
						g_ConsoleMan.PrintString("SYSTEM: Scene Preview was dumped to: " + fullFileName);
						saveSuccess = true;
					}
					destroy_bitmap(scenePreviewDumpBuffer);
				} else {
					DrawWorldDump();

					BITMAP *depthConvertBitmap = create_bitmap_ex(24, m_WorldDumpBuffer->w, m_WorldDumpBuffer->h);
					blit(m_WorldDumpBuffer.get(), depthConvertBitmap, 0, 0, 0, 0, m_WorldDumpBuffer->w, m_WorldDumpBuffer->h);

					if (save_png(fullFileName.c_str(), depthConvertBitmap, nullptr) == 0) {
						g_ConsoleMan.PrintString("SYSTEM: World was dumped to: " + fullFileName);
						saveSuccess = true;
					}
					destroy_bitmap(depthConvertBitmap);
				}
				break;
			default:
				g_ConsoleMan.PrintString("ERROR: Wrong bitmap save mode passed in, no bitmap was saved!");
				return -1;
		}
		if (!saveSuccess) {
			g_ConsoleMan.PrintString("ERROR: Unable to save bitmap to: " + fullFileName);
			return -1;
		} else {
			return 0;
		}
	}

	void FrameMan::SaveScreenToBitmap() {
		if (!m_ScreenDumpBuffer) {
			return;
		}

		glBindTexture(GL_TEXTURE_2D, g_WindowMan.GetScreenBufferTexture());
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, m_ScreenDumpBuffer->line[0]);

	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int FrameMan::SaveIndexedPNG(const char *fileName, BITMAP *bitmapToSave) const {
		// nullptr for the PALETTE parameter here because the bitmap is 32bpp and whatever we index it with will end up wrong anyway.
		save_png(fileName, bitmapToSave, nullptr);

		int lastColorConversionMode = get_color_conversion();
		set_color_conversion(COLORCONV_REDUCE_TO_256);
		// nullptr for the PALETTE parameter here because we don't need the bad palette from it and don't want it to overwrite anything.
		BITMAP *tempLoadBitmap = load_bitmap(fileName, nullptr);
		std::remove(fileName);

		BITMAP *tempConvertingBitmap = create_bitmap_ex(8, bitmapToSave->w, bitmapToSave->h);
		blit(tempLoadBitmap, tempConvertingBitmap, 0, 0, 0, 0, tempConvertingBitmap->w, tempConvertingBitmap->h);

		int saveResult = save_png(fileName, tempConvertingBitmap, m_Palette);

		set_color_conversion(lastColorConversionMode);
		destroy_bitmap(tempLoadBitmap);
		destroy_bitmap(tempConvertingBitmap);

		return saveResult;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int FrameMan::SharedDrawLine(BITMAP *bitmap, const Vector &start, const Vector &end, int color, int altColor, int skip, int skipStart, bool shortestWrap, bool drawDot, BITMAP *dot) const {
		RTEAssert(bitmap, "Trying to draw line to null Bitmap");
		if (drawDot) { RTEAssert(dot, "Trying to draw line of dots without specifying a dot Bitmap"); }

		int error = 0;
		int dom = 0;
		int sub = 0;
		int domSteps = 0;
		int skipped = skip + (skipStart - skip);
		int intPos[2];
		int delta[2];
		int delta2[2];
		int increment[2];
		bool drawAlt = false;

		int dotHeight = drawDot ? dot->h : 0;
		int dotWidth = drawDot ? dot->w : 0;

		// Just make the alt the same color as the main one if no one was specified
		if (altColor == 0) { altColor = color; }

		intPos[X] = start.GetFloorIntX();
		intPos[Y] = start.GetFloorIntY();

		// Wrap line around the scene if it makes it shorter
		if (shortestWrap) {
			Vector deltaVec = g_SceneMan.ShortestDistance(start, end, false);
			delta[X] = deltaVec.GetFloorIntX();
			delta[Y] = deltaVec.GetFloorIntY();
		} else {
			delta[X] = end.GetFloorIntX() - intPos[X];
			delta[Y] = end.GetFloorIntY() - intPos[Y];
		}
		if (delta[X] == 0 && delta[Y] == 0) {
			return 0;
		}

		// Bresenham's line drawing algorithm preparation
		if (delta[X] < 0) {
			increment[X] = -1;
			delta[X] = -delta[X];
		} else {
			increment[X] = 1;
		}
		if (delta[Y] < 0) {
			increment[Y] = -1;
			delta[Y] = -delta[Y];
		} else {
			increment[Y] = 1;
		}

		// Scale by 2, for better accuracy of the error at the first pixel
		delta2[X] = delta[X] << 1;
		delta2[Y] = delta[Y] << 1;

		// If X is dominant, Y is submissive, and vice versa.
		if (delta[X] > delta[Y]) {
			dom = X;
			sub = Y;
		} else {
			dom = Y;
			sub = X;
		}
		error = delta2[sub] - delta[dom];

		// Bresenham's line drawing algorithm execution
		for (domSteps = 0; domSteps < delta[dom]; ++domSteps) {
			intPos[dom] += increment[dom];
			if (error >= 0) {
				intPos[sub] += increment[sub];
				error -= delta2[dom];
			}
			error += delta2[sub];

			// Only draw pixel if we're not due to skip any
			if (++skipped > skip) {
				// Scene wrapping, if necessary
				g_SceneMan.WrapPosition(intPos[X], intPos[Y]);

				if (drawDot) {
					masked_blit(dot, bitmap, 0, 0, intPos[X] - (dotWidth / 2), intPos[Y] - (dotHeight / 2), dot->w, dot->h);
				} else {
					putpixel(bitmap, intPos[X], intPos[Y], drawAlt ? color : altColor);
				}
				drawAlt = !drawAlt;
				skipped = 0;
			}
		}

		// Return the end phase state of the skipping
		return skipped;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	GUIFont * FrameMan::GetFont(bool isSmall, bool trueColor) {
		size_t colorIndex = trueColor ? 1 : 0;

		if (!m_GUIScreens[colorIndex]) {
			m_GUIScreens[colorIndex] = new AllegroScreen(trueColor ? m_BackBuffer32.get() : m_BackBuffer8.get());
		}

		if (isSmall) {
			if (!m_SmallFonts[colorIndex]) {
				std::string fontName = "SmallFont";
				std::string fontPath = "Base.rte/GUIs/Skins/FontSmall.png";

				if (trueColor) {
					fontName = "SmallFont32";
					fontPath = "Base.rte/GUIs/Skins/Menus/FontSmall.png";
				}
				m_SmallFonts[colorIndex] = new GUIFont(fontName);
				m_SmallFonts[colorIndex]->Load(m_GUIScreens[colorIndex], fontPath);
			}
			return m_SmallFonts[colorIndex];
		}
		if (!m_LargeFonts[colorIndex]) {
			std::string fontName = "FatFont";
			std::string fontPath = "Base.rte/GUIs/Skins/FontLarge.png";

			if (trueColor) {
				fontName = "FatFont32";
				fontPath = "Base.rte/GUIs/Skins/Menus/FontLarge.png";
			}
			m_LargeFonts[colorIndex] = new GUIFont(fontName);
			m_LargeFonts[colorIndex]->Load(m_GUIScreens[colorIndex], fontPath);
		}
		return m_LargeFonts[colorIndex];
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void FrameMan::UpdateScreenOffsetForSplitScreen(int playerScreen, Vector &screenOffset) const {
		switch (playerScreen) {
			case Players::PlayerTwo:
				// If both splits, or just VSplit, then in upper right quadrant
				if ((m_VSplit && !m_HSplit) || (m_VSplit && m_HSplit)) {
					screenOffset.SetXY(g_WindowMan.GetResX() / 2, 0);
				} else {
					// If only HSplit, then lower left quadrant
					screenOffset.SetXY(0, g_WindowMan.GetResY() / 2);
				}
				break;
			case Players::PlayerThree:
				// Always lower left quadrant
				screenOffset.SetXY(0, g_WindowMan.GetResY() / 2);
				break;
			case Players::PlayerFour:
				// Always lower right quadrant
				screenOffset.SetXY(g_WindowMan.GetResX() / 2, g_WindowMan.GetResY() / 2);
				break;
			default:
				// Always upper left corner
				screenOffset.SetXY(0, 0);
				break;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void FrameMan::Draw() {
		ZoneScopedN("Draw");

		// Count how many split screens we'll need
		int screenCount = (m_HSplit ? 2 : 1) * (m_VSplit ? 2 : 1);
		RTEAssert(screenCount <= 1 || m_PlayerScreen, "Splitscreen surface not ready when needed!");

		g_PostProcessMan.ClearScreenPostEffects();

		// These accumulate the effects for each player's screen area, and are then transferred to the post-processing lists with the player screen offset applied
		std::list<PostEffect> screenRelativeEffects;
		std::list<Box> screenRelativeGlowBoxes;

		const Activity *pActivity = g_ThreadMan.GetDrawableGameState().m_Activity;

		for (int playerScreen = 0; playerScreen < screenCount; ++playerScreen) {
			screenRelativeEffects.clear();
			screenRelativeGlowBoxes.clear();

			BITMAP *drawScreen = (screenCount == 1) ? m_BackBuffer8.get() : m_PlayerScreen.get();
			BITMAP *drawScreenGUI = drawScreen;
			if (IsInMultiplayerMode()) {
				drawScreen = m_NetworkBackBufferIntermediate8[m_NetworkFrameCurrent][playerScreen].get();
				drawScreenGUI = m_NetworkBackBufferIntermediateGUI8[m_NetworkFrameCurrent][playerScreen].get();
			}
			// Need to clear the backbuffers because Scene background layers can be too small to fill the whole backbuffer or drawn masked resulting in artifacts from the previous frame.
			clear_to_color(drawScreenGUI, ColorKeys::g_MaskColor);
			// If in online multiplayer mode clear to mask color otherwise the scene background layers will get drawn over.
			clear_to_color(drawScreen, IsInMultiplayerMode() ? ColorKeys::g_MaskColor : m_BlackColor);

			AllegroBitmap playerGUIBitmap(drawScreenGUI);

			// Update the scene view to line up with a specific screen and then draw it onto the intermediate screen
			g_SceneMan.Update(playerScreen);

			// Save scene layer's offsets for each screen, server will pick them to build the frame state and send to client
			if (IsInMultiplayerMode()) {
				int layerCount = 0;

				for (const SceneLayer *sceneLayer : g_SceneMan.GetScene()->GetBackLayers()) {
					SLOffset[playerScreen][layerCount] = sceneLayer->GetOffset();
					layerCount++;

					if (layerCount >= c_MaxLayersStoredForNetwork) {
						break;
					}
				}
			}

			Vector targetPos = g_CameraMan.GetOffset(playerScreen);

			// Adjust the drawing position on the target screen for if the target screen is larger than the scene in non-wrapping dimension.
			// Scene needs to be displayed centered on the target bitmap then, and that has to be adjusted for when drawing to the screen
			if (!g_SceneMan.SceneWrapsX() && drawScreen->w > g_SceneMan.GetSceneWidth()) { targetPos.m_X += (drawScreen->w - g_SceneMan.GetSceneWidth()) / 2; }
			if (!g_SceneMan.SceneWrapsY() && drawScreen->h > g_SceneMan.GetSceneHeight()) { targetPos.m_Y += (drawScreen->h - g_SceneMan.GetSceneHeight()) / 2; }

			// Try to move at the frame buffer copy time to maybe prevent wonkyness
			m_TargetPos[m_NetworkFrameCurrent][playerScreen] = targetPos;

			// Draw the scene
			if (!IsInMultiplayerMode()) {
				g_SceneMan.Draw(drawScreen, drawScreenGUI, targetPos);
			} else {
				g_SceneMan.Draw(drawScreen, drawScreenGUI, targetPos, true, true);
			}

			// Get only the scene-relative post effects that affect this player's screen
			if (pActivity) {
				// TODO_MULTITHREAD
#ifndef MULTITHREAD_SIM_AND_RENDER
				g_PostProcessMan.GetPostScreenEffectsWrapped(targetPos, drawScreen->w, drawScreen->h, screenRelativeEffects, pActivity->GetTeamOfPlayer(pActivity->PlayerOfScreen(playerScreen)));
				g_PostProcessMan.GetGlowAreasWrapped(targetPos, drawScreen->w, drawScreen->h, screenRelativeGlowBoxes);
#endif

				if (IsInMultiplayerMode()) { g_PostProcessMan.SetNetworkPostEffectsList(playerScreen, screenRelativeEffects); }
			}

			// TODO: Find out what keeps disabling the clipping on the draw bitmap
			// Enable clipping on the draw bitmap
			set_clip_state(drawScreen, 1);

			DrawScreenText(playerScreen, playerGUIBitmap);

			// The position of the current draw screen on the backbuffer
			Vector screenOffset;

			// If we are dealing with split screens, then deal with the fact that we need to draw the player screens to different locations on the final buffer
			if (screenCount > 1) { UpdateScreenOffsetForSplitScreen(playerScreen, screenOffset); }

			DrawScreenFlash(playerScreen, drawScreenGUI);

			if (!IsInMultiplayerMode()) {
				// Draw the intermediate draw splitscreen to the appropriate spot on the back buffer
				blit(drawScreen, m_BackBuffer8.get(), 0, 0, screenOffset.GetFloorIntX(), screenOffset.GetFloorIntY(), drawScreen->w, drawScreen->h);

				g_PostProcessMan.AdjustEffectsPosToPlayerScreen(playerScreen, drawScreen, screenOffset, screenRelativeEffects, screenRelativeGlowBoxes);
			}
		}

		// Clears the pixels that have been revealed from the unseen layers
		g_SceneMan.ClearSeenPixels();

		if (!IsInMultiplayerMode()) {
			// Draw separating lines for split-screens
			if (m_HSplit) {
				hline(m_BackBuffer8.get(), 0, (m_BackBuffer8->h / 2) - 1, m_BackBuffer8->w - 1, m_AlmostBlackColor);
				hline(m_BackBuffer8.get(), 0, (m_BackBuffer8->h / 2), m_BackBuffer8->w - 1, m_AlmostBlackColor);
			}
			if (m_VSplit) {
				vline(m_BackBuffer8.get(), (m_BackBuffer8->w / 2) - 1, 0, m_BackBuffer8->h - 1, m_AlmostBlackColor);
				vline(m_BackBuffer8.get(), (m_BackBuffer8->w / 2), 0, m_BackBuffer8->h - 1, m_AlmostBlackColor);
			}

			// Replace 8 bit backbuffer contents with network received image before post-processing as it is where this buffer is copied to 32 bit buffer
			if (GetDrawNetworkBackBuffer()) {
				m_NetworkBitmapLock[0].lock();

				blit(m_NetworkBackBufferFinal8[m_NetworkFrameReady][0].get(), m_BackBuffer8.get(), 0, 0, 0, 0, m_BackBuffer8->w, m_BackBuffer8->h);
				masked_blit(m_NetworkBackBufferFinalGUI8[m_NetworkFrameReady][0].get(), m_BackBuffer8.get(), 0, 0, 0, 0, m_BackBuffer8->w, m_BackBuffer8->h);

				if (g_UInputMan.FlagAltState() || g_UInputMan.FlagCtrlState() || g_UInputMan.FlagShiftState()) { g_PerformanceMan.DrawCurrentPing(); }

				m_NetworkBitmapLock[0].unlock();
			}
		}

		if (IsInMultiplayerMode()) { 
			PrepareFrameForNetwork(); 
		}

		if (g_ActivityMan.IsInActivity()) {
			// TODO_MULTITHREAD: add post processing effects to RenderableGameState
			g_PostProcessMan.PostProcess(); 
		}

		// Draw the performance stats and console on top of everything.
		g_PerformanceMan.Draw(m_BackBuffer32.get());
		g_ConsoleMan.Draw(m_BackBuffer32.get());
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void FrameMan::DrawScreenText(int playerScreen, AllegroBitmap playerGUIBitmap) {
		int textPosY = 0;
		// Only draw screen text to actual human players
		if (playerScreen < g_ActivityMan.GetActivity()->GetHumanCount()) {
			textPosY += 12;

			if (!m_ScreenText[playerScreen].empty()) {
				int bufferOrScreenWidth = IsInMultiplayerMode() ? GetPlayerFrameBufferWidth(playerScreen) : GetPlayerScreenWidth();
				int bufferOrScreenHeight = IsInMultiplayerMode() ? GetPlayerFrameBufferHeight(playerScreen) : GetPlayerScreenHeight();

				if (m_TextCentered[playerScreen]) { textPosY = (bufferOrScreenHeight / 2) - 52; }

				int screenOcclusionOffsetX = g_CameraMan.GetScreenOcclusion(playerScreen).GetRoundIntX();
				// If there's really no room to offset the text into, then don't
				if (GetPlayerScreenWidth() <= g_WindowMan.GetResX() / 2) { screenOcclusionOffsetX = 0; }

				std::string screenTextToDraw = m_ScreenText[playerScreen];
				if (m_TextBlinking[playerScreen] && m_TextBlinkTimer.AlternateReal(m_TextBlinking[playerScreen])) {
					screenTextToDraw = ">>> " + screenTextToDraw + " <<<";
				}
				screenTextToDraw = SplitStringToFitWidth(screenTextToDraw, bufferOrScreenWidth, false);
				GetLargeFont()->DrawAligned(&playerGUIBitmap, (bufferOrScreenWidth + screenOcclusionOffsetX) / 2, textPosY, screenTextToDraw, GUIFont::Centre);
				textPosY += 12;
			}

			// Draw info text when in material layer draw mode
			switch (g_SceneMan.GetLayerDrawMode()) {
				case g_LayerTerrainMatter:
					GetSmallFont()->DrawAligned(&playerGUIBitmap, GetPlayerScreenWidth() / 2, GetPlayerScreenHeight() - 12, "Viewing terrain material layer\nHit Ctrl+M to toggle", GUIFont::Centre, GUIFont::Bottom);
					break;
				default:
					break;
			}
		} else {
			// If superfluous screen (as in a three-player match), make the fourth the Observer one
			GetLargeFont()->DrawAligned(&playerGUIBitmap, GetPlayerScreenWidth() / 2, textPosY, "- Observer View -", GUIFont::Centre);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void FrameMan::DrawScreenFlash(int playerScreen, BITMAP *playerGUIBitmap) {
		if (m_FlashScreenColor[playerScreen] != -1) {
			// If set to flash for a period of time, first be solid and then start flashing slower
			double timeTillLimit = m_FlashTimer[playerScreen].LeftTillRealTimeLimitMS();

			if (timeTillLimit < 10 || m_FlashTimer[playerScreen].AlternateReal(50)) {
				if (m_FlashedLastFrame[playerScreen]) {
					m_FlashedLastFrame[playerScreen] = false;
				} else {
					rectfill(playerGUIBitmap, 0, 0, playerGUIBitmap->w, playerGUIBitmap->h, m_FlashScreenColor[playerScreen]);
					m_FlashedLastFrame[playerScreen] = true;
				}
			}
			if (m_FlashTimer[playerScreen].IsPastRealTimeLimit()) { m_FlashScreenColor[playerScreen] = -1; }
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void FrameMan::DrawWorldDump(bool drawForScenePreview) const {
		float worldBitmapWidth = static_cast<float>(m_WorldDumpBuffer->w);
		float worldBitmapHeight = static_cast<float>(m_WorldDumpBuffer->h);

		// Draw sky gradient if we're not dumping a scene preview
		if (!drawForScenePreview) {
			clear_to_color(m_WorldDumpBuffer.get(), makecol32(132, 192, 252)); // Light blue color
			for (int i = 0; i < m_WorldDumpBuffer->h; i++) {
				int lineColor = makecol32(64 + ((static_cast<float>(i) / worldBitmapHeight) * (128 - 64)), 64 + ((static_cast<float>(i) / worldBitmapHeight) * (192 - 64)), 96 + ((static_cast<float>(i) / worldBitmapHeight) * (255 - 96)));
				hline(m_WorldDumpBuffer.get(), 0, i, worldBitmapWidth - 1, lineColor);
			}
		} else {
			clear_to_color(m_WorldDumpBuffer.get(), makecol32(255, 0, 255)); // Magenta
		}

		// Draw scene
		draw_sprite(m_WorldDumpBuffer.get(), g_SceneMan.GetTerrain()->GetBGColorBitmap(), 0, 0);
		draw_sprite(m_WorldDumpBuffer.get(), g_SceneMan.GetTerrain()->GetFGColorBitmap(), 0, 0);

		// If we're not dumping a scene preview, draw objects and post-effects.
		if (!drawForScenePreview) {
			std::list<PostEffect> postEffectsList;
			BITMAP *effectBitmap = nullptr;
			int effectPosX = 0;
			int effectPosY = 0;
			int effectStrength = 0;
			Vector targetPos(0, 0);

			// Draw objects
			g_MovableMan.Draw(m_WorldDumpBuffer.get());

			// Draw post-effects
			g_PostProcessMan.GetPostScreenEffectsWrapped(targetPos, worldBitmapWidth, worldBitmapHeight, postEffectsList, -1);

			for (const PostEffect &postEffect : postEffectsList) {
				effectBitmap = postEffect.m_Bitmap;
				effectStrength = postEffect.m_Strength;
				set_screen_blender(effectStrength, effectStrength, effectStrength, effectStrength);
				effectPosX = postEffect.m_Pos.GetFloorIntX() - (effectBitmap->w / 2);
				effectPosY = postEffect.m_Pos.GetFloorIntY() - (effectBitmap->h / 2);

				if (postEffect.m_Angle == 0.0F) {
					draw_trans_sprite(m_WorldDumpBuffer.get(), effectBitmap, effectPosX, effectPosY);
				} else {
					BITMAP *targetBitmap = g_PostProcessMan.GetTempEffectBitmap(effectBitmap);
					clear_to_color(targetBitmap, 0);

					Matrix newAngle(postEffect.m_Angle);
					rotate_sprite(targetBitmap, effectBitmap, 0, 0, ftofix(newAngle.GetAllegroAngle()));
					draw_trans_sprite(m_WorldDumpBuffer.get(), targetBitmap, effectPosX, effectPosY);
				}
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void FrameMan::PrepareFrameForNetwork() {
		int dx = 0;
		int dy = 0;
		int dw = m_BackBuffer8->w / 2;
		int dh = m_BackBuffer8->h / 2;

		// Blit all four internal player screens onto the backbuffer
		for (int i = 0; i < c_MaxScreenCount; i++) {
			dx = (i == 1 || i == 3) ? dw : dx;
			dy = (i == 2 || i == 3) ? dh : dy;

			m_NetworkBitmapLock[i].lock();
			blit(m_NetworkBackBufferIntermediate8[m_NetworkFrameCurrent][i].get(), m_NetworkBackBufferFinal8[m_NetworkFrameCurrent][i].get(), 0, 0, 0, 0, m_NetworkBackBufferFinal8[m_NetworkFrameCurrent][i]->w, m_NetworkBackBufferFinal8[m_NetworkFrameCurrent][i]->h);
			blit(m_NetworkBackBufferIntermediateGUI8[m_NetworkFrameCurrent][i].get(), m_NetworkBackBufferFinalGUI8[m_NetworkFrameCurrent][i].get(), 0, 0, 0, 0, m_NetworkBackBufferFinalGUI8[m_NetworkFrameCurrent][i]->w, m_NetworkBackBufferFinalGUI8[m_NetworkFrameCurrent][i]->h);
			m_NetworkBitmapLock[i].unlock();

#ifndef RELEASE_BUILD
			// Draw all player's screen into one
			if (g_UInputMan.KeyHeld(SDLK_5)) {
				stretch_blit(m_NetworkBackBufferFinal8[m_NetworkFrameCurrent][i].get(), m_BackBuffer8.get(), 0, 0, m_NetworkBackBufferFinal8[m_NetworkFrameReady][i]->w, m_NetworkBackBufferFinal8[m_NetworkFrameReady][i]->h, dx, dy, dw, dh);
			}
#endif
		}

#ifndef RELEASE_BUILD
		if (g_UInputMan.KeyHeld(SDLK_1)) {
			stretch_blit(m_NetworkBackBufferFinal8[0][0].get(), m_BackBuffer8.get(), 0, 0, m_NetworkBackBufferFinal8[m_NetworkFrameReady][0]->w, m_NetworkBackBufferFinal8[m_NetworkFrameReady][0]->h, 0, 0, m_BackBuffer8->w, m_BackBuffer8->h);
		}
		if (g_UInputMan.KeyHeld(SDLK_2)) {
			stretch_blit(m_NetworkBackBufferFinal8[1][0].get(), m_BackBuffer8.get(), 0, 0, m_NetworkBackBufferFinal8[m_NetworkFrameReady][1]->w, m_NetworkBackBufferFinal8[m_NetworkFrameReady][1]->h, 0, 0, m_BackBuffer8->w, m_BackBuffer8->h);
		}
		if (g_UInputMan.KeyHeld(SDLK_3)) {
			stretch_blit(m_NetworkBackBufferFinal8[m_NetworkFrameReady][2].get(), m_BackBuffer8.get(), 0, 0, m_NetworkBackBufferFinal8[m_NetworkFrameReady][2]->w, m_NetworkBackBufferFinal8[m_NetworkFrameReady][2]->h, 0, 0, m_BackBuffer8->w, m_BackBuffer8->h);
		}
		if (g_UInputMan.KeyHeld(SDLK_4)) {
			stretch_blit(m_NetworkBackBufferFinal8[m_NetworkFrameReady][3].get(), m_BackBuffer8.get(), 0, 0, m_NetworkBackBufferFinal8[m_NetworkFrameReady][3]->w, m_NetworkBackBufferFinal8[m_NetworkFrameReady][3]->h, 0, 0, m_BackBuffer8->w, m_BackBuffer8->h);
		}
#endif
		// Rendering complete, we can finally mark current frame as ready. This is needed to make rendering look totally atomic for the server pulling data in separate threads.
		m_NetworkFrameReady = m_NetworkFrameCurrent;
		m_NetworkFrameCurrent = (m_NetworkFrameCurrent == 0) ? 1 : 0;
	}
}