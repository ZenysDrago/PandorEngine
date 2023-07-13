#pragma once
#include <Components/BaseComponent.h>
namespace Component
{
	class VehicleComponent : public IComponent<VehicleComponent>
	{
	public:
		VehicleComponent();
		~VehicleComponent();
		
		void ShowInInspector();

		void Update() override;

		void GameUpdate() override;

		std::string GetComponentName() { return "Vehicle Component"; }

		std::ostream& operator<<(std::ostream& os) override;

		void ReadComponent(std::fstream& sceneFile) override;

	private:
		void SetVelocity(const Vector3& velocity);
		void SetAngularVelocity(const Vector3& velocity);

		std::weak_ptr<class Rigidbody> m_frontrightwheel;
		std::weak_ptr<class Rigidbody> m_rearrightwheel;
		std::weak_ptr<class Rigidbody> m_rearleftwheel;
		std::weak_ptr<class Rigidbody> m_frontleftwheel;

		float m_speed = 2000.f;

		std::array<int, 4> m_waitingRB = { -1, -1, -1, -1 };

	};
}