#include "pch.h"

#include <Components\Transform.h>
#include <Components\Rigidbody.h>
#include <Core\GameObject.h>
#include <Core/Scene.h>
#include <Core/App.h>

Component::Transform::Transform()
{
}

Component::Transform::Transform(const Vector3& position, const Quaternion& rotation, const Vector3& scale /*= { 1, 1, 1 }*/)
{
	this->SetWorldPosition(position);
	this->SetWorldRotation(rotation);
	this->SetWorldScale(scale);
}

Component::Transform::~Transform()
{
}

void Component::Transform::ComputeModelMatrix()
{
	m_modelMatrix = GetLocalModelMatrix();
	m_dirty = false;
}

void Component::Transform::ComputeModelMatrix(const Math::Matrix4& parent)
{
	m_modelMatrix = GetLocalModelMatrix() * parent;
	m_dirty = false;
}

void Component::Transform::SetWorldPosition(const Math::Vector3& position, bool influenceChild /*= true*/)
{
	if (!gameObject)
	{
		SetLocalPosition(position);
		return;
	}

	std::vector<Vector3> childPos;
	childPos.resize(gameObject->GetChildrens().size());

	for (int i = 0; i < childPos.size(); i++)
	{
		childPos[i] = gameObject->GetChild(i)->transform->GetWorldPosition();
	}


	if (!gameObject->GetParent())
		SetLocalPosition(position);
	else {
		auto mat = gameObject->GetParent()->transform->GetModelMatrix().CreateInverse().GetTransposed();
		auto worldPos = mat * Vector4(position, 1.f);
		SetLocalPosition(worldPos);
	}

	if (!influenceChild)
	{
		for (int i = 0; i < childPos.size(); i++)
		{
			gameObject->GetChild(i)->transform->SetWorldPosition(childPos[i]);
		}
	}
	m_dirty = true;
}

void Component::Transform::SetWorldRotation(const Math::Quaternion& rotation, bool influenceChild /*= true*/)
{
	Quaternion parentRotation = GetWorldRotation();
	Vector3 parentPosition = GetWorldPosition();
	if (!gameObject || !gameObject->GetParent())
		m_localRotation = rotation;
	else
	{
		auto worldRotation = gameObject->GetParent()->transform->GetWorldRotation();
		SetLocalRotation(rotation * worldRotation.GetInverse());
	}
	// Don't work
	if (!influenceChild)
	{
		// Get the parent's current rotation and position
		// Iterate through all the child transforms
		for (auto& child : gameObject->GetChildrens())
		{
			// Get the child's current local position and rotation
			Vector3 localPosition = child->transform->GetLocalPosition();
			Quaternion localRotation = child->transform->GetLocalRotation();

			// Set the child's world position and rotation
			auto pos = parentPosition + parentRotation * localPosition;
			auto rot = parentRotation * localRotation;
			child->transform->SetWorldPosition(pos);
			child->transform->SetWorldRotation(rot);
		}
	}

	m_dirty = true;
}

void Component::Transform::SetWorldScale(const Math::Vector3& sca)
{
	if (!gameObject || !gameObject->GetParent())
		m_localScale = sca;
	else
	{
		auto worldScale = gameObject->GetParent()->transform->GetWorldScale();
		SetLocalScale(sca / worldScale);
	}
	m_dirty = true;
}

void Component::Transform::SetWorldTransform(const Math::Vector3& position, const Math::Quaternion& rotation, bool influenceChild /*= true*/)
{
	// Work !
	// Store the original child positions and rotations if influenceChild is false
	std::vector<Vector3> originalChildPositions;
	std::vector<Quaternion> originalChildRotations;
	auto childs = gameObject->GetChildrens();
	size_t size = childs.size();
	originalChildPositions.resize(size);
	originalChildRotations.resize(size);

	for (int i = 0; i < size; i++)
	{
		originalChildPositions[i] = childs[i]->transform->GetWorldPosition();
		originalChildRotations[i] = childs[i]->transform->GetWorldRotation();
	}

	// Calculate the local position and rotation of the GameObject relative to its parent
	Vector3 localPosition;
	if (gameObject->GetParent()) {
		auto mat = gameObject->GetParent()->transform->GetModelMatrix().CreateInverse(true);
		localPosition = mat * Vector4(position, 1.f);
	}
	else
		localPosition = position;

	Quaternion localRotation;
	if (gameObject->GetParent()) {
		auto inverse = gameObject->GetParent()->transform->GetWorldRotation().GetInverse();
		localRotation = inverse * rotation;
	}
	else
		localRotation = rotation;

	// Set the localPosition and localRotation properties of the Transform component
	SetLocalPosition(localPosition);
	SetLocalRotation(localRotation);

	ForceUpdate();

	if (!influenceChild)
	{
		for (int i = 0; i < size; i++)
		{
			if (childs[i]->GetComponent<Component::Rigidbody>())
				childs[i]->transform->SetWorldTransform(originalChildPositions[i], originalChildRotations[i]);
		}
	}
}

Math::Vector3 Component::Transform::GetWorldPosition()
{
	if (gameObject && gameObject->GetParent()) {
		return GetModelMatrix().GetPosition();
	}
	else
		return m_localPosition;
}

Math::Quaternion Component::Transform::GetWorldRotation()
{
	if (gameObject && gameObject->GetParent())
	{
		return gameObject->GetParent()->transform->GetWorldRotation() * m_localRotation;
	}
	return m_localRotation;
}

Math::Vector3 Component::Transform::GetWorldScale()
{
	if (gameObject && gameObject->GetParent())
	{
		return m_modelMatrix.GetScale();
	}
	else
		return m_localScale;
}

Math::Matrix4 Component::Transform::GetModelMatrix(bool update /*= false*/)
{
	if (m_dirty || update) {
		if (gameObject && gameObject->GetParent())
			return m_modelMatrix = gameObject->GetParent()->transform->GetModelMatrix() * GetLocalModelMatrix();
		return m_modelMatrix = GetLocalModelMatrix();
	}
	return m_modelMatrix;
}

void Component::Transform::SetLocalPosition(const Math::Vector3& position, bool influenceChild /*= true*/)
{
	// TO TEST
	if (!influenceChild)
	{
		for (auto& child : gameObject->GetChildrens())
		{
			auto worldPos = m_localPosition - position;
			auto childPosition = child->transform->GetLocalPosition();
			child->transform->SetLocalPosition(childPosition + worldPos);
		}
	}
	m_localPosition = position;
	m_dirty = true;
}

void Component::Transform::SetLocalRotation(const Math::Quaternion& newrot, bool influenceChild /*= true*/)
{
	// TO TEST
	if (!influenceChild)
	{
		for (auto& child : gameObject->GetChildrens())
		{
			auto worldRot = m_localRotation * newrot.GetInverse();
			auto childRotation = child->transform->GetLocalRotation();
			child->transform->SetLocalRotation(childRotation * worldRot);
		}
	}
	m_localRotation = newrot;
	m_localEulerRotation = newrot.ToEuler();
	m_dirty = true;
}

void Component::Transform::SetLocalRotation(const Math::Vector3& eulerAngle)
{
	m_localEulerRotation = eulerAngle;
	m_localRotation = m_localEulerRotation.ToQuaternion();
	m_dirty = true;
}

void Component::Transform::SetLocalScale(const Math::Vector3& newscale)
{
	m_localScale = newscale;
	m_dirty = true;
}

void Component::Transform::RotateArround(Vector3 target, Vector3 axis, float angle)
{
	Quaternion q = Quaternion::AngleAxis(angle, axis);
	Vector3 dif = GetWorldPosition() - target;
	dif = q * dif;
	SetWorldPosition(target + dif);
	Quaternion rot = this->GetWorldRotation();
	SetWorldRotation(rot * (rot.GetInverse() * q * rot));
}

Math::Vector3 Component::Transform::GetLocalPosition()
{
	return m_localPosition;
}

Math::Quaternion Component::Transform::GetLocalRotation()
{
	return m_localRotation;
}

Math::Vector3 Component::Transform::GetLocalScale()
{
	return m_localScale;
}

Math::Matrix4 Component::Transform::GetLocalModelMatrix()
{
	return Math::GetTransformMatrix(m_localPosition, m_localRotation, m_localScale);
}

Math::Vector3 Component::Transform::GetForwardVector()
{
	return GetWorldRotation() * Math::Vector3::Forward();
}

Math::Vector3 Component::Transform::GetRightVector()
{
	return GetWorldRotation() * Math::Vector3::Right();
}

Math::Vector3 Component::Transform::GetUpVector()
{
	return GetWorldRotation() * Math::Vector3::Up();
}

void Component::Transform::Update()
{
	if (!m_dirty)
		return;
	ForceUpdate();
}

void Component::Transform::ForceUpdate()
{
	if (gameObject && gameObject->GetParent())
		ComputeModelMatrix(gameObject->GetParent()->transform->GetModelMatrix());
	else
		ComputeModelMatrix();

	if (gameObject)
	{
		for (auto child : gameObject->GetChildrens())
		{
			child->transform->ForceUpdate();
		}
	}
}

void Component::Transform::ShowInInspector()
{
	Math::Vector3 position = m_localPosition;
	Math::Vector3 rotation = m_localEulerRotation;
	Math::Vector3 scale = m_localScale;

	WrapperUI::DrawVec3Control("Position", &position.x);
	WrapperUI::DrawVec3Control("Rotation", &rotation.x);
	WrapperUI::DrawVec3Control("Scale", &scale.x, 1.f, true);

	if (position != m_localPosition || rotation != m_localEulerRotation || scale != m_localScale) {
		SetLocalPosition(position);
		SetLocalRotation(rotation);
		SetLocalScale(scale);
	}

}

std::ostream& Component::Transform::operator<<(std::ostream& os)
{
	os << m_localPosition << "\n";
	os << m_localRotation << "\n";
	os << m_localScale << "\n";
	return os;
}

void Component::Transform::ReadComponent(std::fstream& sceneFile)
{
	std::string line;
	if (getline(sceneFile, line) && line != "end")
		SetLocalPosition(Math::ParseVector3(line));

	if (getline(sceneFile, line) && line != "end")
		SetLocalRotation(Math::ParseQuaternion(line));

	if (getline(sceneFile, line) && line != "end")
		SetLocalScale(Math::ParseVector3(line));


	while (getline(sceneFile, line) && line != "end") {}
}