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
	class PANDOR_API Collider : public IComponent<Collider>, public Core::Wrapper::WrapperPhysic::Shape
	{
	protected:
		std::weak_ptr<class Rigidbody>     p_rigidbody;
		Resources::Model*    p_model    = nullptr;
		Resources::Mesh*     p_mesh     = nullptr;
		Resources::Material* p_material = nullptr;
		Resources::PhysicMaterial* p_physicalMaterial = nullptr;

		bool p_isTrigger = false;

	public:
		Collider() {}

		void Initialize() override;

		void Start() override;

		void Update() override;

		void SetOffset(const Vector3& offset) override;

		void GameUpdate() override;

		void OnDestroy() override;

		void Draw() override;

		void ShowInInspector() override;

		std::string GetComponentName() override { return "Collider"; }
		friend Core::Wrapper::WrapperPhysic::PhysicManager;


		virtual std::ostream& operator<<(std::ostream& os) override;

		void ReadComponent(std::fstream& sceneFile) override;
		friend Core::Wrapper::WrapperPhysic::Shape;
	};
}