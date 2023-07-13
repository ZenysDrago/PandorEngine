#include "pch.h"
#include <EditorUI/GameWindow.h>
#include <Core/App.h>
#include <Core/Scene.h>
#include <Core/SceneManager.h>
#include <Render/Camera.h>
#include <Render/Framebuffer.h>

EditorUI::GameWindow::GameWindow()
{
}

EditorUI::GameWindow::~GameWindow()
{
}

void EditorUI::GameWindow::Draw()
{
	if (!p_open)
		return;
	if (m_visible = WrapperUI::Begin("GameWindow", &p_open))
	{
		if (auto mainCamera = Core::App::Get().sceneManager->GetCurrentScene()->mainCamera) {
			m_size = WrapperUI::GetWindowSize();
			m_size = { m_size.x - 16.f, m_size.y - 36.f };
			WrapperUI::Image(Core::App::Get().sceneManager->GetCurrentScene()->mainCamera->framebuffer->GetFrameBufferTexture(), m_size);
		}
	}
	WrapperUI::End();
}

bool EditorUI::GameWindow::IsVisible()
{
	return m_visible;
}
