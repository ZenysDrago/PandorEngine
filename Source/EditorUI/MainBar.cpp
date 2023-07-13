#include "pch.h"

#include <EditorUI/MainBar.h>
#include <EditorUI/EditorUIManager.h>
#include <EditorUI/Console.h>
#include <EditorUI/PerformanceWindow.h>
#include <EditorUI/Hierarchy.h>
#include <EditorUI/Inspector.h>
#include <EditorUI/SceneWindow.h>
#include <EditorUI/ResourcesWindow.h>
#include <EditorUI/FileExplorer.h>
#include <EditorUI/GameWindow.h>
#include <EditorUI/SceneWindow.h>
#include <EditorUI/AnimatorWindow.h>
#include <EditorUI/PrefabWindow.h>

#include <Render/Camera.h>
#include <Render/Framebuffer.h>
#include <Render/Gizmo.h>

#include <Core/App.h>
#include <Core/SceneManager.h>
#include <Core/Scene.h>
#include <ShObjIdl.h>

#include <Resources/Prefab.h>
/* TEMP*/
#include <Core/Wrappers/WrapperAudio.h>
#include <Resources/Sound.h>

EditorUI::MainBar::MainBar()
{

}

EditorUI::MainBar::~MainBar()
{
}

void EditorUI::MainBar::Draw()
{
	Core::App& app = Core::App::Get();
	if (WrapperUI::BeginMainMenuBar()) {
		// File
		if (WrapperUI::BeginMenu("File"))
		{
			Core::SceneManager* sceneManager = Core::SceneManager::Get();

			if (WrapperUI::MenuItem("New Scene", "Ctrl+N"))
			{
				Core::Scene* scene = sceneManager->CreateScene("newScene");
				sceneManager->SwitchScene(scene);
			}
			if (WrapperUI::MenuItem("Open Scene", "Ctrl+O"))
			{
				auto path = Utils::OpenFile(".scn", Core::App::Get().window);
				if (!path.empty())
				{
					size_t target_pos = path.find("Resources");

					if (target_pos != std::string::npos)
					{
						path = path.substr(target_pos);
					}
					Core::Scene* scene = sceneManager->FindSceneByName(path);
					if (!scene)
						scene = sceneManager->CreateSceneFromPath(path);

					sceneManager->SwitchScene(scene);

					if (Core::App::Get().GetGameState() != Core::GameState::Editor) {
						Core::App::Get().SetGameState(Core::GameState::Editor);

						std::remove((ENGINEPATH"Scenes/savePlay.scn").c_str());
					}
				}
			}
			if (WrapperUI::MenuItem("Save Scene", "Ctrl+S"))
			{
				if (Core::App::Get().GetEditorUIManager().GetHierarchy().m_sceneFocused)
					sceneManager->GetCurrentScene()->SaveSceneFile(sceneManager->GetCurrentScene()->path);
				else if(sceneManager->GetCurrentPrefabScene())
					sceneManager->GetCurrentPrefabScene()->GetPrefab()->SavePrefab();
			}
			if (WrapperUI::MenuItem("Save as Scene", "Ctrl+Shift+S"))
			{
				if (Core::App::Get().GetEditorUIManager().GetHierarchy().m_sceneFocused)
				{
					auto path = Utils::SaveFile(".scn", Core::App::Get().window);
					if (!path.empty())
					{
						sceneManager->GetCurrentScene()->SaveSceneFile(path + ".scn");
						sceneManager->GetCurrentScene()->path = path + ".scn";
					}
				}
				else
				{
					PrintWarning("Only scenes can be saved like that");
				}
			}
			if (WrapperUI::MenuItem("Exit", "Alt+F4"))
			{
				Core::App::Get().CloseApp();
			}
			WrapperUI::EndMenu();
		}
		// Window
		if (WrapperUI::BeginMenu("Window"))
		{
			WrapperUI::Checkbox("Console", Core::App::Get().GetEditorUIManager().GetConsole().IsOpen());
			WrapperUI::Checkbox("Performance Window", Core::App::Get().GetEditorUIManager().GetPerformanceWindow().IsOpen());
			WrapperUI::Checkbox("Hierarchy", Core::App::Get().GetEditorUIManager().GetHierarchy().IsOpen());
			WrapperUI::Checkbox("Inspector", Core::App::Get().GetEditorUIManager().GetInspector().IsOpen());
			WrapperUI::Checkbox("Resources Window", Core::App::Get().GetEditorUIManager().GetResourceWindow().IsOpen());
			WrapperUI::Checkbox("FileExplorer", Core::App::Get().GetEditorUIManager().GetFileExplorer().IsOpen());
			WrapperUI::Checkbox("Game Window", Core::App::Get().GetEditorUIManager().GetGameWindow().IsOpen());
			WrapperUI::Checkbox("Scene Window", Core::App::Get().GetEditorUIManager().GetSceneWindow().IsOpen());
			WrapperUI::Checkbox("Animator Window", Core::App::Get().GetEditorUIManager().GetAnimatorWindow().IsOpen());
			WrapperUI::Checkbox("Prefab Window", Core::App::Get().GetEditorUIManager().GetPrefabWindow().IsOpen());
			WrapperUI::EndMenu();
		}
		// Settings
		if (WrapperUI::BeginMenu("Settings"))
		{
			if (WrapperUI::Combo("Theme", &Core::App::Get().GetEditorUIManager().themeIndex, "Dark\0Light\0Classic\0Purple\0Rainbow"))
			{
				WrapperUI::SetupTheme(Core::App::Get().GetEditorUIManager().themeIndex);
			}

			if (WrapperUI::BeginMenu("Editor Camera"))
			{
				WrapperUI::TextUnformatted("Editor Camera");
				WrapperUI::Separator();
				WrapperUI::ColorEdit3("Color Outline", &m_outlineColor.x);
				WrapperUI::Checkbox("Enable PostProcessing", Core::App::Get().sceneManager->GetCurrentScene()->GetEditorCamera()->framebuffer->EnablePostProcessing());
				WrapperUI::BeginDisabled(!Core::App::Get().sceneManager->GetCurrentScene()->GetEditorCamera()->framebuffer->EnablePostProcessing());
				if (WrapperUI::Button("Change Post-Processing Shader"))
				{
					WrapperUI::OpenPopup("Shader Popup");
				}
				if (auto shader = Resources::ResourcesManager::Get()->ResourcePopup<Resources::Shader>("Shader Popup"))
				{
					Core::App::Get().sceneManager->GetCurrentScene()->GetEditorCamera()->framebuffer->SetShader(shader);
				}
				WrapperUI::EndDisabled();
				if (WrapperUI::Button("Camera Settings"))
				{
					WrapperUI::OpenPopup("Camera Settings");
				}
				if (WrapperUI::BeginPopup("Camera Settings"))
				{
					Core::SceneManager::Get()->GetCurrentScene()->GetEditorCamera()->DrawCameraSettings();
					WrapperUI::EndPopup();
				}
				WrapperUI::EndMenu();
			}
			WrapperUI::Checkbox("Draw Grid", &Core::SceneManager::Get()->GetCurrentScene()->drawGrid);
			WrapperUI::Checkbox("Draw Icons", &Core::SceneManager::Get()->GetCurrentScene()->drawIcons);
			WrapperUI::Checkbox("Draw Colliders", &Core::App::Get().shouldDrawCollider);
			WrapperUI::Checkbox("Show Popup on exit", &Core::App::Get().showCloseAppPopup);
			if (WrapperUI::Button("Project Settings"))
			{
				WrapperUI::OpenPopup("Project Settings Popup");
			}
			if (WrapperUI::BeginPopupModal("Project Settings Popup"))
			{
				WrapperUI::SeparatorText("Scenes Builds");
				auto& settings = Core::App::Get().projectSettings;
				auto& list = Core::App::Get().projectSettings.scenes;
				static int selected = -1;
				if (WrapperUI::BeginChild("SceneList", { 0, 150 }, true, WindowFlags::NoResize)) {
					int i = 0;
					for (int i = 0; i < list.size(); i++)
					{
						if (WrapperUI::Selectable(list[i].c_str(), selected == i))
							selected = i;

						// Handle dropping an item
						if (WrapperUI::BeginDragDropSource())
						{
							WrapperUI::SetDragDropPayload("ITEM", &i, sizeof(int));
							WrapperUI::TextUnformatted(list[i].c_str());
							WrapperUI::EndDragDropSource();
						}

						if (WrapperUI::BeginDragDropTarget())
						{
							if (const Payload* payload = WrapperUI::AcceptDragDropPayload("ITEM"))
							{
								int* j = (int*)payload->Data;
								std::string prev = list[*j];
								list[*j] = list[i];
								list[i] = prev;
							}
							WrapperUI::EndDragDropTarget();
						}
					}
				}
				WrapperUI::EndChild();
				if (WrapperUI::Button("Add Current Scene"))
				{
					std::string path = Core::SceneManager::Get()->GetCurrentScene()->path;
						Utils::ToPath(path);
					if (std::find(list.begin(), list.end(), path) == list.end())
						Core::App::Get().projectSettings.scenes.push_back(path);
				}
				WrapperUI::SameLine();
				if (WrapperUI::Button("Delete Selected"))
				{
					if (list.size() > selected)
						list.erase(list.begin() + selected);
				}
				WrapperUI::SeparatorText("Game Settings");
				WrapperUI::Checkbox("Fullscreen", &settings.fullscreen);
				WrapperUI::BeginDisabled(settings.fullscreen);
				WrapperUI::InputFloat2("Window Size", &settings.screenSize.x);
				WrapperUI::Checkbox("Resizable", &settings.resizableScreen);
				WrapperUI::EndDisabled();
				if (WrapperUI::Button("Save & Close"))
				{
					Core::App::Get().projectSettings.Save();
					WrapperUI::CloseCurrentPopup();
				}
				WrapperUI::EndPopup();
			}
			if (WrapperUI::BeginMenu("Gizmo Settings"))
			{
				int mode = (int)Core::SceneManager::Get()->GetCurrentScene()->drawGizmo->GetGizmoMode();
				if (WrapperUI::Combo("Gizmo Space", &mode, "World\0Local"))
				{
					Core::SceneManager::Get()->GetCurrentScene()->drawGizmo->SetGizmoMode((Render::GizmoMode)mode);
					WrapperUI::CloseCurrentPopup();
				}
				WrapperUI::EndMenu();
			}
			WrapperUI::EndMenu();
		}
		
		if (app.sceneManager->GetCurrentPrefabScene() && WrapperUI::BeginMenu("Prefab"))
		{
			Core::SceneManager* sceneManager = Core::SceneManager::Get();
			Resources::Prefab* curPrefab = sceneManager->GetCurrentPrefabScene()->GetPrefab();

			if (WrapperUI::Button("Save Prefab"))
				curPrefab->SavePrefab();

			if (WrapperUI::Button("Leave Prefab"))
				curPrefab->exitPopup = true;

			WrapperUI::EndMenu();
		}
		if (WrapperUI::BeginMenu("Switch Scene"))
		{
			Core::SceneManager* sceneManager = app.sceneManager;
			static std::vector<Core::Scene*> listScene = sceneManager->GetAllScene();

			if (app.GetGameState() == Core::GameState::Editor)
			{
				listScene = sceneManager->GetAllScene();
				for (Core::Scene* scene : listScene)
				{
					if (WrapperUI::Button(("Switch scene : " + scene->GetName()).c_str()))
					{
						sceneManager->SwitchScene(scene);
					}
				}
			}
			else if (app.GetGameState() == Core::GameState::Play)
			{
				for (Core::Scene* scene : listScene)
				{
					if (WrapperUI::Button(("Switch scene : " + scene->GetName()).c_str()))
					{
						sceneManager->SetSwitchSceneInGame(scene);
					}
				}
			}

			WrapperUI::EndMenu();
		}
		WrapperUI::SetCursorPosX((float)(Core::App::Get().window->GetSize().x / 2) - 25.f);
		if (WrapperUI::MenuItem(Core::App::Get().GetGameState() == Core::GameState::Play || app.GetGameState() == Core::GameState::Pause ? "[]" : "I>"))
		{
			if (app.GetGameState() == Core::GameState::Editor)
			{
				app.SetGameState(Core::GameState::Play);
				app.sceneManager->GetCurrentScene()->SaveSceneState();
			}
			else if (app.GetGameState() == Core::GameState::Play || app.GetGameState() == Core::GameState::Pause)
			{
				app.SetGameState(Core::GameState::Editor);
				app.sceneManager->GetCurrentScene()->ReloadSceneState();
			}
		}
		if (WrapperUI::MenuItem("||"))
		{
			if (app.GetGameState() == Core::GameState::Play)
			{
				app.SetGameState(Core::GameState::Pause);
			}
			else if (app.GetGameState() == Core::GameState::Pause)
			{
				app.SetGameState(Core::GameState::Play);
			}
		}
		switch (app.GetGameState())
		{
		case Core::GameState::Editor:
		{
			WrapperUI::TextUnformatted("Editor");
			break;
		}
		case Core::GameState::Play:
		{
			WrapperUI::TextUnformatted("Play");
			break;
		}
		case Core::GameState::Pause:
		{
			WrapperUI::TextUnformatted("Pause");
			break;
		}
		default:
			break;
		}
	}
	WrapperUI::EndMainMenuBar();
	if (Core::App::Get().GetEditorUIManager().themeIndex == 4)
	{
		WrapperUI::SetupTheme(4);
	}
}

Vector3 EditorUI::MainBar::GetOutlineColor()
{
	Vector3 tempColor = m_outlineColor;
	return  tempColor;
}
