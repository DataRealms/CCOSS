#include "TitleScreen.h"

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void TitleScreen::CreateTitleElements() {
		m_DataRealmsLogo.Create(ContentFile("Base.rte/GUIs/Title/Intro/DRLogo5x.png"));
		m_FmodLogo.Create(ContentFile("Base.rte/GUIs/Title/Intro/FMODLogo.png"));
		m_GameLogo.Create(ContentFile("Base.rte/GUIs/Title/Title.png"));
		m_GameLogoGlow.Create(ContentFile("Base.rte/GUIs/Title/TitleGlow.png"));
		m_Planet.Create(ContentFile("Base.rte/GUIs/Title/Planet.png"));
		m_Moon.Create(ContentFile("Base.rte/GUIs/Title/Moon.png"));
		m_Station.Create(ContentFile("Base.rte/GUIs/Title/Station.png"));
		m_Nebula.Create(ContentFile("Base.rte/GUIs/Title/Nebula.png"), false, Vector(), false, false, Vector(0, -1.0F));

		set_write_alpha_blender();
		draw_trans_sprite(m_GameLogo.GetSpriteFrame(0), ContentFile("Base.rte/GUIs/Title/TitleAlpha.png").GetAsBitmap(COLORCONV_NONE, false), 0, 0);
		draw_trans_sprite(m_Planet.GetSpriteFrame(0), ContentFile("Base.rte/GUIs/Title/PlanetAlpha.png").GetAsBitmap(COLORCONV_NONE, false), 0, 0);
		draw_trans_sprite(m_Moon.GetSpriteFrame(0), ContentFile("Base.rte/GUIs/Title/MoonAlpha.png").GetAsBitmap(COLORCONV_NONE, false), 0, 0);
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
	}

}