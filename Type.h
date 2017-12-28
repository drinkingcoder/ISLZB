#pragma once

#include "Eigen/Eigen"
#include "Eigen/Core"
#include "Eigen/Geometry"

typedef float real_type;
typedef Eigen::Matrix<real_type, 3, 1> Vector3;
typedef Eigen::Matrix<real_type, 2, 1> Vector2;

typedef Eigen::Matrix<real_type, 4, 4> Matrix4;
typedef Eigen::Matrix<real_type, 3, 3> Matrix3;

inline Vector3 transform(const Matrix4 & M, const Vector3 & v) {
    return M.block(0, 0, 3, 3) * v + M.block(0, 3, 3, 1);
}

inline Vector3 rotate(const Matrix4 & M, const Vector3 & v) {
    return M.block(0, 0, 3, 3) * v;
}