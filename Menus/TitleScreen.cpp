#include "TitleScreen.h"
#include "FrameMan.h"
#include "UInputMan.h"
#include "ActivityMan.h"
#include "SettingsMan.h"

#include "GUI.h"
#include "AllegroScreen.h"
#include "AllegroBitmap.h"

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void TitleScreen::Clear() {
		m_FadeAmount = 0;

		m_BackdropStars.clear();
		m_Nebula.Reset();
		m_Moon.Reset();
		m_Planet.Reset();
		m_PlanetPos.Reset();
		m_PlanetRadius = 240;
		m_Station.Reset();
		m_StationOffset.Reset();
		m_StationOrbitRotation = 0;
		m_StationOrbitTimer.Reset();
		m_StationOrbitTimerElapsedTime = 0;
		m_StationOrbitProgress = 0;
		m_OrbitRadius = 274;
		m_GameLogo.Reset();
		m_GameLogoGlow.Reset();
		m_GameLogoMainMenuOffsetY = 64;
		m_GameLogoPlanetViewOffsetY = -150;

		m_TitleTransitionState = TitleTransition::TransitionPending;

		m_SectionSwitch = true;
		m_SectionTimer.Reset();
		m_SectionElapsedTime = 0;
		m_SectionDuration = 0;
		m_SectionProgress = 0;

		m_ScrollOffset.Reset();
		m_BackdropScrollRatio = 1.0F / 3.0F;
		m_PlanetViewScrollOffsetY = 325 + m_PlanetRadius - (static_cast<float>(g_FrameMan.GetResY()) / 2);

		m_FinishedPlayingIntro = false;
		m_IntroSequenceState = IntroSequence::DataRealmsLogoFadeIn;
		m_IntroSongTimer.Reset();
		m_IntroScrollStartTime = 0;
		m_IntroScrollDuration = 0;
		m_IntroScrollStartOffsetY = 0;
		m_GameLogoAppearScrollOffsetY = 900;
		m_PreMainMenuScrollOffsetY = 100;
		m_SlideFadeInDuration = 0;
		m_SlideFadeOutDuration = 0;
		m_IntroTextFont = nullptr;
		m_SlideshowSlideText.clear();
		m_DataRealmsLogo = nullptr;
		m_FmodLogo = nullptr;
		m_PreGameLogoText.Reset();
		m_PreGameLogoTextGlow.Reset();

		m_IntroSlides.fill(nullptr);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void TitleScreen::Create(AllegroScreen *guiScreen) {
		CreateTitleElements();

		m_IntroScrollStartOffsetY = (static_cast<float>(m_Nebula.GetBitmap()->h) / m_BackdropScrollRatio) - (static_cast<float>(g_FrameMan.GetResY()) / m_BackdropScrollRatio);
		m_ScrollOffset.SetXY(0, m_IntroScrollStartOffsetY);

		m_StationOrbitTimer.SetElapsedRealTimeS(15);

		if (!g_FrameMan.ResolutionChanged()) {
			if (!g_SettingsMan.SkipIntro() && !g_ActivityMan.IsSetToLaunchIntoActivity()) {
				m_IntroTextFont = std::make_unique<GUIFont>("FontLarge");
				m_IntroTextFont->Load(guiScreen, "Base.rte/GUIs/Skins/Menus/FontLarge.png");
				CreateIntroSequenceSlides();
			} else {
				m_IntroSequenceState = IntroSequence::MainMenuAppear;
			}
		} else {
			SetTitleTransitionState(TitleTransition::MainMenu);
			m_FinishedPlayingIntro = true;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void TitleScreen::CreateTitleElements() {
		m_DataRealmsLogo = ContentFile("Base.rte/GUIs/Title/Intro/DRLogo5x.png").GetAsBitmap();
		m_FmodLogo = ContentFile("Base.rte/GUIs/Title/Intro/FMODLogo.png").GetAsBitmap();

		m_PreGameLogoText.Create(ContentFile("Base.rte/GUIs/Title/Intro/PreTitle.png"));
		m_PreGameLogoTextGlow.Create(ContentFile("Base.rte/GUIs/Title/Intro/PreTitleGlow.png"));
		m_GameLogo.Create(ContentFile("Base.rte/GUIs/Title/Title.png"));
		m_GameLogoGlow.Create(ContentFile("Base.rte/GUIs/Title/TitleGlow.png"));
		m_Planet.Create(ContentFile("Base.rte/GUIs/Title/Planet.png"));
		m_Moon.Create(ContentFile("Base.rte/GUIs/Title/Moon.png"));
		m_Station.Create(ContentFile("Base.rte/GUIs/Title/Station.png"));
		m_Nebula.Create(ContentFile("Base.rte/GUIs/Title/Nebula.png"), false, Vector(), false, false, Vector(0, -1.0F));

		set_write_alpha_blender();
		draw_trans_sprite(m_PreGameLogoText.GetSpriteFrame(0), ContentFile("Base.rte/GUIs/Title/Intro/PreTitleAlpha.png").GetAsBitmap(), 0, 0);
		draw_trans_sprite(m_GameLogo.GetSpriteFrame(0), ContentFile("Base.rte/GUIs/Title/TitleAlpha.png").GetAsBitmap(), 0, 0);
		draw_trans_sprite(m_Planet.GetSpriteFrame(0), ContentFile("Base.rte/GUIs/Title/PlanetAlpha.png").GetAsBitmap(), 0, 0);
		draw_trans_sprite(m_Moon.GetSpriteFrame(0), ContentFile("Base.rte/GUIs/Title/MoonAlpha.png").GetAsBitmap(), 0, 0);

		int starSmallBitmapCount = 4;
		std::vector<BITMAP *> starSmallBitmaps = ConvertDoublePointerToVectorOfPointers(ContentFile("Base.rte/GUIs/Title/Stars/StarSmall.png").GetAsAnimation(starSmallBitmapCount), starSmallBitmapCount);

		int starLargeBitmapCount = 1;
		std::vector<BITMAP *> starLargeBitmaps = ConvertDoublePointerToVectorOfPointers(ContentFile("Base.rte/GUIs/Title/Stars/StarLarge.png").GetAsAnimation(starLargeBitmapCount), starLargeBitmapCount);

		int starHugeBitmapCount = 2;
		std::vector<BITMAP *> starHugeBitmaps = ConvertDoublePointerToVectorOfPointers(ContentFile("Base.rte/GUIs/Title/Stars/StarHuge.png").GetAsAnimation(starHugeBitmapCount), starHugeBitmapCount);

		int starCount = (g_FrameMan.GetResX() * m_Nebula.GetBitmap()->h) / 1000;
		for (int i = 0; i < starCount; ++i) {
			Star newStar;
			if (RandomNum() < 0.95F) {
				newStar.Size = Star::StarSize::StarSmall;
				newStar.Bitmap = starSmallBitmaps.at(RandomNum(0, starSmallBitmapCount - 1));
				newStar.Intensity = RandomNum(0, 92);
			} else if (RandomNum() < 0.85F) {
				newStar.Size = Star::StarSize::StarLarge;
				newStar.Bitmap = starLargeBitmaps.at(RandomNum(0, starLargeBitmapCount - 1));
				newStar.Intensity = RandomNum(111, 185);
			} else {
				newStar.Size = Star::StarSize::StarHuge;
				newStar.Bitmap = starHugeBitmaps.at(RandomNum(0, starLargeBitmapCount - 1));
				newStar.Intensity = RandomNum(166, 185);
			}
			newStar.Position = Vector(RandomNum(0.0F, static_cast<float>(g_FrameMan.GetResX())), RandomNum(-100.0F, static_cast<float>(m_Nebula.GetBitmap()->h)));

			m_BackdropStars.emplace_back(newStar);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void TitleScreen::CreateIntroSequenceSlides() {
		std::string highRes = (g_FrameMan.GetResY() >= 680) ? "HD" : "";
		for (int slideNum = 0; slideNum < m_IntroSlides.size(); ++slideNum) {
			m_IntroSlides.at(slideNum) = ContentFile(("Base.rte/GUIs/Title/Intro/IntroSlide" + std::to_string(slideNum + 1) + highRes + ".png").c_str()).GetAsBitmap();
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void TitleScreen::Update() {
		if (m_SectionSwitch) { m_SectionTimer.Reset(); }
		m_SectionElapsedTime = static_cast<float>(m_SectionTimer.GetElapsedRealTimeS());
		m_SectionProgress = std::min((m_SectionDuration > 0) ? m_SectionElapsedTime / m_SectionDuration : 0, 0.9999F);

		// Checking for 0.999 instead of 0.9999 or 1.0 here otherwise there is a hiccup between ending and starting a new orbit cycle.
		if (m_StationOrbitProgress >= 0.999F) { m_StationOrbitTimer.Reset(); }
		m_StationOrbitProgress = std::clamp(static_cast<float>(m_StationOrbitTimer.GetElapsedRealTimeS()) / 60.0F, 0.0F, 0.9999F);
		m_StationOrbitRotation = LERP(0, 1.0F, c_PI, -c_PI, m_StationOrbitProgress);

		if (!m_FinishedPlayingIntro) {
			float introScrollProgress = (static_cast<float>(m_IntroSongTimer.GetElapsedRealTimeS()) - m_IntroScrollStartTime) / m_IntroScrollDuration;

			if (m_IntroSequenceState >= IntroSequence::DataRealmsLogoFadeIn && m_IntroSequenceState <= IntroSequence::FmodLogoFadeOut) {
				UpdateIntroLogoSequence(g_UInputMan.AnyStartPress());
			} else if (m_IntroSequenceState >= IntroSequence::SlideshowFadeIn && m_IntroSequenceState <= IntroSequence::SlideshowEnd) {
				m_ScrollOffset.SetY(LERP(0, 1.0F, m_IntroScrollStartOffsetY, m_GameLogoAppearScrollOffsetY, introScrollProgress));
				UpdateIntroSlideshowSequence(g_UInputMan.AnyStartPress());
			} else if (m_IntroSequenceState >= IntroSequence::GameLogoAppear && m_IntroSequenceState <= IntroSequence::MainMenuAppear) {
				if (m_IntroSequenceState < IntroSequence::PreMainMenu) { m_ScrollOffset.SetY(EaseOut(m_GameLogoAppearScrollOffsetY, m_PreMainMenuScrollOffsetY, introScrollProgress)); }
				UpdateIntroPreMainMenuSequence();
			}
			if (m_SectionElapsedTime >= m_SectionDuration) {
				m_SectionSwitch = true;
				m_IntroSequenceState = static_cast<IntroSequence>(static_cast<int>(m_IntroSequenceState) + 1);
			}
		} else {
			UpdateTitleTransitions();
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
					SetSectionDurationAndResetSwitch(0.5F);
					g_GUISound.SplashSound()->Play();
				}
				m_FadeAmount = static_cast<int>(LERP(0, 1.0F, 255.0F, 0, m_SectionProgress));
				break;
			case IntroSequence::DataRealmsLogoDisplay:
				if (m_SectionSwitch) { SetSectionDurationAndResetSwitch(2.0F); }
				break;
			case IntroSequence::DataRealmsLogoFadeOut:
				if (m_SectionSwitch) { SetSectionDurationAndResetSwitch(0.25F); }
				m_FadeAmount = static_cast<int>(LERP(0, 1.0F, 0, 255.0F, m_SectionProgress));
				break;
			case IntroSequence::FmodLogoFadeIn:
				if (m_SectionSwitch) { SetSectionDurationAndResetSwitch(0.25F); }
				m_FadeAmount = static_cast<int>(LERP(0, 1.0F, 255.0F, 0, m_SectionProgress));
				break;
			case IntroSequence::FmodLogoDisplay:
				if (m_SectionSwitch) { SetSectionDurationAndResetSwitch(2.0F); }
				break;
			case IntroSequence::FmodLogoFadeOut:
				if (m_SectionSwitch) { SetSectionDurationAndResetSwitch(0.5F); }
				m_FadeAmount = static_cast<int>(LERP(0, 1.0F, 0, 255.0F, m_SectionProgress));
				break;
			default:
				break;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void TitleScreen::UpdateIntroSlideshowSequence(bool skipSlideshow) {
		if (skipSlideshow && (m_IntroSequenceState > IntroSequence::SlideshowFadeIn && m_IntroSequenceState != IntroSequence::MainMenuAppear)) {
			m_SectionSwitch = true;
			m_IntroSequenceState = IntroSequence::MainMenuAppear;
			m_GameLogo.SetPos(Vector(static_cast<float>(g_FrameMan.GetResX() / 2), 120));
			m_StationOrbitTimer.SetElapsedRealTimeS(19);
			return;
		}
		m_SlideshowSlideText.clear();

		switch (m_IntroSequenceState) {
			case IntroSequence::SlideshowFadeIn:
				if (m_SectionSwitch) {
					SetSectionDurationAndResetSwitch(1.5F);
					m_IntroSongTimer.SetElapsedRealTimeS(0.05F);
					m_IntroScrollStartTime = static_cast<float>(m_IntroSongTimer.GetElapsedRealTimeS());
					m_IntroScrollDuration = 66.6F - m_IntroScrollStartTime;
					m_ScrollOffset.SetY(m_IntroScrollStartOffsetY);

					g_AudioMan.PlayMusic("Base.rte/Music/Hubnester/ccintro.ogg", 0);
					g_AudioMan.SetMusicPosition(0.05F);
				}
				m_FadeAmount = static_cast<int>(LERP(0, 1.0F, 255.0F, 0, m_SectionProgress));
				break;
			case IntroSequence::PreSlideshowPause:
				if (m_SectionSwitch) { SetSectionDurationAndResetSwitch(3.2F - static_cast<float>(m_IntroSongTimer.GetElapsedRealTimeS())); }
				break;
			case IntroSequence::ShowSlide1:
				if (m_SectionSwitch) {
					SetSectionDurationAndResetSwitch(11.4F - static_cast<float>(m_IntroSongTimer.GetElapsedRealTimeS()));
					m_SlideFadeInDuration = 2.0F;
					m_SlideFadeOutDuration = 0.5F;
				}
				if (m_SectionElapsedTime > 1.25F) { m_SlideshowSlideText = "At the end of humanity's darkest century..."; }
				break;
			case IntroSequence::ShowSlide2:
				if (m_SectionSwitch) {
					SetSectionDurationAndResetSwitch(17.3F - static_cast<float>(m_IntroSongTimer.GetElapsedRealTimeS()));
					m_SlideFadeInDuration = 0.5F;
					m_SlideFadeOutDuration = 2.5F;
				}
				if (m_SectionElapsedTime < m_SectionDuration - 1.75F) { m_SlideshowSlideText = "...a curious symbiosis between man and machine emerged."; }
				break;
			case IntroSequence::ShowSlide3:
				if (m_SectionSwitch) {
					SetSectionDurationAndResetSwitch(25.1F - static_cast<float>(m_IntroSongTimer.GetElapsedRealTimeS()));
					m_SlideFadeInDuration = 0.5F;
					m_SlideFadeOutDuration = 0.5F;
				}
				if (m_SectionProgress < 0.49F) {
					m_SlideshowSlideText = "This eventually enabled humans to leave their natural bodies...";
				} else if (m_SectionProgress > 0.51F) {
					m_SlideshowSlideText = "...and to free their minds from obsolete constraints.";
				}
				break;
			case IntroSequence::ShowSlide4:
				if (m_SectionSwitch) {
					SetSectionDurationAndResetSwitch(31.3F - static_cast<float>(m_IntroSongTimer.GetElapsedRealTimeS()));
					m_SlideFadeInDuration = 0.5F;
					m_SlideFadeOutDuration = 0.5F;
				}
				m_SlideshowSlideText = "With their brains sustained by artificial means, space travel also became feasible.";
				break;
			case IntroSequence::ShowSlide5:
				if (m_SectionSwitch) {
					SetSectionDurationAndResetSwitch(38.0F - static_cast<float>(m_IntroSongTimer.GetElapsedRealTimeS()));
					m_SlideFadeInDuration = 0.5F;
					m_SlideFadeOutDuration = 0.5F;
				}
				m_SlideshowSlideText = "Other civilizations were encountered...";
				break;
			case IntroSequence::ShowSlide6:
				if (m_SectionSwitch) {
					SetSectionDurationAndResetSwitch(44.1F - static_cast<float>(m_IntroSongTimer.GetElapsedRealTimeS()));
					m_SlideFadeInDuration = 0.5F;
					m_SlideFadeOutDuration = 0.5F;
				}
				m_SlideshowSlideText = "...and peaceful intragalactic trade soon established.";
				break;
			case IntroSequence::ShowSlide7:
				if (m_SectionSwitch) {
					SetSectionDurationAndResetSwitch(51.5F - static_cast<float>(m_IntroSongTimer.GetElapsedRealTimeS()));
					m_SlideFadeInDuration = 0.5F;
					m_SlideFadeOutDuration = 0.5F;
				}
				m_SlideshowSlideText = "Now, the growing civilizations create a huge demand for resources...";
				break;
			case IntroSequence::ShowSlide8:
				if (m_SectionSwitch) {
					SetSectionDurationAndResetSwitch(64.5F - static_cast<float>(m_IntroSongTimer.GetElapsedRealTimeS()));
					m_SlideFadeInDuration = 0.5F;
					m_SlideFadeOutDuration = 0.5F;
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
					SetSectionDurationAndResetSwitch(66.6F - static_cast<float>(m_IntroSongTimer.GetElapsedRealTimeS()));
					m_PreGameLogoText.SetPos(Vector(static_cast<float>(g_FrameMan.GetResX() / 2), static_cast<float>(g_FrameMan.GetResY() / 2)));
					m_PreGameLogoTextGlow.SetPos(m_PreGameLogoText.GetPos());
					m_GameLogo.SetPos(Vector(static_cast<float>(g_FrameMan.GetResX() / 2), static_cast<float>(g_FrameMan.GetResY() / 2) - 20));
				}
				break;
			default:
				break;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void TitleScreen::UpdateIntroPreMainMenuSequence() {
		switch (m_IntroSequenceState) {
			case IntroSequence::GameLogoAppear:
				if (m_SectionSwitch) {
					SetSectionDurationAndResetSwitch(68.2F - static_cast<float>(m_IntroSongTimer.GetElapsedRealTimeS()));
					m_IntroScrollStartTime = static_cast<float>(m_IntroSongTimer.GetElapsedRealTimeS());
					m_IntroScrollDuration = 92.4F - m_IntroScrollStartTime; // 92.4s is the end of the planet scrolling
					m_StationOrbitTimer.SetElapsedRealTimeS(40);
					clear_to_color(g_FrameMan.GetOverlayBitmap32(), 0xFFFFFFFF);
				}
				m_FadeAmount = static_cast<int>(LERP(0, 1.0F, 255.0F, 0, m_SectionProgress));
				break;
			case IntroSequence::PlanetScroll:
				if (m_SectionSwitch) {
					SetSectionDurationAndResetSwitch(92.4F - static_cast<float>(m_IntroSongTimer.GetElapsedRealTimeS()));
					clear_to_color(g_FrameMan.GetOverlayBitmap32(), 0);
				}
				if (m_SectionProgress > 0.5F) { m_GameLogo.SetPos(Vector(static_cast<float>(g_FrameMan.GetResX() / 2), EaseIn((static_cast<float>(g_FrameMan.GetResY() / 2)) - 20, 120, (m_SectionProgress - 0.5F) / 0.5F))); }
				break;
			case IntroSequence::PreMainMenu:
				if (m_SectionSwitch) {
					SetSectionDurationAndResetSwitch(2.0F);
					m_ScrollOffset.SetY(m_PreMainMenuScrollOffsetY);
				}
				break;
			case IntroSequence::MainMenuAppear:
				if (m_SectionSwitch) {
					SetSectionDurationAndResetSwitch(0.5F * g_SettingsMan.GetMenuTransitionDurationMultiplier());
					m_FadeAmount = 0;
					g_AudioMan.PlayMusic("Base.rte/Music/Hubnester/ccmenu.ogg", -1);
				}
				m_ScrollOffset.SetY(EaseOut(m_PreMainMenuScrollOffsetY, 0, m_SectionProgress));
				m_GameLogo.SetPos(Vector(static_cast<float>(g_FrameMan.GetResX() / 2), EaseOut(120, m_GameLogoMainMenuOffsetY, m_SectionProgress)));
				if (m_SectionElapsedTime >= m_SectionDuration) {
					SetTitleTransitionState(TitleTransition::MainMenu);
					m_FinishedPlayingIntro = true;
				}
				break;
			default:
				break;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void TitleScreen::UpdateTitleTransitions() {
		switch (m_TitleTransitionState) {
			case TitleTransition::MainMenu:
				if (m_SectionSwitch) {
					m_SectionSwitch = false;
					m_ScrollOffset.SetY(0);
					m_GameLogo.SetPos(Vector(static_cast<float>(g_FrameMan.GetResX() / 2), m_GameLogoMainMenuOffsetY));
				}
				break;
			case TitleTransition::ScenarioMenu:
			case TitleTransition::MetaGameMenu:
				if (m_SectionSwitch) {
					m_SectionSwitch = false;
					m_ScrollOffset.SetY(m_PlanetViewScrollOffsetY);
				}
				break;
			case TitleTransition::MainMenuToScenario:
			case TitleTransition::MainMenuToMetaGame:
				if (m_SectionSwitch) {
					SetSectionDurationAndResetSwitch(1.0F * g_SettingsMan.GetMenuTransitionDurationMultiplier());
					g_GUISound.SplashSound()->Play();
					g_AudioMan.PlayMusic("Base.rte/Music/dBSoundworks/thisworld5.ogg", -1);
				}
				m_ScrollOffset.SetY(EaseOut(0, m_PlanetViewScrollOffsetY, m_SectionProgress));
				m_GameLogo.SetPos(Vector(static_cast<float>(g_FrameMan.GetResX() / 2), EaseOut(m_GameLogoMainMenuOffsetY, m_GameLogoPlanetViewOffsetY, m_SectionProgress)));
				if (m_SectionElapsedTime >= m_SectionDuration) { SetTitleTransitionState((m_TitleTransitionState == TitleTransition::MainMenuToScenario) ? TitleTransition::ScenarioMenu : TitleTransition::MetaGameMenu); }
				break;
			case TitleTransition::PlanetToMainMenu:
				if (m_SectionSwitch) {
					SetSectionDurationAndResetSwitch(1.0F * g_SettingsMan.GetMenuTransitionDurationMultiplier());
					g_AudioMan.PlayMusic("Base.rte/Music/Hubnester/ccmenu.ogg", -1);
				}
				m_ScrollOffset.SetY(EaseOut(m_PlanetViewScrollOffsetY, 0, m_SectionProgress));
				m_GameLogo.SetPos(Vector(static_cast<float>(g_FrameMan.GetResX() / 2), EaseOut(m_GameLogoPlanetViewOffsetY, m_GameLogoMainMenuOffsetY, m_SectionProgress)));
				if (m_SectionElapsedTime >= m_SectionDuration) { SetTitleTransitionState(TitleTransition::MainMenu); }
				break;
			case TitleTransition::MainMenuToCredits:
				if (m_SectionSwitch) { SetSectionDurationAndResetSwitch(1.0F * g_SettingsMan.GetMenuTransitionDurationMultiplier()); }
				m_ScrollOffset.SetY(EaseOut(0, m_PlanetViewScrollOffsetY, m_SectionProgress));
				m_FadeAmount = static_cast<int>(EaseOut(0, 128.0F, m_SectionProgress));
				break;
			case TitleTransition::CreditsToMainMenu:
				if (m_SectionSwitch) { SetSectionDurationAndResetSwitch(1.0F * g_SettingsMan.GetMenuTransitionDurationMultiplier()); }
				m_ScrollOffset.SetY(EaseOut(m_PlanetViewScrollOffsetY, 0, m_SectionProgress));
				m_FadeAmount = static_cast<int>(EaseOut(128.0F, 0, m_SectionProgress));
				if (m_SectionElapsedTime >= m_SectionDuration) { SetTitleTransitionState(TitleTransition::MainMenu); }
				break;
			case TitleTransition::ScenarioFadeIn:
			case TitleTransition::MetaGameFadeIn:
				if (m_SectionSwitch) {
					SetSectionDurationAndResetSwitch(0.75F * g_SettingsMan.GetMenuTransitionDurationMultiplier());
					m_ScrollOffset.SetY(m_PlanetViewScrollOffsetY);
					m_GameLogo.SetPos(Vector(static_cast<float>(g_FrameMan.GetResX() / 2), m_GameLogoPlanetViewOffsetY));
					m_StationOrbitTimer.SetElapsedRealTimeS(m_StationOrbitTimerElapsedTime);
					g_AudioMan.PlayMusic("Base.rte/Music/dBSoundworks/thisworld5.ogg", -1);
				}
				g_AudioMan.SetTempMusicVolume(EaseOut(0, 1.0F, m_SectionProgress));
				m_FadeAmount = static_cast<int>(LERP(0, 1.0F, 255.0F, 0, m_SectionProgress));
				if (m_SectionElapsedTime >= m_SectionDuration) { SetTitleTransitionState((m_TitleTransitionState == TitleTransition::ScenarioFadeIn) ? TitleTransition::ScenarioMenu : TitleTransition::MetaGameMenu); }
				break;
			case TitleTransition::FadeOut:
				if (m_SectionSwitch) { SetSectionDurationAndResetSwitch(0.75F * g_SettingsMan.GetMenuTransitionDurationMultiplier()); }
				g_AudioMan.SetTempMusicVolume(EaseIn(1.0F, 0, m_SectionProgress));
				m_FadeAmount = static_cast<int>(EaseIn(0, 255, m_SectionProgress));
				if (m_SectionElapsedTime >= m_SectionDuration) { SetTitleTransitionState(TitleTransition::TransitionEnd); }
				break;
			case TitleTransition::ScrollingFadeIn:
				if (m_SectionSwitch) {
					SetSectionDurationAndResetSwitch(0.75F * g_SettingsMan.GetMenuTransitionDurationMultiplier());
					m_StationOrbitTimer.SetElapsedRealTimeS(m_StationOrbitTimerElapsedTime);
					g_AudioMan.PlayMusic("Base.rte/Music/Hubnester/ccmenu.ogg", -1);
				}
				g_AudioMan.SetTempMusicVolume(EaseOut(0, 1.0F, m_SectionProgress));
				m_ScrollOffset.SetY(EaseOut(250, 0, m_SectionProgress));
				m_GameLogo.SetPos(Vector(static_cast<float>(g_FrameMan.GetResX() / 2), EaseOut(m_GameLogoPlanetViewOffsetY, m_GameLogoMainMenuOffsetY, m_SectionProgress)));
				m_FadeAmount = static_cast<int>(EaseOut(255, 0, m_SectionProgress));
				if (m_SectionElapsedTime >= m_SectionDuration) { SetTitleTransitionState(TitleTransition::MainMenu); }
				break;
			case TitleTransition::ScrollingFadeOut:
			case TitleTransition::ScrollingFadeOutQuit:
				if (m_SectionSwitch) { SetSectionDurationAndResetSwitch(0.75F * g_SettingsMan.GetMenuTransitionDurationMultiplier()); }
				g_AudioMan.SetTempMusicVolume(EaseIn(1.0F, 0, m_SectionProgress));
				m_ScrollOffset.SetY(EaseIn(0, 250, m_SectionProgress));
				m_GameLogo.SetPos(Vector(static_cast<float>(g_FrameMan.GetResX() / 2), EaseIn(m_GameLogoMainMenuOffsetY, m_GameLogoPlanetViewOffsetY, m_SectionProgress)));
				m_FadeAmount = static_cast<int>(EaseIn(0, 255, m_SectionProgress));
				if (m_SectionElapsedTime >= m_SectionDuration) { SetTitleTransitionState((m_TitleTransitionState == TitleTransition::ScrollingFadeOutQuit) ? TitleTransition::TransitionEndQuit : TitleTransition::TransitionEnd); }
				break;
			case TitleTransition::TransitionEnd:
				if (m_SectionSwitch) {
					m_SectionSwitch = false;
					m_TitleTransitionState = TitleTransition::TransitionEnd;
				}
				break;
			default:
				break;
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void TitleScreen::Draw() {
		if (!m_FinishedPlayingIntro) {
			if (m_IntroSequenceState >= IntroSequence::SlideshowFadeIn) { DrawTitleScreenScene(); }
			if (m_IntroSequenceState >= IntroSequence::GameLogoAppear) { DrawGameLogo(); }

			if (m_IntroSequenceState >= IntroSequence::DataRealmsLogoFadeIn && m_IntroSequenceState <= IntroSequence::DataRealmsLogoFadeOut) {
				draw_sprite(g_FrameMan.GetBackBuffer32(), m_DataRealmsLogo, (g_FrameMan.GetResX() - m_DataRealmsLogo->w) / 2, (g_FrameMan.GetResY() - m_DataRealmsLogo->h) / 2);
				std::string copyrightNotice(64, '\0');
				std::snprintf(copyrightNotice.data(), copyrightNotice.size(), "Cortex Command is TM and %c 2017 Data Realms, LLC", -35);
				AllegroBitmap guiBackBuffer(g_FrameMan.GetBackBuffer32());
				m_IntroTextFont->DrawAligned(&guiBackBuffer, g_FrameMan.GetResX() / 2, g_FrameMan.GetResY() - m_IntroTextFont->GetFontHeight() - 5, copyrightNotice, GUIFont::Centre);
			} else if (m_IntroSequenceState >= IntroSequence::FmodLogoFadeIn && m_IntroSequenceState <= IntroSequence::FmodLogoFadeOut) {
				draw_sprite(g_FrameMan.GetBackBuffer32(), m_FmodLogo, (g_FrameMan.GetResX() - m_FmodLogo->w) / 2, (g_FrameMan.GetResY() - m_FmodLogo->h) / 2);
				AllegroBitmap guiBackBuffer(g_FrameMan.GetBackBuffer32());
				m_IntroTextFont->DrawAligned(&guiBackBuffer, g_FrameMan.GetResX() / 2, g_FrameMan.GetResY() - m_IntroTextFont->GetFontHeight() - 5, "Made with FMOD Studio by Firelight Technologies Pty Ltd.", GUIFont::Centre);
			} else if (m_IntroSequenceState >= IntroSequence::ShowSlide1 && m_IntroSequenceState <= IntroSequence::ShowSlide8) {
				DrawSlideshowSlide();
			} else if (m_IntroSequenceState == IntroSequence::SlideshowEnd) {
				m_PreGameLogoText.Draw(g_FrameMan.GetBackBuffer32(), Vector(), DrawMode::g_DrawAlpha);
				int blendAmount = 220 + RandomNum(-35, 35);
				set_screen_blender(blendAmount, blendAmount, blendAmount, blendAmount);
				m_PreGameLogoTextGlow.Draw(g_FrameMan.GetBackBuffer32(), Vector(), DrawMode::g_DrawTrans);
			}
		} else {
			DrawTitleScreenScene();

			// In credits have to draw the overlay before the game logo otherwise drawing the game logo again on top of an existing one causes the glow effect to look wonky.
			if (m_TitleTransitionState == TitleTransition::MainMenuToCredits || m_TitleTransitionState == TitleTransition::CreditsToMainMenu) {
				if (m_FadeAmount > 0) { DrawOverlayEffectBitmap(); }
				DrawGameLogo();
				return;
			}
			DrawGameLogo();
		}
		if (m_FadeAmount > 0) { DrawOverlayEffectBitmap(); }
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void TitleScreen::DrawTitleScreenScene() {
		Vector nebulaPos(0, m_ScrollOffset.GetY() * m_BackdropScrollRatio);
		if (m_Nebula.GetBitmap()->w != g_FrameMan.GetResX()) { nebulaPos.SetX(static_cast<float>((m_Nebula.GetBitmap()->w - g_FrameMan.GetResX()) / 2)); }
		Box nebulaTargetBox;
		m_Nebula.Draw(g_FrameMan.GetBackBuffer32(), nebulaTargetBox, nebulaPos);

		for (const Star &star : m_BackdropStars) {
			int intensity = star.Intensity + RandomNum(0, (star.Size == Star::StarSize::StarSmall) ? 35 : 70);
			set_screen_blender(intensity, intensity, intensity, intensity);
			int starPosY = static_cast<int>(star.Position.GetY() - (m_ScrollOffset.GetY() * (m_BackdropScrollRatio * ((star.Size == Star::StarSize::StarSmall) ? 0.8F : 1.0F))));
			draw_trans_sprite(g_FrameMan.GetBackBuffer32(), star.Bitmap, star.Position.GetFloorIntX(), starPosY);
		}

		m_PlanetPos.SetXY(static_cast<float>(g_FrameMan.GetResX() / 2), static_cast<float>(567 - m_ScrollOffset.GetFloorIntY()));
		m_Moon.SetPos(Vector(m_PlanetPos.GetX() + 200, 364 - (m_ScrollOffset.GetY() * 0.60F)));
		m_Moon.Draw(g_FrameMan.GetBackBuffer32(), Vector(), DrawMode::g_DrawAlpha);
		m_Planet.SetPos(m_PlanetPos);
		m_Planet.Draw(g_FrameMan.GetBackBuffer32(), Vector(), DrawMode::g_DrawAlpha);

		m_StationOffset.SetXY(m_OrbitRadius, 0);
		m_StationOffset.RadRotate(m_StationOrbitRotation);
		m_Station.SetPos(m_PlanetPos + m_StationOffset);
		m_Station.SetRotAngle(-c_HalfPI + m_StationOrbitRotation);
		m_Station.Draw(g_FrameMan.GetBackBuffer32());
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void TitleScreen::DrawGameLogo() {
		m_GameLogo.Draw(g_FrameMan.GetBackBuffer32(), Vector(), DrawMode::g_DrawAlpha);
		m_GameLogoGlow.SetPos(m_GameLogo.GetPos());
		int glowIntensity = 220 + RandomNum(-35, 35);
		set_screen_blender(glowIntensity, glowIntensity, glowIntensity, glowIntensity);
		m_GameLogoGlow.Draw(g_FrameMan.GetBackBuffer32(), Vector(), DrawMode::g_DrawTrans);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void TitleScreen::DrawSlideshowSlide() {
		int slide = static_cast<int>(m_IntroSequenceState) - static_cast<int>(IntroSequence::ShowSlide1);
		Vector slidePos(static_cast<float>((g_FrameMan.GetResX() / 2) - (m_IntroSlides.at(slide)->w / 2)), static_cast<float>((g_FrameMan.GetResY() / 2) - (m_IntroSlides.at(slide)->h / 2)));

		if (m_IntroSlides.at(slide)->w > g_FrameMan.GetResX()) {
			if (m_SectionElapsedTime < m_SlideFadeInDuration) {
				slidePos.SetX(0);
			} else if (m_SectionElapsedTime < m_SectionDuration - m_SlideFadeOutDuration) {
				slidePos.SetX(EaseInOut(0, static_cast<float>(g_FrameMan.GetResX() - m_IntroSlides.at(slide)->w), (m_SectionElapsedTime - m_SlideFadeInDuration) / (m_SectionDuration - m_SlideFadeInDuration - m_SlideFadeOutDuration)));
			} else {
				slidePos.SetX(static_cast<float>(g_FrameMan.GetResX() - m_IntroSlides.at(slide)->w));
			}
		}
		int fadeAmount = static_cast<int>((m_SectionElapsedTime < m_SlideFadeInDuration) ? EaseOut(0, 255.0F, m_SectionElapsedTime / m_SlideFadeInDuration) : EaseIn(255.0F, 0, (m_SectionElapsedTime - m_SectionDuration + m_SlideFadeOutDuration) / m_SlideFadeOutDuration));
		set_trans_blender(fadeAmount, fadeAmount, fadeAmount, fadeAmount);
		draw_trans_sprite(g_FrameMan.GetBackBuffer32(), m_IntroSlides.at(slide), slidePos.GetFloorIntX(), slidePos.GetFloorIntY());

		if (!m_SlideshowSlideText.empty()) {
			AllegroBitmap guiBackBuffer(g_FrameMan.GetBackBuffer32());
			m_IntroTextFont->DrawAligned(&guiBackBuffer, g_FrameMan.GetResX() / 2, (g_FrameMan.GetResY() / 2) + (m_IntroSlides.at(slide)->h / 2) + 12, m_SlideshowSlideText, GUIFont::Centre);
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void TitleScreen::DrawOverlayEffectBitmap() const {
		set_trans_blender(m_FadeAmount, m_FadeAmount, m_FadeAmount, m_FadeAmount);
		draw_trans_sprite(g_FrameMan.GetBackBuffer32(), g_FrameMan.GetOverlayBitmap32(), 0, 0);
	}
}