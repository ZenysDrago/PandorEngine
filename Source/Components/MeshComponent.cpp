#include "pch.h"

#include <Components/MeshComponent.h>
#include <Core/App.h>

#include <Resources/Mesh.h>
#include <Resources/Material.h>
#include <Resources/Model.h>
#include <Resources/Shader.h>
#include <Resources/Texture.h>
#include <Render/Camera.h>
#include <Core/GameObject.h>
#include <Core/App.h>
#include <Core/Scene.h>
#include <Core/SceneManager.h>
#ifndef PANDOR_GAME
#include <EditorUI/EditorUIManager.h>
#include <EditorUI/Inspector.h>
#endif
#include <Resources/Model.h>
#include <Utils/Utils.h>
#include <Components/Collider.h>
#include <Core/Wrappers/WrapperRHI.h>
#include <LowRenderer/LightManager.h>
#include <Components/LightComponent.h>

Component::MeshComponent::MeshComponent()
{
}

Component::MeshComponent::~MeshComponent()
{
	m_mesh = nullptr;
}

void Component::MeshComponent::Update()
{
}

void Component::MeshComponent::Draw()
{
	if (!m_MeshPath.empty() && !m_mesh)
		SetMesh(Resources::ResourcesManager::Get()->Find<Resources::Mesh>(m_MeshPath));
}

void Component::MeshComponent::GameDraw()
{
	if (!m_mesh)
		return;

	bool onFrustum = m_mesh->IsVisible(Core::SceneManager::Get()->GetCurrentScene()->currentCamera, gameObject->transform);
	if (onFrustum)
		m_mesh->Render(gameObject->transform->GetModelMatrix() * Core::App::Get().sceneManager->GetCurrentScene()->GetVP(), gameObject->transform->GetModelMatrix(), this->m_materials);
}

void Component::MeshComponent::EditorDraw()
{
#ifndef PANDOR_GAME
	if (!m_mesh)
		return;

	bool onFrustum = m_mesh->IsVisible(Core::SceneManager::Get()->GetCurrentScene()->currentCamera, gameObject->transform);
	if (onFrustum && gameObject != Core::App::Get().GetEditorUIManager().GetInspector().GetGameObjectSelected())
		m_mesh->Render(gameObject->transform->GetModelMatrix() * Core::App::Get().sceneManager->GetCurrentScene()->GetVP(), gameObject->transform->GetModelMatrix(), this->m_materials, false, true);
#endif
}

void Component::MeshComponent::DrawShadow()
{
	if (!m_mesh)
		return;

	m_mesh->Render(gameObject->transform->GetModelMatrix() * m_mesh->ShadowVP(), gameObject->transform->GetModelMatrix(), this->m_materials, false, true, false, true);
	
}

void Component::MeshComponent::DrawPicking(int ID)
{
	Math::Matrix4 mat4 = this->gameObject->transform->GetModelMatrix() * Core::App::Get().sceneManager->GetCurrentScene()->GetVP();
	if (GetMesh())
		GetMesh()->PickingResource(mat4, m_materials, ID);
}

void Component::MeshComponent::SetMesh(Resources::Mesh* mesh)
{
	if (!mesh || !mesh->HasBeenSent())
		return;
	m_mesh = mesh;
	m_MeshPath = m_mesh->GetPath();
	m_ModelPath = m_mesh->GetModel()->GetPath();

	if (m_mesh->GetSubMeshes().size() != this->m_materials.size())
	{
		m_materials.resize(m_mesh->GetSubMeshes().size());
		for (auto&& mat : m_materials)
		{
			if (!mat)
				mat = Core::App::Get().resourcesManager->GetOrLoad<Resources::Material>(ENGINEPATH"/Materials/defaultmaterial.mat");
		}
	}
}

void Component::MeshComponent::AddMaterial(Resources::Material* material)
{
	this->m_materials.push_back(material);
}

void Component::MeshComponent::ShowInInspector()
{
	if (m_mesh)
		WrapperUI::TextUnformatted(m_mesh->GetPath().c_str());
	if (WrapperUI::Button("Change Mesh"))
	{
		WrapperUI::OpenPopup("Mesh Popup");
	}
	if (auto mesh = Core::App::Get().resourcesManager->MeshPopup("Mesh Popup"))
	{
		SetMesh(mesh);
	}

	if (!m_mesh) return;
	// Material List.
	static int SelectedRow = 0;
	if (WrapperUI::BeginTable("Materials", 3, TableFlags::Borders))
	{
		for (int row = 0; row < m_materials.size(); row++)
		{
			WrapperUI::TableNextRow();
			for (int column = 0; column < 3; column++)
			{
				WrapperUI::TableSetColumnIndex(column);
				switch (column)
				{
				case 0:
					WrapperUI::TextUnformatted(std::to_string(row).c_str());
					break;
				case 1:
					if (m_materials[row])
						WrapperUI::TextUnformatted(m_materials[row]->GetPath().c_str());
					else
						WrapperUI::TextUnformatted("Missing Material");
					break;
				case 2:
					WrapperUI::PushID(row);
					if (WrapperUI::Button("Change Material"))
					{
						SelectedRow = row;
						WrapperUI::OpenPopup("MaterialPopup");
					}
					WrapperUI::PopID();
					break;
				}
			}
		}
		WrapperUI::PushID(SelectedRow);
		if (auto mat = Core::App::Get().resourcesManager->MaterialPopup("MaterialPopup")) {
			m_materials[SelectedRow] = Resources::ResourcesManager::Get()->GetOrLoad<Resources::Material>(mat->GetPath());
		}
		WrapperUI::PopID();
		WrapperUI::EndTable();
	}

	// Material Components List.
	int index = 0;
	for (auto&& mat : m_materials)
	{
		if (!mat)
			continue;
		WrapperUI::PushID(index++);
		WrapperUI::Separator();
		if (WrapperUI::CollapsingHeader(mat->GetName().c_str())) {

			mat->ShowInInspector();
		}
		WrapperUI::PopID();
	}
}

std::ostream& Component::MeshComponent::operator<<(std::ostream& os)
{
	os << m_ModelPath;
	os << "\n";
	os << m_MeshPath;
	os << "\n";

	for (int i = 0; i < m_materials.size(); i++)
	{
		if (m_materials[i])
			m_materials[i]->operator<<(os);
	}

	return os;
}

void Component::MeshComponent::SetMeshToLoad(const std::string& path)
{
	m_MeshPath = path;
}

void Component::MeshComponent::ReadComponent(std::fstream& sceneFile)
{
	Resources::Model* model{};

	std::string line;

	if (getline(sceneFile, line))
	{
		if (line != "end") {
			model = Resources::ResourcesManager::Get()->GetOrLoad<Resources::Model>(line);
			m_ModelPath = line;
		}
		else {
			return;
		}
	}

	if (getline(sceneFile, line))
	{
		if (line != "end" && model)
			model->SetupMeshComp(line, this);
		else
			return;
	}

	while (getline(sceneFile, line) && line != "end")
	{
		if (getline(sceneFile, line) && line != "end" && line == "Material") {
			std::string line;

			getline(sceneFile, line);
			this->AddMaterial(Resources::ResourcesManager::Get()->GetOrLoad<Resources::Material>(line));
		}
	}
}

void Component::MeshComponent::OnTriggerEnter(class Collider* collider)
{
	//PrintError("Enter Trigger With %s", collider->gameObject->GetName().c_str());

}

void Component::MeshComponent::OnTriggerStay(class Collider* collider)
{
	//PrintError("Stay Trigger With %s", collider->gameObject->GetName().c_str());
}

void Component::MeshComponent::OnTriggerExit(class Collider* collider)
{
	//PrintError("Exit Trigger With %s", collider->gameObject->GetName().c_str());
}

void Component::MeshComponent::OnCollisionEnter(class Collider* collider)
{
	//PrintError("Enter Collide With %s", collider->gameObject->GetName().c_str());
}

void Component::MeshComponent::OnCollisionStay(class Collider* collider)
{
	//PrintError("Stay Collide With %s", collider->gameObject->GetName().c_str());

}

void Component::MeshComponent::OnCollisionExit(class Collider* collider)
{
	//PrintError("Exit Collide With %s", collider->gameObject->GetName().c_str());
}

