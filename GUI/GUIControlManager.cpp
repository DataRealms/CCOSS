//////////////////////////////////////////////////////////////////////////////////////////
// File:            GUIControlManager.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     GUIControlManager class
// Project:         GUI Library
// Author(s):       Jason Boettcher
//                  jackal@shplorb.com
//                  www.shplorb.com/~jackal


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "GUI.h"

using namespace RTE;

//////////////////////////////////////////////////////////////////////////////////////////
// Constructor:     GUIControlmanager
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Constructor method used to instantiate a GUIControlManager object in
//                  system memory.

GUIControlManager::GUIControlManager()
{
    m_Screen = 0;
    m_Input = 0;
    m_Skin = 0;
    m_GUIManager = 0;
    m_ControlList.clear();
    m_EventQueue.clear();

    m_CursorType = Pointer;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Creates the data for the control manager

bool GUIControlManager::Create(GUIScreen *Screen, GUIInput *Input, const std::string SkinDir, const std::string SkinFilename)
{
    assert(Screen && Input);
    
    m_Screen = Screen;
    m_Input = Input;

    // Create the skin
    m_Skin = new GUISkin(Screen);
    if (!m_Skin)
        return false;

    // Load the skin
    if (!m_Skin->Load(SkinDir, SkinFilename)) {
        delete m_Skin;
        m_Skin = 0;
        return false;
    }

    // Create the GUI manager
    m_GUIManager = new GUIManager(Input);
    if (!m_GUIManager)
        return false;

    return true;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Frees all the allocated resources.

void GUIControlManager::Destroy(void)
{
    // Free the skin
    if (m_Skin) {
        m_Skin->Destroy();
        delete m_Skin;
        m_Skin = 0;
    }
    
    // Destroy the controls & event queue
    Clear();

    // Free the GUI manager
    if (m_GUIManager) {
        delete m_GUIManager;
        m_GUIManager = 0;
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the controls.

void GUIControlManager::Clear(void)
{
    vector<GUIControl *>::iterator it;

    // Destroy every control
    for(it = m_ControlList.begin(); it != m_ControlList.end(); it++) {
        GUIControl *C = *it;

        C->Destroy();
        delete C;
    }

    m_ControlList.clear();

    m_GUIManager->Clear();

    // Destroy the event queue
    vector<GUIEvent *>::iterator ite;
    for(ite = m_EventQueue.begin(); ite != m_EventQueue.end(); ite++) {
        GUIEvent *E = *ite;
        if (E)
            delete E;
    }

    m_EventQueue.clear();
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ChangeSkin
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Changes the skin of the controls.

void GUIControlManager::ChangeSkin(const string SkinDir, const std::string SkinFilename)
{
    vector<GUIControl *>::iterator it;

    m_Skin->Destroy();
    m_Skin->Load(SkinDir, SkinFilename);

    // Go through every control and change its skin
    for(it = m_ControlList.begin(); it != m_ControlList.end(); it++) {
        GUIControl *C = *it;

        C->ChangeSkin(m_Skin);
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AddControl
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Manually creates a control

GUIControl *GUIControlManager::AddControl(const std::string Name, const std::string Type,
                                          GUIControl *Parent,
                                          int X, int Y, 
                                          int Width, int Height)
{
    // Skip if we already have a control of this name
    if (GetControl(Name))
        return 0;

    // Create the control
    GUIControl *Control = GUIControlFactory::CreateControl(m_GUIManager, this, Type);
    if (!Control)
        return 0;

    Control->Create(Name, X, Y, Width, Height);
    Control->ChangeSkin(m_Skin);

    GUIPanel *Pan = 0;
    if (Parent) {
        Pan = Parent->GetPanel();
        Parent->AddChild(Control);
    }
    if (Pan)
        Pan->AddChild(Control->GetPanel());
    else
        m_GUIManager->AddPanel(Control->GetPanel());

    // Add the control to the list
    m_ControlList.push_back(Control);

    // Ready
    Control->Activate();

    return Control;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AddControl
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Manually creates a control.

GUIControl *GUIControlManager::AddControl(GUIProperties *Property)
{
    assert(Property);

    // Get the control type and name
    string Type;
    Property->GetValue("ControlType", &Type);
    string Name;
    Property->GetValue("Name", &Name);

    // Skip if we already have a control of this name
    if (GetControl(Name))
        return 0;

    // Create the control
    GUIControl *Control = GUIControlFactory::CreateControl(m_GUIManager, this, Type);
    if (!Control)
        return 0;

    Control->Create(Property);
    Control->ChangeSkin(m_Skin);

    // Get the parent control
    string Parent;
    Property->GetValue("Parent", &Parent);
    
    GUIControl *Par = GetControl(Parent);
    GUIPanel *Pan = 0;
    if (Par && Parent.compare("None") != 0) {
        Pan = Par->GetPanel();
        Par->AddChild(Control);
    }

    if (Pan)
        Pan->AddChild(Control->GetPanel());
    else
        m_GUIManager->AddPanel(Control->GetPanel());

    // Add the control to the list
    m_ControlList.push_back(Control);

    // Ready
    Control->Activate();

    return Control;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetControl
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets a control

GUIControl *GUIControlManager::GetControl(const std::string& Name)
{
    std::vector<GUIControl *>::iterator it;

    for(it = m_ControlList.begin(); it != m_ControlList.end(); it++) {
        GUIControl *C = *it;
        if (C->GetName().compare(Name) == 0)
            return C;
    }

    // Not found
    return 0;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetControlList
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the control list

vector<GUIControl *> *GUIControlManager::GetControlList(void)
{
    return &m_ControlList;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          ControlUnderPoint
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Checks if a control is under a specific point

GUIControl * GUIControlManager::GetControlUnderPoint(int pointX, int pointY, GUIControl *pParent, int depth)
{
    // Default to the root object if no parent specified
    if (!pParent)
        pParent = m_ControlList.front();
    if (!pParent)
        return 0;

    // Clicked on the parent?
    int X, Y, Width, Height;
    pParent->GetControlRect(&X, &Y, &Width, &Height);

    if (pointX < X || pointX > X+Width)
        return NULL;

    if (pointY < Y || pointY > Y+Height)
        return NULL;

    // Check children
    vector<GUIControl *> *List = pParent->GetChildren();
    vector<GUIControl *>::reverse_iterator it;

    assert(List);

    // Control the depth. If negative, it'll go forever
    if (depth != 0)
    {
        for (it = List->rbegin(); it != List->rend(); it++)
        {
            // Only check visible controls
            if ((*it)->GetVisible())
            {
                GUIControl *C = GetControlUnderPoint(pointX, pointY, *it, depth - 1);
                if (C)
                    return C;
            }
        }
    }

    // If not asked to search for the root object, return the parent if point is on it
    return pParent == m_ControlList.front() ? 0 : pParent;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          RemoveControl
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Removes a control by name

void GUIControlManager::RemoveControl(const string Name, bool RemoveFromParent)
{
    // NOTE: We can't simply remove it because some controls need to remove extra panels
    //         And it's silly to add 'remove' to every control to remove their extra panels
    //         (IE, Combobox).
    //         Signals and stuff are also linked in
    //
    //         So we just remove the controls from the list and not from memory


    std::vector<GUIControl *>::iterator it;

    for(it = m_ControlList.begin(); it != m_ControlList.end(); it++) {
        GUIControl *C = *it;
        if (C->GetName().compare(Name) == 0) {

            // Just remove it from the list
            C->SetVisible(false);
            m_ControlList.erase(it);

            // Remove all my children
            C->RemoveChildren();

            // Remove me from my parent
            if (C->GetParent() && RemoveFromParent)
                C->GetParent()->RemoveChild(Name);

            break;
        }
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the GUI every frame

void GUIControlManager::Update(bool ignoreKeyboardEvents)
{
    // Clear the event queue
    m_EventQueue.clear();

    // Process the manager
    m_GUIManager->Update(ignoreKeyboardEvents);

}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws the GUI to the back buffer.

void GUIControlManager::Draw(void)
{
    m_GUIManager->Draw(m_Screen);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws the GUI to the back buffer.

void GUIControlManager::Draw(GUIScreen *pScreen)
{
    m_GUIManager->Draw(pScreen);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          DrawMouse
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws the mouse.

void GUIControlManager::DrawMouse(void)
{
    int MouseX, MouseY;
    m_Input->GetMousePosition(&MouseX, &MouseY);

    switch(m_CursorType) {
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
    }
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetEvent
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets an event from the queue.

bool GUIControlManager::GetEvent(GUIEvent *Event)
{
    if (Event && !m_EventQueue.empty()) {

        // Copy the event
        *Event = *m_EventQueue.back();
        
        // Free the event
        GUIEvent *ptr = m_EventQueue.at(m_EventQueue.size()-1);
        if (ptr)
            delete ptr;

        m_EventQueue.pop_back();
        return true;
    }

    // Empty queue OR null Event pointer
    return false;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          AddEvent
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Add a new event to the queue.

void GUIControlManager::AddEvent(GUIEvent *Event)
{
    // Add the event to the queue
    if (Event)
        m_EventQueue.push_back(Event);
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetCursor
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the cursor type.

void GUIControlManager::SetCursor(int CursorType)
{
    m_CursorType = CursorType;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the layout to a file.

bool GUIControlManager::Save(const std::string Filename)
{
    Writer W;
    if (W.Create((char *)Filename.c_str()) != 0)
        return false;

    bool Result = Save(&W);
    
    W.EndWrite();

    return Result;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Save
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Saves the layout to a Writer class.

bool GUIControlManager::Save(Writer *W)
{
    assert(W);

    // Go through each control
    vector<GUIControl *>::iterator it;

    for(it = m_ControlList.begin(); it != m_ControlList.end(); it++) {
        GUIControl *C = *it;
        C->Save(W);
        // Separate controls by one line
        W->NewLine();
    }

    return true;
}


//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Load
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Loads the layout from a Reader class.
// Arguments:       Reader class
// Returns:         True if sucessful.

bool GUIControlManager::Load(const std::string &Filename, bool keepOld) {
	Reader reader;
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
    for(it = ControlList.begin(); it != ControlList.end(); it++) {
        GUIProperties *Prop = *it;
        AddControl(Prop);
        // Free the property class
        delete Prop;
    }

    return true;
}
