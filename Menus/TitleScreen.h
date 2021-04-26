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
	/// Handling for the title screen scene composition, intro sequence and transitions between menu screens.
	/// </summary>
	class TitleScreen {

	public:

		/// <summary>
		/// Enumeration for the different transition (scrolling) states of the title screen.
		/// </summary>
		enum class TitleTransition {
			MainMenu,
			MainMenuToScenario,
			MainMenuToCampaign,
			MainMenuToCredits,
			CreditsToMainMenu,
			PlanetToMainMenu,
			ScenarioMenu,
			ScenarioFadeIn,
			CampaignMenu,
			CampaignFadeIn,
			ScrollFadeOut,
			FadeOut,
			TransitionEnd,
			TransitionPending,
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
		/// Gets the current title transition state.
		/// </summary>
		/// <returns>The current title transition state. See TitleTransition enumeration for values.</returns>
		TitleTransition GetTitleTransitionState() const { return m_TitleTransitionState; }

		/// <summary>
		/// Sets the target title transition state and if different from the current, sets the section switch to trigger the transition.
		/// </summary>
		/// <param name="newTransitionState">The target title transition state.</param>
		void SetTitleTransitionState(TitleTransition newTransitionState) { if (newTransitionState != m_TitleTransitionState) { m_TitleTransitionState = newTransitionState; m_SectionSwitch = true; } }

		/// <summary>
		/// Sets the title transition to a pending state and resets the fade screen blend value. This is used to correctly restart transition states after breaking out of the game loop back to the menu loop.
		/// </summary>
		void SetTitlePendingTransition() { m_TitleTransitionState = TitleTransition::TransitionPending; m_FadeAmount = 0; }

		/// <summary>
		/// Gets the position of the planet on the title screen scene.
		/// </summary>
		/// <returns>Vector with the position of the planet on the title screen scene.</returns>
		Vector GetPlanetPos() const { return m_PlanetPos; }

		/// <summary>
		/// Gets the radius of the planet.
		/// </summary>
		/// <returns>The radius of the planet.</returns>
		float GetPlanetRadius() const { return m_PlanetRadius; }

		/// <summary>
		/// Gets the position of the station on the title screen scene.
		/// </summary>
		/// <returns>Vector with the position of the station on the title screen scene.</returns>
		Vector GetStationPos() const { return m_Station.GetPos(); }
#pragma endregion

#pragma region Concrete Methods
		/// <summary>
		/// Updates the TitleScreen state.
		/// </summary>
		void Update();

		/// <summary>
		/// Draws the TitleScreen to the screen.
		/// </summary>
		void Draw();
#pragma endregion

	private:

		/// <summary>
		/// Enumeration for the different states of the intro sequence.
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
		/// Struct that holds information each title scene backdrop star.
		/// </summary>
		struct Star {
			/// <summary>
			/// Enumeration for the different Star sizes.
			/// </summary>
			enum class StarSize { StarSmall, StarLarge, StarHuge };

			StarSize Size; //!< The size of the Star. Used for the appropriate Bitmap selection and Intensity randomization when drawing.
			BITMAP *Bitmap; //!< The bitmap to draw, not owned by this.
			int Intensity; //!< Intensity value on a scale from 0 to 255.
			Vector Position; //!< The position of the Star on the title scene backdrop.
		};

		TitleTransition m_TitleTransitionState; //!< The current title transition (scroll) state.

		BITMAP *m_BackBuffer; //!<
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

		bool m_FinishedPlayingIntro; //!< Whether the intro sequence finished playing.
		IntroSequence m_IntroSequenceState; //!< The current intro sequence state.
		BITMAP *m_DataRealmsLogo; //!< The DataRealms logo bitmap used in the logo splash screen.
		BITMAP *m_FmodLogo; //!< The Fmod logo bitmap used in the logo splash screen.
		MOSParticle m_PreGameLogoText; //!<
		MOSParticle m_PreGameLogoTextGlow; //!<
		Timer m_IntroSongTimer; //!<
		float m_SlideFadeInDuration; //!<
		float m_SlideFadeOutDuration; //!<
		GUIFont *m_IntroTextFont; //!<
		std::string m_SlideshowSlideText; //!<
		std::array<BITMAP *, 8> m_IntroSlides; //!<

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
		/// Sets the duration of a new section and resets the switch.
		/// </summary>
		/// <param name="newDuration">The duration of the new section.</param>
		void SetSectionDurationAndResetSwitch(float newDuration) { m_SectionDuration = newDuration; m_SectionSwitch = false; }

		/// <summary>
		/// Updates the title screen transition states and scrolls the title scene accordingly.
		/// </summary>
		void UpdateTitleTransitions();

		/// <summary>
		/// Draws the whole title screen scene to the screen.
		/// </summary>
		void DrawTitleScreenScene();

		/// <summary>
		/// Draws the game logo and effects to the screen.
		/// </summary>
		void DrawGameLogo();
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
		/// Updates the state of the intro pre-main menu sequence.
		/// </summary>
		void UpdateIntroPreMainMenuSequence();

		/// <summary>
		/// Draws the current slideshow sequence slide to the screen.
		/// </summary>
		void DrawSlideshowSlide();
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