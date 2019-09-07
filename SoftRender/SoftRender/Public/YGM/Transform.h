﻿#ifndef _YRENDER_YGM_TRANSFORM_H
#define _YRENDER_YGM_TRANSFORM_H

#include <Public/YGM/Vector3.hpp>
#include <Public/YGM/Matrix4.hpp>


namespace YRender {
	namespace YGM {
		//opengl坐标系，除了NDC外都使用右手坐标系，但是在相机的局部空间下是可以视为左手坐标系。（因为刚好z轴翻转）
		class Transform {
		public:
			Transform(float d) :Matrix(d) {}//InvMatrix(1.f/d){};
			Transform(const Mat4f& matrix) : Matrix(matrix) {} //InvMatrix(matrix.Inverse()) {}
			Transform(const Mat4f& matrix, const Mat4f& invMatrix) :Matrix(matrix),InvMatrix(invMatrix){};



		public:
			static const Transform LookAt(const Vector3& pos, const Vector3& target,const Vector3& up = Vector3(0,1,0));
			static const Transform Perspective(const float fovy, const float aspect, const float zNear, const float zFar);

		public:
			const Mat4f& GetMatrix() const {
				return Matrix;
			};

			//const Mat4f& GetInvMatrix() const {
			//	return InvMatrix;
			//}

		private:
			Mat4f Matrix;
			Mat4f InvMatrix;
		};
	}
}



#endif

