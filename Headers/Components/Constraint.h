#pragma once
#include "PandorAPI.h"

#include <Components/BaseComponent.h>

#include <Core/Wrappers/WrapperPhysic.h>

namespace Resources
{
	class Model;
	class Mesh;
	class Material;
	class PhysicMaterial;
}

namespace Component
{
	enum class ConstraintType : int
	{
		Fixed,
		Distance,
		Spring,
		Revolute,
		Spherical,
		Prismatic,

	};

	class PANDOR_API Constraint : public IComponent<Constraint>, public Core::Wrapper::WrapperPhysic::Joint
	{
	protected:
		class Rigidbody* p_rigidbody;
		class std::weak_ptr<Rigidbody> p_otherRigidbody;

		int waitingRigidbodyIndex = -1;

		ConstraintType m_type = ConstraintType::Fixed;

		Vector2 m_distanceMinMax = { 0 ,100.f };
		float m_stifness = 1000.f;
		float m_damping = 100.f;

		bool m_enableLimit = false;
		Vector2 m_rotationalLimit = { -45.f, 45.f };

		bool m_setLimitToDistance = true;
	public:
		Constraint() {}
		~Constraint();

		void OnDestroy() override;

		void Start() override;

		void Update() override;

		void EditorDraw() override;

		void SetParameters();

		void ShowInInspector() override;

		std::string GetComponentName() override { return "Constraint"; }

		virtual std::ostream& operator<<(std::ostream& os) override;

		void ReadComponent(std::fstream& sceneFile) override;

		void SetOther(std::shared_ptr<Rigidbody> rb);

		friend Core::Wrapper::WrapperPhysic::PhysicManager;

		void SetConstraintType(Component::ConstraintType val) { m_type = val; }
	};
}