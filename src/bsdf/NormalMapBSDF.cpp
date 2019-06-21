#include <bsdf\NormalMapBSDF.hpp>
#include <core\Frame.hpp>
#include <core\Texture.hpp>
#include <core\Intersection.hpp>

NAMESPACE_BEGIN

REGISTER_CLASS(NormalMapBSDF, XML_BSDF_NORMAL_MAP);

NormalMapBSDF::NormalMapBSDF(const PropertyList & PropList)
{
	m_pNestedBSDF = nullptr;
	m_pNormalMap = nullptr;
}

NormalMapBSDF::~NormalMapBSDF()
{
	delete m_pNormalMap;
	delete m_pNestedBSDF;
}

Color3f NormalMapBSDF::Sample(BSDFQueryRecord & Record, const Point2f & Sample) const
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

Color3f NormalMapBSDF::Eval(const BSDFQueryRecord & Record) const
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

float NormalMapBSDF::Pdf(const BSDFQueryRecord & Record) const
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

bool NormalMapBSDF::IsDiffuse() const
{
	return m_pNestedBSDF->IsDiffuse();
}

bool NormalMapBSDF::IsAnisotropic() const
{
	return m_pNestedBSDF->IsAnisotropic();
}

void NormalMapBSDF::AddChild(Object * pChildObj, const std::string & Name)
{
	if (pChildObj->GetClassType() == EClassType::EBSDF)
	{
		if (m_pNestedBSDF == nullptr)
		{
			m_pNestedBSDF = (BSDF *)(pChildObj);
		}
		else
		{
			throw HikariException("NormalMapBSDF: tried to specify multiple nested BSDF");
		}
	}
	else if (pChildObj->GetClassType() == EClassType::ETexture)
	{
		if (m_pNormalMap == nullptr)
		{
			m_pNormalMap = (Texture *)(pChildObj);
		}
		else
		{
			throw HikariException("NormalMapBSDF: tried to specify multiple normal map texture.");
		}
	}
}

void NormalMapBSDF::Activate()
{
	if (m_pNormalMap == nullptr)
	{
		throw HikariException("NormalMapBSDF needs a nested BSDF!");
	}

	if (m_pNestedBSDF == nullptr)
	{
		throw HikariException("NormalMapBSDF needs a bump map texture!");
	}

	AddBSDFType(m_pNestedBSDF->GetBSDFTypes());
	if (!m_pNormalMap->IsConstant())
	{
		AddBSDFType(EBSDFType::EUVDependent);
	}
}

std::string NormalMapBSDF::ToString() const
{
	return tfm::format(
		"NormalMapBSDF[\n"
		"  normalMap = %f,\n"
		"  nestedBSDF = %s\n"
		"]",
		Indent(m_pNormalMap->ToString()),
		Indent(m_pNestedBSDF->ToString())
	);
}

Frame NormalMapBSDF::GetPerturbebFrame(const Intersection & Isect) const
{
	Frame PerturbebFrame;

	Color3f NormalOrigin = m_pNormalMap->Eval(Isect, false);

	Vector3f Normal = {
		2.0f * NormalOrigin[0] - 1.0f,
		2.0f * NormalOrigin[1] - 1.0f,
		2.0f * NormalOrigin[2] - 1.0f
	};

	PerturbebFrame.N = Isect.ShadingFrame.ToWorld(Normal).normalized();
	PerturbebFrame.S = (Isect.dPdU - PerturbebFrame.N * PerturbebFrame.N.dot(Isect.dPdU)).normalized();
	PerturbebFrame.T = PerturbebFrame.N.cross(PerturbebFrame.S);

	return PerturbebFrame;
}

NAMESPACE_END

