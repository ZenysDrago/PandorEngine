#pragma once
#include "PandorAPI.h"
#include <Components/BaseComponent.h>
#include <unordered_map>
#include <any>

namespace Scripting
{
	class ScriptInstance;
	class ScriptClass;
	struct EditorField;
	struct ScriptField;
}
namespace Popups
{
	bool ScriptPopup(std::string& name);
	bool GameObjectPopup(Core::GameObject*& gameObject, const std::string& fieldName);
}

namespace Component
{
	class PANDOR_API ScriptComponent : public IComponent<ScriptComponent>
	{
	private:
		bool m_canUpdate = false;

	public:
		std::string name = "None";
		std::shared_ptr<Scripting::ScriptInstance> instance;
		std::shared_ptr<std::unordered_map<std::string, Scripting::EditorField>> editorFields;

	private:
		void ShowFieldsPlayMode();
		void ShowFieldsEditorMode();
		void ShowDefaultField(const Scripting::ScriptField& field, std::shared_ptr<Scripting::ScriptClass> scriptClass);
		void ShowEditorField(const Scripting::ScriptField& field, Scripting::EditorField& editorField);

	public:
		ScriptComponent();
		~ScriptComponent();

		void ShowInInspector() override;

		void Awake() override;
		void Start() override;
		void GameUpdate() override;

		void OnCollisionEnter(class Collider* collider) override;
		void OnCollisionStay(class Collider* collider) override;
		void OnCollisionExit(class Collider* collider) override;

		void OnTriggerEnter(class Collider* collider) override;
		void OnTriggerStay(class Collider* collider) override;
		void OnTriggerExit(class Collider* collider) override;

		std::string GetComponentName() override { return "Script Component"; }
		BaseComponent* Clone() const { return new ScriptComponent(); }

		std::ostream& operator<<(std::ostream& os) override;
		void ReadComponent(std::fstream& sceneFile) override;
	};
}