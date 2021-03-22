#include "TitleScreen.h"

#include "AllegroScreen.h"

#include "FrameMan.h"
#include "SettingsMan.h"

#include "GUISound.h"

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void TitleScreen::Clear() {
		m_GUIBackBuffer = nullptr;
		m_IntroSequenceState = IntroSequence::Start;
		m_TitleTransitionState = TitleTransition::MenusDisabled;
		m_FadeScreen = nullptr;
		m_OrbitRotation = c_HalfPI - c_EighthPI;
		m_OrbitRadius = 274;
		m_PlanetRadius = 240;
		m_PreMainMenuOffsetY = 100;
		m_ScrollOffset.Reset();
		m_BackdropScrollRatio = 1.0F / 3.0F;
		m_ScrollDuration = 0;
		m_ScrollStart = 0;
		m_ScrollProgress = 0;
		m_TitleAppearOffsetY = 900;
		m_MenuTopOffsetY = 0;
		m_FinishedPlayingIntro = false;
		m_FadeAmount = 255;
		m_IntroSongTimer.Reset();
		m_SectionTimer.Reset();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void TitleScreen::Create() {
		m_GUIBackBuffer = AllegroBitmap(g_FrameMan.GetBackBuffer32());

		m_FadeScreen = create_bitmap_ex(32, g_FrameMan.GetResX(), g_FrameMan.GetResY());
		clear_to_color(m_FadeScreen, 0);

		CreateTitleElements();
		CreateBackdropStars();
		CreateIntroSequenceSlides();

		// Set Y to title offset so there's no jump when entering the main menu
		m_ScrollOffset = Vector(0, static_cast<float>(m_PreMainMenuOffsetY));
		m_BackdropScrollStartOffsetY = static_cast<int>(static_cast<float>(m_Nebula.GetBitmap()->h) / m_BackdropScrollRatio - (static_cast<float>(g_FrameMan.GetResY()) / m_BackdropScrollRatio));

		// So planet is centered on the screen regardless of resolution
		//int planetViewYOffset = 325 + m_PlanetRadius - (g_FrameMan.GetResY() / 2);

	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void TitleScreen::CreateTitleElements() {
		m_DataRealmsLogo = ContentFile("Base.rte/GUIs/Title/Intro/DRLogo5x.png").GetAsBitmap(COLORCONV_NONE, false);
		m_FmodLogo = ContentFile("Base.rte/GUIs/Title/Intro/FMODLogo.png").GetAsBitmap(COLORCONV_NONE, false);

		m_GameLogo.Create(ContentFile("Base.rte/GUIs/Title/Title.png"));
		m_GameLogoGlow.Create(ContentFile("Base.rte/GUIs/Title/TitleGlow.png"));
		m_Planet.Create(ContentFile("Base.rte/GUIs/Title/Planet.png"));
		m_Moon.Create(ContentFile("Base.rte/GUIs/Title/Moon.png"));
		m_Station.Create(ContentFile("Base.rte/GUIs/Title/Station.png"));
		m_Nebula.Create(ContentFile("Base.rte/GUIs/Title/Nebula.png"), false, Vector(), false, false, Vector(0, -1.0F));

		set_write_alpha_blender();
		draw_trans_sprite(m_GameLogo.GetSpriteFrame(0), ContentFile("Base.rte/GUIs/Title/TitleAlpha.png").GetAsBitmap(COLORCONV_NONE, false), 0, 0);
		draw_trans_sprite(m_Planet.GetSpriteFrame(0), ContentFile("Base.rte/GUIs/Title/PlanetAlpha.png").GetAsBitmap(COLORCONV_NONE, false), 0, 0);
		draw_trans_sprite(m_Moon.GetSpriteFrame(0), ContentFile("Base.rte/GUIs/Title/MoonAlpha.png").GetAsBitmap(COLORCONV_NONE, false), 0, 0);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void TitleScreen::CreateIntroSequenceSlides() {
		m_IntroSlides.at(0) = ContentFile("Base.rte/GUIs/Title/Intro/IntroSlideA.png").GetAsBitmap(COLORCONV_NONE, false);
		m_IntroSlides.at(1) = ContentFile("Base.rte/GUIs/Title/Intro/IntroSlideB.png").GetAsBitmap(COLORCONV_NONE, false);
		m_IntroSlides.at(2) = ContentFile("Base.rte/GUIs/Title/Intro/IntroSlideC.png").GetAsBitmap(COLORCONV_NONE, false);
		m_IntroSlides.at(3) = ContentFile("Base.rte/GUIs/Title/Intro/IntroSlideD.png").GetAsBitmap(COLORCONV_NONE, false);
		m_IntroSlides.at(4) = ContentFile("Base.rte/GUIs/Title/Intro/IntroSlideE.png").GetAsBitmap(COLORCONV_NONE, false);
		m_IntroSlides.at(5) = ContentFile("Base.rte/GUIs/Title/Intro/IntroSlideF.png").GetAsBitmap(COLORCONV_NONE, false);
		m_IntroSlides.at(6) = ContentFile("Base.rte/GUIs/Title/Intro/IntroSlideG.png").GetAsBitmap(COLORCONV_NONE, false);
		m_IntroSlides.at(7) = ContentFile("Base.rte/GUIs/Title/Intro/IntroSlideH.png").GetAsBitmap(COLORCONV_NONE, false);

		for (int i = 0; i < m_IntroSlides.size(); ++i) {
			if (m_IntroSlides.at(i)->h > g_FrameMan.GetResY()) {
				float targetHeight = static_cast<float>(g_FrameMan.GetResY()) - 80.0F;
				float targetWidth = static_cast<float>(m_IntroSlides.at(i)->w) * (targetHeight / static_cast<float>(m_IntroSlides.at(i)->h));
				BITMAP *resizeBitmap = create_bitmap_ex(32, static_cast<int>(targetWidth), static_cast<int>(targetHeight));
				// TODO: implement resizing with bilinear/whatever filtering because nearest neighbor looks pretty bad.
				stretch_blit(m_IntroSlides.at(i), resizeBitmap, 0, 0, m_IntroSlides.at(i)->w, m_IntroSlides.at(i)->h, 0, 0, resizeBitmap->w, resizeBitmap->h);
				destroy_bitmap(m_IntroSlides.at(i));
				m_IntroSlides.at(i) = resizeBitmap;
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void TitleScreen::CreateBackdropStars() {
		int starSmallBitmapCount = 4;
		int starLargeBitmapCount = 1;
		int starHugeBitmapCount = 2;
		BITMAP **starSmallBitmaps = ContentFile("Base.rte/GUIs/Title/Stars/StarSmall.png").GetAsAnimation(starSmallBitmapCount);
		BITMAP **starLargeBitmaps = ContentFile("Base.rte/GUIs/Title/Stars/StarLarge.png").GetAsAnimation(starLargeBitmapCount);
		BITMAP **starHugeBitmaps = ContentFile("Base.rte/GUIs/Title/Stars/StarHuge.png").GetAsAnimation(starHugeBitmapCount);

		int starCount = (g_FrameMan.GetResX() * m_Nebula.GetBitmap()->h) / 1000;
		for (int i = 0; i < starCount; ++i) {
			Star newStar;
			if (RandomNum() < 0.95F) {
				newStar.Bitmap = starSmallBitmaps[RandomNum(0, starSmallBitmapCount - 1)];
				newStar.Intensity = RandomNum(0, 92);
			} else if (RandomNum() < 0.85F) {
				newStar.Size = Star::StarSize::StarLarge;
				newStar.Bitmap = starLargeBitmaps[RandomNum(0, starLargeBitmapCount - 1)];
				newStar.Intensity = RandomNum(111, 185);
			} else {
				newStar.Size = Star::StarSize::StarHuge;
				newStar.Bitmap = starHugeBitmaps[RandomNum(0, starLargeBitmapCount - 1)];
				newStar.Intensity = RandomNum(166, 185);
			}
			newStar.PosX = RandomNum(0, g_FrameMan.GetResX());
			newStar.PosY = RandomNum(0, m_Nebula.GetBitmap()->h);
			newStar.ScrollRatio = m_BackdropScrollRatio;

			m_BackdropStars.emplace_back(newStar);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void TitleScreen::Destroy() {
		destroy_bitmap(m_FadeScreen);
		for (BITMAP *slide : m_IntroSlides) {
			destroy_bitmap(slide);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	TitleScreen::TitleTransition TitleScreen::Update(bool skipSection) {
		if (!m_FinishedPlayingIntro && !g_SettingsMan.SkipIntro()) {
			UpdateIntro(skipSection);
			return TitleTransition::MenusDisabled;
		}
		return m_TitleTransitionState;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void TitleScreen::DrawGameLogo() {
		//if (m_IntroSequenceState == MAINTOSCENARIO || m_IntroSequenceState == MAINTOCAMPAIGN) {
		//	m_GameLogo.SetPos(Vector(static_cast<float>(g_FrameMan.GetResX() / 2), EaseOut(64, -150, m_SectionProgress)));
		//} else if (m_IntroSequenceState >= IntroSequence::MainMenuAppear) {
		//	m_GameLogo.SetPos(Vector(static_cast<float>(g_FrameMan.GetResX() / 2), 64));
		//}
		m_GameLogo.Draw(g_FrameMan.GetBackBuffer32(), Vector(), g_DrawAlpha);
		m_GameLogoGlow.SetPos(m_GameLogo.GetPos());
		int blendAmount = 220 + RandomNum(-35, 35);
		set_screen_blender(blendAmount, blendAmount, blendAmount, blendAmount);
		m_GameLogoGlow.Draw(g_FrameMan.GetBackBuffer32(), Vector(), g_DrawTrans);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void TitleScreen::DrawTitleScreenScene() {
		Vector nebulaPos = Vector(0, m_ScrollOffset.GetY() * m_BackdropScrollRatio);
		if (m_Nebula.GetBitmap()->w != g_FrameMan.GetResX()) { nebulaPos.SetX(static_cast<float>((m_Nebula.GetBitmap()->w - g_FrameMan.GetResX()) / 2)); }
		m_Nebula.Draw(g_FrameMan.GetBackBuffer32(), Box(), nebulaPos);

		for (const Star &star : m_BackdropStars) {
			int intensity = star.Intensity + RandomNum(0, (star.Size == Star::StarSize::StarSmall) ? 35 : 70);
			set_screen_blender(intensity, intensity, intensity, intensity);
			int starDrawPosY = star.PosY - static_cast<int>(m_ScrollOffset.GetY() * star.ScrollRatio);
			draw_trans_sprite(g_FrameMan.GetBackBuffer32(), star.Bitmap, star.PosX, starDrawPosY);
		}

		m_PlanetPos.SetXY(static_cast<float>(g_FrameMan.GetResX() / 2), static_cast<float>(567 - m_ScrollOffset.GetFloorIntY()));
		m_Moon.SetPos(Vector(m_PlanetPos.GetX() + 200, 364 - (m_ScrollOffset.GetY() * 0.60F)));
		m_Moon.Draw(g_FrameMan.GetBackBuffer32(), Vector(), DrawMode::g_DrawAlpha);
		m_Planet.SetPos(m_PlanetPos);
		m_Planet.Draw(g_FrameMan.GetBackBuffer32(), Vector(), DrawMode::g_DrawAlpha);

		m_StationOffset.SetXY(m_OrbitRadius, 0);
		m_StationOffset.RadRotate(m_OrbitRotation);
		m_Station.SetPos(m_PlanetPos + m_StationOffset);
		m_Station.SetRotAngle(-c_HalfPI + m_OrbitRotation);
		m_Station.Draw(g_FrameMan.GetBackBuffer32());

		m_OrbitRotation -= 0.0020F;
		// Keep the rotation angle from getting too large
		if (m_OrbitRotation < -c_TwoPI) { m_OrbitRotation += c_TwoPI; }

		m_StationOffsetX = m_StationOffset.GetFloorIntX();
		m_StationOffsetY = m_StationOffset.GetFloorIntY();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void TitleScreen::Draw() {
		if (!m_FinishedPlayingIntro && !g_SettingsMan.SkipIntro()) {
			DrawIntro();
			return;
		}

		DrawTitleScreenScene();
		DrawGameLogo();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void TitleScreen::UpdateIntroLogoSequence(bool skipSection) {
		switch (m_IntroSequenceState) {
			case IntroSequence::Start:
				m_SectionSwitch = true;
				m_IntroSequenceState = IntroSequence::DataRealmsLogoFadeIn;
				break;
			case IntroSequence::DataRealmsLogoFadeIn:
				if (m_SectionSwitch) {
					m_SectionSwitch = false;
					m_SectionDuration = 0.25F;
					g_GUISound.SplashSound()->Play();
				}
				m_FadeAmount = 255 - static_cast<int>(255.0F * m_SectionProgress);
				if (m_SectionElapsedTime >= m_SectionDuration) {
					m_SectionSwitch = true;
					m_IntroSequenceState = IntroSequence::DataRealmsLogoDisplay;
				} else if (skipSection) {
					m_SectionSwitch = true;
					m_IntroSequenceState = IntroSequence::DataRealmsLogoFadeOut;
				}
				break;
			case IntroSequence::DataRealmsLogoDisplay:
				if (m_SectionSwitch) {
					m_SectionSwitch = false;
					m_SectionDuration = 2.0F;
				}
				if (m_SectionElapsedTime > m_SectionDuration || skipSection) {
					m_SectionSwitch = true;
					m_IntroSequenceState = IntroSequence::DataRealmsLogoFadeOut;
				}
				break;
			case IntroSequence::DataRealmsLogoFadeOut:
				if (m_SectionSwitch) {
					m_SectionSwitch = false;
					m_SectionDuration = 0.25F;
				}
				m_FadeAmount = static_cast<int>(255.0F * m_SectionProgress);
				if (m_SectionElapsedTime >= m_SectionDuration || skipSection) {
					m_SectionSwitch = true;
					m_IntroSequenceState = IntroSequence::FmodLogoFadeIn;
				}
				break;
			case IntroSequence::FmodLogoFadeIn:
				if (m_SectionSwitch) {
					m_SectionSwitch = false;
					m_SectionDuration = 0.25F;
				}
				m_FadeAmount = 255 - static_cast<int>(255.0F * m_SectionProgress);
				if (m_SectionElapsedTime >= m_SectionDuration) {
					m_SectionSwitch = true;
					m_IntroSequenceState = IntroSequence::FmodLogoDisplay;
				} else if (skipSection) {
					m_SectionSwitch = true;
					m_IntroSequenceState = IntroSequence::FmodLogoFadeOut;
				}
				break;
			case IntroSequence::FmodLogoDisplay:
				if (m_SectionSwitch) {
					m_SectionSwitch = false;
					m_SectionDuration = 2.0F;
				}
				if (m_SectionElapsedTime > m_SectionDuration || skipSection) {
					m_SectionSwitch = true;
					m_IntroSequenceState = IntroSequence::FmodLogoFadeOut;
				}
				break;
			case IntroSequence::FmodLogoFadeOut:
				if (m_SectionSwitch) {
					m_SectionSwitch = false;
					m_SectionDuration = 0.25F;
				}
				m_FadeAmount = static_cast<int>(255.0F * m_SectionProgress);
				if (m_SectionElapsedTime >= m_SectionDuration || skipSection) {
					m_SectionSwitch = true;
					m_IntroSequenceState = IntroSequence::SlideshowFadeIn;
				}
				break;
			default:
				break;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void TitleScreen::UpdateIntroSlideshowSequence(bool skipSection) {
		int textPosY = 0;
		switch (m_IntroSequenceState) {
			case IntroSequence::SlideshowFadeIn:
				if (m_SectionSwitch) {
					m_SectionSwitch = false;
					m_ScrollOffset.SetY(static_cast<float>(m_BackdropScrollStartOffsetY));
					m_SectionDuration = 1.0F;

					g_AudioMan.PlayMusic("Base.rte/Music/Hubnester/ccintro.ogg", 0);
					g_AudioMan.SetMusicPosition(0.05F);
					// Override music volume setting for the intro if it's set to anything
					if (g_AudioMan.GetMusicVolume() > 0.1F) { g_AudioMan.SetTempMusicVolume(1.0F); }
					m_IntroSongTimer.SetElapsedRealTimeS(0.05F);
				}
				m_FadeAmount = 255 - static_cast<int>(255.0F * m_SectionProgress);
				if (m_SectionElapsedTime >= m_SectionDuration) {
					m_SectionSwitch = true;
					m_IntroSequenceState = IntroSequence::PreSlideshowPause;
				}
				break;
			case IntroSequence::PreSlideshowPause:
				if (m_SectionSwitch) {
					m_SectionSwitch = false;
					m_SectionDuration = 3.7F - static_cast<float>(m_IntroSongTimer.GetElapsedRealTimeS());
				}
				if (m_SectionElapsedTime >= m_SectionDuration) {
					m_SectionSwitch = true;
					m_IntroSequenceState = IntroSequence::ShowSlide1;
				}
				break;
			case IntroSequence::ShowSlide1:
				if (m_SectionSwitch) {
					m_SectionSwitch = false;
					m_SlideFadeInDuration = 2.0F;
					m_SlideFadeOutDuration = 0.5F;
					m_SectionDuration = 11.4F - static_cast<float>(m_IntroSongTimer.GetElapsedRealTimeS());
				}
				textPosY = (g_FrameMan.GetResY() / 2) + (m_IntroSlides.at(m_IntroSequenceState - IntroSequence::ShowSlide1)->h / 2) + 12;
				if (m_SectionElapsedTime > 1.25F) {
					//g_FrameMan.GetLargeFont()->DrawAligned(&m_GUIBackBuffer, g_FrameMan.GetResX() / 2, textPosY, "At the end of humanity's darkest century...", GUIFont::Centre);
				}

				if (m_SectionElapsedTime >= m_SectionDuration) {
					m_IntroSequenceState = IntroSequence::ShowSlide2;
					m_SectionSwitch = true;
				}
				break;
			case IntroSequence::ShowSlide2:
				if (m_SectionSwitch) {
					m_SlideFadeInDuration = 0.5F;
					m_SlideFadeOutDuration = 2.5F;
					m_SectionDuration = 17.3F - static_cast<float>(m_IntroSongTimer.GetElapsedRealTimeS());
					m_SectionSwitch = false;
				}
				textPosY = (g_FrameMan.GetResY() / 2) + (m_IntroSlides.at(m_IntroSequenceState - IntroSequence::ShowSlide1)->h / 2) + 12;
				if (m_SectionElapsedTime < m_SectionDuration - 1.75F) {
					//g_FrameMan.GetLargeFont()->DrawAligned(&m_GUIBackBuffer, g_FrameMan.GetResX() / 2, textPosY, "...a curious symbiosis between man and machine emerged.", GUIFont::Centre);
				}
				if (m_SectionElapsedTime >= m_SectionDuration) {
					m_IntroSequenceState = IntroSequence::ShowSlide3;
					m_SectionSwitch = true;
				}
				break;
			case IntroSequence::ShowSlide3:
				if (m_SectionSwitch) {
					m_SlideFadeInDuration = 0.5F;
					m_SlideFadeOutDuration = 0.5F;
					m_SectionDuration = 25.1F - static_cast<float>(m_IntroSongTimer.GetElapsedRealTimeS());
					m_SectionSwitch = false;
				}
				textPosY = (g_FrameMan.GetResY() / 2) + (m_IntroSlides.at(m_IntroSequenceState - IntroSequence::ShowSlide1)->h / 2) + 12;
				if (m_SectionProgress < 0.49F) {
					//g_FrameMan.GetLargeFont()->DrawAligned(&m_GUIBackBuffer, g_FrameMan.GetResX() / 2, textPosY, "This eventually enabled humans to leave their natural bodies...", GUIFont::Centre);
				} else if (m_SectionProgress > 0.51F) {
					//g_FrameMan.GetLargeFont()->DrawAligned(&m_GUIBackBuffer, g_FrameMan.GetResX() / 2, textPosY, "...and to free their minds from obsolete constraints.", GUIFont::Centre);
				}
				if (m_SectionElapsedTime >= m_SectionDuration) {
					m_IntroSequenceState = IntroSequence::ShowSlide4;
					m_SectionSwitch = true;
				}
				break;
			case IntroSequence::ShowSlide4:
				if (m_SectionSwitch) {
					m_SlideFadeInDuration = 0.5F;
					m_SlideFadeOutDuration = 0.5F;
					m_SectionDuration = 31.3F - static_cast<float>(m_IntroSongTimer.GetElapsedRealTimeS());
					m_SectionSwitch = false;
				}
				textPosY = (g_FrameMan.GetResY() / 2) + (m_IntroSlides.at(m_IntroSequenceState - IntroSequence::ShowSlide1)->h / 2) + 12;
				//g_FrameMan.GetLargeFont()->DrawAligned(&m_GUIBackBuffer, g_FrameMan.GetResX() / 2, textPosY, "With their brains sustained by artificial means, space travel also became feasible.", GUIFont::Centre);
				if (m_SectionElapsedTime >= m_SectionDuration) {
					m_IntroSequenceState = IntroSequence::ShowSlide5;
					m_SectionSwitch = true;
				}
				break;
			case IntroSequence::ShowSlide5:
				if (m_SectionSwitch) {
					m_SlideFadeInDuration = 0.5F;
					m_SlideFadeOutDuration = 0.5F;
					m_SectionDuration = 38.0F - static_cast<float>(m_IntroSongTimer.GetElapsedRealTimeS());
					m_SectionSwitch = false;
				}
				textPosY = (g_FrameMan.GetResY() / 2) + (m_IntroSlides.at(m_IntroSequenceState - IntroSequence::ShowSlide1)->h / 2) + 12;
				//g_FrameMan.GetLargeFont()->DrawAligned(&m_GUIBackBuffer, g_FrameMan.GetResX() / 2, textPosY, "Other civilizations were encountered...", GUIFont::Centre);
				if (m_SectionElapsedTime >= m_SectionDuration) {
					m_IntroSequenceState = IntroSequence::ShowSlide6;
					m_SectionSwitch = true;
				}
				break;
			case IntroSequence::ShowSlide6:
				if (m_SectionSwitch) {
					m_SlideFadeInDuration = 0.5F;
					m_SlideFadeOutDuration = 0.5F;
					m_SectionDuration = 44.1F - static_cast<float>(m_IntroSongTimer.GetElapsedRealTimeS());
					m_SectionSwitch = false;
				}
				textPosY = (g_FrameMan.GetResY() / 2) + (m_IntroSlides.at(m_IntroSequenceState - IntroSequence::ShowSlide1)->h / 2) + 12;
				//g_FrameMan.GetLargeFont()->DrawAligned(&m_GUIBackBuffer, g_FrameMan.GetResX() / 2, textPosY, "...and peaceful intragalactic trade soon established.", GUIFont::Centre);
				if (m_SectionElapsedTime >= m_SectionDuration) {
					m_IntroSequenceState = IntroSequence::ShowSlide7;
					m_SectionSwitch = true;
				}
				break;
			case IntroSequence::ShowSlide7:
				if (m_SectionSwitch) {
					m_SlideFadeInDuration = 0.5F;
					m_SlideFadeOutDuration = 0.5F;
					m_SectionDuration = 51.5F - static_cast<float>(m_IntroSongTimer.GetElapsedRealTimeS());
					m_SectionSwitch = false;
				}
				textPosY = (g_FrameMan.GetResY() / 2) + (m_IntroSlides.at(m_IntroSequenceState - IntroSequence::ShowSlide1)->h / 2) + 12;
				//g_FrameMan.GetLargeFont()->DrawAligned(&m_GUIBackBuffer, g_FrameMan.GetResX() / 2, textPosY, "Now, the growing civilizations create a huge demand for resources...", GUIFont::Centre);
				if (m_SectionElapsedTime >= m_SectionDuration) {
					m_IntroSequenceState = IntroSequence::ShowSlide8;
					m_SectionSwitch = true;
				}
				break;
			case IntroSequence::ShowSlide8:
				if (m_SectionSwitch) {
					m_SlideFadeInDuration = 0.5F;
					m_SlideFadeOutDuration = 0.5F;
					m_SectionDuration = 64.5F - static_cast<float>(m_IntroSongTimer.GetElapsedRealTimeS());
					m_SectionSwitch = false;
				}
				textPosY = (g_FrameMan.GetResY() / 2) + (m_IntroSlides.at(m_IntroSequenceState - IntroSequence::ShowSlide1)->h / 2) + 12;
				if (m_SectionProgress < 0.30F) {
					//g_FrameMan.GetLargeFont()->DrawAligned(&m_GUIBackBuffer, g_FrameMan.GetResX() / 2, textPosY, "...which can only be satisfied by the ever-expanding frontier.", GUIFont::Centre);
				} else if (m_SectionProgress > 0.33F && m_SectionProgress < 0.64F) {
					//g_FrameMan.GetLargeFont()->DrawAligned(&m_GUIBackBuffer, g_FrameMan.GetResX() / 2, textPosY, "Competition is brutal and anything goes in this galactic gold rush.", GUIFont::Centre);
				} else if (m_SectionProgress > 0.67F) {
					//g_FrameMan.GetLargeFont()->DrawAligned(&m_GUIBackBuffer, g_FrameMan.GetResX() / 2, textPosY, "You will now join it on a venture to an untapped planet...", GUIFont::Centre);
				}
				if (m_SectionElapsedTime >= m_SectionDuration) {
					m_IntroSequenceState = IntroSequence::SlideshowEnd;
					m_SectionSwitch = true;
				}
				break;
			case IntroSequence::SlideshowEnd:
				if (m_SectionSwitch) {
					m_SectionDuration = 66.6F - static_cast<float>(m_IntroSongTimer.GetElapsedRealTimeS());
					m_SectionSwitch = false;
				}
				if (m_SectionElapsedTime > 0.05F) {
					//g_FrameMan.GetLargeFont()->DrawAligned(&m_GUIBackBuffer, g_FrameMan.GetResX() / 2, g_FrameMan.GetResY() / 2, "Prepare to assume...", GUIFont::Centre);
				}
				if (m_SectionElapsedTime >= m_SectionDuration) {
					m_SectionSwitch = true;
					m_IntroSequenceState = IntroSequence::GameLogoAppear;
				}
				break;
			case IntroSequence::GameLogoAppear:
				if (m_SectionSwitch) {
					m_SectionSwitch = false;
					clear_to_color(m_FadeScreen, 0xFFFFFFFF); // White fade
					m_SectionDuration = 68.2F - static_cast<float>(m_IntroSongTimer.GetElapsedRealTimeS());
				}
				m_GameLogo.SetPos(Vector(static_cast<float>(g_FrameMan.GetResX() / 2), static_cast<float>(g_FrameMan.GetResY() / 2) - 20));
				m_FadeAmount = 255 - static_cast<int>(255.0F * m_SectionProgress);
				if (m_SectionElapsedTime >= m_SectionDuration) {
					m_SectionSwitch = true;
					m_IntroSequenceState = IntroSequence::PlanetScroll;
				}
				break;
			case IntroSequence::PlanetScroll:
				if (m_SectionSwitch) {
					m_SectionSwitch = false;
					m_SectionDuration = 92.4F - static_cast<float>(m_IntroSongTimer.GetElapsedRealTimeS());
				}
				if (m_SectionProgress > 0.5F) { m_GameLogo.SetPos(Vector(static_cast<float>(g_FrameMan.GetResX() / 2), EaseIn((static_cast<float>(g_FrameMan.GetResY() / 2)) - 20, 120, (m_SectionProgress - 0.5F) / 0.5F))); }
				if (m_SectionElapsedTime >= m_SectionDuration) {
					m_SectionSwitch = true;
					m_IntroSequenceState = IntroSequence::PreMainMenu;
				}
				break;
			case IntroSequence::PreMainMenu:
				if (m_SectionSwitch) {
					m_SectionSwitch = false;
					m_SectionDuration = 3.0F;
					m_ScrollOffset.SetY(static_cast<float>(m_PreMainMenuOffsetY));
				}
				if (m_SectionElapsedTime >= m_SectionDuration || skipSection) {
					m_SectionSwitch = true;
					m_IntroSequenceState = IntroSequence::MainMenuAppear;
				}
				break;
			case IntroSequence::MainMenuAppear:
				if (m_SectionSwitch) {
					m_SectionSwitch = false;
					clear_to_color(m_FadeScreen, 0);
					m_SectionDuration = 1.0F /* * g_SettingsMan.GetMenuTransitionDurationMultiplier()*/;
					m_ScrollOffset.SetY(static_cast<float>(m_PreMainMenuOffsetY));
					g_AudioMan.PlayMusic("Base.rte/Music/Hubnester/ccmenu.ogg", -1);
				}
				m_ScrollOffset.SetY(EaseOut(static_cast<float>(m_PreMainMenuOffsetY), static_cast<float>(m_MenuTopOffsetY), m_SectionProgress));
				m_GameLogo.SetPos(Vector(static_cast<float>(g_FrameMan.GetResX() / 2), EaseOut(120, 64, m_SectionProgress)));
				if (m_SectionElapsedTime >= m_SectionDuration /*|| g_NetworkServer.IsServerModeEnabled()*/) {
					m_SectionSwitch = true;
					m_TitleTransitionState = TitleTransition::MainMenu;
					m_FinishedPlayingIntro = true;
				}
				break;
			default:
				break;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void TitleScreen::UpdateIntro(bool skipSection) {
		if (m_SectionSwitch) { m_SectionTimer.Reset(); }
		m_SectionElapsedTime = static_cast<float>(m_SectionTimer.GetElapsedRealTimeS());
		m_SectionProgress = std::min((m_SectionDuration > 0) ? m_SectionElapsedTime / m_SectionDuration : 0, 0.9999F);

		if (m_IntroSequenceState >= IntroSequence::SlideshowFadeIn && m_IntroSequenceState <= IntroSequence::SlideshowEnd) {
			if (m_IntroSequenceState == IntroSequence::SlideshowFadeIn && m_SectionSwitch) {
				m_IntroSongTimer.SetElapsedRealTimeS(0.05F);
				m_ScrollStart = static_cast<float>(m_IntroSongTimer.GetElapsedRealTimeS());
				m_ScrollDuration = 66.6F - m_ScrollStart; // 66.6s This is the end of the slideshow
			}
			m_ScrollProgress = (static_cast<float>(m_IntroSongTimer.GetElapsedRealTimeS()) - m_ScrollStart) / m_ScrollDuration;
			m_ScrollOffset.SetY(LERP(0, 1.0F, static_cast<float>(m_BackdropScrollStartOffsetY), static_cast<float>(m_TitleAppearOffsetY), m_ScrollProgress));
		} else if (m_IntroSequenceState >= IntroSequence::GameLogoAppear && m_IntroSequenceState <= IntroSequence::PlanetScroll) {
			if (m_IntroSequenceState == IntroSequence::GameLogoAppear && m_SectionSwitch) {
				m_ScrollStart = static_cast<float>(m_IntroSongTimer.GetElapsedRealTimeS());
				m_ScrollDuration = 92.4F - m_ScrollStart; // 92.4s is the end of PlanetScroll
			}
			m_ScrollProgress = (static_cast<float>(m_IntroSongTimer.GetElapsedRealTimeS()) - m_ScrollStart) / m_ScrollDuration;
			m_ScrollOffset.SetY(EaseOut(static_cast<float>(m_TitleAppearOffsetY), static_cast<float>(m_PreMainMenuOffsetY), m_ScrollProgress));
		}

		if (m_IntroSequenceState >= IntroSequence::Start && m_IntroSequenceState <= IntroSequence::FmodLogoFadeOut) {
			UpdateIntroLogoSequence(skipSection);
		} else if (m_IntroSequenceState >= IntroSequence::SlideshowFadeIn && m_IntroSequenceState <= IntroSequence::MainMenuAppear) {
			UpdateIntroSlideshowSequence(skipSection);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void TitleScreen::DrawSlideshowSlide() {
		int slide = m_IntroSequenceState - IntroSequence::ShowSlide1;
		Vector slidePos(static_cast<float>((g_FrameMan.GetResX() / 2) - (m_IntroSlides.at(slide)->w / 2)), static_cast<float>((g_FrameMan.GetResY() / 2) - (m_IntroSlides.at(slide)->h / 2)));

		// Sideways pan slides that are wider than the screen
		if (m_IntroSlides.at(slide)->w > g_FrameMan.GetResX()) {
			if (m_SectionElapsedTime < m_SlideFadeInDuration) {
				slidePos.SetX(0);
			} else if (m_SectionElapsedTime < m_SectionDuration - m_SlideFadeOutDuration) {
				slidePos.SetX(EaseInOut(0, static_cast<float>(g_FrameMan.GetResX() - m_IntroSlides.at(slide)->w), (m_SectionElapsedTime - m_SlideFadeInDuration) / (m_SectionDuration - m_SlideFadeInDuration - m_SlideFadeOutDuration)));
			} else {
				slidePos.SetX(static_cast<float>(g_FrameMan.GetResX() - m_IntroSlides.at(slide)->w));
			}
		}
		m_FadeAmount = static_cast<int>((m_SectionElapsedTime < m_SlideFadeInDuration) ? EaseOut(0, 255.0F, m_SectionElapsedTime / m_SlideFadeInDuration) : EaseIn(255.0F, 0, (m_SectionElapsedTime - m_SectionDuration + m_SlideFadeOutDuration) / m_SlideFadeOutDuration));
		set_trans_blender(m_FadeAmount, m_FadeAmount, m_FadeAmount, m_FadeAmount);
		draw_trans_sprite(g_FrameMan.GetBackBuffer32(), m_IntroSlides.at(slide), slidePos.GetFloorIntX(), slidePos.GetFloorIntY());
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void TitleScreen::DrawIntro() {
		if (m_IntroSequenceState >= IntroSequence::SlideshowFadeIn) { DrawTitleScreenScene(); }
		if (m_IntroSequenceState >= IntroSequence::GameLogoAppear) { DrawGameLogo(); }

		bool drawFadeScreen = false;

		if (m_IntroSequenceState >= IntroSequence::DataRealmsLogoFadeIn && m_IntroSequenceState <= IntroSequence::DataRealmsLogoFadeOut) {
			draw_sprite(g_FrameMan.GetBackBuffer32(), m_DataRealmsLogo, (g_FrameMan.GetResX() - m_DataRealmsLogo->w) / 2 - 5, (g_FrameMan.GetResY() - m_DataRealmsLogo->h) / 2);

			std::string copyrightNotice(64, '\0');
			std::snprintf(copyrightNotice.data(), copyrightNotice.size(), "Cortex Command is TM and %c 2017 Data Realms, LLC", -35);
			//m_IntroTextFont.DrawAligned(&m_GUIBackBuffer, g_FrameMan.GetResX() / 2, g_FrameMan.GetResY() - m_IntroTextFont.GetFontHeight(), copyrightNotice, GUIFont::Centre);
			drawFadeScreen = true;
		} else if (m_IntroSequenceState >= IntroSequence::FmodLogoFadeIn && m_IntroSequenceState <= IntroSequence::FmodLogoFadeOut) {
			draw_sprite(g_FrameMan.GetBackBuffer32(), m_FmodLogo, (g_FrameMan.GetResX() - m_FmodLogo->w) / 2, (g_FrameMan.GetResY() - m_FmodLogo->h) / 2);
			drawFadeScreen = true;
		} else if (m_IntroSequenceState == IntroSequence::SlideshowFadeIn || m_IntroSequenceState == IntroSequence::GameLogoAppear) {
			drawFadeScreen = true;
		} else if (m_IntroSequenceState >= IntroSequence::ShowSlide1 && m_IntroSequenceState <= ShowSlide8) {
			DrawSlideshowSlide();
		}

		if (drawFadeScreen && m_FadeAmount > 0) {
			set_trans_blender(m_FadeAmount, m_FadeAmount, m_FadeAmount, m_FadeAmount);
			draw_trans_sprite(g_FrameMan.GetBackBuffer32(), m_FadeScreen, 0, 0);
		}
	}
}