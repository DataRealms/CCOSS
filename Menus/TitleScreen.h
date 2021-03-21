#ifndef _RTETITLESCREEN_
#define _RTETITLESCREEN_

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
			Start,
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
			MenusDisabled = -1,
			MainMenu, // Main menu is active and operational
			MainMenuToScenario, // Scenario mode views and transitions
			ScenarioFadeIn, // Back from a scenario game to the scenario selection menu
			ScenarioMenu,
			MainMenuToCampaign, // Campaign mode views and transitions
			CampaignFadeIn, // Back from a battle to the campaign view
			CampaignPlay,
			PlanetToMain, // Going back to the main menu view from a planet-centered view
			FadeScrollOut,
			FadeOut,
			End
		};

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a TitleScreen object in system memory.
		/// </summary>
		TitleScreen() { Clear(); Create(); }

		/// <summary>
		/// 
		/// </summary>
		void Create();
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// 
		/// </summary>
		~TitleScreen();
#pragma endregion

#pragma region Concrete Methods

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

		IntroSequence m_IntroSequenceState; //!<
		TitleTransition m_TitleTransitionState; //!<
		bool m_FinishedPlayingIntro; //!<


		Timer m_SectionTimer; //!<
		float m_SectionElapsedTime; //!< How many seconds have elapsed on a section.
		float m_SectionDuration; //!< How many seconds a section is supposed to elapse.
		float m_SectionProgress; //!< Progress made on a section, from 0.0 to 1.0.
		bool m_SectionSwitch; //!<

		Timer m_IntroSongTimer; //!<

		std::array<BITMAP *, 8> m_IntroSlides; //!<
		Vector m_SlidePos; //!<
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
		int m_FadeAmount;

		BITMAP *m_DataRealmsLogo; //!<
		BITMAP *m_FmodLogo; //!<

		MOSParticle m_GameLogo; //!<
		MOSParticle m_GameLogoGlow; //!<

		MOSParticle m_Moon; //!<

		SceneLayer m_Nebula; //!<
		std::vector<Star> m_BackdropStars; //!<

		float m_BackdropScrollRatio; //!<

		float m_SectionSongEnd; //!< When a section is supposed to end, relative to the song timer.
		int m_BackdropScrollStartOffsetY; //!< Set the start so that the nebula is fully scrolled up.

		int m_TitleAppearOffsetY; //!<
		int m_MenuTopOffsetY; //!<

		float m_ScrollDuration; //!<
		float m_ScrollStart; //!<
		float m_ScrollProgress; //!<
		int m_PreMainMenuOffsetY; //!<


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
		void DrawSlideshowSlide();
#pragma endregion
#pragma region Create Breakdown
		/// <summary>
		/// 
		/// </summary>
		void CreateTitleElements();

		/// <summary>
		/// 
		/// </summary>
		void CreateBackdropStars();

		/// <summary>
		/// 
		/// </summary>
		void CreateIntroSequenceSlides();
#pragma endregion

#pragma region Drawing

		/// <summary>
		/// 
		/// </summary>

		/// <summary>
		/// 
		/// </summary>
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