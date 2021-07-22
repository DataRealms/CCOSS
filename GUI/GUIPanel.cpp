#include "GUI.h"

using namespace RTE;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

GUIPanel::GUIPanel(GUIManager *Manager) {
	Clear();
	m_Manager = Manager;
	m_Font = nullptr;
	m_FontColor = 0;
	m_FontShadow = 0;
	m_FontKerning = 1;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

GUIPanel::GUIPanel() {
	Clear();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIPanel::Clear() {
	m_X = 0;
	m_Y = 0;
	m_Width = 0;
	m_Height = 0;
	m_ID = -1;
	m_GotFocus = false;
	m_Captured = false;
	m_Visible = true;
	m_Enabled = true;

	m_Parent = nullptr;
	m_Children.clear();

	m_Manager = nullptr;
	m_ValidRegion = false;
	m_SignalTarget = this;
	m_ZPos = 0;

	m_Font = nullptr;
	m_FontColor = 0;
	m_FontShadow = 0;
	m_FontKerning = 1;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIPanel::Setup(GUIManager *manager, int ZPos) {
	m_Manager = manager;
	m_ZPos = ZPos;

	// Request a new ID
	m_ID = m_Manager->GetPanelID();

	// Set the manager for all the children
	int Z = 0;
	std::vector<GUIPanel *>::iterator it;
	for (it = m_Children.begin(); it != m_Children.end(); it++) {
		GUIPanel *P = *it;
		if (P) {
			Z++;
			P->Setup(manager, Z);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIPanel::AddChild(GUIPanel *child, bool convertToAbsolutePos) {
	if (child) {
		// Convert the child's coordinates into absolute coordinates
		if (convertToAbsolutePos) {
			child->m_X += m_X;
			child->m_Y += m_Y;
		}

		// Make sure the rectangle is valid
		child->m_Width = std::max(child->m_Width, 0);
		child->m_Height = std::max(child->m_Height, 0);

		int zPos = 0;
		if (!m_Children.empty()) {
			const GUIPanel *lastChild = m_Children.back();
			zPos = lastChild->GetZPos() + 1;
		}

		// Remove the child from any previous parent
		if (child->GetParentPanel()) { child->GetParentPanel()->GUIPanel::RemoveChild(child); }

		// Setup the inherited values
		child->m_Parent = this;
		child->Setup(m_Manager, zPos);

		// Add the child to the list
		m_Children.push_back(child);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIPanel::RemoveChild(const GUIPanel *pChild) {
	// Note: We do NOT free the children because they are still linked in through their controls. This merely removes the panel from the list.
	// This will cause a small memory leak, but this is only designed for the GUI Editor and is a bit of a hack

	for (std::vector<GUIPanel *>::iterator itr = m_Children.begin(); itr != m_Children.end(); itr++) {
		const GUIPanel *pPanel = *itr;
		if (pPanel && pPanel == pChild) {
			m_Children.erase(itr);
			break;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIPanel::LoadProperties(GUIProperties *Props) {
	assert(Props);

	Props->GetValue("X", &m_X);
	Props->GetValue("Y", &m_Y);
	Props->GetValue("Width", &m_Width);
	Props->GetValue("Height", &m_Height);

	Props->GetValue("Visible", &m_Visible);
	Props->GetValue("Enabled", &m_Enabled);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIPanel::Invalidate() {
	m_ValidRegion = false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool GUIPanel::IsValid() const {
	return m_ValidRegion;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIPanel::Draw(GUIScreen *Screen) {
	// Validate this panel
	m_ValidRegion = true;

	// Calculate this panel's clipping region - set the clipping rect to be the intersection of what
	// was already set by the parent, and the dimensions of this panel.
	Screen->GetBitmap()->AddClipRect(GetRect());
	// Now save this intersection clipping rect so we can re-set it before each new child is drawn
	GUIRect thisClip;
	Screen->GetBitmap()->GetClipRect(&thisClip);

	// Draw children
	std::vector<GUIPanel *>::iterator it;
	for (it = m_Children.begin(); it != m_Children.end(); it++) {
		GUIPanel *P = *it;

		if (P->_GetVisible()) {
			// Re-set the clipping rect of this panel since the last child has messed with it
			Screen->GetBitmap()->SetClipRect(&thisClip);
			P->Draw(Screen);
		}
	}

	// Get rid of the clipping rect since the parent will re-set it if necessary
	Screen->GetBitmap()->SetClipRect(nullptr);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIPanel::OnMouseDown(int X, int Y, int Buttons, int Modifier) {}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIPanel::OnMouseUp(int X, int Y, int Buttons, int Modifier) {}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIPanel::OnDoubleClick(int X, int Y, int Buttons, int Modifier) {}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIPanel::OnMouseMove(int X, int Y, int Buttons, int Modifier) {}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIPanel::OnMouseEnter(int X, int Y, int Buttons, int Modifier) {}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIPanel::OnMouseLeave(int X, int Y, int Buttons, int Modifier) {}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIPanel::OnMouseHover(int X, int Y, int Buttons, int Modifier) {}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIPanel::OnKeyDown(int KeyCode, int Modifier) {}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIPanel::OnKeyUp(int KeyCode, int Modifier) {}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIPanel::OnKeyPress(int KeyCode, int Modifier) {}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIPanel::OnGainFocus() {
	m_GotFocus = true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIPanel::OnLoseFocus() {
	m_GotFocus = false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIPanel::CaptureMouse() {
	m_Manager->CaptureMouse(this);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIPanel::ReleaseMouse() {
	m_Manager->ReleaseMouse();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

GUIPanel * GUIPanel::BottomPanelUnderPoint(int x, int y) {
	if (!PointInside(x, y)) {
		return nullptr;
	}
	// If this panel is invisible or disabled, this panel is ignored
	if (!m_Visible || !m_Enabled) {
		return nullptr;
	}

	// Go through the children
	GUIPanel *CurPanel = nullptr;
	std::vector<GUIPanel *>::iterator it;
	for (it = m_Children.begin(); it != m_Children.end(); it++) {
		GUIPanel *P = *it;
		if (P) {
			CurPanel = P->BottomPanelUnderPoint(x, y);
			if (CurPanel != nullptr) {
				return CurPanel;
			}
		}
	}
	// Return this panel
	return this;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

GUIPanel * GUIPanel::TopPanelUnderPoint(int x, int y) {
	if (!PointInside(x, y)) {
		return nullptr;
	}
	// If this panel is invisible or disabled, this panel is ignored
	if (!m_Visible || !m_Enabled) {
		return nullptr;
	}

	// Go through the children
	GUIPanel *CurPanel = nullptr;
	std::vector<GUIPanel *>::reverse_iterator it;
	for (it = m_Children.rbegin(); it != m_Children.rend(); it++) {
		GUIPanel *P = *it;
		if (P) {
			CurPanel = P->TopPanelUnderPoint(x, y);
			if (CurPanel != nullptr) {
				return CurPanel;
			}
		}
	}

	// Return this panel
	return this;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool GUIPanel::PointInside(int X, int Y) {
	// Can't be inside an invisible panel
	if (!m_Visible) {
		return false;
	}
	if (X < m_X || Y < m_Y) {
		return false;
	}
	if (X > m_X + m_Width || Y > m_Y + m_Height) {
		return false;
	}
	// Mouse is inside
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIPanel::SetSize(int Width, int Height) {
	m_Width = Width;
	m_Height = Height;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIPanel::SetPositionAbs(int X, int Y, bool moveChildren) {
	int DX = X - m_X;
	int DY = Y - m_Y;

	m_X = X;
	m_Y = Y;

	// Move children
	if (moveChildren) {
		std::vector<GUIPanel *>::iterator it;
		for (it = m_Children.begin(); it != m_Children.end(); it++) {
			GUIPanel *P = *it;
			P->SetPositionAbs(P->m_X + DX, P->m_Y + DY);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIPanel::SetPositionRel(int X, int Y) {
	X += m_Parent->GetXPos();
	Y += m_Parent->GetYPos();

	int DX = X - m_X;
	int DY = Y - m_Y;

	m_X = X;
	m_Y = Y;

	// Move children
	std::vector<GUIPanel *>::iterator it;
	for (it = m_Children.begin(); it != m_Children.end(); it++) {
		GUIPanel *P = *it;
		P->SetPositionAbs(P->m_X + DX, P->m_Y + DY);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIPanel::MoveRelative(int dX, int dY) {
	m_X += dX;
	m_Y += dY;

	// Move children
	std::vector<GUIPanel *>::iterator it;
	for (it = m_Children.begin(); it != m_Children.end(); it++) {
		GUIPanel *P = *it;
		P->SetPositionAbs(P->m_X + dX, P->m_Y + dY);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIPanel::CenterInParent(bool centerX, bool centerY) {
	int newRelX = m_X - m_Parent->GetXPos();
	int newRelY = m_Y - m_Parent->GetYPos();

	if (centerX) { newRelX = (m_Parent->GetWidth() / 2) - (GetWidth() / 2); }
	if (centerY) { newRelY = (m_Parent->GetHeight() / 2) - (GetHeight() / 2); }

	SetPositionRel(newRelX, newRelY);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIPanel::_SetVisible(bool Visible) {
	m_Visible = Visible;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool GUIPanel::_GetVisible() const {
	return m_Visible;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIPanel::_SetEnabled(bool Enabled) {
	m_Enabled = Enabled;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool GUIPanel::_GetEnabled() const {
	return m_Enabled;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int GUIPanel::GetWidth() const {
	return m_Width;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int GUIPanel::GetHeight() const {
	return m_Height;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

GUIRect * GUIPanel::GetRect() {
	SetRect(&m_Rect, m_X, m_Y, m_X + m_Width, m_Y + m_Height);

	return &m_Rect;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIPanel::GetRect(int *X, int *Y, int *Width, int *Height) const {
	if (X) { *X = m_X; }
	if (Y) { *Y = m_Y; }
	if (Width) { *Width = m_Width; }
	if (Height) { *Height = m_Height; }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int GUIPanel::GetPanelID() const {
	return m_ID;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIPanel::TrackMouseHover(bool Enabled, int Delay) {
	m_Manager->TrackMouseHover(this, Enabled, Delay);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIPanel::SetFocus() {
	m_Manager->SetFocus(this);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool GUIPanel::HasFocus() const {
	return m_GotFocus;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIPanel::SetCaptureState(bool Captured) {
	m_Captured = Captured;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool GUIPanel::IsCaptured() const {
	return m_Captured;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool GUIPanel::IsEnabled() const {
	return m_Enabled;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIPanel::SendSignal(int Code, int Data) {
	if (m_SignalTarget) { m_SignalTarget->ReceiveSignal(this, Code, Data); }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIPanel::ReceiveSignal(GUIPanel *Source, int Code, int Data) {}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIPanel::SetSignalTarget(GUIPanel *Target) {
	if (Target) { m_SignalTarget = Target; }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

GUIPanel * GUIPanel::GetParentPanel() {
	return m_Parent;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIPanel::SetZPos(int Z) {
	m_ZPos = Z;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int GUIPanel::GetZPos() const {
	return m_ZPos;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIPanel::ChangeZPosition(int Type) {
	// If we don't have a parent, get the manager to alter the Z Position
	if (!m_Parent) {
		//m_Manager->ChangeZPosition(this, Type);
		return;
	}

	// Get the parent to change the position
	m_Parent->_ChangeZ(this, Type);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIPanel::_ChangeZ(GUIPanel *Child, int Type) {
	assert(Child);

	int Index = -1;

	// Find the child in our children list
	std::vector<GUIPanel *>::iterator it;
	int Count = 0;
	for (it = m_Children.begin(); it != m_Children.end(); it++, Count++) {
		const GUIPanel *P = *it;
		if (P && P->GetPanelID() == Child->GetPanelID()) {
			Index = Count;
			break;
		}
	}

	// Didn't find the child
	if (Index == -1) {
		return;
	}

	switch (Type) {
		// Put the child at the end of the list
		case TopMost:
			m_Children.erase(m_Children.begin() + Index);
			m_Children.push_back(Child);
			break;

			// Put the child at the start of the list
		case BottomMost:
			m_Children.erase(m_Children.begin() + Index);
			m_Children.insert(m_Children.begin(), Child);
			break;
		default:
			break;
	}

	// Go through and re-order the Z positions
	Count = 0;
	for (it = m_Children.begin(); it != m_Children.end(); it++, Count++) {
		GUIPanel *P = *it;
		if (P) { P->SetZPos(Count); }
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string GUIPanel::ToString() {
	std::string OutString = "";

	// Subtract the position from the parent
	int X = m_X;
	int Y = m_Y;

	if (m_Parent) {
		X -= m_Parent->m_X;
		Y -= m_Parent->m_Y;
	}

	OutString += WriteValue("X", X);
	OutString += WriteValue("Y", Y);
	OutString += WriteValue("Width", m_Width);
	OutString += WriteValue("Height", m_Height);
	OutString += WriteValue("Visible", m_Visible);
	OutString += WriteValue("Enabled", m_Enabled);

	return OutString;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIPanel::BuildProperties(GUIProperties *Prop) {
	assert(Prop);

	// Subtract the position from the parent
	int X = m_X;
	int Y = m_Y;

	if (m_Parent) {
		X -= m_Parent->m_X;
		Y -= m_Parent->m_Y;
	}

	Prop->AddVariable("X", X);
	Prop->AddVariable("Y", Y);
	Prop->AddVariable("Width", m_Width);
	Prop->AddVariable("Height", m_Height);
	Prop->AddVariable("Visible", m_Visible);
	Prop->AddVariable("Enabled", m_Enabled);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string GUIPanel::WriteValue(const std::string &Name, int Value) {
	char buf[32];

	std::string OutString = Name;
	OutString += " = ";

	std::snprintf(buf, sizeof(buf), "%i", Value);
	OutString += buf;
	OutString += "\n";

	return OutString;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string GUIPanel::WriteValue(const std::string &Name, bool Value) {
	std::string OutString = Name;
	OutString += " = ";
	OutString += (Value ? "True" : "False");
	OutString += "\n";

	return OutString;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIPanel::_ApplyProperties(GUIProperties *Props) {
	assert(Props);

	Props->GetValue("Visible", &m_Visible);
	Props->GetValue("Enabled", &m_Enabled);
}