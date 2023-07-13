#pragma once
#include "PandorAPI.h"

#include "BaseComponent.h"
#include <Math/Maths.h>

namespace Core
{
	class GameObject;
	class Scene;
}
namespace Component
{
	class PANDOR_API Transform : public IComponent<Transform>
	{
	private:
		Math::Matrix4    m_modelMatrix        = Math::Matrix4();
		Math::Vector3    m_localPosition      = Math::Vector3();
		Math::Quaternion m_localRotation      = Math::Quaternion();
		Math::Vector3    m_localEulerRotation = Math::Vector3();
		Math::Vector3    m_localScale         = Math::Vector3(1);
		bool m_dirty = true;
		bool was_dirty = false;
	public:
		Transform();
		Transform(const Vector3& position, const Quaternion& rotation, const Vector3& scale = { 1, 1, 1 });
		~Transform();
		
		// Nodes
		void ComputeModelMatrix();
		void ComputeModelMatrix(const Math::Matrix4&);

		// World Setters
		void SetWorldPosition(const Math::Vector3& position, bool influenceChild = true);
		void SetWorldRotation(const Math::Quaternion& rotation, bool influenceChild = true);
		void SetWorldScale(const Math::Vector3& scale);
		void SetWorldTransform(const Math::Vector3& position, const Math::Quaternion& rotation, bool influenceChilde = true);

		// World Getters
		Math::Vector3 GetWorldPosition();
		Math::Quaternion GetWorldRotation();
		Math::Vector3 GetWorldScale();
		Math::Matrix4 GetModelMatrix(bool update = false);

		// Local Setters
		void SetLocalPosition(const Math::Vector3&, bool influencChild = true);
		void SetLocalRotation(const Math::Vector3& eulerAngle);
		void SetLocalRotation(const Math::Quaternion&, bool influenceChild = true);
		void SetLocalScale(const Math::Vector3&);

		void SetModelMatrix(const Matrix4& matrix) { m_modelMatrix = matrix; }

		void RotateArround(Vector3 target, Vector3 axis, float angle);

		// Local Getters
		Math::Vector3 GetLocalPosition();
		Math::Quaternion GetLocalRotation();
		Math::Vector3 GetLocalScale();
		Math::Matrix4 GetLocalModelMatrix();

		Math::Vector3 GetForwardVector();
		Math::Vector3 GetRightVector();
		Math::Vector3 GetUpVector();

		//void RotateAround(Math::Vector3 point, Math::Vector3 axis, float angle);

		void Update();
		void ForceUpdate();

		void ShowInInspector() override;

		bool IsDirty() { return m_dirty; }

		void SetDirty() { m_dirty = true; }

		// Update the transform to adapt with the parent transform
		void ChangeParent(Transform* newParent);

		std::string GetComponentName() override { return "Transform"; }
		std::ostream& operator<<(std::ostream& os) override;
		void ReadComponent(std::fstream& sceneFile) override;

		bool WasDirty() const { return was_dirty; }
	};
}