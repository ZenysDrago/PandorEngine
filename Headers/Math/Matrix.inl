#include "Matrix.h"

using namespace Math;

template <int R, int C>
inline Matrix<R, C> Matrix<R, C>::GetInverse()
{
	return Matrix<R, C>();
}

template <>
inline Matrix<2, 2> Matrix<2, 2>::GetInverse()
{
	Matrix2 val({ matrix[1][1], -matrix[0][1] },
		{ -matrix[1][0], matrix[0][0] });

	return val / val.det2();
}

template <>
inline Matrix<3, 3> Matrix<3, 3>::GetInverse()
{
	Matrix4 val{ Vector4{matrix[0][0], matrix[0][1], matrix[0][2], 0},
					Vector4{matrix[1][0], matrix[1][1], matrix[1][2], 0},
					Vector4{matrix[2][0], matrix[2][1], matrix[2][2], 0},
					Vector4{0, 0, 0, 1}
	};

	val.Inverse();
	Matrix3 result{ {val.GetValueAt(0,0), val.GetValueAt(0,1), val.GetValueAt(0,2)},
						{val.GetValueAt(1,0), val.GetValueAt(1,1), val.GetValueAt(1,2)},
						{val.GetValueAt(2,0), val.GetValueAt(2,1), val.GetValueAt(2,2)}
	};
	return result;
}

template <>
inline Matrix<4, 4> Matrix<4, 4>::GetInverse()
{
	Matrix2 a(Vector2{ matrix[0][0], matrix[0][1] }, Vector2{ matrix[1][0], matrix[1][1] });
	Matrix2 b(Vector2{ matrix[0][2], matrix[0][3] }, Vector2{ matrix[1][2], matrix[1][3] });
	Matrix2 c(Vector2{ matrix[2][0], matrix[2][1] }, Vector2{ matrix[3][0], matrix[3][1] });
	Matrix2 d(Vector2{ matrix[2][2], matrix[2][3] }, Vector2{ matrix[3][2], matrix[3][3] });

	Matrix4 result =
	{
		 (a - b * d.GetInverse() * c).GetInverse(),							-(a - b * d.GetInverse() * c).GetInverse() * b * d.GetInverse(),
		-(d - c * a.GetInverse() * b).GetInverse() * c * a.GetInverse(),	 (d - c * a.GetInverse() * b).GetInverse()
	};

	return result;
}

template <>
inline Matrix<4, 4> Matrix<4, 4>::CreateInverse(bool shouldTranspose)
{
	// Find determinant of matrix
	Matrix4 inverse(false);
	Matrix4 transpose;
	transpose = GetTransposed();

	float det = transpose.GetDeterminant(4);
	if (det == 0.0f)
	{
		printf("ERROR");
		return inverse;
	}

	// Find adjoint
	Matrix4 adj = transpose.CreateAdjMatrix();

	// Find Inverse using formula "inverse(A) = adj(A)/det(A)"
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			inverse[i][j] = adj[i][j] / float(det);

	if (!shouldTranspose)
		return inverse.GetTransposed();
	return inverse;
}

template<>
inline Matrix<4, 4> Matrix<4, 4>::CreateAdjMatrix()
{

	// temp is used to store cofactors of matrix
	Matrix4 temp;
	Matrix4 adj;
	int sign = 1;

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			// Get cofactor of matrix[i][j]
			temp = GetCofactor(i, j, 4);

			// sign of adj positive if sum of row
			// and column indexes is even.
			sign = ((i + j) % 2 == 0) ? 1 : -1;

			// Interchanging rows and columns to get the
			// transpose of the cofactor matrix
			adj[j][i] = (float)((sign) * (temp.GetDeterminant(3)));
		}
	}
	return adj;
}


template<>
inline Matrix<4, 4> Matrix<4, 4>::GetCofactor(int p, int q, int n)
{
	Matrix4 mat;
	int i = 0, j = 0;
	// Looping for each element of the matrix
	for (int row = 0; row < n; row++)
	{
		for (int col = 0; col < n; col++)
		{
			//  Copying into temporary matrix only those element
			//  which are not in given row and column
			if (row != p && col != q)
			{
				mat[i][j++] = matrix[row][col];

				// Row is filled, so increase row index and
				// reset col index
				if (j == n - 1)
				{
					j = 0;
					i++;
				}
			}
		}
	}
	return mat;
}



template <int R, int C>
float Matrix<R, C>::GetDeterminant(int n) const
{
	if (n == 2)
	{
		float result = matrix[0][0] * matrix[1][1] - matrix[1][0] * matrix[0][1];
		return result;
	}
	else if (n == 3)
	{
		float result = matrix[0][0] * matrix[1][1] * matrix[2][2]
			- matrix[0][0] * matrix[2][1] * matrix[1][2]
			+ matrix[1][0] * matrix[2][1] * matrix[0][2]
			- matrix[1][0] * matrix[0][1] * matrix[2][2]
			+ matrix[2][0] * matrix[0][1] * matrix[1][2]
			- matrix[2][0] * matrix[1][1] * matrix[0][2];
		return result;
	}
	else if (n == 4)
	{
		float result = matrix[0][0] * (matrix[1][1] * matrix[2][2] * matrix[3][3] // a(fkp
			- matrix[1][1] * matrix[3][2] * matrix[2][3] //flo
			- matrix[2][1] * matrix[1][2] * matrix[3][3] //gjp
			+ matrix[2][1] * matrix[3][2] * matrix[1][3] //gln
			+ matrix[3][1] * matrix[1][2] * matrix[2][3] //hjo
			- matrix[3][1] * matrix[2][2] * matrix[1][3]) // hkn

			- matrix[1][0] * (matrix[0][1] * matrix[2][2] * matrix[3][3] //b(ekp
				- matrix[0][1] * matrix[3][2] * matrix[2][3] // elo
				- matrix[2][1] * matrix[0][2] * matrix[3][3] //gip
				+ matrix[2][1] * matrix[3][2] * matrix[0][3] //glm
				+ matrix[3][1] * matrix[0][2] * matrix[2][3] //hio
				- matrix[3][1] * matrix[2][2] * matrix[0][3]) //hkm

			+ matrix[2][0] * (matrix[0][1] * matrix[1][2] * matrix[3][3] // c(ejp
				- matrix[0][1] * matrix[3][2] * matrix[1][3] //eln
				- matrix[1][1] * matrix[0][2] * matrix[3][3] //fip
				+ matrix[1][1] * matrix[3][2] * matrix[0][3] //flm
				+ matrix[3][1] * matrix[0][2] * matrix[1][3] //hin
				- matrix[3][1] * matrix[1][2] * matrix[0][3]) //hjm

			- matrix[3][0] * (matrix[0][1] * matrix[1][2] * matrix[2][3] // d(ejo
				- matrix[0][1] * matrix[2][2] * matrix[1][3] //ekn
				- matrix[1][1] * matrix[0][2] * matrix[2][3] //fio
				+ matrix[1][1] * matrix[2][2] * matrix[0][3] //fkm
				+ matrix[2][1] * matrix[0][2] * matrix[1][3] //gin
				- matrix[2][1] * matrix[1][2] * matrix[0][3]); //gjm
		return result;
	}
	else return 0.0f;
}


/* Get row and column as Vector 2 , 3 and 4 */
template <> template<>
inline Vector2 Matrix<2, 2>::GetRow(const int& rowIndex)
{
	return Vector2{ matrix[rowIndex][0]		, matrix[rowIndex][1] };
}

template <> template<>
inline Vector2 Matrix<2, 2>::GetColumn(const int& columnIndex)
{
	return Vector2{ matrix[0][columnIndex]	, matrix[1][columnIndex] };
}

template <> template<>
inline Vector3 Matrix<3, 3>::GetRow(const int& rowIndex)
{
	return Vector3{ matrix[rowIndex][0]		, matrix[rowIndex][1]	, matrix[rowIndex][2] };
}

template <> template<>
inline Vector3 Matrix<3, 3>::GetColumn(const int& columnIndex)
{
	return Vector3{ matrix[0][columnIndex]	, matrix[1][columnIndex]	, matrix[2][columnIndex] };
}

template <> template<>
inline Vector4 Matrix<4, 4>::GetRow(const int& rowIndex)
{
	return Vector4{ matrix[rowIndex][0]		, matrix[rowIndex][1]	, matrix[rowIndex][2]		, matrix[rowIndex][3] };
}

template <> template<>
inline Vector4 Matrix<4, 4>::GetColumn(const int& columnIndex)
{
	return Vector4{ matrix[0][columnIndex]	, matrix[1][columnIndex]	, matrix[2][columnIndex]	, matrix[3][columnIndex] };
}

