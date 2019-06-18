#include <bsdf\CoatingBSDF.hpp>
#include <core\Frame.hpp>
#include <core\Texture.hpp>

NAMESPACE_BEGIN

REGISTER_CLASS(CoatingBSDF, XML_BSDF_COATING);

CoatingBSDF::CoatingBSDF(const PropertyList & PropList)
{
	/* Interior IOR */
	m_IntIOR = PropList.GetFloat(XML_BSDF_COATING_INT_IOR, DEFAULT_BSDF_COATING_INT_IOR);

	/* Exterior IOR */
	m_ExtIOR = PropList.GetFloat(XML_BSDF_COATING_EXT_IOR, DEFAULT_BSDF_COATING_EXT_IOR);

	/* Layer's thickness using the inverse units of sigmaA */
	m_Thickness = PropList.GetFloat(XML_BSDF_COATING_THICKNESS, DEFAULT_BSDF_COATING_THICKNESS);

	/* Absorption within the layer */
	m_pSigmaA = new ConstantColor3fTexture(PropList.GetColor(XML_BSDF_COATING_SIGMA_A, DEFAULT_BSDF_COATING_SIGMA_A));

	/* Specular reflectance */
	m_pKs = new ConstantColor3fTexture(PropList.GetColor(XML_BSDF_COATING_KS, DEFAULT_BSDF_COATING_KS));

	m_pNestedBSDF = nullptr;

	m_Eta = m_IntIOR / m_ExtIOR;
	m_InvEta = 1.0f / m_Eta;
}

CoatingBSDF::~CoatingBSDF()
{
	delete m_pSigmaA;
	delete m_pKs;
	delete m_pNestedBSDF;
}

Color3f CoatingBSDF::Sample(BSDFQueryRecord & Record, const Point2f & Sample) const
{
	float R12;
	Vector3f WiPrime = RefractIn(Record.Wi, R12);

	float SpecularPDF = (R12 * m_SpecularSamplingWeight) /
		(R12 * m_SpecularSamplingWeight + (1.0f - R12) * (1.0f - m_SpecularSamplingWeight));

	bool bSpecular;

	Point2f Sample2D(Sample);

	if (Sample2D.x() < SpecularPDF)
	{
		bSpecular = true;
		Sample2D.x() /= SpecularPDF;
	}
	else
	{
		bSpecular = false;
		Sample2D.x() = (Sample2D.x() - SpecularPDF) / (1.0f - SpecularPDF);
	}

	if (bSpecular)
	{
		Record.Wo = Reflect(Record.Wi);
		Record.Eta = 1.0f;
		Record.Measure = EMeasure::EDiscrete;
		return m_pKs->Eval(Record.Isect) * (R12 / SpecularPDF);
	}
	else
	{
		if (R12 == 1.0f)
		{
			return Color3f(0.0f);
		}

		Vector3f WiBackup = Record.Wi;
		Record.Wi = WiPrime;
		Color3f Result = m_pNestedBSDF->Sample(Record, Sample2D);
		Record.Wi = WiBackup;

		if (Result.isZero())
		{
			return Color3f(0.0f);
		}

		Vector3f WoPrime = Record.Wo;

		Color3f SigmaA = m_pSigmaA->Eval(Record.Isect) * m_Thickness;
		if (!SigmaA.isZero())
		{
			Result *= (-SigmaA * 
				(1.0f / std::abs(Frame::CosTheta(WiPrime)) + 1.0f / std::abs(Frame::CosTheta(WoPrime)))
			).exp();
		}

		float R21;
		Record.Wo = RefractOut(WoPrime, R21);
		if (R21 == 1.0f)
		{
			/* Total internal reflectance */
			return Color3f(0.0f);
		}

		Result /= (1.0f - SpecularPDF);
		Result *= (1.0f - R12) * (1.0f - R21);

		return Result;
	}
}

Color3f CoatingBSDF::Eval(const BSDFQueryRecord & Record) const
{
	if (Record.Measure == EMeasure::EDiscrete)
	{
		if (std::abs(Reflect(Record.Wi).dot(Record.Wo) - 1.0f) <= DeltaEpsilon)
		{
			float CosThetaT;
			return m_pKs->Eval(Record.Isect) * FresnelDielectric(std::abs(Frame::CosTheta(Record.Wi)), m_Eta, m_InvEta, CosThetaT);
		}
		return Color3f(0.0f);
	}
	else
	{
		float R12, R21;
		BSDFQueryRecord RecordInt(Record);
		RecordInt.Wi = RefractIn(Record.Wi, R12);
		RecordInt.Wo = RefractIn(Record.Wo, R21);

		if (R12 == 1.0f || R21 == 1.0f)
		{
			/* Total internal reflection */
			return Color3f(0.0f);
		}

		Color3f Result = m_pNestedBSDF->Eval(RecordInt) * (1.0f - R12) * (1.0f - R21);

		Color3f SigmaA = m_pSigmaA->Eval(Record.Isect) * m_Thickness;
		if (!SigmaA.isZero())
		{
			Result *= (-SigmaA *
				(1.0f / std::abs(Frame::CosTheta(RecordInt.Wi)) + 1.0f / std::abs(Frame::CosTheta(RecordInt.Wo)))
			).exp();
		}

		if (Record.Measure == EMeasure::ESolidAngle)
		{
			Result *= (m_InvEta * m_InvEta * Frame::CosTheta(Record.Wo) / Frame::CosTheta(RecordInt.Wo));
		}

		return Result;
	}
}

float CoatingBSDF::Pdf(const BSDFQueryRecord & Record) const
{
	float R12;
	Vector3f WiPrime = RefractIn(Record.Wi, R12);

	float SpecularPDF = (R12 * m_SpecularSamplingWeight) /
		(R12 * m_SpecularSamplingWeight + (1.0f - R12) * (1.0f - m_SpecularSamplingWeight));

	if (Record.Measure == EMeasure::EDiscrete)
	{
		if (std::abs(Reflect(Record.Wi).dot(Record.Wo) - 1.0f) <= DeltaEpsilon)
		{
			return SpecularPDF;
		}
		return 0.0f;
	}
	else
	{
		float R21;
		BSDFQueryRecord RecordInt(Record);
		RecordInt.Wi = WiPrime;
		RecordInt.Wo = RefractIn(Record.Wo, R21);

		if (R12 == 1.0f || R21 == 1.0f)
		{
			/* Total internal reflection */
			return 0.0f;
		}

		float Pdf = m_pNestedBSDF->Pdf(RecordInt);

		if (Record.Measure == EMeasure::ESolidAngle)
		{
			Pdf *= (m_InvEta * m_InvEta * Frame::CosTheta(Record.Wo) / Frame::CosTheta(RecordInt.Wo));
		}

		return Pdf * (1.0f - SpecularPDF);
	}
}

bool CoatingBSDF::IsDiffuse() const
{
	return m_pNestedBSDF->IsDiffuse();
}

bool CoatingBSDF::IsAnisotropic() const
{
	return m_pNestedBSDF->IsAnisotropic();
}

void CoatingBSDF::AddChild(Object * pChildObj, const std::string & Name)
{
	if (pChildObj->GetClassType() == EClassType::ETexture && Name == XML_BSDF_COATING_KS)
	{
		if (m_pKs->IsConstant())
		{
			delete m_pKs;
			m_pKs = (Texture *)(pChildObj);
			if (m_pKs->IsMonochromatic())
			{
				LOG(WARNING) << "Ks texture is monochromatic! Make sure that it is done intentionally.";
			}
		}
		else
		{
			throw HikariException("CoatingBSDF: tried to specify multiple Ks texture");
		}
	}
	else if (pChildObj->GetClassType() == EClassType::ETexture && Name == XML_BSDF_COATING_SIGMA_A)
	{
		if (m_pSigmaA->IsConstant())
		{
			delete m_pSigmaA;
			m_pSigmaA = (Texture *)(pChildObj);
			if (m_pSigmaA->IsMonochromatic())
			{
				LOG(WARNING) << "SigmaA texture is monochromatic! Make sure that it is done intentionally.";
			}
		}
		else
		{
			throw HikariException("CoatingBSDF: tried to specify multiple SigmaA texture");
		}
	}
	else if (pChildObj->GetClassType() == EClassType::EBSDF)
	{
		if (m_pNestedBSDF == nullptr)
		{
			m_pNestedBSDF = (BSDF *)(pChildObj);
		}
		else
		{
			throw HikariException("CoatingBSDF: tried to specify multiple nested BSDF");
		}
	}
}

void CoatingBSDF::Activate()
{
	Color3f Temp = (m_pSigmaA->GetAverage() * (-2.0f * m_Thickness)).exp();
	float AvgAbsorption = (Temp[0] + Temp[1] + Temp[2]) / 3.0f;
	m_SpecularSamplingWeight = 1.0f / (AvgAbsorption + 1.0f);

	if (m_pNestedBSDF == nullptr)
	{
		throw HikariException("CoatingBSDF needs a nested BSDF!");
	}

	AddBSDFType(m_pNestedBSDF->GetBSDFTypes());
	AddBSDFType(EBSDFType::EDeltaReflection);
	if (!m_pSigmaA->IsConstant() || !m_pKs->IsConstant())
	{
		AddBSDFType(EBSDFType::EUVDependent);
	}
}

std::string CoatingBSDF::ToString() const
{
	return tfm::format(
		"CoatingBSDF[\n"
		"  intIOR = %f,\n"
		"  extIOR = %f,\n"
		"  thickness = %f,\n"
		"  sigmaA = %s,\n"
		"  ks = %s,\n"
		"  nestedBSDF = %s\n"
		"]",
		m_IntIOR,
		m_ExtIOR,
		m_Thickness,
		m_pSigmaA->IsConstant() ? m_pSigmaA->GetAverage().ToString() : Indent(m_pSigmaA->ToString()),
		m_pKs->IsConstant() ? m_pKs->GetAverage().ToString() : Indent(m_pKs->ToString()),
		Indent(m_pNestedBSDF->ToString())
	);
}

Vector3f CoatingBSDF::RefractIn(const Vector3f & Wi, float & Reflectance) const
{
	float CosThetaT;
	Reflectance = FresnelDielectric(std::abs(Frame::CosTheta(Wi)), m_Eta, m_InvEta, CosThetaT);
	return Vector3f(m_InvEta * Wi.x(), m_InvEta * Wi.y(), -Signum(Frame::CosTheta(Wi)) * CosThetaT);
}

Vector3f CoatingBSDF::RefractOut(const Vector3f & Wi, float & Reflectance) const
{
	float CosThetaT;
	Reflectance = FresnelDielectric(std::abs(Frame::CosTheta(Wi)), m_InvEta, m_Eta, CosThetaT);
	return Vector3f(m_Eta * Wi.x(), m_Eta * Wi.y(), -Signum(Frame::CosTheta(Wi)) * CosThetaT);
}

NAMESPACE_END