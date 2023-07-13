#include "pch.h"
#include <EditorUI/CanvasWindow.h>
#include <EditorUI/SceneWindow.h>
#include <EditorUI/EditorUIManager.h>
#include <ImGui/imgui.h>
#include <Core/App.h>
#include <Render/Camera.h>

EditorUI::CanvasWindow::CanvasWindow()
{
	p_open = false;
}

EditorUI::CanvasWindow::~CanvasWindow()
{
}

void EditorUI::CanvasWindow::Draw()
{
	if (!p_open)
		return;
	if (WrapperUI::Begin("Canvas Window"), &p_open)
	{
		WrapperUI::DrawCanvas(Core::App::Get().GetEditorUIManager().GetSceneWindow().GetWindowSize(), m_canvas);
	}
	WrapperUI::End();
}
