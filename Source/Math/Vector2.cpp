#include "pch.h"
#include <string>
#include "Vector2.h"

using namespace Math;

Vector2::Vector2()
{
	x = 0;
	y = 0;
}

Vector2::Vector2(const float& _x, const float& _y)
{
	x = _x;
	y = _y;
}

Vector2::Vector2(const float& xy)
{
	x = xy;
	y = xy;
}

Vector2::Vector2(const float& angle, const float& length, bool isAngle)
{
	x = cos(angle) * length;
	y = sin(angle) * length;
}

Vector2::Vector2(const Vector2& copied)
{
	x = copied.x;
	y = copied.y;
}

Vector2::Vector2(const Vector2& p1, const Vector2& p2)
{
	*this = p2 - p1;
}

/* ================================= */
/*				Operators			 */
/* ================================= */

void Vector2::operator=(const Vector2& v)
{
	x = v.x;
	y = v.y;
}

bool Vector2::operator==(const Vector2& v) const
{
	return (x == v.x && y == v.y);
}

bool Vector2::operator!=(const Vector2& v) const
{
	return !((*this) == v);
}

bool Math::Vector2::operator<(const Vector2& v) const
{
	return x < v.x && y < v.y;
}

bool Math::Vector2::operator<=(const Vector2& v) const
{
	return x <= v.x && y <= v.y;
}

bool Math::Vector2::operator>(const Vector2& v) const
{
	return x > v.x && y > v.y;
}

bool Math::Vector2::operator>=(const Vector2& v) const
{
	return x >= v.x && y >= v.y;
}

Vector2 Vector2::operator+(const Vector2& v) const
{
	return Vector2(x + v.x, y + v.y);
}

Vector2 Vector2::operator-(const Vector2& v) const
{
	return Vector2(x - v.x, y - v.y);
}

Vector2 Vector2::operator*(const Vector2& v) const
{
	return Vector2(x * v.x, y * v.y);
}

Vector2 Vector2::operator/(const Vector2& v) const
{
	return Vector2(x / v.x, y / v.y);
}

void Vector2::operator+=(const Vector2& v)
{
	*this = *this + v;
}

void Vector2::operator-=(const Vector2& v)
{
	*this = *this - v;
}

void Vector2::operator*=(const Vector2& v)
{
	*this = *this * v;
}

void Vector2::operator/=(const Vector2& v)
{
	*this = *this / v;
}

Vector2 Vector2::operator-() const
{
	return GetNegated();
}

Vector2 Vector2::operator*(const float& value) const
{
	return { x * value, y * value};
}

Vector2 Vector2::operator/(const float& value) const
{
	return { x / value, y / value};
}

void Vector2::operator*=(const float& value)
{
	x *= value;
	y *= value;
}

void Vector2::operator/=(const float& value)
{
	x /= value;
	y /= value;
}

float Vector2::operator[](const int& idx) const
{
	if(idx >= 2 || idx < 0)
	{
		std::cout << "idx isn't valid" << "\n";
		return -1;
	}

	switch (idx)
	{
	case 0 :
		return x;
	case 1 :
		return y;
	default:
		return 0;		
	}
}

float& Math::Vector2::operator[](size_t idx)
{
	return *((&x) + idx);
}

/* ================================= */
/*				Methods 			 */
/* ================================= */

Vector2 Vector2::GetMiddle() const
{
	return Vector2(x / 2, y / 2);
}

float Vector2::Length() const
{
	return sqrt(Arithmetics::pow(x) + Arithmetics::pow(y));
}

void Vector2::Normalize()
{
	*this = GetNormalized();
}

Vector2 Vector2::GetNormalized() const
{
	float length = Length();
	return Vector2(x / length, y / length);
}

void Vector2::Negate()
{
	*this = GetNegated();
}

Vector2 Vector2::GetNegated() const
{
	return Vector2(-x, -y);
}

Vector2 Vector2::GetNormal() const
{
	return Vector2(-y, x);
}

float Vector2::GetDistanceFromPoint(const Vector2& p) const
{
	return Vector2(*this, p).Length();
}

float Vector2::GetAngle() const
{
	return std::copysign(std::acos(GetNormalized().x), std::asin(GetNormalized().y));
}

float Vector2::GetAngleVector(const Vector2& v) const
{
	return acos(Dot(v) / ( Length() * v.Length()));
}

void Vector2::Rotate(const float& angle)
{
	float radAngle = angle * DEG2RAD;
	Vector2 r = { (float)((x * cos(radAngle)) - (y * sin(radAngle))) , (float)((x * sin(radAngle)) + (y * cos(radAngle))) };
	*this = r;
}

void Vector2::RotateAsPoint(const Vector2& pivot, const float& angle)
{
	Vector2 trVector = *this - pivot;
	trVector.Rotate(angle);
	*this = trVector + pivot;
}

float Math::Vector2::Dot(const Vector2& vec) const
{
	return x * vec.x + y * vec.y;
}

void Math::Vector2::Print()
{
	std::cout << "===================================" << "\n";
	std::cout << x << " , " << y << std::endl;
	std::cout << "===================================" << "\n";
}

std::string Vector2::ToString() const
{
	return std::to_string(x) + " " + std::to_string(y);
}

#ifdef GLM

bool Math::operator==(const Vector2& v, const glm::vec2& g)
{
	return (v.x == g.x && v.y == g.y && v.y);
}

bool Math::operator==(const glm::vec2& g, const Vector2& v)
{
	return (v.x == g.x && v.y == g.y && v.y);
}

#endif 


Vector2 Math::ParseVector2(const std::string& line)
{
	std::string s = line;
	std::string delimiter = ",";

	Vector2 v ;

	size_t pos = 0;
	std::string token;
	size_t i = 0;
	while ((pos = s.find(delimiter)) != std::string::npos) {
		token = s.substr(0, pos);
		v[i] = std::stof(token);
		s.erase(0, pos + delimiter.length());
		i++;
	}
	token = s.substr(0, pos);
	v[i] = std::stof(token);
	s.erase(0, pos + delimiter.length());


	return v;
}