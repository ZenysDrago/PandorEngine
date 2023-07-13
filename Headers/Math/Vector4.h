#pragma once
#include "PandorAPI.h"

#include "Vector3.h"

namespace Math
{
	class PANDOR_API Vector4
	{
	public:
		float x, y, z, w;

		/* Consructors */
		Vector4();																							// vector null
		Vector4(const float& _x, const float& _y, const float& _z, const float& _w);						// vector four coordinates
		Vector4(const float& xyzw);																			// vector from single float
		Vector4(const Vector4& copied);																		// copy a vector
		Vector4(const Vector4& p1, const Vector4& p2);														// vector between p1 and p2
		Vector4(const Vector4& p1, const Vector4& p2, const float& _w);										// vector between p1 and p2 with specified w
		Vector4(const Vector3& vec3, const float& _w = 0.f);	 											// transform vector 3 to vector 4
		Vector4(const Vector2& vec3, const float& _z, const float& _w = 1.f);	 											// transform vector 3 to vector 4
		Vector4(const float& _x, const float& _y, const float& _z, const float& _w, const bool& isAngle);	// Vector from angles bool is useless


		/* Operators */

		void    operator= (const Vector4& v);
		bool    operator==(const Vector4& v) const;
		bool    operator!=(const Vector4& v) const;
		Vector4 operator+ (const Vector4& v) const;
		Vector4 operator- (const Vector4& v) const;
		Vector4 operator* (const Vector4& v) const;
		Vector4 operator/ (const Vector4& v) const;
		void    operator+=(const Vector4& v);
		void    operator-=(const Vector4& v);
		void    operator*=(const Vector4& v);
		void    operator/=(const Vector4& v);
		Vector4 operator- ()				 const;
		Vector4 operator* (const float& value) const;
		Vector4 operator/ (const float& value) const;
		void    operator*=(const float& value);
		void    operator/=(const float& value);
		float   operator[](const int& idx) const;
		float& operator[](size_t idx);

		/* If glm included */
#ifdef GLM
		friend bool operator==(const Vector4& v, const glm::vec4& g);
		friend bool operator==(const glm::vec4& g, const Vector4& v);
#endif 

		// Homogenized vector by w
		void Homogenize();
		Vector4 GetHomogenized() const;

		// Returns the length of the vector
		float Length() const;

		//Return the vector Magnitude
		float Magnitude() const;
		
		// Normalized vector
		void Normalize();
		Vector4 GetNormalized() const;

		// Negates both of the coordinates of vector.
		void Negate();
		Vector4 GetNegated() const;

		// Interprets the vector as a point and returns the distance to another point.
		float GetDistanceFromPoint(const Vector4& p) const;

		// Rotates vector by the given angle.
		void Rotate(const float& theta, const float& phi);

		// Return the angles of vector
		float GetAngleTheta() const;
		float GetAnglePhi() const;

		// Return the angle between given vector and vector
		float GetAngleTheta (const Vector4& v);
		float GetAnglePhi	(const Vector4& v);

		// dot operation
		float Dot(const Vector4& v);

		// Creates a Vector3 from this vector.
		Vector3 ToVector3(bool shouldHomogenize = false) const;

		Math::Vector4 Lerp(const Vector4& other, float t) const;


		// Functions to print values 
		void Print();
		friend std::ostream& operator<<(std::ostream& os, const Vector4& v)
		{
			os << v.x << "," << v.y << "," << v.z << "," << v.w;
			return os;
		}

		std::string ToString() const;
	};
	Vector4 ParseVector4(const std::string& line);
}
