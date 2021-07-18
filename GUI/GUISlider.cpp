#include "GUI.h"
#include "GUISlider.h"

using namespace RTE;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

GUISlider::GUISlider(GUIManager *Manager, GUIControlManager *ControlManager) : GUIControl(), GUIPanel(Manager) {
	m_ControlID = "SLIDER";
	m_DrawBitmap = nullptr;
	m_KnobImage = nullptr;
	m_ControlManager = ControlManager;
	m_Orientation = Horizontal;
	m_TickDirection = BottomRight;
	m_KnobPosition = 0;
	m_KnobSize = 0;
	m_KnobGrabbed = false;
	m_Minimum = 0;
	m_Value = 0;
	m_Maximum = 100;
	m_ValueResolution = 1;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUISlider::Create(const std::string &Name, int X, int Y, int Width, int Height) {
	GUIControl::Create(Name, X, Y, Width, Height);

	// Minimum size of the control
	m_MinWidth = 40;
	m_MinHeight = 10;

	// Default size of the control
	m_DefWidth = 100;
	m_DefHeight = 20;

	// Setup the panel
	m_X = X;
	m_Y = Y;
	m_Width = m_DefWidth;
	m_Height = m_DefHeight;

	if (Width != -1) { m_Width = Width; }
	if (Height != -1) { m_Height = Height; }

	// Re-Calculate the knob info
	CalculateKnob();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUISlider::Create(GUIProperties *Props) {
	GUIControl::Create(Props);

	// Minimum size of the control
	m_MinWidth = 40;
	m_MinHeight = 10;

	// Default size of the control
	m_DefWidth = 100;
	m_DefHeight = 20;

	// Setup the panel
	GUIPanel::LoadProperties(Props);

	// Make sure the button isn't too small
	m_Width = std::max(m_Width, m_MinWidth);
	m_Height = std::max(m_Height, m_MinHeight);


	// Get the values
	std::string ori;
	Props->GetValue("Orientation", &ori);
	if (stricmp(ori.c_str(), "horizontal") == 0) {
		m_Orientation = Horizontal;
	} else if (stricmp(ori.c_str(), "vertical") == 0) {
		m_Orientation = Vertical;
	}

	std::string tick;
	Props->GetValue("TickDirection", &tick);
	if (stricmp(tick.c_str(), "TopLeft") == 0) {
		m_TickDirection = TopLeft;
	} else if (stricmp(tick.c_str(), "BottomRight") == 0) {
		m_TickDirection = BottomRight;
	}
	Props->GetValue("Minimum", &m_Minimum);
	Props->GetValue("Maximum", &m_Maximum);
	Props->GetValue("Value", &m_Value);
	if (!Props->GetValue("ValueResolution", &m_ValueResolution)) { m_ValueResolution = std::max((m_Maximum - m_Minimum) / 100, 1); }

	m_Value = std::clamp(m_Value, m_Minimum, m_Maximum);

	// Re-Calculate the knob info
	CalculateKnob();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUISlider::Destroy() {
	// Destroy the draw bitmap
	if (m_DrawBitmap) {
		m_DrawBitmap->Destroy();
		delete m_DrawBitmap;
		m_DrawBitmap = nullptr;
	}

	// Destroy the knob image bitmap
	if (m_KnobImage) {
		m_KnobImage->Destroy();
		delete m_KnobImage;
		m_KnobImage = nullptr;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUISlider::ChangeSkin(GUISkin *Skin) {
	GUIControl::ChangeSkin(Skin);

	// Build the progressbar bitmap
	BuildBitmap();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUISlider::BuildBitmap() {
	// Free any old bitmaps
	if (m_DrawBitmap) {
		m_DrawBitmap->Destroy();
		delete m_DrawBitmap;
		m_DrawBitmap = nullptr;
	}
	if (m_KnobImage) {
		m_KnobImage->Destroy();
		delete m_KnobImage;
		m_KnobImage = nullptr;
	}

	std::string Section;
	if (m_Orientation == Horizontal) {
		Section = "Slider_Horz";
	} else {
		Section = "Slider_Vert";
	}

	// Load the source image
	std::string Filename;
	m_Skin->GetValue(Section, "Filename", &Filename);
	GUIBitmap *SrcImage = m_Skin->CreateBitmap(Filename);
	if (!SrcImage) {
		return;
	}

	// Build the line & ticks
	BuildLine(Section, SrcImage);

	// Load the indicator image
	std::string Side;
	if (m_TickDirection == TopLeft) {
		Side = "TopLeftSlider";
	} else {
		Side = "BottomRightSlider";
	}

	int Values[4];
	GUIRect Rect;
	m_Skin->GetValue(Section, Side, Values, 4);
	SetRect(&Rect, Values[0], Values[1], Values[0] + Values[2], Values[1] + Values[3]);

	// Create the indicator bitmap
	m_KnobImage = m_Skin->CreateBitmap(Values[2], Values[3]);
	SrcImage->Draw(m_KnobImage, 0, 0, &Rect);

	unsigned long ColorKey;
	m_Skin->GetValue(Section, "ColorKeyIndex", &ColorKey);
	ColorKey = m_Skin->ConvertColor(ColorKey, m_KnobImage->GetColorDepth());
	m_KnobImage->SetColorKey(ColorKey);
	m_DrawBitmap->SetColorKey(ColorKey);

	// Re-Calculate the knob info
	CalculateKnob();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUISlider::BuildLine(const std::string &Section, GUIBitmap *SrcImage) {
	int Values[4];
	GUIRect Rect;

	// Get the size of the middle bit for determining part of the bitmap size
	m_Skin->GetValue(Section, "Middle", Values, 4);
	if (m_Orientation == Horizontal) {
		m_DrawBitmap = m_Skin->CreateBitmap(m_Width, Values[3]);
	} else {
		m_DrawBitmap = m_Skin->CreateBitmap(Values[2], m_Height);
	}

	// Tile the line
	m_Skin->GetValue(Section, "Middle", Values, 4);
	SetRect(&Rect, Values[0], Values[1], Values[0] + Values[2], Values[1] + Values[3]);
	if (m_Orientation == Horizontal) {
		for (int i = 0; i < m_Width; i += Values[2]) {
			SrcImage->Draw(m_DrawBitmap, i, 0, &Rect);
		}
	} else {
		for (int i = 0; i < m_Height; i += Values[3]) {
			SrcImage->Draw(m_DrawBitmap, 0, i, &Rect);
		}
	}

	// Draw the start
	m_Skin->GetValue(Section, "Start", Values, 4);
	SetRect(&Rect, Values[0], Values[1], Values[0] + Values[2], Values[1] + Values[3]);
	SrcImage->Draw(m_DrawBitmap, 0, 0, &Rect);

	m_EndThickness = m_Orientation == Horizontal ? Values[2] : Values[3];

	// Draw the end
	int X = 0;
	int Y = 0;
	m_Skin->GetValue(Section, "End", Values, 4);
	SetRect(&Rect, Values[0], Values[1], Values[0] + Values[2], Values[1] + Values[3]);
	if (m_Orientation == Horizontal) {
		X = m_Width - Values[2];
	} else {
		Y = m_Height - Values[3];
	}
	SrcImage->Draw(m_DrawBitmap, X, Y, &Rect);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUISlider::Draw(GUIScreen *Screen) {
	int X = 0;
	int Y = 0;

	if (m_Orientation == Horizontal) {
		Y = m_Height / 2 - m_DrawBitmap->GetHeight() / 2;
	} else {
		X = m_Width / 2 - m_DrawBitmap->GetWidth() / 2;
	}

	// Draw the base
	Screen->DrawBitmap(m_DrawBitmap, m_X + X, m_Y + Y, nullptr);

	// Draw the indicator
	if (!m_KnobImage) {
		return;
	}

	int Half = 0;
	if (m_Orientation == Horizontal) {
		Half = m_DrawBitmap->GetHeight() / 2;
	} else {
		Half = m_DrawBitmap->GetWidth() / 2;
	}
	if (m_Orientation == Horizontal) {
		m_KnobImage->DrawTrans(Screen->GetBitmap(), m_X + m_KnobPosition, m_Y + Y + Half - m_KnobImage->GetHeight() / 2, nullptr);
	} else {
		m_KnobImage->DrawTrans(Screen->GetBitmap(), m_X + X + Half - m_KnobImage->GetWidth() / 2, m_Y + m_KnobPosition, nullptr);
	}
	GUIPanel::Draw(Screen);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUISlider::OnMouseDown(int X, int Y, int Buttons, int Modifier) {
	CaptureMouse();
	SetFocus();

	m_OldValue = m_Value;
	int Size = 0;

	// Only do stuff here on a left mouse click
	if (!(Buttons & MOUSE_LEFT)) {
		return;
	}

	if (m_Orientation == Horizontal) {
		if (X > m_X + m_EndThickness && X < m_X + m_Width - m_EndThickness) {
			m_KnobGrabbed = true;
			m_KnobPosition = X - m_X - (m_KnobSize / 2);
			m_KnobGrabPos = X - (m_X + m_KnobPosition);
		}
		Size = m_Width;
	} else {
		if (Y > m_Y + m_EndThickness && Y < m_Y + m_Height - m_EndThickness) {
			m_KnobGrabbed = true;
			m_KnobPosition = Y - m_Y - (m_KnobSize / 2);
			m_KnobGrabPos = Y - (m_Y + m_KnobPosition);
		}
		Size = m_Height;
	}

	m_KnobPosition = std::clamp(m_KnobPosition, m_EndThickness, Size - m_KnobSize - m_EndThickness);

	// Calculate the new value
	int Area = Size - m_KnobSize;
	if (Area > 0) {
		float p = (float)m_KnobPosition / (float)(Area);
		int MaxRange = (m_Maximum - m_Minimum);
		m_Value = (float)MaxRange * p + m_Minimum;
	}

	m_Value = std::clamp(m_Value, m_Minimum, m_Maximum);

	// If the value has changed, add the "Changed" notification
	if (m_Value != m_OldValue)
		AddEvent(GUIEvent::Notification, Changed, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUISlider::OnMouseUp(int X, int Y, int Buttons, int Modifier) {
	ReleaseMouse();

	m_KnobGrabbed = false;

	// If the value has changed, add the "Changed" notification
	if (m_Value != m_OldValue) { AddEvent(GUIEvent::Notification, Changed, 0); }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUISlider::OnMouseMove(int X, int Y, int Buttons, int Modifier) {
	int MousePos = X;
	int KnobTop = 0;
	int Size = 1;

	// Horizontal
	if (m_Orientation == Horizontal) {
		MousePos = X;
		KnobTop = m_X + m_KnobPosition;
		Size = m_Width;
	}

	// Vertical
	if (m_Orientation == Vertical) {
		MousePos = Y;
		KnobTop = m_Y + m_KnobPosition;
		Size = m_Height;
	}

	// Move the knob if it is grabbed
	if (m_KnobGrabbed) {
		int Delta = m_KnobGrabPos - (MousePos - KnobTop);
		m_KnobPosition -= Delta;

		// Clamp the knob position
		m_KnobPosition = std::max(m_KnobPosition, 0);
		m_KnobPosition = std::min(m_KnobPosition, Size - m_KnobSize);

		// Calculate the new value
		int Area = Size - m_KnobSize;
		if (Area > 0) {
			float p = (float)m_KnobPosition / (float)(Area);
			int MaxRange = (m_Maximum - m_Minimum);
			m_Value = (float)MaxRange * p + m_Minimum;
		}

		// Clamp the value
		m_Value = std::max(m_Value, m_Minimum);
		m_Value = std::min(m_Value, m_Maximum);

		// Clamp the knob position again for the graphics
		m_KnobPosition = std::max(m_KnobPosition, m_EndThickness);
		m_KnobPosition = std::min(m_KnobPosition, Size - m_KnobSize - m_EndThickness);

		// If the value has changed, add the "Changed" notification
		if (m_Value != m_OldValue) { AddEvent(GUIEvent::Notification, Changed, 0); }

		m_OldValue = m_Value;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUISlider::OnMouseWheelChange(int x, int y, int modifier, int mouseWheelChange) {
	m_OldValue = m_Value;

	if (mouseWheelChange < 0) {
		m_Value = std::max(m_Value - m_ValueResolution, m_Minimum);
	} else {
		m_Value = std::min(m_Value + m_ValueResolution, m_Maximum);
	}

	if (m_Value != m_OldValue) {
		CalculateKnob();
		AddEvent(GUIEvent::Notification, Changed, 0);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

GUIPanel * GUISlider::GetPanel() {
	return this;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUISlider::CalculateKnob() {
	if (!m_KnobImage) {
		return;
	}

	if (m_Maximum > m_Minimum) {
		const bool horizontalOrientation = (m_Orientation == Horizontal);
		m_KnobSize = (horizontalOrientation) ? m_KnobImage->GetWidth() : m_KnobImage->GetHeight();
		const int size = (horizontalOrientation) ? m_Width : m_Height;
		const float valueRatio = static_cast<float>(m_Value - m_Minimum) / static_cast<float>(m_Maximum - m_Minimum);
		m_KnobPosition = m_EndThickness + static_cast<int>(static_cast<float>(size - m_KnobSize - (m_EndThickness * 2)) * valueRatio);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUISlider::Move(int X, int Y) {
	GUIPanel::SetPositionAbs(X, Y);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUISlider::Resize(int Width, int Height) {
	// Make sure the control isn't too small
	Width = std::max(Width, m_MinWidth);
	Height = std::max(Height, m_MinHeight);

	GUIPanel::SetSize(Width, Height);

	// Rebuild the bitmap
	BuildBitmap();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUISlider::GetControlRect(int *X, int *Y, int *Width, int *Height) {
	GUIPanel::GetRect(X, Y, Width, Height);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUISlider::SetOrientation(int Orientation) {
	m_Orientation = Orientation;

	BuildBitmap();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int GUISlider::GetOrientation() const {
	return m_Orientation;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUISlider::SetTickDirection(int TickDir) {
	m_TickDirection = TickDir;

	BuildBitmap();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int GUISlider::GetTickDirection() const {
	return m_TickDirection;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUISlider::SetMinimum(int Minimum) {
	if (Minimum != m_Minimum) {
		m_Minimum = Minimum;
		m_Value = std::max(m_Value, m_Minimum);

		// Re-Calculate the knob info
		CalculateKnob();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int GUISlider::GetMinimum() const {
	return m_Minimum;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUISlider::SetMaximum(int Maximum) {
	if (Maximum != m_Maximum) {
		m_Maximum = Maximum;
		m_Value = std::min(m_Value, m_Maximum);

		// Re-Calculate the knob info
		CalculateKnob();
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int GUISlider::GetMaximum() const {
	return m_Maximum;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUISlider::SetValue(int Value) {
	int OldValue = m_Value;

	m_Value = std::clamp(Value, m_Minimum, m_Maximum);

	if (m_Value != OldValue) {
		CalculateKnob();
		AddEvent(GUIEvent::Notification, Changed, 0);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int GUISlider::GetValue() const {
	return m_Value;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUISlider::StoreProperties() {
	m_Properties.AddVariable("Value", m_Value);
	m_Properties.AddVariable("Minimum", m_Minimum);
	m_Properties.AddVariable("Maximum", m_Maximum);
	m_Properties.AddVariable("Orientation", m_Orientation == Horizontal ? "Horizontal" : "Vertical");
	m_Properties.AddVariable("TickDirection", m_TickDirection == TopLeft ? "TopLeft" : "BottomRight");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUISlider::ApplyProperties(GUIProperties *Props) {
	GUIControl::ApplyProperties(Props);

	// Get the values
	std::string ori;
	m_Properties.GetValue("Orientation", &ori);
	if (stricmp(ori.c_str(), "horizontal") == 0) {
		m_Orientation = Horizontal;
	} else if (stricmp(ori.c_str(), "vertical") == 0) {
		m_Orientation = Vertical;
	}

	std::string tick;
	m_Properties.GetValue("TickDirection", &tick);
	if (stricmp(tick.c_str(), "TopLeft") == 0) {
		m_TickDirection = TopLeft;
	} else if (stricmp(tick.c_str(), "BottomRight") == 0) {
		m_TickDirection = BottomRight;
	}
	m_Properties.GetValue("Minimum", &m_Minimum);
	m_Properties.GetValue("Maximum", &m_Maximum);
	m_Properties.GetValue("Value", &m_Value);

	// Clamp the value
	m_Value = std::max(m_Value, m_Minimum);
	m_Value = std::min(m_Value, m_Maximum);

	BuildBitmap();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUISlider::SetValueResolution(int valueRes) {
	if (valueRes >= 1 && valueRes <= m_Maximum - m_Minimum) { m_ValueResolution = valueRes; }
}