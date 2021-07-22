#include "GUI.h"

using namespace RTE;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

GUIControlManager::GUIControlManager() {
	m_Screen = nullptr;
	m_Input = nullptr;
	m_Skin = nullptr;
	m_GUIManager = nullptr;
	m_ControlList.clear();
	m_EventQueue.clear();

	m_CursorType = Pointer;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool GUIControlManager::Create(GUIScreen *Screen, GUIInput *Input, const std::string &SkinDir, const std::string &SkinFilename) {
	assert(Screen && Input);

	m_Screen = Screen;
	m_Input = Input;

	// Create the skin
	m_Skin = new GUISkin(Screen);
	if (!m_Skin) {
		return false;
	}

	// Load the skin
	if (!m_Skin->Load(SkinDir, SkinFilename)) {
		delete m_Skin;
		m_Skin = 0;
		return false;
	}

	// Create the GUI manager
	m_GUIManager = new GUIManager(Input);
	if (!m_GUIManager) {
		return false;
	}
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIControlManager::Destroy() {
	// Free the skin
	if (m_Skin) {
		m_Skin->Destroy();
		delete m_Skin;
		m_Skin = nullptr;
	}

	// Destroy the controls & event queue
	Clear();

	// Free the GUI manager
	if (m_GUIManager) {
		delete m_GUIManager;
		m_GUIManager = nullptr;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIControlManager::Clear() {
	std::vector<GUIControl *>::iterator it;

	// Destroy every control
	for (it = m_ControlList.begin(); it != m_ControlList.end(); it++) {
		GUIControl *C = *it;

		C->Destroy();
		delete C;
	}

	m_ControlList.clear();

	m_GUIManager->Clear();

	// Destroy the event queue
	std::vector<GUIEvent *>::iterator ite;
	for (ite = m_EventQueue.begin(); ite != m_EventQueue.end(); ite++) {
		GUIEvent *E = *ite;
		if (E) { delete E; }
	}
	m_EventQueue.clear();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIControlManager::ChangeSkin(const std::string &SkinDir, const std::string &SkinFilename) {
	std::vector<GUIControl *>::iterator it;

	m_Skin->Destroy();
	m_Skin->Load(SkinDir, SkinFilename);

	// Go through every control and change its skin
	for (it = m_ControlList.begin(); it != m_ControlList.end(); it++) {
		GUIControl *C = *it;

		C->ChangeSkin(m_Skin);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

GUIControl * GUIControlManager::AddControl(const std::string &Name, const std::string &Type, GUIControl *Parent, int X, int Y, int Width, int Height) {
	// Skip if we already have a control of this name
	if (GetControl(Name)) {
		return nullptr;
	}

	// Create the control
	GUIControl *Control = GUIControlFactory::CreateControl(m_GUIManager, this, Type);
	if (!Control) {
		return nullptr;
	}

	Control->Create(Name, X, Y, Width, Height);
	Control->ChangeSkin(m_Skin);

	GUIPanel *Pan = nullptr;
	if (Parent) {
		Pan = Parent->GetPanel();
		Parent->AddChild(Control);
	}
	if (Pan) {
		Pan->AddChild(Control->GetPanel());
	} else {
		m_GUIManager->AddPanel(Control->GetPanel());
	}
	// Add the control to the list
	m_ControlList.push_back(Control);

	// Ready
	Control->Activate();

	return Control;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

GUIControl * GUIControlManager::AddControl(GUIProperties *Property) {
	assert(Property);

	// Get the control type and name
	std::string Type;
	Property->GetValue("ControlType", &Type);
	std::string Name;
	Property->GetValue("Name", &Name);

	// Skip if we already have a control of this name
	if (GetControl(Name)) {
		return nullptr;
	}
	// Create the control
	GUIControl *Control = GUIControlFactory::CreateControl(m_GUIManager, this, Type);
	if (!Control) {
		return nullptr;
	}

	Control->Create(Property);
	Control->ChangeSkin(m_Skin);

	// Get the parent control
	std::string Parent;
	Property->GetValue("Parent", &Parent);

	GUIControl *Par = GetControl(Parent);
	GUIPanel *Pan = nullptr;
	if (Par && Parent.compare("None") != 0) {
		Pan = Par->GetPanel();
		Par->AddChild(Control);
	}

	if (Pan) {
		Pan->AddChild(Control->GetPanel());
	} else {
		m_GUIManager->AddPanel(Control->GetPanel());
	}

	// Add the control to the list
	m_ControlList.push_back(Control);

	// Ready
	Control->Activate();

	return Control;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

GUIControl * GUIControlManager::GetControl(const std::string &Name) {
	std::vector<GUIControl *>::iterator it;

	for (it = m_ControlList.begin(); it != m_ControlList.end(); it++) {
		GUIControl *C = *it;
		if (C->GetName().compare(Name) == 0) {
			return C;
		}
	}

	// Not found
	return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::vector<GUIControl *> * GUIControlManager::GetControlList() {
	return &m_ControlList;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

GUIControl * GUIControlManager::GetControlUnderPoint(int pointX, int pointY, GUIControl *pParent, int depth) {
	// Default to the root object if no parent specified
	if (!pParent) { pParent = m_ControlList.front(); }
	if (!pParent) {
		return nullptr;
	}

	// Clicked on the parent?
	int X;
	int Y;
	int Width;
	int Height;
	pParent->GetControlRect(&X, &Y, &Width, &Height);

	if (pointX < X || pointX > X + Width) {
		return nullptr;
	}

	if (pointY < Y || pointY > Y + Height) {
		return nullptr;
	}

	// Check children
	std::vector<GUIControl *> *List = pParent->GetChildren();
	std::vector<GUIControl *>::reverse_iterator it;

	assert(List);

	// Control the depth. If negative, it'll go forever
	if (depth != 0) {
		for (it = List->rbegin(); it != List->rend(); it++) {
			// Only check visible controls
			if ((*it)->GetVisible()) {
				GUIControl *C = GetControlUnderPoint(pointX, pointY, *it, depth - 1);
				if (C) {
					return C;
				}
			}
		}
	}

	// If not asked to search for the root object, return the parent if point is on it
	return pParent == m_ControlList.front() ? nullptr : pParent;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIControlManager::RemoveControl(const std::string &Name, bool RemoveFromParent) {
	// NOTE: We can't simply remove it because some controls need to remove extra panels and it's silly to add 'remove' to every control to remove their extra panels (ie. Combobox).
	// Signals and stuff are also linked in so we just remove the controls from the list and not from memory.
	std::vector<GUIControl *>::iterator it;

	for (it = m_ControlList.begin(); it != m_ControlList.end(); it++) {
		GUIControl *C = *it;
		if (C->GetName().compare(Name) == 0) {

			// Just remove it from the list
			C->SetVisible(false);
			m_ControlList.erase(it);

			// Remove all my children
			C->RemoveChildren();

			// Remove me from my parent
			if (C->GetParent() && RemoveFromParent) { C->GetParent()->RemoveChild(Name); }

			break;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIControlManager::Update(bool ignoreKeyboardEvents) {
	// Clear the event queue
	m_EventQueue.clear();

	// Process the manager
	m_GUIManager->Update(ignoreKeyboardEvents);

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIControlManager::Draw() {
	m_GUIManager->Draw(m_Screen);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIControlManager::Draw(GUIScreen *pScreen) {
	m_GUIManager->Draw(pScreen);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIControlManager::DrawMouse() {
	int MouseX;
	int MouseY;
	m_Input->GetMousePosition(&MouseX, &MouseY);

	switch (m_CursorType) {
		// Pointer
		case Pointer:
			m_Skin->DrawMouse(0, MouseX, MouseY);
			break;

			// Text
		case Text:
			m_Skin->DrawMouse(1, MouseX, MouseY);
			break;

			// Horizontal Resize
		case HorSize:
			m_Skin->DrawMouse(2, MouseX, MouseY);
			break;
		default:
			break;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool GUIControlManager::GetEvent(GUIEvent *Event) {
	if (Event && !m_EventQueue.empty()) {

		// Copy the event
		*Event = *m_EventQueue.back();

		// Free the event
		if (GUIEvent *ptr = m_EventQueue.at(m_EventQueue.size() - 1)) { delete ptr; }

		m_EventQueue.pop_back();
		return true;
	}
	// Empty queue OR null Event pointer
	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIControlManager::AddEvent(GUIEvent *Event) {
	// Add the event to the queue
	if (Event) { m_EventQueue.push_back(Event); }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIControlManager::SetCursor(int CursorType) {
	m_CursorType = CursorType;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool GUIControlManager::Save(const std::string &Filename) {
	GUIWriter W;
	if (W.Create(Filename) != 0) {
		return false;
	}
	bool Result = Save(&W);

	W.EndWrite();

	return Result;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool GUIControlManager::Save(GUIWriter *W) {
	assert(W);

	// Go through each control
	std::vector<GUIControl *>::iterator it;

	for (it = m_ControlList.begin(); it != m_ControlList.end(); it++) {
		GUIControl *C = *it;
		C->Save(W);
		// Separate controls by one line
		W->NewLine();
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool GUIControlManager::Load(const std::string &Filename, bool keepOld) {
	GUIReader reader;
	if (reader.Create(Filename.c_str()) != 0) {
		return false;
	}

	// Clear the current layout, IF directed to
	if (!keepOld) { Clear(); }

	std::vector<GUIProperties *> ControlList;
	ControlList.clear();

	GUIProperties *CurProp = nullptr;

	while (!reader.GetStream()->eof()) {
		std::string line = reader.ReadLine();

		if (line.empty()) {
			continue;
		}

		// Is the line a section?
		if (line.front() == '[' && line.back() == ']') {
			GUIProperties *p = new GUIProperties(line.substr(1, line.size() - 2));
			CurProp = p;
			ControlList.push_back(p);
			continue;
		}

		// Is the line a valid property?
		size_t Position = line.find_first_of('=');
		if (Position != std::string::npos) {
			// Break the line into variable & value, but only add a property if it belongs to a section
			if (CurProp) {
				// Grab the variable & value strings and trim them
				std::string Name = reader.TrimString(line.substr(0, Position));
				std::string Value = reader.TrimString(line.substr(Position + 1, std::string::npos));

				// Add it to the current property
				CurProp->AddVariable(Name, Value);
			}
			continue;
		}
	}

	// Go through each control item and create it
	std::vector<GUIProperties *>::iterator it;
	for (it = ControlList.begin(); it != ControlList.end(); it++) {
		GUIProperties *Prop = *it;
		AddControl(Prop);
		// Free the property class
		delete Prop;
	}

	return true;
}