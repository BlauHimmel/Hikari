#include <bsdf\RoughConductorBSDF.hpp>
#include <core\Frame.hpp>

NAMESPACE_BEGIN

REGISTER_CLASS(RoughConductorBSDF, XML_BSDF_ROUGH_CONDUCTOR);

RoughConductorBSDF::RoughConductorBSDF(const PropertyList & PropList)
{
	/* Interior IOR */
	m_IntIOR = PropList.GetFloat(XML_BSDF_ROUGH_CONDUCTOR_INT_IOR, DEFAULT_BSDF_ROUGH_CONDUCTOR_INT_IOR);

	/* Exterior IOR */
	m_ExtIOR = PropList.GetFloat(XML_BSDF_ROUGH_CONDUCTOR_EXT_IOR, DEFAULT_BSDF_ROUGH_CONDUCTOR_EXT_IOR);

	/* Extinction coefficient */
	m_K = PropList.GetColor(XML_BSDF_ROUGH_CONDUCTOR_K, DEFAULT_BSDF_ROUGH_CONDUCTOR_K);

	/* Speculer reflectance */
	m_Ks = PropList.GetColor(XML_BSDF_ROUGH_CONDUCTOR_KS, DEFAULT_BSDF_ROUGH_CONDUCTOR_KS);

	/* Distribution type */
	std::string TypeStr = PropList.GetString(XML_BSDF_ROUGH_CONDUCTOR_TYPE, DEFAULT_BSDF_ROUGH_CONDUCTOR_TYPE);
	if (TypeStr == "beckmann") { m_Type = MicrofacetDistribution::EBeckmann; }
	else if (TypeStr == "ggx") { m_Type = MicrofacetDistribution::EGGX; }
	else if (TypeStr == "phong") { m_Type = MicrofacetDistribution::EPhong; }
	else { throw HikariException("Unexpected distribution type : %s", TypeStr); }


	bool bAs = PropList.GetBoolean(XML_BSDF_ROUGH_CONDUCTOR_AS, DEFAULT_BSDF_ROUGH_CONDUCTOR_AS);

	if (m_Type == MicrofacetDistribution::EPhong && bAs)
	{
		LOG(ERROR) << "Anisotropic phong does not work correctly in some cases. Bugs need to be fixed.";
	}

	if (bAs)
	{
		m_AlphaU = PropList.GetFloat(XML_BSDF_ROUGH_CONDUCTOR_ALPHA_U, DEFAULT_BSDF_ROUGH_CONDUCTOR_ALPHA_U);
		m_AlphaV = PropList.GetFloat(XML_BSDF_ROUGH_CONDUCTOR_ALPHA_V, DEFAULT_BSDF_ROUGH_CONDUCTOR_ALPHA_V);
	}
	else
	{
		m_AlphaU = PropList.GetFloat(XML_BSDF_ROUGH_CONDUCTOR_ALPHA, DEFAULT_BSDF_ROUGH_CONDUCTOR_ALPHA);
		m_AlphaV = m_AlphaU;
	}

	m_Eta = Color3f(m_IntIOR / m_ExtIOR);
	m_EtaK = m_K / m_ExtIOR;
}

Color3f RoughConductorBSDF::Sample(BSDFQueryRecord & Record, const Point2f & Sample) const
{
	Record.Measure = EMeasure::ESolidAngle;
	
	float CosThetaI = Frame::CosTheta(Record.Wi);
	if (CosThetaI <= 0.0f)
	{
		return Color3f(0.0f);
	}

	/* Construct the microfacet distribution matching the
	  roughness values at the current surface position.
	  (texture will be implemented later) */
	MicrofacetDistribution Distribution(m_Type, m_AlphaU, m_AlphaV);

	float Pdf;
	Vector3f M = Distribution.Sample(Record.Wi, Sample, Pdf);

	if (Pdf == 0.0f)
	{
		return Color3f(0.0f);
	}

	Record.Wo = Reflect(Record.Wi, M);
	Record.Eta = 1.0f;

	if (Frame::CosTheta(Record.Wo) < 0.0f)
	{
		return Color3f(0.0f);
	}

	float MDotWi = M.dot(Record.Wi);
	Color3f F = FresnelConductor(MDotWi, m_Eta, m_EtaK);
	float G = Distribution.G(Record.Wi, Record.Wo, M);

	return m_Ks * F * Distribution.Eval(M) * G * MDotWi / (Pdf * Frame::CosTheta(Record.Wi));
}

Color3f RoughConductorBSDF::Eval(const BSDFQueryRecord & Record) const
{
	float CosThetaI = Frame::CosTheta(Record.Wi);
	float CosThetaO = Frame::CosTheta(Record.Wo);
	if (Record.Measure != EMeasure::ESolidAngle ||
		CosThetaI <= 0.0f ||
		CosThetaO <= 0.0f)
	{
		return Color3f(0.0f);
	}

	/* Calculate the reflection half-vector */
	Vector3f H = (Record.Wi + Record.Wo).normalized();

	/* Construct the microfacet distribution matching the
	  roughness values at the current surface position. 
	  (texture will be implemented later) */
	MicrofacetDistribution Distribution(m_Type, m_AlphaU, m_AlphaV);

	float D = Distribution.Eval(H);

	if (D == 0.0f)
	{
		return Color3f(0.0f);
	}

	Color3f F = FresnelConductor(H.dot(Record.Wi), m_Eta, m_EtaK) * m_Ks;

	float G = Distribution.G(Record.Wi, Record.Wo, H);

	return D * G * F /(4.0f * CosThetaI);
}

float RoughConductorBSDF::Pdf(const BSDFQueryRecord & Record) const
{
	float CosThetaI = Frame::CosTheta(Record.Wi);
	float CosThetaO = Frame::CosTheta(Record.Wo);
	if (Record.Measure != EMeasure::ESolidAngle ||
		CosThetaI <= 0.0f ||
		CosThetaO <= 0.0f)
	{
		return 0.0f;
	}

	/* Calculate the reflection half-vector */
	Vector3f H = (Record.Wi + Record.Wo).normalized();

	/* Construct the microfacet distribution matching the
	  roughness values at the current surface position.
	  (texture will be implemented later) */
	MicrofacetDistribution Distribution(m_Type, m_AlphaU, m_AlphaV);

	return Distribution.Pdf(H) / (4.0f * std::abs(H.dot(Record.Wo)));
}

bool RoughConductorBSDF::IsAnisotropic() const
{
	return m_AlphaV != m_AlphaU;
}

std::string RoughConductorBSDF::ToString() const
{
	return tfm::format(
		"RoughConductor[\n"
		"  type = %s,\n"
		"  intIOR = %f,\n"
		"  extIOR = %f,\n"
		"  k = %s,\n"
		"  ks = %s,\n"
		"  alphaU = %f,\n"
		"  alphaV = %f\n"
		"]",
		MicrofacetDistribution::TypeName(m_Type),
		m_IntIOR,
		m_ExtIOR,
		m_K.ToString(),
		m_Ks.ToString(),
		m_AlphaU,
		m_AlphaV
	);
}

NAMESPACE_END