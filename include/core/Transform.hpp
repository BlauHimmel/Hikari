#pragma once

#include <core\Common.hpp>
#include <core\Ray.hpp>

NAMESPACE_BEGIN

/**
* \brief Homogeneous coordinate transformation
*
* This class stores a general homogeneous coordinate tranformation, such as
* rotation, translation, uniform or non-uniform scaling, and perspective
* transformations. The inverse of this transformation is also recorded
* here, since it is required when transforming normal vectors.
*/
struct Transform
{
public:
	/// Create the identity transform
	Transform();

	/// Create a new transform instance for the given matrix 
	Transform(const Eigen::Matrix4f & Trans);

	/// Create a new transform instance for the given matrix and its inverse
	Transform(const Eigen::Matrix4f & Trans, const Eigen::Matrix4f & InvTrans);

	/// Return the underlying matrix
	const Eigen::Matrix4f & GetMatrix() const;

	/// Return the inverse of the underlying matrix
	const Eigen::Matrix4f & GetInverseMatrix() const;

	/// Return the inverse transformation
	Transform Inverse() const;

	/// Concatenate with another transform
	Transform operator*(const Transform & Trans) const;

	/// Apply the homogeneous transformation to a 3D vector
	Vector3f operator*(const Vector3f & Vec) const;

	/// Apply the homogeneous transformation to a 3D normal
	Normal3f operator*(const Normal3f & Norm) const;

	/// Transform a point by an arbitrary matrix in homogeneous coordinates
	Point3f operator*(const Point3f & Pt) const;

	/// Apply the homogeneous transformation to a ray
	Ray3f operator*(const Ray3f & Ray) const;

	/// Return a string representation
	std::string ToString() const;

private:
	Eigen::Matrix4f m_Transform;
	Eigen::Matrix4f m_Inverse;
};

NAMESPACE_END