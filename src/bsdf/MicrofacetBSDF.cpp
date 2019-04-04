#include <bsdf\MicrofacetBSDF.hpp>
#include <core\Frame.hpp>
#include <core\Sampling.hpp>
#include <core\Texture.hpp>

NAMESPACE_BEGIN

REGISTER_CLASS(MicrofacetBSDF, XML_BSDF_MICROFACET);

MicrofacetBSDF::MicrofacetBSDF(const PropertyList & PropList)
{
	/* RMS surface roughness */
	m_pAlpha = new ConstantFloatTexture(std::max(PropList.GetFloat(XML_BSDF_MICROFACET_ALPHA, DEFAULT_BSDF_MICROFACET_ALPHA), float(MIN_ALPHA)));

	/* Interior IOR */
	m_IntIOR = PropList.GetFloat(XML_BSDF_MICROFACET_INT_IOR, DEFAULT_BSDF_MICROFACET_INT_IOR);

	/* Exterior IOR */
	m_ExtIOR = PropList.GetFloat(XML_BSDF_MICROFACET_EXT_IOR, DEFAULT_BSDF_MICROFACET_EXT_IOR);

	/* Albedo of the diffuse base material (a.k.a "kd") */
	m_pKd = new ConstantColor3fTexture(PropList.GetColor(XML_BSDF_MICROFACET_KD, DEFAULT_BSDF_MICROFACET_ALBEDO));
	
	m_Eta = m_IntIOR / m_ExtIOR;
	m_InvEta = 1.0f / m_Eta;
}

MicrofacetBSDF::~MicrofacetBSDF()
{
	delete m_pAlpha;
	delete m_pKd;
}

Color3f MicrofacetBSDF::Sample(BSDFQueryRecord & Record, const Point2f & Sample) const
{
	if (Frame::CosTheta(Record.Wi) <= 0.0f)
	{
		return Color3f(0.0f);
	}

	Record.Measure = EMeasure::ESolidAngle;

	float Alpha = Clamp(m_pAlpha->Eval(Record.Isect)[0], float(MIN_ALPHA), 1.0f);
	Color3f Kd = m_pKd->Eval(Record.Isect);

	/*
	To ensure energy conservation, we must scale the
	specular component by 1-kd.

	While that is not a particularly realistic model of what
	happens in reality, this will greatly simplify the
	implementation.
	*/
	float Ks = 1.0f - Kd.maxCoeff();

	if (Sample.x() < Ks)
	{
		// Specular reflection
		float ReuseSampleX = Sample.x() / Ks;
		Normal3f Wh = Sampling::SquareToBeckmann(Point2f(ReuseSampleX, Sample.y()), Alpha);
		Record.Wo = 2.0f * Wh.dot(Record.Wi) * Wh - Record.Wi;
	}
	else
	{
		// Diffuse
		float ReuseSampleX = (Sample.x() - Ks) / (1.0f - Ks);
		Record.Wo = Sampling::SquareToCosineHemisphere(Point2f(ReuseSampleX, Sample.y()));
	}

	Record.Eta = 1.0f;

	float PDF = Pdf(Record);
	if (PDF == 0.0f)
	{
		return Color3f(0.0f);
	}

	float CosThetaT;
	Vector3f Wh = (Record.Wi + Record.Wo).normalized();

	float D = BeckmannD(Wh, Alpha);
	float F = FresnelDielectric(Wh.dot(Record.Wi), m_Eta, m_InvEta, CosThetaT);
	float G = SmithBeckmannG1(Record.Wi, Wh, Alpha) * SmithBeckmannG1(Record.Wo, Wh, Alpha);

	Color3f SpecularTerm = Ks * F * D * G / (4.0f * Frame::CosTheta(Record.Wi) * Frame::CosTheta(Record.Wo));
	if (!SpecularTerm.IsValid())
	{
		SpecularTerm = Color3f(0.0f);
	}

	Color3f DiffuseTerm = Kd / float(M_PI);

	return (SpecularTerm + DiffuseTerm) * Frame::CosTheta(Record.Wo) / PDF;
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

	float Alpha = Clamp(m_pAlpha->Eval(Record.Isect)[0], float(MIN_ALPHA), 1.0f);
	Color3f Kd = m_pKd->Eval(Record.Isect);

	/*
	To ensure energy conservation, we must scale the
	specular component by 1-kd.

	While that is not a particularly realistic model of what
	happens in reality, this will greatly simplify the
	implementation.
	*/
	float Ks = 1.0f - Kd.maxCoeff();

	Vector3f Wh = (Record.Wi + Record.Wo).normalized();

	float CosThetaT;

	float D = BeckmannD(Wh, Alpha);
	float F = FresnelDielectric(Wh.dot(Record.Wi), m_Eta, m_InvEta, CosThetaT);
	float G = SmithBeckmannG1(Record.Wi, Wh, Alpha) * SmithBeckmannG1(Record.Wo, Wh, Alpha);

	Color3f SpecularTerm = Ks * F * D * G / (4.0f * Frame::CosTheta(Record.Wi) * Frame::CosTheta(Record.Wo));
	if (!SpecularTerm.IsValid())
	{
		SpecularTerm = Color3f(0.0f);
	}

	Color3f DiffuseTerm = Kd / float(M_PI);
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

	float Alpha = Clamp(m_pAlpha->Eval(Record.Isect)[0], float(MIN_ALPHA), 1.0f);
	Color3f Kd = m_pKd->Eval(Record.Isect);

	/*
	To ensure energy conservation, we must scale the
	specular component by 1-kd.

	While that is not a particularly realistic model of what
	happens in reality, this will greatly simplify the
	implementation.
	*/
	float Ks = 1.0f - Kd.maxCoeff();

	Vector3f Wh = (Record.Wi + Record.Wo).normalized();
	float J = 0.25f / Wh.dot(Record.Wo);
	float SpecularPDF = Ks * Sampling::SquareToBeckmannPdf(Wh, Alpha) * J;
	float DiffusePDF = (1.0f - Ks) * Sampling::SquareToCosineHemispherePdf(Record.Wo);
	return SpecularPDF + DiffusePDF;
}

bool MicrofacetBSDF::IsDiffuse() const
{
	/* While microfacet BRDFs are not perfectly diffuse, they can be
	handled by sampling techniques for diffuse/non-specular materials,
	hence we return true here */
	return true;
}

void MicrofacetBSDF::AddChild(Object * pChildObj, const std::string & Name)
{
	if (pChildObj->GetClassType() == EClassType::ETexture && Name == XML_BSDF_MICROFACET_KD)
	{
		if (m_pKd != nullptr)
		{
			m_pKd = (Texture *)(pChildObj);
			if (m_pKd->IsMonochromatic())
			{
				LOG(WARNING) << "Kd texture is monochromatic! Make sure that it is done intentionally.";
			}
		}
		else
		{
			throw HikariException("MicrofacetBSDF: tried to specify multiple kd texture");
		}
	}
	else if (pChildObj->GetClassType() == EClassType::ETexture && Name == XML_BSDF_MICROFACET_ALPHA)
	{
		if (m_pAlpha != nullptr)
		{
			m_pAlpha = (Texture *)(pChildObj);
			if (!m_pAlpha->IsMonochromatic())
			{
				LOG(WARNING) << "Alpha texture is not monochromatic, only R channel will be used.";
			}
		}
		else
		{
			throw HikariException("MicrofacetBSDF: tried to specify multiple alpha texture");
		}
	}
	else
	{
		throw HikariException("MicrofacetBSDF::AddChild(<%s>, <%s>) is not supported!",
			ClassTypeName(pChildObj->GetClassType()), Name
		);
	}
}

std::string MicrofacetBSDF::ToString() const
{
	return tfm::format(
		"Microfacet[\n"
		"  alpha = %s,\n"
		"  intIOR = %f,\n"
		"  extIOR = %f,\n"
		"  kd = %s\n"
		"]",
		m_pAlpha->IsConstant() ? std::to_string(m_pAlpha->GetAverage()[0]) : Indent(m_pAlpha->ToString()),
		m_IntIOR,
		m_ExtIOR,
		m_pKd->IsConstant() ? m_pKd->GetAverage().ToString() : Indent(m_pKd->ToString())
	);
}

float MicrofacetBSDF::BeckmannD(const Normal3f & M, float Alpha) const
{
	float Expon = Frame::TanTheta(M) / Alpha;
	float CosTheta = Frame::CosTheta(M);
	float CosTheta2 = CosTheta * CosTheta;
	return std::exp(-Expon * Expon) / (float(M_PI) * Alpha * Alpha * CosTheta2 * CosTheta2);
}

float MicrofacetBSDF::SmithBeckmannG1(const Vector3f & V, const Normal3f & M, float Alpha) const
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

	float B = 1.0f / (Alpha * TanTheta);
	if (B >= 1.6f)
	{
		return 1.0f;
	}

	float B2 = B * B;
	return (3.535f * B + 2.181f * B2) / (1.0f + 2.276f * B + 2.577f * B2);
}

NAMESPACE_END
