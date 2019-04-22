#include <bsdf\MirrorBSDF.hpp>
#include <core\Frame.hpp>

NAMESPACE_BEGIN

REGISTER_CLASS(MirrorBSDF, XML_BSDF_MIRROR);

MirrorBSDF::MirrorBSDF(const PropertyList & PropList)
{
}

Color3f MirrorBSDF::Sample(BSDFQueryRecord & Record, const Point2f & Sample) const
{
	if (Frame::CosTheta(Record.Wi) <= 0)
	{
		return Color3f(0.0f);
	}

	// Reflection in local coordinates
	Record.Wo = Vector3f(-Record.Wi.x(), -Record.Wi.y(), Record.Wi.z());
	Record.Measure = EMeasure::EDiscrete;

	/* Relative index of refraction: no change */
	Record.Eta = 1.0f;
	
	return Color3f(1.0f);
}

Color3f MirrorBSDF::Eval(const BSDFQueryRecord & Record) const
{
	float CosThetaI = Frame::CosTheta(Record.Wi);
	float CosThetaO = Frame::CosTheta(Record.Wo);

	if (CosThetaI > 0.0f && CosThetaO > 0.0f && Record.Measure == EMeasure::EDiscrete &&
		std::abs(Reflect(Record.Wi).dot(Record.Wo) - 1.0f) <= DeltaEpsilon)
	{
		return Color3f(1.0f);
	}

	return Color3f(0.0f);
}

float MirrorBSDF::Pdf(const BSDFQueryRecord & Record) const
{
	float CosThetaI = Frame::CosTheta(Record.Wi);
	float CosThetaO = Frame::CosTheta(Record.Wo);

	if (CosThetaI > 0.0f && CosThetaO > 0.0f && Record.Measure == EMeasure::EDiscrete &&
		std::abs(Reflect(Record.Wi).dot(Record.Wo) - 1.0f) <= DeltaEpsilon)
	{
		return 1.0f;
	}

	return 0.0f;
}

void MirrorBSDF::Activate()
{
	AddBSDFType(EBSDFType::EDeltaReflection);
}

std::string MirrorBSDF::ToString() const
{
	return "Mirror[]";
}

NAMESPACE_END


