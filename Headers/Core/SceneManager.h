#pragma once
#include <Debug/Log.h>

namespace Core
{
	class Scene;

	class SceneManager
	{
	private :
		static SceneManager* m_instance;

		std::vector<Scene*> m_sceneList;

		void GetAllScenes();

		bool m_shouldSwitchScene = false;
		bool m_popupSwitch = false;

		Scene* m_scene = nullptr;
		Scene* m_PrefabScene = nullptr;
		Scene* m_currentScene = nullptr;
		Scene* m_switchScene = nullptr;

		Scene* m_inGameSwitchScene = nullptr;
		bool m_shouldSwitchSceneGame = false;

		void SwitchSceneInGame();
		SceneManager();
	public :
		static SceneManager* Get() { return m_instance; }
		static void CreateInstance() { m_instance = new SceneManager(); }

		void Initialize();
		void Delete();
		void Update();
		void InitScenes();
		Scene* FindSceneByName(std::string sceneName);
		bool LoadScene(Scene* scene);	// return false if couldn't load
		bool UnloadScene(Scene* scene); // return false if couldn't unload
		bool CheckSceneLoaded(Scene* scene);
		void DeleteScene(Scene* scene);
		void AddScene(Scene* scene);
		Scene* CreateScene(std::string name, bool shouldLoad = false);
		Scene* CreateSceneFromPath(std::string path, bool shouldLoad = false);
#ifndef PANDOR_GAME
		void SaveScenes(); 
#endif
		void SwitchScene(Scene* scene); /* Maybe add bool shouldReloadResources*/
#ifndef PANDOR_GAME
		void OpenPrefabScene(Scene* scene);
		void LeavePrefab();
#endif
		void ShouldSwitchScene();
		bool RenderingPrefab();

		void SetSwitchSceneInGame(Scene* newScene);
		void LoadAllScenes();
		void UnloadAllScenes();
		void SetCurrentScene(Scene* scene) { m_currentScene = scene; };
		Scene* GetScene() { return m_scene; }
		Scene* GetCurrentScene() { return m_currentScene; }
		Scene* GetCurrentPrefabScene() { return m_PrefabScene; }
		class GameObject* GetCurrentNode();
		std::vector<Scene*> GetAllScene() { return m_sceneList; }

		Scene* GetSceneWithPath(std::string path);


		std::vector<std::string> FindFiles(const std::string& folderName, const std::string& extension);
	};
}
