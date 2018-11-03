#include <core\BSDF.hpp>

NAMESPACE_BEGIN

BSDFQueryRecord::BSDFQueryRecord(const Vector3f & Wi) : Wi(Wi), Measure(EMeasure::EUnknownMeasure) { }

BSDFQueryRecord::BSDFQueryRecord(const Vector3f & Wi, const Vector3f & Wo, EMeasure Measure) : Wi(Wi), Wo(Wo), Measure(Measure) { }

Object::EClassType BSDF::GetClassType() const
{
	return EClassType::EBSDF;
}

bool BSDF::IsDiffuse() const
{
	return false;
}

NAMESPACE_END