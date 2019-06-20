#include <texture\ScaleTexture.hpp>

NAMESPACE_BEGIN

REGISTER_CLASS(ScaleTexture, XML_TEXTURE_SCALE);

ScaleTexture::ScaleTexture(const PropertyList & PropList)
{
	m_Scale = PropList.GetFloat(XML_TEXTURE_SCALE_SCALE);

	m_pNestedTexture = nullptr;
}

ScaleTexture::~ScaleTexture()
{
	delete m_pNestedTexture;
}

Color3f ScaleTexture::Eval(const Intersection & Isect, bool bFilter) const
{
	return m_pNestedTexture->Eval(Isect, bFilter) * m_Scale;
}

void ScaleTexture::EvalGradient(const Intersection & Isect, Color3f * pGradients) const
{
	m_pNestedTexture->EvalGradient(Isect, pGradients);

	pGradients[0] *= m_Scale;
	pGradients[1] *= m_Scale;
}

Color3f ScaleTexture::GetAverage() const
{
	return m_pNestedTexture->GetAverage() * m_Scale;
}

Color3f ScaleTexture::GetMinimum() const
{
	return m_pNestedTexture->GetMinimum() * m_Scale;
}

Color3f ScaleTexture::GetMaximum() const
{
	return m_pNestedTexture->GetMaximum() * m_Scale;
}

Vector3i ScaleTexture::GetDimension() const
{
	return m_pNestedTexture->GetDimension();
}

bool ScaleTexture::IsConstant() const
{
	return m_pNestedTexture->IsConstant();
}

bool ScaleTexture::IsMonochromatic() const
{
	return m_pNestedTexture->IsMonochromatic();
}

Texture * ScaleTexture::ActualTexture()
{
	return m_pNestedTexture;
}

void ScaleTexture::AddChild(Object * pChildObj, const std::string & Name)
{
	if (pChildObj->GetClassType() == EClassType::ETexture)
	{
		if (m_pNestedTexture == nullptr)
		{
			m_pNestedTexture = (Texture *)(pChildObj);
		}
		else
		{
			throw HikariException("ScaleTexture: tried to specify multiple nested texture");
		}
	}
}

void ScaleTexture::Activate()
{
	if (m_pNestedTexture == nullptr)
	{
		throw HikariException("ScaleTexture needs a nested texture!");
	}
}

std::string ScaleTexture::ToString() const
{
	return tfm::format(
		"ScaleTexture[\n"
		"  scale = %f,\n"
		"  texture = %s,\n"
		"]",
		m_Scale,
		Indent(m_pNestedTexture->ToString())
	);
}

NAMESPACE_END
