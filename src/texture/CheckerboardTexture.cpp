#include <texture\CheckerboardTexture.hpp>

NAMESPACE_BEGIN

REGISTER_CLASS(CheckerboardTexture, XML_TEXTURE_CHECKERBOARD);

CheckerboardTexture::CheckerboardTexture(const PropertyList & PropList)
{
	/* Number of the blocks per 1.0 u or per 1.0 v */
	m_Blocks = PropList.GetInteger(XML_TEXTURE_CHECKERBOARD_BLOCKS, DEFAULT_TEXTURE_CHECKERBOARD_BLOCKS);
	m_ColorA = PropList.GetColor(XML_TEXTURE_CHECKERBOARD_COLOR_A, DEFAULT_TEXTURE_CHECKERBOARD_COLOR_A);
	m_ColorB = PropList.GetColor(XML_TEXTURE_CHECKERBOARD_COLOR_B, DEFAULT_TEXTURE_CHECKERBOARD_COLOR_B);
	m_UVOffset[0] = PropList.GetFloat(XML_TEXTURE_CHECKERBOARD_OFFSE_U, DEFAULT_TEXTURE_CHECKERBOARD_OFFSET_U);
	m_UVOffset[1] = PropList.GetFloat(XML_TEXTURE_CHECKERBOARD_OFFSE_V, DEFAULT_TEXTURE_CHECKERBOARD_OFFSET_V);
	m_UVScale[0] = PropList.GetFloat(XML_TEXTURE_CHECKERBOARD_SCALE_U, DEFAULT_TEXTURE_CHECKERBOARD_SCALE_U);
	m_UVScale[1] = PropList.GetFloat(XML_TEXTURE_CHECKERBOARD_SCALE_V, DEFAULT_TEXTURE_CHECKERBOARD_SCALE_V);
}

Color3f CheckerboardTexture::Eval(const Point2f & UV, const Vector2f & D0, const Vector2f & D1) const
{
	return Eval(UV);
}

Color3f CheckerboardTexture::Eval(const Point2f & UV) const
{
	bool bEvenX = ModPositive(ModPositive(int(UV[0] * m_Blocks), m_Blocks), 2) == 0;
	bool bEvenY = ModPositive(ModPositive(int(UV[1] * m_Blocks), m_Blocks), 2) == 0;
	return (bEvenX ^ bEvenY) ? m_ColorA : m_ColorB;
}

Color3f CheckerboardTexture::GetAverage() const
{
	return (m_ColorA + m_ColorB) * 0.5f;
}

Color3f CheckerboardTexture::GetMinimum() const
{
	Color3f Min;
	Min[0] = std::min(m_ColorA[0], m_ColorB[0]);
	Min[1] = std::min(m_ColorA[1], m_ColorB[1]);
	Min[2] = std::min(m_ColorA[2], m_ColorB[2]);
	return Min;
}

Color3f CheckerboardTexture::GetMaximum() const
{
	Color3f Max;
	Max[0] = std::max(m_ColorA[0], m_ColorB[0]);
	Max[1] = std::max(m_ColorA[1], m_ColorB[1]);
	Max[2] = std::max(m_ColorA[2], m_ColorB[2]);
	return Max;
}

Vector3i CheckerboardTexture::GetDimension() const
{
	LOG(WARNING) << "CheckerboardTexture::GetDimension() - information meaningless!";
	return Vector3i(0, 0, 0);
}

bool CheckerboardTexture::IsConstant() const
{
	return false;
}

bool CheckerboardTexture::IsMonochromatic() const
{
	return m_ColorA[0] == m_ColorA[1] && m_ColorA[0] == m_ColorA[2] &&
		m_ColorB[0] == m_ColorB[1] && m_ColorB[0] == m_ColorB[2];
}

std::string CheckerboardTexture::ToString() const
{
	return tfm::format(
		"CheckerboardTexture[\n"
		"  colorA = %s,\n"
		"  colorB = %s,\n"
		"  blocks = %d,\n"
		"]",
		m_ColorA.ToString(),
		m_ColorB.ToString(),
		m_Blocks
	);
}

NAMESPACE_END


