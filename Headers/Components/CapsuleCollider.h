#pragma once
#include <PandorAPI.h>
#include <Components/Collider.h>
namespace Core::Wrapper::WrapperRHI
{
	class Buffer;
}
namespace Component
{
	class CapsuleCollider : public Collider
	{
	public:
		CapsuleCollider();
		~CapsuleCollider();

		void Initialize() override;

		void Awake() override;

		void ShowInInspector() override;

		void GameUpdate() override;

		void EditorDraw() override;

		std::string GetComponentName() override { return "Capsule Collider"; }

		BaseComponent* Clone() const;

		void ReadComponent(std::fstream& sceneFile) override;

		std::ostream& operator<<(std::ostream& os) override;
	private:
		Core::Wrapper::WrapperRHI::Buffer* m_buffers[3];
		std::vector<float> m_vertices[3];

		Math::Vector3 m_center = { 0 };
		float m_height = 1.f;
		float m_radius = 1.f;
		friend Core::Wrapper::WrapperPhysic::PhysicManager;
	};
}