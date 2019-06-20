#include <bsdf\BumpMapBSDF.hpp>
#include <core\Frame.hpp>
#include <core\Texture.hpp>
#include <core\Intersection.hpp>

NAMESPACE_BEGIN

REGISTER_CLASS(BumpMapBSDF, XML_BSDF_BUMP_MAP);

BumpMapBSDF::BumpMapBSDF(const PropertyList & PropList)
{
	m_pBumpMap = nullptr;
	m_pNestedBSDF = nullptr;
}

BumpMapBSDF::~BumpMapBSDF()
{
	delete m_pBumpMap;
	delete m_pNestedBSDF;
}

Color3f BumpMapBSDF::Sample(BSDFQueryRecord & Record, const Point2f & Sample) const
{
	Intersection PerturbedIsect(Record.Isect);
	PerturbedIsect.ShadingFrame = GetPerturbebFrame(Record.Isect);

	BSDFQueryRecord PerturbedRecord(
		PerturbedIsect.ToLocal(Record.Isect.ToWorld(Record.Wi)),
		Record.Mode,
		Record.pSampler,
		PerturbedIsect
	);

	if (Frame::CosTheta(Record.Wi) * Frame::CosTheta(PerturbedRecord.Wi) <= 0.0f)
	{
		return Color3f(0.0f);
	}

	Color3f Result = m_pNestedBSDF->Sample(PerturbedRecord, Sample);

	if (!Result.isZero())
	{
		Record.Wo = Record.Isect.ToLocal(PerturbedIsect.ToWorld(PerturbedRecord.Wo));
		Record.Eta = PerturbedRecord.Eta;
		Record.Measure = PerturbedRecord.Measure;

		if (Frame::CosTheta(Record.Wo) * Frame::CosTheta(PerturbedRecord.Wo) <= 0.0f)
		{
			return Color3f(0.0f);
		}
	}

	return Result;
}

Color3f BumpMapBSDF::Eval(const BSDFQueryRecord & Record) const
{
	Intersection PerturbedIsect(Record.Isect);
	PerturbedIsect.ShadingFrame = GetPerturbebFrame(Record.Isect);

	BSDFQueryRecord PerturbedRecord(
		PerturbedIsect.ToLocal(Record.Isect.ToWorld(Record.Wi)),
		PerturbedIsect.ToLocal(Record.Isect.ToWorld(Record.Wo)),
		Record.Measure,
		Record.Mode,
		Record.pSampler,
		PerturbedIsect
	);

	if (Frame::CosTheta(Record.Wo) * Frame::CosTheta(PerturbedRecord.Wo) <= 0.0f ||
		Frame::CosTheta(Record.Wi) * Frame::CosTheta(PerturbedRecord.Wi) <= 0.0f)
	{
		return Color3f(0.0f);
	}

	return m_pNestedBSDF->Eval(PerturbedRecord);
}

float BumpMapBSDF::Pdf(const BSDFQueryRecord & Record) const
{
	Intersection PerturbedIsect(Record.Isect);
	PerturbedIsect.ShadingFrame = GetPerturbebFrame(Record.Isect);

	BSDFQueryRecord PerturbedRecord(
		PerturbedIsect.ToLocal(Record.Isect.ToWorld(Record.Wi)),
		PerturbedIsect.ToLocal(Record.Isect.ToWorld(Record.Wo)),
		Record.Measure,
		Record.Mode,
		Record.pSampler,
		PerturbedIsect
	);

	if (Frame::CosTheta(Record.Wo) * Frame::CosTheta(PerturbedRecord.Wo) <= 0.0f ||
		Frame::CosTheta(Record.Wi) * Frame::CosTheta(PerturbedRecord.Wi) <= 0.0f)
	{
		return 0.0f;
	}

	return m_pNestedBSDF->Pdf(PerturbedRecord);
}

bool BumpMapBSDF::IsDiffuse() const
{
	return m_pNestedBSDF->IsDiffuse();
}

bool BumpMapBSDF::IsAnisotropic() const
{
	return m_pNestedBSDF->IsAnisotropic();
}

void BumpMapBSDF::AddChild(Object * pChildObj, const std::string & Name)
{
	if (pChildObj->GetClassType() == EClassType::EBSDF)
	{
		if (m_pNestedBSDF == nullptr)
		{
			m_pNestedBSDF = (BSDF *)(pChildObj);
		}
		else
		{
			throw HikariException("BumpMapBSDF: tried to specify multiple nested BSDF");
		}
	}
	else if (pChildObj->GetClassType() == EClassType::ETexture)
	{
		if (m_pBumpMap == nullptr)
		{
			m_pBumpMap = (Texture *)(pChildObj);
		}
		else
		{
			throw HikariException("BumpMapBSDF: tried to specify multiple bump map texture.");
		}
	}
}

void BumpMapBSDF::Activate()
{
	if (m_pBumpMap == nullptr)
	{
		throw HikariException("BumpMapBSDF needs a nested BSDF!");
	}

	if (m_pNestedBSDF == nullptr)
	{
		throw HikariException("BumpMapBSDF needs a bump map texture!");
	}

	AddBSDFType(m_pNestedBSDF->GetBSDFTypes());
	if (!m_pBumpMap->IsConstant())
	{
		AddBSDFType(EBSDFType::EUVDependent);
	}
}

std::string BumpMapBSDF::ToString() const
{
	return tfm::format(
		"BumpMapBSDF[\n"
		"  bumpMap = %f,\n"
		"  nestedBSDF = %s\n"
		"]",
		Indent(m_pBumpMap->ToString()),
		Indent(m_pNestedBSDF->ToString())
	);
}

Frame BumpMapBSDF::GetPerturbebFrame(const Intersection & Isect) const
{
	Color3f Gradients[2];
	m_pBumpMap->EvalGradient(Isect, Gradients);

	float dDdU = Gradients[0].GetLuminance();
	float dDdV = Gradients[1].GetLuminance();

	/* Normal derivative terms are ignored */
	Vector3f dPdU = Isect.dPdU + Isect.ShadingFrame.N * (dDdU - Isect.ShadingFrame.N.dot(Isect.dPdU));
	Vector3f dPdV = Isect.dPdV + Isect.ShadingFrame.N * (dDdV - Isect.ShadingFrame.N.dot(Isect.dPdV));

	Frame PerturbebFrame;

	PerturbebFrame.N = dPdU.cross(dPdV).normalized();
	PerturbebFrame.S = (dPdU - PerturbebFrame.N * PerturbebFrame.N.dot(dPdU)).normalized();
	PerturbebFrame.T = PerturbebFrame.N.cross(PerturbebFrame.S);

	if (PerturbebFrame.N.dot(Isect.GeometricFrame.N) < 0.0f)
	{
		PerturbebFrame.N *= -1.0f;
	}

	return PerturbebFrame;
}

NAMESPACE_END


