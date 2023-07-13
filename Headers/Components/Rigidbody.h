#pragma once
#include "PandorAPI.h"

#include <Components/BaseComponent.h>
#include <Math/Maths.h>
#include <Core/Wrappers/WrapperPhysic.h>

namespace Component {
	class PANDOR_API Rigidbody : public IComponent<Rigidbody>, public Core::Wrapper::WrapperPhysic::RigidDynamic
	{
	public:
		float mass = 10.f;
		Math::Vector3 velocity = { 0 };
		Math::Vector3 angularVelocity = { 0 };
		bool useGravity = true;
		bool isKinematic = false;
		bool fixedRotationX = false;
		bool fixedRotationY = false;
		bool fixedRotationZ = false;

	public:
		Rigidbody();
		~Rigidbody();

		void Awake() override;

		void Start() override;

		void ShowInInspector() override;
		
		void GameUpdate() override;

		void SetParameters();

		void OnDestroy();

		std::string GetComponentName() { return "Rigidbody"; }

		std::ostream& operator<<(std::ostream& os);

		void ReadComponent(std::fstream& sceneFile) override;

		BaseComponent* Clone() const;
	};
}

