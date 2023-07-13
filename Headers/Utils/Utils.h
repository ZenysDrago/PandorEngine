#pragma once
#include "PandorAPI.h"
#include <string>
#include <algorithm>
namespace Render
{
	struct Frustum;
}
namespace Component
{
	class Transform;
}
namespace Resources
{
	class Model;
	class Mesh;
}
namespace Core::Wrapper
{
	class WrapperWindow;
}
namespace Utils
{
	template<typename ... Args>
	std::string StringFormat(const std::string& format, Args ... args)
	{
		int size_s = std::snprintf(nullptr, 0, format.c_str(), args ...) + 1; // Extra space for '\0'
		if (size_s <= 0) { throw std::runtime_error("Error during formatting."); }
		auto size = static_cast<size_t>(size_s);
		std::unique_ptr<char[]> buf(new char[size]);
		std::snprintf(buf.get(), size, format.c_str(), args ...);
		return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
	}

	void PANDOR_API ToPath(std::string& path);

	float RandomFloat(float min, float max); 
	
	Vector3 RandomPointInSphere(float radius);

	Vector3 RandomDirection3D(float angle, Vector3 axis);

	void GetRandomPositionAndDirection(float radius, float angle, Vector3& position, Vector3& direction);

	Vector3 RandomDirectionCone(Vector3 axis, float topRadius, float apexAngle);

	Vector3 RandomPointOnCircle(float radius, Vector3 axis);
	
	Vector3 RandomPointOnRectangle(Vector3 scale);

	struct Volume
	{
		Volume() {}
		virtual bool isOnFrustum(const Render::Frustum& camFrustum, Component::Transform* transform) = 0;
		virtual bool isOnOrForwardPlane(const Math::MathUtils::Plane& plane) const = 0;
	};

	struct Sphere : public Volume
	{
		Vector3 center;
		float radius;

		Sphere(Vector3 _center, float _radius)
		{
			center = _center;
			radius = _radius;
		}

		Sphere(float _radius, Vector3 _center)
		{
			center = _center;
			radius = _radius;
		}


		bool isOnFrustum(const Render::Frustum& camFrustum, Component::Transform* transform) override;
		
		bool isOnOrForwardPlane(const Math::MathUtils::Plane& plane) const override
		{
			float distance = plane.GetDistanceToPoint(center);
			if (distance > -radius)
				return true;
			return false;
		}
		
		static Sphere* GenerateSphereBV(Resources::Mesh* model);


	};

	struct AABB : public Volume
	{
		Vector3 center{ 0.f, 0.f, 0.f };
		Vector3 extents{ 0.f, 0.f, 0.f };

		AABB(const Vector3& min, const Vector3& max)
			: Volume{},
			center{ (max + min) * 0.5f },
			extents{ max.x - center.x, max.y - center.y, max.z - center.z }
		{}

		AABB(const Vector3& inCenter, float iI, float iJ, float iK)
			: Volume{}, center{ inCenter }, extents{ iI, iJ, iK }
		{}

		bool isOnFrustum(const Render::Frustum& camFrustum, Component::Transform* transform) override;

		bool isOnOrForwardPlane(const Math::MathUtils::Plane& plane) const override;

		static AABB* GenerateAABB(Resources::Mesh* model);
	};
	std::vector<float> GetDemiSphereVertices(float radius, bool up);
	std::vector<float> GetCylinderVertices(float radius, float height);

	bool IsOnlyLetters(const char* str);

	std::string OpenFile(const char* filter, Core::Wrapper::WrapperWindow* window);

	std::string SaveFile(const char* filter, Core::Wrapper::WrapperWindow* window);
}