#include "GUI.h"
#include "GUITextBox.h"

using namespace RTE;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

GUITextBox::GUITextBox(GUIManager *Manager, GUIControlManager *ControlManager) : GUIControl(), GUITextPanel(Manager) {
	m_ControlID = "TEXTBOX";
	m_ControlManager = ControlManager;
	m_DrawBitmap = nullptr;
	m_HAlignment = GUIFont::Left;
	m_VAlignment = GUIFont::Top;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUITextBox::Create(const std::string &Name, int X, int Y, int Width, int Height) {
	GUIControl::Create(Name, X, Y, Width, Height);

	// Minimum size of the control
	m_MinWidth = 30;
	m_MinHeight = 10;

	// Default size of the control
	m_DefWidth = 60;
	m_DefHeight = 16;

	// Create the ListPanel
	int w = m_DefWidth;
	int h = m_DefHeight;
	if (Width != -1) { w = Width; }
	if (Height != -1) { h = Height; }

	GUITextPanel::Create(X, Y, w, h);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUITextBox::Create(GUIProperties *Props) {
	GUIControl::Create(Props);

	// Minimum size of the control
	m_MinWidth = 30;
	m_MinHeight = 10;

	// Default size of the control
	m_DefWidth = 60;
	m_DefHeight = 16;

	// Setup the panel
	GUIPanel::LoadProperties(Props);
	GUITextPanel::Create(m_X, m_Y, m_Width, m_Height);

	// Make sure the textbox isn't too small
	m_Width = std::max(m_Width, m_MinWidth);
	m_Height = std::max(m_Height, m_MinHeight);

	// Alignment values - these don't affect anything as of yet
	std::string alignString;
	Props->GetValue("HAlignment", &alignString);
	if (stricmp(alignString.c_str(), "left") == 0) { m_HAlignment = GUIFont::Left; }
	if (stricmp(alignString.c_str(), "centre") == 0 || stricmp(alignString.c_str(), "center") == 0) { m_HAlignment = GUIFont::Centre; }
	if (stricmp(alignString.c_str(), "right") == 0) { m_HAlignment = GUIFont::Right; }

	Props->GetValue("VAlignment", &alignString);
	if (stricmp(alignString.c_str(), "top") == 0) { m_VAlignment = GUIFont::Top; }
	if (stricmp(alignString.c_str(), "middle") == 0) { m_VAlignment = GUIFont::Middle; }
	if (stricmp(alignString.c_str(), "bottom") == 0) { m_VAlignment = GUIFont::Bottom; }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUITextBox::Destroy() {
	// Destroy the draw bitmap
	if (m_DrawBitmap) {
		m_DrawBitmap->Destroy();
		delete m_DrawBitmap;
		m_DrawBitmap = nullptr;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUITextBox::ChangeSkin(GUISkin *Skin) {
	GUIControl::ChangeSkin(Skin);

	// Free any old bitmap
	if (m_DrawBitmap) {
		m_DrawBitmap->Destroy();
		delete m_DrawBitmap;
		m_DrawBitmap = nullptr;
	}

	// Create a new bitmap
	m_DrawBitmap = m_Skin->CreateBitmap(m_Width, m_Height);

	// Build the background
	m_Skin->BuildStandardRect(m_DrawBitmap, "TextBox", 0, 0, m_Width, m_Height);

	// Setup the skin in the panel too
	GUITextPanel::ChangeSkin(Skin);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUITextBox::Draw(GUIScreen *Screen) {
	// Draw the background
	m_DrawBitmap->Draw(Screen->GetBitmap(), m_X, m_Y, nullptr);

	GUITextPanel::Draw(Screen);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

GUIPanel * GUITextBox::GetPanel() {
	return this;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUITextBox::Move(int X, int Y) {
	GUITextPanel::SetPositionAbs(X, Y);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUITextBox::Resize(int Width, int Height) {
	// Make sure the control isn't too small
	Width = std::max(Width, m_MinWidth);
	Height = std::max(Height, m_MinHeight);

	GUITextPanel::SetSize(Width, Height);

	// Force a rebuild
	ChangeSkin(m_Skin);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUITextBox::GetControlRect(int *X, int *Y, int *Width, int *Height) {
	GUITextPanel::GetRect(X, Y, Width, Height);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUITextBox::ReceiveSignal(GUIPanel *Source, int Code, int Data) {
	// Clicked
	if (Code == GUITextPanel::Clicked) { AddEvent(GUIEvent::Notification, Clicked, Data); }

	// Changed
	if (Code == GUITextPanel::Changed) { AddEvent(GUIEvent::Notification, Changed, 0); }

	// Enter
	if (Code == GUITextPanel::Enter) { AddEvent(GUIEvent::Notification, Enter, 0); }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUITextBox::ApplyProperties(GUIProperties *Props) {
	GUIControl::ApplyProperties(Props);

	// Force a rebuild of the bitmap
	ChangeSkin(m_Skin);
}