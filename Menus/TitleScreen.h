#ifndef _RTETITLESCREEN_
#define _RTETITLESCREEN_

#include "Timer.h"
#include "Vector.h"
#include "MOSRotating.h"
#include "MOSParticle.h"
#include "SceneLayer.h"

#include "GUI.h"
#include "GUIFont.h"

namespace RTE {

	/// <summary>
	/// Handling for the title screen scene, intro sequence and transitions between menu screens.
	/// </summary>
	class TitleScreen {

	public:

		/// <summary>
		/// Enumeration for the different menu screens that are active based on transition states.
		/// </summary>
		enum ActiveMenu {
			MenusDisabled = -1,
			MainMenuActive,
			ScenarioMenuActive,
			CampaignMenuActive
		};

		/// <summary>
		/// Enumeration for the different transition (scrolling) states of the title screen.
		/// </summary>
		enum class TitleTransition {
			PendingTransition = -1,
			MainMenu, // Main menu is active and operational
			MainMenuToScenario, // Scenario mode views and transitions
			MainMenuToCampaign, // Campaign mode views and transitions
			MainMenuToCredits,
			CreditsToMainMenu,
			PlanetToMainMenu, // Going back to the main menu view from a planet-centered view
			ScenarioMenu,
			ScenarioFadeIn, // Back from a scenario game to the scenario selection menu
			CampaignMenu,
			CampaignFadeIn, // Back from a battle to the campaign view
			ScrollFadeOut,
			FadeOut,
			End,
		};

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a TitleScreen object in system memory.
		/// </summary>
		/// <param name="introTextFont">Pointer to the GUIFont object that will handle text drawing to the screen.</param>
		explicit TitleScreen(GUIFont *introTextFont) { Clear(); Create(introTextFont); }

		/// <summary>
		/// Makes the TitleScreen object ready for use.
		/// </summary>
		/// <param name="introTextFont">Pointer to the GUIFont object that will handle text drawing to the screen.</param>
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
		TitleTransition GetTitleTransitionState() const { return m_TitleTransitionState; }

		/// <summary>
		/// 
		/// </summary>
		/// <param name="newState"></param>
		void SetTitleTransitionStateTarget(TitleTransition targetState) { if (targetState != m_TitleTransitionState) { m_TitleTransitionStateTarget = targetState; m_SectionSwitch = true; } }

		/// <summary>
		/// 
		/// </summary>
		/// <param name="newState"></param>
		void SetTitlePendingTransition();

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
		Vector GetStationPos() const { return m_Station.GetPos(); }
#pragma endregion

#pragma region Concrete Methods
		/// <summary>
		/// Updates the TitleScreen state each frame.
		/// </summary>
		ActiveMenu Update();

		/// <summary>
		/// Draws the TitleScreen to the screen.
		/// </summary>
		void Draw();
#pragma endregion

	private:

		/// <summary>
		/// 
		/// </summary>
		enum class IntroSequence {
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

		IntroSequence m_IntroSequenceState; //!<
		TitleTransition m_TitleTransitionState; //!<
		TitleTransition m_TitleTransitionStateTarget; //!<
		ActiveMenu m_ActiveMenu; //!<

		int m_ScreenResX; //!<
		int m_ScreenResY; //!<

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
		/// Creates all the elements that compose the title screen scene.
		/// </summary>
		void CreateTitleElements();

		/// <summary>
		/// Creates the intro sequence slideshow slides.
		/// </summary>
		void CreateIntroSequenceSlides();
#pragma endregion

#pragma region Title Scene Handling
		/// <summary>
		/// 
		/// </summary>
		void UpdateTitleTransitions();

		/// <summary>
		/// Draws the game logo and effects to the screen.
		/// </summary>
		void DrawGameLogo();

		/// <summary>
		/// Draws the whole title screen scene to the screen.
		/// </summary>
		void DrawTitleScreenScene();
#pragma endregion

#pragma region Intro Sequence Handling
		/// <summary>
		/// Updates the state of the intro sequence logo splash.
		/// </summary>
		/// <param name="skipSection">Whether the current section of the logo splash sequence should be skipped.</param>
		void UpdateIntroLogoSequence(bool skipSection = false);

		/// <summary>
		/// Updates the state of the intro sequence slideshow.
		/// </summary>
		/// <param name="skipSlideshow">Whether the whole slideshow should be skipped.</param>
		void UpdateIntroSlideshowSequence(bool skipSlideshow = false);

		/// <summary>
		/// Updates the state of the intro sequence.
		/// </summary>
		/// <param name="skipSection">Whether the current section of the intro sequence should be skipped.</param>
		void UpdateIntro(bool skipSection = false);

		/// <summary>
		/// Draws the current slideshow sequence slide to the screen.
		/// </summary>
		void DrawSlideshowSlide();

		/// <summary>
		/// Draws the current intro sequence state to the screen.
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