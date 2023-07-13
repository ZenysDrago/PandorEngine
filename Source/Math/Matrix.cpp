#include "pch.h"
#include <string>
#include "Matrix.h"
#include "Quaternion.h"
using namespace Math;

#ifdef GLM

bool Math::operator==(const Matrix<4, 4>& m, const glm::mat4& glmMat)
{
	for (int row = 0; row < 4; row++)
		for (int column = 0; column < 4; column++)
			if (glmMat[row][column] != m.matrix[row][column])
				return false;

	return true;
}

bool Math::operator==(const glm::mat4& glmMat, const Matrix<4, 4>& m)
{
	for (int row = 0; row < 4; row++)
		for (int column = 0; column < 4; column++)
			if (glmMat[row][column] != m.matrix[row][column])
				return false;

	return true;
}
#endif

/* Static function */
Matrix4 Math::GetTranslationMatrix(const Vector3& translation)
{
	return Matrix4{ Vector4{1, 0, 0, 0},
					Vector4{0, 1, 0, 0},
					Vector4{0, 0, 1, 0},
					Vector4{translation.x, translation.y, translation.z, 1}
	};
}

Matrix4 Math::GetScaleMatrix(const Vector3& scale)
{
	return Matrix4{ Vector4{scale.x, 0,		0, 0},
					Vector4{0, scale.y,		0, 0},
					Vector4{0,    0,	scale.z, 0},
					Vector4{0,    0,		0, 1}
	};
}

Matrix4 Math::GetXRotationMatrix(float angle)
{
	return Matrix4{ Vector4{1, 0, 0, 0},
					Vector4{0, cosf(angle), -sinf(angle), 0},
					Vector4{0, sinf(angle),  cosf(angle), 0},
					Vector4{0, 0, 0, 1}
	};
}

Matrix4 Math::GetYRotationMatrix(float angle)
{
	return Matrix4{ Vector4{cosf(angle), 0, sinf(angle), 0},
					Vector4{0, 1, 0, 0},
					Vector4{-sinf(angle), 0, cosf(angle), 0},
					Vector4{0, 0, 0, 1}
	};
}

Matrix4 Math::GetZRotationMatrix(float angle)
{
	return Matrix4{ Vector4{cosf(angle), -sinf(angle), 0, 0},
					Vector4{sinf(angle),  cosf(angle), 0, 0},
					Vector4{0, 0, 1, 0},
					Vector4{0, 0, 0, 1}
	};
}

Matrix4 Math::GetRotationMatrix(const Vector3& rotation, const bool& reverse)
{
	// For cameras.
	if (reverse)
		return	GetYRotationMatrix(rotation.y) * GetXRotationMatrix(-rotation.x) * GetZRotationMatrix(rotation.z);


	return		GetZRotationMatrix(rotation.z) * GetXRotationMatrix(-rotation.x) * GetYRotationMatrix(rotation.y);
}

Matrix4 Math::GetTransformMatrix(const Vector3& position, const Vector3& rotation, const Vector3& scale, const bool& reverse, const bool& transformNormals)
{
	// For cameras.
	if (reverse)
		return	GetTranslationMatrix(position) * GetYRotationMatrix(rotation.y) * GetXRotationMatrix(rotation.x) * GetZRotationMatrix(rotation.z) * GetScaleMatrix(scale);

	if (transformNormals)
		return	GetTransformMatrix(position, rotation, scale).GetInverse().GetTransposed();

	return		GetScaleMatrix(scale) * GetZRotationMatrix(rotation.z) * GetXRotationMatrix(rotation.x) * GetYRotationMatrix(rotation.y) * GetTranslationMatrix(position);
}

Matrix4 Math::GetTransformMatrix(const Vector3& position, const Quaternion& rotation, const Vector3& scale)
{
	return  GetScaleMatrix(scale) * rotation.ToRotationMatrix() * GetTranslationMatrix(position);
}

Matrix3 Math::GetRotationMatrixFromTransform(const Matrix4& mat)
{
	Matrix3 m = { Vector3{ mat[0][0] , mat[0][1] , mat[0][2] },
					Vector3{ mat[1][0] , mat[1][1] , mat[1][2] },
					Vector3{ mat[2][0] , mat[2][1] , mat[2][2] }
	};

	return m;
}

Quaternion Math::GetQuaternionFromRotationMatrix(const Matrix3& mat)
{
	const float w = -1 * sqrtf(1 + mat[0][0] + mat[1][1] + mat[2][2]) / 2.f;
	return Quaternion(w,
		(mat[1][2] - mat[2][1]) / (w * 4.f),
		(mat[2][0] - mat[0][2]) / (w * 4.f),
		(mat[0][1] - mat[1][0]) / (w * 4.f));
}

Quaternion Math::GetQuaternionFromTransform(const Matrix4& mat)
{
	Matrix3 rot = GetRotationMatrixFromTransform(mat);

	return GetQuaternionFromRotationMatrix(rot);
}

Matrix4 Math::ParseMatrix4(const std::string& line)
{
	std::string s = line;
	std::string delimiter = ",";

	float values[16] = { 0 };

	size_t pos = 0;
	std::string token;
	int i = 0;
	while ((pos = s.find(delimiter)) != std::string::npos) {
		token = s.substr(0, pos);
		values[i] = std::stof(token);
		s.erase(0, pos + delimiter.length());
		i++;
	}

	Matrix4 m = values;

	return m;
}

Matrix4 Math::CreateProjectionMatrix(float fov, float aspect, float zNear, float zFar)
{
	float tanHalfFovy = tanf(fov / 2.0f);

	Matrix4 out = Matrix4(0.f); // Assuming Matrix4 has a zero function to initialize all elements to 0
	out[0][0] = 1.0f / (aspect * tanHalfFovy);
	out[1][1] = 1.0f / tanHalfFovy;
	out[2][2] = zFar / (zNear - zFar);
	out[2][3] = -1.0f;
	out[3][2] = -(zFar * zNear) / (zFar - zNear);
	return out;
}

Matrix4 Math::CreateViewMatrix(const Math::Vector3& position, const Math::Vector3& target, const Math::Vector3& up)
{
	Math::Matrix4 temp;
	const Math::Vector3 z = (position - target).GetNormalized();
	const Math::Vector3 x = up.Cross(z).GetNormalized();
	const Math::Vector3 y = z.Cross(x);
	const Math::Vector3 delta = Math::Vector3(-x.Dot(position), -y.Dot(position), -z.Dot(position));
	for (int i = 0; i < 3; i++)
	{
		temp[i][0] = x[i];
		temp[i][1] = y[i];
		temp[i][2] = z[i];
		temp[3][i] = delta[i];
	}
	temp[3][3] = 1;
	return temp;
}

Matrix4 Math::CreateOrthoMatrix(float left, float right, float bottom, float top, float zNear, float zFar)
{
	Matrix4 res = Matrix4(0.f);
	res[0][0] = 2.f / (right - left);
	res[1][1] = 2.f / (top - bottom);
	res[2][2] = -2.f / (zFar - zNear);
	res[3][0] = -(left + right) / (right - left);
	res[3][1] = -(top + bottom) / (top - bottom);
	res[3][2] = -(zFar + zNear) / (zFar - zNear);
	return res;
}

/*Math::Matrix4 Math::LookAt(Math::Vector3 cameraPosition, Math::Vector3 targetPosition, Math::Vector3 upVector)
{
	Vector3 f = (targetPosition - cameraPosition).GetNormalized();
	Vector3 s = (f.Cross(upVector)).GetNormalized();
	Vector3 u = s.Cross(f);
	float tab[16] = { s.x, u.x, -f.x, 0.f,
					  s.y, u.y, -f.y, 0.f,
					  s.z, u.z, -f.z, 0.f,
					-s.Dot(cameraPosition), -u.Dot(cameraPosition), f.Dot(cameraPosition) , 1.f };
	Matrix4 res1(tab);
	res1 = res1.GetTransposed();

	return res1;

}*/

Math::Matrix4 Math::LookAt(Math::Vector3 cameraPosition, Math::Vector3 targetPosition, Math::Vector3 upVector)
{
	Vector3 Z = (targetPosition - cameraPosition);
	Z.Normalize();
	Vector3 X = Z.Cross(upVector);
	X.Normalize();
	Vector3 Y = X.Cross(Z);

	float tab[16] = { X.x, Y.x, -Z.x, 0.f ,
					  X.y, Y.y, -Z.y, 0.f ,
					  X.z, Y.z, -Z.z, 0.f ,
				     -X.Dot(cameraPosition), -Y.Dot(cameraPosition), Z.Dot(cameraPosition), 1.f  };

	Matrix4 result(tab);
	return result;

}
