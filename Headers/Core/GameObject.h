#pragma once
#include "PandorAPI.h"

#include <vector>
#include <string>
#include <Components/Transform.h>
#include <Core/Wrappers/WrapperUI.h>
#include <iostream>

namespace Scripting
{
	struct ScriptField;
}

namespace Component
{
	class Collider;
}
namespace Core
{
	class Scene;

	class PANDOR_API GameObject
	{
	protected:
		Scene* m_scene;

		std::string m_name = "GameObject";

		GameObject* m_parent = nullptr;

		std::vector<GameObject*> m_childrens = {};

		std::vector<std::shared_ptr<Component::BaseComponent>> m_components;

		std::string prefabPath = "";

		bool m_open = false;
		bool m_active = true; // Boolean for inspector active
		bool m_selected = false;
		int m_pickingID = 0;
		bool m_nameLocked = false;

	private:

		void UnSetPrefabAllChildren();
	public:
		uint64_t uuid = 0;
		size_t index = -1;
		bool isPrefab = false;
		Component::Transform* transform;

		GameObject();
		GameObject(std::string name);
		GameObject(GameObject* copied);

		virtual ~GameObject();

		void AddChildren(GameObject* child);
		void RemoveChildren(GameObject* child);

		Component::BaseComponent* AddComponent(Component::BaseComponent* component);
		template<typename T> Component::BaseComponent* AddComponent()
		{
			auto component = std::make_shared<T>();
			component->SetGameObject(this);
			uint64_t index = 0;
			for (index; index < m_components.size(); index++)
			{
				if (index + 1 == m_components[index]->uuid)
					continue;
				break;
			}
			component->uuid = index + 1;
			component->Initialize();



			m_components.insert(m_components.begin() + index, component);
			component->ShouldStart();
			return component.get();
		}
		void RemoveComponent(Component::BaseComponent* component);

		void SetActive(bool val) { m_active = val; }

		void Awake();
		void Start();

		static GameObject* GetSceneNode();

		void UpdateSelfAndChild(size_t& _index);
		void DrawModelForShadow();

		void DrawSelfAndChild(bool editorCamera = false);

		void UpdateIndex(size_t& _index);

		void OnCollisionStay(Component::Collider* collider);
		void OnCollisionEnter(Component::Collider* collider);
		void OnCollisionExit(Component::Collider* collider);

		void OnTriggerEnter(Component::Collider* collider);
		void OnTriggerStay(Component::Collider* collider);
		void OnTriggerExit(Component::Collider* collider);

		void ShowInHierarchy();

		std::string GetName() const { return m_name; }
		GameObject* GetParent() const { return m_parent; }

		std::vector<GameObject*> GetChildrens() const;
		GameObject* GetChild(size_t index) const;

		std::vector<std::shared_ptr<Component::BaseComponent>> GetComponents() { return m_components; }
		std::vector<std::shared_ptr<Component::BaseComponent>>* GetComponentsList() { return &m_components; }

		bool* GetActive() { return &m_active; }
		bool IsActive() { return m_active; }

		void SetName(const std::string& name);
		void SetParent(GameObject* parent);
		void UpdateChildScene(GameObject* go);

		void DrawPicking(int& ID);

		virtual void RemoveFromParent();

		void SetNameLocked(bool lock) { m_nameLocked = lock; }
		void SetSelected(bool value) { m_selected = value; }
		void SetPrefabObject(std::string _path) { isPrefab = true; prefabPath = _path; }
		void UnSetPrefabObject();

		template<typename T>
		inline T* GetComponent() const
		{
			for (auto comp : m_components)
			{
				if (auto value = dynamic_cast<T*>(comp.get()))
				{
					return value;
				}
			}
			return nullptr;
		}

		template<typename T>
		inline std::vector<T*> GetComponents() const
		{
			std::vector<T*> list;
			for (auto comp : m_components)
			{
				if (auto value = std::dynamic_pointer_cast<T>(comp))
				{
					list.push_back(value.get());
				}
			}
			return list;
		}

		template<typename T>
		inline std::shared_ptr<T> GetComponentShared()
		{
			for (auto comp : m_components)
			{
				if (auto value = std::dynamic_pointer_cast<T>(comp))
				{
					return value;
				}
			}
			return nullptr;
		}

		template<typename T>
		inline T* GetOrAddComponent()
		{
			for (auto comp : m_components)
			{
				if (auto value = dynamic_cast<T*>(comp.get()))
				{
					return value;
				}
			}
			return dynamic_cast<T*>(AddComponent(new T()));
		}

		template<typename T>
		inline std::vector<T*> GetComponentsInChildren()
		{
			std::vector<T*> components;

			T* component = GetComponent<T>();
			if (component)
				components.push_back(component);

			for (auto child : m_childrens)
			{
				for (auto comp : child->m_components)
				{
					if (auto value = dynamic_cast<T*>(comp.get()))
					{
						components.push_back(value);
					}
				}
			}

			return components;
		}

		template<typename T>
		inline T* GetComponentInParent()
		{
			if (!m_parent)
				return nullptr;
			for (auto comp : m_parent->m_components)
			{
				if (auto value = dynamic_cast<T*>(comp.get()))
				{
					return value;
				}
			}
			return m_parent->GetComponentInParent<T>();
		}

		template<typename T>
		inline std::shared_ptr<T> GetComponentSharedInParent()
		{
			if (!m_parent)
				return nullptr;
			for (auto comp : m_parent->m_components)
			{
				if (auto value = std::dynamic_pointer_cast<T>(comp))
				{
					return value;
				}
			}
			return m_parent->GetComponentSharedInParent<T>();
		}

		std::vector<GameObject*> GetAllChildren();

		GameObject* GetRootPrefab();

		GameObject* GetWithPickingIndex(int index);
		GameObject* GetWithIndex(uint64_t _index);
		Component::BaseComponent* GetComponentByFieldType(const Scripting::ScriptField* scriptField);

		Component::BaseComponent* GetComponentByID(const uint64_t ID);

		template<typename T>
		inline T* GetComponentByID(const uint64_t ID)
		{
			if (ID == 0)
				return nullptr;

			for (auto& component : m_components)
			{
				if (component->uuid == ID)
				{
					if (auto comp = std::dynamic_pointer_cast<T>(component))
					{
						return comp.get();
					}
				}
			}
			return nullptr;
		}

		std::shared_ptr<Component::BaseComponent> GetComponentSharedByID(const uint64_t ID);

		virtual GameObject* Instantiate(GameObject* parent = nullptr);

		// Check if the Given GameObject is a parent of this.
		bool IsAParent(GameObject* gameObject);

		void OnDestroy();
		bool IsSelected();

		virtual void ReadGameObject(std::fstream& sceneFile, Scene* scene);

		virtual std::ostream& operator<<(std::ostream& os);

		template <typename T> static std::shared_ptr<T> ComponentDragDropButton(const char* buttonName, const char* componentName)
		{
			WrapperUI::TextUnformatted(componentName);
			WrapperUI::SameLine();
			float size = WrapperUI::CalcItemWidth();
			WrapperUI::Button(buttonName, Vector2(size, 0));
			if (WrapperUI::BeginDragDropTarget()) {
				if (const Payload* payload = WrapperUI::AcceptDragDropPayload("GAMEOBJECT")) {
					uint64_t PayloadIndex = *(uint64_t*)payload->Data;
					GameObject* gameObject = GetSceneNode()->GetWithIndex(PayloadIndex);
					if (gameObject)
					{
						if (auto comp = gameObject->GetComponentShared<T>())
							return comp;
					}
				}
				else if (const Payload* payload = WrapperUI::AcceptDragDropPayload("COMPONENT")) {
					auto PayloadIndex = *(std::pair<uint64_t, uint64_t>*)payload->Data;
					auto gameobject = GetSceneNode()->GetWithIndex(PayloadIndex.first);
					if (auto component = std::dynamic_pointer_cast<T>(gameobject->GetComponentSharedByID(PayloadIndex.second)))
						return component;
				}
				WrapperUI::EndDragDropTarget();
			}
			return nullptr;

		}

		Scene* GetScene() { return m_scene; }
		void SetScene(Scene* scene) { m_scene = scene; }
	};

}