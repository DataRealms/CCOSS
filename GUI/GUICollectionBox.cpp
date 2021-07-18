#include "GUI.h"
#include "GUICollectionBox.h"

using namespace RTE;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

GUICollectionBox::GUICollectionBox(GUIManager *Manager, GUIControlManager *ControlManager) : GUIControl(), GUIPanel(Manager) {
	m_ControlID = "COLLECTIONBOX";
	m_Background = nullptr;
	m_ControlManager = ControlManager;
	m_DrawBackground = true;
	m_DrawType = Color;
	m_DrawColor = 0;
	m_DrawBitmap = nullptr;

	m_IsContainer = true; // We are a container
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUICollectionBox::Create(const std::string &Name, int X, int Y, int Width, int Height) {
	GUIControl::Create(Name, X, Y, Width, Height);

	// Minimum size of the control
	m_MinWidth = 10;
	m_MinHeight = 10;

	// Default size of the control
	m_DefWidth = 100;
	m_DefHeight = 100;

	// Setup the panel
	m_X = X;
	m_Y = Y;
	m_Width = m_DefWidth;
	m_Height = m_DefHeight;

	if (Width != -1) { m_Width = Width; }
	if (Height != -1) { m_Height = Height; }

	// Make sure the box isn't too small
	m_Width = std::max(m_Width, m_MinWidth);
	m_Height = std::max(m_Height, m_MinHeight);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUICollectionBox::Create(GUIProperties *Props) {
	GUIControl::Create(Props);

	// Minimum size of the control
	m_MinWidth = 10;
	m_MinHeight = 10;

	// Default size of the control
	m_DefWidth = 100;
	m_DefHeight = 100;

	// Setup the panel
	GUIPanel::LoadProperties(Props);

	// Make sure the box isn't too small
	m_Width = std::max(m_Width, m_MinWidth);
	m_Height = std::max(m_Height, m_MinHeight);

	// Get the values
	Props->GetValue("DrawBackground", &m_DrawBackground);
	std::string v;
	Props->GetValue("DrawType", &v);
	if (stricmp(v.c_str(), "Color") == 0) {
		m_DrawType = Color;
	} else if (stricmp(v.c_str(), "Image") == 0) {
		m_DrawType = Image;
	} else if (stricmp(v.c_str(), "Panel") == 0) {
		m_DrawType = Panel;
	}
	Props->GetValue("DrawColor", &m_DrawColor);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUICollectionBox::Destroy() {
	delete m_Background;
	delete m_DrawBitmap;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUICollectionBox::ChangeSkin(GUISkin *Skin) {
	GUIControl::ChangeSkin(Skin);

	// Build the panel bitmap
	BuildBitmap();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUICollectionBox::BuildBitmap() {
	// Free any old bitmap
	delete m_DrawBitmap;

	// Create a new bitmap.
	m_DrawBitmap = m_Skin->CreateBitmap(m_Width, m_Height);

	// Create the button image
	m_Skin->BuildStandardRect(m_DrawBitmap, "CollectionBox_Panel", 0, 0, m_Width, m_Height);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUICollectionBox::Draw(GUIScreen *Screen) {
	if (m_DrawBackground) {
		if (m_DrawType == Color) {
			Screen->GetBitmap()->DrawRectangle(m_X, m_Y, m_Width, m_Height, m_Skin->ConvertColor(m_DrawColor, Screen->GetBitmap()->GetColorDepth()), true);
		} else if (m_DrawType == Image) {
			if (m_DrawBitmap && m_DrawBackground) {
				// Setup the clipping
				Screen->GetBitmap()->SetClipRect(GetRect());

				// Draw the image
				m_DrawBitmap->DrawTrans(Screen->GetBitmap(), m_X, m_Y, 0);

				// Get rid of clipping
				Screen->GetBitmap()->SetClipRect(0);
			}
		} else if (m_DrawType == Panel && m_DrawBackground) {
			if (m_DrawBitmap) {
				GUIRect Rect;
				SetRect(&Rect, 0, 0, m_Width, m_Height);
				Screen->DrawBitmapTrans(m_DrawBitmap, m_X, m_Y, &Rect);
			}
		}
	}
	GUIPanel::Draw(Screen);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUICollectionBox::OnMouseDown(int X, int Y, int Buttons, int Modifier) {
	CaptureMouse();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUICollectionBox::OnMouseUp(int X, int Y, int Buttons, int Modifier) {
	ReleaseMouse();

	AddEvent(GUIEvent::Notification, Clicked, Buttons);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUICollectionBox::OnMouseMove(int X, int Y, int Buttons, int Modifier) {
	AddEvent(GUIEvent::Notification, MouseMove, Buttons);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

GUIPanel * GUICollectionBox::GetPanel() {
	return this;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUICollectionBox::Move(int X, int Y) {
	int DX = X - m_X;
	int DY = Y - m_Y;

	m_X = X;
	m_Y = Y;

	// Go through all my children moving them
	std::vector<GUIControl *>::iterator it;
	for (it = m_ControlChildren.begin(); it != m_ControlChildren.end(); it++) {
		GUIControl *C = *it;
		int CX;
		int CY;
		int CW;
		int CH;
		C->GetControlRect(&CX, &CY, &CW, &CH);

		C->Move(CX + DX, CY + DY);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUICollectionBox::Resize(int Width, int Height) {
	int OldWidth = m_Width;
	int OldHeight = m_Height;

	m_Width = Width;
	m_Height = Height;

	// Go through all my children moving them
	std::vector<GUIControl *>::iterator it;
	for (it = m_ControlChildren.begin(); it != m_ControlChildren.end(); it++) {
		GUIControl *C = *it;
		int CX, CY, CW, CH;
		int Anchor = C->GetAnchor();

		C->GetControlRect(&CX, &CY, &CW, &CH);

		int DX = CX;
		int DY = CY;
		int W = CW;
		int H = CH;

		// Attached to Right and/or Bottom edges
		if ((Anchor & GUIControl::Anchor_Right) && !(Anchor & GUIControl::Anchor_Left)) { DX = m_Width - (OldWidth - (CX - m_X)) + m_X; }
		if ((Anchor & GUIControl::Anchor_Bottom) && !(Anchor & GUIControl::Anchor_Top)) { DY = m_Height - (OldHeight - (CY - m_Y)) + m_Y; }

		if (DX != CX || DY != CY) { C->Move(DX, DY); }

		CX -= m_X;
		CY -= m_Y;

		// Attached to opposing edges
		if (Anchor & GUIControl::Anchor_Left && Anchor & GUIControl::Anchor_Right) { W = (m_Width - (OldWidth - (CX + CW))) - CX; }
		if (Anchor & GUIControl::Anchor_Top && Anchor & GUIControl::Anchor_Bottom) { H = (m_Height - (OldHeight - (CY + CH))) - CY; }

		if (W != CW || H != CH) { C->Resize(W, H); }
	}
	BuildBitmap();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUICollectionBox::GetControlRect(int *X, int *Y, int *Width, int *Height) {
	GUIPanel::GetRect(X, Y, Width, Height);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUICollectionBox::SetDrawImage(GUIBitmap *Bitmap) {
	// Free any old bitmap
	delete m_DrawBitmap;

	m_DrawBitmap = Bitmap;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUICollectionBox::SetDrawBackground(bool DrawBack) {
	m_DrawBackground = DrawBack;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUICollectionBox::SetDrawType(int Type) {
	m_DrawType = Type;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUICollectionBox::SetDrawColor(unsigned long Color) {
	m_DrawColor = Color;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUICollectionBox::StoreProperties() {
	m_Properties.AddVariable("DrawBackground", m_DrawBackground);
	m_Properties.AddVariable("DrawType", m_DrawType == Color ? "Color" : "Image");
	m_Properties.AddVariable("DrawColor", (int)m_DrawColor);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUICollectionBox::ApplyProperties(GUIProperties *Props) {
	GUIControl::ApplyProperties(Props);

	// Get the values
	m_Properties.GetValue("DrawBackground", &m_DrawBackground);
	std::string v;
	m_Properties.GetValue("DrawType", &v);
	if (stricmp(v.c_str(), "Color") == 0) {
		m_DrawType = Color;
	} else if (stricmp(v.c_str(), "Image") == 0) {
		m_DrawType = Image;
	}
	m_Properties.GetValue("DrawColor", &m_DrawColor);
}