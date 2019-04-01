#include <texture\BitmapTexture.hpp>

NAMESPACE_BEGIN

REGISTER_CLASS(BitmapTexture, XML_TEXTURE_BITMAP);

BitmapTexture::BitmapTexture(const PropertyList & PropList)
{
	m_Filename = PropList.GetString(XML_TEXTURE_BITMAP_FILENAME);
	filesystem::path Filename = GetFileResolver()->resolve(m_Filename);
	m_Filename = Filename.str();

	m_Gamma = PropList.GetFloat(XML_TEXTURE_BITMAP_GAMMA, DEFAULT_TEXTURE_BITMAP_GAMMA);

	std::string WrapMode = PropList.GetString(XML_TEXTURE_BITMAP_WRAP_MODE, "");
	if (WrapMode != "")
	{
		if (WrapMode == XML_TEXTURE_BITMAP_WRAP_MODE_REPEAT) { m_UWrapMode = EWrapMode::ERepeat; m_VWrapMode = EWrapMode::ERepeat; }
		else if (WrapMode == XML_TEXTURE_BITMAP_WRAP_MODE_CLAMP) { m_UWrapMode = EWrapMode::EClamp; m_VWrapMode = EWrapMode::EClamp; }
		else if (WrapMode == XML_TEXTURE_BITMAP_WRAP_MODE_BLACK) { m_UWrapMode = EWrapMode::EBlack; m_VWrapMode = EWrapMode::EBlack; }
		else { throw HikariException("Illegal wrap mode [%s].", WrapMode.c_str()); }
	}
	else
	{
		std::string UWrapMode = "", VWrapMode = "";
		UWrapMode = PropList.GetString(XML_TEXTURE_BITMAP_WRAP_MODE_U, DEFAULT_TEXTURE_BITMAP_WRAP_MODE_U);
		VWrapMode = PropList.GetString(XML_TEXTURE_BITMAP_WRAP_MODE_V, DEFAULT_TEXTURE_BITMAP_WRAP_MODE_V);

		if (UWrapMode == XML_TEXTURE_BITMAP_WRAP_MODE_REPEAT) { m_UWrapMode = EWrapMode::ERepeat; }
		else if (UWrapMode == XML_TEXTURE_BITMAP_WRAP_MODE_CLAMP) { m_UWrapMode = EWrapMode::EClamp; }
		else if (UWrapMode == XML_TEXTURE_BITMAP_WRAP_MODE_BLACK) { m_UWrapMode = EWrapMode::EBlack; }
		else { throw HikariException("Illegal wrap mode [%s].", UWrapMode.c_str()); }

		if (VWrapMode == XML_TEXTURE_BITMAP_WRAP_MODE_REPEAT) { m_VWrapMode = EWrapMode::ERepeat; }
		else if (VWrapMode == XML_TEXTURE_BITMAP_WRAP_MODE_CLAMP) { m_VWrapMode = EWrapMode::EClamp; }
		else if (VWrapMode == XML_TEXTURE_BITMAP_WRAP_MODE_BLACK) { m_VWrapMode = EWrapMode::EBlack; }
		else { throw HikariException("Illegal wrap mode [%s].", VWrapMode.c_str()); }

	}

	std::string FilterType = PropList.GetString(XML_TEXTURE_BITMAP_FILTER_TYPE, DEFAULT_TEXTURE_BITMAP_FILTER_TYPE);
	if (FilterType == XML_TEXTURE_BITMAP_FILTER_TYPE_NEAREST) { m_FilterType = EFilterType::ENearest; }
	else if (FilterType == XML_TEXTURE_BITMAP_FILTER_TYPE_BILINEAR) { m_FilterType = EFilterType::EBilinear; }
	else if (FilterType == XML_TEXTURE_BITMAP_FILTER_TYPE_TRILINEAR) { m_FilterType = EFilterType::ETrilinear; }
	else if (FilterType == XML_TEXTURE_BITMAP_FILTER_TYPE_EWA) { m_FilterType = EFilterType::EEWA; }
	else { throw HikariException("Illegal filter type [%s].", FilterType.c_str()); }

	m_MaxAnisotropy = PropList.GetFloat(XML_TEXTURE_BITMAP_MAX_ANISOTROPY, DEFAULT_TEXTURE_BITMAP_MAX_ANISOTROPY);
	m_UVOffset[0] = PropList.GetFloat(XML_TEXTURE_BITMAP_OFFSE_U, DEFAULT_TEXTURE_BITMAP_OFFSET_U);
	m_UVOffset[1] = PropList.GetFloat(XML_TEXTURE_BITMAP_OFFSE_V, DEFAULT_TEXTURE_BITMAP_OFFSET_V);
	m_UVScale[0] = PropList.GetFloat(XML_TEXTURE_BITMAP_SCALE_U, DEFAULT_TEXTURE_BITMAP_SCALE_U);
	m_UVScale[1] = PropList.GetFloat(XML_TEXTURE_BITMAP_SCALE_V, DEFAULT_TEXTURE_BITMAP_SCALE_V);

	std::string Channel = PropList.GetString(XML_TEXTURE_BITMAP_CHANNEL, DEFAULT_TEXTURE_BITMAP_CHANNEL);
	if (Channel == XML_TEXTURE_BITMAP_CHANNEL_R) { m_Channel = 1; }
	else if (Channel == XML_TEXTURE_BITMAP_CHANNEL_RGB) { m_Channel = 3; }
	else { throw HikariException("Illegal channel [%s].", Channel.c_str()); }

	if (m_Channel == 1)
	{
		float Average, Maximum, Minimum;
		int Width, Height;
		std::unique_ptr<float[]> Data = LoadImageFromFileR(m_Filename, m_Gamma, Width, Height, &Average, &Maximum, &Minimum);
		m_Texture1f.reset(new MipMap1f(
			Point2i(Width, Height),
			Data.get(),
			m_FilterType == EFilterType::ETrilinear,
			m_MaxAnisotropy,
			m_UWrapMode,
			m_VWrapMode
		));
		m_Texture3f.reset();
		m_Width = uint32_t(Width);
		m_Height = uint32_t(Height);
		m_Average = Color3f(Average);
		m_Maximum = Color3f(Maximum);
		m_Minimum = Color3f(Minimum);
	}
	else //if (m_Channel == 3)
	{
		int Width, Height;
		std::unique_ptr<Color3f[]> Data = LoadImageFromFileRGB(m_Filename, m_Gamma, Width, Height, &m_Average, &m_Maximum, &m_Minimum);
		m_Texture3f.reset(new MipMap3f(
			Point2i(Width, Height),
			Data.get(),
			m_FilterType == EFilterType::ETrilinear,
			m_MaxAnisotropy,
			m_UWrapMode,
			m_VWrapMode
		));
		m_Texture1f.reset();
		m_Width = uint32_t(Width);
		m_Height = uint32_t(Height);
	}
}

Color3f BitmapTexture::Eval(const Point2f & UV, const Vector2f & D0, const Vector2f & D1) const
{
	if (m_Channel == 1)
	{
		if (m_FilterType == EFilterType::ENearest)
		{
			return Color3f(m_Texture1f->Lookup(UV));
		}
		else if (m_FilterType == EFilterType::EBilinear)
		{
			return Color3f(m_Texture1f->Lookup(UV, 0.0f));
		}
		else
		{
			return Color3f(m_Texture1f->Lookup(UV, D0, D1));
		}
	}
	else //if (m_Channel == 3)
	{
		if (m_FilterType == EFilterType::ENearest)
		{
			return m_Texture3f->Lookup(UV);
		}
		else if (m_FilterType == EFilterType::EBilinear)
		{
			return m_Texture3f->Lookup(UV, 0.0f);
		}
		else
		{
			return m_Texture3f->Lookup(UV, D0, D1);
		}
	}
}

Color3f BitmapTexture::GetAverage() const
{
	return m_Average;
}

Color3f BitmapTexture::GetMinimum() const
{
	return m_Maximum;
}

Color3f BitmapTexture::GetMaximum() const
{
	return m_Minimum;
}

Vector3i BitmapTexture::GetDimension() const
{
	CHECK(m_Channel == 1 || m_Channel == 3);
	if (m_Channel == 1)
	{
		return Vector3i(m_Texture1f->GetWidth(), m_Texture1f->GetHeight());
	}
	else //if (m_Channel == 3)
	{
		return Vector3i(m_Texture3f->GetWidth(), m_Texture3f->GetHeight());
	}
}

bool BitmapTexture::IsConstant() const
{
	return false;
}

bool BitmapTexture::IsMonochromatic() const
{
	return m_Texture1f.get() != nullptr;
}

std::string BitmapTexture::ToString() const
{
	std::string UWrapMode, VWrapMode, FilterType;
	
	if (m_UWrapMode == EWrapMode::ERepeat) { UWrapMode = XML_TEXTURE_BITMAP_WRAP_MODE_REPEAT; }
	else if (m_UWrapMode == EWrapMode::EClamp) { UWrapMode = XML_TEXTURE_BITMAP_WRAP_MODE_CLAMP; }
	else if (m_UWrapMode == EWrapMode::EBlack) { UWrapMode = XML_TEXTURE_BITMAP_WRAP_MODE_BLACK; }
	
	if (m_VWrapMode == EWrapMode::ERepeat) { VWrapMode = XML_TEXTURE_BITMAP_WRAP_MODE_REPEAT; }
	else if (m_VWrapMode == EWrapMode::EClamp) { VWrapMode = XML_TEXTURE_BITMAP_WRAP_MODE_CLAMP; }
	else if (m_VWrapMode == EWrapMode::EBlack) { VWrapMode = XML_TEXTURE_BITMAP_WRAP_MODE_BLACK; }

	if (m_FilterType == EFilterType::ENearest) { FilterType = XML_TEXTURE_BITMAP_FILTER_TYPE_NEAREST; }
	else if (m_FilterType == EFilterType::EBilinear) { FilterType = XML_TEXTURE_BITMAP_FILTER_TYPE_BILINEAR; }
	else if (m_FilterType == EFilterType::ETrilinear) { FilterType = XML_TEXTURE_BITMAP_FILTER_TYPE_TRILINEAR; }
	else if (m_FilterType == EFilterType::EEWA) { FilterType = XML_TEXTURE_BITMAP_FILTER_TYPE_EWA; }

	return tfm::format(
		"BitmapTexture[\n"
		"  filename = %s,\n"
		"  gamma = %f,\n"
		"  uWrapMode = %s,\n"
		"  vWrapMode = %s,\n"
		"  filterType = %s,\n"
		"  maxAnisotropy = %f,\n"
		"  channel = %s,\n"
		"  width = %d,\n"
		"  height = %d\n"
		"]",
		m_Filename,
		m_Gamma,
		UWrapMode,
		VWrapMode,
		FilterType,
		m_MaxAnisotropy,
		m_Channel == 1 ? "R" : "RGB",
		m_Width,
		m_Height
	);
}

NAMESPACE_END