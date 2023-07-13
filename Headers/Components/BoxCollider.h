#pragma once
#include "PandorAPI.h"

#include <Components/BaseComponent.h>
#include <Components/Collider.h>

#include <Math/Maths.h>
namespace Component {
	class PANDOR_API BoxCollider : public Collider
	{
	public:
		BoxCollider();

		void Initialize() override;

		void Awake() override;

		void ShowInInspector() override;

		void EditorDraw() override;

		void GameUpdate() override;

		virtual std::ostream& operator<<(std::ostream& os) override;

		std::string GetComponentName() override { return "Box Collider"; }

		BaseComponent* Clone() const;

		void ReadComponent(std::fstream& sceneFile) override;
	private:
		Math::Vector3 m_center = Vector3(0, 0, 0);
		Math::Vector3 m_extent = Vector3(1, 1, 1);
		friend Core::Wrapper::WrapperPhysic::PhysicManager;
	};
}

