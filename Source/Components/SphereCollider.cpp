#include "pch.h"
#include <Components/SphereCollider.h>
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

Component::SphereCollider::SphereCollider()
{
}

Component::SphereCollider::~SphereCollider()
{
}

void Component::SphereCollider::Initialize()
{
	Collider::Initialize();
	p_model = Resources::ResourcesManager::Get()->GetOrLoad<Resources::Model>(ENGINEPATH"Models/Sphere.obj");
}

void Component::SphereCollider::Awake()
{
	Collider::Start();
	Core::App::Get().physic->CreateSphere(this);
	if (p_rigidbody.lock().get())
		p_rigidbody.lock().get()->SetParameters();
	SetOffset(m_center);
}

void Component::SphereCollider::ShowInInspector()
{
	Collider::ShowInInspector();
	if (WrapperUI::DragFloat3("Center", &m_center.x, 0.1f))
	{
		SetOffset(m_center);
	}
	WrapperUI::DragFloat("Radius", &m_radius, 0.1f);
}

void Component::SphereCollider::GameUpdate()
{
	Collider::SetOffset(m_center);
	SetScale(gameObject->transform->GetWorldScale() * m_radius);
}

void Component::SphereCollider::EditorDraw()
{
	if (p_model && !p_mesh && p_model->GetMeshes().size() > 0)
		p_mesh = p_model->GetMeshes()[0];
	if (p_mesh && (Core::App::Get().shouldDrawCollider || gameObject->IsSelected())) {
		Math::Matrix4 M = Math::GetScaleMatrix(Vector3(m_radius)) * Math::GetTranslationMatrix(m_center) * gameObject->transform->GetModelMatrix();
		Math::Matrix4 MVP = M * Core::App::Get().sceneManager->GetCurrentScene()->GetVP();
		p_mesh->Render(MVP, Math::Matrix4(), { p_material }, true, false);
	}
}

Component::BaseComponent* Component::SphereCollider::Clone() const
{
	auto sphere = new SphereCollider(*this);
	sphere->p_rigidbody.reset();
	return sphere;
}

std::ostream& Component::SphereCollider::operator<<(std::ostream& os)
{
	Collider::operator<<(os);
	os << m_center << '\n';
	os << m_radius << "\n";
	return os;
}

void Component::SphereCollider::ReadComponent(std::fstream& sceneFile)
{
	Collider::ReadComponent(sceneFile);
	std::string line;

	if (getline(sceneFile, line) && line != "end")
		m_center = Math::ParseVector3(line);

	if (getline(sceneFile, line) && line != "end")
		SetRadius(std::stof(line));

	while (getline(sceneFile, line) && line != "end") {}
}
