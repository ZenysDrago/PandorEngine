#pragma once
#include "PandorAPI.h"

#include "MathDefines.h"

namespace Math
{
	class PANDOR_API Vector2
	{
	public:
		float x, y;
		
		/* Consructors */
		Vector2();															// vector null
		Vector2(const float& _x, const float& _y);							// vector two coordinates
		Vector2(const float& xy);											// vector from single float
		Vector2(const float& angle, const float& length, bool isAngle );	// vector from angle (in Radian) + length, the bool is useless
		Vector2(const Vector2& copied);										// copy a vector
		Vector2(const Vector2& p1 , const Vector2& p2);						// vector between p1 and p2

		/* Operators */

		void    operator= (const Vector2& v);
		bool    operator==(const Vector2& v) const;
		bool    operator!=(const Vector2& v) const;
		bool    operator<(const Vector2& v) const;
		bool    operator<=(const Vector2& v) const;
		bool    operator>(const Vector2& v) const;
		bool    operator>=(const Vector2& v) const;
		Vector2 operator+ (const Vector2& v) const;
		Vector2 operator- (const Vector2& v) const;
		Vector2 operator* (const Vector2& v) const;
		Vector2 operator/ (const Vector2& v) const;
		void    operator+=(const Vector2& v);
		void    operator-=(const Vector2& v);
		void    operator*=(const Vector2& v);
		void    operator/=(const Vector2& v);
		Vector2 operator- ()				 const;
		Vector2 operator* (const float& value) const;
		Vector2 operator/ (const float& value) const;
		void    operator*=(const float& value);
		void    operator/=(const float& value);
		float   operator[](const int& idx) const;
		float&  operator[](size_t idx);

	#ifdef GLM
		friend bool operator==(const Vector2& v, const glm::vec2& g);
		friend bool operator==(const glm::vec2& g, const Vector2& v);
	#endif 

	    // Returns the middle of the given vector
		Vector2 GetMiddle() const; // 

		// Returns the length of the vector
		float Length() const; //
		
		// Normalizes vector
		void Normalize(); //
		Vector2 GetNormalized() const; //

		// Negates both of the coordinates of vector.
		void Negate(); //
		Vector2 GetNegated() const; //

		// Returns the normal of vector.
		Vector2 GetNormal() const; //

		// Interprets the vector as a point and returns the distance to another point.
		float GetDistanceFromPoint(const Vector2& p) const; //

		// Returns the angle (in radians) of vector.
		float GetAngle() const;

		// Returns the angle (in radians) between two vectors.
		float GetAngleVector(const Vector2& v) const;

		// Rotates vector by the given angle.
		void Rotate(const float& angle);

		// Rotates the point around the given pivot point by the given angle (in rad).
		void RotateAsPoint(const Vector2& pivot, const float& angle);

		float Dot(const Vector2& vec) const;


		// Functions to print values 
		void Print();
		friend std::ostream& operator<<(std::ostream& os, const Vector2& v)
		{
			os << v.x << "," << v.y;
			return os;
		}

		std::string ToString() const;
	};
	Vector2 ParseVector2(const std::string& line);


}
