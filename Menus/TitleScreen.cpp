#include "TitleScreen.h"

#include "AllegroScreen.h"

#include "FrameMan.h"
#include "SettingsMan.h"

#include "GUISound.h"

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void TitleScreen::Clear() {
		m_GUIBackBuffer = nullptr;
		m_IntroSequenceState = IntroSequence::DataRealmsLogoFadeIn;
		//m_IntroSequenceState = IntroSequence::SlideshowEnd;
		//m_IntroSequenceState = IntroSequence::SlideshowFadeIn;
		//m_IntroSequenceState = IntroSequence::MainMenuAppear;
		m_TitleTransitionState = TitleTransition::Intro;
		m_ActiveMenu = ActiveMenu::MenusDisabled;
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
		m_FinishedPlayingIntro = false;
		m_FadeAmount = 255;
		m_IntroSongTimer.Reset();
		m_SectionTimer.Reset();
		m_ScreenResX = g_FrameMan.GetResX();
		m_ScreenResY = g_FrameMan.GetResY();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void TitleScreen::Create(GUIFont *introTextFont) {
		CreateTitleElements();

		m_FadeScreen = create_bitmap_ex(32, m_ScreenResX, m_ScreenResY);
		clear_to_color(m_FadeScreen, 0);

		m_BackdropScrollStartOffsetY = (static_cast<float>(m_Nebula.GetBitmap()->h) / m_BackdropScrollRatio) - (static_cast<float>(m_ScreenResY) / m_BackdropScrollRatio);
		m_ScrollOffset = Vector(0, m_BackdropScrollStartOffsetY);

		m_PlanetViewOffsetY = 325 + m_PlanetRadius - (static_cast<float>(m_ScreenResY) / 2);

		if (!g_SettingsMan.SkipIntro()) {
			m_GUIBackBuffer = AllegroBitmap(g_FrameMan.GetBackBuffer32());
			m_IntroTextFont = introTextFont;
			CreateIntroSequenceSlides();
		} else {
			m_IntroSequenceState = IntroSequence::MainMenuAppear;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void TitleScreen::CreateTitleElements() {
		m_DataRealmsLogo = ContentFile("Base.rte/GUIs/Title/Intro/DRLogo5x.png").GetAsBitmap(COLORCONV_NONE, false);
		m_FmodLogo = ContentFile("Base.rte/GUIs/Title/Intro/FMODLogo.png").GetAsBitmap(COLORCONV_NONE, false);

		m_PreGameLogoText.Create(ContentFile("Base.rte/GUIs/Title/Intro/PreTitle.png"));
		m_PreGameLogoTextGlow.Create(ContentFile("Base.rte/GUIs/Title/Intro/PreTitleGlow.png"));
		m_GameLogo.Create(ContentFile("Base.rte/GUIs/Title/Title.png"));
		m_GameLogoGlow.Create(ContentFile("Base.rte/GUIs/Title/TitleGlow.png"));
		m_Planet.Create(ContentFile("Base.rte/GUIs/Title/Planet.png"));
		m_Moon.Create(ContentFile("Base.rte/GUIs/Title/Moon.png"));
		m_Station.Create(ContentFile("Base.rte/GUIs/Title/Station.png"));
		m_Nebula.Create(ContentFile("Base.rte/GUIs/Title/Nebula.png"), false, Vector(), false, false, Vector(0, -1.0F));

		set_write_alpha_blender();
		draw_trans_sprite(m_PreGameLogoText.GetSpriteFrame(0), ContentFile("Base.rte/GUIs/Title/Intro/PreTitleAlpha.png").GetAsBitmap(COLORCONV_NONE, false), 0, 0);
		draw_trans_sprite(m_GameLogo.GetSpriteFrame(0), ContentFile("Base.rte/GUIs/Title/TitleAlpha.png").GetAsBitmap(COLORCONV_NONE, false), 0, 0);
		draw_trans_sprite(m_Planet.GetSpriteFrame(0), ContentFile("Base.rte/GUIs/Title/PlanetAlpha.png").GetAsBitmap(COLORCONV_NONE, false), 0, 0);
		draw_trans_sprite(m_Moon.GetSpriteFrame(0), ContentFile("Base.rte/GUIs/Title/MoonAlpha.png").GetAsBitmap(COLORCONV_NONE, false), 0, 0);

		int starSmallBitmapCount = 4;
		int starLargeBitmapCount = 1;
		int starHugeBitmapCount = 2;
		BITMAP **starSmallBitmaps = ContentFile("Base.rte/GUIs/Title/Stars/StarSmall.png").GetAsAnimation(starSmallBitmapCount);
		BITMAP **starLargeBitmaps = ContentFile("Base.rte/GUIs/Title/Stars/StarLarge.png").GetAsAnimation(starLargeBitmapCount);
		BITMAP **starHugeBitmaps = ContentFile("Base.rte/GUIs/Title/Stars/StarHuge.png").GetAsAnimation(starHugeBitmapCount);

		int starCount = (m_ScreenResX * m_Nebula.GetBitmap()->h) / 1000;
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
			newStar.PosX = RandomNum(0, m_ScreenResX);
			newStar.PosY = RandomNum(0, m_Nebula.GetBitmap()->h);
			newStar.ScrollRatio = m_BackdropScrollRatio;

			m_BackdropStars.emplace_back(newStar);
		}
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
			if (m_IntroSlides.at(i)->h > m_ScreenResY) {
				float targetHeight = static_cast<float>(m_ScreenResY) - 80.0F;
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

	void TitleScreen::Destroy() {
		destroy_bitmap(m_FadeScreen);
		for (BITMAP *slide : m_IntroSlides) {
			destroy_bitmap(slide);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void TitleScreen::UpdateMenuTransitions() {
		switch (m_TitleTransitionState) {
			case TitleTransition::MainMenu:
				if (m_SectionSwitch) {
					m_SectionSwitch = false;
					m_ScrollOffset.SetY(0);
					m_ActiveMenu = ActiveMenu::MainMenuActive;
				}
				break;
			case TitleTransition::MainMenuToPlanet:
				if (m_SectionSwitch) {
					m_SectionSwitch = false;
					m_ScrollDuration = 2.0F * g_SettingsMan.GetMenuTransitionDurationMultiplier();
					g_GUISound.SplashSound()->Play();
					g_AudioMan.PlayMusic("Base.rte/Music/dBSoundworks/thisworld5.ogg", -1);
					m_ActiveMenu = ActiveMenu::ScenarioMenuActive;
				}
				m_ScrollOffset.SetY(EaseOut(0, m_PlanetViewOffsetY, m_SectionProgress));
				m_GameLogo.SetPos(Vector(static_cast<float>(m_ScreenResX / 2), EaseOut(64, -150, m_SectionProgress)));
				if (m_SectionElapsedTime >= m_ScrollDuration /*|| g_NetworkServer.IsServerModeEnabled()*/) { SetTitleTransitionState(TitleTransition::ScenarioMenu); }
				break;
			case TitleTransition::PlanetToMainMenu:
				if (m_SectionSwitch) {
					m_SectionSwitch = false;
					m_ScrollDuration = 2.0F * g_SettingsMan.GetMenuTransitionDurationMultiplier();
					g_AudioMan.PlayMusic("Base.rte/Music/Hubnester/ccmenu.ogg", -1);
					m_ActiveMenu = ActiveMenu::MenusDisabled;
				}
				m_ScrollOffset.SetY(EaseOut(m_PlanetViewOffsetY, 0, m_SectionProgress));
				m_GameLogo.SetPos(Vector(static_cast<float>(m_ScreenResX / 2), EaseOut(-150, 64, m_SectionProgress)));
				if (m_SectionElapsedTime >= m_ScrollDuration / 2) { SetTitleTransitionState(TitleTransition::MainMenu); }
				break;
			case TitleTransition::ScenarioFadeIn:
				if (m_SectionSwitch) {
					m_SectionSwitch = false;
					m_ScrollOffset.SetY(m_PlanetViewOffsetY);
					m_ScrollDuration = 1.0F * g_SettingsMan.GetMenuTransitionDurationMultiplier();
				}
				m_FadeAmount = 255 - static_cast<int>(255.0F * m_SectionProgress);
				if (m_SectionElapsedTime >= m_SectionDuration) {
					SetTitleTransitionState(TitleTransition::ScenarioMenu);
					m_ActiveMenu = ActiveMenu::ScenarioMenuActive;
				}
				break;
			case TitleTransition::ScenarioMenu:
				if (m_SectionSwitch) {
					m_SectionSwitch = false;
					m_ScrollOffset.SetY(m_PlanetViewOffsetY);
					m_ActiveMenu = ActiveMenu::ScenarioMenuActive;
				}
				break;
			case TitleTransition::MainMenuToCampaign:
				/*
				if (m_SectionSwitch) {
					m_SectionSwitch = false;
					m_ScrollDuration = 2.0F * g_SettingsMan.GetMenuTransitionDurationMultiplier();
					g_GUISound.SplashSound()->Play();
					g_AudioMan.PlayMusic("Base.rte/Music/dBSoundworks/thisworld5.ogg", -1);
				}
				m_ScrollOffset.SetY(EaseOut(0, m_PlanetViewOffsetY, m_SectionProgress));
				m_GameLogo.SetPos(Vector(static_cast<float>(m_ScreenResX / 2), EaseOut(64, -150, m_SectionProgress)));
				if (m_SectionElapsedTime >= m_SectionDuration || keyPressed) {
					m_SectionSwitch = true;
					g_IntroState = CAMPAIGNPLAY;
				}
				*/
				break;
			case TitleTransition::CampaignFadeIn:
				/*
				if (m_SectionSwitch) {
					// Scroll to campaign pos
					m_ScrollOffset.SetY(m_PlanetViewOffsetY);
					// Black fade
					clear_to_color(m_FadeScreen, 0);

					m_ScrollDuration = 1.0F * g_SettingsMan.GetMenuTransitionDurationMultiplier();
					m_SectionSwitch = false;
				}

				m_FadeAmount = 255 - (255 * m_SectionProgress);
				set_trans_blender(m_FadeAmount, m_FadeAmount, m_FadeAmount, m_FadeAmount);
				draw_trans_sprite(g_FrameMan.GetBackBuffer32(), m_FadeScreen, 0, 0);

				if (m_SectionElapsedTime >= m_ScrollDuration) {
					g_IntroState = CAMPAIGNPLAY;
					m_SectionSwitch = true;
				}
				*/
				break;
			case TitleTransition::CampaignPlay:
				/*
				if (m_SectionSwitch) {
					m_ScrollOffset.SetY(static_cast<float>(m_PlanetViewOffsetY));
					m_SectionSwitch = false;
				}
				// Detect quitting of the program from the menu button
				g_Quit = g_Quit || g_MetaMan.GetGUI()->QuitProgram();

				// Detect if user wants to go back to main menu
				if (g_MetaMan.GetGUI()->BackToMain()) {
					g_IntroState = PLANETTOMAIN;
					m_SectionSwitch = true;
				}

				// Detect if a game has been commanded to restart
				if (g_MetaMan.GetGUI()->ActivityRestarted()) {
					// Make sure the scene is going to be reset with the new parameters
					g_ResetActivity = true;

					m_TitleTransitionState = FADEOUT;
					m_SectionSwitch = true;
				}
				// Detect if the current game has been commanded to resume
				if (g_MetaMan.GetGUI()->ActivityResumed()) { g_ResumeActivity = true; }
				*/
				break;
			case TitleTransition::FadeScrollOut:
				if (m_SectionSwitch) {
					m_SectionSwitch = false;
					m_ScrollDuration = 1.5F * g_SettingsMan.GetMenuTransitionDurationMultiplier();
				}
				m_ScrollOffset.SetY(EaseIn(0, 250, m_SectionProgress));
				m_FadeAmount = static_cast<int>(EaseIn(0, 255, m_SectionProgress));
				g_AudioMan.SetTempMusicVolume(EaseIn(g_AudioMan.GetMusicVolume(), 0, m_SectionProgress));
				if (m_SectionElapsedTime >= m_ScrollDuration) { SetTitleTransitionState(TitleTransition::End); }
				break;
			case TitleTransition::FadeOut:
				if (m_SectionSwitch) {
					m_SectionSwitch = false;
					m_ScrollDuration = 1.5F * g_SettingsMan.GetMenuTransitionDurationMultiplier();
				}
				m_FadeAmount = static_cast<int>(EaseIn(0, 255, m_SectionProgress));
				g_AudioMan.SetTempMusicVolume(EaseIn(g_AudioMan.GetMusicVolume(), 0, m_SectionProgress));
				if (m_SectionElapsedTime >= m_ScrollDuration) { SetTitleTransitionState(TitleTransition::End); }
				break;
			default:
				break;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	TitleScreen::ActiveMenu TitleScreen::Update(bool skipSection) {
		if (m_SectionSwitch) { m_SectionTimer.Reset(); }
		m_SectionElapsedTime = static_cast<float>(m_SectionTimer.GetElapsedRealTimeS());
		m_SectionProgress = std::min((m_SectionDuration > 0) ? m_SectionElapsedTime / m_SectionDuration : 0, 0.9999F);

		if (!m_FinishedPlayingIntro) {
			UpdateIntro(skipSection);
			return ActiveMenu::MenusDisabled;
		}
		//if (m_SectionSwitch) { UpdateMenuTransitions(); }
		UpdateMenuTransitions();
		return m_ActiveMenu;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void TitleScreen::DrawGameLogo() {
		m_GameLogo.Draw(g_FrameMan.GetBackBuffer32(), Vector(), g_DrawAlpha);
		m_GameLogoGlow.SetPos(m_GameLogo.GetPos());
		int glowIntensity = 220 + RandomNum(-35, 35);
		set_screen_blender(glowIntensity, glowIntensity, glowIntensity, glowIntensity);
		m_GameLogoGlow.Draw(g_FrameMan.GetBackBuffer32(), Vector(), g_DrawTrans);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void TitleScreen::DrawTitleScreenScene() {
		Vector nebulaPos = Vector(0, m_ScrollOffset.GetY() * m_BackdropScrollRatio);
		if (m_Nebula.GetBitmap()->w != m_ScreenResX) { nebulaPos.SetX(static_cast<float>((m_Nebula.GetBitmap()->w - m_ScreenResX) / 2)); }
		m_Nebula.Draw(g_FrameMan.GetBackBuffer32(), Box(), nebulaPos);

		for (const Star &star : m_BackdropStars) {
			int intensity = star.Intensity + RandomNum(0, (star.Size == Star::StarSize::StarSmall) ? 35 : 70);
			set_screen_blender(intensity, intensity, intensity, intensity);
			int starDrawPosY = star.PosY - static_cast<int>(m_ScrollOffset.GetY() * star.ScrollRatio);
			draw_trans_sprite(g_FrameMan.GetBackBuffer32(), star.Bitmap, star.PosX, starDrawPosY);
		}

		m_PlanetPos.SetXY(static_cast<float>(m_ScreenResX / 2), static_cast<float>(567 - m_ScrollOffset.GetFloorIntY()));
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
		if (!m_FinishedPlayingIntro) {
			DrawIntro();
			return;
		}
		DrawTitleScreenScene();
		DrawGameLogo();

		if (m_FadeAmount > 0) {
			//set_trans_blender(m_FadeAmount, m_FadeAmount, m_FadeAmount, m_FadeAmount);
			//draw_trans_sprite(g_FrameMan.GetBackBuffer32(), m_FadeScreen, 0, 0);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void TitleScreen::UpdateIntroLogoSequence(bool skipSection) {
		if (skipSection && m_IntroSequenceState != IntroSequence::FmodLogoFadeOut) {
			m_SectionSwitch = true;
			if (m_IntroSequenceState < IntroSequence::DataRealmsLogoFadeOut) {
				m_IntroSequenceState = IntroSequence::DataRealmsLogoFadeOut;
			} else if (m_IntroSequenceState == IntroSequence::DataRealmsLogoFadeOut) {
				m_IntroSequenceState = IntroSequence::FmodLogoFadeIn;
			} else if (m_IntroSequenceState < IntroSequence::FmodLogoFadeOut) {
				m_IntroSequenceState = IntroSequence::FmodLogoFadeOut;
			}
			return;
		}

		switch (m_IntroSequenceState) {
			case IntroSequence::DataRealmsLogoFadeIn:
				if (m_SectionSwitch) {
					m_SectionSwitch = false;
					m_SectionDuration = 0.5F;
					g_GUISound.SplashSound()->Play();
				}
				m_FadeAmount = 255 - static_cast<int>(255.0F * m_SectionProgress);
				break;
			case IntroSequence::DataRealmsLogoDisplay:
				if (m_SectionSwitch) {
					m_SectionSwitch = false;
					m_SectionDuration = 2.0F;
				}
				break;
			case IntroSequence::DataRealmsLogoFadeOut:
				if (m_SectionSwitch) {
					m_SectionSwitch = false;
					m_SectionDuration = 0.25F;
				}
				m_FadeAmount = static_cast<int>(255.0F * m_SectionProgress);
				break;
			case IntroSequence::FmodLogoFadeIn:
				if (m_SectionSwitch) {
					m_SectionSwitch = false;
					m_SectionDuration = 0.25F;
				}
				m_FadeAmount = 255 - static_cast<int>(255.0F * m_SectionProgress);
				break;
			case IntroSequence::FmodLogoDisplay:
				if (m_SectionSwitch) {
					m_SectionSwitch = false;
					m_SectionDuration = 2.0F;
				}
				break;
			case IntroSequence::FmodLogoFadeOut:
				if (m_SectionSwitch) {
					m_SectionSwitch = false;
					m_SectionDuration = 0.5F;
				}
				m_FadeAmount = static_cast<int>(255.0F * m_SectionProgress);
				break;
			default:
				break;
		}
		if (m_SectionElapsedTime >= m_SectionDuration) {
			m_SectionSwitch = true;
			m_IntroSequenceState = static_cast<IntroSequence>(static_cast<int>(m_IntroSequenceState) + 1);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void TitleScreen::UpdateIntroSlideshowSequence(bool skipSection) {
		if (skipSection && m_IntroSequenceState > IntroSequence::SlideshowFadeIn) {
			m_SectionSwitch = true;
			m_ScrollOffset.SetY(m_PreMainMenuOffsetY);
			m_IntroSequenceState = IntroSequence::MainMenuAppear;
			return;
		}
		m_SlideshowSlideText.clear();

		switch (m_IntroSequenceState) {
			case IntroSequence::SlideshowFadeIn:
				if (m_SectionSwitch) {
					m_SectionSwitch = false;
					m_SectionDuration = 1.5F;
					m_ScrollOffset.SetY(m_BackdropScrollStartOffsetY);

					g_AudioMan.PlayMusic("Base.rte/Music/Hubnester/ccintro.ogg", 0);
					g_AudioMan.SetMusicPosition(0.05F);
					// Override music volume setting for the intro if it's set to anything
					if (g_AudioMan.GetMusicVolume() > 0.1F) { g_AudioMan.SetTempMusicVolume(1.0F); }
					m_IntroSongTimer.SetElapsedRealTimeS(0.05F);
				}
				m_FadeAmount = 255 - static_cast<int>(255.0F * m_SectionProgress);
				break;
			case IntroSequence::PreSlideshowPause:
				if (m_SectionSwitch) {
					m_SectionSwitch = false;
					m_SectionDuration = 3.2F - static_cast<float>(m_IntroSongTimer.GetElapsedRealTimeS());
				}
				break;
			case IntroSequence::ShowSlide1:
				if (m_SectionSwitch) {
					m_SectionSwitch = false;
					m_SlideFadeInDuration = 2.0F;
					m_SlideFadeOutDuration = 0.5F;
					m_SectionDuration = 11.4F - static_cast<float>(m_IntroSongTimer.GetElapsedRealTimeS());
				}
				if (m_SectionElapsedTime > 1.25F) { m_SlideshowSlideText = "At the end of humanity's darkest century..."; }
				break;
			case IntroSequence::ShowSlide2:
				if (m_SectionSwitch) {
					m_SectionSwitch = false;
					m_SlideFadeInDuration = 0.5F;
					m_SlideFadeOutDuration = 2.5F;
					m_SectionDuration = 17.3F - static_cast<float>(m_IntroSongTimer.GetElapsedRealTimeS());
				}
				if (m_SectionElapsedTime < m_SectionDuration - 1.75F) { m_SlideshowSlideText = "...a curious symbiosis between man and machine emerged."; }
				break;
			case IntroSequence::ShowSlide3:
				if (m_SectionSwitch) {
					m_SectionSwitch = false;
					m_SlideFadeInDuration = 0.5F;
					m_SlideFadeOutDuration = 0.5F;
					m_SectionDuration = 25.1F - static_cast<float>(m_IntroSongTimer.GetElapsedRealTimeS());
				}
				if (m_SectionProgress < 0.49F) {
					m_SlideshowSlideText = "This eventually enabled humans to leave their natural bodies...";
				} else if (m_SectionProgress > 0.51F) {
					m_SlideshowSlideText = "...and to free their minds from obsolete constraints.";
				}
				break;
			case IntroSequence::ShowSlide4:
				if (m_SectionSwitch) {
					m_SectionSwitch = false;
					m_SlideFadeInDuration = 0.5F;
					m_SlideFadeOutDuration = 0.5F;
					m_SectionDuration = 31.3F - static_cast<float>(m_IntroSongTimer.GetElapsedRealTimeS());
				}
				m_SlideshowSlideText = "With their brains sustained by artificial means, space travel also became feasible.";
				break;
			case IntroSequence::ShowSlide5:
				if (m_SectionSwitch) {
					m_SectionSwitch = false;
					m_SlideFadeInDuration = 0.5F;
					m_SlideFadeOutDuration = 0.5F;
					m_SectionDuration = 38.0F - static_cast<float>(m_IntroSongTimer.GetElapsedRealTimeS());
				}
				m_SlideshowSlideText = "Other civilizations were encountered...";
				break;
			case IntroSequence::ShowSlide6:
				if (m_SectionSwitch) {
					m_SectionSwitch = false;
					m_SlideFadeInDuration = 0.5F;
					m_SlideFadeOutDuration = 0.5F;
					m_SectionDuration = 44.1F - static_cast<float>(m_IntroSongTimer.GetElapsedRealTimeS());
				}
				m_SlideshowSlideText = "...and peaceful intragalactic trade soon established.";
				break;
			case IntroSequence::ShowSlide7:
				if (m_SectionSwitch) {
					m_SectionSwitch = false;
					m_SlideFadeInDuration = 0.5F;
					m_SlideFadeOutDuration = 0.5F;
					m_SectionDuration = 51.5F - static_cast<float>(m_IntroSongTimer.GetElapsedRealTimeS());
				}
				m_SlideshowSlideText = "Now, the growing civilizations create a huge demand for resources...";
				break;
			case IntroSequence::ShowSlide8:
				if (m_SectionSwitch) {
					m_SectionSwitch = false;
					m_SlideFadeInDuration = 0.5F;
					m_SlideFadeOutDuration = 0.5F;
					m_SectionDuration = 64.5F - static_cast<float>(m_IntroSongTimer.GetElapsedRealTimeS());
				}
				if (m_SectionProgress < 0.3F) {
					m_SlideshowSlideText = "...which can only be satisfied by the ever-expanding frontier.";
				} else if (m_SectionProgress > 0.33F && m_SectionProgress < 0.64F) {
					m_SlideshowSlideText = "Competition is brutal and anything goes in this galactic gold rush.";
				} else if (m_SectionProgress > 0.67F) {
					m_SlideshowSlideText = "You will now join it on a venture to an untapped planet...";
				}
				break;
			case IntroSequence::SlideshowEnd:
				if (m_SectionSwitch) {
					m_SectionSwitch = false;
					m_SectionDuration = 66.6F - static_cast<float>(m_IntroSongTimer.GetElapsedRealTimeS());
				}
				m_PreGameLogoText.SetPos(Vector(static_cast<float>(m_ScreenResX / 2), static_cast<float>(m_ScreenResY / 2)));
				m_PreGameLogoTextGlow.SetPos(m_PreGameLogoText.GetPos());
				break;
			case IntroSequence::GameLogoAppear:
				if (m_SectionSwitch) {
					m_SectionSwitch = false;
					clear_to_color(m_FadeScreen, 0xFFFFFFFF); // White fade
					m_SectionDuration = 68.2F - static_cast<float>(m_IntroSongTimer.GetElapsedRealTimeS());
				}
				m_GameLogo.SetPos(Vector(static_cast<float>(m_ScreenResX / 2), static_cast<float>(m_ScreenResY / 2) - 20));
				m_FadeAmount = 255 - static_cast<int>(255.0F * m_SectionProgress);
				break;
			case IntroSequence::PlanetScroll:
				if (m_SectionSwitch) {
					m_SectionSwitch = false;
					m_SectionDuration = 92.4F - static_cast<float>(m_IntroSongTimer.GetElapsedRealTimeS());
				}
				if (m_SectionProgress > 0.5F) { m_GameLogo.SetPos(Vector(static_cast<float>(m_ScreenResX / 2), EaseIn((static_cast<float>(m_ScreenResY / 2)) - 20, 120, (m_SectionProgress - 0.5F) / 0.5F))); }
				break;
			case IntroSequence::PreMainMenu:
				if (m_SectionSwitch) {
					m_SectionSwitch = false;
					m_SectionDuration = 3.0F;
					m_ScrollOffset.SetY(m_PreMainMenuOffsetY);
				}
				break;
			case IntroSequence::MainMenuAppear:
				if (m_SectionSwitch) {
					m_SectionSwitch = false;
					m_SectionDuration = 1.0F * g_SettingsMan.GetMenuTransitionDurationMultiplier();
					g_AudioMan.PlayMusic("Base.rte/Music/Hubnester/ccmenu.ogg", -1);
				}
				m_ScrollOffset.SetY(EaseOut(m_PreMainMenuOffsetY, 0, m_SectionProgress));
				m_GameLogo.SetPos(Vector(static_cast<float>(m_ScreenResX / 2), EaseOut(120, 64, m_SectionProgress)));
				if (m_SectionElapsedTime >= m_SectionDuration /*|| g_NetworkServer.IsServerModeEnabled()*/) {
					m_FinishedPlayingIntro = true;
					clear_to_color(m_FadeScreen, 0);
					SetTitleTransitionState(TitleTransition::MainMenu);
					return;
				}
				break;
			default:
				break;
		}
		if (m_SectionElapsedTime >= m_SectionDuration) {
			m_SectionSwitch = true;
			m_IntroSequenceState = static_cast<IntroSequence>(static_cast<int>(m_IntroSequenceState) + 1);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void TitleScreen::UpdateIntro(bool skipSection) {
		if (m_IntroSequenceState >= IntroSequence::SlideshowFadeIn && m_IntroSequenceState <= IntroSequence::SlideshowEnd) {
			if (m_IntroSequenceState == IntroSequence::SlideshowFadeIn && m_SectionSwitch) {
				m_IntroSongTimer.SetElapsedRealTimeS(0.05F);
				m_ScrollStart = static_cast<float>(m_IntroSongTimer.GetElapsedRealTimeS());
				m_ScrollDuration = 66.6F - m_ScrollStart; // 66.6s This is the end of the slideshow
			}
			m_ScrollProgress = (static_cast<float>(m_IntroSongTimer.GetElapsedRealTimeS()) - m_ScrollStart) / m_ScrollDuration;
			m_ScrollOffset.SetY(LERP(0, 1.0F, m_BackdropScrollStartOffsetY, m_TitleAppearOffsetY, m_ScrollProgress));
		} else if (m_IntroSequenceState >= IntroSequence::GameLogoAppear && m_IntroSequenceState <= IntroSequence::PlanetScroll) {
			if (m_IntroSequenceState == IntroSequence::GameLogoAppear && m_SectionSwitch) {
				m_ScrollStart = static_cast<float>(m_IntroSongTimer.GetElapsedRealTimeS());
				m_ScrollDuration = 92.4F - m_ScrollStart; // 92.4s is the end of the planet scrolling
			}
			m_ScrollProgress = (static_cast<float>(m_IntroSongTimer.GetElapsedRealTimeS()) - m_ScrollStart) / m_ScrollDuration;
			m_ScrollOffset.SetY(EaseOut(m_TitleAppearOffsetY, m_PreMainMenuOffsetY, m_ScrollProgress));
		}

		if (m_IntroSequenceState >= IntroSequence::DataRealmsLogoFadeIn && m_IntroSequenceState <= IntroSequence::FmodLogoFadeOut) {
			UpdateIntroLogoSequence(skipSection);
		} else if (m_IntroSequenceState >= IntroSequence::SlideshowFadeIn && m_IntroSequenceState <= IntroSequence::MainMenuAppear) {
			UpdateIntroSlideshowSequence(skipSection);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void TitleScreen::DrawSlideshowSlide() {
		int slide = m_IntroSequenceState - IntroSequence::ShowSlide1;
		Vector slidePos(static_cast<float>((m_ScreenResX / 2) - (m_IntroSlides.at(slide)->w / 2)), static_cast<float>((m_ScreenResY / 2) - (m_IntroSlides.at(slide)->h / 2)));

		// Sideways pan slides that are wider than the screen
		if (m_IntroSlides.at(slide)->w > m_ScreenResX) {
			if (m_SectionElapsedTime < m_SlideFadeInDuration) {
				slidePos.SetX(0);
			} else if (m_SectionElapsedTime < m_SectionDuration - m_SlideFadeOutDuration) {
				slidePos.SetX(EaseInOut(0, static_cast<float>(m_ScreenResX - m_IntroSlides.at(slide)->w), (m_SectionElapsedTime - m_SlideFadeInDuration) / (m_SectionDuration - m_SlideFadeInDuration - m_SlideFadeOutDuration)));
			} else {
				slidePos.SetX(static_cast<float>(m_ScreenResX - m_IntroSlides.at(slide)->w));
			}
		}
		m_FadeAmount = static_cast<int>((m_SectionElapsedTime < m_SlideFadeInDuration) ? EaseOut(0, 255.0F, m_SectionElapsedTime / m_SlideFadeInDuration) : EaseIn(255.0F, 0, (m_SectionElapsedTime - m_SectionDuration + m_SlideFadeOutDuration) / m_SlideFadeOutDuration));
		set_trans_blender(m_FadeAmount, m_FadeAmount, m_FadeAmount, m_FadeAmount);
		draw_trans_sprite(g_FrameMan.GetBackBuffer32(), m_IntroSlides.at(slide), slidePos.GetFloorIntX(), slidePos.GetFloorIntY());

		if (!m_SlideshowSlideText.empty()) { m_IntroTextFont->DrawAligned(&m_GUIBackBuffer, m_ScreenResX / 2, (m_ScreenResY / 2) + (m_IntroSlides.at(m_IntroSequenceState - IntroSequence::ShowSlide1)->h / 2) + 12, m_SlideshowSlideText, GUIFont::Centre); }
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void TitleScreen::DrawIntro() {
		if (m_IntroSequenceState >= IntroSequence::SlideshowFadeIn) { DrawTitleScreenScene(); }
		if (m_IntroSequenceState >= IntroSequence::GameLogoAppear) { DrawGameLogo(); }

		bool drawFadeScreen = false;

		if (m_IntroSequenceState >= IntroSequence::DataRealmsLogoFadeIn && m_IntroSequenceState <= IntroSequence::DataRealmsLogoFadeOut) {
			draw_sprite(g_FrameMan.GetBackBuffer32(), m_DataRealmsLogo, (m_ScreenResX - m_DataRealmsLogo->w) / 2, (m_ScreenResY - m_DataRealmsLogo->h) / 2);

			std::string copyrightNotice(64, '\0');
			std::snprintf(copyrightNotice.data(), copyrightNotice.size(), "Cortex Command is TM and %c 2017 Data Realms, LLC", -35);
			m_IntroTextFont->DrawAligned(&m_GUIBackBuffer, m_ScreenResX / 2, m_ScreenResY - m_IntroTextFont->GetFontHeight(), copyrightNotice, GUIFont::Centre);
			drawFadeScreen = true;
		} else if (m_IntroSequenceState >= IntroSequence::FmodLogoFadeIn && m_IntroSequenceState <= IntroSequence::FmodLogoFadeOut) {
			draw_sprite(g_FrameMan.GetBackBuffer32(), m_FmodLogo, (m_ScreenResX - m_FmodLogo->w) / 2, (m_ScreenResY - m_FmodLogo->h) / 2);
			drawFadeScreen = true;
		} else if (m_IntroSequenceState == IntroSequence::SlideshowFadeIn || m_IntroSequenceState == IntroSequence::GameLogoAppear) {
			drawFadeScreen = true;
		} else if (m_IntroSequenceState >= IntroSequence::ShowSlide1 && m_IntroSequenceState <= ShowSlide8) {
			DrawSlideshowSlide();
		} else if (m_IntroSequenceState == IntroSequence::SlideshowEnd) {
			m_PreGameLogoText.Draw(g_FrameMan.GetBackBuffer32(), Vector(), g_DrawAlpha);
			int blendAmount = 220 + RandomNum(-35, 35);
			set_screen_blender(blendAmount, blendAmount, blendAmount, blendAmount);
			m_PreGameLogoTextGlow.Draw(g_FrameMan.GetBackBuffer32(), Vector(), g_DrawTrans);
		}
		if (drawFadeScreen && m_FadeAmount > 0) {
			set_trans_blender(m_FadeAmount, m_FadeAmount, m_FadeAmount, m_FadeAmount);
			draw_trans_sprite(g_FrameMan.GetBackBuffer32(), m_FadeScreen, 0, 0);
		}
	}
}