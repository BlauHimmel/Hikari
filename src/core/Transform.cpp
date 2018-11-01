#include <core\Transform.hpp>

NAMESPACE_BEGIN

Transform::Transform() :
	m_Transform(Eigen::Matrix4f::Identity()),
	m_Inverse(Eigen::Matrix4f::Identity()) { }

Transform::Transform(const Eigen::Matrix4f & Trans) :
	m_Transform(Trans),
	m_Inverse(Trans.inverse()) { }

Transform::Transform(const Eigen::Matrix4f & Trans, const Eigen::Matrix4f & InvTrans) :
	m_Transform(Trans),
	m_Inverse(InvTrans) { }

const Eigen::Matrix4f & Transform::GetMatrix() const
{
	return m_Transform;
}

const Eigen::Matrix4f & Transform::GetInverseMatrix() const
{
	return m_Inverse;
}

Transform Transform::Inverse() const
{
	return Transform(m_Inverse, m_Transform);
}

Transform Transform::operator*(const Transform & Trans) const
{
	return Transform(
		m_Transform * Trans.m_Transform,
		Trans.m_Inverse * m_Inverse
	);
}

Vector3f Transform::operator*(const Vector3f & Vec) const
{
	return m_Transform.topLeftCorner<3, 3>() * Vec;
}

Normal3f Transform::operator*(const Normal3f & Norm) const
{
	return m_Inverse.topLeftCorner<3, 3>().transpose() * Norm;
}

Point3f Transform::operator*(const Point3f & Pt) const
{
	Vector4f Result = m_Transform * Vector4f(Pt[0], Pt[1], Pt[2], 1.0f);
	return Result.head<3>() / Result.w();
}

Ray3f Transform::operator*(const Ray3f & Ray) const
{
	return Ray3f(
		operator*(Ray.Origin),
		operator*(Ray.Direction),
		Ray.MinT, Ray.MaxT
	);
}

std::string Transform::ToString() const
{
	std::ostringstream OSS;
	OSS << m_Transform.format(Eigen::IOFormat(4, 0, ", ", ";\n", "", "", "[", "]"));
	return OSS.str();
}

NAMESPACE_END
