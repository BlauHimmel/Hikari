#include <core\BSDF.hpp>

NAMESPACE_BEGIN

BSDFQueryRecord::BSDFQueryRecord(const Vector3f & Wi) : Wi(Wi), Measure(EMeasure::EUnknownMeasure) { }

BSDFQueryRecord::BSDFQueryRecord(const Vector3f & Wi, const Vector3f & Wo, EMeasure Measure) : Wi(Wi), Wo(Wo), Measure(Measure) { }

std::string BSDFQueryRecord::ToString() const
{
	auto MeasureToString = [=](EMeasure Measure)
	{
		switch (Measure)
		{
		case EMeasure::ESolidAngle:
			return "solidAngle";
			break;
		case EMeasure::EDiscrete:
			return "discrete";
			break;
		default:
			return "<unknown>";
			break;
		}
	};

	return tfm::format(
		"BSDFQueryRecord[\n"
		"  wi = %s,\n"
		"  wo = %s,\n"
		"  eta = %f,\n"
		"  measure = %s\n"
		"]",
		Wi.ToString(),
		Wo.ToString(),
		Eta,
		MeasureToString(Measure)
	);
}

Object::EClassType BSDF::GetClassType() const
{
	return EClassType::EBSDF;
}

bool BSDF::IsDiffuse() const
{
	return false;
}

NAMESPACE_END