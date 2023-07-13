#include "pch.h"

#include "Quaternion.h"
#include "MathDefines.h"

using namespace Math;
using namespace Arithmetics;

Quaternion Quaternion::operator+(const Quaternion& a) const
{
	return Quaternion(x + a.x, y + a.y, z + a.z, w + a.w);
}

Quaternion Quaternion::operator-(const Quaternion& a) const
{
	return Quaternion(x - a.x, y - a.y, z - a.z, w - a.w);
}

Quaternion Quaternion::operator*(const Quaternion& a) const
{
	return Quaternion(
		w * a.x + x * a.w + y * a.z - z * a.y,
		w * a.y + y * a.w + z * a.x - x * a.z,
		w * a.z + z * a.w + x * a.y - y * a.x,
		w * a.w - x * a.x - y * a.y - z * a.z);
}

Quaternion Quaternion::operator*(const float& a) const
{
	return Quaternion(this->x * a, this->y * a, this->z * a, this->w * a);
}

Vector3 Quaternion::operator*(const Vector3& a) const
{
	Vector3 vector;
	float ax = x * 2.f;
	float ay = y * 2.f;
	float az = z * 2.f;
	float xx = x * ax;
	float yy = y * ay;
	float zz = z * az;
	float xy = x * ay;
	float xz = x * az;
	float yz = y * az;
	float wx = w * ax;
	float wy = w * ay;
	float wz = w * az;
	return { (1.f - (yy + zz)) * a.x + (xy - wz) * a.y + (xz + wy) * a.z ,
			(xy + wz) * a.x + (1.f - (xx + zz)) * a.y + (yz - wx) * a.z ,
			(xz - wy) * a.x + (yz + wx) * a.y + (1.f - (xx + yy)) * a.z };
}

bool Quaternion::operator!=(const Quaternion& q) const
{
	return (x != q.x || y != q.y || z != q.z || w != q.w);
}

float& Quaternion::operator[](const size_t index)
{
	return *((&x) + index);
}

Quaternion Quaternion::AngleAxis(float angle, Vector3 axis)
{
	Quaternion q;
	float rad = angle * DEG2RAD;
	axis.Normalize();
	q.w = cosf(rad / 2);
	q.x = sinf(rad / 2) * axis.x;
	q.y = sinf(rad / 2) * axis.y;
	q.z = sinf(rad / 2) * axis.z;
	return q;
}
Quaternion Quaternion::LookRotation(Vector3 forward, Vector3 up)
{
	forward.Normalize();
	Vector3 vector = forward.GetNormalized();
	Vector3 vector2 = up.Cross(vector).GetNormalized();
	Vector3 vector3 = vector.Cross(vector2);
	float m00 = vector2.x;
	float m01 = vector2.y;
	float m02 = vector2.z;
	float m10 = vector3.x;
	float m11 = vector3.y;
	float m12 = vector3.z;
	float m20 = vector.x;
	float m21 = vector.y;
	float m22 = vector.z;

	float num8 = (m00 + m11) + m22;
	Quaternion quaternion;
	if (num8 > 0.f)
	{
		float num = (float)sqrtf(num8 + 1.f);
		quaternion.w = num * 0.5f;
		num = 0.5f / num;
		quaternion.x = (m12 - m21) * num;
		quaternion.y = (m20 - m02) * num;
		quaternion.z = (m01 - m10) * num;
		return quaternion;
	}
	if ((m00 >= m11) && (m00 >= m22))
	{
		float num7 = (float)sqrtf(((1.f + m00) - m11) - m22);
		float num4 = 0.5f / num7;
		quaternion.x = 0.5f * num7;
		quaternion.y = (m01 + m10) * num4;
		quaternion.z = (m02 + m20) * num4;
		quaternion.w = (m12 - m21) * num4;
		return quaternion;
	}
	if (m11 > m22)
	{
		float num6 = (float)sqrtf(((1.f + m11) - m00) - m22);
		float num3 = 0.5f / num6;
		quaternion.x = (m10 + m01) * num3;
		quaternion.y = 0.5f * num6;
		quaternion.z = (m21 + m12) * num3;
		quaternion.w = (m20 - m02) * num3;
		return quaternion;
	}
	float num5 = (float)sqrtf(((1.f + m22) - m00) - m11);
	float num2 = 0.5f / num5;
	return { (m20 + m02) * num2 , (m21 + m12) * num2, 0.5f * num5,(m01 - m10) * num2 };
}

Quaternion Quaternion::SLerp(const Quaternion& a, const Quaternion& b, float time)
{
	if (time < 0.0f)
		return a;
	else if (time >= 1.0f)
		return b;
	float d = a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
	float s0, s1;
	float sd = (float)((d > 0.0f) - (d < 0.0f));

	d = fabs(d);

	if (d < 0.9995f)
	{
		float s = sqrtf(1.0f - d * d);
		float a = atan2f(s, d);
		float c = cosf(time * a);


		s1 = sqrtf(1.0f - c * c) / s;
		s0 = c - d * s1;
	}
	else
	{
		s0 = 1.0f - time;
		s1 = time;
	}

	return a * s0 + b * sd * s1;
}

void Quaternion::Inverse()
{
	*this = GetInverse();
}
Quaternion Quaternion::GetInverse() const
{
	float d = w * w + x * x + y * y + z * z;
	if (d == 0)
		return Quaternion::Identity();

	d = 1 / d;
	return Quaternion(-x * d, -y * d, -z * d, w * d);
}
void Quaternion::Normalize()
{
	*this = GetNormal();
}
Quaternion Quaternion::GetNormal() const
{
	float mag = sqrtf(Dot(*this));

	if (mag < FLT_MIN)
		return Quaternion::Identity();
	else
		return Quaternion(x / mag, y / mag, z / mag, w / mag);
}

void Quaternion::Conjugate()
{
	*this = GetConjugate();
}

Quaternion Quaternion::GetConjugate() const
{
	return Quaternion(-x, -y, -z, w);
}

float Quaternion::Dot(const Quaternion& a) const
{
	return x * a.x + y * a.y + z * a.z + w * a.w;
}

float NormalizeAngle(float angle)
{
	float modAngle = fmodf(angle, 360.0f);

	if (modAngle < 0.0f)
		return modAngle + 360.0f;
	else
		return modAngle;
}

Vector3 NormalizeAngles(Vector3 angles)
{
	angles.x = NormalizeAngle(angles.x);
	angles.y = NormalizeAngle(angles.y);
	angles.z = NormalizeAngle(angles.z);
	return angles;
}

Vector3 Quaternion::ToEuler() const
{
#if 0
	// IT WORKS
	Vector3 euler;
	Matrix4 rotMat = ToRotationMatrix();

	// Get Rotation from rotation matrix.
	float thetaX, thetaY, thetaZ;
	if (rotMat[2][1] < 0.9999f)
	{
		if (rotMat[2][1] > -0.9999f)
		{
			thetaX = asinf(-rotMat[2][1]);
			thetaY = atan2f(rotMat[2][0], rotMat[2][2]);
			thetaZ = atan2f(rotMat[0][1], rotMat[1][1]);
		}
		else
		{
			thetaX = PI;
			thetaY = -atan2f(-rotMat[1][0], rotMat[0][0]);
			thetaZ = 0;
		}
	}
	else
	{
		thetaX = -PI;
		thetaY = atan2f(-rotMat[1][0], rotMat[0][0]);
		thetaZ = 0;
	}
	return Vector3(thetaX, thetaY, thetaZ) * RAD2DEG;
#else
	// Extract quaternion components
	// Calculate Euler angles
	Vector3 euler;
	float sinr_cosp = 2.0f * (w * x + y * z);
	float cosr_cosp = 1.0f - 2.0f * (x * x + y * y);
	euler.x = std::atan2f(sinr_cosp, cosr_cosp);

	float sinp = 2.0f * (w * y - z * x);
	if (std::abs(sinp) >= 1.0)
		euler.y = std::copysign(PI / 2.0f, sinp);
	else
		euler.y = std::asinf(sinp);

	float siny_cosp = 2.0f * (w * z + x * y);
	float cosy_cosp = 1.0f - 2.0f * (y * y + z * z);
	euler.z = std::atan2f(siny_cosp, cosy_cosp);

	return euler * RAD2DEG;
#endif
}

Matrix4 Quaternion::ToRotationMatrix() const
{
	Matrix4 m = Matrix4();
	auto q0 = w;
	auto q1 = x;
	auto q2 = y;
	auto q3 = z;

	/*
	m[0][0] = 1 - 2 * (y * y) - 2 * (z * z);
	m[1][0] = 2 * x * y + 2 * w * z;
	m[2][0] = 2 * x * z + 2 * w * y;

	m[0][1] = 2 * x * y + 2 * w * z;
	m[1][1] = 1 - 2 * (x * x) - 2 * (z * z);
	m[2][1] = 2 * y * z - 2 * w * x;

	m[0][2] = 2 * x * z - 2 * w * y;
	m[1][2] = 2 * y * z + 2 * w * x;
	m[2][2] = 1 - 2 * (x * x) - 2 * (y* y);
	*/
	m.SetValueAt(0, 0, 2 * (q0 * q0 + q1 * q1) - 1);
	m.SetValueAt(1, 0, 2 * (q1 * q2 - q0 * q3));
	m.SetValueAt(2, 0, 2 * (q1 * q3 + q0 * q2));

	m.SetValueAt(0, 1, 2 * (q1 * q2 + q0 * q3));
	m.SetValueAt(1, 1, 2 * (q0 * q0 + q2 * q2) - 1);
	m.SetValueAt(2, 1, 2 * (q2 * q3 - q0 * q1));

	m.SetValueAt(0, 2, 2 * (q1 * q3 - q0 * q2));
	m.SetValueAt(1, 2, 2 * (q2 * q3 + q0 * q1));
	m.SetValueAt(2, 2, 2 * (q0 * q0 + q3 * q3) - 1);

	return m;
}

void Quaternion::Print() const
{
	printf("Quaternion { %f, %f, %f, %f}\n", x, y, z, w);
}
std::string Quaternion::ToString() const
{
	return std::to_string(x) + " " + std::to_string(y) + " " + std::to_string(z) + " " + std::to_string(w);
}

Math::Quaternion Quaternion::FromToRotation(Vector3 fromDirection, Vector3 toDirection)
{
	fromDirection.Normalize();
	toDirection.Normalize();

	float dot =	fromDirection.Dot(toDirection);

	if (dot >= 1.f)
	{
		// From and to directions are identical, no rotation needed
		return Identity();
	}
	else if (dot <= -1.f)
	{
		// From and to directions are exactly opposite, need to rotate 180 degrees around any orthogonal axis
		Vector3 orthoAxis = Vector3::Forward().Cross(fromDirection);
		if (orthoAxis.Length() < std::numeric_limits<float>::epsilon())
		{
			// From direction is collinear with forward, so we use a different orthogonal axis
			orthoAxis = Vector3::Up().Cross(fromDirection);
		}

		orthoAxis.Normalize();
		return Quaternion::AngleAxis(180.f, orthoAxis);
	}
	else
	{
		// Normal rotation between from and to directions
		float angle = std::acosf(dot) * RAD2DEG;
		Vector3 axis = fromDirection.Cross(toDirection);
		axis.Normalize();
		return Quaternion::AngleAxis(angle, axis);
	}
}

void Quaternion::GetAngleAxis(float& angle, Vector3& axis) const
{
	// Compute the angle
	angle = 2 * std::acosf(w);

	// Compute the axis
	float sinHalfAngle = std::sqrtf(1 - w * w);
	if (sinHalfAngle < 0.0001f)
	{
		// If sin(angle/2) is close to zero, set the axis to (1, 0, 0)
		axis.x = 1.0f;
		axis.y = 0.0f;
		axis.z = 0.0f;
	}
	else
	{
		// Otherwise, compute the axis
		axis.x = x / sinHalfAngle;
		axis.y = y / sinHalfAngle;
		axis.z = z / sinHalfAngle;
	}
}


float Quaternion::GetAngleX() const
{
	// Compute the angle in the x-axis
	return std::atan2(2 * (w * x + y * z), 1 - 2 * (x * x + y * y)) * RAD2DEG;
}

float Quaternion::GetAngleY() const
{
	// Compute the angle in the y-axis
	//return std::atan2f(2 * (w * y + x * z), 1 - 2 * (y * y + z * z)) * RAD2DEG;

	float sinp = 2 * (w * y - z * x);
	float pitch = std::asinf(sinp);
	return pitch * RAD2DEG;
}

float Quaternion::GetAngleZ() const
{
	// Compute the angle in the z-axis
	return std::atan2(2 * (w * z + x * y), 1 - 2 * (y * y + z * z)) * RAD2DEG;
}

Quaternion Math::ParseQuaternion(const std::string& line)
{
	std::string s = line;
	std::string delimiter = ",";

	Vector4 v;

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

	Quaternion q = { v.x, v.y, v.z , v.w };
	return q;
}

