#include <pch.h>

#include <Resources/Prefab.h>
#include <Core/GameObject.h>
#include <Core/Scene.h>
#include <Core/SceneManager.h>
#include <Utils/Utils.h>
#include <Components/MeshComponent.h>
#include <Resources/ResourcesManager.h>

using namespace Resources;

Resources::Prefab::Prefab(std::string _path, ResourcesType _type) : IResources(_path, _type)
{

}

Resources::Prefab::Prefab(Core::GameObject* prefabObject, std::string _name) : IResources(Resources::ResourcesManager::AssetPath() + _name + ".prefab", Prefab::GetResourceType())
{
	m_prefab = new Core::GameObject(prefabObject);
	m_prefab->RemoveFromParent();
	
	std::filesystem::create_directory(Resources::ResourcesManager::AssetPath() + "Prefab");

	if (_name == "")
	{
		p_name = prefabObject->GetName();
		std::string tmpPath = Resources::ResourcesManager::AssetPath() + p_name + ".prefab";
		p_fullPath = Resources::ResourcesManager::AssetPath() + p_name + ".prefab";
		Utils::ToPath(tmpPath);
		p_path = tmpPath;
	}

	GetPrefabNode()->transform->SetWorldPosition(Vector3(0.f, 0.f, 0.f));
#ifndef PANDOR_GAME
	SavePrefab();
#endif
}

Resources::Prefab::~Prefab()
{
	if (GetPrefabNode())
	{
		delete m_prefab;
		m_prefab = nullptr;
	}
}

#ifndef PANDOR_GAME
void Resources::Prefab::SavePrefab()
{
	std::fstream prefabFile;
	prefabFile.open(p_fullPath.c_str(), std::fstream::trunc | std::fstream::out);

	if (prefabFile.is_open())
	{
		m_prefab->operator<<(prefabFile);
	}

	/* Close file */
	prefabFile.close();
	PrintLog("Prefab file has been written correctly");
}

void Resources::Prefab::OpenPrefabScene()
{
	m_scenePrefab = new Core::Scene(ENGINEPATH"TMPSCENEPREFAB");
	m_scenePrefab->m_isPrefab = true;
	m_scenePrefab->m_parentPrefab = this;
	Core::SceneManager* sceneManagerRef = Core::SceneManager::Get();
	m_prevSceneRef = sceneManagerRef->GetCurrentScene();

	m_scenePrefab->Load();
	auto gameObjects = m_scenePrefab->m_sceneNode->GetAllChildren();
	m_scenePrefab->m_sceneNode->SetScene(m_scenePrefab);
	for (auto gameobject : gameObjects)
	{
		gameobject->SetScene(m_scenePrefab);
	}

	sceneManagerRef->OpenPrefabScene(m_scenePrefab);
}

void Resources::Prefab::ExitPrefabPopup()
{
	WrapperUI::OpenPopup("Exit Prefab?");

	if (WrapperUI::BeginPopupModal("Exit Prefab?", NULL, WindowFlags::AlwaysAutoResize))
	{
		WrapperUI::Text("Did you saved your prefab ?");
		if (WrapperUI::Button("Save Prefab"))
			SavePrefab();

		WrapperUI::SameLine();
		if (WrapperUI::Button("Save & Quit"))
		{
			SavePrefab();
			LeavePrefab();
			exitPopup = false;
			WrapperUI::CloseCurrentPopup();
		}

		WrapperUI::Text("Do you really want to exit ?");
		if (WrapperUI::Button("Yes"))
		{
			LeavePrefab();
			exitPopup = false;
			WrapperUI::CloseCurrentPopup();
		}
		WrapperUI::SameLine();
		if (WrapperUI::Button("No"))
		{
			exitPopup = false;
			WrapperUI::CloseCurrentPopup();
		}
		WrapperUI::EndPopup();
	}
}

void Resources::Prefab::LeavePrefab()
{
	UpdatePrefabInScene();
	Core::SceneManager::Get()->LeavePrefab();
}
#endif

void Resources::Prefab::UpdatePrefabInScene()
{
	Core::SceneManager* manager = Core::SceneManager::Get();

	if (m_scenePrefab)
	{
		std::vector<Core::GameObject*>objectList = manager->GetCurrentScene()->FindAllObject("[PR] " + p_name);

		for (Core::GameObject* obj : objectList)
		{
			if (obj->isPrefab)
			{
				Component::Transform* transform = new Component::Transform(*obj->transform);
				Core::GameObject* parent = obj->GetParent();
				parent->RemoveChildren(obj);

				obj = new Core::GameObject();
				obj->transform = transform;
				obj->transform->SetGameObject(obj);

				LoadGameObjectAsPrefab(obj);

				parent->AddChildren(obj);
			}
		}
	}
}

Core::GameObject* Resources::Prefab::AddPrefabToScene(Core::Scene* scene, Core::GameObject* parent /*= nullptr*/)
{
	if (!scene)
		return nullptr;

	if (scene->m_isPrefab)
	{
		if (scene->m_parentPrefab = this)
		{
			PrintWarning("Can't add a prefab to itself");
			return nullptr;
		}
	}

	Core::GameObject* prefabObject = CopyPrefab();
	scene->AddObject(prefabObject , parent);
	return prefabObject;
}

Core::GameObject* Prefab::CopyPrefab()
{
	Core::GameObject* prefabObject = new Core::GameObject("[PR] " + p_name);
	prefabObject->SetPrefabObject(p_path);
	Core::GameObject* copyObject;
	if (m_prefab)
	{
		copyObject = new Core::GameObject(m_prefab);
	}
	else
	{
		copyObject = new Core::GameObject();
		ReadPrefab(copyObject);
	}
	copyObject->transform->SetWorldPosition((Vector3)(0.f, 0.f, 0.f));
	prefabObject->AddChildren(copyObject);
	prefabObject->SetPrefabObject(p_path);
	SetEveryObjectAsPrefab(prefabObject);
	return prefabObject;
}

void Resources::Prefab::SetEveryObjectAsPrefab(Core::GameObject* gameObject)
{
	for (Core::GameObject* go : gameObject->GetChildrens())
	{
		if (!go->isPrefab)
		{
			go->SetPrefabObject("");
			SetEveryObjectAsPrefab(go);
		}
	}
}

void Resources::Prefab::LoadGameObjectAsPrefab(Core::GameObject* gameObject)
{
	gameObject->SetName("[PR] " + p_name);
	gameObject->SetPrefabObject(p_path);
	Core::GameObject* copyObject;
	if (m_prefab)
	{
		copyObject = new Core::GameObject(m_prefab);
	}
	else
	{
		copyObject = new Core::GameObject();
		ReadPrefab(copyObject);
	}
	gameObject->AddChildren(copyObject);

	SetEveryObjectAsPrefab(gameObject);
}

void Resources::Prefab::LoadScene()
{
	if (m_scenePrefab->isLoaded)
		return;

	ReadPrefab();
	m_scenePrefab->m_sceneNode = new Core::GameObject();
	m_scenePrefab->m_sceneNode->SetName("[PR] " + p_name);
	m_scenePrefab->m_sceneNode->SetNameLocked(true);

	m_scenePrefab->AddObject(m_prefab);
}

void Resources::Prefab::ReadPrefab(Core::GameObject* reader)
{
	std::fstream prefabFile;
	prefabFile.open(p_fullPath.c_str());

	std::string line;

	if (!reader)
	{
		if (m_prefab)
		{
			delete m_prefab;
			m_prefab = nullptr;
		}

		m_prefab = new Core::GameObject();
	}	

	if (prefabFile.is_open())
	{
		if (getline(prefabFile, line) && line != "end" && line == "===")
		{
			if (getline(prefabFile, line) && line != "end" && line == "GameObject")
			{
				if (reader)
					reader->ReadGameObject(prefabFile, nullptr);
				else
					m_prefab->ReadGameObject(prefabFile, nullptr);	
			}
		}
	}

	/* Close file */
	prefabFile.close();
	PrintLog("Prefab has been read correctly");
}
