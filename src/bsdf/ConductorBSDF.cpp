#include <bsdf\ConductorBSDF.hpp>
#include <core\Frame.hpp>

NAMESPACE_BEGIN

REGISTER_CLASS(ConductorBSDF, XML_BSDF_CONDUCTOR);

ConductorBSDF::ConductorBSDF(const PropertyList & PropList)
{
	/* Interior IOR */
	m_IntIOR = PropList.GetFloat(XML_BSDF_CONDUCTOR_INT_IOR, DEFAULT_BSDF_CONDUCTOR_INT_IOR);

	/* Exterior IOR */
	m_ExtIOR = PropList.GetFloat(XML_BSDF_CONDUCTOR_EXT_IOR, DEFAULT_BSDF_CONDUCTOR_EXT_IOR);

	/* Extinction coefficient */
	m_K = PropList.GetColor(XML_BSDF_CONDUCTOR_K, DEFAULT_BSDF_CONDUCTOR_K);

	/* Speculer reflectance  */
	m_Ks = PropList.GetColor(XML_BSDF_CONDUCTOR_KS, DEFAULT_BSDF_CONDUCTOR_KS);

	m_Eta = Color3f(m_IntIOR / m_ExtIOR);
	m_EtaK = m_K / m_ExtIOR;
}

Color3f ConductorBSDF::Sample(BSDFQueryRecord & Record, const Point2f & Sample) const
{
	Record.Measure = EMeasure::EDiscrete;

	float CosThetaI = Frame::CosTheta(Record.Wi);

	if (CosThetaI < 0.0f)
	{
		return Color3f(0.0f);
	}

	Color3f FresnelTerm = FresnelConductor(CosThetaI, m_Eta, m_EtaK);

	Record.Wo = Reflect(Record.Wi);
	Record.Eta = 1.0f;

	return m_Ks * Color3f(FresnelTerm);
}

Color3f ConductorBSDF::Eval(const BSDFQueryRecord & Record) const
{
	/* Discrete BRDFs always evaluate to zero */
	return Color3f(0.0f);
}

float ConductorBSDF::Pdf(const BSDFQueryRecord & Record) const
{
	/* Discrete BRDFs always evaluate to zero */
	return 0.0f;
}

std::string ConductorBSDF::ToString() const
{
	return tfm::format("Conductor[intIOR = %f, extIOR = %f, k = %f, ks = %s]", m_IntIOR, m_ExtIOR, m_K, m_Ks.ToString());
}

NAMESPACE_END