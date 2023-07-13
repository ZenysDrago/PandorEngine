#include "pch.h"

#include "Vector3.h"
#include "Quaternion.h"
#include "Matrix.h"
#include <string>
#include <Math/Maths.h>

using namespace Math;
float defaultValue = -1;

Vector3::Vector3()
{
	x = 0;
	y = 0;
	z = 0;
}

Vector3::Vector3(const float& _x, const float& _y, const float& _z)
{
	x = _x;
	y = _y;
	z = _z;
}

Vector3::Vector3(const float& xyz)
{
	x = xyz;
	y = xyz;
	z = xyz;
}

Vector3::Vector3(const Vector3& copied)
{
	*this = copied;
}

Vector3::Vector3(const Vector3& p1, const Vector3& p2)
{
	*this = p2 - p1;
}

Vector3::Vector3(const Vector2& vec2, const float& _z)
{
	*this = Vector3(vec2.x, vec2.y, _z);
}

Vector3::Vector3(const Vector3& angles, const float& length)
{
	x = -1;
	y = -1;
	z = -1;
	//TODO
	//*this = (Vector4(0, 0, 1, 1) * getRotationMatrix({ -angles.x, -angles.y, -angles.z })).toVector3(true) * length;
}

void Vector3::operator=(const Vector3& v)
{
	x = v.x;
	y = v.y;
	z = v.z;
}

bool Vector3::operator==(const Vector3& v) const
{
	return x == v.x && y == v.y && z == v.z;
}

bool Vector3::operator!=(const Vector3& v) const
{
	return !((*this) == v);
}

Vector3 Vector3::operator+(const Vector3& v) const
{
	return Vector3(x + v.x, y + v.y, z + v.z);
}

Vector3 Vector3::operator-(const Vector3& v) const
{
	return Vector3(x - v.x, y - v.y, z - v.z);
}

Vector3 Vector3::operator*(const Vector3& v) const
{
	return Vector3(x * v.x, y * v.y, z * v.z);
}

Vector3 Vector3::operator/(const Vector3& v) const
{
	return Vector3(x / v.x, y / v.y, z / v.z);
}

void Vector3::operator+=(const Vector3& v)
{
	*this = *this + v;
}

void Vector3::operator-=(const Vector3& v)
{
	*this = *this - v;
}

void Vector3::operator*=(const Vector3& v)
{
	*this = *this * v;
}

void Vector3::operator/=(const Vector3& v)
{
	*this = *this / v;
}

Vector3 Vector3::operator*(const float& value) const
{
	return { x * value, y * value , z * value };
}

Vector3 Vector3::operator/(const float& value) const
{
	return { x / value, y / value , z / value };
}

void Vector3::operator*=(const float& value)
{
	x *= value;
	y *= value;
	z *= value;
}

void Vector3::operator/=(const float& value)
{
	x /= value;
	y /= value;
	z /= value;
}

Vector3 Vector3::operator-() const
{
	return GetNegated();
}

float Vector3::operator[](const int& idx) const
{
	if (idx >= 3 || idx < 0)
	{
		std::cout << "idx isn't valid" << "\n";
		return -1;
	}

	switch (idx)
	{
	case 0:
		return x;
	case 1:
		return y;
	case 2:
		return z;
	default:
		return 0;
	}
}

Vector3::Vector3(const Vector4& vec4)
{
	x = vec4.x;
	y = vec4.y;
	z = vec4.z;
}

float& Math::Vector3::operator[](size_t idx)
{
	return *((&x) + idx);
}

std::string Vector3::ToString() const
{
	return std::to_string(x) + " " + std::to_string(y) + " " + std::to_string(z);
}

Math::Vector3 Vector3::Lerp(const Vector3& a, const Vector3& b, float t)
{
	if (t < 0)
		return a;
	else if (t >= 1)
		return b;
	return a * (1 - t) + b * t;
}

#ifdef PHYSX
physx::PxVec3 Math::Vector3::operator=(const physx::PxVec3& p)
{
	return physx::PxVec3(p.x, p.y, p.z);
}
#endif
float Vector3::Length() const
{
	return sqrt(GetMagnitude());
}

float Vector3::GetMagnitude() const
{
	return Arithmetics::pow(x) + Arithmetics::pow(y) + Arithmetics::pow(z);
}

void Vector3::Normalize()
{
	*this = GetNormalized();
}

Vector3 Vector3::GetNormalized() const
{
	float length = Length();
	if (length == 0)
		return 0;
	return Vector3(x / length, y / length, z / length);
}

void Vector3::Negate()
{
	x = -x;
	y = -y;
	z = -z;
}

float Vector3::Dot(const Vector3& v) const
{
	return x * v.x + y * v.y + z * v.z;
}

Vector3 Vector3::Cross(const Vector3& v) const
{
	Vector3 res;
	res.x = y * v.z - z * v.y;
	res.y = z * v.x - x * v.z;
	res.z = x * v.y - y * v.x;

	return res;
}

Vector3 Vector3::GetNegated() const
{
	return Vector3(-x, -y, -z);
}

float Vector3::GetDistanceFromPoint(const Vector3& p) const
{
	return Vector3(*this, p).Length();
}

// Returns the angle (in radians) of the given vector.
float Vector3::GetXAngle() const
{
	return asin(-y);
}

float Vector3::GetYAngle() const
{
	return atan2(x, z);
}

void Vector3::Rotate(const Vector3& axis, const float angle)
{
	*this = GetRotated(axis, angle);
}

Vector3 Vector3::GetRotated(const Vector3& axis, const float angle) const
{
	Matrix4 rot = GetRotationMatrix(axis * angle);


	return rot * (*this);
}

void Vector3::Print()
{
	std::cout << "===================================" << "\n";
	std::cout << x << " , " << y << " , " << z << std::endl;
	std::cout << "===================================" << "\n";
}

Vector3 Vector3::GetSphericalCoords(const float& r, const float& pitch, const float& yaw)
{
	return { r * sinf(pitch) * cosf(yaw),
			 r * cosf(pitch),
			 r * sinf(pitch) * sinf(yaw) };
}

Math::Quaternion Vector3::ToQuaternion()
{

	//TODO : fix this
	auto a = (*this) * DEG2RAD;
	// Calculate the cosine of each half angle
	float cy = std::cosf(a.z * 0.5f);
	float cp = std::cosf(a.y * 0.5f);
	float cr = std::cosf(a.x * 0.5f);

	// Calculate the sine of each half angle
	float sy = std::sinf(a.z * 0.5f);
	float sp = std::sinf(a.y * 0.5f);
	float sr = std::sinf(a.x * 0.5f);

	// Calculate the quaternion elements
	Quaternion q;
	q.w = cy * cp * cr + sy * sp * sr;
	q.x = cy * cp * sr - sy * sp * cr;
	q.y = sy * cp * sr + cy * sp * cr;
	q.z = sy * cp * cr - cy * sp * sr;

	return q;
}

#ifdef GLM
bool Math::operator==(const Vector3& v, const glm::vec3& g)
{
	return (v.x == g.x && v.y == g.y && v.y && v.z == g.z);
}

bool Math::operator==(const glm::vec3& g, const Vector3& v)
{
	return (v.x == g.x && v.y == g.y && v.y && v.z == g.z);
}
#endif

Vector3 Math::ParseVector3(const std::string& line)
{
	std::string s = line;
	std::string delimiter = ",";

	Vector3 v;

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

float Vector3::Angle(Vector3 from, Vector3 to) {
	float denominator = (float)sqrt(from.Length() * to.Length());
	if (denominator < 1e-15F)
		return 0;

	float dot = Arithmetics::Clamp(from.Dot(to) / denominator, -1.f, 1.f);
	return (acosf(dot)) * RAD2DEG;
}

Vector3 Vector3::Project(Vector3 vector, Vector3 onNormal)
{
	float sqrMag = onNormal.Dot(onNormal);
	if (sqrMag < FLT_MIN)
		return 0;
	else
	{
		auto dot = vector.Dot(onNormal);
		return Vector3(onNormal.x * dot / sqrMag,
			onNormal.y * dot / sqrMag,
			onNormal.z * dot / sqrMag);
	}
}

float Vector3::AngleBetweenVector(const Vector3& vec1, const Vector3& axis) {
	Vector3 vec1Projected = vec1 - axis * vec1.Dot(axis);
	Vector3 vec2Projected = *this - axis * this->Dot(axis);

	vec1Projected.Normalize();
	vec2Projected.Normalize();

	float dotProduct = vec1Projected.Dot(vec2Projected);
	float angle = std::acos(std::fmaxf(-1.0f, std::fminf(1.0f, dotProduct)));

	Vector3 crossProduct = vec1Projected.Cross(vec2Projected);
	if (crossProduct.Dot(axis) < 0) {
		angle = -angle;
	}

	return angle * RAD2DEG;
}

float Vector3::Distance(Vector3 a, Vector3 b) {
	float diff_x = a.x - b.x;
	float diff_y = a.y - b.y;
	float diff_z = a.z - b.z;
	return (float)sqrtf(diff_x * diff_x + diff_y * diff_y + diff_z * diff_z);
}