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
		};
#pragma region Creation
		/// <summary>
		/// 
		/// </summary>
		TitleScreen();
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

		IntroSequence m_IntroSequenceState; //!<
		Timer m_SectionTimer; //!<
		float m_SectionElapsedTime; //!< How many seconds have elapsed on a section.
		float m_SectionDuration; //!< How many seconds a section is supposed to elapse.
		float m_SectionProgress; //!< Progress made on a section, from 0.0 to 1.0.
		bool m_SectionSwitch; //!<
		std::array<BITMAP *, 8> m_IntroSlides; //!<

		std::vector<Star> m_BackdropStars; //!<

		MOSParticle m_DataRealmsLogo; //!<
		MOSParticle m_FmodLogo; //!<
		SceneLayer m_Nebula; //!<
		MOSParticle m_GameLogo; //!<
		MOSParticle m_GameLogoGlow; //!<
		MOSParticle m_Planet; //!<
		MOSParticle m_Moon; //!<
		MOSParticle m_Station; //!<

		/// <summary>
		/// 
		/// </summary>
		/// <returns></returns>
		void PlayIntroLogoSequence(bool skipSection);

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
		void DrawDataRealmsLogo();

		/// <summary>
		/// 
		/// </summary>
		void DrawFmodLogo();
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