#include <bsdf\MicrofacetBSDF.hpp>
#include <core\Frame.hpp>
#include <core\Sampling.hpp>

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

	/*
	To ensure energy conservation, we must scale the
	specular component by 1-kd.
	
	While that is not a particularly realistic model of what
	happens in reality, this will greatly simplify the
	implementation.

	TODO : Use a more realistic model in the future
	*/
	m_Ks = 1.0F - m_Kd.maxCoeff();
}

Color3f MicrofacetBSDF::Sample(BSDFQueryRecord & Record, const Point2f & Sample) const
{
	if (Frame::CosTheta(Record.Wi) <= 0.0f)
	{
		return Color3f(0.0f);
	}

	Record.Measure = EMeasure::ESolidAngle;

	float PDF = 0.0f;
	if (Sample.x() < m_Ks)
	{
		// Specular reflection
		float ReuseSampleX = Sample.x() / m_Ks;
		Normal3f Wh = Sampling::SquareToBeckmann(Point2f(ReuseSampleX, Sample.y()), m_Alpha);
		Record.Wo = Vector3f(-Record.Wi.x(), -Record.Wi.y(), Record.Wi.z());
		float J = 0.25f / Wh.dot(Record.Wo);
		PDF = m_Ks * Sampling::SquareToBeckmannPdf(Wh, m_Alpha) * J;
	}
	else
	{
		// Diffuse
		float ReuseSampleX = (Sample.x() - m_Ks) / (1.0f - m_Ks);
		Record.Wo = Sampling::SquareToCosineHemisphere(Point2f(ReuseSampleX, Sample.y()));
		PDF = (1.0f - m_Ks) * Sampling::SquareToCosineHemispherePdf(Record.Wo);
	}

	Record.Eta = 1.0f;

	if (PDF == 0.0f)
	{
		return Color3f(0.0f);
	}
	return Eval(Record) / PDF;
}

Color3f MicrofacetBSDF::Eval(const BSDFQueryRecord & Record) const
{
	if (Record.Measure != EMeasure::ESolidAngle ||
		Frame::CosTheta(Record.Wi) <= 0.0f ||
		Frame::CosTheta(Record.Wo) <= 0.0f
		)
	{
		return Color3f(0.0f);
	}

	Vector3f Wh = (Record.Wi + Record.Wo).normalized();

	float D = BeckmannD(Wh);
	float F = Fresnel(Wh.dot(Record.Wi), m_ExtIOR, m_IntIOR);
	float G = SmithBeckmannG1(Record.Wi, Wh) * SmithBeckmannG1(Record.Wo, Wh);

	Color3f SpecularTerm = m_Ks * F * D * G / (4.0f * Frame::CosTheta(Record.Wi) * Frame::CosTheta(Record.Wo));
	if (!SpecularTerm.IsValid())
	{
		SpecularTerm = Color3f(0.0f);
	}

	Color3f DiffuseTerm = m_Kd / float(M_PI);
	return SpecularTerm + DiffuseTerm;
}

float MicrofacetBSDF::Pdf(const BSDFQueryRecord & Record) const
{
	if (Record.Measure != EMeasure::ESolidAngle ||
		Frame::CosTheta(Record.Wi) <= 0.0f ||
		Frame::CosTheta(Record.Wo) <= 0.0f
		)
	{
		return 0.0f;
	}

	Vector3f Wh = (Record.Wi + Record.Wo).normalized();
	float J = 0.25f / Wh.dot(Record.Wo);
	return m_Ks * Sampling::SquareToBeckmannPdf(Wh, m_Alpha) * J +
		(1.0f - m_Ks) * Sampling::SquareToCosineHemispherePdf(Record.Wo);
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

float MicrofacetBSDF::BeckmannD(const Normal3f & M) const
{
	float Expon = Frame::TanTheta(M) / m_Alpha;
	float CosTheta = Frame::CosTheta(M);
	float CosTheta2 = CosTheta * CosTheta;
	return std::exp(-Expon * Expon) / (float(M_PI) * m_Alpha * m_Alpha * CosTheta2 * CosTheta2);
}

float MicrofacetBSDF::SmithBeckmannG1(const Vector3f & V, const Normal3f & M) const
{
	float TanTheta = Frame::TanTheta(V);

	// Perpendicular indidence
	if (TanTheta == 0.0f)
	{
		return 1.0f;
	}

	// Backside
	if (M.dot(V) * Frame::CosTheta(V) <= 0.0f)
	{
		return 0.0f;
	}

	float B = 1.0f / (m_Alpha * TanTheta);
	if (B >= 1.6f)
	{
		return 1.0f;
	}

	float B2 = B * B;
	return (3.535f * B + 2.181f * B2) / (1.0f + 2.276f * B + 2.577f * B2);
}

NAMESPACE_END
