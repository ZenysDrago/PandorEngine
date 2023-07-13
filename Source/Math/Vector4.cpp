#include "pch.h"

#include "Vector4.h"
#include <string>

using namespace Math;

Vector4::Vector4()
{
	x = 0;
	y = 0;
	z = 0;
	w = 0;
}

Vector4::Vector4(const float& _x, const float& _y, const float& _z, const float& _w)
{
	x = _x;
	y = _y;
	z = _z;
	w = _w;
}

Vector4::Vector4(const float& xyzw)
{
	x = xyzw;
	y = xyzw;
	z = xyzw;
	w = xyzw;
}

Vector4::Vector4(const Vector4& copied)
{
	*this = copied;
}

Vector4::Vector4(const Vector4& p1, const Vector4& p2)
{
	*this = p2 - p1;
}

Vector4::Vector4(const Vector4& p1, const Vector4& p2, const float& _w)
{
	*this = p2 - p1;
	w = _w;
}

Vector4::Vector4(const Vector3& vec3, const float& _w)
{
	x = vec3.x;
	y = vec3.y;
	z = vec3.z;
	w = _w;
}

Vector4::Vector4(const Vector2& vec2, const float& _z, const float& _w /*= 1.f*/)
{
	x = vec2.x;
	y = vec2.y;
	z = _z;
	w = _w;
}

Vector4::Vector4(const float& theta, const float& phi, const float& length, const float& _w, const bool& isAngle) // useless bool
{
	x = length * sin(theta) * cos(phi);
	y = length * cos(theta);
	z = length * sin(theta) * sin(phi);
	w = _w;
}

void Vector4::operator=(const Vector4& v)
{
	x = v.x;
	y = v.y;
	z = v.z;
	w = v.w;
}

bool Vector4::operator==(const Vector4& v) const
{
	return x == v.x && y == v.y && z == v.z && w == v.w;
}

bool Vector4::operator!=(const Vector4& v) const
{
	return !(*this == v);
}

Vector4 Vector4::operator+(const Vector4& v) const
{
	return Vector4(x + v.x, y + v.y, z + v.z, w + v.w);
}

Vector4 Vector4::operator-(const Vector4& v) const
{
	return Vector4(x - v.x, y - v.y, z - v.z, w - v.w);
}

Vector4 Vector4::operator*(const Vector4& v) const
{
	return Vector4(x * v.x, y * v.y, z * v.z, w * v.w);
}

Vector4 Vector4::operator/(const Vector4& v) const
{
	return Vector4(x / v.x, y / v.y, z / v.z, w / v.w);
}

void Vector4::operator+=(const Vector4& v)
{
	*this = *this + v;
}

void Vector4::operator-=(const Vector4& v)
{
	*this = *this - v;
}

void Vector4::operator*=(const Vector4& v)
{
	*this = *this * v;
}

void Vector4::operator/=(const Vector4& v)
{
	*this = *this / v;
}

Vector4 Vector4::operator*(const float& value) const
{
	return { x * value, y * value , z * value , w * value};
}

Vector4 Vector4::operator/(const float& value) const
{
	return { x / value, y / value , z / value , w / value};
}

void Vector4::operator*=(const float& value)
{
	x *= value;
	y *= value;
	z *= value;
	w *= value;
}

void Vector4::operator/=(const float& value)
{
	x /= value;
	y /= value;
	z /= value;
	w /= value;
}

float Vector4::operator[](const int& idx) const
{
	if(idx >= 4 || idx < 0)
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
	case 2 :
		return z;
	case 3 :
		return w;
	default:
		return 0;		
	}
}

float& Math::Vector4::operator[](size_t idx)
{
	return *((&x) + idx);
}

Vector4 Vector4::operator-() const
{
	return GetNegated();
}

void Vector4::Homogenize()
{
	*this = GetHomogenized();
}

Vector4 Vector4::GetHomogenized() const
{
	return Vector4(x / w, y / w, z / w, w / w);
}

float Vector4::Length() const
{
	return sqrt(Magnitude());
}

float Vector4::Magnitude() const
{
	return Arithmetics::pow(x) + Arithmetics::pow(y) + Arithmetics::pow(z) + Arithmetics::pow(w);
}

void Vector4::Normalize()
{
	*this = GetNormalized();
}

Vector4 Vector4::GetNormalized() const
{
	float length = Length();
	return Vector4(x / length, y / length, z / length, w/length);
}

void Vector4::Negate()
{
	*this = GetNegated();
}

Vector4 Vector4::GetNegated() const
{
	return Vector4(-x, -y, -z, -w);
}

float Vector4::GetDistanceFromPoint(const Vector4& p) const
{
	return sqrt(Arithmetics::pow(p.x - x) + Arithmetics::pow(p.y - y) + Arithmetics::pow(p.z - z) + Arithmetics::pow(p.w - w));
}

float Vector4::GetAngleTheta() const 
{
	return acos(z / Length()); 
}

float Vector4::GetAnglePhi()   const
{
	if (x > 0) return atan(y / x);
	if (x < 0) return atan(y / x) + PI;
	return PI / 2.f;
}

float Vector4::GetAngleTheta(const Vector4& v)
{
	float this_angle = GetAngleTheta();
	float v_angle = v.GetAngleTheta();
	return (this_angle >= v_angle ? (this_angle - v_angle) : (v_angle - this_angle));
}

float Vector4::GetAnglePhi(const Vector4& v)
{
	float this_angle = GetAnglePhi();
	float v_angle = v.GetAnglePhi();
	return (this_angle >= v_angle ? (this_angle - v_angle) : (v_angle - this_angle));
}

void Vector4::Rotate(const float& theta, const float& phi)
{
	*(this) = Vector4(GetAngleTheta() + theta, GetAnglePhi() + phi, Length(), w, true);
}

float Vector4::Dot(const Vector4& v)
{
	return (x * v.x) + (y * v.y) + (z * v.z) + (w * v.w);
}

Vector3 Vector4::ToVector3(bool shouldHomogenize) const
{
	if (shouldHomogenize)
		return Vector3(x / w, y / w, z / w);
	else
		return Vector3(x, y, z);
}

Math::Vector4 Vector4::Lerp(const Vector4& other, float t) const
{
	return Vector4(
		x + (other.x - x) * t,
		y + (other.y - y) * t,
		z + (other.z - z) * t,
		w + (other.w - w) * t
	);
}

void Math::Vector4::Print()
{
	std::cout << "===================================" << "\n";
	std::cout << x << " , " << y << " , " << z << " , " << w << std::endl;
	std::cout << "===================================" << "\n";
}

std::string Vector4::ToString() const
{
	return std::to_string(x) + " " + std::to_string(y) + " " + std::to_string(z) + " " + std::to_string(w);
}

#ifdef GLM

bool Math::operator==(const Vector4& v, const glm::vec4& g)
{
	return (v.x == g.x && v.y == g.y && v.y && v.z == g.z && v.w == g.w);
}

bool Math::operator==(const glm::vec4& g, const Vector4& v)
{
	return (v.x == g.x && v.y == g.y && v.y && v.z == g.z && v.w == g.w);
}

#endif // GLM

Vector4 Math::ParseVector4(const std::string& line)
{
	std::string s = line;
	std::string delimiter = ",";

	Vector4 v;

	size_t pos = 0;
	std::string token;
	size_t i = 0;
	size_t max = 3;
	while ((pos = s.find(delimiter)) != std::string::npos && i < max) {
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