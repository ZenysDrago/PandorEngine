#include "pch.h"
#include "Components/Constraint.h"
#include <Components/Rigidbody.h>
#include <Core/App.h>
#include <Core/Scene.h>
#include <Core/SceneManager.h>
#include <Core/Wrappers/WrapperPhysic.h>
#include <Core/GameObject.h>

Component::Constraint::~Constraint()
{
}

void Component::Constraint::OnDestroy()
{
	Core::App::Get().physic->DestroyJoint(this);
}

void Component::Constraint::Start()
{
	p_rigidbody = gameObject->GetComponent<Component::Rigidbody>();
	Update();
	if (!p_otherRigidbody.lock())
		return;
	Core::App::Get().physic->CreateJoint(this);
}

void Component::Constraint::Update()
{
	if (!p_otherRigidbody.lock() && waitingRigidbodyIndex != -1)
	{
		if (auto go = Core::SceneManager::Get()->GetScene()->GetSceneNode()->GetWithIndex(waitingRigidbodyIndex))
			p_otherRigidbody = go->GetComponentShared<Component::Rigidbody>();
	}
}

void Component::Constraint::EditorDraw()
{
	if (p_otherRigidbody.expired())
		return;
	WrapperRHI::Line::Get().LineWidth = 5.f;
	WrapperRHI::Line::Get().Draw(gameObject->transform->GetWorldPosition(), p_otherRigidbody.lock()->gameObject->transform->GetWorldPosition());
	WrapperRHI::Line::Get().LineWidth = 1.f;
}

void Component::Constraint::SetParameters()
{
	if (Initalized())
	{
		SetMinDistance(m_distanceMinMax.x);
		SetMaxDistance(m_distanceMinMax.y);
		SetStiffness(m_stifness);
		SetDamping(m_damping);
	}
}

void Component::Constraint::ShowInInspector()
{
	int type = (int)m_type;
	if (WrapperUI::Combo("Type", &type, "Fixed\0Distance\0Spring\0Revolute\0Spherical\0Prismatic"))
	{
		SetConstraintType((ConstraintType)type);
	}
	if (auto rb = Core::GameObject::ComponentDragDropButton<Rigidbody>(p_otherRigidbody.lock() ? p_otherRigidbody.lock()->gameObject->GetName().c_str() : "None", "Rigidbody"))
		p_otherRigidbody = rb;

	switch (m_type)
	{
	case Component::ConstraintType::Fixed:
		break;
	case Component::ConstraintType::Distance:
	{
		if (WrapperUI::DragFloatRange2("Min/Max Distance", &m_distanceMinMax.x, &m_distanceMinMax.y))
		{
			SetMinDistance(m_distanceMinMax.x);
			SetMaxDistance(m_distanceMinMax.y);
		}
		WrapperUI::Checkbox("Limit to Current Distance", &m_setLimitToDistance);
		break;
	}
	case Component::ConstraintType::Spring:
	{
		if (WrapperUI::DragFloatRange2("Min/Max Distance", &m_distanceMinMax.x, &m_distanceMinMax.y))
		{
			SetMinDistance(m_distanceMinMax.x);
			SetMaxDistance(m_distanceMinMax.y);
		}
		if (WrapperUI::DragFloat("Stiffness", &m_stifness))
		{
			SetStiffness(m_stifness);
		}
		if (WrapperUI::DragFloat("Damping", &m_damping))
		{
			SetDamping(m_damping);
		}
		WrapperUI::Checkbox("Limit to Current Distance", &m_setLimitToDistance);
		break;
	}
	case ConstraintType::Spherical:
	case ConstraintType::Revolute:
	{
		WrapperUI::Checkbox("Limit", &m_enableLimit);
		WrapperUI::BeginDisabled(!m_enableLimit);
		WrapperUI::DragFloatRange2("Min/Max Angle", &m_rotationalLimit.x, &m_rotationalLimit.y);
		WrapperUI::EndDisabled();
		break;
	}
	case ConstraintType::Prismatic:
	{
		WrapperUI::Checkbox("Limit", &m_enableLimit);
		WrapperUI::BeginDisabled(!m_enableLimit);
		WrapperUI::DragFloatRange2("Min/Max Distance", &m_distanceMinMax.x, &m_distanceMinMax.y);
		WrapperUI::EndDisabled();
		break;
	}
	default:
		break;
	}
}

std::ostream& Component::Constraint::operator<<(std::ostream& os)
{
	if (p_otherRigidbody.lock())
		os << p_otherRigidbody.lock()->gameObject->uuid << '\n';
	else
		os << "nullptr" << '\n';

	os << (int)m_type << '\n';
	os << m_distanceMinMax << '\n';
	os << m_stifness << '\n';
	os << m_damping << '\n';
	os << m_rotationalLimit << '\n';
	os << m_enableLimit << '\n';
	os << m_setLimitToDistance << '\n';
	return os;
}

void Component::Constraint::ReadComponent(std::fstream& sceneFile)
{
	std::string line;
	if (getline(sceneFile, line) && line != "end")
		if (line != "nullptr")
			waitingRigidbodyIndex = std::stoi(line);

	if (gameObject->GetScene()->GetSceneNode()) {
		if (auto go = gameObject->GetScene()->GetSceneNode()->GetWithIndex(waitingRigidbodyIndex))
		{
			if (auto rb = go->GetComponentShared<Rigidbody>())
				p_otherRigidbody = rb;
		}
	}

	if (getline(sceneFile, line) && line != "end")
		SetConstraintType((ConstraintType)std::stoi(line));

	if (getline(sceneFile, line) && line != "end")
		m_distanceMinMax = ParseVector2(line);

	if (getline(sceneFile, line) && line != "end")
		m_stifness = std::stof(line);

	if (getline(sceneFile, line) && line != "end")
		m_damping = std::stof(line);

	if (getline(sceneFile, line) && line != "end")
		m_rotationalLimit = ParseVector2(line);

	if (getline(sceneFile, line) && line != "end")
		m_enableLimit = std::stoi(line);

	if (getline(sceneFile, line) && line != "end")
		m_setLimitToDistance = std::stoi(line);

	while (getline(sceneFile, line) && line != "end") {}
}

void Component::Constraint::SetOther(std::shared_ptr<Rigidbody> rb)
{
	p_otherRigidbody = rb;
}
