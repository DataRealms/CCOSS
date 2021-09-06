#ifndef _RTETITLESCREEN_
#define _RTETITLESCREEN_

#include "Timer.h"
#include "Vector.h"
#include "MOSRotating.h"
#include "MOSParticle.h"
#include "SceneLayer.h"

namespace RTE {

	class AllegroScreen;
	class GUIFont;

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
			MainMenuToMetaGame,
			MainMenuToCredits,
			CreditsToMainMenu,
			PlanetToMainMenu,
			ScenarioMenu,
			ScenarioFadeIn,
			MetaGameMenu,
			MetaGameFadeIn,
			ScrollingFadeIn,
			ScrollingFadeOut,
			ScrollingFadeOutQuit,
			FadeOut,
			TransitionEnd,
			TransitionEndQuit,
			TransitionPending,
		};

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a TitleScreen object in system memory and make it ready for use.
		/// </summary>
		/// <param name="guiScreen">Pointer to a GUIScreen interface that will be used to create this TitleScreen's GUIFont. Ownership is NOT transferred!</param>
		explicit TitleScreen(AllegroScreen *guiScreen) { Clear(); Create(guiScreen); }

		/// <summary>
		/// Makes the TitleScreen object ready for use.
		/// </summary>
		/// <param name="guiScreen">Pointer to a GUIScreen interface that will be used to create this TitleScreen's GUIFont. Ownership is NOT transferred!</param>
		void Create(AllegroScreen *guiScreen);
#pragma endregion

#pragma region Getters and Setters
		/// <summary>
		/// Gets the current title transition state.
		/// </summary>
		/// <returns>The current title transition state. See TitleTransition enumeration for values.</returns>
		TitleTransition GetTitleTransitionState() const { return m_TitleTransitionState; }

		/// <summary>
		/// Sets the target title transition state and, if different from the current, sets the section switch to trigger the transition.
		/// </summary>
		/// <param name="newTransitionState">The target title transition state.</param>
		void SetTitleTransitionState(TitleTransition newTransitionState) { if (newTransitionState != m_TitleTransitionState) { m_TitleTransitionState = newTransitionState; m_SectionSwitch = true; } }

		/// <summary>
		/// Sets the title transition to a pending state, stores the orbit timer elapsed time and resets the fade screen blend value.
		/// This is used to correctly restart transition states after breaking out of the game loop back to the menu loop.
		/// </summary>
		void SetTitlePendingTransition() { m_TitleTransitionState = TitleTransition::TransitionPending; m_StationOrbitTimerElapsedTime = static_cast<float>(m_StationOrbitTimer.GetElapsedRealTimeS()); m_FadeAmount = 0; }

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
		/// Gets the position of the station on the planet orbit.
		/// </summary>
		/// <returns>Vector with the position of the station on the planet orbit.</returns>
		Vector GetStationPos() const { return m_StationOffset; }
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
		/// Struct that holds information for a title screen scene backdrop star.
		/// </summary>
		struct Star {
			/// <summary>
			/// Enumeration for the different Star sizes.
			/// </summary>
			enum class StarSize { StarSmall, StarLarge, StarHuge };

			StarSize Size; //!< The size of the Star. Used for the appropriate Bitmap selection and Intensity randomization when drawing.
			BITMAP *Bitmap; //!< The bitmap to draw, not owned by this. Not Owned.
			int Intensity; //!< Intensity value on a scale from 0 to 255.
			Vector Position; //!< The position of the Star on the title screen scene backdrop.
		};

		int m_FadeAmount; //!< The amount of blending to use when drawing the screen overlay bitmap.

		std::vector<Star> m_BackdropStars; //!< Vector that contains all the individual Stars that are present in the title screen scene background.
		SceneLayer m_Nebula; //!< The title screen scene nebula background layer.
		MOSParticle m_Moon; //!< The title screen scene moon.
		MOSParticle m_Planet; //!< The title screen scene planet.
		Vector m_PlanetPos; //!< The position of the planet on the title screen scene.
		float m_PlanetRadius; //!< The radius of the planet.
		MOSRotating m_Station; //!< The title screen scene station.
		Vector m_StationOffset; //!< The position of the station on the planet orbit.
		float m_StationOrbitRotation; //!< The rotation of the station on the planet orbit.
		Timer m_StationOrbitTimer; //!< Timer to keep track of the station orbit elapsed time.
		float m_StationOrbitTimerElapsedTime; //!< The elapsed time of the station orbit timer at the point of breaking out of the menu loop. Used to restore the station to last position when returning to menu.
		float m_StationOrbitProgress; //!< Progress the station made on the orbit, from 0.0 to 1.0;
		float m_OrbitRadius; //!< The radius of the planet orbit.
		MOSParticle m_GameLogo; //!< The title screen scene Cortex Command logo.
		MOSParticle m_GameLogoGlow; //!< The title screen scene Cortex Command logo glow.
		float m_GameLogoMainMenuOffsetY; //!< The game logo position in the main menu.
		float m_GameLogoPlanetViewOffsetY; //!< The game logo position in planet overview menus.

		TitleTransition m_TitleTransitionState; //!< The current title transition (scroll) state.

		bool m_SectionSwitch; //!< Whether a new section or a transition has started and new properties need to be applied.
		Timer m_SectionTimer; //!< Timer to keep track of the section elapsed time.
		float m_SectionElapsedTime; //!< How many seconds have elapsed on a section.
		float m_SectionDuration; //!< How many seconds a section is supposed to elapse.
		float m_SectionProgress; //!< Progress made on a section, from 0.0 to 1.0.

		Vector m_ScrollOffset; //!< The scrolling position of the whole title screen scene.
		float m_BackdropScrollRatio; //!< The scrolling ratio of the whole title screen scene background (nebula and stars).
		float m_PlanetViewScrollOffsetY; //!< The title screen scene scroll position when in planet overview menus.

		bool m_FinishedPlayingIntro; //!< Whether the intro sequence finished playing.
		IntroSequence m_IntroSequenceState; //!< The current intro sequence state.
		Timer m_IntroSongTimer; //!< Timer used to sync the intro sequence progress with the intro song.
		float m_IntroScrollStartTime; //!< Time point during the intro sequence where the title screen scene scrolling starts.
		float m_IntroScrollDuration; //!< How many seconds the duration of the title screen scene scrolling should elapse during the intro sequence.
		float m_IntroScrollStartOffsetY; //!< The title screen scene starting scroll position during the intro sequence.
		float m_GameLogoAppearScrollOffsetY; //!< The title screen scene scroll position at which the game logo appears during the intro sequence.
		float m_PreMainMenuScrollOffsetY; //!< The title screen scene scroll position before scrolling to and activating the main menu.
		float m_SlideFadeInDuration; //!< How many seconds the duration of a slideshow slide fade in is supposed to elapse.
		float m_SlideFadeOutDuration; //!< How many seconds the duration of a slideshow slide fade out is supposed to elapse.
		std::unique_ptr<GUIFont> m_IntroTextFont; //!< The GUIFont used for drawing text during the logo splash screens and slideshow.
		std::string m_SlideshowSlideText; //!< String containing the slide text during each section of the slideshow.
		BITMAP *m_DataRealmsLogo; //!< The DataRealms logo bitmap used in the logo splash screen. Not Owned.
		BITMAP *m_FmodLogo; //!< The Fmod logo bitmap used in the logo splash screen. Not Owned.
		MOSParticle m_PreGameLogoText; //!< The pre-game logo text that appears at the end of the slideshow.
		MOSParticle m_PreGameLogoTextGlow; //!< The pre-game logo text glow.
		std::array<BITMAP *, 8> m_IntroSlides; //!< Array that contains all the slideshow slide bitmaps. Not Owned.

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
		/// <param name="newDuration">The duration of the new section, in seconds.</param>
		void SetSectionDurationAndResetSwitch(float newDuration) { m_SectionDuration = newDuration; m_SectionSwitch = false; }

		/// <summary>
		/// Updates the title screen transition states and scrolls the title screen scene accordingly.
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

		/// <summary>
		/// Draws the overlay bitmap for fade in/out or darkening effects to the screen.
		/// </summary>
		void DrawOverlayEffectBitmap() const;
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
		/// Clears all the member variables of this TitleScreen, effectively resetting the members of this object.
		/// </summary>
		void Clear();

		// Disallow the use of some implicit methods.
		TitleScreen(const TitleScreen &reference) = delete;
		TitleScreen & operator=(const TitleScreen &rhs) = delete;
	};
}
#endif