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

	void TitleScreen::Draw() {
		g_FrameMan.ClearBackBuffer32();
	void TitleScreen::UpdateIntroSlideshowSequence(bool skipSection) {
		int textPosY = 0;
		switch (m_IntroSequenceState) {
			case IntroSequence::SlideshowFadeIn:
				if (m_SectionSwitch) {
					m_SectionSwitch = false;
					m_ScrollOffset.SetY(static_cast<float>(m_BackdropScrollStartOffsetY));
					m_SectionDuration = 1.0F;

		if (m_IntroSequenceState >= IntroSequence::DataRealmsLogoFadeIn && m_IntroSequenceState <= IntroSequence::DataRealmsLogoFadeOut) {
			draw_sprite(g_FrameMan.GetBackBuffer32(), m_DataRealmsLogo, (g_FrameMan.GetResX() - m_DataRealmsLogo->w) / 2 - 5, (g_FrameMan.GetResY() - m_DataRealmsLogo->h) / 2);
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

			char copyrightNotice[64];
			std::snprintf(copyrightNotice, sizeof(copyrightNotice), "Cortex Command is TM and %c 2017 Data Realms, LLC", -35);
			//g_FrameMan.GetLargeFont()->DrawAligned(&m_GUIBackBuffer, g_FrameMan.GetResX() / 2, g_FrameMan.GetResY() - g_FrameMan.GetLargeFont()->GetFontHeight(), std::string(copyrightNotice), GUIFont::Centre);
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
				m_FadeAmount = static_cast<int>(LERP(0, 0.5F, 255.0F, 0, m_SectionProgress));
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
				if (m_SectionElapsedTime >= m_SectionDuration /*|| g_NetworkServer.IsServerModeEnabled()*/) {
					m_SectionSwitch = true;
					m_TitleTransitionState = TitleTransition::MainMenu;
					m_FinishedPlayingIntro = true;
				}
				break;
			default:
				break;
		}
		if (m_IntroSequenceState >= IntroSequence::FmodLogoFadeIn && m_IntroSequenceState <= IntroSequence::FmodLogoFadeOut) {
			draw_sprite(g_FrameMan.GetBackBuffer32(), m_FmodLogo, (g_FrameMan.GetResX() - m_FmodLogo->w) / 2, (g_FrameMan.GetResY() - m_FmodLogo->h) / 2);
	}
		}

		if (m_FadeAmount > 0) {
			set_trans_blender(m_FadeAmount, m_FadeAmount, m_FadeAmount, m_FadeAmount);
			draw_trans_sprite(g_FrameMan.GetBackBuffer32(), m_FadeScreen, 0, 0);
		}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void TitleScreen::DrawSlideshowSlide() {
		int slide = m_IntroSequenceState - IntroSequence::ShowSlide1;
		Vector slideCenteredPos(static_cast<float>((g_FrameMan.GetResX() / 2) - (m_IntroSlides.at(slide)->w / 2)), static_cast<float>((g_FrameMan.GetResY() / 2) - (m_IntroSlides.at(slide)->h / 2)));
		Vector slidePos = slideCenteredPos;

		if (m_IntroSlides.at(slide)->w <= g_FrameMan.GetResX())
			slidePos.SetX(static_cast<float>((g_FrameMan.GetResX() / 2) - (m_IntroSlides.at(slide)->w / 2)));
		else {
			// Sideways pan slides that are wider than the screen
			if (m_SectionElapsedTime < m_SlideFadeInDuration) {
				slidePos.SetX(0);
			} else if (m_SectionElapsedTime < m_SectionDuration - m_SlideFadeOutDuration) {
				slidePos.SetX(EaseInOut(0, static_cast<float>(g_FrameMan.GetResX() - m_IntroSlides.at(slide)->w), (m_SectionElapsedTime - m_SlideFadeInDuration) / (m_SectionDuration - m_SlideFadeInDuration - m_SlideFadeOutDuration)));
			} else {
				slidePos.SetX(static_cast<float>(g_FrameMan.GetResX() - m_IntroSlides.at(slide)->w));
			}
		}

		int fadeAmount = 0;
		if (m_SectionElapsedTime < m_SlideFadeInDuration) {
			fadeAmount = static_cast<int>(EaseOut(0, 255.0F, m_SectionElapsedTime / m_SlideFadeInDuration));
		} else if (m_SectionElapsedTime < m_SectionDuration - m_SlideFadeOutDuration) {
			fadeAmount = 255;
			slidePos.SetY(slideCenteredPos.GetY());
		} else {
			fadeAmount = static_cast<int>(EaseIn(255.0F, 0, (m_SectionElapsedTime - m_SectionDuration + m_SlideFadeOutDuration) / m_SlideFadeOutDuration));
		}
		//if (fadeAmount > 0) {
			set_trans_blender(fadeAmount, fadeAmount, fadeAmount, fadeAmount);
			draw_trans_sprite(g_FrameMan.GetBackBuffer32(), m_IntroSlides.at(slide), slidePos.GetFloorIntX(), slidePos.GetFloorIntY());
		//}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	}
}