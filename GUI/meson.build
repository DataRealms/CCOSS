sources += files(
'AllegroBitmap.cpp',
'AllegroInput.cpp',
'AllegroScreen.cpp',
'GUIBanner.cpp',
'GUIButton.cpp',
'GUICheckbox.cpp',
'GUICollectionBox.cpp',
'GUIComboBox.cpp',
'GUIControl.cpp',
'GUIControlFactory.cpp',
'GUIControlManager.cpp',
'GUIEvent.cpp',
'GUIFont.cpp',
'GUIInput.cpp',
'GUILabel.cpp',
'GUIListBox.cpp',
'GUIListPanel.cpp',
'GUIManager.cpp',
'GUIPanel.cpp',
'GUIProgressBar.cpp',
'GUIProperties.cpp',
'GUIPropertyPage.cpp',
'GUIRadioButton.cpp',
'GUIScrollbar.cpp',
'GUIScrollPanel.cpp',
'GUISkin.cpp',
'GUISlider.cpp',
'GUISound.cpp',
'GUITab.cpp',
'GUITextBox.cpp',
'GUITextPanel.cpp',
'GUIUtil.cpp'
)

if build_machine.system() == 'windows'
  sources+=files('WinUtil.cpp')
endif

if build_machine.system() == 'linux'
  sources+=files('LinuxUtil.cpp')
endif
