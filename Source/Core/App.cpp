#include "pch.h"
#include <chrono>

#include <Resources/Texture.h>
#include <Resources/Shader.h>
#include <Render/Camera.h>
#include <Core/App.h>
#include <Core/GameObject.h>
#include <Utils/Loader.h>
#include <Render/Framebuffer.h>
#include <Core/Wrappers/WrapperPhysic.h>
#include <Core/Wrappers/WrapperRHI.h>
#include <Core/Wrappers/WrapperFont.h>
#include <Core/Wrappers/WrapperAudio.h>

#include <Core/SceneManager.h>
#include <Core/Scene.h>
#include <Resources/Shader.h>
#include <Resources/Texture.h>
#include <Resources/Model.h>
#include <Resources/Font.h>
#include <Resources/Prefab.h>
#include <LowRenderer/LightManager.h>

#include <Components/ComponentsData.h>
#ifndef PANDOR_GAME
#include <Render/Gizmo.h>
#include <EditorUI/EditorUiManager.h>
#include <EditorUI/SceneWindow.h>
#include <EditorUI/MainBar.h>
#endif

#include "Scripting\ScriptEngine.h"

using namespace Core;

App* App::m_instance = nullptr;

void Core::App::InitializeUI()
{
	WrapperUI::Initialize(window->GetWindow());
#ifndef PANDOR_GAME
	m_editorUIManager = new EditorUI::EditorUiManager();
#endif // PANDOR_GAME
}


void App::InitializeResources()
{
	Core::Wrapper::WrapperFont::Initialize();
}

void App::InitializePhysic()
{
	physic = new Core::Wrapper::WrapperPhysic::PhysicManager();
	physic->Initialize();
}

void App::InitializeScenes()
{
	Core::SceneManager::CreateInstance();
	sceneManager = Core::SceneManager::Get();
	sceneManager->Initialize();
	PrintLog("Scene Loaded");
}

void Core::App::InitializeAudio()
{
	audioManager = new Core::Wrapper::WrapperAudio::AudioManager();
	WrapperAudio::Listener* listener = audioManager->CreateListener(true);
	listener->SetPosition({0,0,0});
	listener->SetOrientation({0,1,0});
	listener->SetCone({360 * DEG2RAD , 360 * DEG2RAD, 0});
}

void Core::App::InitializeRHI()
{
	WrapperRHI::InitializeAPI();
	WrapperRHI::EnableDebugOutput();
}

void App::InitializeWindow(AppInit init)
{
	window = WrapperWindow::CreateInstance();
	window->Initialize(init);
}

void Core::App::Init(AppInit init, const std::string& projectPath)
{
#ifdef MULTITHREAD
	threadManager = new ThreadManager(std::thread::hardware_concurrency());
#endif // MULTITHREAD
	PrintLog("Creating Application...");

#ifdef PANDOR_GAME
	init.width = (uint32_t)projectSettings.screenSize.x;
	init.height = (uint32_t)projectSettings.screenSize.y;
	init.resizable = projectSettings.resizableScreen;
#endif

	projectSettings = ProjectSettings();
	projectSettings.Load(projectPath);
	InitializeWindow(init);
	InitializeRHI();
#ifdef PANDOR_GAME
	if (projectSettings.fullscreen)
		window->SetFullscreen(true);
#endif
	InitializeUI();
	InitializePhysic();
	InitializeResources();
	InitializeAudio();

	Resources::ResourcesManager::CreateInstance(threadManager, projectPath, projectPath.substr(projectPath.find_last_of('\\') + 1));
	Scripting::ScriptEngine::Init();
	resourcesManager = Resources::ResourcesManager::Get();
	PrintLog("Application Created !");
	resourcesManager->FilesLoad("EngineResources");
	resourcesManager->FilesLoad(projectPath + "/Assets");

	resourcesManager->LoadNecessaryResources();
	resourcesManager->SetupInitalResources();

	shadowMap = new Core::Wrapper::WrapperRHI::ShadowMap();
	shadowMap->InitShadow();

#ifndef PANDOR_GAME
	m_editorUIManager->Initialize();
#endif
	Component::ComponentsData::Create();
	Component::ComponentsData::Get().Initialize();
#ifndef PANDOR_GAME
	LoadEngineParameters();
#endif
	InitializeScenes();
}

void App::BeginFrame()
{
	window->PollEvents();
	WrapperUI::NewFrame();
}

void CloseEngineEvent(GLFWwindow* window)
{
	if (App::Get().shouldCloseWindowEvent && App::Get().appCallCloseWindow) {
		App::Get().window->CloseWindow();
	}
	else {
		App::Get().window->CloseWindow(false);
		App::Get().shouldCloseWindowEvent = true;
	}
}

void App::Update()
{
	WrapperRHI::DepthActive();

	window->SetCloseCallback(CloseEngineEvent);
	while (!window->ShouldClose())
	{
		BeginFrame();

		ExecuteEventQueue();

		if (shouldCloseWindowEvent) { CloseAppPopup(); }

		if (m_resourceToSend.size() != 0)
		{
			IResources* res = resourcesManager->Find<IResources>(m_resourceToSend.front());
			if (res && !res->HasBeenSent()) {
				res->SendResource();
			}
			if (!res || res->HasBeenSent()) {
				if (appMutex.try_lock()) {
					m_resourceToSend.pop_front();
					appMutex.unlock();
				}
			}
		}

		if (WrapperUI::IsKeyPressed(Key::Key_F11))
		{
			window->ToggleFullscreen();
		}

		sceneManager->Update();

#ifdef PANDOR_GAME

		static bool performanceOpen = false;
		if (WrapperUI::IsKeyPressed(Key::Key_F3))
		{
			performanceOpen = !performanceOpen;
		}
		WrapperUI::SetNextWindowSize(Vector2(350, 0));
		WrapperUI::SetNextWindowPos(window->GetWindowPos() + Vector2(window->GetSize().x - 350, 0));
		WrapperUI::ShowPerformanceWindow("##PeformanceWindow", &performanceOpen);
#endif

		EndFrame();
	}
#ifndef PANDOR_GAME
	SaveEngineParameters();
#endif
}


void App::Clear()
{
	WrapperUI::Destroy();
	Delete();
}

void Core::App::Delete()
{
	if (sceneManager)
		sceneManager->Delete();
	sceneManager = nullptr;
	Scripting::ScriptEngine::Shutdown();

	Core::Wrapper::WrapperFont::Delete();

	delete physic;
	physic = nullptr;

	delete threadManager;
	threadManager = nullptr;

	window->Terminate();
	delete window;
	window = nullptr;

	delete sceneManager;
	sceneManager = nullptr;

	Component::ComponentsData::Delete();

	delete resourcesManager;
	resourcesManager = nullptr;

	delete audioManager;
	audioManager = nullptr;

#ifndef PANDOR_GAME
	delete m_editorUIManager;
	m_editorUIManager = nullptr;
#endif
	delete m_instance;
	m_instance = nullptr;

}

void Core::App::AddResourceToSend(const std::string& resourceName)
{
	while (!appMutex.try_lock()) {}

	if (std::find(m_resourceToSend.begin(), m_resourceToSend.end(), resourceName) == m_resourceToSend.end())
	{
		m_resourceToSend.push_back(resourceName);
	}
	appMutex.unlock();
}

void Core::App::AddEvent(const std::function<void()>& function)
{
	std::scoped_lock<std::mutex> lock(m_EventQueueMutex);
	m_EventQueue.emplace_back(function);
}


void App::SetGameState(GameState gs)
{
#ifndef PANDOR_GAME
	timeScale = 1.f;
	if (gs == m_gameState)
		return;
	switch (gs)
	{
	case Core::GameState::Play: {
		if (m_gameState == GameState::Pause)
			break;

		physic->CreateScene();
		//Scripting
		{
			// Stores scene pointer for scripting
			Scripting::ScriptEngine::OnStart(sceneManager->GetCurrentScene());
		}
		m_editorUIManager->GetConsole().Clear();
		sceneManager->GetCurrentScene()->BeginPlay();
		break;
	}
	case Core::GameState::Editor:
		physic->DeleteScene();
		//Scripting
		{
			// Stores scene pointer for scripting
			Scripting::ScriptEngine::OnStop();
		}
		break;
	case Core::GameState::Pause:
		break;
	default:
		break;
	}
	m_gameState = gs;
#else
	if (gs == m_gameState)
		return;
	m_gameState = gs;
	physic->CreateScene();
	//Scripting
	{
		// Stores scene pointer for scripting
		Scripting::ScriptEngine::OnStart(sceneManager->GetCurrentScene());
	}
	size_t index = 0;
	sceneManager->GetScene()->GetSceneNode()->UpdateIndex(index);
	sceneManager->GetCurrentNode()->transform->ForceUpdate();
	sceneManager->GetCurrentNode()->Awake();
	sceneManager->GetCurrentNode()->Start();
#endif
}

void App::CloseApp()
{
	window->CloseWindow();
}

void App::EndFrame()
{
	WrapperUI::EndFrame();
	window->SwapBuffers();
}

void Core::App::ExecuteEventQueue()
{
	if (GetGameState() == GameState::Play)
		return;

	std::vector<std::function<void()>> queueCopy;
	{
		std::scoped_lock<std::mutex> lock(m_EventQueueMutex);
		queueCopy = m_EventQueue;
		m_EventQueue.clear();
	}

	for (const std::function<void()>& func : queueCopy)
		func();
}

void Core::App::CloseAppPopup()
{
#ifndef	PANDOR_GAME
	if (!showCloseAppPopup)
	{
		appCallCloseWindow = true;
		window->CloseWindow();
		WrapperUI::CloseCurrentPopup();
		return;
	}

	WrapperUI::OpenPopup("Exit?");

	if (WrapperUI::BeginPopupModal("Exit?", NULL, WindowFlags::AlwaysAutoResize))
	{
		WrapperUI::Text("Did you saved your scene ?");
		if (WrapperUI::Button("Save scene"))
			sceneManager->SaveScenes();
		WrapperUI::SameLine();
		if (WrapperUI::Button("Save & Quit"))
		{
			sceneManager->SaveScenes();
			appCallCloseWindow = true;
			window->CloseWindow();
			WrapperUI::CloseCurrentPopup();
		}

		WrapperUI::Text("Do you really want to exit ?");
		if (WrapperUI::Button("Yes"))
		{
			appCallCloseWindow = true;
			window->CloseWindow();
			WrapperUI::CloseCurrentPopup();
		}
		WrapperUI::SameLine();
		if (WrapperUI::Button("No"))
		{
			shouldCloseWindowEvent = false;
			WrapperUI::CloseCurrentPopup();
		}

		WrapperUI::Checkbox("Show this message again", &showCloseAppPopup);

		WrapperUI::EndPopup();
	}
#else
	appCallCloseWindow = true;
	window->CloseWindow();
#endif
}

#ifndef PANDOR_GAME
void Core::App::SaveEngineParameters()
{
	/* Create or open file */
	std::fstream engineParametersFile;
	engineParametersFile.open(ResourcesManager::ProjectPath() + '/' + "EngineParameters.pandorSettings", std::fstream::trunc | std::fstream::out);

	if (engineParametersFile.is_open())
	{
		engineParametersFile << m_editorUIManager->GetMainBar().GetOutlineColor() << "\n";
		engineParametersFile << showCloseAppPopup << "\n";
		// Get scene Node name because it will never be saveLoad scene
		engineParametersFile << Core::SceneManager::Get()->GetCurrentScene()->GetSceneNode()->GetName() << '\n';
		engineParametersFile << GetEditorUIManager().themeIndex << '\n';
	}

	engineParametersFile.close();
}
#endif

#ifndef PANDOR_GAME
void Core::App::LoadEngineParameters()
{
	std::fstream engineParametersFile;
	engineParametersFile.open(ResourcesManager::ProjectPath() + '/' + "EngineParameters.pandorSettings");
	std::string line;
	if (engineParametersFile.is_open())
	{
		if (getline(engineParametersFile, line))
			m_editorUIManager->GetMainBar().SetOutlineColor(ParseVector3(line));

		if (getline(engineParametersFile, line))
			showCloseAppPopup = (bool)std::atoi(line.c_str());

		if (getline(engineParametersFile, line))
			sceneToLoad = line;

		if (getline(engineParametersFile, line))
			m_editorUIManager->themeIndex = std::stoi(line);
	}
	WrapperUI::SetupTheme(m_editorUIManager->themeIndex);
	engineParametersFile.close();
}
#endif

void ProjectSettings::Save()
{
	std::fstream projectParametersFile;
	projectParametersFile.open(ResourcesManager::ProjectPath() + '/' + "ProjectParameters.pandorSettings", std::fstream::trunc | std::fstream::out);

	if (projectParametersFile.is_open())
	{
		for (auto&& scene : scenes)
		{
			projectParametersFile << "===" << '\n';
			projectParametersFile << scene << '\n';
		}
		projectParametersFile << fullscreen << '\n';
		projectParametersFile << screenSize << '\n';
		projectParametersFile << resizableScreen << '\n';
	}

	projectParametersFile.close();
}

void Core::ProjectSettings::Load(const std::string& projectPath)
{
	std::fstream projectParametersFile;
	projectParametersFile.open(projectPath + '/' + "ProjectParameters.pandorSettings");
	std::string line;
	if (projectParametersFile.is_open())
	{
		while (getline(projectParametersFile, line) && line == "===")
		{
			if (getline(projectParametersFile, line))
				this->scenes.push_back(line);
		}
		this->fullscreen = std::stoi(line);

		if (getline(projectParametersFile, line))
			this->screenSize = ParseVector2(line);

		if (getline(projectParametersFile, line))
			this->resizableScreen = std::stoi(line);
	}
	else
	{
		PrintError("Failed to open File %s", (projectPath + '/' + "ProjectParameters.pandorSettings").c_str());
	}

	projectParametersFile.close();
}
