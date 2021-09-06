#ifndef _GUICONTROLFACTORY_
#define _GUICONTROLFACTORY_

namespace RTE {

/// <summary>
/// A class used to create the different controls based on name.
/// </summary>
class GUIControlFactory {

public:

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          CreateControl
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Method used for creating controls
// Arguments:       Control Type Name.

    static GUIControl *CreateControl(GUIManager *Manager,  GUIControlManager *ControlManager, const std::string &ControlName);

};
};
#endif