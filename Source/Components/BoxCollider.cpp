#include "pch.h"

#include <Components/BoxCollider.h>
#include <Components/Rigidbody.h>
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

Component::BoxCollider::BoxCollider()
{
}

void Component::BoxCollider::Initialize()
{
	Collider::Initialize();
	p_model = Resources::ResourcesManager::Get()->GetOrLoad<Resources::Model>(ENGINEPATH"Models/Cube.fbx");
}

void Component::BoxCollider::Awake()
{
	Collider::Start();
	Core::App::Get().physic->CreateCube(this);
	if (p_rigidbody.lock().get())
		p_rigidbody.lock().get()->SetParameters();
}

void Component::BoxCollider::ShowInInspector()
{
	Collider::ShowInInspector();
	if (WrapperUI::DragFloat3("Center", &m_center.x, 0.1f))
	{
		SetOffset(m_center);
	}
	WrapperUI::DragFloat3("Extent", &m_extent.x, 0.1f);
}

void Component::BoxCollider::EditorDraw()
{
	if (p_model && !p_mesh && p_model->GetMeshes().size() > 0)
		p_mesh = p_model->GetMeshes()[0];
	if (p_mesh && (Core::App::Get().shouldDrawCollider || gameObject->IsSelected())) {
		Math::Matrix4 M = Math::GetScaleMatrix(m_extent) * Math::GetTranslationMatrix(m_center) * gameObject->transform->GetModelMatrix();
		Math::Matrix4 MVP = M * Core::App::Get().sceneManager->GetCurrentScene()->GetVP();
		p_mesh->Render(MVP, Math::Matrix4(), { p_material }, true, false);
	}
}

void Component::BoxCollider::GameUpdate()
{
	Collider::SetOffset(m_center);
	SetScale(gameObject->transform->GetWorldScale() * m_extent);
}

std::ostream& Component::BoxCollider::operator<<(std::ostream& os)
{	
	Collider::operator<<(os);

	os << m_center << "\n";
	os << m_extent << "\n";

	return os;
}

Component::BaseComponent* Component::BoxCollider::Clone() const
{
	auto box = new BoxCollider(*this);
	box->p_rigidbody.reset();
	return box;
}

void Component::BoxCollider::ReadComponent(std::fstream& sceneFile)
{
	std::string line;

	Collider::ReadComponent(sceneFile);

	if (getline(sceneFile, line) && line != "end")
		m_center = Math::ParseVector3(line);
	if (getline(sceneFile, line) && line != "end")
		m_extent = Math::ParseVector3(line);

	while (getline(sceneFile, line) && line != "end") {}

}
