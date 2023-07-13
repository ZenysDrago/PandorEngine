#pragma once

#include "Vector4.h"
#include "Quaternion.h" 
#include "Matrix.h"

namespace Math::MathUtils
{
	struct Raycast
	{
		Vector3 origin;
		Vector3 direction;
		float   scale = 0;
	};

	struct Circle
	{
		Vector3 center;
		Vector3 orientation;
		float   radius;
	};

	struct Plane
	{
		Vector3 normal = { 0, 1, 0 };
		float distance = 0.f;

		Plane() = default;
		Plane(Vector3 _normal, float _distance)
		{
			normal = _normal.GetNormalized();
			distance = _distance;
		}
		Plane(Vector3 _inPointdistance, Vector3 _normal)
		{
			normal = _normal.GetNormalized();
			distance = normal.Dot(_inPointdistance);
		}

		float GetDistanceToPoint(const Vector3& point) const
		{
			return normal.Dot(point) - distance;
		}
	};

	inline Vector3 IntersectRayPlane(const MathUtils::Plane& p, const MathUtils::Raycast& ray)
	{
		Vector3 planePoint = -p.normal * p.distance;
		float dotProduct = (ray.direction.Dot(p.normal));

		if (dotProduct == 0.f) // ray and plane are parallel
		{
			return Vector3(0.0f, 0.0f, 0.0f);
		}

		float distance = p.normal.Dot(planePoint - ray.origin) / dotProduct;

		if (distance < 0.f) // intersection behind the ray origin
		{
			return Vector3(0.0f, 0.0f, 0.0f);
		}

		return ray.origin + ray.direction * distance;
	}

	inline bool IntersectRayCircle(const MathUtils::Raycast& ray, const MathUtils::Circle& c, Vector3& insertPoint) {
		float t = c.orientation.Dot(c.center - ray.origin) / c.orientation.Dot(ray.direction);
		insertPoint = ray.origin + ray.direction * t;

		float distance = Vector3::Distance(insertPoint, c.center);

		return distance <= c.radius;
	}

	inline float ClosestDistanceBetweenLines(MathUtils::Raycast& l1, MathUtils::Raycast& l2)
	{
		Vector3 w0 = l1.origin - l2.origin; // vector from origin of l2 to origin of l1
		float a = l1.direction.Dot(l1.direction); // dot product of l1's direction with itself
		float b = l1.direction.Dot(l2.direction); // dot product of l1's direction with l2's direction
		float c = l2.direction.Dot(l2.direction); // dot product of l2's direction with itself
		float d = l1.direction.Dot(w0); // dot product of l1's direction with w0
		float e = l2.direction.Dot(w0); // dot product of l2's direction with w0
		float denom = a * c - b * b; // denominator of equation for t1 and t2
		l1.scale = (b * e - c * d) / denom; // parameter value for closest point on l1
		l2.scale = (a * e - b * d) / denom; // parameter value for closest point on l2
		Vector3 p1 = l1.origin + l1.direction * l1.scale; // closest point on l1
		Vector3 p2 = l2.origin + l2.direction * l2.scale; // closest point on l2
		return (p1 - p2).GetMagnitude(); // distance between closest points

	}

	inline Vector3 ClosestDistanceLineCircle(const MathUtils::Raycast& line, const MathUtils::Circle& c) {
		MathUtils::Plane f = MathUtils::Plane({ c.orientation.GetNormalized(), -c.orientation.Dot(c.center) });

		Vector3 insertPoint;
		if (IntersectRayCircle(line, c, insertPoint))
		{
			return c.center + (insertPoint - c.center).GetNormalized() * c.radius;
		}
		else
		{
			return IntersectRayPlane(f, line);
		}
	}
}