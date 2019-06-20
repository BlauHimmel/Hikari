#include <texture\CurvatureTexture.hpp>
#include <core\Shape.hpp>
#include <core\Intersection.hpp>

NAMESPACE_BEGIN

REGISTER_CLASS(CurvatureTexture, XML_TEXTURE_CURVATURE);

CurvatureTexture::CurvatureTexture(const PropertyList & PropList)
{
	m_PositiveColor = PropList.GetColor(XML_TEXTURE_CURVATURE_POSITIVE_COLOR, DEFAULT_TEXTURE_CURVATURE_POSITIVE_COLOR);
	m_NegativeColor = PropList.GetColor(XML_TEXTURE_CURVATURE_NEGATIVE_COLOR, DEFAULT_TEXTURE_CURVATURE_NEGATIVE_COLOR);
	m_Scale = PropList.GetFloat(XML_TEXTURE_CURVATURE_SCALE, DEFAULT_TEXTURE_CURVATURE_SCALE);
	m_CurvatureType = PropList.GetString(XML_TEXTURE_CURVATURE_TYPE);
	m_UVOffset[0] = PropList.GetFloat(XML_TEXTURE_CURVATURE_OFFSE_U, DEFAULT_TEXTURE_CURVATURE_OFFSET_U);
	m_UVOffset[1] = PropList.GetFloat(XML_TEXTURE_CURVATURE_OFFSE_V, DEFAULT_TEXTURE_CURVATURE_OFFSET_V);
	m_UVScale[0] = PropList.GetFloat(XML_TEXTURE_CURVATURE_SCALE_U, DEFAULT_TEXTURE_CURVATURE_SCALE_U);
	m_UVScale[1] = PropList.GetFloat(XML_TEXTURE_CURVATURE_SCALE_V, DEFAULT_TEXTURE_CURVATURE_SCALE_V);

	if (m_CurvatureType == "gaussian")
	{
		m_bGaussianCurvature = true;
	}
	else if (m_CurvatureType == "mean")
	{
		m_bGaussianCurvature = false;
	}
	else
	{
		throw HikariException("Unexpected curvature type <%s>.", m_CurvatureType);
	}
}

Color3f CurvatureTexture::Eval(const Intersection & Isect, bool bFilter) const
{
	float H, K;
	Isect.pShape->ComputeCurvature(Isect, H, K);

	static auto LookUpColor = [&](float Value) -> Color3f
	{
		if (Value < 0.0f)
		{
			return std::min(-Value * m_Scale, 1.0f) * m_NegativeColor;
		}
		if (Value > 0.0f)
		{
			return std::min(Value * m_Scale, 1.0f) * m_PositiveColor;
		}
		return Color3f(0.0f);
	};

	return LookUpColor(m_bGaussianCurvature ? K : H);
}

Color3f CurvatureTexture::GetAverage() const
{
	LOG(WARNING) << "CurvatureTexture::GetAverage() - information meaningless!";
	return Color3f(0.5f);
}

Color3f CurvatureTexture::GetMinimum() const
{
	LOG(WARNING) << "CurvatureTexture::GetMaximum() - information meaningless!";
	return Color3f(0.0f);
}

Color3f CurvatureTexture::GetMaximum() const
{
	LOG(WARNING) << "CurvatureTexture::GetMaximum() - information meaningless!";
	return Color3f(1.0f);
}

Vector3i CurvatureTexture::GetDimension() const
{
	LOG(WARNING) << "CurvatureTexture::GetDimension() - information meaningless!";
	return Vector3i(0, 0, 0);
}

bool CurvatureTexture::IsConstant() const
{
	return false;
}

bool CurvatureTexture::IsMonochromatic() const
{
	return false;
}

std::string CurvatureTexture::ToString() const
{
	return tfm::format(
		"CurvatureTexture[\n"
		"  scale = %f,\n"
		"  curvatureType = %s,\n"
		"]",
		m_Scale,
		m_CurvatureType
	);
}

NAMESPACE_END
