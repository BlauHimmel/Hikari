#include <texture\GridTexture.hpp>

NAMESPACE_BEGIN

REGISTER_CLASS(GridTexture, XML_TEXTURE_GRID);

GridTexture::GridTexture(const PropertyList & PropList)
{
	/* Number of the lines per 1.0 u or per 1.0 v */
	m_Lines = PropList.GetInteger(XML_TEXTURE_GRID_LINES, DEFAULT_TEXTURE_GRID_LINES);
	m_BackgroundColor = PropList.GetColor(XML_TEXTURE_GRID_COLOR_BACKGROUND, DEFAULT_TEXTURE_GRID_COLOR_BACKGROUND);
	m_LineColor = PropList.GetColor(XML_TEXTURE_GRID_COLOR_LINE, DEFAULT_TEXTURE_GRID_COLOR_LINE);
	m_LineWidth = PropList.GetFloat(XML_TEXTURE_GRID_LINE_WIDTH, DEFAULT_TEXTURE_GRID_LINE_WIDTH);
	m_UVOffset[0] = PropList.GetFloat(XML_TEXTURE_GRID_OFFSE_U, DEFAULT_TEXTURE_GRID_OFFSET_U);
	m_UVOffset[1] = PropList.GetFloat(XML_TEXTURE_GRID_OFFSE_V, DEFAULT_TEXTURE_GRID_OFFSET_V);
	m_UVScale[0] = PropList.GetFloat(XML_TEXTURE_GRID_SCALE_U, DEFAULT_TEXTURE_GRID_SCALE_U);
	m_UVScale[1] = PropList.GetFloat(XML_TEXTURE_GRID_SCALE_V, DEFAULT_TEXTURE_GRID_SCALE_V);
}

Color3f GridTexture::Eval(const Point2f & UV, const Vector2f & D0, const Vector2f & D1) const
{
	return Eval(UV);
}

Color3f GridTexture::Eval(const Point2f & UV) const
{
	Point2f ScaledUV = UV * (m_Lines - 1);
	float dX = ScaledUV[0] - std::floor(ScaledUV[0]);
	float dY = ScaledUV[1] - std::floor(ScaledUV[1]);

	if (dX > 0.5f) { dX -= 1.0f; }
	if (dY > 0.5f) { dY -= 1.0f; }

	if (std::abs(dX) < m_LineWidth || std::abs(dY) < m_LineWidth)
	{
		return m_LineColor;
	}
	else
	{
		return m_BackgroundColor;
	}
}

Color3f GridTexture::GetAverage() const
{
	float InteriorWidth = std::max(0.0f, 1.0f - 2.0f * m_LineWidth);
	float InteriorArea = InteriorWidth * InteriorWidth;
	float LineArea = 1.0f - InteriorArea;
	return m_LineColor * LineArea + m_BackgroundColor * InteriorArea;
}

Color3f GridTexture::GetMinimum() const
{
	Color3f Min;
	Min[0] = std::min(m_BackgroundColor[0], m_LineColor[0]);
	Min[1] = std::min(m_BackgroundColor[1], m_LineColor[1]);
	Min[2] = std::min(m_BackgroundColor[2], m_LineColor[2]);
	return Min;
}

Color3f GridTexture::GetMaximum() const
{
	Color3f Max;
	Max[0] = std::max(m_BackgroundColor[0], m_LineColor[0]);
	Max[1] = std::max(m_BackgroundColor[1], m_LineColor[1]);
	Max[2] = std::max(m_BackgroundColor[2], m_LineColor[2]);
	return Max;
}

Vector3i GridTexture::GetDimension() const
{
	LOG(WARNING) << "GridTexture::GetDimension() - information meaningless!";
	return Vector3i(0, 0, 0);
}

bool GridTexture::IsConstant() const
{
	return false;
}

bool GridTexture::IsMonochromatic() const
{
	return m_BackgroundColor[0] == m_BackgroundColor[1] && m_BackgroundColor[0] == m_BackgroundColor[2] &&
		m_LineColor[0] == m_LineColor[1] && m_LineColor[0] == m_LineColor[2];
}

std::string GridTexture::ToString() const
{
	return tfm::format(
		"GridTexture[\n"
		"  backgroundColor = %s,\n"
		"  lineColor = %s,\n"
		"  lineWidth = %f,\n"
		"]",
		m_BackgroundColor.ToString(),
		m_LineColor.ToString(),
		m_LineWidth
	);
}

NAMESPACE_END

