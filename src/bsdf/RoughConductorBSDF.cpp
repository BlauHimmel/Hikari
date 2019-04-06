#include <bsdf\RoughConductorBSDF.hpp>
#include <core\Frame.hpp>
#include <core\Texture.hpp>

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
	m_pKs = new ConstantColor3fTexture(PropList.GetColor(XML_BSDF_ROUGH_CONDUCTOR_KS, DEFAULT_BSDF_ROUGH_CONDUCTOR_KS));

	/* Distribution type */
	std::string TypeStr = PropList.GetString(XML_BSDF_ROUGH_CONDUCTOR_TYPE, DEFAULT_BSDF_ROUGH_CONDUCTOR_TYPE);
	if (TypeStr == XML_BSDF_BECKMANN) { m_Type = MicrofacetDistribution::EBeckmann; }
	else if (TypeStr == XML_BSDF_GGX) { m_Type = MicrofacetDistribution::EGGX; }
	else { throw HikariException("Unexpected distribution type : %s", TypeStr); }

	/* Whether anisotropic */
	m_bAnisotropic = PropList.GetBoolean(XML_BSDF_ROUGH_CONDUCTOR_AS, DEFAULT_BSDF_ROUGH_CONDUCTOR_AS);

	if (m_bAnisotropic)
	{
		m_pAlphaU = new ConstantColor3fTexture(Clamp(PropList.GetFloat(XML_BSDF_ROUGH_CONDUCTOR_ALPHA_U, DEFAULT_BSDF_ROUGH_CONDUCTOR_ALPHA_U), float(MIN_ALPHA), float(MAX_ALPHA)));
		m_pAlphaV = new ConstantColor3fTexture(Clamp(PropList.GetFloat(XML_BSDF_ROUGH_CONDUCTOR_ALPHA_V, DEFAULT_BSDF_ROUGH_CONDUCTOR_ALPHA_V), float(MIN_ALPHA), float(MAX_ALPHA)));
	}
	else
	{
		m_pAlphaU = new ConstantColor3fTexture(Clamp(PropList.GetFloat(XML_BSDF_ROUGH_CONDUCTOR_ALPHA, DEFAULT_BSDF_ROUGH_CONDUCTOR_ALPHA), float(MIN_ALPHA), float(MAX_ALPHA)));
		m_pAlphaV = m_pAlphaU;
	}

	m_Eta = Color3f(m_IntIOR / m_ExtIOR);
	m_EtaK = m_K / m_ExtIOR;
}

RoughConductorBSDF::~RoughConductorBSDF()
{
	delete m_pKs;
	delete m_pAlphaU;
	delete m_pAlphaV;
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
	float AlphaU = Clamp(m_pAlphaU->Eval(Record.Isect)[0], float(MIN_ALPHA), float(MAX_ALPHA));
	float AlphaV = AlphaU;
	if (m_bAnisotropic)
	{
		AlphaV = Clamp(m_pAlphaV->Eval(Record.Isect)[0], float(MIN_ALPHA), float(MAX_ALPHA));
	}
	MicrofacetDistribution Distribution(m_Type, AlphaU, AlphaV);

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

	return m_pKs->Eval(Record.Isect) * F * Distribution.Eval(M) * G * MDotWi / (Pdf * Frame::CosTheta(Record.Wi));
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
	float AlphaU = Clamp(m_pAlphaU->Eval(Record.Isect)[0], float(MIN_ALPHA), float(MAX_ALPHA));
	float AlphaV = AlphaU;
	if (m_bAnisotropic)
	{
		AlphaV = Clamp(m_pAlphaV->Eval(Record.Isect)[0], float(MIN_ALPHA), float(MAX_ALPHA));
	}
	MicrofacetDistribution Distribution(m_Type, AlphaU, AlphaV);

	float D = Distribution.Eval(H);

	if (D == 0.0f)
	{
		return Color3f(0.0f);
	}

	Color3f F = FresnelConductor(H.dot(Record.Wi), m_Eta, m_EtaK) * m_pKs->Eval(Record.Isect);

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
	float AlphaU = Clamp(m_pAlphaU->Eval(Record.Isect)[0], float(MIN_ALPHA), float(MAX_ALPHA));
	float AlphaV = AlphaU;
	if (m_bAnisotropic)
	{
		AlphaV = Clamp(m_pAlphaV->Eval(Record.Isect)[0], float(MIN_ALPHA), float(MAX_ALPHA));
	}
	MicrofacetDistribution Distribution(m_Type, AlphaU, AlphaV);

	return Distribution.Pdf(H) / (4.0f * std::abs(H.dot(Record.Wo)));
}

bool RoughConductorBSDF::IsAnisotropic() const
{
	return m_bAnisotropic;
}

void RoughConductorBSDF::AddChild(Object * pChildObj, const std::string & Name)
{
	if (pChildObj->GetClassType() == EClassType::ETexture && Name == XML_BSDF_ROUGH_CONDUCTOR_KS)
	{
		if (m_pKs != nullptr)
		{
			m_pKs = (Texture *)(pChildObj);
			if (m_pKs->IsMonochromatic())
			{
				LOG(WARNING) << "Ks texture is monochromatic! Make sure that it is done intentionally.";
			}
		}
		else
		{
			throw HikariException("RoughConductorBSDF: tried to specify multiple Ks texture");
		}
	}
	else if (pChildObj->GetClassType() == EClassType::ETexture && !m_bAnisotropic && Name == XML_BSDF_ROUGH_CONDUCTOR_ALPHA)
	{
		if (m_pAlphaU != nullptr && m_pAlphaV != nullptr)
		{
			m_pAlphaU = (Texture *)(pChildObj);
			m_pAlphaV = m_pAlphaU;
			if (!m_pAlphaU->IsMonochromatic())
			{
				LOG(WARNING) << "Alpha texture is not monochromatic, only R channel will be used.";
			}
		}
		else
		{
			throw HikariException("RoughConductorBSDF: tried to specify multiple Alpha texture");
		}
	}
	else if (pChildObj->GetClassType() == EClassType::ETexture && m_bAnisotropic && Name == XML_BSDF_ROUGH_CONDUCTOR_ALPHA_U)
	{
		if (m_pAlphaU != nullptr)
		{
			m_pAlphaU = (Texture *)(pChildObj);
			if (!m_pAlphaU->IsMonochromatic())
			{
				LOG(WARNING) << "AlphaU texture is not monochromatic, only R channel will be used.";
			}
		}
		else
		{
			throw HikariException("RoughConductorBSDF: tried to specify multiple AlphaU texture");
		}
	}
	else if (pChildObj->GetClassType() == EClassType::ETexture && m_bAnisotropic && Name == XML_BSDF_ROUGH_CONDUCTOR_ALPHA_V)
	{
		if (m_pAlphaV != nullptr)
		{
			m_pAlphaV = (Texture *)(pChildObj);
			if (!m_pAlphaV->IsMonochromatic())
			{
				LOG(WARNING) << "AlphaV texture is not monochromatic, only R channel will be used.";
			}
		}
		else
		{
			throw HikariException("RoughConductorBSDF: tried to specify multiple AlphaV texture");
		}
	}
	else
	{
		throw HikariException("RoughConductorBSDF::AddChild(<%s>, <%s>) is not supported!",
			ClassTypeName(pChildObj->GetClassType()), Name
		);
	}
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
		m_pKs->IsConstant() ? m_pKs->GetAverage().ToString() : Indent(m_pKs->ToString()),
		m_pAlphaU->IsConstant() ? std::to_string(m_pAlphaU->GetAverage()[0]) : Indent(m_pAlphaU->ToString()),
		m_pAlphaV->IsConstant() ? std::to_string(m_pAlphaV->GetAverage()[0]) : Indent(m_pAlphaV->ToString())
	);
}

NAMESPACE_END