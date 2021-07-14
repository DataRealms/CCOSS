#include "GUI.h"

using namespace RTE;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

GUIControl::GUIControl() {
	m_Skin = nullptr;
	m_SkinPreset = 1;
	m_Properties.Clear();
	m_ControlChildren.clear();
	m_ControlParent = nullptr;
	m_IsContainer = false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIControl::Create(const std::string &Name, int X, int Y, int Width, int Height) {
	m_Properties.Clear();
	m_Properties.AddVariable("Name", Name);
	m_Properties.AddVariable("Anchor", "Left, Top");
	m_Properties.AddVariable("ToolTip", "");
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIControl::Create(GUIProperties *Props) {
	assert(Props);

	// Add the default variables
	m_Properties.AddVariable("Name", "");
	m_Properties.AddVariable("Anchor", "Left, Top");
	m_Properties.AddVariable("ToolTip", "");

	m_Properties.Update(Props);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIControl::Destroy() {}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIControl::Activate() {}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIControl::ChangeSkin(GUISkin *Skin) {
	m_Skin = Skin;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIControl::AddEvent(int Type, int Msg, int Data) {
	m_ControlManager->AddEvent(new GUIEvent(this, Type, Msg, Data));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string GUIControl::GetName() {
	std::string Name;
	m_Properties.GetValue("Name", &Name);

	return Name;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string GUIControl::GetToolTip() {
	std::string tip;
	m_Properties.GetValue("ToolTip", &tip);

	return tip;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string GUIControl::GetID() const {
	return m_ControlID;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

GUIPanel * GUIControl::GetPanel() {
	return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIControl::AddChild(GUIControl *Control) {
	assert(Control);

	// Remove the control from any previous parent
	if (Control->GetParent()) { Control->GetParent()->GUIControl::RemoveChild(Control->GetName()); }

	Control->m_ControlParent = this;
	m_ControlChildren.push_back(Control);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<GUIControl *> * GUIControl::GetChildren() {
	return &m_ControlChildren;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool GUIControl::Save(GUIWriter *W) {
	std::string OutString = "";
	std::string Name;

	// Get the control to store its properties
	StoreProperties();

	// Section Header
	m_Properties.GetValue("Name", &Name);

	OutString.append("[");
	OutString.append(Name);
	OutString.append("]\n");

	// General control values
	OutString.append("ControlType = ");
	OutString += m_ControlID;
	OutString.append("\n");

	// Parent
	OutString += "Parent = ";
	if (m_ControlParent) {
		OutString += m_ControlParent->GetName();
	} else {
		OutString += "None";
	}
	OutString += "\n";

	// Get the main panel and write its location
	GUIPanel *Pan = GetPanel();
	if (Pan) { OutString.append(Pan->ToString()); }

	// Write out the properties
	OutString.append(m_Properties.ToString());

	// Write to the writer class
	*W << OutString;

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIControl::Move(int X, int Y) {}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIControl::Resize(int Width, int Height) {}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIControl::GetControlRect(int *X, int *Y, int *Width, int *Height) {
	// Zero the values for controls that don't override this
	if (X) { *X = 0; }
	if (Y) { *Y = 0; }
	if (Width) { *Width = 0; }
	if (Height) { *Height = 0; }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int GUIControl::GetAnchor() {
	int Anchor = 0;
	std::string Value[4];

	int Count = m_Properties.GetValue("Anchor", Value, 4);

	for (int i = 0; i < Count; i++) {
		if (stricmp(Value[i].c_str(), "left") == 0) { Anchor |= Anchor_Left; }
		if (stricmp(Value[i].c_str(), "top") == 0) { Anchor |= Anchor_Top; }
		if (stricmp(Value[i].c_str(), "right") == 0) { Anchor |= Anchor_Right; }
		if (stricmp(Value[i].c_str(), "bottom") == 0) { Anchor |= Anchor_Bottom; }
	}

	// The anchor cannot have both sides missing, so we default to Left, Top is that is the case
	if (!(Anchor & Anchor_Left) && !(Anchor & Anchor_Right)) { Anchor |= Anchor_Left; }
	if (!(Anchor & Anchor_Top) && !(Anchor & Anchor_Bottom)) { Anchor |= Anchor_Top; }

	return Anchor;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIControl::StoreProperties() {}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIControl::SetVisible(bool Visible) {
	// Default method is the grab the main panel and directly set its state. Controls that use multiple panels on the same layer will need to override this function
	GUIPanel *Panel = GetPanel();
	if (Panel) { Panel->_SetVisible(Visible); }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool GUIControl::GetVisible() {
	// See SetVisible() comment
	GUIPanel *Panel = GetPanel();
	if (Panel) {
		return Panel->_GetVisible();
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIControl::SetEnabled(bool Enabled) {
	// See SetVisible() comment
	GUIPanel *Panel = GetPanel();
	if (Panel) { Panel->_SetEnabled(Enabled); }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool GUIControl::GetEnabled() {
	// See SetVisible() comment
	GUIPanel *Panel = GetPanel();
	if (Panel) { return Panel->_GetEnabled(); }

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

GUIControl * GUIControl::GetParent() {
	return m_ControlParent;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

GUIProperties * GUIControl::GetProperties() {
	return &m_Properties;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIControl::ApplyProperties(GUIProperties *Props) {
	assert(Props);

	m_Properties.Update(Props);

	int X, Y;
	int Width, Height;
	bool Enabled;
	bool Visible;
	Props->GetValue("X", &X);
	Props->GetValue("Y", &Y);
	Props->GetValue("Width", &Width);
	Props->GetValue("Height", &Height);
	Props->GetValue("Enabled", &Enabled);
	Props->GetValue("Visible", &Visible);

	// Adjust position from parent
	GUIPanel *P = GetPanel();
	if (P && P->GetParentPanel()) {
		int px;
		int py;
		int pw;
		int ph;
		P->GetParentPanel()->GetRect(&px, &py, &pw, &ph);
		X += px;
		Y += py;
	}

	SetEnabled(Enabled);

	Move(X, Y);
	Resize(Width, Height);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool GUIControl::IsContainer() {
	return m_IsContainer;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIControl::RemoveChild(const std::string Name) {
	// Note: We do NOT free the children because they are still linked in through their panels. This merely removes the control from the list.
	// This will cause a small memory leak, but this is only designed for the GUI Editor and is a bit of a hack.
	std::vector<GUIControl *>::iterator it;

	for (it = m_ControlChildren.begin(); it != m_ControlChildren.end(); it++) {
		GUIControl *C = *it;
		if (C && C->GetName().compare(Name) == 0) {
			m_ControlChildren.erase(it);
			break;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIControl::RemoveChildren() {
	// Note: We do NOT free the children because they are still linked in through their panels. This merely removes the control from the list.
	// This will cause a small memory leak, but this is only designed for the GUI Editor and is a bit of a hack.
	std::vector<GUIControl *>::iterator it;

	for (it = m_ControlChildren.begin(); it != m_ControlChildren.end(); it++) {
		GUIControl *C = *it;
		if (C) { m_ControlManager->RemoveControl(C->GetName(), false); }
	}

	m_ControlChildren.clear();
}
