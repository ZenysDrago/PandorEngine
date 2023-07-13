#pragma once

#include <Resources/IResources.h>

namespace Core
{
	class Scene;
	class GameObject;
}

namespace Resources
{
	class PANDOR_API Prefab : public IResources
	{
	private :
		Core::Scene* m_prevSceneRef = nullptr;
		Core::GameObject* m_prefab = nullptr;
		Core::Scene* m_scenePrefab = nullptr;

	private : 
		void SetEveryObjectAsPrefab(Core::GameObject* gameObject);
	public :
		Prefab(std::string _name, ResourcesType _type);
		/* Create the prefab from a GameObject the name can be set or will be the same as the name of the gameObject */
		Prefab(Core::GameObject* prefabObject , std::string name = "");
		~Prefab();

		bool exitPopup = false;

		void Load() override {}
		void SendResource() override {}

		void LoadScene();
#ifndef PANDOR_GAME
		void SavePrefab();
		void OpenPrefabScene();
		void ExitPrefabPopup();
#endif
		Core::GameObject* GetPrefabNode() const { return m_prefab; }
		void LeavePrefab();
		void UpdatePrefabInScene();
		Core::GameObject* AddPrefabToScene(Core::Scene* scene, Core::GameObject* parent = nullptr);
		Core::GameObject* CopyPrefab();
		void LoadGameObjectAsPrefab(Core::GameObject* gameObject);
		void ReadPrefab(Core::GameObject* reader = nullptr);
		static ResourcesType GetResourceType() { return ResourcesType::Prefab; }
	};
}