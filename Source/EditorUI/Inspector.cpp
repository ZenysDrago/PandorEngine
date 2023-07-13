#include "pch.h"

#include <EditorUI/Inspector.h>
#include <EditorUI/File.h>
#include <EditorUI/EditorUIManager.h>

#include <Core/GameObject.h>
#include <Components/BaseComponent.h>
#include <Components/ComponentsData.h>

#include <Resources/ResourcesManager.h>
#include <Resources/Texture.h>
#include <Core/App.h>


EditorUI::Inspector::Inspector()
{
}

EditorUI::Inspector::~Inspector()
{
}

Component::BaseComponent* ComponentsPopup()
{
	WrapperUI::SetNextWindowSize({ 0.f, 250.f });
	Component::BaseComponent* out = nullptr;
	if (WrapperUI::BeginPopup("Components", WindowFlags::AlwaysVerticalScrollbar))
	{
		static TextFilter filter;
		filter.Draw("##");
		for (auto&& component : Component::ComponentsData::Get().Components)
		{
			if (!filter.PassFilter(component->GetComponentName().c_str()))
				continue;
			if (WrapperUI::Selectable(component->GetComponentName().c_str(), false, SelectableFlags::SpanAllColumns)) {
				out = component->Clone();
				WrapperUI::CloseCurrentPopup();
			}
		}
		WrapperUI::EndPopup();
	}
	return out;
}

void EditorUI::Inspector::Draw()
{
	if (!p_open)
		return;
	if (WrapperUI::Begin("Inspector", &p_open))
	{
		if (m_selected && m_selected->GetParent()) {
			WrapperUI::Text("%d", m_selected->uuid);

			WrapperUI::Checkbox("##active", m_selected->GetActive());
			WrapperUI::SameLine();
			WrapperUI::BeginDisabled(!m_selected->IsActive());
			// Name Input.
			char name[65];
			strcpy_s(name, 64, m_selected->GetName().substr(0, 64).c_str());

			if(m_selected->GetParent())
				WrapperUI::BeginDisabled(m_selected->GetParent()->isPrefab);

			if (WrapperUI::InputText("Name", name, 64, InputTextFlags::EnterReturnsTrue))
			{
				m_selected->SetName(name);
			}
			if (WrapperUI::CollapsingHeader("Transform", TreeNodeFlags::DefaultOpen))
			{
				m_selected->transform->ShowInInspector();
			}
			WrapperUI::NewLine();
			WrapperUI::Separator();

			WrapperUI::BeginDisabled(m_selected->isPrefab);

			int index = -1;
			for (auto&& component : m_selected->GetComponents())
			{
				bool destroy = true;
				WrapperUI::PushID(++index);
				WrapperUI::Checkbox("##", component->GetEnable());
				WrapperUI::SameLine();
				bool open = WrapperUI::CollapsingHeader(component->GetComponentName().c_str(), &destroy, (TreeNodeFlags)((int)TreeNodeFlags::AllowItemOverlap | (int)TreeNodeFlags::DefaultOpen));

				// Drag and Drop
				if (WrapperUI::BeginDragDropSource()) {
					std::pair<uint64_t, uint64_t> indices = std::make_pair(m_selected->uuid, component->uuid);
					WrapperUI::SetDragDropPayload("COMPONENT", &indices, sizeof(std::pair<uint64_t, int>));
					WrapperUI::Text(component->GetComponentName().c_str());
					WrapperUI::EndDragDropSource();
				}
				if (WrapperUI::BeginDragDropTarget()) {
					if (const Payload* payload = WrapperUI::AcceptDragDropPayload("COMPONENT")) {
						auto PayloadIndex = (*(std::pair<uint64_t, uint64_t>*)payload->Data).second;

						std::vector<std::shared_ptr<Component::BaseComponent>>* list = m_selected->GetComponentsList();
						auto it = std::find_if(list->begin(), list->end(), [PayloadIndex](std::shared_ptr<Component::BaseComponent> component) {
							return component->uuid == PayloadIndex;
							});

						if (it != list->end()) {
							uint64_t foundindex = std::distance(list->begin(), it);
							// Insert the element at its new position
							std::shared_ptr<Component::BaseComponent> element = (*list)[foundindex];
							list->insert(list->begin() + index, element);

							// Removes the element from its original position
							if (foundindex < index) {
								list->erase(list->begin() + foundindex);
							}
							else {
								list->erase(list->begin() + foundindex + 1);
							}

						}
					}
					WrapperUI::EndDragDropTarget();
				}

				if (open) {
					WrapperUI::BeginDisabled(!component->IsEnable());
					WrapperUI::TreePush(component->GetComponentName().c_str());
					component->ShowInInspector();
					WrapperUI::TreePop();
					WrapperUI::EndDisabled();
				}
				WrapperUI::NewLine();
				WrapperUI::Separator();
				WrapperUI::PopID();
				if (!destroy)
					component->RemoveFromGameObject();

			}
			WrapperUI::EndDisabled();
			WrapperUI::EndDisabled();

			WrapperUI::NewLine();

			/* Add possibility for the user to remove the ref to the prefab of the gameObject */
			if (m_selected->isPrefab)
			{
				bool isLinked = m_selected->isPrefab;
				if (isLinked)
				{
					if (WrapperUI::Checkbox("Link to prefab", &isLinked))
						WrapperUI::OpenPopup("Delete link to prefab");
				}

				if (WrapperUI::BeginPopupModal("Delete link to prefab", NULL, WindowFlags::AlwaysAutoResize))
				{
					WrapperUI::Text("WARNING /!\\");
					WrapperUI::Text("If you delete the link to the prefab you won't be able to set it back");
					if (WrapperUI::Button("Delete link to prefab")) {
						m_selected->UnSetPrefabObject();
						WrapperUI::CloseCurrentPopup();
					}
					if (WrapperUI::Button("Abort")) {
						WrapperUI::CloseCurrentPopup();
					}
					WrapperUI::EndPopup();
				}
			}

			if (!m_selected->isPrefab)
			{
				WrapperUI::NewLine();
				WrapperUI::SetCursorPosX(WrapperUI::GetWindowSize().x / 2 - 100);
				if (WrapperUI::Button("New Component", Math::Vector2(200, 0)))
				{
					WrapperUI::OpenPopup("Components");
				}
				if (auto comp = ComponentsPopup())
				{
					m_selected->AddComponent(comp);
				}
			}
			WrapperUI::EndDisabled();
		}
		else if (m_fileSelected)
		{
			if (m_fileSelected->icon) {
				WrapperUI::Image(m_fileSelected->icon->ID, Math::Vector2(64, 64));
				WrapperUI::SameLine();
			}
			WrapperUI::BeginGroup();
			WrapperUI::TextUnformatted(m_fileSelected->name.c_str());
			WrapperUI::TextUnformatted(m_fileSelected->directory.c_str());
			WrapperUI::EndGroup();
			WrapperUI::Separator();
			WrapperUI::NewLine();
			if (m_fileSelected->resourceLink && m_fileSelected->resourceLink->IsLoaded())
				m_fileSelected->resourceLink->ShowInInspector();
		}
	}
	WrapperUI::End();
}

void EditorUI::Inspector::SetGameObjectSelected(Core::GameObject* gameObject)
{
	if (m_selected)
		m_selected->SetSelected(false);
	if (gameObject)
		gameObject->SetSelected(true);
	m_selected = gameObject;
	m_fileSelected = nullptr;
}

void EditorUI::Inspector::SetFileSelected(std::shared_ptr<File> file)
{
	SetGameObjectSelected(nullptr);
	m_fileSelected = file;
}
