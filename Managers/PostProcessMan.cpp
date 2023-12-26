#include "PostProcessMan.h"

#include "CameraMan.h"
#include "FrameMan.h"
#include "Scene.h"
#include "ContentFile.h"
#include "Matrix.h"

#include "PresetMan.h"

#include "GLCheck.h"
#include "glad/gl.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PostProcessMan::Clear() {
		m_PostScreenEffects.clear();
		m_PostSceneEffects.clear();
		m_YellowGlow = nullptr;
		m_YellowGlowHash = 0;
		m_RedGlow = nullptr;
		m_RedGlowHash = 0;
		m_BlueGlow = nullptr;
		m_BlueGlowHash = 0;
		m_TempEffectBitmaps.clear();
		m_BackBuffer8 = 0;
		m_BackBuffer32 = 0;
		m_Palette8Texture = 0;
		m_PostProcessFramebuffer = 0;
		m_PostProcessDepthBuffer = 0;
		m_BitmapTextures.clear();
		m_VertexBuffer = 0;
		m_VertexArray = 0;
		for (int i = 0; i < c_MaxScreenCount; ++i) {
			m_ScreenRelativeEffects[i].clear();
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int PostProcessMan::Initialize() {
		InitializeGLPointers();
		CreateGLBackBuffers();

		m_Blit8 = std::make_unique<Shader>(g_PresetMan.GetFullModulePath("Base.rte/Shaders/Blit8.vert"), g_PresetMan.GetFullModulePath("Base.rte/Shaders/Blit8.frag"));
		m_PostProcessShader = std::make_unique<Shader>(g_PresetMan.GetFullModulePath("Base.rte/Shaders/PostProcess.vert"), g_PresetMan.GetFullModulePath("Base.rte/Shaders/PostProcess.frag"));
		// TODO: Make more robust and load more glows!
		ContentFile glowFile("Base.rte/Effects/Glows/YellowTiny.png");
		m_YellowGlow = glowFile.GetAsBitmap();
		m_YellowGlowHash = glowFile.GetHash();
		glowFile.SetDataPath("Base.rte/Effects/Glows/RedTiny.png");
		m_RedGlow = glowFile.GetAsBitmap();
		m_RedGlowHash = glowFile.GetHash();
		glowFile.SetDataPath("Base.rte/Effects/Glows/BlueTiny.png");
		m_BlueGlow = glowFile.GetAsBitmap();
		m_BlueGlowHash = glowFile.GetHash();

		// Create temporary bitmaps to rotate post effects in.
		m_TempEffectBitmaps = {
		    {16, create_bitmap(16, 16)},
		    {32, create_bitmap(32, 32)},
		    {64, create_bitmap(64, 64)},
		    {128, create_bitmap(128, 128)},
		    {256, create_bitmap(256, 256)},
		    {512, create_bitmap(512, 512)}};

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PostProcessMan::InitializeGLPointers() {
		glGenTextures(1, &m_BackBuffer8);
		glGenTextures(1, &m_BackBuffer32);
		glGenTextures(1, &m_Palette8Texture);
		glGenFramebuffers(1, &m_BlitFramebuffer);
		glGenFramebuffers(1, &m_PostProcessFramebuffer);
		glGenTextures(1, &m_PostProcessDepthBuffer);
		glGenVertexArrays(1, &m_VertexArray);
		glGenBuffers(1, &m_VertexBuffer);

		glBindVertexArray(m_VertexArray);
		glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(c_Quad), c_Quad.data(), GL_STATIC_DRAW);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
		glEnableVertexAttribArray(1);
		glBindVertexArray(0);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PostProcessMan::DestroyGLPointers() {
		glDeleteTextures(1, &m_BackBuffer8);
		glDeleteTextures(1, &m_BackBuffer32);
		glDeleteTextures(1, &m_Palette8Texture);
		glDeleteFramebuffers(1, &m_BlitFramebuffer);
		for (auto &bitmapTexture : m_BitmapTextures) {
			glDeleteTextures(1, &bitmapTexture->m_Texture);
		}
		glDeleteFramebuffers(1, &m_PostProcessFramebuffer);
		glDeleteTextures(1, &m_PostProcessDepthBuffer);
		glDeleteVertexArrays(1, &m_VertexArray);
		glDeleteBuffers(1, &m_VertexBuffer);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PostProcessMan::CreateGLBackBuffers() {
		glBindTexture(GL_TEXTURE_2D, m_BackBuffer8);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, g_FrameMan.GetBackBuffer8()->w, g_FrameMan.GetBackBuffer8()->h, 0, GL_RED, GL_UNSIGNED_BYTE, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, m_BackBuffer32);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, g_FrameMan.GetBackBuffer8()->w, g_FrameMan.GetBackBuffer8()->h, 0, GL_RED, GL_UNSIGNED_BYTE, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindTexture(GL_TEXTURE_1D, m_Palette8Texture);
		glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, c_PaletteEntriesNumber, 0, GL_RGBA, GL_UNSIGNED_INT, 0);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		UpdatePalette();
		glActiveTexture(GL_TEXTURE0);
		m_ProjectionMatrix = glm::ortho(0.0F, static_cast<float>(g_FrameMan.GetBackBuffer8()->w), 0.0F, static_cast<float>(g_FrameMan.GetBackBuffer8()->h), -1.0F, 1.0F);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PostProcessMan::UpdatePalette() {
		glBindTexture(GL_TEXTURE_1D, m_Palette8Texture);
		std::array<unsigned int, c_PaletteEntriesNumber> palette;
		for (int i = 0; i < c_PaletteEntriesNumber; ++i) {
			if (i == g_MaskColor) {
				palette[i] = 0;
				continue;
			}
			palette[i] = makeacol32(getr8(i), getg8(i), getb8(i), 255);
		}
		glTexSubImage1D(GL_TEXTURE_1D, 0, 0, c_PaletteEntriesNumber, GL_RGBA, GL_UNSIGNED_BYTE, palette.data());
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PostProcessMan::LazyInitBitmap(BITMAP *bitmap) {
		m_BitmapTextures.emplace_back(new GLBitmapInfo);
		glGenTextures(1, &m_BitmapTextures.back()->m_Texture);
		bitmap->extra = reinterpret_cast<void *>(m_BitmapTextures.back().get());
		glPixelStorei(GL_UNPACK_ALIGNMENT, bitmap_color_depth(bitmap) == 8 ? 1 : 4);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, reinterpret_cast<GLBitmapInfo*>(bitmap->extra)->m_Texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bitmap->w, bitmap->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, bitmap->line[0]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glGenerateMipmap(GL_TEXTURE_2D);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PostProcessMan::Destroy() {
		for (std::pair<int, BITMAP *> tempBitmapEntry : m_TempEffectBitmaps) {
			destroy_bitmap(tempBitmapEntry.second);
		}
		DestroyGLPointers();
		ClearScreenPostEffects();
		ClearScenePostEffects();
		Clear();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PostProcessMan::AdjustEffectsPosToPlayerScreen(int playerScreen, BITMAP *targetBitmap, const Vector &targetBitmapOffset, std::list<PostEffect> &screenRelativeEffectsList, std::list<Box> &screenRelativeGlowBoxesList) {
		int screenOcclusionOffsetX = g_CameraMan.GetScreenOcclusion(playerScreen).GetFloorIntX();
		int screenOcclusionOffsetY = g_CameraMan.GetScreenOcclusion(playerScreen).GetFloorIntY();
		int occludedOffsetX = targetBitmap->w + screenOcclusionOffsetX;
		int occludedOffsetY = targetBitmap->h + screenOcclusionOffsetY;

		// Copy post effects received by client if in network mode
		if (g_FrameMan.GetDrawNetworkBackBuffer()) {
			GetNetworkPostEffectsList(0, screenRelativeEffectsList);
		}

		// Adjust for the player screen's position on the final buffer
		for (const PostEffect &postEffect : screenRelativeEffectsList) {
			// Make sure we won't be adding any effects to a part of the screen that is occluded by menus and such
			if (postEffect.m_Pos.GetFloorIntX() > screenOcclusionOffsetX && postEffect.m_Pos.GetFloorIntY() > screenOcclusionOffsetY && postEffect.m_Pos.GetFloorIntX() < occludedOffsetX && postEffect.m_Pos.GetFloorIntY() < occludedOffsetY) {
				m_PostScreenEffects.emplace_back(postEffect.m_Pos + targetBitmapOffset, postEffect.m_Bitmap, postEffect.m_BitmapHash, postEffect.m_Strength, postEffect.m_Angle);
			}
		}
		// Adjust glow areas for the player screen's position on the final buffer
		for (const Box &glowBox : screenRelativeGlowBoxesList) {
			m_PostScreenGlowBoxes.push_back(glowBox);
			// Adjust each added glow area for the player screen's position on the final buffer
			m_PostScreenGlowBoxes.back().m_Corner += targetBitmapOffset;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PostProcessMan::RegisterPostEffect(const Vector &effectPos, BITMAP *effect, size_t hash, int strength, float angle) {
		// TODO_MULTITHREAD
#ifndef MULTITHREAD_SIM_AND_RENDER
		// These effects get applied when there's a drawn frame that followed one or more sim updates.
		// They are not only registered on drawn sim updates; flashes and stuff could be missed otherwise if they occur on undrawn sim updates.
		if (effect) {
			m_PostSceneEffects.push_back(PostEffect(effectPos, effect, hash, strength, angle));
		}
#endif
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool PostProcessMan::GetPostScreenEffectsWrapped(const Vector &boxPos, int boxWidth, int boxHeight, std::list<PostEffect> &effectsList, int team) {
		bool found = false;

		// Do the first unwrapped rect
		found = GetPostScreenEffects(boxPos, boxWidth, boxHeight, effectsList, team);

		int left = boxPos.GetFloorIntX();
		int top = boxPos.GetFloorIntY();
		int right = left + boxWidth;
		int bottom = top + boxHeight;

		if (g_SceneMan.SceneWrapsX()) {
			int sceneWidth = g_SceneMan.GetScene()->GetWidth();
			if (left < 0) {
				found = GetPostScreenEffects(left + sceneWidth, top, right + sceneWidth, bottom, effectsList, team) || found;
			}
			if (right >= sceneWidth) {
				found = GetPostScreenEffects(left - sceneWidth, top, right - sceneWidth, bottom, effectsList, team) || found;
			}
		}
		if (g_SceneMan.SceneWrapsY()) {
			int sceneHeight = g_SceneMan.GetScene()->GetHeight();
			if (top < 0) {
				found = GetPostScreenEffects(left, top + sceneHeight, right, bottom + sceneHeight, effectsList, team) || found;
			}
			if (bottom >= sceneHeight) {
				found = GetPostScreenEffects(left, top - sceneHeight, right, bottom - sceneHeight, effectsList, team) || found;
			}
		}
		return found;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	BITMAP *PostProcessMan::GetTempEffectBitmap(BITMAP *bitmap) const {
		// Get the largest dimension of the bitmap and convert it to a multiple of 16, i.e. 16, 32, etc
		int bitmapSizeNeeded = static_cast<int>(std::ceil(static_cast<float>(std::max(bitmap->w, bitmap->h)) / 16.0F)) * 16;
		std::unordered_map<int, BITMAP *>::const_iterator correspondingBitmapSizeEntry = m_TempEffectBitmaps.find(bitmapSizeNeeded);

		// If we didn't find a match then the bitmap size is greater than 512 but that's the biggest we've got, so return it
		if (correspondingBitmapSizeEntry == m_TempEffectBitmaps.end()) {
			correspondingBitmapSizeEntry = m_TempEffectBitmaps.find(512);
		}

		return correspondingBitmapSizeEntry->second;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PostProcessMan::RegisterGlowDotEffect(const Vector &effectPos, DotGlowColor color, int strength) {
		// These effects only apply only once per drawn sim update, and only on the first frame drawn after one or more sim updates
		if (color != NoDot) {
			RegisterPostEffect(effectPos, GetDotGlowEffect(color), GetDotGlowEffectHash(color), strength);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool PostProcessMan::GetGlowAreasWrapped(const Vector &boxPos, int boxWidth, int boxHeight, std::list<Box> &areaList) const {
		bool foundAny = false;
		Vector intRectPosRelativeToBox;

		// Account for wrapping in any registered glow IntRects, as well as on the box we're testing against
		std::list<IntRect> wrappedGlowRects;

		for (const IntRect &glowArea : m_GlowAreas) {
			g_SceneMan.WrapRect(glowArea, wrappedGlowRects);
		}
		std::list<IntRect> wrappedTestRects;
		g_SceneMan.WrapRect(IntRect(boxPos.GetFloorIntX(), boxPos.GetFloorIntY(), boxPos.GetFloorIntX() + boxWidth, boxPos.GetFloorIntY() + boxHeight), wrappedTestRects);

		// Check for intersections. If any are found, cut down the intersecting IntRect to the bounds of the IntRect we're testing against, then make and store a Box out of it
		for (IntRect &wrappedTestRect : wrappedTestRects) {
			for (const IntRect &wrappedGlowRect : wrappedGlowRects) {
				if (wrappedTestRect.Intersects(wrappedGlowRect)) {
					IntRect cutRect(wrappedGlowRect);
					cutRect.IntersectionCut(wrappedTestRect);
					intRectPosRelativeToBox = Vector(static_cast<float>(cutRect.m_Left) - boxPos.m_X, static_cast<float>(cutRect.m_Top) - boxPos.m_Y);
					areaList.push_back(Box(intRectPosRelativeToBox, static_cast<float>(cutRect.m_Right - cutRect.m_Left), static_cast<float>(cutRect.m_Bottom - cutRect.m_Top)));
					foundAny = true;
				}
			}
		}
		return foundAny;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PostProcessMan::GetNetworkPostEffectsList(int whichScreen, std::list<PostEffect> &outputList) {
		ScreenRelativeEffectsMutex.at(whichScreen).lock();
		outputList.clear();
		for (const PostEffect &postEffect : m_ScreenRelativeEffects.at(whichScreen)) {
			outputList.push_back(PostEffect(postEffect.m_Pos, postEffect.m_Bitmap, postEffect.m_BitmapHash, postEffect.m_Strength, postEffect.m_Angle));
		}
		ScreenRelativeEffectsMutex.at(whichScreen).unlock();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PostProcessMan::SetNetworkPostEffectsList(int whichScreen, std::list<PostEffect> &inputList) {
		ScreenRelativeEffectsMutex.at(whichScreen).lock();
		m_ScreenRelativeEffects.at(whichScreen).clear();
		for (const PostEffect &postEffect : inputList) {
			m_ScreenRelativeEffects.at(whichScreen).push_back(PostEffect(postEffect.m_Pos, postEffect.m_Bitmap, postEffect.m_BitmapHash, postEffect.m_Strength, postEffect.m_Angle));
		}
		ScreenRelativeEffectsMutex.at(whichScreen).unlock();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool PostProcessMan::GetPostScreenEffects(Vector boxPos, int boxWidth, int boxHeight, std::list<PostEffect> &effectsList, int team) {
		bool found = false;
		bool unseen = false;
		Vector postEffectPosRelativeToBox;

		if (g_SceneMan.GetScene()) {
			for (PostEffect &scenePostEffect : m_PostSceneEffects) {
				if (team != Activity::NoTeam) { unseen = g_SceneMan.IsUnseen(scenePostEffect.m_Pos.GetFloorIntX(), scenePostEffect.m_Pos.GetFloorIntY(), team); }

				if (WithinBox(scenePostEffect.m_Pos, boxPos, static_cast<float>(boxWidth), static_cast<float>(boxHeight)) && !unseen) {
					found = true;
					postEffectPosRelativeToBox = scenePostEffect.m_Pos - boxPos;
					effectsList.push_back(PostEffect(postEffectPosRelativeToBox, scenePostEffect.m_Bitmap, scenePostEffect.m_BitmapHash, scenePostEffect.m_Strength, scenePostEffect.m_Angle));
				}
			}
		}
		return found;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool PostProcessMan::GetPostScreenEffects(int left, int top, int right, int bottom, std::list<PostEffect> &effectsList, int team) {
		bool found = false;
		bool unseen = false;
		Vector postEffectPosRelativeToBox;

		for (PostEffect &scenePostEffect : m_PostSceneEffects) {
			if (team != Activity::NoTeam) {
				unseen = g_SceneMan.IsUnseen(scenePostEffect.m_Pos.GetFloorIntX(), scenePostEffect.m_Pos.GetFloorIntY(), team);
			}

			if (WithinBox(scenePostEffect.m_Pos, static_cast<float>(left), static_cast<float>(top), static_cast<float>(right), static_cast<float>(bottom)) && !unseen) {
				found = true;
				postEffectPosRelativeToBox = Vector(scenePostEffect.m_Pos.m_X - static_cast<float>(left), scenePostEffect.m_Pos.m_Y - static_cast<float>(top));
				effectsList.push_back(PostEffect(postEffectPosRelativeToBox, scenePostEffect.m_Bitmap, scenePostEffect.m_BitmapHash, scenePostEffect.m_Strength, scenePostEffect.m_Angle));
			}
		}
		return found;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	BITMAP * PostProcessMan::GetDotGlowEffect(DotGlowColor whichColor) const {
		switch (whichColor) {
			case NoDot:
				return nullptr;
			case YellowDot:
				return m_YellowGlow;
			case RedDot:
				return m_RedGlow;
			case BlueDot:
				return m_BlueGlow;
			default:
				RTEAbort("Undefined glow dot color value passed in. See DotGlowColor enumeration for defined values.");
				return nullptr;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	size_t PostProcessMan::GetDotGlowEffectHash(DotGlowColor whichColor) const {
		switch (whichColor) {
			case NoDot:
				return 0;
			case YellowDot:
				return m_YellowGlowHash;
			case RedDot:
				return m_RedGlowHash;
			case BlueDot:
				return m_BlueGlowHash;
			default:
				RTEAbort("Undefined glow dot color value passed in. See DotGlowColor enumeration for defined values.");
				return 0;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PostProcessMan::PostProcess() {
		UpdatePalette();

		// First copy the current 8bpp backbuffer to the 32bpp buffer; we'll add effects to it
		GL_CHECK(glDisable(GL_BLEND));
		GL_CHECK(glActiveTexture(GL_TEXTURE0));
		GL_CHECK(glBindTexture(GL_TEXTURE_2D, m_BackBuffer8));
		GL_CHECK(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, g_FrameMan.GetBackBuffer8()->w, g_FrameMan.GetBackBuffer8()->h, 0, GL_RED, GL_UNSIGNED_BYTE, g_FrameMan.GetBackBuffer8()->line[0]);
		GL_CHECK(glActiveTexture(GL_TEXTURE1));
		GL_CHECK(glBindTexture(GL_TEXTURE_1D, m_Palette8Texture));
		GL_CHECK(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_BlitFramebuffer));
		GL_CHECK(glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_BackBuffer32, 0));
		glViewport(0, 0, g_FrameMan.GetBackBuffer8()->w, g_FrameMan.GetBackBuffer8()->h);
		m_Blit8->Use();
		m_Blit8->SetInt(m_Blit8->GetTextureUniform(), 0);
		int paletteUniform = m_Blit8->GetUniformLocation("rtePalette");
		m_Blit8->SetInt(paletteUniform, 1);
		m_Blit8->SetMatrix4f(m_Blit8->GetProjectionUniform(), glm::mat4(1));
		m_Blit8->SetMatrix4f(m_Blit8->GetUVTransformUniform(), glm::mat4(1));
		m_Blit8->SetMatrix4f(m_Blit8->GetTransformUniform(), glm::mat4(1));
		GL_CHECK(glBindVertexArray(m_VertexArray));
		GL_CHECK(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));

		// Set the screen blender mode for glows
		set_screen_blender(128, 128, 128, 128);
		GL_CHECK(glEnable(GL_BLEND));
		GL_CHECK(glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD));
		GL_CHECK(glBlendFuncSeparate(GL_ONE, GL_ONE_MINUS_SRC_COLOR, GL_ONE, GL_ONE_MINUS_SRC_ALPHA));
		GL_CHECK(glBlendColor(0.5F, 0.5F, 0.5F, 0.5F));
		GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, m_PostProcessFramebuffer));
		GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_BackBuffer32, 0));
		glViewport(0, 0, g_FrameMan.GetBackBuffer8()->w, g_FrameMan.GetBackBuffer8()->h);

		m_PostProcessShader->Use();

		// TODO_MULTITHREAD: add post processing effects to RenderableGameState
#ifndef MULTITHREAD_SIM_AND_RENDER
		DrawDotGlowEffects();
		DrawPostScreenEffects();

		// Clear the effects list for this frame
		ClearScreenPostEffects();
#endif
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PostProcessMan::DrawDotGlowEffects() {
		int startX = 0;
		int startY = 0;
		int endX = 0;
		int endY = 0;
		int testpixel = 0;
		if(!m_YellowGlow->extra) {
			LazyInitBitmap(m_YellowGlow);
		}

		GL_CHECK(glBindVertexArray(m_VertexArray));
		GL_CHECK(glActiveTexture(GL_TEXTURE0));
		GL_CHECK(glBindTexture(GL_TEXTURE_2D, reinterpret_cast<GLBitmapInfo*>(m_YellowGlow->extra)->m_Texture));

		// Randomly sample the entire backbuffer, looking for pixels to put a glow on.
		for (const Box &glowBox : m_PostScreenGlowBoxes) {
			startX = glowBox.m_Corner.GetFloorIntX();
			startY = glowBox.m_Corner.GetFloorIntY();
			endX = startX + static_cast<int>(glowBox.m_Width);
			endY = startY + static_cast<int>(glowBox.m_Height);

			// Sanity check a little at least
			if (startX < 0 || startX >= g_FrameMan.GetBackBuffer8()->w || startY < 0 || startY >= g_FrameMan.GetBackBuffer8()->h ||
				endX < 0 || endX >= g_FrameMan.GetBackBuffer8()->w || endY < 0 || endY >= g_FrameMan.GetBackBuffer8()->h) {
				continue;
			}

#ifdef DEBUG_BUILD
			// Draw a rectangle around the glow box so we see it's position and size
			rect(g_FrameMan.GetBackBuffer32(), startX, startY, endX, endY, g_RedColor);
#endif

			for (int y = startY; y < endY; ++y) {
				for (int x = startX; x < endX; ++x) {
					testpixel = _getpixel(g_FrameMan.GetBackBuffer8(), x, y);

					// YELLOW
					if ((testpixel == g_YellowGlowColor && RandomNum() < 0.9F) || testpixel == 98 || (testpixel == 120 && RandomNum() < 0.7F)) {
						glm::mat4 transformMatrix(1);
						transformMatrix = glm::translate(transformMatrix, glm::vec3(x + 0.5f, y + 0.5f, 0));
						transformMatrix = glm::scale(transformMatrix, glm::vec3(m_YellowGlow->w * 0.5f, m_YellowGlow->h * 0.5f, 1.0));
						m_PostProcessShader->SetInt(m_PostProcessShader->GetTextureUniform(), 0);
						m_PostProcessShader->SetMatrix4f(m_PostProcessShader->GetProjectionUniform(), m_ProjectionMatrix);
						m_PostProcessShader->SetMatrix4f(m_PostProcessShader->GetTransformUniform(), transformMatrix);
						GL_CHECK(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
					}
					// TODO: Enable and add more colors once we actually have something that needs these.
					// RED
					/*
					if (testpixel == 13) {
						draw_trans_sprite(m_BackBuffer32, m_RedGlow, x - 2, y - 2);
					}
					// BLUE
					if (testpixel == 166) {
						draw_trans_sprite(g_FrameMan.GetBackBuffer32(), m_BlueGlow, x - 2, y - 2);
					}
					*/
				}
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PostProcessMan::DrawPostScreenEffects() {
		BITMAP *effectBitmap = nullptr;
		float effectPosX = 0;
		float effectPosY = 0;
		float effectStrength = 0;

		glActiveTexture(GL_TEXTURE0);
		glBindVertexArray(m_VertexArray);
		m_PostProcessShader->Use();
		m_PostProcessShader->SetInt(m_PostProcessShader->GetTextureUniform(), 0);
		m_PostProcessShader->SetMatrix4f(m_PostProcessShader->GetProjectionUniform(), m_ProjectionMatrix);

		for (const PostEffect &postEffect : m_PostScreenEffects) {
			if (postEffect.m_Bitmap) {
				if (!postEffect.m_Bitmap->extra) {
					LazyInitBitmap(postEffect.m_Bitmap);
				}
				effectBitmap = postEffect.m_Bitmap;
				effectStrength = postEffect.m_Strength / 255.f;
				effectPosX = postEffect.m_Pos.m_X;
				effectPosY = postEffect.m_Pos.m_Y;
				m_PostProcessShader->SetVector4f(m_PostProcessShader->GetColorUniform(), glm::vec4(effectStrength, effectStrength, effectStrength, 1.0f));

				glm::mat4 transformMatrix(1);
				transformMatrix = glm::translate(transformMatrix, glm::vec3(effectPosX, effectPosY, 0));
				transformMatrix = glm::rotate(transformMatrix, -postEffect.m_Angle, glm::vec3(0, 0, 1));
				transformMatrix = glm::scale(transformMatrix, glm::vec3(effectBitmap->w * 0.5f, effectBitmap->h * 0.5f, 1.0));

				glBindTexture(GL_TEXTURE_2D, reinterpret_cast<GLBitmapInfo*>(postEffect.m_Bitmap->extra)->m_Texture);
				m_PostProcessShader->SetMatrix4f(m_PostProcessShader->GetTransformUniform(), transformMatrix);

				glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
			}
		}
	}
} // namespace RTE