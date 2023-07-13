#include "pch.h"

#include <Components/MeshCollider.h>
#include <Components/Rigidbody.h>
#include <Components/MeshComponent.h>
#include <Core/App.h>
#include <Core/GameObject.h>
#include <Core/Scene.h>
#include <Core/SceneManager.h>

#include <Resources/ResourcesManager.h>
#include <Resources/Model.h>
#include <Resources/Material.h>
#include <Resources/Mesh.h>
#include <Resources/PhysicMaterial.h>

#include <Render/Camera.h>

Component::MeshCollider::MeshCollider()
{
}

void Component::MeshCollider::Initialize()
{
	Collider::Initialize();
	if (!p_mesh) 
	{
		if (auto mesh = gameObject->GetComponent<MeshComponent>())
			p_mesh = mesh->GetMesh();
	}
}

void Component::MeshCollider::Awake()
{
	Collider::Start();
	if (!p_mesh && !m_meshToSet.empty())
	{
		SetMesh(Resources::ResourcesManager::Get()->Find<Resources::Mesh>(m_meshToSet));
	}
	Core::App::Get().physic->CreateMesh(this);
	if (p_rigidbody.lock())
		p_rigidbody.lock()->SetParameters();
}

void Component::MeshCollider::Update()
{
	if (!m_meshToSet.empty())
	{
		if (p_mesh)
		{
			m_meshToSet.clear();
		}
		else
		{
			SetMesh(Resources::ResourcesManager::Get()->Find<Resources::Mesh>(m_meshToSet));
		}
	}
}

void Component::MeshCollider::GameUpdate()
{
	Collider::SetOffset(0);
	SetScale(gameObject->transform->GetWorldScale());
}

void Component::MeshCollider::ShowInInspector()
{
	Collider::ShowInInspector();
	if (WrapperUI::Button("Mesh"))
	{
		WrapperUI::OpenPopup("MeshPopup");
	}
	if (auto mesh = Resources::ResourcesManager::Get()->MeshPopup("MeshPopup"))
	{
		SetMesh(ResourcesManager::Get()->Find<Resources::Mesh>(mesh->GetPath()));
	}
	WrapperUI::SameLine();
	WrapperUI::TextUnformatted(p_mesh != nullptr ? p_mesh->GetPath().c_str() : "None");
	WrapperUI::Checkbox("Is Convex", &m_isConvex);
}

void Component::MeshCollider::EditorDraw()
{
	if (p_mesh && (Core::App::Get().shouldDrawCollider || gameObject->IsSelected())) {
		bool onFrustum = p_mesh->IsVisible(Core::SceneManager::Get()->GetCurrentScene()->currentCamera, gameObject->transform);
		if (onFrustum) {
			Math::Matrix4 M = gameObject->transform->GetModelMatrix();
			Math::Matrix4 MVP = M * Core::App::Get().sceneManager->GetCurrentScene()->GetVP();
			p_mesh->Render(MVP, Math::Matrix4(), { p_material }, true, false);
		}
	}
}

Component::BaseComponent* Component::MeshCollider::Clone() const
{
	auto meshCol = new MeshCollider(*this);
	meshCol->p_rigidbody.reset();
	return meshCol;
}

std::ostream& Component::MeshCollider::operator<<(std::ostream& os)
{
	Collider::operator<<(os);

	if (p_mesh) {
		os << p_mesh->GetModel()->GetPath() << '\n';
		os << p_mesh->GetPath() << "\n";
	}
	else {
		os << "nullptr" << '\n';
		os << "nullptr" << "\n";
	}

	os << m_isConvex << "\n";
	return os;
}

void Component::MeshCollider::ReadComponent(std::fstream& sceneFile)
{
	Collider::ReadComponent(sceneFile);
	std::string line;

	if (getline(sceneFile, line) && line != "end")
		if (line != "nullptr")
			ResourcesManager::Get()->GetOrLoad<Resources::Model>(line);
	if (getline(sceneFile, line) && line != "end")
		if (line != "nullptr")
			m_meshToSet = line;

	if (getline(sceneFile, line) && line != "end")
		m_isConvex = std::stoi(line);

	while (getline(sceneFile, line) && line != "end") {}
}

void Component::MeshCollider::SetMesh(Resources::Mesh* val)
{
	if (!val)
		return;
	p_mesh = val;

}
