#ifndef _YRENDER_YGM_MAT4x4_HPP_
#define _YRENDER_YGM_MAT4x4_HPP_

#include <Public/YGM/Vector4.hpp>
#include <iostream>


namespace YGM {
	template<typename T>
	class Mat4x4 {
		//private:
		//	using type_value = T;
	public:
		Mat4x4(T d0, T d1, T d2, T d3) : m{
			{d0, static_cast<T>(0),static_cast<T>(0),static_cast<T>(0)},
			{static_cast<T>(0),d1,static_cast<T>(0),static_cast<T>(0)},
			{static_cast<T>(0),static_cast<T>(0),d2,static_cast<T>(0)},
			{static_cast<T>(0),static_cast<T>(0),static_cast<T>(0),d3} }
		{}

		explicit Mat4x4(T d = static_cast<T>(1)) : Mat4x4(d, d, d, d) { }

		Mat4x4(const Vector<4, T> & row0, const Vector<4, T> & row1, const Vector<4, T> & row2, const Vector<4, T> & row3)
			:m{ row0, row1, row2, row3 } { }

		// mat Ϊ�����򣬵���glsl���ǰ��н��������ڴ��е�ֵ��00 10 20 30����
		explicit Mat4x4(const T mat[4][4]) { memcpy(m, mat, 16 * sizeof(T)); }

		Mat4x4(
			T t00, T t01, T t02, T t03,
			T t10, T t11, T t12, T t13,
			T t20, T t21, T t22, T t23,
			T t30, T t31, T t32, T t33
		) : m{
		{t00, t01, t02, t03},
		{t10, t11, t12, t13},
		{t20, t21, t22, t23},
		{t30, t31, t32, t33} } { }

		Mat4x4(const Mat4x4<T>& mat) { memcpy(m, mat.m, 16 * sizeof(T)); }

	public:
		const Vector<4, T>& GetRow(int i) const { return m[i]; }
		Vector<4, T>& GetRow(int i) { return m[i]; }

		// ������
		T* Data() { return &(m[0][0]); }

		// ������
		const T* Data() const { return const_cast<T*>(&(m[0][0])); }

	public:
		bool IsIdentity() const {
			static constexpr T one = static_cast<T>(1);
			static constexpr T zero = static_cast<T>(0);

			return (m[0][0] == one && m[0][1] == zero && m[0][2] == zero && m[0][3] == zero
				&& m[1][0] == zero && m[1][1] == one && m[1][2] == zero && m[1][3] == zero
				&& m[2][0] == zero && m[2][1] == zero && m[2][2] == one && m[2][3] == zero
				&& m[3][0] == zero && m[3][1] == zero && m[3][2] == zero && m[3][3] == one);
		}

		T Tr() const { return m[0][0] + m[1][1] + m[2][2] + m[3][3]; }

		Mat4x4 Transpose() const {
			const auto& mat = *this;
			return Mat4x4(
				mat(0, 0), mat(1, 0), mat(2, 0), mat(3, 0),
				mat(0, 1), mat(1, 1), mat(2, 1), mat(3, 1),
				mat(0, 2), mat(1, 2), mat(2, 2), mat(3, 2),
				mat(0, 3), mat(1, 3), mat(2, 3), mat(3, 3));
		}

		const Mat4x4& selfTranspose() {
			for (int i = 0; i < 4; ++i) {
				for (int j = i; j < 4; ++j) {
					if (i == j)
						continue;
					std::swap(m[i][j], m[j][i]);
				}
			}
			return *this;
		}

		Mat4x4 Inverse() const {
			int indxc[4], indxr[4];
			int ipiv[4] = { 0, 0, 0, 0 };
			float minv[4][4];
			memcpy(minv, m, 4 * 4 * sizeof(float));
			for (int i = 0; i < 4; i++) {
				int irow = -1, icol = -1;
				float big = 0.;
				// Choose pivot
				for (int j = 0; j < 4; j++) {
					if (ipiv[j] != 1) {
						for (int k = 0; k < 4; k++) {
							if (ipiv[k] == 0) {
								if (fabsf(minv[j][k]) >= big) {
									big = float(fabsf(minv[j][k]));
									irow = j;
									icol = k;
								}
							}
							else if (ipiv[k] > 1)
								std::cerr << "Singular matrix in MatrixInvert" << std::endl;
						}
					}
				}
				++ipiv[icol];
				// Swap rows _irow_ and _icol_ for pivot
				if (irow != icol) {
					for (int k = 0; k < 4; ++k)
						std::swap(minv[irow][k], minv[icol][k]);
				}
				indxr[i] = irow;
				indxc[i] = icol;
				if (minv[icol][icol] == 0.)
				{
					std::cerr << "Singular matrix in MatrixInvert" << std::endl;
				}
				// Set m[icol][icol] to one by scaling row _icol_ appropriately
				float pivinv = 1.f / minv[icol][icol];
				minv[icol][icol] = 1.f;
				for (int j = 0; j < 4; j++)
					minv[icol][j] *= pivinv;

				// Subtract this row from others to zero out their columns
				for (int j = 0; j < 4; j++) {
					if (j != icol) {
						float save = minv[j][icol];
						minv[j][icol] = 0;
						for (int k = 0; k < 4; k++)
							minv[j][k] -= minv[icol][k] * save;
					}
				}
			}
			// Swap columns to reflect permutation
			for (int j = 3; j >= 0; j--) {
				if (indxr[j] != indxc[j]) {
					for (int k = 0; k < 4; k++)
						std::swap(minv[k][indxr[j]], minv[k][indxc[j]]);
				}
			}
			return Mat4x4(minv);

			//int indxc[4], indxr[4];
			//int ipiv[4] = { 0, 0, 0, 0 };
			//T minv[4][4];
			//memcpy(minv, m, 4 * 4 * sizeof(T));
			//for (int i = 0; i < 4; i++) {
			//	int irow = 0, icol = 0;
			//	T big = static_cast<T>(0);
			//	// Choose pivot
			//	//��¼���ֵ�����У��ں�������Ӧ�е����Խ�Ԫֵ�Ƿ�Ϊ0��
			//	for (int j = 0; j < 4; j++) {
			//		if (ipiv[j] != 1) {
			//			for (int k = 0; k < 4; k++) {
			//				if (ipiv[k] == 0) {
			//					if (std::abs(minv[j][k]) >= big) {
			//						big = static_cast<T>(std::abs(minv[j][k]));
			//						irow = j;
			//						icol = k;
			//					}
			//				}
			//				//����������²������ĳ���Խ�Ԫ��Ӧֵ����1
			//				//else if (ipiv[k] > 1)
			//				//	return Mat4x4(static_cast<T>(0));
			//			}
			//		}
			//	}
			//	//���õ�ǰ�е����Խ�ԪֵΪ1��֮����ѡȡ��Ԫʱ������Ը��и���
			//	++ipiv[icol];
			//	//�滻pivot��[icol][icol]λ�ã���λ��Ϊ��ǰ�е����Խ��ߴ������ﲢû���滻�����Ͻ�
			//	if (irow != icol) {
			//		for (int k = 0; k < 4; ++k)
			//			std::swap(minv[irow][k], minv[icol][k]);
			//	}
			//	//���ڼ�¼����λ�õ��������飬�������Ľ��Ϊ�����ĺ�����
			//	indxr[i] = irow;
			//	indxc[i] = icol;
			//	if (minv[icol][icol] == static_cast<T>(0))
			//		return Mat4x4(static_cast<T>(0));

			//	// Set $m[icol][icol]$ to one by scaling row _icol_ appropriately
			//	//����Ӧ���Խ�Ԫ���г���ԭֵ
			//	T pivinv = static_cast<T>(1) / minv[icol][icol];
			//	minv[icol][icol] = 1.;
			//	for (int j = 0; j < 4; j++)
			//		minv[j][icol] *= pivinv;

			//	// Subtract this row from others to zero out their columns
			//	//���������м�ȥ���н��������㣬�����һ����ֵ�ڵ����е����У����������������ж�Ҫ��ȥ���Ӧ�е�ֵ���ϵ����ж�Ӧ�����õ����е�ֵΪ�㣨��ͨ�ĸ�˹��Ԫ��
			//	//���Ϊ�������£�
			//	//[a1 b1 c1 d1] = v1        v1 - d1*(v4) = [a1-a4*d1,b1-b4*d1,c1-c4*d1,0]
			//	//[a2 b2 c2 d2] = v2        v2 - d2*(v4)
			//	//[a3 b3 c3 d3] = v3  ==>   v3 - d3*(v4)
			//	//[a4 b4 c4 1] = v4         v4

			//	//������Ĵ�������ĵط�����minv[icol][j] = 0; 
			//	//����Ӧ�е�ֵ����뵥λ��������ͬ��Ȼ���پ�����ͬ�ļ��������������λ����任��Ľ����ͬ
			//	//������Ӧ������������Ԫֵ��ͬ��������֤�˺�������ʱ������Ȼ��ȷ���ǵ�λ����ı任���Ҳ���������
			//	for (int j = 0; j < 4; j++) {
			//		if (j != icol) {
			//			T save = minv[icol][j];
			//			minv[icol][j] = 0;
			//			for (int k = 0; k < 4; k++)
			//				minv[k][j] -= minv[k][icol] * save;
			//		}
			//	}
			//}

			////��󽫽���ǰ��˳��ԭ���������������Ǿ����н������������������൱�ڳ���ԭʼ�б任����ԭ˳��
			//// Swap columns to reflect permutation
			//for (int j = 3; j >= 0; j--) {
			//	if (indxr[j] != indxc[j]) {
			//		for (int k = 0; k < 4; k++)
			//			std::swap(minv[indxr[j]][k], minv[indxc[j]][k]);
			//	}
			//}
			//return Mat4x4(minv);
		}

	public:
		Mat4x4& operator=(const Mat4x4& rhs) {
			memcpy(m, rhs.m, 16 * sizeof(T));
			return *this;
		}

		//Mat4x4& operator=(Mat4x4&& rhs) {

		//}

		bool operator ==(const Mat4x4 & rhs) const {
			return
				m[0][0] == rhs.m[0][0]
				&& m[0][1] == rhs.m[0][1]
				&& m[0][2] == rhs.m[0][2]
				&& m[0][3] == rhs.m[0][3]
				&& m[1][0] == rhs.m[1][0]
				&& m[1][1] == rhs.m[1][1]
				&& m[1][2] == rhs.m[1][2]
				&& m[1][3] == rhs.m[1][3]
				&& m[2][0] == rhs.m[2][0]
				&& m[2][1] == rhs.m[2][1]
				&& m[2][2] == rhs.m[2][2]
				&& m[2][3] == rhs.m[2][3]
				&& m[3][0] == rhs.m[3][0]
				&& m[3][1] == rhs.m[3][1]
				&& m[3][2] == rhs.m[3][2]
				&& m[3][3] == rhs.m[3][3];
		}

		bool operator!=(const Mat4x4 & rhs) const {
			return
				m[0][0] != rhs.m[0][0]
				|| m[0][1] != rhs.m[0][1]
				|| m[0][2] != rhs.m[0][2]
				|| m[0][3] != rhs.m[0][3]
				|| m[1][0] != rhs.m[1][0]
				|| m[1][1] != rhs.m[1][1]
				|| m[1][2] != rhs.m[1][2]
				|| m[1][3] != rhs.m[1][3]
				|| m[2][0] != rhs.m[2][0]
				|| m[2][1] != rhs.m[2][1]
				|| m[2][2] != rhs.m[2][2]
				|| m[2][3] != rhs.m[2][3]
				|| m[3][0] != rhs.m[3][0]
				|| m[3][1] != rhs.m[3][1]
				|| m[3][2] != rhs.m[3][2]
				|| m[3][3] != rhs.m[3][3];
		}

		const Mat4x4 operator*(const Mat4x4 & rhs) const {
			const auto& lhs = *this;
			T t00 = lhs(0, 0) * rhs(0, 0) + lhs(0, 1) * rhs(1, 0) + lhs(0, 2) * rhs(2, 0) + lhs(0, 3) * rhs(3, 0);
			T t01 = lhs(0, 0) * rhs(0, 1) + lhs(0, 1) * rhs(1, 1) + lhs(0, 2) * rhs(2, 1) + lhs(0, 3) * rhs(3, 1);
			T t02 = lhs(0, 0) * rhs(0, 2) + lhs(0, 1) * rhs(1, 2) + lhs(0, 2) * rhs(2, 2) + lhs(0, 3) * rhs(3, 2);
			T t03 = lhs(0, 0) * rhs(0, 3) + lhs(0, 1) * rhs(1, 3) + lhs(0, 2) * rhs(2, 3) + lhs(0, 3) * rhs(3, 3);
			T t10 = lhs(1, 0) * rhs(0, 0) + lhs(1, 1) * rhs(1, 0) + lhs(1, 2) * rhs(2, 0) + lhs(1, 3) * rhs(3, 0);
			T t11 = lhs(1, 0) * rhs(0, 1) + lhs(1, 1) * rhs(1, 1) + lhs(1, 2) * rhs(2, 1) + lhs(1, 3) * rhs(3, 1);
			T t12 = lhs(1, 0) * rhs(0, 2) + lhs(1, 1) * rhs(1, 2) + lhs(1, 2) * rhs(2, 2) + lhs(1, 3) * rhs(3, 2);
			T t13 = lhs(1, 0) * rhs(0, 3) + lhs(1, 1) * rhs(1, 3) + lhs(1, 2) * rhs(2, 3) + lhs(1, 3) * rhs(3, 3);
			T t20 = lhs(2, 0) * rhs(0, 0) + lhs(2, 1) * rhs(1, 0) + lhs(2, 2) * rhs(2, 0) + lhs(2, 3) * rhs(3, 0);
			T t21 = lhs(2, 0) * rhs(0, 1) + lhs(2, 1) * rhs(1, 1) + lhs(2, 2) * rhs(2, 1) + lhs(2, 3) * rhs(3, 1);
			T t22 = lhs(2, 0) * rhs(0, 2) + lhs(2, 1) * rhs(1, 2) + lhs(2, 2) * rhs(2, 2) + lhs(2, 3) * rhs(3, 2);
			T t23 = lhs(2, 0) * rhs(0, 3) + lhs(2, 1) * rhs(1, 3) + lhs(2, 2) * rhs(2, 3) + lhs(2, 3) * rhs(3, 3);
			T t30 = lhs(3, 0) * rhs(0, 0) + lhs(3, 1) * rhs(1, 0) + lhs(3, 2) * rhs(2, 0) + lhs(3, 3) * rhs(3, 0);
			T t31 = lhs(3, 0) * rhs(0, 1) + lhs(3, 1) * rhs(1, 1) + lhs(3, 2) * rhs(2, 1) + lhs(3, 3) * rhs(3, 1);
			T t32 = lhs(3, 0) * rhs(0, 2) + lhs(3, 1) * rhs(1, 2) + lhs(3, 2) * rhs(2, 2) + lhs(3, 3) * rhs(3, 2);
			T t33 = lhs(3, 0) * rhs(0, 3) + lhs(3, 1) * rhs(1, 3) + lhs(3, 2) * rhs(2, 3) + lhs(3, 3) * rhs(3, 3);
			return Mat4x4(
				t00, t01, t02, t03,
				t10, t11, t12, t13,
				t20, t21, t22, t23,
				t30, t31, t32, t33
			);
		}

		const Vector<4, T> operator*(const Vector<4, T>& rhs) const {
			return Vector<4, T>
				(
					this->m[0][0] * rhs[0] + this->m[0][1] * rhs[1] + this->m[0][2] * rhs[2] + this->m[0][3] * rhs[3],
					this->m[1][0] * rhs[0] + this->m[1][1] * rhs[1] + this->m[1][2] * rhs[2] + this->m[1][3] * rhs[3],
					this->m[2][0] * rhs[0] + this->m[2][1] * rhs[1] + this->m[2][2] * rhs[2] + this->m[2][3] * rhs[3],
					this->m[3][0] * rhs[0] + this->m[3][1] * rhs[1] + this->m[3][2] * rhs[2] + this->m[3][3] * rhs[3]
					);
		}



		T operator()(int row, int col) const {
			return m[row][col];
		}

		T& operator()(int row, int col) {
			return m[row][col];
		}




		friend std::ostream & operator<<(std::ostream& os, const Mat4x4& mat) {
			os << "[" << Math::ToZero(mat(0, 0)) << ", " << Math::ToZero(mat(0, 1)) << ", " << Math::ToZero(mat(0, 2)) << ", " << Math::ToZero(mat(0, 3)) << std::endl;
			os << Math::ToZero(mat(1, 0)) << ", " << Math::ToZero(mat(1, 1)) << ", " << Math::ToZero(mat(1, 2)) << ", " << Math::ToZero(mat(1, 3)) << std::endl;
			os << Math::ToZero(mat(2, 0)) << ", " << Math::ToZero(mat(2, 1)) << ", " << Math::ToZero(mat(2, 2)) << ", " << Math::ToZero(mat(2, 3)) << std::endl;
			os << Math::ToZero(mat(3, 0)) << ", " << Math::ToZero(mat(3, 1)) << ", " << Math::ToZero(mat(3, 2)) << ", " << Math::ToZero(mat(3, 3)) << "]";
			return os;
		}

	private:
		Vector<4, T> m[4];
	};
}
using Mat4f = YGM::Mat4x4<float>;



#endif
