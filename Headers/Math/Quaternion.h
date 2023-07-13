#pragma once
#include "PandorAPI.h"

#include"MathDefines.h"
#include"Vector4.h"
#include"Matrix.h"

namespace Math
{
	class PANDOR_API Quaternion
	{
	public:
		float x;
		float y;
		float z;
		float w;

		Quaternion() : x(0), y(0), z(0), w(1) {}

		Quaternion(float a) : x(a), y(a), z(a), w(a) {}

		Quaternion(float a, float b, float c, float d = 1) : x(a), y(b), z(c), w(d) {}

		Quaternion(const Vector3& a) : x(a.x), y(a.y), z(a.z), w(1.f) {}

		Quaternion(float angle, const Vector3& a) { *this = AngleAxis(angle, a); }

		Quaternion(const Vector4& a) : x(a.x), y(a.y), z(a.z), w(a.w) {}

		Quaternion operator+(const Quaternion& a) const;

		Quaternion operator-(const Quaternion& a) const;

		Quaternion operator*(const Quaternion& a) const;

		Quaternion operator*(const float& a) const;

		Vector3 operator*(const Vector3& a) const;

		bool operator!=(const Quaternion& q) const;

		float& operator[](const size_t index);

		friend std::ostream& operator<<(std::ostream& os, const Quaternion& q)
		{
			os << q.x << "," << q.y << "," << q.z << "," << q.w;
			return os;
		}

		static Quaternion Identity() { return Quaternion(0, 0, 0, 1); }

		static Quaternion AngleAxis(float angle, Vector3 axis);

		void GetAngleAxis(float& angle, Vector3& axis) const; 
		
		float GetAngleX() const;

		float GetAngleY() const;

		float GetAngleZ() const;

		static Quaternion LookRotation(Vector3 forward, Vector3 up);

		static Quaternion SLerp(const Quaternion& a, const Quaternion& b, float time);

		void Inverse();

		Quaternion GetInverse() const;

		void Normalize();

		Quaternion GetNormal() const;

		void Conjugate();

		Quaternion GetConjugate() const;

		float Dot(const Quaternion& a) const;

		Vector3 ToEuler() const;

		static Quaternion FromToRotation(Vector3 fromDirection, Vector3 toDirection);

		Matrix4 ToRotationMatrix() const;

		void Print() const;

		std::string ToString() const;
	};
	Quaternion ParseQuaternion(const std::string& line);

}
