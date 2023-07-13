#include "pch.h"
#include <Components/CapsuleCollider.h>
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

Component::CapsuleCollider::CapsuleCollider()
{

}

Component::CapsuleCollider::~CapsuleCollider()
{
	delete m_buffers[0];
	delete m_buffers[1];
	delete m_buffers[2];
	m_buffers[0] = nullptr;
	m_buffers[1] = nullptr;
	m_buffers[2] = nullptr;
}

void Component::CapsuleCollider::Initialize()
{
	Collider::Initialize();
	m_vertices[0] = Utils::GetDemiSphereVertices(m_radius, true);
	m_vertices[1] = Utils::GetCylinderVertices(m_radius, 1.f);
	m_vertices[2] = Utils::GetDemiSphereVertices(m_radius, false);

	for (int i = 0; i < 3; i++) {
		m_buffers[i] = new Core::Wrapper::WrapperRHI::Buffer(m_vertices[i].data(), m_vertices[i].size());
		m_buffers[i]->LinkAttribute(0, 3, PR_FLOAT, 6 * sizeof(float), (void*)0);
		m_buffers[i]->LinkAttribute(2, 3, PR_FLOAT, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	}

}

void Component::CapsuleCollider::Awake()
{
	Collider::Start();
	Core::App::Get().physic->CreateCapsule(this);
	if (p_rigidbody.lock().get())
		p_rigidbody.lock().get()->SetParameters();
	SetOffset(m_center);
}

void Component::CapsuleCollider::ShowInInspector()
{
	Collider::ShowInInspector();
	if (WrapperUI::DragFloat3("Offset", &m_center.x, 0.1f))
	{
		SetOffset(m_center);
	}
	WrapperUI::DragFloat("Radius", &m_radius, 0.01f);
	WrapperUI::DragFloat("Height", &m_height, 0.01f);
}

void Component::CapsuleCollider::GameUpdate()
{
	Collider::SetOffset(m_center);
	SetScale(gameObject->transform->GetWorldScale() * Vector3(m_height, m_radius, m_radius));
}

void Component::CapsuleCollider::EditorDraw()
{
	if ((Core::App::Get().shouldDrawCollider || gameObject->IsSelected())) {
		Math::Matrix4 M = Math::GetTransformMatrix(m_center - Vector3(m_height, 0, 0), Quaternion{ 0.f, 0.f, 0.707f, 0.707f }, m_radius) * gameObject->transform->GetModelMatrix();
		Math::Matrix4 MVP = M * Core::App::Get().sceneManager->GetCurrentScene()->GetVP();
		p_material->GetShader()->Use();
		WrapperRHI::ShaderSendVec4(p_material->GetShader()->GetLocation("ourColor"), p_material->GetDiffuse());

		m_buffers[0]->Bind();
		WrapperRHI::ShaderSendMat4(p_material->GetShader()->GetLocation("MVP"), MVP);
		WrapperRHI::DrawArrays(0, m_vertices[0].size() / 3, true, false);

		M = Math::GetTransformMatrix(m_center + Vector3(m_height * 1.3f, 0, 0), Quaternion{ 0.f, 0.f, 0.707f, 0.707f }, m_radius) * gameObject->transform->GetModelMatrix();
		MVP = M * Core::App::Get().sceneManager->GetCurrentScene()->GetVP();

		m_buffers[2]->Bind();
		WrapperRHI::ShaderSendMat4(p_material->GetShader()->GetLocation("MVP"), MVP);
		WrapperRHI::DrawArrays(0, m_vertices[2].size() / 3, true, false);

		M = Math::GetTransformMatrix(m_center, Quaternion{ 0.f, 0.f, 0.707f, 0.707f }, Vector3(m_radius, m_height * 2.f, m_radius)) * gameObject->transform->GetModelMatrix();
		MVP = M * Core::App::Get().sceneManager->GetCurrentScene()->GetVP();

		m_buffers[1]->Bind();
		WrapperRHI::ShaderSendMat4(p_material->GetShader()->GetLocation("MVP"), MVP);
		WrapperRHI::DrawArrays(0, m_vertices[1].size() / 3, true, false);
	}
}

Component::BaseComponent* Component::CapsuleCollider::Clone() const
{
	auto cap = new CapsuleCollider(*this);
	cap->p_rigidbody.reset();
	return cap;
}

void Component::CapsuleCollider::ReadComponent(std::fstream& sceneFile)
{
	Collider::ReadComponent(sceneFile);
	std::string line;
	if (getline(sceneFile, line) && line != "end")
		m_center = ParseVector3(line);
	if (getline(sceneFile, line) && line != "end")
		m_height = std::stof(line);
	if (getline(sceneFile, line) && line != "end")
		m_radius = std::stof(line);


	while (getline(sceneFile, line) && line != "end") {}
}

std::ostream& Component::CapsuleCollider::operator<<(std::ostream& os)
{
	Collider::operator<<(os);
	os << m_center << '\n';
	os << m_height << '\n';
	os << m_radius << '\n';
	return os;
}
