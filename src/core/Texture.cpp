#include <core\Texture.hpp>
#include <core\Intersection.hpp>
#include <core\Color.hpp>
#include <core\Vector.hpp>
#include <core\Timer.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

NAMESPACE_BEGIN

std::unique_ptr<float[]> LoadImageFromFileR(
	const std::string & Filename,
	float Gamma,
	int & Width,
	int & Height,
	float * pAverage,
	float * pMaximum,
	float * pMinimum
)
{
	// Top left - (0,0)

	LOG(INFO) << "Loading texture (R) \"" << Filename << "\" ... ";
	cout.flush();
	Timer ObjTimer;

	int ChannelsInFile = -1, RequestChannels = 1;
	unsigned char * pData = stbi_load(Filename.c_str(), &Width, &Height, &ChannelsInFile, RequestChannels);

	if (pData == nullptr)
	{
		const char * pFailReason = stbi_failure_reason();
		throw HikariException("Load texture \"%s\" failed. Reason : [%s]", Filename.c_str(), pFailReason);
	}

	std::unique_ptr<float[]> Pixels(new float[Width * Height]);

	constexpr float Inv255 = 1.0f / 255.0f;
	const float InvGamma = 1.0f / Gamma;

	float Maximum = std::numeric_limits<float>::min();
	float Minimum = std::numeric_limits<float>::max();
	float Average = 0.0f;

	for (int x = 0; x < Width; x++)
	{
		for (int y = 0; y < Height; y++)
		{
			int LoadPixelIdx = y * Width + x;
			int StorePixelIdx = (Height - 1 - y) * Width + x;
			
			Pixels[StorePixelIdx] = GammaCorrect(float(pData[LoadPixelIdx * RequestChannels + 0]) * Inv255, InvGamma);

			if (Pixels[StorePixelIdx] > Maximum) { Maximum = Pixels[StorePixelIdx]; }
			if (Pixels[StorePixelIdx] < Minimum) { Minimum = Pixels[StorePixelIdx]; }
			Average += Pixels[StorePixelIdx];
		}
	}

	Average /= (Width * Height);

	if (pMaximum != nullptr) { *pMaximum = Maximum; }
	if (pMinimum != nullptr) { *pMinimum = Minimum; }
	if (pAverage != nullptr) { *pAverage = Average; }

	stbi_image_free(pData);

	LOG(INFO) << "Done. (Took " << ObjTimer.ElapsedString() << ")";

	return Pixels;
}

std::unique_ptr<Color3f[]> LoadImageFromFileRGB(
	const std::string & Filename,
	float Gamma,
	int & Width,
	int & Height,
	Color3f * pAverage,
	Color3f * pMaximum,
	Color3f * pMinimum
)
{
	// Top left - (0,0)

	LOG(INFO) << "Loading texture (RGB) \"" << Filename << "\" ... ";
	cout.flush();
	Timer ObjTimer;

	int ChannelsInFile = -1, RequestChannels = 3;
	unsigned char * pData = stbi_load(Filename.c_str(), &Width, &Height, &ChannelsInFile, RequestChannels);

	if (pData == nullptr)
	{
		const char * pFailReason = stbi_failure_reason();
		throw HikariException("Load texture \"%s\" failed. Reason : [%s]", Filename.c_str(), pFailReason);
	}

	std::unique_ptr<Color3f[]> Pixels(new Color3f[Width * Height]);

	constexpr float Inv255 = 1.0f / 255.0f;
	const float InvGamma = 1.0f / Gamma;

	Color3f Maximum = Color3f(std::numeric_limits<float>::min());
	Color3f Minimum = Color3f(std::numeric_limits<float>::max());
	Color3f Average = Color3f(0.0f);

	for (int x = 0; x < Width; x++)
	{
		for (int y = 0; y < Height; y++)
		{
			int LoadPixelIdx = y * Width + x;
			int StorePixelIdx = (Height - 1 - y) * Width + x;

			Pixels[StorePixelIdx] = Color3f(
				GammaCorrect(float(pData[LoadPixelIdx * RequestChannels + 0]) * Inv255, InvGamma),
				GammaCorrect(float(pData[LoadPixelIdx * RequestChannels + 1]) * Inv255, InvGamma),
				GammaCorrect(float(pData[LoadPixelIdx * RequestChannels + 2]) * Inv255, InvGamma)
			);

			if (Pixels[StorePixelIdx][0] > Maximum[0]) { Maximum[0] = Pixels[StorePixelIdx][0]; }
			if (Pixels[StorePixelIdx][1] > Maximum[1]) { Maximum[1] = Pixels[StorePixelIdx][1]; }
			if (Pixels[StorePixelIdx][2] > Maximum[2]) { Maximum[2] = Pixels[StorePixelIdx][2]; }
			if (Pixels[StorePixelIdx][0] < Minimum[0]) { Minimum[0] = Pixels[StorePixelIdx][0]; }
			if (Pixels[StorePixelIdx][1] < Minimum[1]) { Minimum[1] = Pixels[StorePixelIdx][1]; }
			if (Pixels[StorePixelIdx][2] < Minimum[2]) { Minimum[2] = Pixels[StorePixelIdx][2]; }
			Average[0] += Pixels[StorePixelIdx][0];
			Average[1] += Pixels[StorePixelIdx][1];
			Average[2] += Pixels[StorePixelIdx][2];
		}
	}

	Average /= float(Width * Height);

	if (pMaximum != nullptr) { *pMaximum = Maximum; }
	if (pMinimum != nullptr) { *pMinimum = Minimum; }
	if (pAverage != nullptr) { *pAverage = Average; }

	stbi_image_free(pData);

	LOG(INFO) << "Done. (Took " << ObjTimer.ElapsedString() << ")";

	return Pixels;
}

Color3f Texture::Eval(const Intersection & Isect) const
{
	throw HikariException("Texture::Eval(const Intersection & Isect) is not implemented!");
}

Color3f Texture::GetAverage() const
{
	throw HikariException("Texture::GetAverage() is not implemented!");
}

Color3f Texture::GetMinimum() const
{
	throw HikariException("Texture::GetMinimum() is not implemented!");
}

Color3f Texture::GetMaximum() const
{
	throw HikariException("Texture::GetMaximum() is not implemented!");
}

Vector3i Texture::GetDimension() const
{
	throw HikariException("Texture::GetDimension() is not implemented!");
}

bool Texture::IsConstant() const
{
	throw HikariException("Texture::IsConstant() is not implemented!");
}

bool Texture::IsMonochromatic() const
{
	throw HikariException("Texture::IsMonochromatic() is not implemented!");
}

Texture * Texture::ActualTexture()
{
	return this;
}

Object::EClassType Texture::GetClassType() const
{
	return EClassType::ETexture;
}

Color3f Texture2D::Eval(const Intersection & Isect) const
{
	Point2f UV = Point2f(Isect.UV.x() * m_UVScale.x(), Isect.UV.y() * m_UVScale.y()) + m_UVOffset;

	return Eval(UV,
		Vector2f(Isect.dUdX * m_UVScale.x(), Isect.dVdX * m_UVScale.y()),
		Vector2f(Isect.dUdY * m_UVScale.x(), Isect.dVdY * m_UVScale.y()));
}

ConstantColor3fTexture::ConstantColor3fTexture(const Color3f & Value) : m_Value(Value) { }

Color3f ConstantColor3fTexture::Eval(const Intersection & Isect) const
{
	return m_Value;
}

Color3f ConstantColor3fTexture::GetAverage() const
{
	return m_Value;
}

Color3f ConstantColor3fTexture::GetMinimum() const
{
	return m_Value;
}

Color3f ConstantColor3fTexture::GetMaximum() const
{
	return m_Value;
}

Vector3i ConstantColor3fTexture::GetDimension() const
{
	return Vector3i(1, 1, 1);
}

bool ConstantColor3fTexture::IsConstant() const
{
	return true;
}

bool ConstantColor3fTexture::IsMonochromatic() const
{
	return m_Value[0] == m_Value[1] && m_Value[1] == m_Value[2];
}

std::string ConstantColor3fTexture::ToString() const
{
	return tfm::format(
		"ConstantColor3fTexture[value = %s]", m_Value.ToString()
	);
}

ConstantFloatTexture::ConstantFloatTexture(float Value) : m_Value(Value) { }

Color3f ConstantFloatTexture::Eval(const Intersection & Isect) const
{
	return Color3f(m_Value);
}

Color3f ConstantFloatTexture::GetAverage() const
{
	return Color3f(m_Value);
}

Color3f ConstantFloatTexture::GetMinimum() const
{
	return Color3f(m_Value);
}

Color3f ConstantFloatTexture::GetMaximum() const
{
	return Color3f(m_Value);
}

Vector3i ConstantFloatTexture::GetDimension() const
{
	return Vector3i(1, 1, 1);
}

bool ConstantFloatTexture::IsConstant() const
{
	return true;
}

bool ConstantFloatTexture::IsMonochromatic() const
{
	return true;
}

std::string ConstantFloatTexture::ToString() const
{
	return tfm::format(
		"ConstantFloatTexture[value = %f]", m_Value
	);
}

Color3fAdditionTexture::Color3fAdditionTexture(const Texture * pTextureA, const Texture * pTextureB) :
	m_pTextureA(pTextureA), m_pTextureB(pTextureB)
{
	CHECK_NOTNULL(m_pTextureA);
	CHECK_NOTNULL(m_pTextureB);
}

Color3f Color3fAdditionTexture::Eval(const Intersection & Isect) const
{
	return m_pTextureA->Eval(Isect) + m_pTextureB->Eval(Isect);
}

Color3f Color3fAdditionTexture::GetAverage() const
{
	return m_pTextureA->GetAverage() + m_pTextureB->GetAverage();
}

Color3f Color3fAdditionTexture::GetMinimum() const
{
	// This is a conservative estimate
	return m_pTextureA->GetMinimum() + m_pTextureB->GetMinimum();
}

Color3f Color3fAdditionTexture::GetMaximum() const
{
	// This is a conservative estimate
	return m_pTextureA->GetMaximum() + m_pTextureB->GetMaximum();
}

Vector3i Color3fAdditionTexture::GetDimension() const
{
	LOG(WARNING) << "Color3fAdditionTexture::GetDimension() - information meaningless!";
	return Vector3i(0, 0, 0);
}

bool Color3fAdditionTexture::IsConstant() const
{
	return m_pTextureA->IsConstant() && m_pTextureB->IsConstant();
}

bool Color3fAdditionTexture::IsMonochromatic() const
{
	return m_pTextureA->IsMonochromatic() && m_pTextureB->IsMonochromatic();
}

std::string Color3fAdditionTexture::ToString() const
{
	return tfm::format(
		"Color3fAdditionTexture[\n"
		" textureA = %s,\n"
		" textureB = %s,\n"
		"]",
		Indent(m_pTextureA->ToString()),
		Indent(m_pTextureB->ToString())
	);
}

Color3fSubtractionTexture::Color3fSubtractionTexture(const Texture * pTextureA, const Texture * pTextureB) :
	m_pTextureA(pTextureA), m_pTextureB(pTextureB)
{
	CHECK_NOTNULL(m_pTextureA);
	CHECK_NOTNULL(m_pTextureB);
}

Color3f Color3fSubtractionTexture::Eval(const Intersection & Isect) const
{
	return m_pTextureA->Eval(Isect) - m_pTextureB->Eval(Isect);
}

Color3f Color3fSubtractionTexture::GetAverage() const
{
	return m_pTextureA->GetAverage() - m_pTextureB->GetAverage();
}

Color3f Color3fSubtractionTexture::GetMinimum() const
{
	// This is a conservative estimate
	return m_pTextureA->GetMinimum() - m_pTextureB->GetMinimum();
}

Color3f Color3fSubtractionTexture::GetMaximum() const
{
	// This is a conservative estimate
	return m_pTextureA->GetMaximum() - m_pTextureB->GetMaximum();
}

Vector3i Color3fSubtractionTexture::GetDimension() const
{
	LOG(WARNING) << "Color3fAdditionTexture::GetDimension() - information meaningless!";
	return Vector3i(0, 0, 0);
}

bool Color3fSubtractionTexture::IsConstant() const
{
	return m_pTextureA->IsConstant() && m_pTextureB->IsConstant();
}

bool Color3fSubtractionTexture::IsMonochromatic() const
{
	return m_pTextureA->IsMonochromatic() && m_pTextureB->IsMonochromatic();
}

std::string Color3fSubtractionTexture::ToString() const
{
	return tfm::format(
		"Color3fSubtractionTexture[\n"
		" textureA = %s,\n"
		" textureB = %s,\n"
		"]",
		Indent(m_pTextureA->ToString()),
		Indent(m_pTextureB->ToString())
	);
}

Color3fProductTexture::Color3fProductTexture(const Texture * pTextureA, const Texture * pTextureB) :
	m_pTextureA(pTextureA), m_pTextureB(pTextureB)
{
	CHECK_NOTNULL(m_pTextureA);
	CHECK_NOTNULL(m_pTextureB);
}

Color3f Color3fProductTexture::Eval(const Intersection & Isect) const
{
	return m_pTextureA->Eval(Isect) * m_pTextureB->Eval(Isect);
}

Color3f Color3fProductTexture::GetAverage() const
{
	LOG(ERROR) << "Color3fProductTexture::GetAverage() - information unavailable!";
	return Color3f(0.0f);
}

Color3f Color3fProductTexture::GetMinimum() const
{
	// This is a conservative estimate
	return m_pTextureA->GetMinimum() * m_pTextureB->GetMinimum();
}

Color3f Color3fProductTexture::GetMaximum() const
{
	// This is a conservative estimate
	return m_pTextureA->GetMaximum() * m_pTextureB->GetMaximum();
}

Vector3i Color3fProductTexture::GetDimension() const
{
	LOG(WARNING) << "Color3fAdditionTexture::GetDimension() - information meaningless!";
	return Vector3i(0, 0, 0);
}

bool Color3fProductTexture::IsConstant() const
{
	return m_pTextureA->IsConstant() && m_pTextureB->IsConstant();
}

bool Color3fProductTexture::IsMonochromatic() const
{
	return m_pTextureA->IsMonochromatic() && m_pTextureB->IsMonochromatic();
}

std::string Color3fProductTexture::ToString() const
{
	return tfm::format(
		"Color3fProductTexture[\n"
		" textureA = %s,\n"
		" textureB = %s,\n"
		"]",
		Indent(m_pTextureA->ToString()),
		Indent(m_pTextureB->ToString())
	);
}

NAMESPACE_END

