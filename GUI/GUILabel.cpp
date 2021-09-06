#include "GUI.h"
#include "GUILabel.h"

using namespace RTE;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

GUILabel::GUILabel(GUIManager *Manager, GUIControlManager *ControlManager) : GUIControl(), GUIPanel(Manager) {
	m_ControlID = "LABEL";
	m_ControlManager = ControlManager;
	m_Font = nullptr;
	m_FontColor = 0;
	m_Text = "";
	m_HAlignment = GUIFont::Left;
	m_VAlignment = GUIFont::Middle;
	m_HorizontalOverflowScroll = false;
	m_VerticalOverflowScroll = false;
	m_OverflowScrollState = OverflowScrollState::Deactivated;
	m_OverflowScrollTimer = Timer();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUILabel::Create(const std::string &Name, int X, int Y, int Width, int Height) {
	GUIControl::Create(Name, X, Y, Width, Height);

	// Minimum size of the control
	m_MinWidth = 20;
	m_MinHeight = 10;

	// Default size of the control
	m_DefWidth = 30;
	m_DefHeight = 16;

	// Setup the panel
	m_X = X;
	m_Y = Y;
	m_Width = m_DefWidth;
	m_Height = m_DefHeight;

	if (Width != -1) { m_Width = Width; }
	if (Height != -1) { m_Height = Height; }

	// Make sure the label isn't too small
	m_Width = std::max(m_Width, m_MinWidth);
	m_Height = std::max(m_Height, m_MinHeight);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUILabel::Create(GUIProperties *Props) {
	GUIControl::Create(Props);

	// Minimum size of the control
	m_MinWidth = 20;
	m_MinHeight = 10;

	// Default size of the control
	m_DefWidth = 30;
	m_DefHeight = 16;

	// Setup the panel
	GUIPanel::LoadProperties(Props);

	// Make sure the label isn't too small
	m_Width = std::max(m_Width, m_MinWidth);
	m_Height = std::max(m_Height, m_MinHeight);

	// Get the values
	Props->GetValue("Text", &m_Text);

	std::string alignString;
	Props->GetValue("HAlignment", &alignString);
	if (stricmp(alignString.c_str(), "left") == 0) { m_HAlignment = GUIFont::Left; }
	if (stricmp(alignString.c_str(), "centre") == 0 || stricmp(alignString.c_str(), "center") == 0) { m_HAlignment = GUIFont::Centre; }
	if (stricmp(alignString.c_str(), "right") == 0) { m_HAlignment = GUIFont::Right; }

	Props->GetValue("VAlignment", &alignString);
	if (stricmp(alignString.c_str(), "top") == 0) { m_VAlignment = GUIFont::Top; }
	if (stricmp(alignString.c_str(), "middle") == 0) { m_VAlignment = GUIFont::Middle; }
	if (stricmp(alignString.c_str(), "bottom") == 0) { m_VAlignment = GUIFont::Bottom; }

	Props->GetValue("HorizontalOverflowScroll", &m_HorizontalOverflowScroll);
	Props->GetValue("VerticalOverflowScroll", &m_VerticalOverflowScroll);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUILabel::ChangeSkin(GUISkin *Skin) {
	GUIControl::ChangeSkin(Skin);

	// Load the font
	std::string Filename;

	m_Skin->GetValue("Label", "Font", &Filename);
	m_Font = m_Skin->GetFont(Filename);
	m_Skin->GetValue("Label", "FontColor", &m_FontColor);
	m_Skin->GetValue("Label", "FontShadow", &m_FontShadow);
	m_Skin->GetValue("Label", "FontKerning", &m_FontKerning);
	m_FontColor = m_Skin->ConvertColor(m_FontColor);
	m_Font->CacheColor(m_FontColor);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUILabel::Draw(GUIScreen *Screen) {
	Draw(Screen->GetBitmap());
	GUIPanel::Draw(Screen);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUILabel::Draw(GUIBitmap *Bitmap, bool overwiteFontColorAndKerning) {
	// Setup the clipping
	Bitmap->AddClipRect(GetRect());

	if (m_Font) {
		if (overwiteFontColorAndKerning) {
			m_Font->SetColor(m_FontColor);
			m_Font->SetKerning(m_FontKerning);
		}

		// Adjust for horizontal alignment
		int xPos = m_X;
		if (m_HAlignment == GUIFont::Centre) {
			xPos += m_Width / 2;
		} else if (m_HAlignment == GUIFont::Right) {
			xPos += m_Width;
		}

		// Adjust for vertical alignment
		int yPos = m_Y;
		if (m_VAlignment == GUIFont::Middle) {
			yPos += (m_Height / 2) - 1;
		} else if (m_VAlignment == GUIFont::Bottom) {
			yPos += (m_Height)-1;
		}

		int textFullWidth = m_HorizontalOverflowScroll ? m_Font->CalculateWidth(m_Text) : 0;
		int textFullHeight = m_VerticalOverflowScroll ? m_Font->CalculateHeight(m_Text) : 0;
		bool modifyXPos = textFullWidth > m_Width;
		bool modifyYPos = textFullHeight > m_Height;
		xPos = modifyXPos ? m_X : xPos;
		yPos = modifyYPos ? m_Y : yPos;
		if (OverflowScrollIsActivated()) {
			switch (m_OverflowScrollState) {
				case OverflowScrollState::WaitAtStart:
					if (m_OverflowScrollTimer.GetRealTimeLimitMS() == -1) {
						m_OverflowScrollTimer.SetRealTimeLimitMS(1000);
						m_OverflowScrollTimer.Reset();
					} else if (m_OverflowScrollTimer.IsPastRealTimeLimit()) {
						m_OverflowScrollState = OverflowScrollState::Scrolling;
						m_OverflowScrollTimer.SetRealTimeLimitMS(-1);
						break;
					}
					break;
				case OverflowScrollState::Scrolling:
					if (m_OverflowScrollTimer.GetRealTimeLimitMS() == -1) {
						//TODO Maybe time limits should account for extra size vs width, so it scrolls slower on small labels, since it can be harder to read fast text on smaller areas. I think it's fine as-is though.
						// Note - time limits set so 5 characters of fatfont horizontal overflow or one line of fatfont vertical overflow will take 1 second.
						if (modifyXPos) {
							m_OverflowScrollTimer.SetRealTimeLimitMS((1000.0 / 30.0) * static_cast<double>(textFullWidth - m_Width));
						} else if (modifyYPos) {
							m_OverflowScrollTimer.SetRealTimeLimitMS((1000.0 / 8.0) * static_cast<double>(textFullHeight - m_Height));
						}
						m_OverflowScrollTimer.Reset();
					} else if (m_OverflowScrollTimer.IsPastRealTimeLimit()) {
						m_OverflowScrollTimer.SetRealTimeLimitMS(-1);
						m_OverflowScrollState = OverflowScrollState::WaitAtEnd;
						break;
					}
					xPos -= modifyXPos ? static_cast<int>(static_cast<double>(textFullWidth - m_Width) * m_OverflowScrollTimer.RealTimeLimitProgress()) : 0;
					yPos -= modifyYPos ? static_cast<int>(static_cast<double>(textFullHeight - m_Height) * m_OverflowScrollTimer.RealTimeLimitProgress()) : 0;
					break;
				case OverflowScrollState::WaitAtEnd:
					if (m_OverflowScrollTimer.GetRealTimeLimitMS() == -1) {
						m_OverflowScrollTimer.SetRealTimeLimitMS(1000);
						m_OverflowScrollTimer.Reset();
					} else if (m_OverflowScrollTimer.IsPastRealTimeLimit()) {
						m_OverflowScrollTimer.SetRealTimeLimitMS(-1);
						m_OverflowScrollState = OverflowScrollState::WaitAtStart;
						break;
					}
					xPos -= modifyXPos ? (textFullWidth - m_Width) : 0;
					yPos -= modifyYPos ? (textFullHeight - m_Height) : 0;
					break;
				default:
					GUIAbort("Invalid GUILabel overflow scroll state " + static_cast<int>(m_OverflowScrollState));
					break;
			}
		}
		m_Font->DrawAligned(Bitmap, xPos, yPos, m_Text, m_HorizontalOverflowScroll && textFullWidth > m_Width ? GUIFont::Left : m_HAlignment, m_VerticalOverflowScroll && textFullHeight > m_Height ? GUIFont::Top : m_VAlignment, m_HorizontalOverflowScroll ? textFullWidth : m_Width, m_FontShadow);
	}

	Bitmap->SetClipRect(nullptr);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUILabel::OnMouseDown(int X, int Y, int Buttons, int Modifier) {
	if (Buttons & MOUSE_LEFT) {
		CaptureMouse();
		SetFocus();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUILabel::OnMouseUp(int X, int Y, int Buttons, int Modifier) {
	// If the mouse is over the button, add the clicked notification to the event queue
	if (PointInside(X, Y) && (Buttons & MOUSE_LEFT) && IsCaptured()) { AddEvent(GUIEvent::Notification, Clicked, Buttons); }

	ReleaseMouse();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

GUIPanel * GUILabel::GetPanel() {
	return this;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUILabel::Move(int X, int Y) {
	GUIPanel::SetPositionAbs(X, Y);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUILabel::Resize(int Width, int Height) {
	// Make sure the control isn't too small
	Width = std::max(Width, m_MinWidth);
	Height = std::max(Height, m_MinHeight);

	GUIPanel::SetSize(Width, Height);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int GUILabel::ResizeHeightToFit() {
	int newHeight = m_Font->CalculateHeight(m_Text, m_Width);
	GUIPanel::SetSize(m_Width, newHeight);

	return newHeight;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUILabel::GetControlRect(int *X, int *Y, int *Width, int *Height) {
	GUIPanel::GetRect(X, Y, Width, Height);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int GUILabel::GetTextHeight() {
	return m_Font->CalculateHeight(m_Text, m_Width);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUILabel::SetHorizontalOverflowScroll(bool newOverflowScroll) {
	m_HorizontalOverflowScroll = newOverflowScroll;
	if (m_HorizontalOverflowScroll) {
		m_VerticalOverflowScroll = false;
	} else if (!m_VerticalOverflowScroll) {
		m_OverflowScrollState = OverflowScrollState::Deactivated;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUILabel::SetVerticalOverflowScroll(bool newOverflowScroll) {
	m_VerticalOverflowScroll = newOverflowScroll;
	if (m_VerticalOverflowScroll) {
		m_HorizontalOverflowScroll = false;
	} else if (!m_HorizontalOverflowScroll) {
		m_OverflowScrollState = OverflowScrollState::Deactivated;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUILabel::ActivateDeactivateOverflowScroll(bool activateScroll) {
	if (OverflowScrollIsEnabled() && activateScroll != OverflowScrollIsActivated()) {
		m_OverflowScrollState = activateScroll ? OverflowScrollState::WaitAtStart : OverflowScrollState::Deactivated;
		m_OverflowScrollTimer.SetRealTimeLimitMS(-1);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUILabel::StoreProperties() {
	m_Properties.AddVariable("Text", m_Text);

	if (m_HAlignment == GUIFont::Left) {
		m_Properties.AddVariable("HAlignment", "left");
	} else if (m_HAlignment == GUIFont::Centre) {
		m_Properties.AddVariable("HAlignment", "centre");
	} else if (m_HAlignment == GUIFont::Right) {
		m_Properties.AddVariable("HAlignment", "right");
	}
	if (m_VAlignment == GUIFont::Top) {
		m_Properties.AddVariable("VAlignment", "top");
	} else if (m_VAlignment == GUIFont::Middle) {
		m_Properties.AddVariable("VAlignment", "middle");
	} else if (m_VAlignment == GUIFont::Bottom) {
		m_Properties.AddVariable("VAlignment", "bottom");
	}
	m_Properties.AddVariable("HorizontalOverflowScroll", m_HorizontalOverflowScroll);
	m_Properties.AddVariable("VerticalOverflowScroll", m_VerticalOverflowScroll);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUILabel::ApplyProperties(GUIProperties *Props) {
	GUIControl::ApplyProperties(Props);

	m_Properties.GetValue("Text", &m_Text);

	std::string alignString;
	m_Properties.GetValue("HAlignment", &alignString);
	if (stricmp(alignString.c_str(), "left") == 0) { m_HAlignment = GUIFont::Left; }
	if (stricmp(alignString.c_str(), "centre") == 0) { m_HAlignment = GUIFont::Centre; }
	if (stricmp(alignString.c_str(), "right") == 0) { m_HAlignment = GUIFont::Right; }

	m_Properties.GetValue("VAlignment", &alignString);
	if (stricmp(alignString.c_str(), "top") == 0) { m_VAlignment = GUIFont::Top; }
	if (stricmp(alignString.c_str(), "middle") == 0) { m_VAlignment = GUIFont::Middle; }
	if (stricmp(alignString.c_str(), "bottom") == 0) { m_VAlignment = GUIFont::Bottom; }

	m_Properties.GetValue("HorizontalOverflowScroll", &m_HorizontalOverflowScroll);
	m_Properties.GetValue("VerticalOverflowScroll", &m_VerticalOverflowScroll);
}
