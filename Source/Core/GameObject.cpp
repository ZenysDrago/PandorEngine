#include "pch.h"

#include <Core/GameObject.h>
#include <Core/App.h>
#include <Core/Scene.h>
#include <Core/SceneManager.h>
#include <Resources/Skeleton.h>
#include <Resources/Prefab.h>
#ifndef PANDOR_GAME
#include <EditorUI/EditorUIManager.h>
#include <EditorUI/Hierarchy.h>
#include <EditorUI/Inspector.h>
#include <EditorUI/CanvasWindow.h>
#endif
#include <Components/MeshComponent.h>
#include <Components/ComponentsData.h>
#include <Components/UI/Text.h>
#include <Components/ScriptComponent.h>
#include <Components/Rigidbody.h>
#include <Components/UI/Text.h>
#include <Components/UI/Button.h>
#include <Components/Animator.h>
#include <Components/SoundEmitter.h>
#include <Components/BoxCollider.h>
#include <Scripting/ScriptEngine.h>

using namespace Core::Wrapper;
using namespace Core;

Core::GameObject::GameObject()
{
	this->transform = new Component::Transform();
	this->transform->SetGameObject(this);
	m_scene = Core::App::Get().sceneManager->GetCurrentScene();
}

Core::GameObject::GameObject(std::string name)
{
	this->transform = new Component::Transform();
	this->transform->SetGameObject(this);
	this->SetName(name);
	m_scene = Core::App::Get().sceneManager->GetCurrentScene();
}

Core::GameObject::GameObject(GameObject* copied)
{
	std::fstream goFile;
	goFile.open("GO_tmp_" + m_name + ".pandor", std::fstream::trunc | std::fstream::out);

	if (goFile.is_open())
	{
		copied->operator<<(goFile);
	}

	goFile.close();

	goFile.open("GO_tmp_" + m_name + ".pandor");

	this->SetScene(copied->GetScene());

	this->transform = new Component::Transform();
	this->transform->SetGameObject(this);

	std::string line;
	if (goFile.is_open())
	{
		if (getline(goFile, line) && line == "===")
		{
			if (getline(goFile, line) && line == "GameObject")
				ReadGameObject(goFile, nullptr);
		}
	}

	goFile.close();

	try
	{
		std::filesystem::remove("GO_tmp_" + m_name + "pandor");
	}
	catch (const std::exception& e)
	{
		LOG(Debug::LogType::L_ERROR, "Error Delete file %s", e.what());
	}
}


Core::GameObject::~GameObject()
{
	while (!m_components.empty())
	{
		RemoveComponent(m_components[0].get());
	}
#ifndef	PANDOR_GAME
	if (Core::App::Get().GetEditorUIManager().GetInspector().GetGameObjectSelected() == this)
	{
		Core::App::Get().GetEditorUIManager().GetInspector().SetGameObjectSelected(nullptr);
	}
	if (Core::App::Get().GetEditorUIManager().GetCanvasWindow().GetCanvas() == this)
	{
		Core::App::Get().GetEditorUIManager().GetCanvasWindow().SetCanvas(nullptr);
	}
#endif

	if (Core::SceneManager::Get() && Core::SceneManager::Get()->GetCurrentScene())
		Core::SceneManager::Get()->GetCurrentScene()->RemoveObject(this);

	Scripting::ScriptEngine::UpdateObjectRefs();

	for (auto&& child : m_childrens)
	{
		delete child;
		child = nullptr;
	}
	delete transform;
	transform = nullptr;
}

void Core::GameObject::UpdateChildScene(GameObject* go)
{
	for (GameObject* g : go->m_childrens)
	{
		g->m_scene = go->m_scene;
		UpdateChildScene(g);
	}
}

void Core::GameObject::AddChildren(GameObject* child)
{
	child->m_parent = this;
	child->m_scene = m_scene;
	UpdateChildScene(child);
	m_childrens.push_back(child);
}

std::vector<Core::GameObject*> Core::GameObject::GetChildrens() const
{
	return m_childrens;
}

Core::GameObject* Core::GameObject::GetChild(size_t index) const
{
	if (m_childrens.size() > index)
		return m_childrens[index];
	else
		return nullptr;
}

void Core::GameObject::RemoveChildren(GameObject* child)
{
	auto it = std::find(m_childrens.begin(), m_childrens.end(), child);
	if (it != m_childrens.end())
	{
		delete child;
		child = nullptr;
		m_childrens.erase(it);
	}
}

Component::BaseComponent* Core::GameObject::AddComponent(Component::BaseComponent* component)
{
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


	m_components.insert(m_components.begin() + index, std::shared_ptr<Component::BaseComponent>(component));
	component->ShouldStart();
	return component;
}

void Core::GameObject::RemoveComponent(Component::BaseComponent* comp)
{
	int index = 0;
	if (!comp)
		return;
	for (auto& component : m_components)
	{
		if (component && component.get() == comp)
		{
			component->OnDestroy();
			component.reset();
			m_components.erase(m_components.begin() + index);
			comp->uuid = 0;
			Scripting::ScriptEngine::UpdateComponentRefs();
			comp = nullptr;
		}
		index++;
	}
}

void Core::GameObject::Awake()
{
	for (int i = 0; i < m_components.size(); ++i)
	{
		m_components[i]->Awake();
	}
	for (int i = 0; i < m_childrens.size(); ++i)
	{
		m_childrens[i]->Awake();
	}
}

void Core::GameObject::Start()
{
	for (int i = 0; i < m_components.size(); ++i)
	{
		m_components[i]->Start();
	}
	for (int i = 0; i < m_childrens.size(); ++i)
	{
		m_childrens[i]->Start();
	}
}


Core::GameObject* GameObject::GetSceneNode()
{
	return Core::SceneManager::Get()->GetCurrentNode();
}

void Core::GameObject::UpdateSelfAndChild(size_t& _index)
{
	if (!this->m_active)
		return;
	index = _index++;
	transform->Update();
	for (int i = 0; i < m_components.size(); i++)
	{
		if (!m_components[i]->IsEnable())
			continue;
		switch (Core::App::Get().GetGameState())
		{
		case GameState::Play:
			if (!GetScene()->IsPrefabScene())
				m_components[i]->GameUpdate();
			break;
		case GameState::Editor:
			m_components[i]->EditorUpdate();
			break;
		default:
			break;
		}
		m_components[i]->Update();
	}
	for (int i = 0; i < m_childrens.size(); i++)
	{
		m_childrens[i]->UpdateSelfAndChild(_index);
	}
}

void GameObject::UpdateIndex(size_t& _index)
{
	index = _index++;
	for (auto&& child : m_childrens)
	{
		child->UpdateIndex(_index);
	}
}

void Core::GameObject::OnCollisionStay(Component::Collider* collider)
{
	for (auto&& component : m_components)
	{
		component->OnCollisionStay(collider);
	}
}

void Core::GameObject::OnCollisionEnter(Component::Collider* collider)
{
	for (auto&& component : m_components)
	{
		component->OnCollisionEnter(collider);
	}
}

void Core::GameObject::OnCollisionExit(Component::Collider* collider)
{
	for (auto&& component : m_components)
	{
		component->OnCollisionExit(collider);
	}
}

void GameObject::OnTriggerEnter(Component::Collider* collider)
{
	for (auto&& component : m_components)
	{
		component->OnTriggerEnter(collider);
	}
}

void GameObject::OnTriggerStay(Component::Collider* collider)
{
	for (auto&& component : m_components)
	{
		component->OnTriggerStay(collider);
	}
}

void GameObject::OnTriggerExit(Component::Collider* collider)
{
	for (auto&& component : m_components)
	{
		component->OnTriggerExit(collider);
	}
}


void Core::GameObject::DrawModelForShadow()
{
	for (auto&& component : m_components)
	{
		Component::MeshComponent* meshComp = dynamic_cast<Component::MeshComponent*>(component.get());

		if (meshComp != NULL)
		{
			meshComp->DrawShadow();
		}

	}
	for (auto&& child : m_childrens)
	{
		child->DrawModelForShadow();
	}
}

void GameObject::DrawSelfAndChild(bool editorCamera /*= false*/)
{
	if (!this->m_active)
		return;
	transform->Draw();
	for (auto&& component : m_components)
	{
		if (!component->IsEnable())
			continue;
		if (!editorCamera)
			component->GameDraw();
		else
			component->EditorDraw();

		component->Draw();
	}
	for (auto&& child : m_childrens)
	{
		child->DrawSelfAndChild(editorCamera);
	}
}
void Core::GameObject::ShowInHierarchy()
{
#ifndef PANDOR_GAME
	WrapperUI::PushID((int)index);
	if (m_childrens.size() > 0) {
		if (!m_open) {
			if (WrapperUI::ArrowButton("##right", Dir::Right))
			{
				m_open = true;
			}
		}
		else if (m_open) {

			if (WrapperUI::ArrowButton("##down", Dir::Down))
			{
				m_open = false;
			}
		}
		WrapperUI::SameLine(0, 10);
	}
	else
	{
		WrapperUI::Button("#", Math::Vector2(19, 18));
		WrapperUI::SameLine();
	}
	if (WrapperUI::Selectable(m_name.c_str(), m_selected, (SelectableFlags)SelectableFlagsPrivate::SelectOnNav))
	{
		Core::App::Get().GetEditorUIManager().GetInspector().SetGameObjectSelected(this);
	}
	if (WrapperUI::IsItemHovered() && WrapperUI::IsMouseClicked(MouseButton::Right))
	{
		Core::App::Get().GetEditorUIManager().GetHierarchy().SetGameObjectRightClicked(this);
		this->m_open = true;
	}

	// Drag And Drop
	if (m_parent && WrapperUI::BeginDragDropSource()) {
		WrapperUI::SetDragDropPayload("GAMEOBJECT", &uuid, sizeof(uint64_t));
		WrapperUI::Text(this->m_name.c_str());
		WrapperUI::EndDragDropSource();
	}
	if (WrapperUI::BeginDragDropTarget()) {
		if (const Payload* payload = WrapperUI::AcceptDragDropPayload("GAMEOBJECT")) {
			uint64_t PayloadIndex = *(uint64_t*)payload->Data;
			GameObject* gameObject = Core::App::Get().sceneManager->GetCurrentNode()->GetWithIndex(PayloadIndex);
			if (gameObject && gameObject->m_parent && !IsAParent(gameObject))
			{
				if (!isPrefab)
				{
					gameObject->SetParent(this);
				}
				m_open = true;
			}
		}
		WrapperUI::EndDragDropTarget();
	}

	if (m_open)
	{
		for (auto&& child : m_childrens)
		{
			WrapperUI::TreePush(child->GetName().c_str());
			child->ShowInHierarchy();
			WrapperUI::TreePop();
		}
	}
	WrapperUI::PopID();
#endif
}

void Core::GameObject::SetName(const std::string& name)
{
	if (!m_nameLocked)
		m_name = name;
	else
		PrintWarning("The name of this GameObject can't be changed");
}

void Core::GameObject::SetParent(GameObject* node)
{
	if (m_parent) {
		// Remove this transform from the current parent's children list
		auto it = std::find(m_parent->m_childrens.begin(), m_parent->m_childrens.end(), this);
		if (it != m_parent->m_childrens.end())
		{
			m_parent->m_childrens.erase(it);
		}
	}

	// Set the new parent and add this transform to the new parent's children list
	m_parent = node;
	if (m_parent)
	{
		m_parent->AddChildren(this);
	}
	// Force Update the transform to adapt to the parent transform.
	transform->ForceUpdate();
}

void Core::GameObject::DrawPicking(int& ID)
{
	m_pickingID = ++ID;
	for (int i = 0; i < m_components.size(); i++)
	{
		m_components[i]->DrawPicking(ID);
	}

	for (int j = 0; j < m_childrens.size(); j++)
	{
		m_childrens[j]->DrawPicking(ID);
	}
}

void Core::GameObject::RemoveFromParent()
{
	if (!m_parent)
		return;

	GameObject* objToDelete = nullptr;
	if (isPrefab)
	{
		objToDelete = this;
		while (objToDelete->m_parent && objToDelete->m_parent->isPrefab)
		{
			objToDelete = objToDelete->m_parent;
		}
	}

	if (objToDelete && objToDelete != this)
	{
		objToDelete->RemoveFromParent();
	}
	else
	{
		m_parent->RemoveChildren(this);
		m_parent = nullptr;
	}
}

void Core::GameObject::UnSetPrefabAllChildren()
{
	for (Core::GameObject* go : m_childrens)
	{
		go->isPrefab = false;
		go->prefabPath = "";

		go->UnSetPrefabAllChildren();
	}
}

void Core::GameObject::UnSetPrefabObject()
{
	GameObject* curLoop = this;

	while (curLoop->m_parent && curLoop->m_parent->isPrefab)
	{
		curLoop = curLoop->m_parent;
	}

	curLoop->isPrefab = false;
	curLoop->prefabPath = "";
	curLoop->UnSetPrefabAllChildren();
}

std::vector<GameObject*> GameObject::GetAllChildren()
{
	std::vector<GameObject*> allChildren = {};

	// Add all direct children to the vector
	for (GameObject* child : m_childrens)
	{
		allChildren.push_back(child);

		// Recursively add all children of the current child to the vector
		std::vector<GameObject*> childChildren = child->GetAllChildren();
		allChildren.insert(allChildren.end(), childChildren.begin(), childChildren.end());
	}

	return allChildren;
}

Core::GameObject* GameObject::GetWithPickingIndex(int index)
{
	if (index == m_pickingID)
		return this;
	for (auto child : m_childrens)
	{
		auto node = child->GetWithPickingIndex(index);
		if (node != nullptr)
			return node;
	}
	return nullptr;
}

Core::GameObject* GameObject::GetRootPrefab()
{
	if (auto parent = GetParent())
	{
		if (parent->isPrefab)
		{
			return parent->GetRootPrefab();
		}
		else
		{
			if (!parent->isPrefab)
			{
				return this;
			}
		}
	}
	return this;
}

Core::GameObject* Core::GameObject::GetWithIndex(uint64_t _index)
{
	if (uuid == _index)
		return this;
	for (auto child : m_childrens)
	{
		auto node = child->GetWithIndex(_index);
		if (node != nullptr)
			return node;
	}
	return nullptr;
}

Component::BaseComponent* Core::GameObject::GetComponentByFieldType(const Scripting::ScriptField* scriptField)
{
	switch (scriptField->type)
	{
	case Scripting::ScriptFieldType::ScriptComponent:
		return Scripting::ScriptEngine::GetScriptFromType(this, *scriptField);
	case Scripting::ScriptFieldType::Rigidbody:
		return GetComponent<Component::Rigidbody>();
	case Scripting::ScriptFieldType::Text:
		return GetComponent<Component::UI::Text>();
	case Scripting::ScriptFieldType::Button:
		return GetComponent<Component::UI::Button>();
	case Scripting::ScriptFieldType::Animator:
		return GetComponent<Component::Animator>();
	case Scripting::ScriptFieldType::SoundEmitter:
		return GetComponent<Component::SoundEmitter>();
	case Scripting::ScriptFieldType::BoxCollider:
		return GetComponent<Component::BoxCollider>();
	default:
		return nullptr;
	}
}

Component::BaseComponent* Core::GameObject::GetComponentByID(const uint64_t ID)
{
	if (ID == 0)
		return nullptr;

	for (auto& component : m_components)
	{
		if (component->uuid == ID)
			return component.get();
	}

	return nullptr;
}

std::shared_ptr<Component::BaseComponent> GameObject::GetComponentSharedByID(const uint64_t ID)
{
	if (ID == 0)
		return nullptr;

	for (auto& component : m_components)
	{
		if (component->uuid == ID)
			return component;
	}

	return nullptr;
}

Core::GameObject* Core::GameObject::Instantiate(GameObject* parent /*= nullptr*/)
{
	GameObject* newInstance = Core::SceneManager::Get()->GetCurrentScene()->CreateObject(this->m_name + "(Clone)");

	for (auto& component : m_components) {
		Component::BaseComponent* newComponent = component->Clone();
		newInstance->AddComponent(newComponent);
	}
	for (auto& component : newInstance->m_components)
	{
		if (Core::App::Get().GetGameState() == GameState::Play)
		{
			component->Awake();
		}
	}
	for (auto& component : newInstance->m_components)
	{
		if (Core::App::Get().GetGameState() == GameState::Play)
		{
			component->Start();
		}
	}

	// Recursively instantiate child GameObjects
	for (int i = 0; i < m_childrens.size(); i++) {
		GameObject* newChild = m_childrens[i]->Instantiate(newInstance);
	}

	if (!parent)
		Core::SceneManager::Get()->GetCurrentNode()->AddChildren(newInstance);
	else
		parent->AddChildren(newInstance);
	return newInstance;
}

bool Core::GameObject::IsAParent(GameObject* gameObject)
{
	if (gameObject == this)
		return true;
	else if (m_parent)
		return m_parent->IsAParent(gameObject);
	return false;
}

void Core::GameObject::OnDestroy()
{
	for (int i = 0; i < m_components.size(); i++)
	{
		m_components[i]->OnDestroy();
		if (m_components.size() <= i)
			break;
	}
}

bool Core::GameObject::IsSelected()
{
	return m_selected;
}

void Core::GameObject::ReadGameObject(std::fstream& sceneFile, Scene* scene)
{
	std::string line;

	if (getline(sceneFile, line) && line != "end")
		GetScene()->ChangeIndexObjectList(this, std::stoull(line));

	if (getline(sceneFile, line) && line != "end")
	{
		bool* b = GetActive();
		*b = (bool)stoi(line);
	}

	if (getline(sceneFile, line) && line == "===")
	{
		getline(sceneFile, line);
		transform->ReadComponent(sceneFile);
		if (!transform->gameObject)
			transform->gameObject = this;
	}

	if (getline(sceneFile, line) && line != "end")
		isPrefab = (bool)stoi(line);

	if (isPrefab)
	{
		if (getline(sceneFile, line) && line != "end")
			prefabPath = line;

		Resources::Prefab* prefab = ResourcesManager::Get()->GetOrLoad<Resources::Prefab>(prefabPath);
		if (prefab)
			prefab->LoadGameObjectAsPrefab(this);

		while (getline(sceneFile, line) && line != "end") {}

		return;
	}

	if (getline(sceneFile, line) && line != "end")
		SetName(line);


	if (getline(sceneFile, line) && line == "basecomp")
	{
		while (getline(sceneFile, line) && line != "childrens" && line != "end")
		{
			if (line == "===") continue;

			for (auto&& component : Component::ComponentsData::Get().Components)
			{
				if (line == component->GetComponentName())
				{
					auto newComponent = component->Clone();
					AddComponent(newComponent)->ReadComponent(sceneFile);
					break;
				}
			}
		}
	}

	if (line == "childrens")
	{
		while (getline(sceneFile, line) && line != "end")
		{
			if (getline(sceneFile, line) && line != "end" && line == "GameObject")
			{
				GameObject* gb;
				if (scene)
					gb = scene->CreateObject();
				else
					gb = new GameObject();

				gb->ReadGameObject(sceneFile, scene);
				this->AddChildren(gb);
			}
			else if (line != "end" && line == "Bone")
			{
				Bone* gb;
				if (scene)
					gb = scene->CreateBoneObject();
				else
					gb = new Bone();
				gb->ReadGameObject(sceneFile, scene);
				this->AddChildren(gb);
			}
		}
	}
}

std::ostream& Core::GameObject::operator<<(std::ostream& os)
{
	os << "===" << "\n";
	os << "GameObject" << "\n";
	os << uuid << '\n';
	os << m_active << '\n';
	os << "===" << "\n";
	os << transform->GetComponentName() << "\n";
	transform->operator<<(os);
	os << "end" << "\n";

	os << isPrefab << "\n";

	if (isPrefab && prefabPath != "") {
		os << prefabPath << "\n";
		os << "end" << "\n";
		return os;
	}

	os << m_name << '\n';

	os << "basecomp" << "\n";
	for (auto& baseComp : m_components)
	{
		os << "===" << "\n";
		os << baseComp->GetComponentName() << "\n";
		baseComp->operator<<(os);
		os << "end" << "\n";
	}

	os << "childrens" << "\n";
	for (GameObject* gameObject : m_childrens)
	{
		gameObject->operator<<(os);
	}

	os << "end" << "\n";
	return os;
}
