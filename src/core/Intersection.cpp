#include <core\Intersection.hpp>
#include <core\Shape.hpp>
#include <core\Ray.hpp>

NAMESPACE_BEGIN

Intersection::Intersection() { }

Vector3f Intersection::ToLocal(const Vector3f & Dir) const
{
	return ShadingFrame.ToLocal(Dir);
}

Vector3f Intersection::ToWorld(const Vector3f & Dir) const
{
	return ShadingFrame.ToWorld(Dir);
}

Ray3f Intersection::SpawnShadowRay(const Point3f & Pt) const
{
	Ray3f ShadowRay;
	ShadowRay.Origin = P;
	ShadowRay.Direction = Pt - ShadowRay.Origin;
	ShadowRay.MaxT = 1.0f - float(Epsilon);
	ShadowRay.MinT = float(Epsilon);
	ShadowRay.Update();
	return ShadowRay;
}

std::string Intersection::ToString() const
{
	if (pShape == nullptr)
	{
		return "Intersection[invalid]";
	}

	return tfm::format(
		"Intersection[\n"
		"  p = %s,\n"
		"  t = %f,\n"
		"  uv = %s,\n"
		"  shadingFrame = %s,\n"
		"  geometricFrame = %s,\n"
		"  shape = %s\n"
		"]",
		P.ToString(),
		T,
		UV.ToString(),
		Indent(ShadingFrame.ToString()),
		Indent(GeometricFrame.ToString()),
		pShape->ToString()
	);
}

NAMESPACE_END