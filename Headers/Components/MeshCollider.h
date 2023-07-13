#pragma once
#include "PandorAPI.h"

#include <Components/BaseComponent.h>
#include <Components/Collider.h>

#include <Math/Maths.h>
namespace Component {
	class PANDOR_API MeshCollider : public Collider
	{
	public:
		MeshCollider();
		~MeshCollider() {}

		void Initialize() override;

		void Awake() override;

		void Update() override;

		void GameUpdate() override;

		void ShowInInspector() override;

		void EditorDraw() override;

		std::string GetComponentName() override { return "Mesh Collider"; }

		Component::BaseComponent* Clone() const override;;

		std::ostream& operator<<(std::ostream& os) override;

		void ReadComponent(std::fstream& sceneFile) override;
		void SetMesh(Resources::Mesh* val);
	private:
		bool m_isConvex = true;
		std::string m_meshToSet;
		friend Core::Wrapper::WrapperPhysic::PhysicManager;
	};
}

