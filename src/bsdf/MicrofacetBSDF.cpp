#include <bsdf\MicrofacetBSDF.hpp>

NAMESPACE_BEGIN

REGISTER_CLASS(MicrofacetBSDF, XML_BSDF_MICROFACET);

MicrofacetBSDF::MicrofacetBSDF(const PropertyList & PropList)
{
	/* RMS surface roughness */
	m_Alpha = PropList.GetFloat(XML_BSDF_MICROFACET_ALPHA, 0.1f);

	/* Interior IOR (default: BK7 borosilicate optical glass) */
	m_IntIOR = PropList.GetFloat(XML_BSDF_MICROFACET_INT_IOR, 1.5046f);

	/* Exterior IOR (default: air) */
	m_ExtIOR = PropList.GetFloat(XML_BSDF_MICROFACET_EXT_IOR, 1.000277f);

	/* Albedo of the diffuse base material (a.k.a "kd") */
	m_Kd = PropList.GetColor(XML_BSDF_MICROFACET_KD, Color3f(0.5f));

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

NAMESPACE_END
