#pragma once
#include "PandorAPI.h"
#include "Collider.h"

namespace Component
{
	class PANDOR_API SphereCollider : public Collider
	{
	public:
		SphereCollider();
		~SphereCollider();

		void Initialize() override;

		void Awake() override;

		void ShowInInspector() override;

		void GameUpdate() override;

		void EditorDraw() override;

		std::string GetComponentName() override { return "Sphere Collider"; }

		Component::BaseComponent* Clone() const;;

		std::ostream& operator<<(std::ostream& os) override;

		void ReadComponent(std::fstream& sceneFile) override;

		void SetRadius(float val) { m_radius = val; }
	private:
		Vector3 m_center;
		float m_radius = 1.f;
		friend Core::Wrapper::WrapperPhysic::PhysicManager;

	};
}