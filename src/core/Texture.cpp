#include <core\Texture.hpp>
#include <core\Intersection.hpp>

NAMESPACE_BEGIN

Color3f Texture::Eval(const Intersection & Isect, bool bFilter) const
{
	throw HikariException("Texture::Eval(const Intersection & Isect, bool bFilter) is not implemented!");
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

Color3f Texture2D::Eval(const Intersection & Isect, bool bFilter) const
{
	Point2f UV = Point2f(Isect.UV.x() * m_UVScale.x(), Isect.UV.y() * m_UVScale.y()) + m_UVOffset;

	if (bFilter)
	{
		return Eval(UV,
			Vector2f(Isect.dUdX * m_UVScale.x(), Isect.dVdX * m_UVScale.y()),
			Vector2f(Isect.dUdY * m_UVScale.x(), Isect.dVdY * m_UVScale.y()));
	}
	else
	{
		return Eval(UV);
	}
}

NAMESPACE_END

