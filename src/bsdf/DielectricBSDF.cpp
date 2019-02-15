#include <bsdf\DielectricBSDF.hpp>
#include <core\Frame.hpp>

NAMESPACE_BEGIN

REGISTER_CLASS(DielectricBSDF, XML_BSDF_DIELECTRIC);

DielectricBSDF::DielectricBSDF(const PropertyList & PropList)
{
	/* Interior IOR */
	m_IntIOR = PropList.GetFloat(XML_BSDF_DIELECTRIC_INT_IOR, DEFAULT_BSDF_DIELECTRIC_INT_IOR);

	/* Exterior IOR */
	m_ExtIOR = PropList.GetFloat(XML_BSDF_DIELECTRIC_EXT_IOR, DEFAULT_BSDF_DIELECTRIC_EXT_IOR);
}

Color3f DielectricBSDF::Sample(BSDFQueryRecord & Record, const Point2f & Sample) const
{
	Record.Measure = EMeasure::EDiscrete;

	float CosThetaI = Frame::CosTheta(Record.Wi);
	float FresnelTerm = FresnelDielectric(CosThetaI, m_ExtIOR, m_IntIOR);

	float ExtIOR = m_ExtIOR;
	float IntIOR = m_IntIOR;
	Normal3f N(0.0f, 0.0f, 1.0f);

	if (CosThetaI < 0.0f)
	{
		std::swap(ExtIOR, IntIOR);
		N *= -1.0f;
		CosThetaI = -CosThetaI;
	}

	// Reflection
	if (Sample.x() < FresnelTerm)
	{
		Record.Wo = Vector3f(-Record.Wi.x(), -Record.Wi.y(), Record.Wi.z());
		Record.Eta = 1.0f;
	}
	// Refraction
	else
	{
		float Eta = ExtIOR / IntIOR;
		float SinThetaT2 = Eta * Eta * (1.0f - CosThetaI * CosThetaI);
		Record.Wo = Eta * -1.0f * Record.Wi + N * (Eta * CosThetaI - std::sqrt(1.0f - SinThetaT2));
		Record.Eta = 1.0f / Eta;
	}

	return Color3f(1.0f);
}

Color3f DielectricBSDF::Eval(const BSDFQueryRecord & Record) const
{
	/* Discrete BRDFs always evaluate to zero */
	return Color3f(0.0f);
}

float DielectricBSDF::Pdf(const BSDFQueryRecord & Record) const
{
	/* Discrete BRDFs always evaluate to zero */
	return 0.0f;
}

std::string DielectricBSDF::ToString() const
{
	return tfm::format("Dielectric[intIOR = %f, extIOR = %f]", m_IntIOR, m_ExtIOR);
}

NAMESPACE_END
