#include "TitleScreen.h"

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
		m_SlidePos.Reset();
		m_ScrollDuration = 0;
		m_ScrollStart = 0;
		m_ScrollProgress = 0;
		m_TitleAppearOffsetY = 900;
		m_MenuTopOffsetY = 0;
		m_FinishedPlayingIntro = false;
		m_FadeAmount = 255;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void TitleScreen::Create() {

		m_FadeScreen = create_bitmap_ex(32, g_FrameMan.GetResX(), g_FrameMan.GetResY());
		clear_to_color(m_FadeScreen, 0);

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

	void TitleScreen::PlayIntroLogoSequence(bool skipSection) {
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

	void TitleScreen::Draw() {
		g_FrameMan.ClearBackBuffer32();

		if (m_IntroSequenceState >= IntroSequence::DataRealmsLogoFadeIn && m_IntroSequenceState <= IntroSequence::DataRealmsLogoFadeOut) {
			draw_sprite(g_FrameMan.GetBackBuffer32(), m_DataRealmsLogo, (g_FrameMan.GetResX() - m_DataRealmsLogo->w) / 2 - 5, (g_FrameMan.GetResY() - m_DataRealmsLogo->h) / 2);

			char copyrightNotice[64];
			std::snprintf(copyrightNotice, sizeof(copyrightNotice), "Cortex Command is TM and %c 2017 Data Realms, LLC", -35);
			//g_FrameMan.GetLargeFont()->DrawAligned(&m_GUIBackBuffer, g_FrameMan.GetResX() / 2, g_FrameMan.GetResY() - g_FrameMan.GetLargeFont()->GetFontHeight(), std::string(copyrightNotice), GUIFont::Centre);
		}
		if (m_IntroSequenceState >= IntroSequence::FmodLogoFadeIn && m_IntroSequenceState <= IntroSequence::FmodLogoFadeOut) {
			draw_sprite(g_FrameMan.GetBackBuffer32(), m_FmodLogo, (g_FrameMan.GetResX() - m_FmodLogo->w) / 2, (g_FrameMan.GetResY() - m_FmodLogo->h) / 2);
		}

		if (m_FadeAmount > 0) {
			set_trans_blender(m_FadeAmount, m_FadeAmount, m_FadeAmount, m_FadeAmount);
			draw_trans_sprite(g_FrameMan.GetBackBuffer32(), m_FadeScreen, 0, 0);
		}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	}
}