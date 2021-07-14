#include "GUI.h"
#include "GUIListBox.h"

using namespace RTE;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

GUIListBox::GUIListBox(GUIManager *Manager, GUIControlManager *ControlManager) : GUIControl(), GUIListPanel(Manager) {
	m_ControlID = "LISTBOX";
	m_ControlManager = ControlManager;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIListBox::Create(const std::string &Name, int X, int Y, int Width, int Height) {
	GUIControl::Create(Name, X, Y, Width, Height);

	// Minimum size of the control
	m_MinWidth = 50;
	m_MinHeight = 12;

	// Default size of the control
	m_DefWidth = 100;
	m_DefHeight = 100;

	// Create the ListPanel
	int w = m_DefWidth;
	int h = m_DefHeight;
	if (Width != -1) { w = Width; }
	if (Height != -1) { h = Height; }

	// Make sure the control isn't too small
	w = std::max(w, m_MinWidth);
	h = std::max(h, m_MinHeight);

	GUIListPanel::Create(X, Y, w, h);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIListBox::Create(GUIProperties *Props) {
	GUIControl::Create(Props);

	// Minimum size of the control
	m_MinWidth = 100;
	m_MinHeight = 12;

	// Default size of the control
	m_DefWidth = 100;
	m_DefHeight = 100;

	// Setup the panel
	GUIPanel::LoadProperties(Props);

	// Make sure the listbox isn't too small
	m_Width = std::max(m_Width, m_MinWidth);
	m_Height = std::max(m_Height, m_MinHeight);

	GUIListPanel::Create(m_X, m_Y, m_Width, m_Height);

	// Get the properties
	bool Multi = false;
	Props->GetValue("MultiSelect", &Multi);
	SetMultiSelect(Multi);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIListBox::Destroy() {
	GUIListPanel::Destroy();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIListBox::ChangeSkin(GUISkin *Skin) {
	GUIListPanel::ChangeSkin(Skin);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIListBox::Move(int X, int Y) {
	SetPositionAbs(X, Y);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIListBox::Resize(int Width, int Height) {
	// Make sure the listbox isn't too small
	Width = std::max(Width, m_MinWidth);
	Height = std::max(Height, m_MinHeight);

	SetSize(Width, Height);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

GUIPanel * GUIListBox::GetPanel() {
	return this;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIListBox::GetControlRect(int *X, int *Y, int *Width, int *Height) {
	GUIListPanel::GetRect(X, Y, Width, Height);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIListBox::StoreProperties() {
	m_Properties.AddVariable("MultiSelect", GetMultiSelect());
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIListBox::ReceiveSignal(GUIPanel *Source, int Code, int Data) {
	if (Source->GetPanelID() == GetPanelID()) {
		if (Code == GUIListPanel::MouseMove) {
			AddEvent(GUIEvent::Notification, MouseMove, Data);
		} else if (Code == GUIListPanel::MouseEnter) {
			AddEvent(GUIEvent::Notification, MouseEnter, Data);
		} else if (Code == GUIListPanel::MouseLeave) {
			AddEvent(GUIEvent::Notification, MouseLeave, Data);
		} else if (Code == GUIListPanel::Select) {
			AddEvent(GUIEvent::Notification, Select, 0);
		} else if (Code == GUIListPanel::MouseDown) {
			AddEvent(GUIEvent::Notification, MouseDown, 0);
		} else if (Code == GUIListPanel::DoubleClick) {
			AddEvent(GUIEvent::Notification, DoubleClick, 0);
		} else if (Code == GUIListPanel::KeyDown) {
			AddEvent(GUIEvent::Notification, KeyDown, Data);
		} else if (Code == GUIListPanel::EdgeHit) {
			AddEvent(GUIEvent::Notification, EdgeHit, Data);
		}
	}

	GUIListPanel::ReceiveSignal(Source, Code, Data);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIListBox::ApplyProperties(GUIProperties *Props) {
	GUIControl::ApplyProperties(Props);

	bool Multi = false;
	m_Properties.GetValue("MultiSelect", &Multi);
	SetMultiSelect(Multi);

	// Rebuild the bitmap
	BuildBitmap(true, true);
}