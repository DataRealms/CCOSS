#include "GUI.h"
#include "GUIButton.h"
#include "AllegroBitmap.h"
#include "GUILabel.h"

using namespace RTE;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

GUIButton::GUIButton(GUIManager *Manager, GUIControlManager *ControlManager) : GUIControl(), GUIPanel(Manager) {
	m_ControlID = "BUTTON";
	m_DrawBitmap = nullptr;
	m_ControlManager = ControlManager;
	m_Pushed = false;
	m_Over = false;
	m_Text = nullptr;
	m_Icon = nullptr;
	m_BorderSizes = nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIButton::Create(const std::string &Name, int X, int Y, int Width, int Height) {
	GUIControl::Create(Name, X, Y, Width, Height);

	// Minimum size of the control
	m_MinWidth = 10;
	m_MinHeight = 10;

	// Default size of the control
	m_DefWidth = 60;
	m_DefHeight = 40;

	// Setup the panel
	m_X = X;
	m_Y = Y;
	m_Width = m_DefWidth;
	m_Height = m_DefHeight;

	if (Width != -1) { m_Width = Width; }
	if (Height != -1) { m_Height = Height; }

	// Make sure the button isn't too small
	m_Width = std::max(m_Width, m_MinWidth);
	m_Height = std::max(m_Height, m_MinHeight);

	if (!m_Text) {
		m_Text = std::make_unique<GUILabel>(m_Manager, m_ControlManager);
		m_Text->SetHAlignment(GUIFont::Centre);
		m_Text->SetVAlignment(GUIFont::Top);
		m_Text->SetFont(m_Font);
		m_Text->SetVisible(false);
		m_Text->SetEnabled(false);
		GUIPanel::AddChild(m_Text.get());
	}
	if (!m_Icon) { m_Icon = std::make_unique<AllegroBitmap>(); }
	if (!m_BorderSizes) { m_BorderSizes = std::make_unique<GUIRect>(); }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIButton::Create(GUIProperties *Props) {
	GUIControl::Create(Props);

	// Minimum size of the control
	m_MinWidth = 10;
	m_MinHeight = 10;

	// Default size of the control
	m_DefWidth = 60;
	m_DefHeight = 40;

	// Setup the panel
	GUIPanel::LoadProperties(Props);

	// Make sure the button isn't too small
	m_Width = std::max(m_Width, m_MinWidth);
	m_Height = std::max(m_Height, m_MinHeight);

	if (!m_Text) {
		m_Text = std::make_unique<GUILabel>(m_Manager, m_ControlManager);
		m_Text->SetHAlignment(GUIFont::Centre);
		m_Text->SetVAlignment(GUIFont::Top);
		m_Text->SetFont(m_Font);
		m_Text->SetVisible(false);
		m_Text->SetEnabled(false);
		GUIPanel::AddChild(m_Text.get());
	}
	if (!m_Icon) { m_Icon = std::make_unique<AllegroBitmap>(); }
	if (!m_BorderSizes) { m_BorderSizes = std::make_unique<GUIRect>(); }

	// Load the values
	std::string text;
	Props->GetValue("Text", &text);
	m_Text->SetText(text);
	bool overflowScroll = false;
	Props->GetValue("HorizontalOverflowScroll", &overflowScroll);
	m_Text->SetHorizontalOverflowScroll(overflowScroll);
	overflowScroll = false;
	Props->GetValue("VerticalOverflowScroll", &overflowScroll);
	m_Text->SetVerticalOverflowScroll(overflowScroll);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIButton::Destroy() {
	// Free the drawing bitmap
	if (m_DrawBitmap) {
		m_DrawBitmap->Destroy();
		delete m_DrawBitmap;
		m_DrawBitmap = nullptr;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIButton::ChangeSkin(GUISkin *Skin) {
	GUIControl::ChangeSkin(Skin);

	// Build the button bitmap
	BuildBitmap();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIButton::BuildBitmap() {
	// Free any old bitmap
	if (m_DrawBitmap) {
		m_DrawBitmap->Destroy();
		delete m_DrawBitmap;
		m_DrawBitmap = 0;
	}

	// Create a new bitmap. Same width, but triple the height to allow for Up, Down
	// and Over states
	m_DrawBitmap = m_Skin->CreateBitmap(m_Width, m_Height * 3);

	// Pre-cache the font
	std::string Filename;
	m_Skin->GetValue("Button_Up", "Font", &Filename);
	m_Skin->GetValue("Button_Up", "FontColor", &m_FontColor);
	m_Skin->GetValue("Button_Up", "FontShadow", &m_FontShadow);
	m_Skin->GetValue("Button_Up", "FontKerning", &m_FontKerning);

	m_FontColor = m_Skin->ConvertColor(m_FontColor, m_DrawBitmap->GetColorDepth());

	//TODO consider getting rid of this line. Because of it GuiButton::SetFont does nothing, which is weird, but maybe there's a good reason for it. Test and investigate.
	// Also, with the change of m_Text from a std::string to a GUILabel this seems maybe even sillier. SetFont should probably set the label's font
	m_Font = m_Skin->GetFont(Filename);
	if (m_Font) { m_Font->CacheColor(m_FontColor); }

	// Create the button image
	GUIRect buttonBorders;
	m_Skin->BuildStandardRect(m_DrawBitmap, "Button_Up", 0, 0, m_Width, m_Height, true, true, &buttonBorders);
	SetRect(m_BorderSizes.get(), buttonBorders.left, buttonBorders.top, buttonBorders.right, buttonBorders.bottom);
	m_Skin->BuildStandardRect(m_DrawBitmap, "Button_Over", 0, m_Height, m_Width, m_Height);
	m_Skin->BuildStandardRect(m_DrawBitmap, "Button_Down", 0, m_Height * 2, m_Width, m_Height);

	//TODO this should be 1 pixel ideally, to give space between content and the border. However, the green skin, which this is primarly used for, has padding built-in and doesn't work properly without it.
	const int buttonContentPadding = 0;
	const int contentMaxWidth = m_Width - m_BorderSizes->left - m_BorderSizes->right - (buttonContentPadding * 2) - 1;
	const int contentMaxHeight = m_Height - m_BorderSizes->top - m_BorderSizes->bottom - (buttonContentPadding * 2) - 1;

	int centerY = m_Height / 2;
	bool hasIcon = m_Icon->GetBitmap();
	bool hasText = !m_Text->GetText().empty();

	int iconYPos = 0;
	float iconStretchRatio = 1;
	int textYPos = 0;

	if (hasIcon && !hasText) {
		iconYPos = centerY - (m_Icon->GetHeight() / 2);
	} else if (!hasIcon && hasText) {
		//int y = m_Height/2-m_Font->CalculateHeight(m_Text)+2; //Note this commented line was here from CCOSS, it was here so that the idea of using the full height of the text stays around.
		textYPos = centerY - (m_Font->GetFontHeight() / 2) - 1;
	} else if (hasIcon && hasText) {
		int iconAndTextHeight = m_Text->GetHorizontalOverflowScroll() ? m_Font->GetFontHeight() : m_Font->CalculateHeight(m_Text->GetText(), contentMaxWidth);

		if (m_Icon->GetWidth() > contentMaxWidth || (m_Icon->GetHeight() + iconAndTextHeight) > contentMaxHeight) {
			iconStretchRatio = std::min(static_cast<float>(contentMaxWidth) / static_cast<float>(m_Icon->GetWidth()), static_cast<float>(contentMaxHeight - iconAndTextHeight) / static_cast<float>(m_Icon->GetHeight()));
			iconAndTextHeight += static_cast<int>(static_cast<float>(m_Icon->GetHeight()) * iconStretchRatio);
		} else {
			iconAndTextHeight += m_Icon->GetHeight();
		}

		iconYPos = centerY - (iconAndTextHeight / 2);
		textYPos = iconYPos + static_cast<int>(static_cast<float>(m_Icon->GetHeight()) * iconStretchRatio);
	}

	if (hasIcon) {
		int iconXPos = (m_Width / 2) - static_cast<int>(static_cast<float>(m_Icon->GetWidth()) * iconStretchRatio / 2.0F);
		if (iconStretchRatio == 1) {
			m_Icon->DrawTrans(m_DrawBitmap, iconXPos, iconYPos, nullptr);
			m_Icon->DrawTrans(m_DrawBitmap, iconXPos, m_Height + iconYPos, nullptr);
			m_Icon->DrawTrans(m_DrawBitmap, iconXPos + 1, (m_Height * 2) + iconYPos + 1, nullptr);
		} else {
			int scaledWidth = static_cast<int>(static_cast<float>(m_Icon->GetWidth()) * iconStretchRatio);
			int scaledHeight = static_cast<int>(static_cast<float>(m_Icon->GetHeight()) * iconStretchRatio);
			m_Icon->DrawTransScaled(m_DrawBitmap, iconXPos, iconYPos, scaledWidth, scaledHeight);
			m_Icon->DrawTransScaled(m_DrawBitmap, iconXPos, m_Height + iconYPos, scaledWidth, scaledHeight);
			m_Icon->DrawTransScaled(m_DrawBitmap, iconXPos + 1, (m_Height * 2) + iconYPos + 1, scaledWidth, scaledHeight);
		}
	}

	if (hasText) {
		m_Font->SetColor(m_FontColor);
		m_Font->SetKerning(m_FontKerning);
		m_Text->SetFont(m_Font);
		m_Text->SetSize(contentMaxWidth, contentMaxHeight);

		int textXPos = m_BorderSizes->left + buttonContentPadding + 1;
		m_Text->SetPositionAbs(textXPos, textYPos);
		m_Text->Draw(m_DrawBitmap, false);
		m_Text->SetPositionAbs(textXPos, m_Height + textYPos);
		m_Text->Draw(m_DrawBitmap, false);
		m_Text->SetPositionAbs(textXPos + 1, (m_Height * 2) + textYPos + 1);
		m_Text->Draw(m_DrawBitmap, false);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIButton::Draw(GUIScreen *Screen) {
	GUIRect Rect;
	int y = 0;
	if (m_Pushed) {
		y = m_Height * 2;
	} else if (m_Over || m_GotFocus) {
		y = m_Height;
	}
	SetRect(&Rect, 0, y, m_Width, y + m_Height);

	if (m_Text->OverflowScrollIsActivated() && m_Font->CalculateWidth(m_Text->GetText()) > m_Width - m_BorderSizes->left - m_BorderSizes->right) { BuildBitmap(); }

	m_DrawBitmap->DrawTrans(Screen->GetBitmap(), m_X, m_Y, &Rect);

	GUIPanel::Draw(Screen);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIButton::OnMouseDown(int X, int Y, int Buttons, int Modifier) {
	if (Buttons & MOUSE_LEFT) {
		SetPushed(true);
		CaptureMouse();

		AddEvent(GUIEvent::Notification, Pushed, 0);
	}
	SetFocus();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIButton::OnMouseUp(int X, int Y, int Buttons, int Modifier) {
	if (PointInside(X, Y)) { AddEvent(GUIEvent::Notification, Clicked, Buttons); }

	if (!IsCaptured()) {
		return;
	}

	SetPushed(false);
	ReleaseMouse();

	// If the mouse is over the button, add the command to the event queue
	if (PointInside(X, Y)) { AddEvent(GUIEvent::Command, 0, 0); }

	AddEvent(GUIEvent::Notification, UnPushed, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIButton::OnMouseEnter(int X, int Y, int Buttons, int Modifier) {
	m_Over = true;
	m_Text->ActivateDeactivateOverflowScroll(true);

	AddEvent(GUIEvent::Notification, Focused, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIButton::OnMouseLeave(int X, int Y, int Buttons, int Modifier) {
	m_Over = false;
	if (!m_GotFocus && !m_Pushed) {
		m_Text->ActivateDeactivateOverflowScroll(false);
		BuildBitmap();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIButton::OnGainFocus() {
	GUIPanel::OnGainFocus();
	m_Text->ActivateDeactivateOverflowScroll(true);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIButton::OnLoseFocus() {
	GUIPanel::OnLoseFocus();
	if (!m_Over && !m_Pushed) {
		m_Text->ActivateDeactivateOverflowScroll(false);
		BuildBitmap();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIButton::OnMouseMove(int X, int Y, int Buttons, int Modifier) {
	if (!(Buttons & MOUSE_LEFT) || !IsCaptured()) {
		return;
	}

	// If the mouse goes outside of the button, un-push the button
	if (!PointInside(X, Y)) {
		if (m_Pushed) {
			AddEvent(GUIEvent::Notification, UnPushed, 0);
			SetPushed(false);
		}
	} else {
		if (!m_Pushed) {
			AddEvent(GUIEvent::Notification, Pushed, 0);
			SetPushed(true);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIButton::OnKeyDown(int KeyCode, int Modifier) {

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

GUIPanel * GUIButton::GetPanel() {
	return this;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIButton::GetControlRect(int *X, int *Y, int *Width, int *Height) {
	GUIPanel::GetRect(X, Y, Width, Height);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIButton::StoreProperties() {
	m_Properties.AddVariable("Text", m_Text->GetText());
	m_Properties.AddVariable("HorizontalOverflowScroll", m_Text->GetHorizontalOverflowScroll());
	m_Properties.AddVariable("VerticalOverflowScroll", m_Text->GetVerticalOverflowScroll());
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIButton::Move(int X, int Y) {
	GUIPanel::SetPositionAbs(X, Y);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIButton::Resize(int Width, int Height) {
	// Make sure the button isn't too small
	Width = std::max(Width, m_MinWidth);
	Height = std::max(Height, m_MinHeight);

	GUIPanel::SetSize(Width, Height);

	BuildBitmap();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIButton::SetPushed(bool pushed) {
	m_Pushed = pushed;
	if (pushed) {
		m_Text->ActivateDeactivateOverflowScroll(true);
	} else if (!pushed && !m_GotFocus && !m_Over) {
		m_Text->ActivateDeactivateOverflowScroll(false);
		BuildBitmap();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIButton::SetText(const std::string_view &newText, bool noBitmapRebuild) {
	if (GetText() != newText) {
		m_Text->SetText(newText);
		if (!noBitmapRebuild) { BuildBitmap(); }
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const std::string & GUIButton::GetText() const {
	return m_Text->GetText();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIButton::SetHorizontalOverflowScroll(bool newOverflowScroll) {
	m_Text->SetHorizontalOverflowScroll(newOverflowScroll);
	if (m_Text->GetHorizontalOverflowScroll()) { BuildBitmap(); }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIButton::SetVerticalOverflowScroll(bool newOverflowScroll) {
	m_Text->SetVerticalOverflowScroll(newOverflowScroll);
	if (m_Text->GetVerticalOverflowScroll()) { BuildBitmap(); }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIButton::SetIcon(BITMAP *newIcon, bool noBitmapRebuild) {
	if (m_Icon && m_Icon->GetBitmap() != newIcon) {
		m_Icon->SetBitmap(newIcon);
		if (!noBitmapRebuild) { BuildBitmap(); }
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIButton::SetIconAndText(BITMAP *newIcon, const std::string_view &newText) {
	bool bitmapNeedsRebuild = (m_Icon && m_Icon->GetBitmap() != newIcon) || (m_Text && m_Text->GetText() != newText);
	SetIcon(newIcon, true);
	SetText(newText, true);
	if (bitmapNeedsRebuild) { BuildBitmap(); }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIButton::ApplyProperties(GUIProperties *Props) {
	GUIControl::ApplyProperties(Props);

	std::string text;
	m_Properties.GetValue("Text", &text);
	m_Text->SetText(text);
	bool overflowScroll = false;
	m_Properties.GetValue("HorizontalOverflowScroll", &overflowScroll);
	m_Text->SetHorizontalOverflowScroll(overflowScroll);
	overflowScroll = false;
	m_Properties.GetValue("VerticalOverflowScroll", &overflowScroll);
	m_Text->SetVerticalOverflowScroll(overflowScroll);

	BuildBitmap();
}