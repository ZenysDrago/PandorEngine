#include "pch.h"
#include <EditorUI/PrefabWindow.h>
#include <Core/App.h>
#include <Core/SceneManager.h>
#include <Core/Scene.h>
#include <Core/GameObject.h>
#include <Render/Framebuffer.h>
#include <Render/Camera.h>
#include <Resources/ResourcesManager.h>
#include <Resources/Model.h>

using namespace EditorUI;

EditorUI::PrefabWindow::PrefabWindow()
{
	p_open = false;
}

EditorUI::PrefabWindow::~PrefabWindow()
{
}

void EditorUI::PrefabWindow::Draw()
{
	Core::Scene* prefabScene = Core::App::Get().sceneManager->GetCurrentPrefabScene();

	if (!p_open)
		return;
	if (prefabScene)
	{
		if (m_visible = WrapperUI::Begin("PrefabWindow", &p_open))
		{
			m_hovered = WrapperUI::IsWindowHovered();
			m_focused = WrapperUI::IsWindowFocused();
			m_windowsize = WrapperUI::GetWindowSize() - Math::Vector2{ 16 , 35 };
			m_windowPos = WrapperUI::GetWindowPos() + Math::Vector2{ 7, 26.5 };
			auto textureId = prefabScene->GetEditorCamera()->framebuffer->GetFrameBufferTexture();
			WrapperUI::Image(textureId, m_windowsize);

			static Core::GameObject* dragged = nullptr;
			static std::string LastDragged = "";
			static Resources::Model* modelDragged = nullptr;
			// Drag n Drop Models
			if (WrapperUI::BeginDragDropTarget()) {
				if (const Payload* payload = WrapperUI::AcceptDragDropPayload("Model", DragDropFlags::AcceptBeforeDelivery)) {
					std::string PayloadIndex = *(std::string*)payload->Data;

					if (LastDragged != PayloadIndex) {
						LastDragged = PayloadIndex;
						modelDragged = Resources::ResourcesManager::Get()->GetOrLoad<Resources::Model>(PayloadIndex);
					}
					if (modelDragged && modelDragged->HasBeenSent() && !dragged) {
						auto gameObject = modelDragged->ToGameObject();
						dragged = gameObject;

					}
					if (dragged) {
						dragged->DrawSelfAndChild(true);
						auto Direction = prefabScene->GetEditorCamera()->UnProject({ GetMousePosition(), 20.f });
						dragged->transform->SetWorldPosition(Direction);
					}
					if (WrapperUI::IsMouseReleased(MouseButton::Left) && dragged)
					{
						if (auto currNode = prefabScene->GetSceneNode())
							currNode->AddChildren(dragged);
						dragged = nullptr;
						LastDragged = "";
						modelDragged = nullptr;
					}
				}
				WrapperUI::EndDragDropTarget();
			}
			else if (dragged && WrapperUI::IsMouseReleased(MouseButton::Left))
			{
				delete dragged;
				dragged = nullptr;
				LastDragged = "";
				modelDragged = nullptr;
			}
		}
		WrapperUI::End();
	}
}

Math::Vector2 EditorUI::PrefabWindow::GetMousePosition()
{
	auto mouse = WrapperUI::GetMousePos();
	return Math::Vector2(mouse.x, mouse.y) - this->GetWindowPos();
}

bool EditorUI::PrefabWindow::IsVisible()
{
	return m_visible;
}
