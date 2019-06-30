set(SOURCES 
AllegroBitmap.cpp
AllegroBitmap.h
AllegroInput.cpp
AllegroInput.h
AllegroScreen.cpp
AllegroScreen.h
GUI.cpp
GUIBanner.h
GUIBanner.cpp
GUIBitmap.h
GUIButton.cpp
GUIButton.h
GUICheckbox.cpp
GUICheckbox.h
GUICollectionBox.cpp
GUICollectionBox.h
GUIComboBox.cpp
GUIComboBox.h
GUIControl.cpp
GUIControl.h
GUIControlFactory.cpp
GUIControlFactory.h
GUIControlManager.cpp
GUIControlManager.h
GUIEvent.cpp
GUIEvent.h
GUIFont.cpp
GUIFont.h
GUIInput.cpp
GUIInput.h
GUILabel.cpp
GUILabel.h
GUIListBox.cpp
GUIListBox.h
GUIListPanel.cpp
GUIListPanel.h
GUIManager.cpp
GUIManager.h
GUIPanel.cpp
GUIPanel.h
GUIProgressBar.cpp
GUIProgressBar.h
GUIProperties.cpp
GUIProperties.h
GUIPropertyPage.cpp
GUIPropertyPage.h
GUIRadioButton.cpp
GUIRadioButton.h
GUIScreen.h
GUIScrollPanel.cpp
GUIScrollPanel.h
GUIScrollbar.cpp
GUIScrollbar.h
GUISkin.cpp
GUISkin.h
GUISlider.cpp
GUISlider.h
GUITab.cpp
GUITab.h
GUITextBox.cpp
GUITextBox.h
GUITextPanel.cpp
GUITextPanel.h
GUIUtil.cpp
GUIUtil.h)

IF(APPLE) 
	LIST(APPEND SOURCES OsxUtil.h)
	LIST(APPEND SOURCES OsxUtil.cpp)
ENDIF(APPLE)


complete_path(${SOURCES})
set(GUI_SRC ${RESULT} PARENT_SCOPE)
source_group(GUI FILES ${RESULT})

