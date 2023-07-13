#include "pch.h"

#include <EditorUi/Hierarchy.h>
#include <EditorUi/EditorUIManager.h>
#include <EditorUi/Inspector.h>
#include <EditorUi/SceneWindow.h>
#include <EditorUi/PrefabWindow.h>

#include <Core/App.h>
#include <Core/Scene.h>
#include <Core/SceneManager.h>
#include <Core/GameObject.h>
#include <Resources/ResourcesManager.h>
#include <Resources/Skeleton.h>
#include <Resources/Model.h>
#include <Resources/Prefab.h>

using namespace Core::Wrapper;

EditorUI::Hierarchy::Hierarchy()
{
}

EditorUI::Hierarchy::~Hierarchy()
{
}

void EditorUI::Hierarchy::Draw()
{
	if (!p_open)
		return;
	if (WrapperUI::Begin("Hierarchy", &p_open))
	{
		if (WrapperUI::IsKeyPressed(Key::Key_Delete))
		{
			if ((WrapperUI::IsWindowFocused(FocusedFlags::ChildWindows)
				|| (Core::App::Get().GetEditorUIManager().GetSceneWindow().IsFocused()
					|| Core::App::Get().GetEditorUIManager().GetPrefabWindow().IsFocused()))
				&& Core::App::Get().GetEditorUIManager().GetInspector().GetGameObjectSelected()) 
			{
				Core::App::Get().GetEditorUIManager().GetInspector().GetGameObjectSelected()->RemoveFromParent();
			}
		}

		if (WrapperUI::IsKeyDown(Key::Key_LeftCtrl) && WrapperUI::IsKeyPressed(Key::Key_C))
		{
			if (auto selected = Core::App::Get().GetEditorUIManager().GetInspector().GetGameObjectSelected()) {
				std::ofstream m_clipboard = std::ofstream("clipboard.txt", std::fstream::trunc | std::fstream::out);
				if (m_clipboard.is_open()) {
					selected->operator<<(m_clipboard);
					m_clipboard.close();
				}
			}
		}
		
		if (WrapperUI::IsKeyDown(Key::Key_LeftCtrl) && WrapperUI::IsKeyPressed(Key::Key_X))
		{
			if (auto selected = Core::App::Get().GetEditorUIManager().GetInspector().GetGameObjectSelected()) {
				std::ofstream m_clipboard = std::ofstream("clipboard.txt", std::fstream::trunc | std::fstream::out);
				if (m_clipboard.is_open()) {
					selected->operator<<(m_clipboard);
					m_clipboard.close();
				}
				selected->RemoveFromParent();
			}
		}

		if (WrapperUI::IsKeyDown(Key::Key_LeftCtrl) && WrapperUI::IsKeyPressed(Key::Key_V))
		{
			if (auto selected = Core::App::Get().GetEditorUIManager().GetInspector().GetGameObjectSelected()) {
				std::fstream file("clipboard.txt", std::ios::in | std::ios::out);
				std::string line;
				getline(file, line);
				Core::GameObject* go = nullptr;
				if (getline(file, line) && line != "end" && line == "GameObject")
				{
					go = Core::SceneManager::Get()->GetCurrentScene()->CreateObject();
					go->ReadGameObject(file, Core::SceneManager::Get()->GetCurrentScene());
					
					if (!selected->isPrefab)
						selected->AddChildren(go);
				}
				else if (line != "end" && line == "Bone")
				{
					go = Core::SceneManager::Get()->GetCurrentScene()->CreateBoneObject();
					go->ReadGameObject(file, Core::SceneManager::Get()->GetCurrentScene());
					if (!selected->isPrefab)
						selected->AddChildren(go);
				}
				go->GetScene()->ResetObjectToList(go);
				for (auto& gameObject : go->GetAllChildren())
				{
					go->GetScene()->ResetObjectToList(gameObject);
				}
			}
		}

		WrapperUI::BeginChild("Child");
		Core::SceneManager* sceneManager = Core::App::Get().sceneManager;

		//Update Focused.
		if ((sceneManager->GetCurrentPrefabScene() && Core::App::Get().GetEditorUIManager().GetPrefabWindow().IsFocused()))
			m_sceneFocused = false;
		else if (Core::App::Get().GetEditorUIManager().GetSceneWindow().IsFocused())
			m_sceneFocused = true;


		if (!m_sceneFocused && sceneManager->GetCurrentPrefabScene())
			sceneManager->GetCurrentPrefabScene()->GetSceneNode()->ShowInHierarchy();
		else
			Core::App::Get().sceneManager->GetCurrentNode()->ShowInHierarchy();

		
		if (m_openRightClick)
		{
			m_openRightClick = false;
			WrapperUI::OpenPopup("RightClick");
		}
		DrawRightClick();

		WrapperUI::EndChild();
	}
	WrapperUI::End();
}

void EditorUI::Hierarchy::DrawRightClick()
{
	if (WrapperUI::BeginPopup("RightClick", WindowFlags::NoDecoration))
	{
		if (WrapperUI::Button("New Node"))
		{
			m_rightClicked->AddChildren(m_rightClicked->GetScene()->CreateObject());
			WrapperUI::CloseCurrentPopup();
		}
		if (WrapperUI::Button("Delete"))
		{
			m_rightClicked->RemoveFromParent();
			WrapperUI::CloseCurrentPopup();
		}
		if (WrapperUI::Button("Clear Parent"))
		{
			if (m_rightClicked->GetParent())
				m_rightClicked->SetParent(m_rightClicked->GetScene()->GetSceneNode());

			WrapperUI::CloseCurrentPopup();
		}
		if (WrapperUI::Button("New Parent"))
		{
			if (m_rightClicked->GetParent()) {
				auto parent = m_rightClicked->GetScene()->CreateObject();
				m_rightClicked->GetParent()->AddChildren(parent);
				m_rightClicked->SetParent(parent);
				WrapperUI::CloseCurrentPopup();
			}
		}
		if (WrapperUI::Button("Load Model"))
		{
			WrapperUI::OpenPopup("Model Popup");
		}
		if (WrapperUI::Button("Create Prefab"))
		{
			Core::App::Get().GetEditorUIManager().GetInspector().SetGameObjectSelected(nullptr);
			Resources::Prefab* prefab = new Prefab(m_rightClicked);
			Resources::ResourcesManager::Get()->Add(prefab->GetName(), prefab); 
			prefab->OpenPrefabScene();
		}
		if (auto model = Resources::ResourcesManager::Get()->ModelPopup("Model Popup"))
		{
			m_waitingModels.push_back(Resources::ResourcesManager::Get()->GetOrLoad<Resources::Model>(model->GetPath()));
			WrapperUI::CloseCurrentPopup();
		}
		WrapperUI::EndPopup();
	}
	for (int i = 0; i < m_waitingModels.size(); i++)
	{
		if (m_waitingModels[i]->HasBeenSent())
		{
			auto node = m_waitingModels[i]->ToGameObject();
			m_rightClicked->AddChildren(node);
			m_waitingModels.erase(m_waitingModels.begin() + i);
			break;
		}
	}
}


void EditorUI::Hierarchy::SetGameObjectRightClicked(Core::GameObject* node)
{
	m_openRightClick = true;
	m_rightClicked = node;
}
