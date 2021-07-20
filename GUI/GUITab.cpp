#include "GUI.h"
#include "GUITab.h"

using namespace RTE;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

GUITab::GUITab(GUIManager *Manager, GUIControlManager *ControlManager) : GUIControl(), GUIPanel(Manager) {
	m_ControlID = "TAB";
	m_Image = nullptr;
	m_ControlManager = ControlManager;
	m_Selected = false;
	m_Font = nullptr;
	m_Mouseover = false;
	m_FontColor = 0;
	m_Text = "";
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUITab::Create(const std::string &Name, int X, int Y, int Width, int Height) {
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

void GUITab::Create(GUIProperties *Props) {
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

	// Get the values
	Props->GetValue("Text", &m_Text);
	Props->GetValue("Checked", &m_Selected);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUITab::ChangeSkin(GUISkin *Skin) {
	GUIControl::ChangeSkin(Skin);

	// Build the checkbox bitmap
	BuildBitmap();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUITab::BuildBitmap() {
	std::string Filename;
	unsigned long ColorIndex = 0;
	int Values[4];

	// Load the image
	m_Skin->GetValue("Tab", "Filename", &Filename);
	m_Image = m_Skin->CreateBitmap(Filename);
	if (!m_Image) {
		return;
	}

	// Load the font
	m_Skin->GetValue("Tab", "Font", &Filename);
	m_Font = m_Skin->GetFont(Filename);

	m_Skin->GetValue("Tab", "FontColor", &m_FontColor);
	m_Skin->GetValue("Tab", "FontShadow", &m_FontShadow);
	m_Skin->GetValue("Tab", "FontKerning", &m_FontKerning);
	m_FontColor = m_Skin->ConvertColor(m_FontColor, m_Image->GetColorDepth());
	m_Font->CacheColor(m_FontColor);

	// Set the color key
	m_Skin->GetValue("Tab", "ColorKeyIndex", &ColorIndex);
	ColorIndex = m_Skin->ConvertColor(ColorIndex, m_Image->GetColorDepth());
	m_Image->SetColorKey(ColorIndex);

	// Load the source image rectangles

	// Base checkbox
	m_Skin->GetValue("Tab", "Base", Values, 4);
	SetRect(&m_ImageRects[0], Values[0], Values[1], Values[0] + Values[2], Values[1] + Values[3]);

	// Mouse over checkbox
	m_Skin->GetValue("Tab", "MouseOver", Values, 4);
	SetRect(&m_ImageRects[1], Values[0], Values[1], Values[0] + Values[2], Values[1] + Values[3]);

	// Selected
	m_Skin->GetValue("Tab", "Selected", Values, 4);
	SetRect(&m_ImageRects[2], Values[0], Values[1], Values[0] + Values[2], Values[1] + Values[3]);

	// Grayed out tab (for disabled mode)
	m_Skin->GetValue("Tab", "Disabled", Values, 4);
	SetRect(&m_ImageRects[3], Values[0], Values[1], Values[0] + Values[2], Values[1] + Values[3]);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUITab::Draw(GUIScreen *Screen) {
	if (!m_Image) {
		return;
	}

	// Setup the clipping
	Screen->GetBitmap()->SetClipRect(GetRect());

	// Calculate the y position of the base
	// Make it centered vertically
	int YPos = m_Height / 2 - (m_ImageRects[0].bottom - m_ImageRects[0].top) / 2 + m_Y;

	// Draw the base
	m_Image->DrawTrans(Screen->GetBitmap(), m_X, YPos, &m_ImageRects[0]);

	// Draw the selected one
	if (m_Selected) {
		if (m_Enabled) {
			m_Image->DrawTrans(Screen->GetBitmap(), m_X, YPos, &m_ImageRects[2]);
		} // else
			//m_Image->DrawTrans(Screen->GetBitmap(), m_X, YPos, &m_ImageRects[3]);
		//}
	}

	// If highlighted, draw that
	if (m_Mouseover || m_GotFocus) {
		m_Image->DrawTrans(Screen->GetBitmap(), m_X, YPos, &m_ImageRects[1]);
	} else if (!m_Enabled) {
		// Should show as grayed out and disabled when it is, regardless of checked or not
		m_Image->DrawTrans(Screen->GetBitmap(), m_X, YPos, &m_ImageRects[3]);
	}

	// Draw the text

	// Add a space to make a gap between checkbox & text
	std::string Text;
	std::string space = " ";
	Text = space.append(m_Text);

	if (m_Font) {
		m_Font->SetColor(m_FontColor);
		m_Font->SetKerning(m_FontKerning);
		// TODO: DONT HARDCODE TEXT OFFSET
		m_Font->Draw(Screen->GetBitmap(), m_X + 4, m_Y + (m_Height / 2) - (m_Font->GetFontHeight() / 2) - 1, Text, m_FontShadow);
	}
	Screen->GetBitmap()->SetClipRect(0);

	GUIPanel::Draw(Screen);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUITab::OnMouseDown(int X, int Y, int Buttons, int Modifier) {
	if (Buttons & MOUSE_LEFT) {
		// Push the checkbox down
		CaptureMouse();
		SetFocus();

		AddEvent(GUIEvent::Notification, Pushed, 0);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUITab::OnMouseUp(int X, int Y, int Buttons, int Modifier) {
	ReleaseMouse();

	// If the mouse is over the button, add the command to the event queue
	if (PointInside(X, Y) && Buttons & MOUSE_LEFT) { SetCheck(true); }

	AddEvent(GUIEvent::Notification, UnPushed, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUITab::OnMouseEnter(int X, int Y, int Buttons, int Modifier) {
	m_Mouseover = true;
	AddEvent(GUIEvent::Notification, Hovered, 0);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUITab::OnMouseLeave(int X, int Y, int Buttons, int Modifier) {
	m_Mouseover = false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

GUIPanel * GUITab::GetPanel() {
	return this;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUITab::Move(int X, int Y) {
	GUIPanel::SetPositionAbs(X, Y);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUITab::Resize(int Width, int Height) {
	// Make sure the control isn't too small
	Width = std::max(Width, m_MinWidth);
	Height = std::max(Height, m_MinHeight);

	GUIPanel::SetSize(Width, Height);

	// Rebuild the bitmap
	BuildBitmap();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUITab::GetControlRect(int *X, int *Y, int *Width, int *Height) {
	GUIPanel::GetRect(X, Y, Width, Height);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUITab::StoreProperties() {
	m_Properties.AddVariable("Text", m_Text);
	m_Properties.AddVariable("Selected", m_Selected);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUITab::SetCheck(bool Check) {
	// Nothing to do if already in the same state
	if (m_Selected == Check) {
		return;
	}
	m_Selected = Check;

	AddEvent(GUIEvent::Notification, Changed, Check);

	// Don't worry if we are not checked
	if (!m_Selected) {
		return;
	}

	// Go through all my RadioButton siblings and un-check them
	if (m_ControlParent) {
		std::vector<GUIControl *>::iterator it;
		std::vector<GUIControl *> *Children = m_ControlParent->GetChildren();

		for (it = Children->begin(); it != Children->end(); it++) {
			GUIControl *C = *it;
			if (C) {
				// Make sure this is not me
				if (C->GetPanel() && GetPanel() && C->GetPanel()->GetPanelID() == GetPanel()->GetPanelID()) {
					continue;
				}
				// Make sure the control is a radio button
				if (C->GetID().compare(GetID()) == 0) {
					GUITab *R = (GUITab *)C;
					R->SetCheck(false);
				}
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool GUITab::GetCheck() const {
	return m_Selected;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUITab::SetText(const std::string &Text) {
	m_Text = Text;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string GUITab::GetText() const {
	return m_Text;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUITab::ApplyProperties(GUIProperties *Props) {
	GUIControl::ApplyProperties(Props);

	m_Properties.GetValue("Text", &m_Text);
	m_Properties.GetValue("Selected", &m_Selected);
}