#include "GUI.h"

#include "GUIButton.h"
#include "GUIListBox.h"
#include "GUICheckbox.h"
#include "GUIRadioButton.h"
#include "GUITab.h"
#include "GUICollectionBox.h"
#include "GUIProgressBar.h"
#include "GUISlider.h"
#include "GUITextBox.h"
#include "GUILabel.h"
#include "GUIScrollbar.h"
#include "GUIComboBox.h"
#include "GUIPropertyPage.h"

using namespace RTE;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

GUIControl * GUIControlFactory::CreateControl(GUIManager *Manager, GUIControlManager *ControlManager, const std::string &ControlName) {
	// Button
	if (ControlName.compare(GUIButton::GetControlID()) == 0) {
		return new GUIButton(Manager, ControlManager);
	}
	// Listbox
	if (ControlName.compare(GUIListBox::GetControlID()) == 0) {
		return new GUIListBox(Manager, ControlManager);
	}
	// Checkbox
	if (ControlName.compare(GUICheckbox::GetControlID()) == 0) {
		return new GUICheckbox(Manager, ControlManager);
	}
	// RadioButton
	if (ControlName.compare(GUIRadioButton::GetControlID()) == 0) {
		return new GUIRadioButton(Manager, ControlManager);
	}
	// Tab
	if (ControlName.compare(GUITab::GetControlID()) == 0) {
		return new GUITab(Manager, ControlManager);
	}
	// CollectionBox
	if (ControlName.compare(GUICollectionBox::GetControlID()) == 0) {
		return new GUICollectionBox(Manager, ControlManager);
	}
	// ProgressBar
	if (ControlName.compare(GUIProgressBar::GetControlID()) == 0) {
		return new GUIProgressBar(Manager, ControlManager);
	}
	// Slider
	if (ControlName.compare(GUISlider::GetControlID()) == 0) {
		return new GUISlider(Manager, ControlManager);
	}
	// TextBox
	if (ControlName.compare(GUITextBox::GetControlID()) == 0) {
		return new GUITextBox(Manager, ControlManager);
	}
	// Label
	if (ControlName.compare(GUILabel::GetControlID()) == 0) {
		return new GUILabel(Manager, ControlManager);
	}
	// Scrollbar
	if (ControlName.compare(GUIScrollbar::GetControlID()) == 0) {
		return new GUIScrollbar(Manager, ControlManager);
	}
	// ComboBox
	if (ControlName.compare(GUIComboBox::GetControlID()) == 0) {
		return new GUIComboBox(Manager, ControlManager);
	}
	// PropertyPage
	if (ControlName.compare(GUIPropertyPage::GetControlID()) == 0) {
		return new GUIPropertyPage(Manager, ControlManager);
	}
	// Not Found
	return 0;
}