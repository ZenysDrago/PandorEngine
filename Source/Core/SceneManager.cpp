#include "pch.h"

#include <Core/SceneManager.h>
#include <Core/Scene.h>
#include <Utils/Loader.h>
#include <Core/App.h>
#include <Core/GameObject.h>
#include <Core/Wrappers/WrapperPhysic.h>
#include <Scripting/ScriptEngine.h>
#ifndef PANDOR_GAME
#include <EditorUI/FileExplorer.h>
#include <EditorUI/EditorUIManager.h>
#include <EditorUI/PrefabWindow.h>
#include <Resources/Prefab.h>
#include <EditorUI/Inspector.h>
#include <Render/Gizmo.h>
#endif

using namespace Core;

SceneManager* SceneManager::m_instance = nullptr;

Core::SceneManager::SceneManager()
{
	GetAllScenes();
}

void Core::SceneManager::GetAllScenes()
{
#ifndef PANDOR_GAME
	std::vector<std::string> sceneFound = FindFiles(Resources::ResourcesManager::AssetPath(),"scn");

	for (auto& scenePath : sceneFound)
	{
		Scene* newScene = new Scene(scenePath);
		m_sceneList.push_back(newScene);
		newScene->Initialize();
	}
#else
	for (auto& scenePath : Core::App::Get().projectSettings.scenes)
	{
		Scene* newScene = new Scene(ResourcesManager::ProjectPath() + '/' + scenePath);
		m_sceneList.push_back(newScene);
		newScene->Initialize();
	}
#endif
}

void Core::SceneManager::InitScenes()
{
	if (m_sceneList.empty())
	{
		CreateScene("Scene0", true);
	}
	else
	{
		if (m_scene = GetSceneWithPath(Core::App::Get().sceneToLoad)) {
			Core::App::Get().sceneToLoad.clear();
			LoadScene(m_scene);
		}
		else
		{
			m_scene = m_sceneList[0];
			LoadScene(m_scene);
		}
	}
}


void Core::SceneManager::Initialize()
{
	InitScenes();
}

void Core::SceneManager::Delete()
{
	for (auto&& scene : m_sceneList)
	{
		scene->Delete();
		delete scene;
		scene = nullptr;
	}
	m_scene = nullptr;
	m_sceneList.clear();
	delete m_instance;
	m_instance = nullptr;
}

void Core::SceneManager::Update()
{
	if (m_scene->GetSceneNode())
		m_scene->Update();
#ifndef PANDOR_GAME
	if (m_PrefabScene)
		m_PrefabScene->UpdatePrefabScene();
#endif
	ShouldSwitchScene();
	SwitchSceneInGame();
}

Scene* Core::SceneManager::FindSceneByName(std::string sceneName)
{
	size_t sepPos = sceneName.find_last_of("/\\") != std::string::npos ? sceneName.find_last_of("/\\") : 0;
	size_t dotPos = sceneName.find_last_of(".") != std::string::npos ? sceneName.find_last_of(".") : sceneName.length();
	if (dotPos < sepPos)
	{
		std::cout << "Error: Invalid file sceneName" << std::endl;
		return nullptr;
	}
	std::string fileName = sceneName.substr(sepPos == 0 ? sepPos : sepPos + 1, dotPos - (sepPos == 0 ? 0 : sepPos + 1));

	for (auto it = m_sceneList.begin(); it < m_sceneList.end(); ++it)
	{
		if ((*it)->GetName() == fileName)
			return *it;
	}

	PrintWarning("Couldn't find scene with name : %s", sceneName.c_str());
	return nullptr;
}

bool Core::SceneManager::LoadScene(Scene* scene)
{
	if (scene)
	{
		if (!m_scene)
			m_scene = scene;

		scene->Load();

		return true;
	}
	else
	{
		PrintWarning("Couldn't load the scene given");
		return false;
	}
}

bool Core::SceneManager::UnloadScene(Scene* scene)
{
	if (scene->isLoaded)
	{
		scene->Delete();
		return true;
	}
	return false;
}

bool Core::SceneManager::CheckSceneLoaded(Scene* scene)
{
	return scene->GetSceneNode();
}

void Core::SceneManager::DeleteScene(Scene* scene)
{
	std::string sceneName = scene->GetName();

	if (scene == m_currentScene)
		PrintWarning("Can't delete current scene abort");

	UnloadScene(scene);

	auto it = std::find(m_sceneList.begin(), m_sceneList.end(), scene);
	if (it != m_sceneList.end()) {
		m_sceneList.erase(it);
		delete scene;
		scene = nullptr;
	}

	PrintLog("Scene %s, has been deleted successfully", sceneName);
}

void Core::SceneManager::AddScene(Scene* scene)
{
	m_sceneList.push_back(scene);
}

Scene* Core::SceneManager::CreateScene(std::string name, bool shouldLoad)
{
	Scene* scene = new Scene((ResourcesManager::AssetPath() + "Scenes/" + name + ".scn").c_str());
	scene->name = name;
	m_sceneList.push_back(scene);
	if (shouldLoad && !LoadScene(scene))
		PrintError("Couldn't load new Scene with name : %s", name);

	return scene;
}

Scene* Core::SceneManager::CreateSceneFromPath(std::string path, bool shouldLoad)
{
	Scene* scene = new Scene(path.c_str());

	m_sceneList.push_back(scene);
	if (shouldLoad && !LoadScene(scene))
		PrintError("Couldn't load new Scene with path : %s", path);

	return scene;
}
#ifndef PANDOR_GAME
void Core::SceneManager::SaveScenes()
{
	for (Scene* scene : m_sceneList)
	{
		if (CheckSceneLoaded(scene))
			scene->SaveSceneFile(scene->path);
	}
}
#endif

void Core::SceneManager::SwitchScene(Scene* scene)
{
	m_switchScene = scene;
	m_shouldSwitchScene = true;
	m_popupSwitch = true;
}

#ifndef PANDOR_GAME
void Core::SceneManager::OpenPrefabScene(Scene* scene)
{
	m_PrefabScene = scene;
	Core::App& app = Core::App::Get();
	EditorUI::PrefabWindow& prefabWindow = app.GetEditorUIManager().GetPrefabWindow();

	if (!(*prefabWindow.IsOpen()))
		prefabWindow.SetOpen(true);

	//WrapperUI::SetWindowFocus("PrefabWindow");
}

void Core::SceneManager::LeavePrefab()
{
	m_PrefabScene = nullptr;
	Core::App& app = Core::App::Get();
	EditorUI::PrefabWindow& prefabWindow = app.GetEditorUIManager().GetPrefabWindow();

	if ((*prefabWindow.IsOpen()))
		prefabWindow.SetOpen(false);
}
#endif

void SceneManager::ShouldSwitchScene()
{
	if (!m_shouldSwitchScene)
		return;
#ifndef PANDOR_GAME
	else if (m_popupSwitch)
	{
		WrapperUI::OpenPopup("SwitchScene Popup");

		if (WrapperUI::BeginPopupModal("SwitchScene Popup", NULL, WindowFlags::AlwaysAutoResize))
		{
			WrapperUI::Text("Do you really want to leave this scene ?");
			if (WrapperUI::Button("Save & Quit"))
			{
				m_currentScene->SaveSceneFile(m_currentScene->path);
				m_shouldSwitchScene = false;
				m_popupSwitch = false;
				WrapperUI::CloseCurrentPopup();
			}
			if (WrapperUI::Button("Switch")) 
			{
				m_shouldSwitchScene = false;
				m_popupSwitch = false;
				WrapperUI::CloseCurrentPopup();
			}
			if (WrapperUI::Button("Abort")) 
			{
				m_shouldSwitchScene = false;
				m_popupSwitch = false;
				WrapperUI::CloseCurrentPopup();
				WrapperUI::EndPopup();
				return;
			}
			WrapperUI::EndPopup();
		}

		if (m_shouldSwitchScene)
			return;
	}
#endif
	m_shouldSwitchScene = false;

	if (m_switchScene && m_switchScene != GetCurrentScene())
	{
#ifndef PANDOR_GAME
		App::Get().GetEditorUIManager().GetInspector().SetGameObjectSelected(nullptr);
#endif

		UnloadScene(m_switchScene);
		UnloadScene(m_currentScene);
		m_currentScene = nullptr;

		/* Unload every resources */
		//App::Get().resourcesManager->DeleteAll();

		//ResourcesManager::Get()->FilesLoad("Resources");
		//ResourcesManager::Get()->LoadNecessaryResources();
		//ResourcesManager::Get()->SetupInitalResources();
		
#ifndef PANDOR_GAME
		m_switchScene->m_editorUi = &Core::App::Get().GetEditorUIManager();
#endif

		if (LoadScene(m_switchScene))
		{
			SetCurrentScene(m_switchScene);
		}
		else
		{
			PrintError("Couldn't Load scene : %s", m_switchScene->GetName());
			CreateScene("scene");
		}
		m_scene = m_currentScene;
	}
}

bool Core::SceneManager::RenderingPrefab()
{
	return m_currentScene->m_isPrefab;
}

void Core::SceneManager::SwitchSceneInGame()
{
	if (!m_shouldSwitchSceneGame)
		return;

	Scripting::ScriptEngine::OnStop();
	m_currentScene->ReloadSceneState();

	m_switchScene = m_inGameSwitchScene;
	m_shouldSwitchScene = m_shouldSwitchSceneGame;
	ShouldSwitchScene();

#ifndef PANDOR_GAME
	m_inGameSwitchScene->SaveSceneState();
#endif

	Core::App::Get().physic->CreateScene();
	{
		// Stores scene pointer for scripting
		Scripting::ScriptEngine::OnStart(m_currentScene);
	}
#ifndef PANDOR_GAME
	Core::App::Get().GetEditorUIManager().GetConsole().Clear();
#endif
	m_currentScene->BeginPlay();

	m_shouldSwitchSceneGame = false;
	m_inGameSwitchScene = nullptr;
}

void Core::SceneManager::SetSwitchSceneInGame(Scene* newScene)
{
	if (!newScene || newScene == m_currentScene || Core::App::Get().GetGameState() != GameState::Play)
		return;

	m_inGameSwitchScene = newScene;
	if(m_inGameSwitchScene)
		m_shouldSwitchSceneGame = true;
}

void Core::SceneManager::LoadAllScenes()
{
	for (Scene* scene : m_sceneList)
	{
		scene->Load();
	}
}

void Core::SceneManager::UnloadAllScenes()
{
	for (Scene* scene : m_sceneList)
	{
		if (scene != m_currentScene)
			UnloadScene(scene);
	}
}

GameObject* Core::SceneManager::GetCurrentNode()
{
	return m_currentScene->GetSceneNode();
}

std::vector<std::string> SceneManager::FindFiles(const std::string& folderName, const std::string& extension)
{
	auto dirIt = std::filesystem::directory_iterator(folderName);
	std::vector<std::string> filesFounded;
	for (const auto& entry : dirIt)
	{
		auto ext = entry.path().string().substr(entry.path().string().find_last_of('.') + 1);
		if (entry.is_directory())
		{
			std::vector<std::string> found = FindFiles(entry.path().string(), extension);
			filesFounded.insert(filesFounded.begin(), found.begin(), found.end());
		}
		else if (ext == extension)
		{
			filesFounded.push_back(entry.path().generic_string().data());
		}
	}

	return filesFounded;
}

Core::Scene* SceneManager::GetSceneWithPath(std::string path)
{

	std::transform(path.begin(), path.end(), path.begin(), ::tolower);

	for (auto& scene : m_sceneList)
	{
		std::string sceneName = scene->GetName();
		std::transform(sceneName.begin(), sceneName.end(), sceneName.begin(), ::tolower);
		if (sceneName == path)
		{
			return scene;
		}
	}
	return nullptr;
}