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
	/* Discrete BRDFs always evaluate to zero */
	return Color3f(0.0f);
}

float MirrorBSDF::Pdf(const BSDFQueryRecord & Record) const
{
	/* Discrete BRDFs always evaluate to zero */
	return 0.0f;
}

std::string MirrorBSDF::ToString() const
{
	return "Mirror[]";
}

NAMESPACE_END


