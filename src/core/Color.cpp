#include <core\Color.hpp>

NAMESPACE_BEGIN

Color3f::Color3f(float Value) : Base(Value, Value, Value) { }

Color3f::Color3f(float R, float G, float B) : Base(R, G, B) { }

float & Color3f::R()
{
	return x();
}

const float & Color3f::R() const
{
	return x();
}

float & Color3f::G()
{
	return y();
}

const float & Color3f::G() const
{
	return y();
}

float & Color3f::B()
{
	return z();
}

const float & Color3f::B() const
{
	return z();
}

Color3f Color3f::Clamp() const
{
	return Color3f(std::max(R(), 0.0f), std::max(G(), 0.0f), std::max(B(), 0.0f));
}

bool Color3f::IsValid() const
{
	for (int i = 0; i < 3; ++i)
	{
		float Value = coeff(i);
		if (Value < 0.0f || !std::isfinite(Value))
		{
			return false;
		}
	}
	return true;
}

Color3f Color3f::ToLinearRGB() const
{
	Color3f Result;
	for (int i = 0; i < 3; ++i)
	{
		float Value = coeff(i);

		if (Value <= 0.04045f)
		{
			Result[i] = Value * (1.0f / 12.92f);
		}
		else
		{
			Result[i] = std::pow((Value + 0.055f) * (1.0f / 1.055f), 2.4f);
		}
	}
	return Result;
}

Color3f Color3f::ToSRGB() const
{
	Color3f Result;
	for (int i = 0; i < 3; ++i)
	{
		float Value = coeff(i);

		if (Value <= 0.0031308f)
		{
			Result[i] = 12.92f * Value;
		}
		else
		{
			Result[i] = (1.0f + 0.055f) * std::pow(Value, 1.0f / 2.4f) - 0.055f;
		}
	}
	return Result;
}

float Color3f::GetLuminance() const
{
	return coeff(0) * 0.212671f + coeff(1) * 0.715160f + coeff(2) * 0.072169f;
}

std::string Color3f::ToString() const
{
	return tfm::format("[%f, %f, %f]", coeff(0), coeff(1), coeff(2));
}

Color4f::Color4f() : Base(0.0f, 0.0f, 0.0f, 0.0f) { }

Color4f::Color4f(const Color3f & Color) : Base(Color.R(), Color.G(), Color.B(), 1.0f) { }

Color4f::Color4f(float R, float G, float B, float W) : Base(R, G, B, W) { }

Color3f Color4f::DivideByFilterWeight() const
{
	if (w() != 0.0f)
	{
		return head<3>() / w();
	}
	else
	{
		return Color3f(0.0f);
	}
}

std::string Color4f::ToString() const
{
	return tfm::format("[%f, %f, %f, %f]", coeff(0), coeff(1), coeff(2), coeff(3));
}

NAMESPACE_END

