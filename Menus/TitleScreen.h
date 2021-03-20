#ifndef _RTETITLESCREEN_
#define _RTETITLESCREEN_

namespace RTE {

	/// <summary>
	/// 
	/// </summary>
	class TitleScreen {

	public:

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