#include "GUI.h"
#include "GUIScrollPanel.h"

using namespace RTE;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

GUIScrollPanel::GUIScrollPanel(GUIManager *Manager) : GUIPanel(Manager) {
	m_Skin = nullptr;
	m_DrawBitmap[0] = m_DrawBitmap[1] = m_DrawBitmap[2] = nullptr;
	m_ButtonSize = 17;
	m_MinimumKnobSize = 9;
	m_SmallChange = 1;
	m_ButtonPushed[0] = m_ButtonPushed[1] = false;
	m_GrabbedKnob = false;
	m_GrabbedBackg = false;
	m_RebuildSize = true;
	m_RebuildKnob = true;
	m_ValueResolution = 1;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

GUIScrollPanel::GUIScrollPanel() : GUIPanel() {
	m_Skin = nullptr;
	m_DrawBitmap[0] = m_DrawBitmap[1] = m_DrawBitmap[2] = nullptr;
	m_ButtonSize = 17;
	m_MinimumKnobSize = 9;
	m_SmallChange = 1;
	m_ButtonPushed[0] = m_ButtonPushed[1] = false;
	m_GrabbedKnob = false;
	m_GrabbedBackg = false;
	m_RebuildSize = true;
	m_RebuildKnob = true;
	m_ValueResolution = 1;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIScrollPanel::Create(int X, int Y, int Width, int Height) {
	m_X = X;
	m_Y = Y;
	m_Width = Width;
	m_Height = Height;

	m_Minimum = 0;
	m_Maximum = 100;
	m_Value = 0;
	m_SmallChange = 1;
	m_PageSize = 1;
	m_Orientation = Horizontal;
	m_GrabbedKnob = false;
	m_GrabbedBackg = false;
	m_ValueResolution = 1;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIScrollPanel::Destroy() {
	// Destroy the three bitmaps
	for (int i = 0; i < 3; i++) {
		if (m_DrawBitmap[i]) {
			m_DrawBitmap[i]->Destroy();
			delete m_DrawBitmap[i];
			m_DrawBitmap[i] = 0;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIScrollPanel::LoadProps(GUIProperties *Props) {
	assert(Props);

	std::string Ori;
	Props->GetValue("Orientation", &Ori);

	m_Orientation = Horizontal;
	if (stricmp(Ori.c_str(), "Vertical") == 0) { m_Orientation = Vertical; }

	Props->GetValue("Minimum", &m_Minimum);
	Props->GetValue("Maximum", &m_Maximum);
	Props->GetValue("Value", &m_Value);
	Props->GetValue("PageSize", &m_PageSize);
	Props->GetValue("SmallChange", &m_SmallChange);
	if (!Props->GetValue("ValueResolution", &m_ValueResolution)) { m_ValueResolution = std::max((m_Maximum - m_Minimum) / 100, 1); }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIScrollPanel::ChangeSkin(GUISkin *Skin) {
	assert(Skin);

	m_Skin = Skin;

	// Build the bitmap
	BuildBitmap(true, true);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIScrollPanel::BuildBitmap(bool UpdateSize, bool UpdateKnob) {
	// It is normal if this function is called but the skin has not been set so we just ignore the call if the skin has not been set
	if (!m_Skin) {
		return;
	}

	// If we update the size, the knob is automatically updated
	if (UpdateSize) { UpdateKnob = true; }

	// Free the old bitmaps
	if (UpdateSize) {
		if (m_DrawBitmap[ButtonStates]) {
			m_DrawBitmap[ButtonStates]->Destroy();
			delete m_DrawBitmap[ButtonStates];
			m_DrawBitmap[ButtonStates] = nullptr;
		}
		if (m_DrawBitmap[Back]) {
			m_DrawBitmap[Back]->Destroy();
			delete m_DrawBitmap[Back];
			m_DrawBitmap[Back] = nullptr;
		}
	}
	if (UpdateKnob) {
		if (m_DrawBitmap[KnobStates]) {
			m_DrawBitmap[KnobStates]->Destroy();
			delete m_DrawBitmap[KnobStates];
			m_DrawBitmap[KnobStates] = nullptr;
		}
	}

	// Calculate the knob size & position
	CalculateKnob();


	// Create the 3 bitmaps
	if (m_Orientation == Vertical) {
		// Vertical
		if (UpdateSize) {
			m_DrawBitmap[ButtonStates] = m_Skin->CreateBitmap(m_Width * 2, m_ButtonSize * 2);
			m_DrawBitmap[Back] = m_Skin->CreateBitmap(m_Width, m_Height);
		}
		if (UpdateKnob) { m_DrawBitmap[KnobStates] = m_Skin->CreateBitmap(m_Width * 2, m_KnobLength); }
	} else {
		// Horizontal
		if (UpdateSize) {
			m_DrawBitmap[ButtonStates] = m_Skin->CreateBitmap(m_ButtonSize * 2, m_Height * 2);
			m_DrawBitmap[Back] = m_Skin->CreateBitmap(m_Width, m_Height);
		}
		if (UpdateKnob) { m_DrawBitmap[KnobStates] = m_Skin->CreateBitmap(m_KnobLength, m_Height * 2); }
	}

	// Update the buttons and the background
	if (UpdateSize) {
		if (m_Orientation == Vertical) {
			// Vertical
			BuildButton("ScrollTopArrow", 0, m_Width, m_ButtonSize);
			BuildButton("ScrollBottomArrow", m_ButtonSize, m_Width, m_ButtonSize);
		} else {
			// Horizontal
			BuildButton("ScrollLeftArrow", 0, m_ButtonSize, m_Height);
			BuildButton("ScrollRightArrow", m_Height, m_ButtonSize, m_Height);
		}
		// Build the background
		BuildBackground();
	}


	// Update the knob
	if (UpdateKnob && m_KnobLength > 0) {
		if (m_Orientation == Vertical) {
			// Vertical
			BuildKnob("ScrollKnob_Up", 0, 0, m_Width, m_KnobLength);
			BuildKnob("ScrollKnob_Down", m_Width, 0, m_Width, m_KnobLength);
		} else {
			// Horizontal
			BuildKnob("ScrollKnob_Up", 0, 0, m_KnobLength, m_Height);
			BuildKnob("ScrollKnob_Down", 0, m_Height, m_KnobLength, m_Height);
		}
	}

	// Don't need to rebuild next frame
	m_RebuildSize = false;
	m_RebuildKnob = false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIScrollPanel::BuildButton(const std::string &ArrowName, int Y, int Width, int Height) {
	// Create the buttons
	m_Skin->BuildStandardRect(m_DrawBitmap[ButtonStates], "ScrollButton_Up", 0, Y, Width, Height);

	m_Skin->BuildStandardRect(m_DrawBitmap[ButtonStates], "ScrollButton_Down", Width, Y, Width, Height);

	// Draw the arrows
	int Values[4];
	GUIRect Rect;

	// Load the image file
	std::string Filename;
	m_Skin->GetValue(ArrowName, "Filename", &Filename);
	GUIBitmap *Arrow = m_Skin->CreateBitmap(Filename);
	if (!Arrow) {
		return;
	}

	// Get the transparent colour key
	unsigned long ColorKey = 0;
	m_Skin->GetValue(ArrowName, "ColorKeyIndex", &ColorKey);
	ColorKey = m_Skin->ConvertColor(ColorKey, Arrow->GetColorDepth());
	Arrow->SetColorKey(ColorKey);

	// Get the source rectangle
	m_Skin->GetValue(ArrowName, "Rect", Values, 4);
	SetRect(&Rect, Values[0], Values[1], Values[0] + Values[2], Values[1] + Values[3]);

	Arrow->DrawTrans(m_DrawBitmap[ButtonStates], (Width / 2) - (Values[2] / 2), Y + (Height / 2) - (Values[3] / 2), &Rect);
	Arrow->DrawTrans(m_DrawBitmap[ButtonStates], Width + (Width / 2) - (Values[2] / 2) + 1, Y + (Height / 2) - (Values[3] / 2) + 1, &Rect);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIScrollPanel::BuildBackground() {
	std::string Filename;
	m_Skin->GetValue("ScrollBackground", "Filename", &Filename);

	GUIBitmap *Background = m_Skin->CreateBitmap(Filename);
	if (!Background) {
		return;
	}

	int Values[4];
	GUIRect Rect;
	m_Skin->GetValue("ScrollBackground", "Filler", Values, 4);
	SetRect(&Rect, Values[0], Values[1], Values[0] + Values[2], Values[1] + Values[3]);

	// Tile the filler
	for (int y = 0; y < m_Height; y += Values[3]) {
		for (int x = 0; x < m_Width; x += Values[2]) {
			Background->Draw(m_DrawBitmap[Back], x, y, &Rect);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIScrollPanel::BuildKnob(const std::string &Section, int X, int Y, int Width, int Height) {
	if (m_DrawBitmap[KnobStates]) { m_Skin->BuildStandardRect(m_DrawBitmap[KnobStates], Section, X, Y, Width, Height); }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIScrollPanel::SetMinimum(int Min) {
	m_Minimum = Min;
	m_Minimum = std::min(m_Minimum, m_Maximum);

	// Rebuild the knob bitmap
	m_RebuildKnob = true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIScrollPanel::SetMaximum(int Max) {
	m_Maximum = Max;
	m_Maximum = std::max(m_Maximum, m_Minimum);

	// Rebuild the knob bitmap
	m_RebuildKnob = true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIScrollPanel::SetValue(int Value) {
	m_Value = std::clamp(Value, m_Minimum, m_Maximum);
	CalculateKnob();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIScrollPanel::SetPageSize(int PageSize) {
	m_PageSize = PageSize;
	m_PageSize = std::max(m_PageSize, 1);

	// Rebuild the knob bitmap
	m_RebuildKnob = true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIScrollPanel::SetOrientation(int Orientation) {
	m_Orientation = Orientation;

	// Rebuild the whole bitmap
	m_RebuildKnob = true;
	m_RebuildSize = true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int GUIScrollPanel::GetValue() const {
	return m_Value;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int GUIScrollPanel::GetPageSize() const {
	return m_PageSize;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIScrollPanel::SetSmallChange(int SmallChange) {
	m_SmallChange = SmallChange;
	m_SmallChange = std::max(m_SmallChange, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIScrollPanel::Draw(GUIScreen *Screen) {
	GUIRect Rect;

	// Do we need to rebuild?
	if (m_RebuildKnob || m_RebuildSize) { BuildBitmap(m_RebuildSize, m_RebuildKnob); }

	GUIBitmap *Dest = Screen->GetBitmap();

	// Draw the background
	m_DrawBitmap[Back]->Draw(Dest, m_X, m_Y, 0);

	// Vertical
	if (m_Orientation == Vertical) {
		// Top Button
		int X = m_ButtonPushed[0] ? m_Width : 0;
		SetRect(&Rect, X, 0, X + m_Width, m_ButtonSize);
		m_DrawBitmap[ButtonStates]->Draw(Dest, m_X, m_Y, &Rect);

		// Bottom Button
		X = m_ButtonPushed[1] ? m_Width : 0;
		SetRect(&Rect, X, m_ButtonSize, X + m_Width, m_ButtonSize * 2);
		m_DrawBitmap[ButtonStates]->Draw(Dest, m_X, (m_Y + m_Height) - m_ButtonSize, &Rect);

		// Knob
		if (m_KnobLength > 0) {
			SetRect(&Rect, 0, 0, m_Width, m_KnobLength);
			m_DrawBitmap[KnobStates]->Draw(Dest, m_X, m_Y + m_ButtonSize + m_KnobPosition, &Rect);
		}
	}

	// Horizontal
	if (m_Orientation == Horizontal) {
		// Left Button
		int X = m_ButtonPushed[0] ? m_ButtonSize : 0;
		SetRect(&Rect, X, 0, X + m_ButtonSize, m_Height);
		m_DrawBitmap[ButtonStates]->Draw(Dest, m_X, m_Y, &Rect);

		// Right Button
		X = m_ButtonPushed[1] ? m_ButtonSize : 0;
		SetRect(&Rect, X, m_Height, X + m_ButtonSize, m_Height * 2);
		m_DrawBitmap[ButtonStates]->Draw(Dest, (m_X + m_Width) - m_ButtonSize, m_Y, &Rect);

		// Knob
		if (m_KnobLength > 0) {
			SetRect(&Rect, 0, 0, m_KnobLength, m_Height);
			m_DrawBitmap[KnobStates]->Draw(Dest, m_X + m_ButtonSize + m_KnobPosition, m_Y, &Rect);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIScrollPanel::OnMouseDown(int X, int Y, int Buttons, int Modifier) {
	m_ButtonPushed[0] = m_ButtonPushed[1] = false;
	m_GrabbedKnob = false;
	m_GrabbedBackg = false;
	m_GrabbedSide = 0;

	SendSignal(Grab, Buttons);

	// Vertical
	if (m_Orientation == Vertical) {
		int KnobTop = m_Y + m_ButtonSize + m_KnobPosition;

		if (Y < m_Y + m_ButtonSize) {
			// Grabbed the top button
			m_ButtonPushed[0] = true;
			AdjustValue(-m_SmallChange);

			// Track the hover
			TrackMouseHover(true, 500);

		} else if (Y > m_Y + m_Height - m_ButtonSize) {
			// Pushed the bottom button
			m_ButtonPushed[1] = true;
			AdjustValue(m_SmallChange);

			// Track the hover
			TrackMouseHover(true, 500);

		} else if (Y >= KnobTop && Y <= KnobTop + m_KnobLength) {
			// Grabbed the knob
			m_GrabbedKnob = true;
			m_GrabbedPos = Y - KnobTop;

		} else {
			// Grabbed the background region
			m_GrabbedBackg = true;
			m_GrabbedPos = Y - (m_Y + m_ButtonSize);
			if (m_GrabbedPos < m_KnobPosition) {
				AdjustValue(-m_PageSize);
				m_GrabbedSide = 0;
			} else {
				AdjustValue(m_PageSize);
				m_GrabbedSide = 1;
			}

			// Now if the knob is over the cursor, grab it instead
			KnobTop = m_Y + m_ButtonSize + m_KnobPosition;
			if (Y >= KnobTop && Y <= KnobTop + m_KnobLength) {
				m_GrabbedBackg = false;
				m_GrabbedKnob = true;
				m_GrabbedPos = Y - KnobTop;
			}

			// Track the hover
			TrackMouseHover(true, 500);
		}
	}

	// Horizontal
	if (m_Orientation == Horizontal) {
		int KnobTop = m_X + m_ButtonSize + m_KnobPosition;

		if (X < m_X + m_ButtonSize) {
			// Grabbed the left button
			m_ButtonPushed[0] = true;
			AdjustValue(-m_SmallChange);

			// Track the hover
			TrackMouseHover(true, 500);

		} else if (X > m_X + m_Width - m_ButtonSize) {
			// Pushed the right button
			m_ButtonPushed[1] = true;
			AdjustValue(m_SmallChange);

			// Track the hover
			TrackMouseHover(true, 500);

		} else if (X >= KnobTop && X <= KnobTop + m_KnobLength) {
			// Grabbed the knob
			m_GrabbedKnob = true;
			m_GrabbedPos = X - KnobTop;
		} else {
			// Grabbed the background region
			m_GrabbedBackg = true;
			m_GrabbedPos = X - (m_X + m_ButtonSize);
			if (m_GrabbedPos < m_KnobPosition) {
				AdjustValue(-m_PageSize);
				m_GrabbedSide = 0;
			} else {
				AdjustValue(m_PageSize);
				m_GrabbedSide = 1;
			}

			// Now if the knob is over the cursor, grab it instead
			KnobTop = m_X + m_ButtonSize + m_KnobPosition;
			if (X >= KnobTop && X <= KnobTop + m_KnobLength) {
				m_GrabbedBackg = false;
				m_GrabbedKnob = true;
				m_GrabbedPos = X - KnobTop;
			}

			// Track the hover
			TrackMouseHover(true, 500);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIScrollPanel::OnMouseUp(int X, int Y, int Buttons, int Modifier) {
	// No more grabs
	m_ButtonPushed[0] = m_ButtonPushed[1] = false;
	m_GrabbedKnob = false;
	m_GrabbedBackg = false;

	SendSignal(Release, Buttons);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIScrollPanel::OnMouseMove(int X, int Y, int Buttons, int Modifier) {
	int KnobTop = 0;
	int MoveLength = 1;
	int MousePos = X;

	// Vertical
	if (m_Orientation == Vertical) {
		MoveLength = m_Height - m_ButtonSize * 2;
		KnobTop = m_Y + m_ButtonSize + m_KnobPosition;
		MousePos = Y;
	}
	// Horizontal
	if (m_Orientation == Horizontal) {
		MoveLength = m_Width - m_ButtonSize * 2;
		KnobTop = m_X + m_ButtonSize + m_KnobPosition;
		MousePos = X;
	}

	// Move the knob if we have grabbed it
	if (m_GrabbedKnob) {
		int Delta = m_GrabbedPos - (MousePos - KnobTop);
		int OldValue = m_Value;
		m_KnobPosition -= Delta;

		// Clamp the knob
		m_KnobPosition = std::max(m_KnobPosition, 0);
		m_KnobPosition = std::min(m_KnobPosition, MoveLength - m_KnobLength);

		// Calculate the value
		int Area = MoveLength - m_KnobLength;
		if (Area > 0) {
			float p = (float)m_KnobPosition / (float)(Area);
			int MaxRange = (m_Maximum - m_Minimum) - m_PageSize;
			m_Value = (float)MaxRange * p + m_Minimum;

			// Clamp the value
			m_Value = std::max(m_Value, m_Minimum);
			m_Value = std::min(m_Value, m_Maximum - m_PageSize);

			if (OldValue != m_Value) { SendSignal(ChangeValue, 0); }
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIScrollPanel::OnMouseHover(int X, int Y, int Buttons, int Modifier) {
	// Ignore if the left mouse button is not down
	if (!(Buttons & MOUSE_LEFT)) {
		return;
	}

	// Ignore the event if the mouse is outside but keep the tracking
	if (!PointInside(X, Y)) {
		// Continue tracking the hover
		TrackMouseHover(true, 50);
		return;
	}

	// Vertical
	if (m_Orientation == Vertical) {
		int KnobTop = m_Y + m_ButtonSize + m_KnobPosition;

		if (Y < m_Y + m_ButtonSize && m_ButtonPushed[0]) {
			// Grabbed the top button
			AdjustValue(-m_SmallChange);

		} else if (Y > m_Y + m_Height - m_ButtonSize && m_ButtonPushed[1]) {
			// Pushed the bottom button
			AdjustValue(m_SmallChange);

		} else if (Y >= KnobTop && Y <= KnobTop + m_KnobLength) {
			// Ignore

		} else if (m_GrabbedBackg) {
			// Grabbed the background region
			int p = Y - (m_Y + m_ButtonSize);
			if (m_GrabbedSide == 0 && p < m_KnobPosition) {
				AdjustValue(-m_PageSize);
			} else if (m_GrabbedSide == 1 && p >= m_KnobPosition) {
				AdjustValue(m_PageSize);
			}
			// Now if the knob is over the cursor, grab it instead
			KnobTop = m_Y + m_ButtonSize + m_KnobPosition;
			if (Y >= KnobTop && Y <= KnobTop + m_KnobLength) {
				m_GrabbedBackg = false;
				m_GrabbedKnob = true;
				m_GrabbedPos = Y - KnobTop;
			}
		}
	}

	// Horizontal
	if (m_Orientation == Horizontal) {
		int KnobTop = m_X + m_ButtonSize + m_KnobPosition;

		if (X < m_X + m_ButtonSize && m_ButtonPushed[0]) {
			// Grabbed the left button
			AdjustValue(-m_SmallChange);

		} else if (X > m_X + m_Width - m_ButtonSize && m_ButtonPushed[1]) {
			// Pushed the right button
			AdjustValue(m_SmallChange);

		} else if (X >= KnobTop && X <= KnobTop + m_KnobLength) {
			// Ignore

		} else if (m_GrabbedBackg) {
			// Grabbed the background region
			int p = X - (m_X + m_ButtonSize);
			if (m_GrabbedSide == 0 && p < m_KnobPosition) {
				AdjustValue(-m_PageSize);
			} else if (m_GrabbedSide == 1 && p >= m_KnobPosition) {
				AdjustValue(m_PageSize);
			}
			// Now if the knob is over the cursor, grab it instead
			KnobTop = m_X + m_ButtonSize + m_KnobPosition;
			if (X >= KnobTop && X <= KnobTop + m_KnobLength) {
				m_GrabbedBackg = false;
				m_GrabbedKnob = true;
				m_GrabbedPos = X - KnobTop;
			}
		}
	}

	// Continue tracking the hover
	TrackMouseHover(true, 50);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIScrollPanel::SetSize(int Width, int Height) {
	// If there is no change in size, ignore the call
	if (m_Width == Width && m_Height == Height) {
		return;
	}

	GUIPanel::SetSize(Width, Height);

	// Rebuild the bitmaps
	m_RebuildSize = true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIScrollPanel::CalculateKnob() {
	int MoveLength = 1;

	// Calculate the length of the movable area (panel minus buttons)
	if (m_Orientation == Vertical) { MoveLength = m_Height - m_ButtonSize * 2; }
	if (m_Orientation == Horizontal) { MoveLength = m_Width - m_ButtonSize * 2; }

	// Calculate the knob length
	m_KnobLength = 0;
	if (m_Maximum - m_Minimum > 0) {
		float s = (float)m_PageSize / (float)(m_Maximum - m_Minimum);
		s *= MoveLength;
		m_KnobLength = (int)s;
	}
	// Make sure the knob is not too small
	m_KnobLength = std::max(m_KnobLength, m_MinimumKnobSize);
	if (MoveLength > 0) { m_KnobLength = std::min(m_KnobLength, MoveLength); }
	if (m_KnobLength < 0) { m_KnobLength = 0; }

	// Calculate the knob position
	m_KnobPosition = 0;
	int MaxRange = (m_Maximum - m_Minimum) - m_PageSize;
	if (MaxRange > 0) {
		float p = (float)(m_Value - m_Minimum) / (float)(MaxRange);
		int Area = MoveLength - m_KnobLength;
		m_KnobPosition = (int)((float)Area * p);
	}

	// Clamp the position
	m_KnobPosition = std::max(m_KnobPosition, 0);
	m_KnobPosition = std::min(m_KnobPosition, MoveLength - m_KnobLength);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIScrollPanel::AdjustValue(int Delta) {
	int OldValue = m_Value;

	m_Value += Delta;

	// Clamp the value
	m_Value = std::max(m_Value, m_Minimum);
	m_Value = std::min(m_Value, m_Maximum - m_PageSize);

	// Calculate the new knob position
	CalculateKnob();

	if (OldValue != m_Value) { SendSignal(ChangeValue, 0); }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIScrollPanel::SaveProps(GUIProperties *Props) const {
	assert(Props);

	Props->AddVariable("Orientation", m_Orientation == Horizontal ? "Horizontal" : "Vertical");
	Props->AddVariable("Minimum", m_Minimum);
	Props->AddVariable("Maximum", m_Maximum);
	Props->AddVariable("Value", m_Value);
	Props->AddVariable("PageSize", m_PageSize);
	Props->AddVariable("SmallChange", m_SmallChange);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int GUIScrollPanel::GetMinimum() const {
	return m_Minimum;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int GUIScrollPanel::GetMaximum() const {
	return m_Maximum;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int GUIScrollPanel::GetOrientation() const {
	return m_Orientation;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int GUIScrollPanel::GetSmallChange() const {
	return m_SmallChange;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int GUIScrollPanel::GetValueResolution() const {
	return m_ValueResolution;
}