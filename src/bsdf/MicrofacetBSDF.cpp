#include <bsdf\MicrofacetBSDF.hpp>

NAMESPACE_BEGIN

REGISTER_CLASS(MicrofacetBSDF, XML_BSDF_MICROFACET);

MicrofacetBSDF::MicrofacetBSDF(const PropertyList & PropList)
{
	/* RMS surface roughness */
	m_Alpha = PropList.GetFloat(XML_BSDF_MICROFACET_ALPHA, DEFAULT_BSDF_MICROFACET_ALPHA);

	/* Interior IOR */
	m_IntIOR = PropList.GetFloat(XML_BSDF_MICROFACET_INT_IOR, DEFAULT_BSDF_MICROFACET_INT_IOR);

	/* Exterior IOR */
	m_ExtIOR = PropList.GetFloat(XML_BSDF_MICROFACET_EXT_IOR, DEFAULT_BSDF_MICROFACET_EXT_IOR);

	/* Albedo of the diffuse base material (a.k.a "kd") */
	m_Kd = PropList.GetColor(XML_BSDF_MICROFACET_KD, DEFAULT_BSDF_MICROFACET_ALBEDO);

	/* To ensure energy conservation, we must scale the
	specular component by 1-kd.
	
	While that is not a particularly realistic model of what
	happens in reality, this will greatly simplify the
	implementation.*/
	m_Ks = 1.0F - m_Kd.maxCoeff();
}

Color3f MicrofacetBSDF::Sample(BSDFQueryRecord & Record, const Point2f & Sample) const
{
	throw HikariException("MicrofacetBRDF::Sample(BSDFQueryRecord & Record, const Point2f & Sample) const: not implemented!");

	// Note: Once you have implemented the part that computes the scattered
	// direction, the last part of this function should simply return the
	// BRDF value divided by the solid angle density and multiplied by the
	// cosine factor from the reflection equation, i.e.
	// return eval(bRec) * Frame::cosTheta(bRec.wo) / pdf(bRec);
}

Color3f MicrofacetBSDF::Eval(const BSDFQueryRecord & Record) const
{
	throw HikariException("MicrofacetBRDF::Eval(const BSDFQueryRecord & Record) const: not implemented!");
}

float MicrofacetBSDF::Pdf(const BSDFQueryRecord & Record) const
{
	throw HikariException("MicrofacetBRDF::Pdf(const BSDFQueryRecord & Record) const: not implemented!");
}

bool MicrofacetBSDF::IsDiffuse() const
{
	/* While microfacet BRDFs are not perfectly diffuse, they can be
	handled by sampling techniques for diffuse/non-specular materials,
	hence we return true here */
	return true;
}

std::string MicrofacetBSDF::ToString() const
{
	return tfm::format(
		"Microfacet[\n"
		"  alpha = %f,\n"
		"  intIOR = %f,\n"
		"  extIOR = %f,\n"
		"  kd = %s,\n"
		"  ks = %f\n"
		"]",
		m_Alpha,
		m_IntIOR,
		m_ExtIOR,
		m_Kd.ToString(),
		m_Ks
	);
}

float MicrofacetBSDF::ChiPlus(float C) const
{
	throw HikariException("MicrofacetBRDF::ChiPlus(float C) const: not implemented!");
}

float MicrofacetBSDF::Wh(const Vector3f & Wi, const Vector3f & Wo) const
{
	throw HikariException("MicrofacetBRDF::Wh(const Vector3f & Wi, const Vector3f & Wo) const: not implemented!");
}

float MicrofacetBSDF::B(float ThetaV) const
{
	throw HikariException("MicrofacetBRDF::B(float ThetaV) const: not implemented!");
}

float MicrofacetBSDF::G1(const Vector3f & Wv, const Vector3f & Wh) const
{
	throw HikariException("MicrofacetBRDF::G1(const Vector3f & Wv, const Vector3f & Wh) const: not implemented!");
}

float MicrofacetBSDF::G(const Vector3f & Wi, const Vector3f & Wo, const Vector3f & Wh) const
{
	throw HikariException("MicrofacetBRDF::G(const Vector3f & Wi, const Vector3f & Wo, const Vector3f & Wh) const: not implemented!");
}

float MicrofacetBSDF::D(float Theta, float Phi) const
{
	throw HikariException("MicrofacetBRDF::D(float Theta, float Phi) const: not implemented!");
}

NAMESPACE_END
