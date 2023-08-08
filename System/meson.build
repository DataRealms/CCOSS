sources += files(
'MicroPather/micropather.cpp',
'glad/gl.c',
'StandardIncludes.cpp',
'Atom.cpp',
'ContentFile.cpp',
'Controller.cpp',
'GenericSavedData.cpp',
'GraphicalPrimitive.cpp',
'Writer.cpp',
'Box.cpp',
'Entity.cpp',
'Vector.cpp',
'Reader.cpp',
'Color.cpp',
'InputScheme.cpp',
'RTETools.cpp',
'System.cpp',
'InputMapping.cpp',
'PathFinder.cpp',
'DataModule.cpp',
'Timer.cpp',
'RTEError.cpp',
'Matrix.cpp',
#'ScreenShader.cpp',
'Shader.cpp',
'Serializable.cpp',
'PieQuadrant.cpp',
'GLCheck.cpp',
'SpatialPartitionGrid.cpp',
)

imgui_sources = files(
'imgui/imgui.cpp',
'imgui/imgui_demo.cpp',
'imgui/imgui_draw.cpp',
'imgui/imgui_impl_opengl3.cpp',
'imgui/imgui_impl_sdl2.cpp',
'imgui/imgui_impl_sdlrenderer2.cpp',
'imgui/imgui_tables.cpp',
'imgui/imgui_widgets.cpp',
)

imugui_include_dirs = include_directories('.', 'imgui')

imgui = static_library('imgui', imgui_sources, dependencies: deps, include_directories:imugui_include_dirs)
