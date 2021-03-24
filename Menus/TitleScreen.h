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

		/// <summary>
		/// 
		/// </summary>
		enum TitleTransition {
			Intro = -1,
			//MenusDisabled = -1,
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

			StarSize Size = StarSize::StarSmall; //!<
			BITMAP *Bitmap = nullptr; //!<
			int PosX = 0; //!<
			int PosY = 0; //!<
			float ScrollRatio = 1.0F; //!<
			int Intensity = 0; //!< Intensity value on a scale from 0 to 255.
		};

		AllegroBitmap m_GUIBackBuffer; //!<

		GUIFont *m_IntroTextFont; //!<
		std::string m_SlideshowSlideText; //!<

		IntroSequence m_IntroSequenceState; //!<
		bool m_FinishedPlayingIntro; //!<

		float m_PlanetViewOffsetY; //!<

		ActiveMenu m_ActiveMenu; //!<

		TitleTransition m_TitleTransitionState; //!<

		Timer m_SectionTimer; //!<
		float m_SectionElapsedTime; //!< How many seconds have elapsed on a section.
		float m_SectionDuration; //!< How many seconds a section is supposed to elapse.
		float m_SectionProgress; //!< Progress made on a section, from 0.0 to 1.0.
		bool m_SectionSwitch; //!<

		Timer m_IntroSongTimer; //!<

		std::array<BITMAP *, 8> m_IntroSlides; //!<
		float m_SlideFadeInDuration; //!<
		float m_SlideFadeOutDuration; //!<

		MOSParticle m_Planet; //!<
		Vector m_PlanetPos; //!<
		float m_PlanetRadius; //!<

		MOSRotating m_Station; //!<
		Vector m_StationOffset; //!<
		int m_StationOffsetX; //!<
		int m_StationOffsetY; //!<
		float m_OrbitRadius; //!<
		float m_OrbitRotation; //!<

		Vector m_ScrollOffset; //!<

		BITMAP *m_FadeScreen; //!<
		int m_FadeAmount; //!<

		BITMAP *m_DataRealmsLogo; //!<
		BITMAP *m_FmodLogo; //!<

		MOSParticle m_PreGameLogoText; //!<
		MOSParticle m_PreGameLogoTextGlow; //!<

		MOSParticle m_GameLogo; //!<
		MOSParticle m_GameLogoGlow; //!<

		MOSParticle m_Moon; //!<

		SceneLayer m_Nebula; //!<
		std::vector<Star> m_BackdropStars; //!<

		float m_BackdropScrollRatio; //!<

		float m_BackdropScrollStartOffsetY; //!< Set the start so that the nebula is fully scrolled up.

		float m_TitleAppearOffsetY; //!<

		float m_ScrollDuration; //!<
		float m_ScrollStart; //!<
		float m_ScrollProgress; //!<
		float m_PreMainMenuOffsetY; //!<

		int m_ScreenResX;
		int m_ScreenResY;


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