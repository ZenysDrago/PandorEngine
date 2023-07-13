#include "pch.h"
#include <Components\VehicleComponent.h>
#include <Components\Rigidbody.h>
#include <Components/Constraint.h>
#include <Core/GameObject.h>
#include <Core/SceneManager.h>
#include <Core/Scene.h>

Component::VehicleComponent::VehicleComponent()
{
}

Component::VehicleComponent::~VehicleComponent()
{
}

void Component::VehicleComponent::ShowInInspector()
{
	if (auto rigid = Core::GameObject::ComponentDragDropButton<Rigidbody>(m_frontrightwheel.lock() ? m_frontrightwheel.lock()->gameObject->GetName().c_str() : "None", "FrontRightWheel (Rigidbody)"))
	{
		m_frontrightwheel = rigid;
	}
	if (auto rigid = Core::GameObject::ComponentDragDropButton<Rigidbody>(m_rearrightwheel.lock() ? m_rearrightwheel.lock()->gameObject->GetName().c_str() : "None", "RearRightWheel (Rigidbody)"))
	{
		m_rearrightwheel = rigid;
	}
	if (auto rigid = Core::GameObject::ComponentDragDropButton<Rigidbody>(m_rearleftwheel.lock() ? m_rearleftwheel.lock()->gameObject->GetName().c_str() : "None", "RearLeftWheel (Rigidbody)"))
	{
		m_rearleftwheel = rigid;
	}
	if (auto rigid = Core::GameObject::ComponentDragDropButton<Rigidbody>(m_frontleftwheel.lock() ? m_frontleftwheel.lock()->gameObject->GetName().c_str() : "None", "FrontLeftWheel (Rigidbody)"))
	{
		m_frontleftwheel = rigid;
	}
	WrapperUI::DragFloat("Speed", &m_speed);
}

void Component::VehicleComponent::Update()
{
	for (int i = 0; i < m_waitingRB.size(); i++)
	{
		if (m_waitingRB[i] != -1)
		{
			switch (i)
			{
			case 0:
			{
				if (auto go = Core::SceneManager::Get()->GetCurrentNode()->GetWithIndex(m_waitingRB[i])) {
					m_frontrightwheel = go->GetComponentShared<Component::Rigidbody>();
				}
				if (m_frontrightwheel.lock())
				{
					m_waitingRB[i] = -1;
				}
				break;
			}
			case 1:
			{
				if (auto go = Core::SceneManager::Get()->GetCurrentNode()->GetWithIndex(m_waitingRB[i])) {
					m_rearrightwheel = go->GetComponentShared<Component::Rigidbody>();
				}
				if (m_rearrightwheel.lock())
				{
					m_waitingRB[i] = -1;
				}
				break;
			}
			case 2:
			{
				if (auto go = Core::SceneManager::Get()->GetCurrentNode()->GetWithIndex(m_waitingRB[i])) {
					m_rearleftwheel = go->GetComponentShared<Component::Rigidbody>();
				}
				if (m_rearleftwheel.lock())
				{
					m_waitingRB[i] = -1;
				}
				break;
			}
			case 3:
			{
				if (auto go = Core::SceneManager::Get()->GetCurrentNode()->GetWithIndex(m_waitingRB[i])) {
					m_frontleftwheel = go->GetComponentShared<Component::Rigidbody>();
				}
				if (m_frontleftwheel.lock())
				{
					m_waitingRB[i] = -1;
				}
				break;
			}
			default:
				break;
			}

		}
	}
}

void Component::VehicleComponent::GameUpdate()
{
	//if (m_rearlefm_scenetwheel.expired() || m_rearrightwheel.expired() || m_frontleftwheel.expired() || m_frontrightwheel.expired())
		//return;
	float angle = 33.f;
	float transitionTime = 2.f;
	static float holdingTime = 0;
	if (WrapperUI::IsKeyDown(Key::Key_UpArrow))
	{
		Quaternion world = gameObject->transform->GetWorldRotation();
		Vector3 Velocity = world * Vector3(m_speed, 0, 0);
		m_rearleftwheel.lock()->AddTorque(Velocity);
		m_rearrightwheel.lock()->AddTorque(Velocity);
	}
	else if (WrapperUI::IsKeyDown(Key::Key_DownArrow))
	{
		Quaternion world = gameObject->transform->GetWorldRotation();
		Vector3 Velocity = world * Vector3(-m_speed, 0, 0);
		m_rearleftwheel.lock()->AddTorque(Velocity);
		m_rearrightwheel.lock()->AddTorque(Velocity);

	}
	if (WrapperUI::IsKeyDown(Key::Key_Space))
	{
		Quaternion world = gameObject->transform->GetWorldRotation();
		auto angular = world.GetInverse() * m_rearleftwheel.lock()->GetAngularVelocity();
		Vector3 Velocity = world * Vector3(-angular.x, 0, 0) * 10.f;
		m_rearleftwheel.lock()->AddTorque(Velocity);
		m_rearrightwheel.lock()->AddTorque(Velocity);

	}
	if (WrapperUI::IsKeyReleased(Key::Key_LeftArrow) || WrapperUI::IsKeyReleased(Key::Key_RightArrow))
	{
		holdingTime = 0;
	}
	if (WrapperUI::IsKeyDown(Key::Key_LeftArrow))
	{
		Vector3 forceY = -Vector3::Up() * 1000.f;

		Quaternion world = gameObject->transform->GetWorldRotation();
		holdingTime += WrapperUI::GetDeltaTime() * transitionTime;

		Quaternion rot;
		if (!m_frontleftwheel.expired())
			rot = m_frontleftwheel.lock()->gameObject->GetParent()->GetParent()->transform->GetLocalRotation().ToEuler();
		float currentAngle = Arithmetics::Lerp(rot.y, angle, holdingTime);

		Math::Quaternion angleAxis = Quaternion::AngleAxis(currentAngle, Vector3(0, 1, 0)) * Quaternion::AngleAxis(-90, Vector3(0, 0, 1));
		if (!m_frontleftwheel.expired())
			m_frontleftwheel.lock()->gameObject->GetParent()->GetParent()->transform->SetLocalRotation(angleAxis);
		if (!m_frontrightwheel.expired())
			m_frontrightwheel.lock()->gameObject->GetParent()->GetParent()->transform->SetLocalRotation(angleAxis);

		if (!m_frontleftwheel.expired())
			m_frontleftwheel.lock()->AddForce(forceY);
		m_rearleftwheel.lock()->AddForce(forceY);
	}
	else if (WrapperUI::IsKeyDown(Key::Key_RightArrow))
	{
		Vector3 forceY = -Vector3::Up() * 1000.f;

		Quaternion world = gameObject->transform->GetWorldRotation();
		holdingTime += WrapperUI::GetDeltaTime() * transitionTime;

		Quaternion rot;
		if (!m_frontleftwheel.expired())
			rot = m_frontleftwheel.lock()->gameObject->GetParent()->GetParent()->transform->GetLocalRotation().ToEuler();
		float currentAngle = Arithmetics::Lerp(rot.y, -angle, holdingTime);
		Math::Quaternion angleAxis = Quaternion::AngleAxis(currentAngle, Vector3(0, 1, 0)) * Quaternion::AngleAxis(-90, Vector3(0, 0, 1));

		if (!m_frontleftwheel.expired())
			m_frontleftwheel.lock()->gameObject->GetParent()->GetParent()->transform->SetLocalRotation(angleAxis);
		if (!m_frontrightwheel.expired())
			m_frontrightwheel.lock()->gameObject->GetParent()->GetParent()->transform->SetLocalRotation(angleAxis);

		if (!m_frontrightwheel.expired())
			m_frontrightwheel.lock()->AddForce(forceY);
		m_rearrightwheel.lock()->AddForce(forceY);
	}
	else if (!m_frontleftwheel.expired() && !m_frontrightwheel.expired())
	{
		float currentAngle = 0.f;
		Math::Quaternion angleAxis = Quaternion::AngleAxis(currentAngle, Vector3(0, 1, 0)) * Quaternion::AngleAxis(-90, Vector3(0, 0, 1));
		m_frontleftwheel.lock()->gameObject->GetParent()->GetParent()->transform->SetLocalRotation(angleAxis);
		m_frontrightwheel.lock()->gameObject->GetParent()->GetParent()->transform->SetLocalRotation(angleAxis);
	}
}

std::ostream& Component::VehicleComponent::operator<<(std::ostream& os)
{
	if (m_frontrightwheel.lock())
		os << m_frontrightwheel.lock()->gameObject->uuid << '\n';
	else
		os << "nullptr" << '\n';
	if (m_rearrightwheel.lock())
		os << m_rearrightwheel.lock()->gameObject->uuid << '\n';
	else
		os << "nullptr" << '\n';
	if (m_rearleftwheel.lock())
		os << m_rearleftwheel.lock()->gameObject->uuid << '\n';
	else
		os << "nullptr" << '\n';
	if (m_frontleftwheel.lock())
		os << m_frontleftwheel.lock()->gameObject->uuid << '\n';
	else
		os << "nullptr" << '\n';

	os << m_speed << '\n';

	return os;
}

void Component::VehicleComponent::ReadComponent(std::fstream& sceneFile)
{
	std::string line;
	if (getline(sceneFile, line) && line != "end")
		if (line != "nullptr")
			m_waitingRB[0] = std::stoi(line);
	if (getline(sceneFile, line) && line != "end")
		if (line != "nullptr")
			m_waitingRB[1] = std::stoi(line);
	if (getline(sceneFile, line) && line != "end")
		if (line != "nullptr")
			m_waitingRB[2] = std::stoi(line);
	if (getline(sceneFile, line) && line != "end")
		if (line != "nullptr")
			m_waitingRB[3] = std::stoi(line);
	if (getline(sceneFile, line) && line != "end")
		m_speed = std::stof(line);

	while (getline(sceneFile, line) && line != "end") {}
}

void Component::VehicleComponent::SetVelocity(const Vector3& velocity)
{
	m_frontleftwheel.lock()->SetVelocity(velocity);
	m_frontrightwheel.lock()->SetVelocity(velocity);
	m_rearleftwheel.lock()->SetVelocity(velocity);
	m_rearrightwheel.lock()->SetVelocity(velocity);
}

void Component::VehicleComponent::SetAngularVelocity(const Vector3& velocity)
{
	m_frontleftwheel.lock()->SetAngularVelocity(velocity);
	m_frontrightwheel.lock()->SetAngularVelocity(velocity);
	m_rearleftwheel.lock()->SetAngularVelocity(velocity);
	m_rearrightwheel.lock()->SetAngularVelocity(velocity);
}
