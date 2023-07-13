#pragma once
#include "PandorAPI.h"

#include <Math/Maths.h>

namespace Core
{
	class GameObject;
	namespace Wrapper::WrapperPhysic
	{
		class PhysicManager;
	}
}
namespace Component
{
	class Collider;
}
namespace Physic
{
	class PANDOR_API RaycastHit
	{
	private:
		bool m_hit = false;
		float m_distance;
		Component::Collider* m_collider;
		Math::Vector3 m_normal;
		Math::Vector3 m_position;
		uint32_t m_faceIndex;
	public :
		friend Core::Wrapper::WrapperPhysic::PhysicManager;

		// Boolean to check if the Ray cast hit
		bool Hit() { return m_hit; }

		// The GameObject hit.
		Component::Collider* GetCollider() { return m_collider; }

		// 	The distance to hit.
		float GetDistance() { return m_distance; }

		// Get the normal of the hit.
		Math::Vector3 GetNormal() { return m_normal; }

		// Get the position of the hit.
		Math::Vector3 GetPosition() { return m_position; }

		// Return the face index hit.
		uint32_t GetFaceIndex() { return m_faceIndex; }
	};
}

