#include "GUI.h"
#include "GUICheckbox.h"

using namespace RTE;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

GUICheckbox::GUICheckbox(GUIManager *Manager, GUIControlManager *ControlManager) : GUIControl(), GUIPanel(Manager) {
	m_ControlID = "CHECKBOX";
	m_Image = nullptr;
	m_ControlManager = ControlManager;
	m_Check = Unchecked;
	m_Mouseover = false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUICheckbox::Create(const std::string &Name, int X, int Y, int Width, int Height) {
	GUIControl::Create(Name, X, Y, Width, Height);

	// Minimum size of the control
	m_MinWidth = 40;
	m_MinHeight = 10;

	// Default size of the control
	m_DefWidth = 60;
	m_DefHeight = 16;

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
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUICheckbox::Create(GUIProperties *Props) {
	GUIControl::Create(Props);

	// Minimum size of the control
	m_MinWidth = 40;
	m_MinHeight = 10;

	// Default size of the control
	m_DefWidth = 60;
	m_DefHeight = 16;

	// Setup the panel
	GUIPanel::LoadProperties(Props);

	// Make sure the button isn't too small
	m_Width = std::max(m_Width, m_MinWidth);
	m_Height = std::max(m_Height, m_MinHeight);


	// Grab the check value
	m_Check = Unchecked;
	std::string value;
	Props->GetValue("Checked", &value);
	if (stricmp(value.c_str(), "Checked") == 0) {
		m_Check = Checked;
	} else if (stricmp(value.c_str(), "Greycheck") == 0) {
		m_Check = Greycheck;
	}
	Props->GetValue("Text", &m_Text);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUICheckbox::Destroy() {}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUICheckbox::ChangeSkin(GUISkin *Skin) {
	GUIControl::ChangeSkin(Skin);

	// Build the checkbox bitmap
	BuildBitmap();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUICheckbox::BuildBitmap() {
	std::string Filename;
	unsigned long ColorIndex = 0;
	int Values[4];

	// Load the image
	m_Skin->GetValue("Checkbox", "Filename", &Filename);
	m_Image = m_Skin->CreateBitmap(Filename);
	if (!m_Image) {
		return;
	}
	// Load the font
	m_Skin->GetValue("Checkbox", "Font", &Filename);
	m_Font = m_Skin->GetFont(Filename);
	m_Skin->GetValue("Checkbox", "FontColor", &m_FontColor);
	m_Skin->GetValue("Checkbox", "FontShadow", &m_FontShadow);
	m_Skin->GetValue("Checkbox", "FontKerning", &m_FontKerning);
	m_FontColor = m_Skin->ConvertColor(m_FontColor, m_Image->GetColorDepth());
	m_Font->CacheColor(m_FontColor);

	// Set the color key
	m_Skin->GetValue("Checkbox", "ColorKeyIndex", &ColorIndex);
	ColorIndex = m_Skin->ConvertColor(ColorIndex, m_Image->GetColorDepth());
	m_Image->SetColorKey(ColorIndex);

	// Load the source image rectangles

	// Base checkbox
	m_Skin->GetValue("Checkbox", "Base", Values, 4);
	SetRect(&m_ImageRects[0], Values[0], Values[1], Values[0] + Values[2], Values[1] + Values[3]);

	// Mouse over checkbox
	m_Skin->GetValue("Checkbox", "MouseOver", Values, 4);
	SetRect(&m_ImageRects[1], Values[0], Values[1], Values[0] + Values[2], Values[1] + Values[3]);

	// Normal check
	m_Skin->GetValue("Checkbox", "Check", Values, 4);
	SetRect(&m_ImageRects[2], Values[0], Values[1], Values[0] + Values[2], Values[1] + Values[3]);

	// Grayed check
	m_Skin->GetValue("Checkbox", "GreyCheck", Values, 4);
	SetRect(&m_ImageRects[3], Values[0], Values[1], Values[0] + Values[2], Values[1] + Values[3]);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUICheckbox::Draw(GUIScreen *Screen) {
	if (!m_Image) {
		return;
	}

	// Setup the clipping
	Screen->GetBitmap()->SetClipRect(GetRect());

	// Calculate the y position of the base
	// Make it centered vertically
	int YPos = m_Height / 2 - (m_ImageRects[0].bottom - m_ImageRects[0].top) / 2 + m_Y;

	// Draw the base
	if (m_Mouseover) {
		m_Image->Draw(Screen->GetBitmap(), m_X, YPos, &m_ImageRects[1]);
	} else {
		m_Image->Draw(Screen->GetBitmap(), m_X, YPos, &m_ImageRects[0]);
	}

	// Draw the check
	switch (m_Check) {
		case Checked:
			m_Image->DrawTrans(Screen->GetBitmap(), m_X, YPos, &m_ImageRects[2]);
			break;
		case Greycheck:
			m_Image->DrawTrans(Screen->GetBitmap(), m_X, YPos, &m_ImageRects[3]);
			break;
		default:
			break;
	}

	// Draw the text
	std::string Text;
	std::string space = " ";
	Text = space.append(m_Text);

	if (m_Font) {
		m_Font->SetColor(m_FontColor);
		m_Font->SetKerning(m_FontKerning);
		m_Font->Draw(Screen->GetBitmap(), m_X + (m_ImageRects[0].right - m_ImageRects[0].left) + 2, m_Y + (m_Height / 2) - (m_Font->GetFontHeight() / 2) - 1, Text, m_FontShadow);
	}
	Screen->GetBitmap()->SetClipRect(nullptr);

	GUIPanel::Draw(Screen);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUICheckbox::OnMouseDown(int X, int Y, int Buttons, int Modifier) {
	if (Buttons & MOUSE_LEFT) {
		// Push the checkbox down
		CaptureMouse();
		SetFocus();

		AddEvent(GUIEvent::Notification, Pushed, 0);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUICheckbox::OnMouseUp(int X, int Y, int Buttons, int Modifier) {
	ReleaseMouse();

	// If the mouse is over the button, add the command to the event queue
	if (PointInside(X, Y) && Buttons & MOUSE_LEFT) {
		if (m_Check == Unchecked) {
			m_Check = Checked;
		} else {
			m_Check = Unchecked;
		}
		AddEvent(GUIEvent::Notification, Changed, 0);
	}

	AddEvent(GUIEvent::Notification, UnPushed, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUICheckbox::OnMouseEnter(int X, int Y, int Buttons, int Modifier) {
	m_Mouseover = true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUICheckbox::OnMouseLeave(int X, int Y, int Buttons, int Modifier) {
	m_Mouseover = false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

GUIPanel * GUICheckbox::GetPanel() {
	return this;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUICheckbox::Move(int X, int Y) {
	SetPositionAbs(X, Y);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUICheckbox::Resize(int Width, int Height) {
	// Make sure the control isn't too small
	Width = std::max(Width, m_MinWidth);
	Height = std::max(Height, m_MinHeight);

	SetSize(Width, Height);

	BuildBitmap();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUICheckbox::GetControlRect(int *X, int *Y, int *Width, int *Height) {
	GUIPanel::GetRect(X, Y, Width, Height);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUICheckbox::StoreProperties() {
	if (m_Check == Unchecked) {
		m_Properties.AddVariable("Checked", "Unchecked");
	} else if (m_Check == Checked) {
		m_Properties.AddVariable("Checked", "Checked");
	} else if (m_Check == Greycheck) {
		m_Properties.AddVariable("Checked", "Greycheck");
	}
	m_Properties.AddVariable("Text", m_Text);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUICheckbox::SetText(const std::string &Text) {
	m_Text = Text;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string GUICheckbox::GetText() const {
	return m_Text;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUICheckbox::SetCheck(int Check) {
	m_Check = Check;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int GUICheckbox::GetCheck() const {
	return m_Check;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUICheckbox::ApplyProperties(GUIProperties *Props) {
	GUIControl::ApplyProperties(Props);

	m_Check = Unchecked;
	std::string value;
	m_Properties.GetValue("Checked", &value);
	if (stricmp(value.c_str(), "Checked") == 0) {
		m_Check = Checked;
	} else if (stricmp(value.c_str(), "Greycheck") == 0) {
		m_Check = Greycheck;
	}
	m_Properties.GetValue("Text", &m_Text);
}