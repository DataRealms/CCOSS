set(SOURCES AreaEditorGUI.cpp
AreaEditorGUI.h
AreaPickerGUI.cpp
AreaPickerGUI.h
AssemblyEditorGUI.cpp
AssemblyEditorGUI.h
BuyMenuGUI.cpp
BuyMenuGUI.h
GibEditorGUI.cpp
GibEditorGUI.h
MainMenuGUI.cpp
MainMenuGUI.h
MetagameGUI.cpp
MetagameGUI.h
ObjectPickerGUI.cpp
ObjectPickerGUI.h
PieMenuGUI.cpp
PieMenuGUI.h
SceneEditorGUI.cpp
SceneEditorGUI.h
ScenarioGUI.cpp
ScenarioGUI.h)

complete_path(${SOURCES})
set(Menus_SRC ${RESULT} PARENT_SCOPE)
source_group(Menus FILES ${RESULT})

