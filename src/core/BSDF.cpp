#include <core\BSDF.hpp>

NAMESPACE_BEGIN

BSDFQueryRecord::BSDFQueryRecord(
	const Vector3f & Wi,
	ETransportMode Mode,
	Sampler * pSampler,
	const Intersection & Isect
) :
	Wi(Wi),
	Measure(EMeasure::EUnknownMeasure),
	Mode(Mode),
	pSampler(pSampler),
	Isect(Isect)
{ }

BSDFQueryRecord::BSDFQueryRecord(
	const Vector3f & Wi,
	const Vector3f & Wo,
	EMeasure Measure,
	ETransportMode Mode,
	Sampler * pSampler,
	const Intersection & Isect
) :
	Wi(Wi),
	Wo(Wo),
	Measure(Measure),
	Mode(Mode),
	pSampler(pSampler),
	Isect(Isect)
{ }

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

bool BSDF::IsAnisotropic() const
{
	return false;
}

void BSDF::AddBSDFType(uint32_t Type)
{
	m_CombinedType |= Type;
}

bool BSDF::HasBSDFType(EBSDFType Type) const
{
	return (uint32_t(Type) & m_CombinedType) != 0;
}

uint32_t BSDF::GetBSDFTypes() const
{
	return m_CombinedType;
}

NAMESPACE_END