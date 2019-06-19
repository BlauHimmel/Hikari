#include <bsdf\CoatingBSDF.hpp>
#include <core\Frame.hpp>
#include "..\..\include\bsdf\TwoSidedBSDF.hpp"

NAMESPACE_BEGIN

REGISTER_CLASS(TwoSidedBSDF, XML_BSDF_TWO_SIDED);

TwoSidedBSDF::TwoSidedBSDF(const PropertyList & PropList)
{
	m_pNestedBSDFs[0] = nullptr;
	m_pNestedBSDFs[1] = nullptr;
}

TwoSidedBSDF::~TwoSidedBSDF()
{
	delete m_pNestedBSDFs[0];
	delete m_pNestedBSDFs[1];
}

Color3f TwoSidedBSDF::Sample(BSDFQueryRecord & Record, const Point2f & Sample) const
{
	bool bFlip = false;

	if (Frame::CosTheta(Record.Wi) < 0.0f)
	{
		Record.Wi.z() *= -1.0f;
		bFlip = true;
	}

	Color3f Result = m_pNestedBSDFs[bFlip ? 1 : 0]->Sample(Record, Sample);

	if (bFlip)
	{
		Record.Wi.z() *= -1.0f;
		if (!Result.isZero())
		{
			Record.Wo.z() *= -1.0f;
		}
	}

	return Result;
}

Color3f TwoSidedBSDF::Eval(const BSDFQueryRecord & Record) const
{
	BSDFQueryRecord RecordCopy(Record);

	if (Frame::CosTheta(RecordCopy.Wi) > 0.0f)
	{
		return m_pNestedBSDFs[0]->Eval(RecordCopy);
	}
	else
	{
		RecordCopy.Wi.z() *= -1.0f;
		RecordCopy.Wo.z() *= -1.0f;
		return m_pNestedBSDFs[1]->Eval(RecordCopy);
	}
}

float TwoSidedBSDF::Pdf(const BSDFQueryRecord & Record) const
{
	BSDFQueryRecord RecordCopy(Record);

	if (Frame::CosTheta(RecordCopy.Wi) > 0.0f)
	{
		return m_pNestedBSDFs[0]->Pdf(RecordCopy);
	}
	else
	{
		RecordCopy.Wi.z() *= -1.0f;
		RecordCopy.Wo.z() *= -1.0f;
		return m_pNestedBSDFs[1]->Pdf(RecordCopy);
	}
}

bool TwoSidedBSDF::IsDiffuse() const
{
	return m_pNestedBSDFs[0]->IsDiffuse() || m_pNestedBSDFs[1]->IsDiffuse();
}

bool TwoSidedBSDF::IsAnisotropic() const
{
	return m_pNestedBSDFs[0]->IsAnisotropic() || m_pNestedBSDFs[1]->IsAnisotropic();
}

void TwoSidedBSDF::AddChild(Object * pChildObj, const std::string & Name)
{
	if (pChildObj->GetClassType() == EClassType::EBSDF)
	{
		if (m_pNestedBSDFs[0] == nullptr)
		{
			m_pNestedBSDFs[0] = (BSDF *)(pChildObj);
		}
		else if (m_pNestedBSDFs[1] == nullptr)
		{
			m_pNestedBSDFs[1] = (BSDF *)(pChildObj);
		}
		else
		{
			throw HikariException("TwoSidedBSDF: tried to specify the third nested BSDF");
		}
	}
}

void TwoSidedBSDF::Activate()
{
	if (m_pNestedBSDFs[0] == nullptr)
	{
		throw HikariException("TwoSidedBSDF needs at least one nested BSDF!");
	}

	if (m_pNestedBSDFs[1] == nullptr)
	{
		m_pNestedBSDFs[1] = m_pNestedBSDFs[0];
	}

	AddBSDFType(m_pNestedBSDFs[0]->GetBSDFTypes());
	AddBSDFType(m_pNestedBSDFs[1]->GetBSDFTypes());

	if (m_pNestedBSDFs[0]->HasBSDFType(EDeltaTransmission) ||
		m_pNestedBSDFs[0]->HasBSDFType(EDiffuseTransmission) ||
		m_pNestedBSDFs[0]->HasBSDFType(EGlossyTransmission) ||
		m_pNestedBSDFs[1]->HasBSDFType(EDeltaTransmission) ||
		m_pNestedBSDFs[1]->HasBSDFType(EDiffuseTransmission) ||
		m_pNestedBSDFs[1]->HasBSDFType(EGlossyTransmission))
	{
		throw HikariException("Only BSDF without transmission can be nested!");
	}
}

std::string TwoSidedBSDF::ToString() const
{
	return tfm::format(
		"TwoSidedBSDF[\n"
		"  nestedBSDF[0] = %s\n"
		"  nestedBSDF[1] = %s\n"
		"]",
		Indent(m_pNestedBSDFs[0]->ToString()),
		Indent(m_pNestedBSDFs[1]->ToString())
	);
}

NAMESPACE_END

