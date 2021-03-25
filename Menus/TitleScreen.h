#ifndef _RTETITLESCREEN_
#define _RTETITLESCREEN_

#include "Timer.h"
#include "Vector.h"

#include "GUI.h"
#include "GUIFont.h"
#include "AllegroBitmap.h"

#include "MOSRotating.h"
#include "MOSParticle.h"
#include "SceneLayer.h"

namespace RTE {

	/// <summary>
	/// 
	/// </summary>
	class TitleScreen {

	public:

		/// <summary>
		/// 
		/// </summary>
		enum TitleTransition {
			Intro = -1,
			MainMenu, // Main menu is active and operational
			MainMenuToPlanet, // Scenario mode views and transitions
			PlanetToMainMenu, // Going back to the main menu view from a planet-centered view
			ScenarioFadeIn, // Back from a scenario game to the scenario selection menu
			ScenarioMenu,
			MainMenuToCampaign, // Campaign mode views and transitions
			CampaignFadeIn, // Back from a battle to the campaign view
			CampaignPlay,
			FadeScrollOut,
			FadeOut,
			End
		};

		/// <summary>
		/// 
		/// </summary>
		enum ActiveMenu {
			MenusDisabled = -1,
			MainMenuActive,
			ScenarioMenuActive,
			CampaignMenuActive
		};

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a TitleScreen object in system memory.
		/// </summary>
		explicit TitleScreen(GUIFont *introTextFont) { Clear(); Create(introTextFont); }

		/// <summary>
		/// 
		/// </summary>
		void Create(GUIFont *introTextFont);
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Destructor method used to clean up a TitleScreen object before deletion from system memory.
		/// </summary>
		~TitleScreen() { Destroy(); }

		/// <summary>
		/// Destroys the TitleScreen object.
		/// </summary>
		void Destroy();
#pragma endregion

#pragma region Getters and Setters
		/// <summary>
		/// 
		/// </summary>
		/// <returns></returns>
		ActiveMenu GetActiveMenu() const { return m_ActiveMenu; }

		/// <summary>
		/// 
		/// </summary>
		/// <returns></returns>
		bool IsSectionSwitched() const { return m_SectionSwitch; }

		/// <summary>
		/// 
		/// </summary>
		/// <returns></returns>
		Vector GetPlanetPos() const { return m_PlanetPos; }

		/// <summary>
		/// 
		/// </summary>
		/// <returns></returns>
		float GetPlanetRadius() const { return m_PlanetRadius; }

		/// <summary>
		/// 
		/// </summary>
		/// <returns></returns>
		TitleTransition GetTitleTransitionState() const { return m_TitleTransitionState; }

		/// <summary>
		/// 
		/// </summary>
		/// <param name="newState"></param>
		void SetTitleTransitionState(TitleTransition newState) { if (newState != m_TitleTransitionState) { m_TitleTransitionState = newState; m_SectionSwitch = true; } }
#pragma endregion

#pragma region Concrete Methods
		/// <summary>
		/// 
		/// </summary>
		/// <param name="skipSection"></param>
		ActiveMenu Update(bool skipSection = false);

		/// <summary>
		/// 
		/// </summary>
		void Draw();
#pragma endregion

	private:

		/// <summary>
		/// 
		/// </summary>
		struct Star {
			/// <summary>
			/// Enumeration for the different Star sizes.
			/// </summary>
			enum StarSize { StarSmall, StarLarge, StarHuge };

			BITMAP *Bitmap = nullptr; //!<
			int PosX = 0; //!<
			int PosY = 0; //!<
			float ScrollRatio = 1.0F; //!<
			int Intensity = 0; //!< Intensity value on a scale from 0 to 255.
			StarSize Size = StarSize::StarSmall; //!<
		};

		/// <summary>
		/// 
		/// </summary>
		enum IntroSequence {
			DataRealmsLogoFadeIn,
			DataRealmsLogoDisplay,
			DataRealmsLogoFadeOut,
			FmodLogoFadeIn,
			FmodLogoDisplay,
			FmodLogoFadeOut,
			SlideshowFadeIn,
			PreSlideshowPause,
			ShowSlide1,
			ShowSlide2,
			ShowSlide3,
			ShowSlide4,
			ShowSlide5,
			ShowSlide6,
			ShowSlide7,
			ShowSlide8,
			SlideshowEnd,
			GameLogoAppear,
			PlanetScroll,
			PreMainMenu,
			MainMenuAppear
		};

		IntroSequence m_IntroSequenceState; //!<
		TitleTransition m_TitleTransitionState; //!<
		ActiveMenu m_ActiveMenu; //!<

		int m_ScreenResX; //!<
		int m_ScreenResY; //!<
		AllegroBitmap m_GUIBackBuffer; //!<

		BITMAP *m_FadeScreen; //!<
		int m_FadeAmount; //!<

		std::vector<Star> m_BackdropStars; //!<
		SceneLayer m_Nebula; //!<
		MOSParticle m_Moon; //!<
		MOSParticle m_Planet; //!<
		Vector m_PlanetPos; //!<
		float m_PlanetRadius; //!<
		MOSRotating m_Station; //!<
		Vector m_StationOffset; //!<
		float m_OrbitRadius; //!<
		float m_OrbitRotation; //!<

		BITMAP *m_DataRealmsLogo; //!<
		BITMAP *m_FmodLogo; //!<
		MOSParticle m_PreGameLogoText; //!<
		MOSParticle m_PreGameLogoTextGlow; //!<
		MOSParticle m_GameLogo; //!<
		MOSParticle m_GameLogoGlow; //!<

		bool m_SectionSwitch; //!<
		float m_SectionDuration; //!< How many seconds a section is supposed to elapse.
		float m_SectionProgress; //!< Progress made on a section, from 0.0 to 1.0.
		float m_SectionElapsedTime; //!< How many seconds have elapsed on a section.
		Timer m_SectionTimer; //!<

		float m_ScrollStart; //!<
		float m_ScrollDuration; //!<
		Vector m_ScrollOffset; //!<
		float m_BackdropScrollRatio; //!<
		float m_BackdropScrollStartOffsetY; //!< Set the start so that the nebula is fully scrolled up.
		float m_TitleAppearOffsetY; //!<
		float m_PreMainMenuOffsetY; //!<
		float m_PlanetViewOffsetY; //!<

		bool m_FinishedPlayingIntro; //!<
		Timer m_IntroSongTimer; //!<
		GUIFont *m_IntroTextFont; //!<
		std::string m_SlideshowSlideText; //!<
		std::array<BITMAP *, 8> m_IntroSlides; //!<
		float m_SlideFadeInDuration; //!<
		float m_SlideFadeOutDuration; //!<

#pragma region Create Breakdown
		/// <summary>
		/// 
		/// </summary>
		void CreateTitleElements();

		/// <summary>
		/// 
		/// </summary>
		void CreateIntroSequenceSlides();
#pragma endregion

#pragma region Title Scene Handling
		/// <summary>
		/// 
		/// </summary>
		void UpdateMenuTransitions();

		/// <summary>
		/// 
		/// </summary>
		void DrawGameLogo();

		/// <summary>
		/// 
		/// </summary>
		void DrawTitleScreenScene();
#pragma endregion

#pragma region Intro Sequence Handling
		/// <summary>
		/// 
		/// </summary>
		/// <returns></returns>
		void UpdateIntroLogoSequence(bool skipSection);

		/// <summary>
		/// 
		/// </summary>
		/// <returns></returns>
		void UpdateIntroSlideshowSequence(bool skipSection);

		/// <summary>
		/// 
		/// </summary>
		void UpdateIntro(bool skipSection);

		/// <summary>
		/// 
		/// </summary>
		void DrawSlideshowSlide();

		/// <summary>
		/// 
		/// </summary>
		void DrawIntro();
#pragma endregion

		/// <summary>
		/// Clears all the member variables of this TitleScreen, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();

		// Disallow the use of some implicit methods.
		TitleScreen(const TitleScreen &reference) = delete;
		TitleScreen & operator=(const TitleScreen &rhs) = delete;
	};
}
#endif