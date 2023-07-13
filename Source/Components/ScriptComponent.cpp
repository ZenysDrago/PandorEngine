#include "pch.h"
#include "Components/ScriptComponent.h"
#include "Components/Rigidbody.h"
#include "Components/UI/Text.h"
#include "Components/UI/Button.h"
#include "Components/Animator.h"
#include "Components/SoundEmitter.h"
#include "Components/BoxCollider.h"
#include "Core/App.h"
#include "Core/Scene.h"
#include "Core/SceneManager.h"
#include "Core/GameObject.h"
#include "Scripting/ScriptEngine.h"
#include "Debug/Log.h"
#include "Utils/Utils.h"

using namespace Scripting;

Component::ScriptComponent* scriptComponent;

namespace Popups
{
	bool ScriptPopup(std::string& name)
	{
		bool out = false;

		if (WrapperUI::BeginPopup("ScriptList", WindowFlags::AlwaysVerticalScrollbar))
		{
			static TextFilter filter;
			filter.Draw("##");
			if (!Utils::IsOnlyLetters(filter.InputBuf)) {
				filter.InputBuf[0] = '\0';
				filter.Build();
			}
			for (const auto& [scriptName, scriptClass] : ScriptEngine::GetScriptClasses())
			{
				if (!filter.PassFilter(scriptName.c_str()))
					continue;
				if (WrapperUI::Selectable(scriptName.c_str(), false, SelectableFlags::SpanAllColumns)) {
					name = scriptName;
					out = true;
					WrapperUI::CloseCurrentPopup();
					break;
				}
			}
			WrapperUI::BeginDisabled(!ScriptEngine::IsScriptNameAvaliable(filter.InputBuf));
			if (WrapperUI::Button("Create Script"))
			{
				ScriptEngine::CreateScriptFile(filter.InputBuf);
				name = filter.InputBuf;
				out = true;
				WrapperUI::CloseCurrentPopup();
			}
			WrapperUI::EndDisabled();
			WrapperUI::EndPopup();
		}
		return out;
	}

	bool GameObjectPopup(Core::GameObject*& gameObject, const std::string& fieldName)
	{
		bool out = false;
		if (WrapperUI::BeginPopup(("GameObjectList##" + fieldName).c_str(), WindowFlags::AlwaysVerticalScrollbar))
		{
			int tag = 0;
			static TextFilter filter;
			filter.Draw("##");
			for (auto& [id, object] : Core::App::Get().sceneManager->GetCurrentScene()->GetObjectMap())
			{
				std::string objectName = object->GetName();
				if (!filter.PassFilter(objectName.c_str()))
					continue;
				objectName += "##" + std::to_string(tag);
				if (WrapperUI::Selectable(objectName.c_str(), false, SelectableFlags::SpanAllColumns)) {
					gameObject = object;
					out = true;
					WrapperUI::CloseCurrentPopup();
					break;
				}
				tag += 1;
			}
			WrapperUI::EndPopup();
		}
		return out;
	}

	template<typename T>
	bool ShowComponentList(Component::BaseComponent*& component)
	{
		int tag = 0;
		static TextFilter filter;
		filter.Draw("##");
		for (auto& [id, object] : Core::App::Get().sceneManager->GetCurrentScene()->GetObjectMap())
		{
			std::vector<T*> compList = object->GetComponents<T>();
			for (T* comp : compList)
			{
				std::string objectName = object->GetName();
				if (!filter.PassFilter(objectName.c_str()))
					continue;
				objectName += "##" + std::to_string(tag);
				if (WrapperUI::Selectable(objectName.c_str(), false, SelectableFlags::SpanAllColumns)) {
					component = dynamic_cast<Component::BaseComponent*>(comp);
					WrapperUI::CloseCurrentPopup();
					return true;
				}
				tag += 1;
			}
		}
		return false;
	}

	bool ShowScriptInstanceList(Component::BaseComponent*& component, const ScriptField& scriptField)
	{
		int tag = 0;
		static TextFilter filter;
		filter.Draw("##");
		for (auto& [id, object] : Core::App::Get().sceneManager->GetCurrentScene()->GetObjectMap())
		{
			std::vector<Component::ScriptComponent*> scriptList = ScriptEngine::GetScriptsFromType(object, scriptField);
			for (Component::ScriptComponent* script : scriptList)
			{
				std::string objectName = object->GetName();
				if (!filter.PassFilter(objectName.c_str()))
					continue;
				objectName += "##" + std::to_string(tag);
				if (WrapperUI::Selectable(objectName.c_str(), false, SelectableFlags::SpanAllColumns)) {
					component = dynamic_cast<Component::BaseComponent*>(script);
					WrapperUI::CloseCurrentPopup();
					return true;
				}
				tag += 1;
			}
		}
		return false;
	}

	bool ComponentPopup(Component::BaseComponent*& component, const Scripting::ScriptField* scriptField)
	{
		bool out = false;
		if (WrapperUI::BeginPopup(("ComponentList##" + scriptField->name).c_str(), WindowFlags::AlwaysVerticalScrollbar))
		{
			switch (scriptField->type)
			{
			case ScriptFieldType::ScriptComponent:
				out = ShowScriptInstanceList(component, *scriptField);
				break;
			case ScriptFieldType::Rigidbody:
				out = ShowComponentList<Component::Rigidbody>(component);
				break;
			case ScriptFieldType::Text:
				out = ShowComponentList<Component::UI::Text>(component);
				break;
			case ScriptFieldType::Button:
				out = ShowComponentList<Component::UI::Button>(component);
				break;
			case ScriptFieldType::Animator:
				out = ShowComponentList<Component::Animator>(component);
				break;
			case ScriptFieldType::SoundEmitter:
				out = ShowComponentList<Component::SoundEmitter>(component);
				break;
			case ScriptFieldType::BoxCollider:
				out = ShowComponentList<Component::BoxCollider>(component);
				break;
			default:
				break;
			}
			WrapperUI::EndPopup();
		}
		return out;
	}
}

Component::ScriptComponent::ScriptComponent()
{
	editorFields = std::make_shared<std::unordered_map<std::string, Scripting::EditorField>>();
	scriptComponent = this;
}

Component::ScriptComponent::~ScriptComponent()
{
	ScriptEngine::DestroyScrpitInstance(*this);
}

void Component::ScriptComponent::Awake()
{
	if (!(m_canUpdate = ScriptEngine::ScriptClassExists(name)))
	{
		ScriptEngine::DestroyScrpitInstance(*this);
		return;
	}

	ScriptEngine::CreateScrpitInstance(*this);
}

void Component::ScriptComponent::Start()
{
	if (!m_canUpdate)
		return;

	ScriptEngine::OnCreateScript(*this);
}

void Component::ScriptComponent::GameUpdate()
{
	if (!m_canUpdate)
		return;

	ScriptEngine::OnUpdateScript(*this, WrapperUI::GetDeltaTime());
}

void Component::ScriptComponent::OnCollisionEnter(Collider* collider)
{
	if (m_canUpdate)
		instance->InvokeOnCollision(Physic::CollisionType::ENTER, collider);
}

void Component::ScriptComponent::OnCollisionStay(Collider* collider)
{
	if (m_canUpdate)
		instance->InvokeOnCollision(Physic::CollisionType::STAY, collider);
}

void Component::ScriptComponent::OnCollisionExit(Collider* collider)
{
	if (m_canUpdate)
		instance->InvokeOnCollision(Physic::CollisionType::EXIT, collider);
}

void Component::ScriptComponent::OnTriggerEnter(Collider* collider)
{
	if (m_canUpdate)
		instance->InvokeOnTrigger(Physic::CollisionType::ENTER, collider);
}

void Component::ScriptComponent::OnTriggerStay(Collider* collider)
{
	if (m_canUpdate)
		instance->InvokeOnTrigger(Physic::CollisionType::STAY, collider);
}

void Component::ScriptComponent::OnTriggerExit(Collider* collider)
{
	if (m_canUpdate)
		instance->InvokeOnTrigger(Physic::CollisionType::EXIT, collider);
}

void Component::ScriptComponent::ShowInInspector()
{
	// Play Mode
	if (Core::App::Get().GetGameState() == Core::GameState::Play) {

		WrapperUI::BeginDisabled(m_canUpdate);
		if (WrapperUI::Button(name.c_str(), Math::Vector2(200, 0)))
			WrapperUI::OpenPopup("ScriptList");
		if (Popups::ScriptPopup(name))
		{
			// Init script
			editorFields.get()->clear();
			Awake();
			Start();
		}
		WrapperUI::EndDisabled();

		if (m_canUpdate)
		{
			WrapperUI::SameLine();
			if (WrapperUI::Button("Edit"))
				ScriptEngine::OpenSolution(ResourcesManager::AssetPath(), name + ".cs");
		}
		WrapperUI::NewLine();

		// Script fileds
		ShowFieldsPlayMode();
	}

	// Editor Mode
	else {

		if (WrapperUI::Button(name.c_str(), Math::Vector2(200, 0)))
			WrapperUI::OpenPopup("ScriptList");
		if (Popups::ScriptPopup(name))
		{
			editorFields.get()->clear();
			return;
		}
		if (!ScriptEngine::ScriptClassExists(name))
			return;

		WrapperUI::SameLine();
		if (WrapperUI::Button("Edit"))
			ScriptEngine::OpenSolution(ResourcesManager::AssetPath(), name + ".cs");

		WrapperUI::NewLine();

		// Script fileds
		ShowFieldsEditorMode();
	}
}

bool SetNameIfPossible(std::string& name, const std::string& fieldTypeName, const std::pair<uint64_t, uint64_t>& id, const ScriptFieldType fieldType)
{
	Core::GameObject* object = Core::App::Get().sceneManager->GetCurrentScene()->GetObjectByID(id.first);
	if (object)
	{
		if (fieldType == ScriptFieldType::ScriptComponent)
		{
			Component::ScriptComponent* script = dynamic_cast<Component::ScriptComponent*>(object->GetComponentByID(id.second));
			if (script != nullptr && fieldTypeName == script->name)
			{
				name = object->GetName();
				return true;
			}
		}
		else
		{
			Component::BaseComponent* comp = object->GetComponentByID(id.second);
			if (comp != nullptr)
			{
				std::string compName = comp->GetComponentName();
				compName.erase(std::remove(compName.begin(), compName.end(), ' '), compName.end());
				if (fieldTypeName == compName)
				{
					name = object->GetName();
					return true;
				}
			}
		}
		
	}
	return false;
}

void Component::ScriptComponent::ShowFieldsPlayMode()
{
	if (instance)
	{
		const auto& fields = instance->GetScriptClass()->GetFields();
		for (const auto& [fieldName, field] : fields)
		{
			switch (field.type)
			{
			case ScriptFieldType::Float:
			{
				float data = instance->GetFieldValue<float>(fieldName);
				if (WrapperUI::DragFloat(fieldName.c_str(), &data))
					instance->SetFieldValue(fieldName, data);
				break;
			}
			case ScriptFieldType::Int:
			{
				int data = instance->GetFieldValue<int>(fieldName);
				if (WrapperUI::DragInt(fieldName.c_str(), &data))
					instance->SetFieldValue(fieldName, data);
				break;
			}
			case ScriptFieldType::Bool:
			{
				bool data = instance->GetFieldValue<bool>(fieldName);
				if (WrapperUI::Checkbox(fieldName.c_str(), &data))
					instance->SetFieldValue(fieldName, data);
				break;
			}
			case ScriptFieldType::Vector2:
			{
				Vector2 data = instance->GetFieldValue<Vector2>(fieldName);
				if (WrapperUI::DragFloat2(fieldName.c_str(), &data.x))
					instance->SetFieldValue(fieldName, data);
				break;
			}
			case ScriptFieldType::Vector3:
			{
				Vector3 data = instance->GetFieldValue<Vector3>(fieldName);
				if (WrapperUI::DragFloat3(fieldName.c_str(), &data.x))
					instance->SetFieldValue(fieldName, data);
				break;
			}
			case ScriptFieldType::Vector4:
			{
				Vector4 data = instance->GetFieldValue<Vector4>(fieldName);
				if (WrapperUI::DragFloat4(fieldName.c_str(), &data.x))
					instance->SetFieldValue(fieldName, data);
				break;
			}
			case ScriptFieldType::GameObject:
			{
				// Get gameobject ID
				uint64_t id = instance->GetFieldObject(fieldName);

				// Set Name
				Core::GameObject* object = Core::App::Get().sceneManager->GetCurrentScene()->GetObjectByID(id);
				std::string name = (object) ? object->GetName() : "None";

				// Set Button
				if (WrapperUI::Button((name + " (GameObject)##" + fieldName).c_str()))
					WrapperUI::OpenPopup(("GameObjectList##" + fieldName).c_str());
				if (Popups::GameObjectPopup(object, fieldName) && object)
					instance->SetFieldObject(*this, fieldName, object->uuid);

				// Drop event 
				else if (WrapperUI::BeginDragDropTarget()) {
					if (const Payload* payload = WrapperUI::AcceptDragDropPayload("GAMEOBJECT")) {
						uint64_t payloadIndex = *(uint64_t*)payload->Data;
						if (id != payloadIndex)
							instance->SetFieldObject(*this, fieldName, payloadIndex);
					}
					WrapperUI::EndDragDropTarget();
				}

				WrapperUI::SameLine(); WrapperUI::Text(fieldName.c_str());

				break;
			}
			default:
			{
				if (field.type <= ScriptFieldType::GameObject)
					break;

				// Get gameobject and component ID
				std::pair<uint64_t, uint64_t> id = instance->GetFieldComponent(fieldName);

				// Set name
				std::string name = "None";
				const std::string fieldTypeName = (field.type == ScriptFieldType::ScriptComponent) ? ScriptEngine::GetFieldTypeName(field) : Helper::ScriptFieldTypeToString(field.type);
				if (id.first != 0)
				{
					if (!SetNameIfPossible(name, fieldTypeName, id, field.type))
						instance->SetFieldValue(fieldName, NULL);
				}

				// Set Button
				if (WrapperUI::Button((name + " (" + fieldTypeName + ")##" + field.name).c_str()))
					WrapperUI::OpenPopup(("ComponentList##" + fieldName).c_str());
				Component::BaseComponent* comp = nullptr;
				if (Popups::ComponentPopup(comp, &field) && comp)
					instance->SetFieldComponent(*this, fieldName, comp->gameObject->uuid, comp->uuid);

				// Drop event 
				if (WrapperUI::BeginDragDropTarget()) {
					if (const Payload* payload = WrapperUI::AcceptDragDropPayload("GAMEOBJECT")) {
						uint64_t payloadIndex = *(uint64_t*)payload->Data;
						Core::GameObject* object = Core::App::Get().sceneManager->GetCurrentScene()->GetObjectByID(payloadIndex);
						BaseComponent* baseComp = object->GetComponentByFieldType(&field);
						if (baseComp)
							instance->SetFieldComponent(*this, fieldName, baseComp->gameObject->uuid, baseComp->uuid);
					}
					WrapperUI::EndDragDropTarget();
				}

				WrapperUI::SameLine(); WrapperUI::Text(fieldName.c_str());

				break;
			}
			}
		}
	}
}

void Component::ScriptComponent::ShowFieldsEditorMode()
{
	std::shared_ptr<ScriptClass> scriptClass = ScriptEngine::GetScriptClass(name);
	const auto& fields = scriptClass->GetFields();
	for (const auto& [fieldName, field] : fields)
	{
		auto it = editorFields->find(fieldName);

		// Field has been set in editor
		if (it != editorFields->end()) {
			ShowEditorField(field, it->second);
		}

		// Field has NOT been set in editor
		else {
			ShowDefaultField(field, scriptClass);
		}
	}
}

void Component::ScriptComponent::ShowEditorField(const Scripting::ScriptField& field, Scripting::EditorField& editorField)
{
	switch (field.type)
	{
	case ScriptFieldType::Float:
	{
		float data = editorField.GetValue<float>();
		if (WrapperUI::DragFloat(field.name.c_str(), &data))
			editorField.SetValue(data);
		break;
	}
	case ScriptFieldType::Int:
	{
		int data = editorField.GetValue<int>();
		if (WrapperUI::DragInt(field.name.c_str(), &data))
			editorField.SetValue(data);
		break;
	}
	case ScriptFieldType::Bool:
	{
		bool data = editorField.GetValue<bool>();
		if (WrapperUI::Checkbox(field.name.c_str(), &data))
			editorField.SetValue(data);
		break;
	}
	case ScriptFieldType::Vector2:
	{
		Vector2 data = editorField.GetValue<Vector2>();
		if (WrapperUI::DragFloat2(field.name.c_str(), &data.x))
			editorField.SetValue(data);
		break;
	}
	case ScriptFieldType::Vector3:
	{
		Vector3 data = editorField.GetValue<Vector3>();
		if (WrapperUI::DragFloat3(field.name.c_str(), &data.x))
			editorField.SetValue(data);
		break;
	}
	case ScriptFieldType::Vector4:
	{
		Vector4 data = editorField.GetValue<Vector4>();
		if (WrapperUI::DragFloat4(field.name.c_str(), &data.x))
			editorField.SetValue(data);
		break;
	}
	case ScriptFieldType::GameObject:
	{
		// Get gameobject ID
		uint64_t id = editorField.GetValue<uint64_t>();

		// Set name
		Core::GameObject* object = Core::App::Get().sceneManager->GetCurrentScene()->GetObjectByID(id);
		std::string name = (object) ? object->GetName() : "None";

		// Set Button
		if (WrapperUI::Button((name + " (GameObject)##" + field.name).c_str()))
			WrapperUI::OpenPopup(("GameObjectList##" + field.name).c_str());
		if (Popups::GameObjectPopup(object, field.name) && object)
			editorField.SetValue(object->uuid);

		// Drop event
		else if (WrapperUI::BeginDragDropTarget()) {
			if (const Payload* payload = WrapperUI::AcceptDragDropPayload("GAMEOBJECT")) {
				uint64_t payloadIndex = *(uint64_t*)payload->Data;
				if (id != payloadIndex)
					editorField.SetValue(payloadIndex);
			}
			WrapperUI::EndDragDropTarget();
		}

		WrapperUI::SameLine(); WrapperUI::Text(field.name.c_str());

		break;
	}
	default:
	{
		if (field.type <= ScriptFieldType::GameObject)
			break;

		// Get gameobject and component ID
		std::pair<uint64_t, uint64_t> id;
		editorField.GetValues(id.first, id.second);

		// Set name
		std::string name = "None";
		const std::string fieldTypeName = (field.type == ScriptFieldType::ScriptComponent) ? ScriptEngine::GetFieldTypeName(field) : Helper::ScriptFieldTypeToString(field.type);
		if (id.first != 0)
		{
			if (!SetNameIfPossible(name, fieldTypeName, id, field.type))
			{
				editorFields.get()->erase(field.name);
				editorField.SetValue<bool>(false);
			}
		}

		// Set Button
		if (WrapperUI::Button((name + " (" + fieldTypeName + ")##" + field.name).c_str()))
			WrapperUI::OpenPopup(("ComponentList##" + field.name).c_str());
		Component::BaseComponent* comp = nullptr;
		if (Popups::ComponentPopup(comp, &field) && comp)
			editorField.SetValues(comp->gameObject->uuid, comp->uuid);

		// Drop event
		if (WrapperUI::BeginDragDropTarget()) {
			if (const Payload* payload = WrapperUI::AcceptDragDropPayload("GAMEOBJECT")) {
				uint64_t payloadIndex = *(uint64_t*)payload->Data;
				Core::GameObject* object = Core::App::Get().sceneManager->GetCurrentScene()->GetObjectByID(payloadIndex);
				BaseComponent* baseComp = object->GetComponentByFieldType(&field);
				if (baseComp)
					editorField.SetValues(baseComp->gameObject->uuid, baseComp->uuid);
			}
			WrapperUI::EndDragDropTarget();
		}

		WrapperUI::SameLine(); WrapperUI::Text(field.name.c_str());
	}
	}
}

void Component::ScriptComponent::ShowDefaultField(const Scripting::ScriptField& field, std::shared_ptr<ScriptClass> scriptClass)
{
	switch (field.type)
	{
	case ScriptFieldType::Float:
	{
		float data;
		ScriptEngine::GetFieldValue(*scriptClass, field.classField, (void*)&data);
		if (WrapperUI::DragFloat(field.name.c_str(), &data))
		{
			EditorField& editorField = (*editorFields)[field.name];
			editorField.field = field;
			editorField.SetValue(data);
		}
		break;
	}
	case ScriptFieldType::Int:
	{
		int data;
		ScriptEngine::GetFieldValue(*scriptClass, field.classField, (void*)&data);
		if (WrapperUI::DragInt(field.name.c_str(), &data))
		{
			EditorField& editorField = (*editorFields)[field.name];
			editorField.field = field;
			editorField.SetValue(data);
		}
		break;
	}
	case ScriptFieldType::Bool:
	{
		bool data;
		ScriptEngine::GetFieldValue(*scriptClass, field.classField, (void*)&data);
		if (WrapperUI::Checkbox(field.name.c_str(), &data))
		{
			EditorField& editorField = (*editorFields)[field.name];
			editorField.field = field;
			editorField.SetValue(data);
		}
		break;
	}
	case ScriptFieldType::Vector2:
	{
		Vector2 data;
		ScriptEngine::GetFieldValue(*scriptClass, field.classField, (void*)&data);
		if (WrapperUI::DragFloat2(field.name.c_str(), &data.x))
		{
			EditorField& editorField = (*editorFields)[field.name];
			editorField.field = field;
			editorField.SetValue(data);
		}
		break;
	}
	case ScriptFieldType::Vector3:
	{
		Vector3 data;
		ScriptEngine::GetFieldValue(*scriptClass, field.classField, (void*)&data);
		if (WrapperUI::DragFloat3(field.name.c_str(), &data.x))
		{
			EditorField& editorField = (*editorFields)[field.name];
			editorField.field = field;
			editorField.SetValue(data);
		}
		break;
	}
	case ScriptFieldType::Vector4:
	{
		Vector4 data;
		ScriptEngine::GetFieldValue(*scriptClass, field.classField, (void*)&data);
		if (WrapperUI::DragFloat4(field.name.c_str(), &data.x))
		{
			EditorField& editorField = (*editorFields)[field.name];
			editorField.field = field;
			editorField.SetValue(data);
		}
		break;
	}
	case ScriptFieldType::GameObject:
	{
		std::string name = "None (GameObject)##" + field.name;

		if (WrapperUI::Button(name.c_str()))
			WrapperUI::OpenPopup(("GameObjectList##" + field.name).c_str());

		Core::GameObject* object = nullptr;
		if (Popups::GameObjectPopup(object, field.name) && object)
		{
			EditorField& editorField = (*editorFields)[field.name];
			editorField.field = field;
			editorField.SetValue(object->uuid);
		}
		else if (WrapperUI::BeginDragDropTarget()) {
			if (const Payload* payload = WrapperUI::AcceptDragDropPayload("GAMEOBJECT")) {
				uint64_t payloadIndex = *(uint64_t*)payload->Data;
				EditorField& editorField = (*editorFields)[field.name];
				editorField.field = field;
				editorField.SetValue(payloadIndex);
			}
			WrapperUI::EndDragDropTarget();
		}

		WrapperUI::SameLine(); WrapperUI::Text(field.name.c_str());

		break;
	}
	default:
	{
		if (field.type <= ScriptFieldType::GameObject)
			break;

		// Set name
		std::string name = (field.type == ScriptFieldType::ScriptComponent) ? ScriptEngine::GetFieldTypeName(field) : Helper::ScriptFieldTypeToString(field.type);
		name = "None (" + name + ")##" + field.name.c_str();

		// Set Button
		if (WrapperUI::Button(name.c_str()))
			WrapperUI::OpenPopup(("ComponentList##" + field.name).c_str());
		Component::BaseComponent* comp = nullptr;
		if (Popups::ComponentPopup(comp, &field) && comp)
		{
			EditorField& editorField = (*editorFields)[field.name];
			editorField.field = field;
			editorField.SetValues(comp->gameObject->uuid, comp->uuid);
		}

		// Drop event
		else if (WrapperUI::BeginDragDropTarget()) {
			if (const Payload* payload = WrapperUI::AcceptDragDropPayload("GAMEOBJECT")) {
				uint64_t payloadIndex = *(uint64_t*)payload->Data;
				Core::GameObject* object = Core::App::Get().sceneManager->GetCurrentScene()->GetObjectByID(payloadIndex);

				BaseComponent* baseComp = object->GetComponentByFieldType(&field);

				if (baseComp)
				{
					EditorField& editorField = (*editorFields)[field.name];
					editorField.field = field;
					editorField.SetValues(baseComp->gameObject->uuid, baseComp->uuid);
				}
			}
			WrapperUI::EndDragDropTarget();
		}

		WrapperUI::SameLine(); WrapperUI::Text(field.name.c_str());
	}
	}
}

std::ostream& Component::ScriptComponent::operator<<(std::ostream& os)
{
	os << name << "\n";

	for (auto& [fieldName, edfield] : *editorFields)
	{
		os << fieldName;
		os << "\n";
		os << Scripting::Helper::ScriptFieldTypeToString(edfield.field.type);
		os << "\n";
	}

	os << "values" << "\n";

	for (auto& [fieldName, edfield] : *editorFields)
	{
		os << fieldName;
		os << " | ";
		// Field has been set in editor
		auto it = editorFields->find(fieldName);
		if (it != editorFields->end())
		{
			switch (edfield.field.type)
			{
			case ScriptFieldType::Float:
			{
				float data = edfield.GetValue<float>();
				os << data;
				break;
			}
			case ScriptFieldType::Int:
			{
				int data = edfield.GetValue<int>();
				os << data;
				break;
			}
			case ScriptFieldType::Bool:
			{
				int data = edfield.GetValue<bool>();
				os << data;
				break;
			}
			case ScriptFieldType::Vector2:
			{
				Vector2 data = edfield.GetValue<Vector2>();
				os << data;
				break;
			}
			case ScriptFieldType::Vector3:
			{
				Vector3 data = edfield.GetValue<Vector3>();
				os << data;
				break;
			}
			case ScriptFieldType::Vector4:
			{
				Vector4 data = edfield.GetValue<Vector4>();
				os << data;
				break;
			}
			case ScriptFieldType::GameObject:
			{
				uint64_t id = edfield.GetValue<uint64_t>();
				os << id;
				break;
			}
			default:
			{
				if (edfield.field.type <= ScriptFieldType::GameObject)
					break;

				std::pair<uint64_t, uint64_t> id;
				edfield.GetValues(id.first, id.second);

				os << id.first << ',' << id.second;
			}
			}
		}

		os << "\n";
	}

	return os;
}

struct Readvariables
{
	std::string name;
	std::string type;
};


void Component::ScriptComponent::ReadComponent(std::fstream& sceneFile)
{
	std::string line;

	if (getline(sceneFile, line) && line != "end")
	{
		if (!ScriptEngine::ScriptClassExists(line))
		{
			while (line != "end")
				getline(sceneFile, line);

			return;
		}
		name = line;
	}

	std::vector<Readvariables> variables = {};

	while (getline(sceneFile, line) && line != "values" && line != "end")
	{
		Readvariables var;
		var.name = line;
		if (getline(sceneFile, line) && line != "end")
		{
			var.type = line;
			variables.push_back(var);
		}
	}

	std::shared_ptr<ScriptClass> scriptClass = ScriptEngine::GetScriptClass(name);
	const auto& fields = scriptClass->GetFields();

	auto SetData = [&](const std::string& fieldName, ScriptField field, const auto& data) {
		EditorField& editorField = (*editorFields)[fieldName];
		editorField.field = field;
		editorField.SetValue(data);
	};

	auto SetDatas = [&](const std::string& fieldName, ScriptField field, std::pair<uint64_t, uint64_t>& data) {
		EditorField& editorField = (*editorFields)[fieldName];
		editorField.field = field;
		editorField.SetValues(data.first, data.second);
	};

	auto SwitchData = [&](const std::string& fieldName, ScriptField field, std::string _data = "")
	{
		switch (field.type)
		{
		case ScriptFieldType::Float:
		{
			float data;
			if (_data == "")
				ScriptEngine::GetFieldValue(*scriptClass, field.classField, (void*)&data);
			else
				data = std::stof(_data);

			SetData(fieldName, field, data);
			break;
		}
		case ScriptFieldType::Int:
		{
			int data;
			if (_data == "")
				ScriptEngine::GetFieldValue(*scriptClass, field.classField, (void*)&data);
			else
				data = std::atoi(_data.c_str());

			SetData(fieldName, field, data);
			break;
		}
		case ScriptFieldType::Bool:
		{
			bool data;
			if (_data == "")
				ScriptEngine::GetFieldValue(*scriptClass, field.classField, (void*)&data);
			else
				data = (bool)std::atoi(_data.c_str());

			SetData(fieldName, field, data);
			break;
		}
		case ScriptFieldType::Vector2:
		{
			Vector2 data;
			if (_data == "")
				ScriptEngine::GetFieldValue(*scriptClass, field.classField, (void*)&data);
			else
				data = Math::ParseVector2(_data);

			SetData(fieldName, field, data);
			break;
		}
		case ScriptFieldType::Vector3:
		{
			Vector3 data;
			if (_data == "")
				ScriptEngine::GetFieldValue(*scriptClass, field.classField, (void*)&data);
			else
				data = Math::ParseVector3(_data);

			SetData(fieldName, field, data);
			break;
		}
		case ScriptFieldType::Vector4:
		{
			Vector4 data;
			if (_data == "")
				ScriptEngine::GetFieldValue(*scriptClass, field.classField, (void*)&data);
			else
				data = Math::ParseVector4(_data);

			SetData(fieldName, field, data);
			break;
		}
		case ScriptFieldType::GameObject:
		{
			if (_data == "")
				break;

			uint64_t data;
			data = std::stoull(_data);
			SetData(fieldName, field, data);
			break;
		}
		default:
		{
			if (field.type <= ScriptFieldType::GameObject)
				break;

			if (_data == "")
				break;

			std::stringstream ss(_data);
			std::string idStr;
			std::pair<uint64_t, uint64_t> data;
			std::getline(ss, idStr, ',');
			data.first = std::stoull(idStr);
			std::getline(ss, idStr, ',');
			data.second = std::stoull(idStr);

			SetDatas(fieldName, field, data);
		}
		}
	};

	while (getline(sceneFile, line) && line != "end")
	{
		std::string name;
		std::string value;

		std::string separator = " | ";
		size_t pos = line.find(separator);

		if (pos != std::string::npos)
		{
			// Extract the name and the word
			name = line.substr(0, pos);
			value = line.substr(pos + separator.length());
		}
		else
		{
			continue;
		}

		auto it = fields.find(name);
		if (it != fields.end())
		{
			std::string name = it->second.name;
			auto foundName = std::find_if(variables.begin(), variables.end(),
				[name](Readvariables variable) { return variable.name == name; });

			if (foundName != variables.end())
			{
				if (it->second.type == Scripting::Helper::ScriptFieldTypeFromString((*foundName).type))
				{
					SwitchData(name, it->second, value);
				}
			}
		}
	}
}