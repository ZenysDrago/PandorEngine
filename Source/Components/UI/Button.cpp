#include "pch.h"
#include <Components/UI/Button.h>

#include <Components/UI/RectTransform.h>
#include <Components/ScriptComponent.h>
#include <Scripting/ScriptEngine.h>
#include <Core/GameObject.h>
#include <Core/SceneManager.h>
#include <Core/Scene.h>
#include <Resources/Mesh.h>
#include <Resources/Material.h>
#include <Resources/Texture.h>
#include <Core/App.h>
#include <Render/Camera.h>
#ifndef PANDOR_GAME
#include <EditorUI/SceneWindow.h>
#include <EditorUI/EditorUIManager.h>
#endif


Component::UI::Button::~Button()
{
}

void Component::UI::Button::Initialize()
{
	UIElement::Initialize();
}

void Component::UI::Button::ShowInInspector()
{
	WrapperUI::Combo("Button Style", (int*)&m_mode, "None\0Color\0Sprite");
	switch (m_mode)
	{
	case Component::UI::ButtonMode::NONE:
		WrapperUI::ChangeTextureButton(m_normalTexture, "Texture");
		break;
	case Component::UI::ButtonMode::COLOR:
		WrapperUI::ChangeTextureButton(m_normalTexture, "Texture");
		WrapperUI::ColorEdit4("Normal Color", &m_defaultColor.x);
		WrapperUI::ColorEdit4("Highlighted Color", &m_highlightedColor.x);
		WrapperUI::ColorEdit4("Pressed Color", &m_pressedColor.x);
		break;
	case Component::UI::ButtonMode::SPRITE:
		WrapperUI::ChangeTextureButton(m_normalTexture, "Normal Texture");
		WrapperUI::ChangeTextureButton(m_highlightedTexture, "Highlighted Texture");
		WrapperUI::ChangeTextureButton(m_pressedTexture, "Pressed Texture");
		break;
	default:
		break;
	}
	if (WrapperUI::Button("Add"))
	{
		m_scriptsRef.push_back(std::make_tuple(nullptr, nullptr, ""));
	}
	TableFlags flags = TableFlags((int)TableFlags::Borders | (int)TableFlags::Resizable);
	if (WrapperUI::BeginTable("##", 3, flags))
	{
		for (int i = 0; i < m_scriptsRef.size(); i++) {
			WrapperUI::PushID(i);
			WrapperUI::TableNextRow();
			WrapperUI::TableNextColumn();
			Core::GameObject* object;
			std::string name = ((m_scriptsRef.size() > i && std::get<0>(m_scriptsRef[i])) ? std::get<0>(m_scriptsRef[i])->GetName() : "None") + " (GameObject)";
			if (WrapperUI::Button(name.c_str()))
				WrapperUI::OpenPopup("GameObjectList##");
			if (Popups::GameObjectPopup(object, "") && object)
			{
				if (m_scriptsRef.size() > i)
					std::get<0>(m_scriptsRef[i]) = object;
			}
			else if (WrapperUI::BeginDragDropTarget()) {
				if (const Payload* payload = WrapperUI::AcceptDragDropPayload("GAMEOBJECT")) {
					uint64_t payloadIndex = *(uint64_t*)payload->Data;
					object = gameObject->GetScene()->GetObjectByID(payloadIndex);
					if (m_scriptsRef.size() > i)
						std::get<0>(m_scriptsRef[i]) = object;
				}
				WrapperUI::EndDragDropTarget();
			}

			WrapperUI::TableNextColumn();

			std::string functionButtonName = ((m_scriptsRef.size() > i && !std::get<2>(m_scriptsRef[i]).empty()) ?  
				std::get<2>(m_scriptsRef[i]): "None") + " (Function)";

			if (WrapperUI::Button(functionButtonName.c_str()))
			{
				WrapperUI::OpenPopup("FunctionPopup");
			}
			if (WrapperUI::BeginPopup("FunctionPopup"))
			{
				if (m_scriptsRef.size() > i && std::get<0>(m_scriptsRef[i]))
				{
					for (auto& component : *std::get<0>(m_scriptsRef[i])->GetComponentsList())
					{
						if (WrapperUI::BeginMenu(component->GetComponentName().c_str()))
						{
							if (auto script = std::dynamic_pointer_cast<ScriptComponent>(component))
							{
								std::vector<MonoMethod*> methods = Scripting::ScriptEngine::GetScriptClass(script->name)->GetMethods();
								for (uint32_t j = 0; j < methods.size(); j++)
								{
									MonoMethod* method = methods[j];
									auto functionName = Scripting::ScriptEngine::GetMethodName(method);
									if (std::regex_search(functionName, std::regex(".ctor")))
										continue;
									if (WrapperUI::MenuItem(functionName.c_str()))
									{
										std::get<1>(m_scriptsRef[i]) = script.get();
										std::get<2>(m_scriptsRef[i]) = functionName;
									}
								}
							}
							WrapperUI::EndMenu();
						}
					}
				}
				WrapperUI::EndPopup();
			}
			WrapperUI::TableNextColumn();
			if (WrapperUI::Button("Delete"))
			{
				m_scriptsRef.erase(m_scriptsRef.begin() + i);
				WrapperUI::PopID();
				break;
			}
			WrapperUI::PopID();
		}
		WrapperUI::EndTable();
	}
}

void Component::UI::Button::GameUpdate()
{
	if (!m_rectTransform) {
		m_rectTransform = gameObject->GetOrAddComponent<RectTransform>();
		return;
	}
	auto rect = m_rectTransform->GetRectangle();
#ifndef PANDOR_GAME
	auto mousePos = Core::App::Get().GetEditorUIManager().GetSceneWindow().GetMousePosition();
#else
	auto mousePos = WrapperUI::GetMousePos() - Core::App::Get().window->GetWindowPos();
#endif
	if (mousePos >= rect.Min && mousePos <= rect.Max)
	{
		if (WrapperUI::IsMouseClicked(MouseButton::Left))
		{
			ChangeState(ButtonState::PRESSED);
			OnClick();
		}
		else if (WrapperUI::IsMouseDown(MouseButton::Left))
		{
			ChangeState(ButtonState::DOWN);
		}
		else
		{
			ChangeState(ButtonState::HOVERED);
		}
	}
	else
	{
		ChangeState(ButtonState::UP);
	}
}

void Component::UI::Button::EditorUpdate()
{
	if (m_material->GetTexture() != m_normalTexture)
		m_material->SetTexture(m_normalTexture);
}

void Component::UI::Button::OnClick()
{
	for (auto&& function : m_onClickFunctions)
	{
		function();
	}
}
void Component::UI::Button::Awake()
{
	Update();
}

void Component::UI::Button::Start()
{
	for (int i = 0; i < m_scriptsRef.size(); i++) {
		auto instance = std::get<1>(m_scriptsRef[i])->instance;
		auto scriptClass = instance->GetScriptClass();
		auto method = scriptClass->GetMethod(std::get<2>(m_scriptsRef[i]), 0);
		AddOnClickFunction(std::mem_fn(&Scripting::ScriptClass::InvokeMethodVoid), scriptClass, instance->GetManagedObject(), method);
	}
}

void Component::UI::Button::ChangeState(ButtonState newState)
{
	if (m_state == newState)
		return;
	m_state = newState;
	switch (m_state)
	{
	case Component::UI::ButtonState::UP:
		if (m_mode == ButtonMode::COLOR)
			m_material->SetDiffuse(m_defaultColor);
		else if (m_mode == ButtonMode::SPRITE)
			m_material->SetTexture(m_normalTexture);
		break;
	case Component::UI::ButtonState::DOWN:
	case Component::UI::ButtonState::PRESSED:
		if (m_mode == ButtonMode::COLOR)
			m_material->SetDiffuse(m_pressedColor);
		else if (m_mode == ButtonMode::SPRITE)
			m_material->SetTexture(m_pressedTexture);
		break;
	case Component::UI::ButtonState::HOVERED:
		if (m_mode == ButtonMode::COLOR)
			m_material->SetDiffuse(m_highlightedColor);
		else if (m_mode == ButtonMode::SPRITE)
			m_material->SetTexture(m_highlightedTexture);
		break;
	default:
		break;
	}
}

void Component::UI::Button::ReadComponent(std::fstream& sceneFile)
{
	std::string line;

	if (getline(sceneFile, line) && line != "end")
		m_mode = (ButtonMode)std::stoi(line);

	if (getline(sceneFile, line) && line != "end")
		SetDefaultColor(Math::ParseVector4(line));
	if (getline(sceneFile, line) && line != "end")
		m_highlightedColor = Math::ParseVector4(line);
	if (getline(sceneFile, line) && line != "end")
		m_pressedColor = Math::ParseVector4(line);

	if (getline(sceneFile, line) && line != "end")
		if (line != "nullptr")
			m_normalTexture = Resources::ResourcesManager::Get()->GetOrLoad<Resources::Texture>(line);
	if (getline(sceneFile, line) && line != "end")
		if (line != "nullptr")
			m_highlightedTexture = Resources::ResourcesManager::Get()->GetOrLoad<Resources::Texture>(line);
	if (getline(sceneFile, line) && line != "end")
		if (line != "nullptr")
			m_pressedTexture = Resources::ResourcesManager::Get()->GetOrLoad<Resources::Texture>(line);

	while (getline(sceneFile, line) && line != "end")
	{
		if (line != "nullptr")
		{
			uint64_t objectID = std::stoll(line);
			getline(sceneFile, line);
			uint64_t componentID = std::stoll(line);
			getline(sceneFile, line);
			m_waitingMethods.push_back(std::make_tuple(objectID, componentID, line));
			m_scriptsRef.push_back(std::make_tuple(nullptr, nullptr, ""));
		}
	}

	while (getline(sceneFile, line) && line != "end") {}
}

std::ostream& Component::UI::Button::operator<<(std::ostream& os)
{
	os << (int)m_mode << '\n';
	os << m_defaultColor << '\n';
	os << m_highlightedColor << '\n';
	os << m_pressedColor << '\n';

	if (m_normalTexture)
		os << m_normalTexture->GetPath() << '\n';
	else
		os << "nullptr" << '\n';
	if (m_highlightedTexture)
		os << m_highlightedTexture->GetPath() << '\n';
	else
		os << "nullptr" << '\n';
	if (m_pressedTexture)
		os << m_pressedTexture->GetPath() << '\n';
	else
		os << "nullptr" << '\n';

	for (int i = 0; i < m_scriptsRef.size(); i++)
	{
		if (std::get<0>(m_scriptsRef[i]))
			os << std::get<0>(m_scriptsRef[i])->uuid << '\n';
		else
			os << "nullptr" << '\n';
		if (std::get<1>(m_scriptsRef[i]))
			os << std::get<1>(m_scriptsRef[i])->uuid << '\n';
		else
			os << "nullptr" << '\n';
		if (!std::get<2>(m_scriptsRef[i]).empty())
			os << std::get<2>(m_scriptsRef[i]) << '\n';
		else
			os << "nullptr" << '\n';
	}
	os << "end" << '\n';
	return os;
}

void Component::UI::Button::Update()
{
	size_t done = 0;
	for (size_t i = 0; i < m_waitingMethods.size(); i++)
	{
		Core::GameObject* object = gameObject->GetScene()->GetObjectByID(std::get<0>(m_waitingMethods[i]));
		if (!object)
			continue;
		std::get<0>(m_scriptsRef[i]) = object;
		Component::ScriptComponent* script = dynamic_cast<Component::ScriptComponent*>(object->GetComponentByID(std::get<1>(m_waitingMethods[i])));
		if (!script)
			continue;
		std::get<1>(m_scriptsRef[i]) = script;
		MonoMethod* method = Scripting::ScriptEngine::GetScriptClass(script->name)->GetMethod(std::get<2>(m_waitingMethods[i]), 0);
		if (!method)
			continue;
		std::get<2>(m_scriptsRef[i]) = std::get<2>(m_waitingMethods[i]);
		done += 1;
	}
	if (done == m_waitingMethods.size())
	{
		m_waitingMethods.clear();
	}


	if (m_mode == ButtonMode::COLOR && m_state == ButtonState::UP)
		m_material->SetDiffuse(m_defaultColor);
	else if (m_mode != ButtonMode::COLOR && m_state == ButtonState::UP)
		m_material->SetDiffuse(Vector4(1, 1, 1, 1));
}
