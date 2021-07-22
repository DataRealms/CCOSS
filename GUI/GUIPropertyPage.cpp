#include "GUI.h"
#include "GUIPropertyPage.h"

using namespace RTE;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

GUIPropertyPage::GUIPropertyPage(GUIManager *Manager, GUIControlManager *ControlManager) : GUIControl(), GUIPanel(Manager) {
	m_ControlID = "PROPERTYPAGE";
	m_DrawBitmap = nullptr;
	m_ControlManager = ControlManager;
	m_Font = nullptr;
	m_VertScroll = nullptr;
	m_FontColor = 0;
	m_LineColor = 0;
	m_PageValues.Clear();
	m_TextPanelList.clear();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIPropertyPage::Create(const std::string &Name, int X, int Y, int Width, int Height) {
	GUIControl::Create(Name, X, Y, Width, Height);

	// Minimum size of the control
	m_MinWidth = 50;
	m_MinHeight = 50;

	// Default size of the control
	m_DefWidth = 80;
	m_DefHeight = 100;

	// Setup the panel
	m_X = X;
	m_Y = Y;
	m_Width = m_DefWidth;
	m_Height = m_DefHeight;

	if (Width != -1) { m_Width = Width; }
	if (Height != -1) { m_Height = Height; }

	// Make sure the control isn't too small
	m_Width = std::max(m_Width, m_MinWidth);
	m_Height = std::max(m_Height, m_MinHeight);

	// Create the vertical scrollbar
	m_VertScroll = new GUIScrollPanel(m_Manager);

	m_VertScroll->Create(m_Width - 12, 0, 12, m_Height);
	m_VertScroll->SetOrientation(GUIScrollPanel::Vertical);
	m_VertScroll->_SetVisible(false);
	m_VertScroll->SetValue(0);
	m_VertScroll->SetSignalTarget(this);

	GUIPanel::AddChild(m_VertScroll);

	// Create the text panels
	int H = 16;
	int Spacer = 0;
	int Size = m_Height / H;
	for (int i = 0; i < Size; i++) {
		GUITextPanel *T = new GUITextPanel(m_Manager);
		T->Create(m_Width / 2, i*H + Spacer, m_Width / 2, H);
		T->_SetVisible(false);
		T->SetSignalTarget(this);
		GUIPanel::AddChild(T);

		m_TextPanelList.push_back(T);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIPropertyPage::Create(GUIProperties *Props) {
	GUIControl::Create(Props);

	// Minimum size of the control
	m_MinWidth = 50;
	m_MinHeight = 50;

	// Default size of the control
	m_DefWidth = 80;
	m_DefHeight = 100;

	// Setup the panel
	GUIPanel::LoadProperties(Props);

	// Make sure the control isn't too small
	m_Width = std::max(m_Width, m_MinWidth);
	m_Height = std::max(m_Height, m_MinHeight);

	// Create the vertical scrollbar
	m_VertScroll = new GUIScrollPanel(m_Manager);

	m_VertScroll->Create(m_Width - 12, 0, 12, m_Height);
	m_VertScroll->SetOrientation(GUIScrollPanel::Vertical);
	m_VertScroll->_SetVisible(false);
	m_VertScroll->SetValue(0);
	m_VertScroll->SetSignalTarget(this);

	GUIPanel::AddChild(m_VertScroll);

	// Create the text panels
	int H = 16;
	int Spacer = 0;
	int Size = m_Height / H;
	for (int i = 0; i < Size; i++) {
		GUITextPanel *T = new GUITextPanel(m_Manager);
		T->Create(m_Width / 2, i*H + Spacer, m_Width / 2, H);
		T->_SetVisible(false);
		T->SetSignalTarget(this);
		GUIPanel::AddChild(T);

		m_TextPanelList.push_back(T);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIPropertyPage::Destroy() {
	// Free the drawing bitmap
	if (m_DrawBitmap) {
		m_DrawBitmap->Destroy();
		delete m_DrawBitmap;
		m_DrawBitmap = nullptr;
	}

	// Free the vertical scrollbar
	if (m_VertScroll) {
		m_VertScroll->Destroy();
		delete m_VertScroll;
		m_VertScroll = nullptr;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIPropertyPage::ChangeSkin(GUISkin *Skin) {
	GUIControl::ChangeSkin(Skin);

	// Change the skin of the text panels
	for (GUITextPanel *textPanel : m_TextPanelList) {
		textPanel->ChangeSkin(Skin);
	}

	// Build the control bitmap
	BuildBitmap();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIPropertyPage::BuildBitmap() {
	// Free any old bitmap
	if (m_DrawBitmap) {
		m_DrawBitmap->Destroy();
		delete m_DrawBitmap;
		m_DrawBitmap = nullptr;
	}

	// Create a new bitmap.
	m_DrawBitmap = m_Skin->CreateBitmap(m_Width, m_Height);

	m_Skin->BuildStandardRect(m_DrawBitmap, "PropertyPage", 0, 0, m_Width, m_Height);



	// Pre-cache the font
	std::string Filename;
	m_Skin->GetValue("PropertyPage", "Font", &Filename);
	m_Skin->GetValue("PropertyPage", "FontShadow", &m_FontShadow);
	m_Skin->GetValue("PropertyPage", "FontColor", &m_FontColor);
	m_Skin->GetValue("PropertyPage", "FontKerning", &m_FontKerning);

	m_FontColor = m_Skin->ConvertColor(m_FontColor, m_DrawBitmap->GetColorDepth());

	m_Font = m_Skin->GetFont(Filename);
	if (m_Font) { m_Font->CacheColor(m_FontColor); }

	m_Skin->GetValue("PropertyPage", "LineColor", &m_LineColor);
	m_LineColor = m_Skin->ConvertColor(m_LineColor, m_DrawBitmap->GetColorDepth());
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIPropertyPage::Draw(GUIScreen *Screen) {
	if (m_DrawBitmap) { m_DrawBitmap->Draw(Screen->GetBitmap(), m_X, m_Y, nullptr); }

	// Check the font first
	if (!m_Font) {
		return;
	}

	// Draw the properties
	int Count = m_PageValues.GetCount();
	int Spacer = 2;
	int Y = m_Y + Spacer;
	int Size = 16;
	std::string Name;
	std::string Value;

	for (int i = 0; i < Count; i++) {
		m_PageValues.GetVariable(i, &Name, &Value);
		m_Font->SetColor(m_FontColor);
		m_Font->SetKerning(m_FontKerning);
		m_Font->Draw(Screen->GetBitmap(), m_X + Spacer, Y, Name, m_FontShadow);

		Screen->GetBitmap()->DrawRectangle(m_X + 1, Y + Size + (m_Font->GetFontHeight() / 2 - Size / 2), m_Width - 2, 0, m_LineColor, false);
		Y += Size;
	}
	Screen->GetBitmap()->DrawRectangle(m_X + m_Width / 2, m_Y + 1, 0, Y - m_Y - Spacer * 2, m_LineColor, false);

	GUIPanel::Draw(Screen);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIPropertyPage::OnMouseDown(int X, int Y, int Buttons, int Modifier) {
	if (Buttons & MOUSE_LEFT) {
		// Push the button down
		//m_Pushed = true;
		//CaptureMouse();

		//AddEvent(GUIEvent::Notification, Pushed, 0);
	}
	SetFocus();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIPropertyPage::OnMouseUp(int X, int Y, int Buttons, int Modifier) {}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIPropertyPage::OnMouseEnter(int X, int Y, int Buttons, int Modifier) {
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIPropertyPage::OnMouseLeave(int X, int Y, int Buttons, int Modifier) {
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIPropertyPage::OnMouseMove(int X, int Y, int Buttons, int Modifier) {
	if (!(Buttons & MOUSE_LEFT) || !IsCaptured()) {
		return;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

GUIPanel * GUIPropertyPage::GetPanel() {
	return this;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIPropertyPage::GetControlRect(int *X, int *Y, int *Width, int *Height) {
	GUIPanel::GetRect(X, Y, Width, Height);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIPropertyPage::Move(int X, int Y) {
	GUIPanel::SetPositionAbs(X, Y);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIPropertyPage::Resize(int Width, int Height) {
	// Make sure the control isn't too small
	Width = std::max(Width, m_MinWidth);
	Height = std::max(Height, m_MinHeight);

	GUIPanel::SetSize(Width, Height);

	// TODO: Alter text panels

	BuildBitmap();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIPropertyPage::StoreProperties() {
	// Note: This is for saving the control, not related directly to our control type
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIPropertyPage::SetPropertyValues(GUIProperties *Props) {
	m_PageValues.Clear();
	m_PageValues.Update(Props, true);

	// Update the text panels
	for (int i = 0; i < m_TextPanelList.size(); i++) {
		GUITextPanel *T = m_TextPanelList.at(i);
		T->_SetVisible(false);
		T->SetText("");

		if (i < m_PageValues.GetCount()) {
			T->_SetVisible(true);
			std::string Name;
			std::string Value;
			if (m_PageValues.GetVariable(i, &Name, &Value)) { T->SetText(Value); }
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

GUIProperties * GUIPropertyPage::GetPropertyValues() {
	return &m_PageValues;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIPropertyPage::ReceiveSignal(GUIPanel *Source, int Code, int Data) {
	assert(Source);

	bool TextSignal = false;

	// Is this a text panel?
	std::vector<GUITextPanel *>::iterator it;
	for (it = m_TextPanelList.begin(); it != m_TextPanelList.end(); it++) {
		const GUITextPanel *T = *it;

		if (Source->GetPanelID() == T->GetPanelID()) {
			TextSignal = true;

			// Change event. Do not update properties
			if (Code == GUITextPanel::Changed) {
				AddEvent(GUIEvent::Notification, GUIPropertyPage::Changed, 0);
				return;
			}
			break;
		}
	}

	// Update the properties.
	// If any of the values are different, fire a 'changed' notification event
	if (TextSignal && InvokeUpdate()) {
		// Fire the enter event
		AddEvent(GUIEvent::Notification, GUIPropertyPage::Enter, 0);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool GUIPropertyPage::InvokeUpdate() {
	bool Changed = false;

	for (int i = 0; i < m_TextPanelList.size(); i++) {
		const GUITextPanel *T = m_TextPanelList.at(i);

		if (i < m_PageValues.GetCount()) {
			std::string Name;
			std::string Value;
			if (m_PageValues.GetVariable(i, &Name, &Value)) {
				if (T->GetText().compare(Value) != 0) { Changed = true; }
				// Set the value
				m_PageValues.SetVariable(i, Name, T->GetText());
			}
		}
	}

	return Changed;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIPropertyPage::ClearValues() {
	m_PageValues.Clear();

	// Hide the text panels
	std::vector<GUITextPanel *>::iterator it;
	for (it = m_TextPanelList.begin(); it != m_TextPanelList.end(); it++) {
		GUITextPanel *T = *it;
		T->_SetVisible(false);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool GUIPropertyPage::HasTextFocus() {
	std::vector<GUITextPanel *>::iterator it;
	for (it = m_TextPanelList.begin(); it != m_TextPanelList.end(); it++) {
		const GUITextPanel *T = *it;

		// Visible & has focus??
		if (T->_GetVisible() && T->HasFocus()) {
			return true;
		}
	}

	return false;
}
