#include "pch.h"

#include <EditorUI/SceneWindow.h>
#include <Core/App.h>
#include <Core/SceneManager.h>
#include <Core/Scene.h>
#include <Core/GameObject.h>
#include <Render/Framebuffer.h>
#include <Render/Camera.h>
#include <Resources/ResourcesManager.h>
#include <Resources/Model.h>
#include <Resources/Prefab.h>
#include <EditorUI/File.h>

using namespace Core::Wrapper;
void EditorUI::SceneWindow::Draw()
{
	if (!p_open)
		return;
	if (m_visible = WrapperUI::Begin("SceneWindow", &p_open))
	{
		m_hovered = WrapperUI::IsWindowHovered();
		m_focused = WrapperUI::IsWindowFocused();
		m_windowsize = WrapperUI::GetWindowSize() - Math::Vector2{ 16 , 35 };
		m_windowPos = WrapperUI::GetWindowPos() + Math::Vector2{ 7, 26.5 };
		auto textureId = Core::SceneManager::Get()->GetCurrentScene()->GetEditorCamera()->framebuffer->GetFrameBufferTexture();
		WrapperUI::Image(textureId, m_windowsize);

		static Core::GameObject* dragged = nullptr;
		static std::string LastDragged = "";
		static Resources::Model* modelDragged = nullptr;
		static Resources::Prefab* prefabDragged = nullptr;
		static bool isAModel;
		static bool loaded = false;
		// Drag n Drop Models
		if (WrapperUI::BeginDragDropTarget()) {
			if (const Payload* payload = WrapperUI::AcceptDragDropPayload("File", DragDropFlags::AcceptBeforeDelivery)) {
				std::pair<std::string, FileType> PayloadIndex = *(std::pair<std::string, FileType>*)payload->Data;

				if (LastDragged != PayloadIndex.first) {
					LastDragged = PayloadIndex.first;
					loaded = false;
					if (PayloadIndex.second == FileType::Model) {
						if (modelDragged = Resources::ResourcesManager::Get()->GetOrLoad<Resources::Model>(PayloadIndex.first))
						{
							isAModel = true;
						}
					}
					if (PayloadIndex.second == FileType::Prefab) {
						if (prefabDragged = Resources::ResourcesManager::Get()->GetOrLoad<Resources::Prefab>(PayloadIndex.first))
						{
							isAModel = false;
						}
					}
				}
				if (isAModel && modelDragged && modelDragged->HasBeenSent() && !dragged) {
					auto gameObject = modelDragged->ToGameObject();
					dragged = gameObject;

				}
				else if (!isAModel && prefabDragged && !dragged)
				{
					loaded = true;
					dragged = prefabDragged->CopyPrefab();
				}
				if (dragged) {
					auto Direction = Core::App::Get().sceneManager->GetCurrentScene()->GetEditorCamera()->UnProject({ GetMousePosition(), 20.f });
					dragged->transform->SetWorldPosition(Direction);
					dragged->transform->ForceUpdate();
					dragged->DrawSelfAndChild(true);
				}
				if (WrapperUI::IsMouseReleased(MouseButton::Left) && dragged)
				{
					if (auto currNode = Core::App::Get().sceneManager->GetCurrentNode())
					{
						if (!currNode->isPrefab) {
							currNode->AddChildren(dragged);
						}
					}
					dragged = nullptr;
					LastDragged.clear();
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

Math::Vector2 EditorUI::SceneWindow::GetMousePosition()
{
	auto mouse = WrapperUI::GetMousePos();
	return Math::Vector2(mouse.x, mouse.y) - this->GetWindowPos();
}

bool EditorUI::SceneWindow::IsVisible()
{
	return m_visible;
}
