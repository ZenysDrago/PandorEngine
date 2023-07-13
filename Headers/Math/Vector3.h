#pragma once
#include "PandorAPI.h"

#include "Vector2.h"

namespace Math
{
	class Quaternion;
	class Vector4;

	static float defaultValue = -1;

	class PANDOR_API Vector3
	{
	public:
		float x, y, z;

		/* Consructors */
		Vector3();														// vector null
		Vector3(const float& _x, const float& _y, const float& _z);		// vector three coordinates
		Vector3(const float& xyz);										// vector from single float
		Vector3(const Vector3& copied);									// copy a vector
		Vector3(const Vector4& vec4);									// vector4 to vector3
		Vector3(const Vector3& p1, const Vector3& p2);					// vector between p1 and p2
		Vector3(const Vector2& vec2, const float& _z = 0);	 			// transform vector 2 to vector 3
		Vector3(const Vector3& angles, const float& length);			// vector from angles

		/* Operators */

		void    operator= (const Vector3& v);
		bool    operator==(const Vector3& v) const;
		bool    operator!=(const Vector3& v) const;
		Vector3 operator+ (const Vector3& v) const;
		Vector3 operator- (const Vector3& v) const;
		Vector3 operator* (const Vector3& v) const;
		Vector3 operator/ (const Vector3& v) const;
		void    operator+=(const Vector3& v);
		void    operator-=(const Vector3& v);
		void    operator*=(const Vector3& v);
		void    operator/=(const Vector3& v);
		Vector3 operator* (const float& value) const;
		Vector3 operator/ (const float& value) const;
		void    operator*=(const float& value);
		void    operator/=(const float& value);
		Vector3 operator- ()				 const;
		float   operator[](const int& idx) const;
		float& operator[](size_t idx);

		/* If glm included */
		#ifdef GLM
			friend bool operator==(const Vector3& v, const glm::vec3& g);
			friend bool operator==(const glm::vec3& g, const Vector3& v);
		#endif 

			/* If PhysX included */
		#ifdef PHYSX
			physx::PxVec3 operator=(const physx::PxVec3& p);
		#endif 

		// Returns the length of the vector
		float Length() const;//

		float GetMagnitude() const; //
		
		// Normalized vector
		void Normalize();//
		Vector3 GetNormalized() const;//

		// Negates both of the coordinates of vector.
		void Negate();//
		Vector3 GetNegated() const;//

		// Interprets the vector as a point and returns the distance to another point.
		float GetDistanceFromPoint(const Vector3& p) const;//

		// Get angles
		float GetXAngle() const;
		float GetYAngle() const;

		// Rotates vector by the given angle.
		void Rotate(const Vector3& axis, const float angle);
		Vector3 GetRotated(const Vector3& axis, const float angle) const;

		float Dot(const Vector3& v) const ; //
		Vector3 Cross(const Vector3& v) const ; //

		// Functions to print values 
		void Print();
		friend std::ostream& operator<<(std::ostream& os, const Vector3& v)
		{
			os << v.x << "," << v.y << "," << v.z;
			return os;
		}


		/* Static functions */
		static Vector3 Forward() { return Vector3(0, 0, 1); }

		static Vector3 Right() { return Vector3(1, 0, 0); }

		static Vector3 Up() { return Vector3(0, 1, 0); }

		static Vector3 Zero() { return Vector3(0, 0, 0); }

		Quaternion ToQuaternion();

		Vector2 ToVector2() const { return { x, y }; }

		std::string ToString() const;

		static Vector3 Lerp(const Vector3& a, const Vector3& b, float t);

		// Returns the coordinates of a point on a sphere of radius r, using the given angles.
		static Vector3 GetSphericalCoords(const float& r, const float& pitch, const float& yaw);

		static float   Angle(Vector3 from, Vector3 to);
		static Vector3 Project(Vector3 vector, Vector3 onNormal);
		float AngleBetweenVector(const Vector3& vec1, const Vector3& axis);
		static float   Distance(Vector3 a, Vector3 b);
	};
	Vector3 ParseVector3(const std::string& line);


}