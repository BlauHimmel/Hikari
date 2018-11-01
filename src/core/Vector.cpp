#include <core/Vector.hpp>

NAMESPACE_BEGIN

void CoordinateSystem(const Vector3f & Va, Vector3f & Vb, Vector3f & Vc)
{
	if (std::abs(Va.x()) > std::abs(Va.y()))
	{
		float InvLen = 1.0f / std::sqrt(Va.x() * Va.x() + Va.z() * Va.z());
		Vc = Vector3f(Va.z() * InvLen, 0.0f, -Va.x() * InvLen);
	}
	else
	{
		float InvLen = 1.0f / std::sqrt(Va.y() * Va.y() + Va.z() * Va.z());
		Vc = Vector3f(0.0f, Va.z() * InvLen, -Va.y() * InvLen);
	}
	Vb = Vc.cross(Va);
}

NAMESPACE_END


