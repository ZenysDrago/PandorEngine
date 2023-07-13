#pragma once
#include "PandorAPI.h"

#include "Vector4.h"



namespace Math
{
	template<int R, int C>
	class Matrix;
	class Quaternion;

	/* typedef to create square Matrix*/
	typedef Matrix<2, 2>Matrix2;
	typedef Matrix<3, 3>Matrix3;
	typedef Matrix<4, 4>Matrix4;

	template<int R, int C>
	class Matrix
	{
	private:

		float matrix[R][C];
	public:
		/* Constructors */
		Matrix(bool allZero = false);
		Matrix(const Matrix<R, C>& copied);

		Matrix(const Vector4& r0, const Vector4& r1, const Vector4& r2, const Vector4& r3);	// Constructors for 4x4 matrix from vector4
		Matrix(const Vector3& r0, const Vector3& r1, const Vector3& r2);					// Constructors for 3x3 matrix from vector4
		Matrix(const Vector2& r0, const Vector2& r1);										// Constructors for 2x2 matrix from vector4

		Matrix(const Vector4& c0, const Vector4& c1, const Vector4& c2, const Vector4& c3, bool isColumn);	// Constructors for 4x4 matrix from vector4 as column , bool is useless
		Matrix(const Vector3& c0, const Vector3& c1, const Vector3& c2, bool isColumn);						// Constructors for 3x3 matrix from vector4 as column , bool is useless
		Matrix(const Vector2& c0, const Vector2& c1, bool isColumn);										// Constructors for 2x2 matrix from vector4 as column , bool is useless

		Matrix(const Matrix2& m0, const Matrix2& m1, const Matrix2& m2, const Matrix2& m3);
		Matrix(const float values[16]);

		/* operators */
		Matrix<R, C>& operator= (const Matrix<R, C>& m);
		bool		 operator==(const Matrix<R, C>& m) const;
		bool		 operator!=(const Matrix<R, C>& m) const;
		Matrix<R, C>  operator+ (const Matrix<R, C>& m) const;
		Matrix<R, C>  operator- (const Matrix<R, C>& m) const;
		void		 operator+=(const Matrix<R, C>& m);
		void		 operator-=(const Matrix<R, C>& m);
		Matrix<R, C>  operator* (const float& value)	const;
		Matrix<R, C>  operator/ (const float& value)	const;
		void		 operator*=(const float& value);
		void		 operator/=(const float& value);
		Matrix<R, C> operator-() const;
		Vector4 operator*(const Vector4& vec) const;
		Vector3 operator*(const Vector3& vec) const;

		float* operator[](int index);
		const float* operator[](int index) const;

		friend std::ostream& operator<<(std::ostream& os, const Matrix<R, C>& m)
		{
			for (int r = 0; r < R; r++)
			{
				for (int c = 0; c < C; c++)
					os << m[r][c] << ",";
			}

			return os;
		}



#ifdef GLM
		friend bool operator== (const Matrix<4,4>& m    ,const glm::mat4& glmMat);
		friend bool operator== (const glm::mat4& glmMat ,const Matrix<4, 4>& m  );
#endif
		

		/* Methods */
		int GetRowNumber();
		int GetColumnNumber();

		float GetValueAt(int i, int j) const;

		void SetValueAt(int i, int j, float value);

		bool IsOrthogonal();
		bool IsSquare();
		bool IsIdentity();

		void Inverse();

		float det2();
		float det3();
		float det4();

		void Print() const;

		Vector3 GetPosition() const;
		Vector3 GetRotation() const;
		Vector3 GetScale() const;

		Matrix<R, C>GetTransposed() const;
		void Transpose();

		template<int R2, int C2>
		Matrix<(R > R2 ? R : R2), (C > C2 ? C : C2)> operator*(const Matrix<R2, C2>& m) const
		{
			// Size condition to calculate
			if (C != R2)
			{
				std::cout << "Given matrices cannot be multiplied." << "\n";
				return Matrix<R2, C>();
			}

			Matrix<(R > R2 ? R : R2), (C > C2 ? C : C2)> result;
			for (int i = 0; i < R; i++)
			{
				for (int j = 0; j < C2; j++)
				{
					result[i][j] = 0;
					for (int k = 0; k < R2; k++)
						result[i][j] += matrix[i][k] * m.matrix[k][j];
				}
			}
			return result;
		}
		
		

		/* Templated function */
		Matrix<R, C> GetInverse();

		Matrix<R, C> CreateInverse(bool shouldTranspose = false);

		Matrix<R, C> CreateAdjMatrix();

		Matrix<R, C> GetCofactor(int p, int q, int n);

		float GetDeterminant(int n) const;

		template<typename V>
		V GetRow(const int& rowIndex) { return matrix[rowIndex]; }

		template<typename V>
		V GetColumn(const int& columnIndex) { return matrix[columnIndex]; }
		
	};

	/* Static function */
	PANDOR_API Matrix4 GetTranslationMatrix(const Vector3& translation);
	PANDOR_API Matrix4 GetScaleMatrix(const Vector3& scale);
	PANDOR_API Matrix4 GetXRotationMatrix(float angle);
	PANDOR_API Matrix4 GetYRotationMatrix(float angle);
	PANDOR_API Matrix4 GetZRotationMatrix(float angle);
	PANDOR_API Matrix4 GetRotationMatrix(const Vector3& rotation, const bool& reverse = false);
	PANDOR_API Matrix4 GetTransformMatrix(const Vector3& position, const Vector3& rotation, const Vector3& scale, const bool& reverse = false, const bool& transformNormals = false);
	PANDOR_API Matrix4 GetTransformMatrix(const Vector3& position, const Quaternion& rotation, const Vector3& scale = Vector3(1, 1, 1));
	PANDOR_API Matrix3 GetRotationMatrixFromTransform(const Matrix4& mat);
	PANDOR_API Quaternion GetQuaternionFromRotationMatrix(const Matrix3& mat);
	PANDOR_API Quaternion GetQuaternionFromTransform(const Matrix4& mat);
	PANDOR_API Matrix4 ParseMatrix4(const std::string& line); 
	PANDOR_API Matrix4 CreateProjectionMatrix(float fov, float aspect, float zNear, float zFar);
	PANDOR_API Matrix4 CreateViewMatrix(const Math::Vector3& position, const Math::Vector3& target, const Math::Vector3& up);
	PANDOR_API Matrix4 CreateOrthoMatrix(float left, float right, float bottom, float top, float zNear, float zFar);
	PANDOR_API Matrix4 LookAt(Math::Vector3 cameraPosition, Math::Vector3 targetPosition, Math::Vector3 upVector);

	/* ========================================================================	*/
	/*																			*/
	/*							Functions Definitions							*/
	/*																			*/
	/* ========================================================================	*/


	/* Constructors */
	template <int R, int C>
	Matrix<R, C>::Matrix(bool allZero)
	{
		/* Assert */
		if (R < 2 || C < 2)
		{
			std::cout << "matrix size not right" << "\n";
			return;
		}

		if (allZero)
		{
			for (int row = 0; row < R; row++)
				for (int column = 0; column < C; column++)
					matrix[row][column] = 0;
		}
		else
		{
			for (int row = 0; row < R; row++)
			{
				for (int column = 0; column < C; column++)
				{
					if (row == column)
						matrix[row][column] = 1;
					else
						matrix[row][column] = 0;
				}
			}
		}
	}

	template <int R, int C>
	Matrix<R, C>::Matrix(const Matrix<R, C>& copied)
	{
		*this = copied;
	}

	template <int R, int C>
	Matrix<R, C>::Matrix(const Vector4& r0, const Vector4& r1, const Vector4& r2, const Vector4& r3)	// Constructors for 4x4 matrix from vector4
	{
		if (R != 4 || C != 4)
		{
			std::cout << "Constructor only usable on square 4 matrix" << "\n";
			return;
		}

		for (int column = 0; column < C; column++)
			matrix[0][column] = r0[column];

		for (int column = 0; column < C; column++)
			matrix[1][column] = r1[column];

		for (int column = 0; column < C; column++)
			matrix[2][column] = r2[column];

		for (int column = 0; column < C; column++)
			matrix[3][column] = r3[column];
	}

	template <int R, int C>
	Matrix<R, C>::Matrix(const Vector3& r0, const Vector3& r1, const Vector3& r2)					// Constructors for 3x3 matrix from vector4
	{
		if (R != 3 || C != 3)
		{
			std::cout << "Constructor only usable on square 3 matrix" << "\n";
			return;
		}

		for (int column = 0; column < C; column++)
			matrix[0][column] = r0[column];

		for (int column = 0; column < C; column++)
			matrix[1][column] = r1[column];

		for (int column = 0; column < C; column++)
			matrix[2][column] = r2[column];

	}

	template <int R, int C>
	Matrix<R, C>::Matrix(const Vector2& r0, const Vector2& r1)										// Constructors for 2x2 matrix from vector4
	{
		if (R != 2 || C != 2)
		{
			std::cout << "Constructor only usable on square 2 matrix" << "\n";
			return;
		}

		for (int column = 0; column < C; column++)
			matrix[0][column] = r0[column];

		for (int column = 0; column < C; column++)
			matrix[1][column] = r1[column];
	}

	template <int R, int C>
	Matrix<R, C>::Matrix(const Vector4& c0, const Vector4& c1, const Vector4& c2, const Vector4& c3, bool isColumn)	// Constructors for 4x4 matrix from vector4 as column , bool is useless
	{
		if (R != 4 || C != 4)
		{
			std::cout << "Constructor only usable on square 4 matrix" << "\n";
			return;
		}

		for (int row = 0; row < C; row++)
			matrix[row][0] = c0[row];

		for (int row = 0; row < C; row++)
			matrix[row][1] = c1[row];

		for (int row = 0; row < C; row++)
			matrix[row][2] = c2[row];

		for (int row = 0; row < C; row++)
			matrix[row][3] = c3[row];
	}

	template <int R, int C>
	Matrix<R, C>::Matrix(const Vector3& c0, const Vector3& c1, const Vector3& c2, bool isColumn)						// Constructors for 3x3 matrix from vector4 as column , bool is useless
	{
		if (R != 3 || C != 3)
		{
			std::cout << "Constructor only usable on square 3 matrix" << "\n";
			return;
		}

		for (int row = 0; row < C; row++)
			matrix[row][0] = c0[row];

		for (int row = 0; row < C; row++)
			matrix[row][1] = c1[row];

		for (int row = 0; row < C; row++)
			matrix[row][2] = c2[row];

	}

	template <int R, int C>
	Matrix<R, C>::Matrix(const Vector2& c0, const Vector2& c1, bool isColumn)										// Constructors for 2x2 matrix from vector4 as column , bool is useless
	{
		if (R != 2 || C != 2)
		{
			std::cout << "Constructor only usable on square 2 matrix" << "\n";
			return;
		}

		for (int row = 0; row < C; row++)
			matrix[row][0] = c0[row];

		for (int row = 0; row < C; row++)
			matrix[row][1] = c1[row];

	}

	template <int R, int C>
	Matrix<R, C>::Matrix(const Matrix2& m0, const Matrix2& m1, const Matrix2& m2, const Matrix2& m3)
	{
		if (R != 4 || C != 4)
		{
			std::cout << "Constructor only usable on square 4 matrix" << "\n";
			return;
		}

		matrix[0][0] = m0[0][0]; matrix[0][1] = m0[0][1]; matrix[0][2] = m1[0][0]; matrix[0][3] = m1[0][1];
		matrix[1][0] = m0[1][0]; matrix[1][1] = m0[1][1]; matrix[1][2] = m1[1][0]; matrix[1][3] = m1[1][1];
		matrix[2][0] = m2[0][0]; matrix[2][1] = m2[0][1]; matrix[2][2] = m3[0][0]; matrix[2][3] = m3[0][1];
		matrix[3][0] = m2[1][0]; matrix[3][1] = m2[1][1]; matrix[3][2] = m3[1][0]; matrix[3][3] = m3[1][1];
	}

	template<int R, int C>
	inline Matrix<R, C>::Matrix(const float values[16])
	{
		if (R != 4 || C != 4)
		{
			std::cout << "Constructor only usable on square 4 matrix" << "\n";
			return;
		}

		for (int r = 0; r < R; r++)
		{
			for (int c = 0; c < C; c++)
			{
				matrix[r][c] = values[r*R + c];
			}
		}
	}

	/* operators */
	template <int R, int C>
	Matrix<R, C>& Matrix<R, C>::operator= (const Matrix<R, C>& m)
	{
		for (int row = 0; row < R; row++)
			for (int column = 0; column < C; column++)
					matrix[row][column] = m.matrix[row][column];

		return *this;
	}

	template <int R, int C>
	bool Matrix<R, C>::operator==(const Matrix<R, C>& m) const
	{
		for (int row = 0; row < R; row++)
			for (int column = 0; column < C; column++)
				if (matrix[row][column] != m.matrix[row][column])
					return false;

		return true;
	}

	template <int R, int C>
	bool Matrix<R, C>::operator!=(const Matrix<R, C>& m) const
	{
		return !((*this) == m);
	}

	template <int R, int C>
	Matrix<R, C>  Matrix<R, C>::operator+ (const Matrix<R, C>& m) const
	{
		Matrix<R, C> res;

		for (int row = 0; row < R; row++)
			for (int column = 0; column < C; column++)
				res.matrix[row][column] = matrix[row][column] + m.matrix[row][column];

		return res;
	}

	template <int R, int C>
	Matrix<R, C>  Matrix<R, C>::operator- (const Matrix<R, C>& m) const
	{
		Matrix<R, C> res;

		for (int row = 0; row < R; row++)
			for (int column = 0; column < C; column++)
				res.matrix[row][column] = matrix[row][column] - m.matrix[row][column];

		return res;
	}

	template <int R, int C>
	void Matrix<R, C>::operator+=(const Matrix<R, C>& m)
	{
		*this = (*this) + m;
	}

	template <int R, int C>
	void Matrix<R, C>::operator-=(const Matrix<R, C>& m)
	{
		*this = (*this) - m;
	}

	template <int R, int C>
	Matrix<R, C>  Matrix<R, C>::operator* (const float& value)	const
	{
		Matrix<R, C> res;

		for (int row = 0; row < R; row++)
			for (int column = 0; column < C; column++)
				res[row][column] = matrix[row][column] * value;

		return res;
	}

	template <int R, int C>
	Matrix<R, C>  Matrix<R, C>::operator/ (const float& value)	const
	{
		Matrix<R, C> res;

		for (int row = 0; row < R; row++)
			for (int column = 0; column < C; column++)
				res[row][column] = matrix[row][column] / value;

		return res;
	}

	template <int R, int C>
	void Matrix<R, C>::operator*=(const float& value)
	{
		*this = (*this) * value;
	}

	template <int R, int C>
	void Matrix<R, C>::operator/=(const float& value)
	{
		*this = (*this) / value;
	}

	template <int R, int C>
	Matrix<R, C> Matrix<R, C>::operator-() const
	{
		Matrix<R, C> tmp;
		for (int i = 0; i < R; i++)
			for (int j = 0; j < C; j++)
				tmp[i][j] = -matrix[i][j];
		return tmp;
	}

	template <int R, int C>
	Vector4 Matrix<R, C>::operator*(const Vector4& vec) const
	{
		Vector4 vecResult = vec;

		float value[4];
		for(int i = 0; i < 4; i++)
			value[i] = vec.x * matrix[i][0] + vec.y * matrix[i][1] + vec.z * matrix[i][2] + vec.w * matrix[i][3];

		vecResult.x = value[0];
		vecResult.y = value[1];
		vecResult.z = value[2];
		vecResult.w = value[3];

		return vecResult;
	}

	template <int R, int C>
	Vector3 Matrix<R, C>::operator*(const Vector3& vec) const
	{
		Vector4 vecCal = Vector4(vec, 0);

		vecCal = (*this) * vecCal;
		Vector3 vecResult = vecCal.ToVector3();
		
		return vecResult;
	}
	
	template <int R, int C>
	float* Matrix<R, C>::operator[](int index) { return matrix[index]; }

	template <int R, int C>
	const float* Matrix<R, C>::operator[](int index) const { return matrix[index]; }

	/* Methods */
	template <int R, int C>
	int Matrix<R, C>::GetRowNumber()
	{
		return R;
	}

	template <int R, int C>
	int Matrix<R, C>::GetColumnNumber()
	{
		return C;
	}

	//template<int R, int C>
	//inline float* Matrix<R, C>::GetPtr() const
	//{
	//	return matrix;
	//}

	template <int R, int C>
	float Matrix<R, C>::GetValueAt(int row, int column) const
	{
		return matrix[column][row];
	}


	template<int R, int C>
	void Matrix<R, C>::SetValueAt(int i, int j, float value)
	{
		matrix[i][j] = value;
	}

	template<int R, int C>
	inline bool Matrix<R, C>::IsOrthogonal()
	{
		return ((*this) * GetTransposed()).IsIdentity();
	}

	template <int R, int C>
	bool Matrix<R, C>::IsSquare()
	{
		return R == C;
	}

	template <int R, int C>
	bool Matrix<R, C>::IsIdentity()
	{
		for (int i = 0; i < R; i++)
			for (int j = 0; j < C; j++)
				if ((i != j && matrix[i][j] != 0) || (i == j && matrix[i][j] != 1))
					return false;

		return true;
	}

	template <int R, int C>
	void Matrix<R, C>::Inverse() { *this = GetInverse(); }

	template <int R, int C>
	float Matrix<R, C>::det2()
	{
		return (matrix[0][0] * matrix[1][1]) - (matrix[0][1] * matrix[1][0]);
	}

	template <int R, int C>
	float Matrix<R, C>::det3()
	{
		if (R < 3 || C < 3)
		{
			std::cout << "Matrix is not usable for det3" << "\n";
			return -1;
		}

		return	matrix[0][0] * Matrix2{ Vector2(matrix[1][1], matrix[1][2]), Vector2(matrix[2][1], matrix[2][2]) }.det2() -
				matrix[0][1] * Matrix2{ Vector2(matrix[1][0], matrix[1][2]), Vector2(matrix[2][0], matrix[2][2]) }.det2() +
				matrix[0][2] * Matrix2{ Vector2(matrix[1][0], matrix[1][1]), Vector2(matrix[2][0], matrix[2][1]) }.det2();
	}

	template <int R, int C>
	float Matrix<R, C>::det4()
	{
		if (R < 4 || C < 4)
		{
			std::cout << "Matrix is not usable for det4" << "\n";
			return -1;
		}

		Matrix3 a(Vector3(matrix[1][1], matrix[1][2], matrix[1][3]), Vector3(matrix[2][1], matrix[2][2], matrix[2][3]), Vector3(matrix[3][1], matrix[3][2], matrix[3][3]));
		Matrix3 b(Vector3(matrix[1][0], matrix[1][2], matrix[1][3]), Vector3(matrix[2][0], matrix[2][2], matrix[2][3]), Vector3(matrix[3][0], matrix[3][2], matrix[3][3]));
		Matrix3 c(Vector3(matrix[1][0], matrix[1][1], matrix[1][3]), Vector3(matrix[2][0], matrix[2][1], matrix[2][3]), Vector3(matrix[3][0], matrix[3][1], matrix[3][3]));
		Matrix3 d(Vector3(matrix[1][0], matrix[1][1], matrix[1][2]), Vector3(matrix[2][0], matrix[2][1], matrix[2][2]), Vector3(matrix[3][0], matrix[3][1], matrix[3][2]));

		return (a.det3() * matrix[0][0] - b.det3() * matrix[0][1] + c.det3() * matrix[0][2] - d.det3() * matrix[0][3]);
	}

	template <int R, int C>
	void Matrix<R, C>::Print() const
	{
		std::cout << "================" << std::endl;
		// Print content
		for (int i = 0; i < R; i++)
		{
			for (int j = 0; j < C; j++) printf(" [%.5f] ", matrix[i][j]);
			printf("\n");
		}
		printf("\n");
	}

	template<int R, int C>
	inline Vector3 Matrix<R, C>::GetPosition() const
	{
		if (R != 4 || C != 4)
		{
			std::cout << "Not right matrix size : abort" << std::endl;
			return Vector3();
		}

		return Vector3(matrix[3][0], matrix[3][1], matrix[3][2]);
	}

	template<int R, int C>
	inline Vector3 Matrix<R, C>::GetRotation() const
	{
		if (R != 4 || C != 4)
		{
			std::cout << "Not right matrix size : abort" << std::endl;
			return Vector3();
		}

		Vector3 sca = GetScale();
		Matrix4 rotMat;
		// Get Rotation Matrix.
		if (sca == Vector3(0))
			sca = Vector3(0.0001f);
		rotMat[0][0] = (*this)[0][0] / sca.x;
		rotMat[0][1] = (*this)[0][1] / sca.x;
		rotMat[0][2] = (*this)[0][2] / sca.x;
		rotMat[1][0] = (*this)[1][0] / sca.y;
		rotMat[1][1] = (*this)[1][1] / sca.y;
		rotMat[1][2] = (*this)[1][2] / sca.y;
		rotMat[2][0] = (*this)[2][0] / sca.z;
		rotMat[2][1] = (*this)[2][1] / sca.z;
		rotMat[2][2] = (*this)[2][2] / sca.z;
		rotMat[3][3] = 1;

		// Get Rotation from rotation matrix.
		float thetaX, thetaY, thetaZ;
		if (rotMat[2][1] < 1)
		{
			if (rotMat[2][1] > -1)
			{
				thetaX = asin(-rotMat[2][1]);
				thetaY = atan2(rotMat[2][0], rotMat[2][2]);
				thetaZ = atan2(rotMat[0][1], rotMat[1][1]);
			}
			else
			{
				thetaX = (float)PI / 2;
				thetaY = -atan2(rotMat[1][0], rotMat[0][0]);
				thetaZ = 0;
			}
		}
		else
		{
			thetaX = -PI / 2;
			thetaY = atan2(rotMat[1][0], rotMat[0][0]);
			thetaZ = 0;
		}
		return Vector3(thetaX, thetaY, thetaZ) * RAD2DEG;
	}

	template<int R, int C>
	inline Vector3 Matrix<R, C>::GetScale() const
	{
		if (R != 4 || C != 4)
		{
			std::cout << "Not right matrix size : abort" << std::endl;
			return Vector3();
		}

		// World Scale equal length of columns of the model matrix.
		float x = Vector3(matrix[0][0], matrix[1][0], matrix[2][0]).Length();
		float y = Vector3(matrix[0][1], matrix[1][1], matrix[2][1]).Length();
		float z = Vector3(matrix[0][2], matrix[1][2], matrix[2][2]).Length();
		return Vector3(x, y, z);
	}

	template<int R, int C>
	inline Matrix<R, C> Matrix<R, C>::GetTransposed() const
	{
		Matrix<C, R> result;
		for (int i = 0; i < R; i++)
			for (int j = 0; j < C; j++)
				result[j][i] = matrix[i][j];
		return result;
	}

	template <int R, int C>
	void Matrix<R, C>::Transpose()
	{
		(*this) = GetTransposed();
	}

}

#include "Matrix.inl"