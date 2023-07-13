#pragma once
#include "PandorAPI.h"

#include <Resources/ResourcesManager.h>
#include <Core/ThreadManager.h>

namespace Resources
{
	class Shader;
	class Texture;
}

namespace Core::Wrapper
{
	class WrapperWindow;
	namespace WrapperPhysic
	{
		class PhysicManager;
	}
	namespace WrapperRHI
	{
		class Buffer;
		class ShadowMap;
	}
	namespace WrapperAudio
	{
		class AudioManager;
		class Listener;
	}
}

namespace EditorUI
{
	class EditorUiManager;
}

namespace Render
{
	class Camera;
	class Gizmo;
}

namespace Core
{
	struct PANDOR_API AppInit
	{
		unsigned int width;
		unsigned int height;
		unsigned int major;
		unsigned int minor;
		const char* name;
		bool resizable;
	};

	struct ProjectSettings
	{
		std::vector<std::string> scenes;
		bool fullscreen = false;
		bool resizableScreen = false;
		Vector2 screenSize = { 1600, 900 };

		void Save();
		void Load(const std::string& projectPath);
	};

	enum class PANDOR_API GameState
	{
		Play,
		Editor,
		Pause
	};

	class PANDOR_API App
	{
	private:
		static  App* m_instance;
		std::mutex appMutex;

#ifndef PANDOR_GAME
		EditorUI::EditorUiManager* m_editorUIManager;
#endif
		std::deque<std::string>   m_resourceToSend;

		GameState m_gameState = GameState::Editor;
	public:
		ThreadManager* threadManager = nullptr;
		Resources::ResourcesManager* resourcesManager = nullptr;
		WrapperRHI::ShadowMap* shadowMap = nullptr;

		Core::Wrapper::WrapperWindow* window = nullptr;
		Core::Wrapper::WrapperPhysic::PhysicManager* physic = nullptr;
		Core::Wrapper::WrapperAudio::AudioManager* audioManager;

		unsigned char data[4];
		class SceneManager* sceneManager = nullptr;

		float timeScale = 1.f;

		/* Boolean for close popup */
		bool shouldCloseWindowEvent = false;
		bool appCallCloseWindow = false;
		bool showCloseAppPopup = true;
		bool shouldDrawCollider = false;
		bool createMeshThumbnails = false;

		size_t drawCall;
		size_t verticeCount;
		size_t triangleCount;

		std::vector<std::function<void()>> m_EventQueue;
		std::mutex m_EventQueueMutex;

		std::string sceneToLoad;

		ProjectSettings projectSettings;
	private:
		void InitializeWindow(const AppInit init);
		void InitializeRHI();
		void InitializeUI();
		void InitializePhysic();
		void InitializeScenes();
		void InitializeAudio();
		void InitializeResources();
		void ExecuteEventQueue();
		void CloseAppPopup();
#ifndef PANDOR_GAME
		void SaveEngineParameters();
		void LoadEngineParameters();
#endif
	public:
		void BeginFrame();
		void EndFrame();

		static App& Get() { return *m_instance; }
		static void CreateInstance() { m_instance = new App(); }
#ifndef PANDOR_GAME
		EditorUI::EditorUiManager& GetEditorUIManager() { return *m_editorUIManager; }
#endif
		void Init(AppInit init, const std::string& projectPath);
		void Update();
		void Clear();
		void CloseApp();
		void Delete();
		void AddResourceToSend(const std::string& resourceName);
		GameState GetGameState() { return m_gameState; }
		void SetGameState(GameState gs);

		void AddEvent(const std::function<void()>& function);
	};
}
