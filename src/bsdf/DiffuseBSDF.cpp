#include <bsdf\DiffuseBSDF.hpp>
#include <core\Frame.hpp>
#include <core\Sampling.hpp>

NAMESPACE_BEGIN

REGISTER_CLASS(DiffuseBSDF, XML_BSDF_DIFFUSE);

DiffuseBSDF::DiffuseBSDF(const PropertyList & PropList)
{
	m_Albedo = PropList.GetColor(XML_BSDF_DIFFUSE_ALBEDO, DEFAULT_BSDF_DIFFUSE_ALBEDO);
}

Color3f DiffuseBSDF::Sample(BSDFQueryRecord & Record, const Point2f & Sample) const
{
	if (Frame::CosTheta(Record.Wi) <= 0)
	{
		return Color3f(0.0f);
	}

	Record.Measure = EMeasure::ESolidAngle;

	/* Warp a uniformly distributed sample on [0,1]^2
	to a direction on a cosine-weighted hemisphere */
	Record.Wo = Sampling::SquareToCosineHemisphere(Sample);

	/* Relative index of refraction: no change */
	Record.Eta = 1.0f;

	/* Eval() / Pdf() * Cos(Theta) = Albedo. There
	is no need to call these functions. */
	return m_Albedo;
}

Color3f DiffuseBSDF::Eval(const BSDFQueryRecord & Record) const
{
	/* This is a smooth BRDF -- return zero if the measure
	is wrong, or when queried for illumination on the backside */
	if (Record.Measure != EMeasure::ESolidAngle || Frame::CosTheta(Record.Wi) <= 0 || Frame::CosTheta(Record.Wo) <= 0)
	{
		return Color3f(0.0f);
	}

	/* The BRDF is simply the albedo / pi */
	return m_Albedo * INV_PI;
}

float DiffuseBSDF::Pdf(const BSDFQueryRecord & Record) const
{
	/* This is a smooth BRDF -- return zero if the measure
	is wrong, or when queried for illumination on the backside */
	if (Record.Measure != EMeasure::ESolidAngle || Frame::CosTheta(Record.Wi) <= 0 || Frame::CosTheta(Record.Wo) <= 0)
	{
		return 0.0f;
	}

	/* Importance sampling density wrt. solid angles:
	Cos(Theta) / pi.

	Note that the directions in 'Record' are in local coordinates,
	so Frame::CosTheta() actually just returns the 'z' component.
	*/
	return float(INV_PI) * Frame::CosTheta(Record.Wo);
}

bool DiffuseBSDF::IsDiffuse() const
{
	return true;
}

std::string DiffuseBSDF::ToString() const
{
	return tfm::format("Diffuse[ albedo = %s\n]", m_Albedo.ToString());
}

NAMESPACE_END
