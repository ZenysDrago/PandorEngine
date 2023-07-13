#pragma once
#include "PandorAPI.h"

#include <string>
#include <vector>
#include <iostream>

namespace Core
{
	class GameObject;
}

namespace Component
{

	class PANDOR_API BaseComponent
	{
	protected:
		bool p_enable = true;
		std::string p_componentName = "Empty";

	public:
		uint64_t uuid = 0;
		Core::GameObject* gameObject = nullptr;

		BaseComponent();
		virtual ~BaseComponent() = default;

		// Call On Creation.
		virtual void Initialize() {}

		// Call On Begin Play.
		virtual void Awake() {}
		virtual void Start() {}

		//Updates
		virtual void Update() {}
		virtual void EditorUpdate() {}
		virtual void GameUpdate() {}
		//Draws
		virtual void Draw() {}
		virtual void EditorDraw() {}
		virtual void GameDraw() {}
		virtual void DrawPicking(int ID) {}

		virtual void OnCollisionEnter(class Collider* collider) {}
		virtual void OnCollisionStay(class Collider* collider) {}
		virtual void OnCollisionExit(class Collider* collider) {}

		virtual void OnTriggerEnter(class Collider* collider) {}
		virtual void OnTriggerStay(class Collider* collider) {}
		virtual void OnTriggerExit(class Collider* collider) {}

		virtual void ShowInInspector() { }
		virtual void SetGameObject(Core::GameObject* go);
		virtual std::ostream& operator<<(std::ostream& os);

		// Remove Component from GameObject.
		void RemoveFromGameObject();

		bool* GetEnable() { return &p_enable; }
		bool IsEnable() { return p_enable; }

		uint64_t ID() { return uuid; };

		void ShouldStart();

		virtual BaseComponent* Clone() const = 0;

		virtual std::string GetComponentName() = 0;

		virtual void OnDestroy() {};

		virtual void ReadComponent(std::fstream& sceneFile);

	};

	template <typename Derived>
	class IComponent : public BaseComponent {
	public:
		typedef BaseComponent Super;

		virtual std::string GetComponentName() override = 0;

		virtual BaseComponent* Clone() const {
			return new Derived(static_cast<Derived const&>(*this));
		}
	};

	BaseComponent* CreateComponent(std::string componentName);
}
